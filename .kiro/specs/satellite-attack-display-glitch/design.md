# Satellite Attack Display Glitch Bugfix Design

## Overview

Satellite Attack (vp_34.bin) exhibits display tearing on ship/satellite characters during movement and inconsistent ship movement speed when holding up/down. Both issues stem from the VDC's `render_current_pixel()` reading graphic registers (`state_.registers[]`) directly on every pixel tick, whereas real 8245 hardware latches graphic register values at the start of each scanline. A secondary issue is that `get_beam_x_at_cpu_read()` adds a fixed offset that doesn't account for the CPU instruction's actual position within the master clock tick sequence, causing the game's `wait_until_scanline` timing loop to exit at variable horizontal positions across frames.

The fix introduces per-scanline latching of graphic registers (0x00-0x7F) and ensures beam X reporting is deterministic relative to the master clock cycle count. Non-graphic registers (control, status, collision, color, sound, grid) retain their current immediate-write timing.

## Glossary

- **Bug_Condition (C)**: CPU writes to graphic registers (0x00-0x7F) while the VDC is rendering the visible portion of a scanline, OR CPU reads beam X register (0xA4) and gets a non-deterministic value
- **Property (P)**: Graphic register writes during visible scanline rendering do not affect the current scanline's output; beam X reads return values deterministic to the master clock tick
- **Preservation**: All non-graphic register timing, HBLANK/VBLANK write visibility on next scanline, normal rendering without mid-scanline writes, and status register behavior remain unchanged
- **`render_current_pixel()`**: The per-pixel rendering function in `src/vdc.cpp` that composites background, grid, characters, and sprites into the framebuffer
- **`is_character_pixel_at()` / `is_sprite_pixel_at()`**: Per-pixel helpers that read character/sprite position, pattern, and color from `state_.registers[]` to determine if an object is present at a given pixel
- **`get_beam_x_at_cpu_read()`**: Method in `src/master_clock.cpp` that returns the beam X value at the point where the CPU's MOVX RD strobe samples the bus
- **Graphic registers**: VDC addresses 0x00-0x7F (sprites 0x00-0x0F, characters 0x10-0x3F, quad characters 0x40-0x7F, sprite patterns 0x80-0x9F)
- **Latched registers**: A snapshot of graphic register values taken at the start of each visible scanline, used by the rendering pipeline instead of live register values

## Bug Details

### Bug Condition

The bug manifests in two related ways: (1) when the CPU writes to graphic registers (0x00-0x7F) while the VDC is actively rendering a visible scanline, the new values take effect immediately mid-scanline causing visual tearing; (2) when the game's timing loop reads beam X, the returned value is not fully deterministic to the master clock state, causing variable loop exit timing and inconsistent ship speed.

**Formal Specification:**
```
FUNCTION isBugCondition(input)
  INPUT: input of type {operation: READ|WRITE, address: uint8, master_tick: uint64, scanline: uint16, scanline_tick: uint32}
  OUTPUT: boolean

  // Condition 1: Mid-scanline write to graphic register during visible rendering
  IF input.operation == WRITE
     AND input.address <= 0x9F
     AND input.scanline < VBLANK_START_LINE
     AND input.scanline_tick > 0
     AND input.scanline_tick < HBLANK_START_TICK
  THEN RETURN true

  // Condition 2: Beam X read returns non-deterministic value
  IF input.operation == READ
     AND input.address == 0xA4
     AND get_beam_x_at_cpu_read(input.master_tick) != get_beam_x_at_cpu_read_deterministic(input.master_tick)
  THEN RETURN true

  RETURN false
END FUNCTION
```

### Examples

- **Tearing example**: Ship character at Y=100, X=50. CPU writes new Y=101 at scanline_tick=200 (mid-visible-scanline). Current behavior: pixels before tick 200 render at Y=100, pixels after render at Y=101 on the same scanline — the ship appears split. Expected: entire scanline renders at Y=100, new position takes effect on scanline 101.

- **Sprite tearing**: Satellite sprite at Y=60, X=80. CPU writes new X=82 at scanline_tick=150. Current behavior: sprite appears partially at X=80 and partially at X=82 on the same scanline. Expected: entire scanline renders sprite at X=80, new X takes effect next scanline.

- **Speed inconsistency**: Game loop at 0x71a reads beam X (register 0xA4), adds 0x27, loops until overflow. If beam X at the CPU read strobe varies by ±1 across frames due to the fixed offset in `get_beam_x_at_cpu_read()`, the loop exits 1 iteration early or late, giving ~20 master ticks more or fewer CPU cycles for ship movement logic. Over sustained input, this manifests as jerky/inconsistent movement speed.

- **VBLANK write (non-bug)**: Game writes character position during VBLANK. Current and expected behavior are identical — the write takes effect before the next visible scanline. This must remain unchanged.

## Expected Behavior

### Preservation Requirements

**Unchanged Behaviors:**
- CPU writes to graphic registers (0x00-0x9F) during HBLANK or VBLANK must take effect before the next visible scanline begins rendering (Req 3.1)
- CPU writes to non-graphic registers (control 0xA0, status 0xA1, collision 0xA2, color 0xA3, sound 0xA7-0xAA, grid 0xC0-0xE9) must retain current immediate-write timing (Req 3.2)
- Characters and sprites rendered without any mid-scanline register updates must appear at correct positions with correct patterns, colors, and priority (Req 3.3)
- Beam X reads during VBLANK or HBLANK must return correct values consistent with master clock state (Req 3.4)
- Status register (0xA1) read behavior — HBLANK, VBLANK, collision bits, clear-on-read semantics — must remain unchanged (Req 3.5)
- Games that only write graphic registers during VBLANK (the standard pattern) must display identically, since scanline latching is transparent when writes occur during blanking (Req 3.6)

**Scope:**
All VDC operations that do NOT involve mid-scanline writes to graphic registers should be completely unaffected by this fix. This includes:
- All register reads (status, collision, beam position)
- All non-graphic register writes (control, color, sound, grid)
- All graphic register writes that occur during HBLANK or VBLANK
- All rendering when no mid-scanline graphic register writes occur

## Hypothesized Root Cause

Based on the bug description and code analysis, the root causes are:

1. **No scanline-level register latching**: `render_current_pixel()` calls `is_character_pixel_at()` and `is_sprite_pixel_at()` which read directly from `state_.registers[]` on every pixel tick. When `write_register()` updates `state_.registers[addr] = value` mid-scanline, the rendering pipeline immediately sees the new value. Real 8245 hardware latches object register values into internal shift registers at the start of each scanline's active display period, so mid-scanline CPU writes don't affect the current scanline's output.

   - `is_character_pixel_at()` (line 2055): reads `state_.registers[base_addr + 0]` (Y), `[base_addr + 1]` (X), `[base_addr + 2]` (pattern ptr), `[base_addr + 3]` (color/attr) for all 12 single characters and 4 quad groups
   - `is_sprite_pixel_at()` (line 2182): reads `state_.registers[base_addr + 0]` (Y), `[base_addr + 1]` (X), `[base_addr + 2]` (color/attr) and pattern registers for all 4 sprites
   - Both functions are called from `render_current_pixel()` and `detect_collision_at_pixel()` on every VDC cycle

2. **Beam X read offset approximation**: `get_beam_x_at_cpu_read()` in `src/master_clock.cpp` adds a fixed `cpu_tick_divisor_` offset (20 master ticks NTSC) to `scanline_tick_` to model the MOVX RD strobe timing. However, the actual CPU read doesn't always occur at exactly `scanline_tick_ + cpu_tick_divisor_` — the CPU instruction may have started at different phases within the CPU tick slot. The current implementation is close but can produce ±1 X value jitter depending on when within the CPU tick window the read executes, which is enough to cause the game's tight timing loop to vary.

## Correctness Properties

Property 1: Bug Condition - Mid-Scanline Graphic Register Writes Do Not Affect Current Scanline

_For any_ VDC state where a CPU write to a graphic register (0x00-0x9F) occurs while the beam is in the visible portion of a scanline (scanline < VBLANK_START and 0 < scanline_tick < HBLANK_START_TICK), the rendering output for the remainder of that scanline SHALL use the register values that were present at the start of that scanline, not the newly written values.

**Validates: Requirements 2.1, 2.2, 2.5**

Property 2: Preservation - Rendering Without Mid-Scanline Writes Is Unchanged

_For any_ VDC state where no CPU writes to graphic registers occur during the visible portion of a scanline, the rendering output SHALL be identical to the output produced by the original (unfixed) code, preserving all character positions, sprite positions, patterns, colors, and priority ordering.

**Validates: Requirements 3.1, 3.3, 3.6**

Property 3: Bug Condition - Beam X Reads Are Deterministic

_For any_ CPU read of the beam X register (0xA4) at a given master clock tick count, the returned value SHALL be identical across repeated executions and SHALL be a deterministic function of the master clock's `scanline_tick_` at the point where the CPU's bus read strobe samples the data.

**Validates: Requirements 2.3, 2.4**

Property 4: Preservation - HBLANK/VBLANK Writes Visible on Next Scanline

_For any_ CPU write to a graphic register (0x00-0x9F) that occurs during HBLANK or VBLANK, the written value SHALL be present in the latched register snapshot used for rendering the next visible scanline, preserving the existing behavior where blanking-period writes take effect immediately.

**Validates: Requirements 3.1, 3.4**

Property 5: Preservation - Non-Graphic Register Timing Unchanged

_For any_ CPU write to a non-graphic register (0xA0-0xFF: control, status, collision, color, sound, grid), the write SHALL take effect immediately with the same timing as the original code, preserving all existing non-graphic register behavior.

**Validates: Requirements 3.2, 3.5**

## Fix Implementation

### Changes Required

Assuming our root cause analysis is correct:

**File**: `include/vdc.h`

**Changes**:
1. **Add latched register array**: Add a `uint8 latched_registers[160]` array (covering 0x00-0x9F) to `VDCState` to hold the per-scanline snapshot of graphic registers.
2. **Add latch method declaration**: Declare `void latch_graphic_registers()` private method.

**File**: `src/vdc.cpp`

**Function**: `tick_one_cycle()`

**Specific Changes**:
1. **Latch on scanline transition**: In the scanline transition detection block (where `new_y != state_.prev_scanline`), call `latch_graphic_registers()` to snapshot registers 0x00-0x9F into `state_.latched_registers[]`. This ensures the latch happens at the start of each new scanline, before any pixels are rendered.

2. **Implement `latch_graphic_registers()`**: Simple `memcpy` of `state_.registers[0x00]` through `state_.registers[0x9F]` into `state_.latched_registers[]`. This captures sprite control (0x00-0x0F), character control (0x10-0x3F), quad character control (0x40-0x7F), and sprite patterns (0x80-0x9F).

**Function**: `is_character_pixel_at()`

**Specific Changes**:
3. **Read from latched registers**: Replace all reads of `state_.registers[base_addr + N]` for character position, pattern pointer, and color attributes with `state_.latched_registers[base_addr + N]`. This applies to both single characters (0x10-0x3F) and quad characters (0x40-0x7F).

**Function**: `is_sprite_pixel_at()`

**Specific Changes**:
4. **Read from latched registers**: Replace all reads of `state_.registers[base_addr + N]` for sprite position, color attributes, and pattern data with `state_.latched_registers[base_addr + N]`. This applies to all 4 sprites (0x00-0x0F) and their patterns (0x80-0x9F).

**Function**: `detect_collision_at_pixel()`

**Specific Changes**:
5. **Read from latched registers for collision**: The collision detection also calls `is_character_pixel_at()` and reads sprite registers directly. The sprite register reads in `detect_collision_at_pixel()` must also use `state_.latched_registers[]` for consistency — collision detection should see the same object positions as rendering.

**File**: `src/master_clock.cpp`

**Function**: `get_beam_x_at_cpu_read()`

**Specific Changes**:
6. **Deterministic beam X**: The current implementation adds `cpu_tick_divisor_` to `scanline_tick_`. This is already close to correct — the key insight is that the CPU can only execute on CPU tick boundaries (every `cpu_tick_divisor_` master ticks), so the MOVX read strobe always occurs at a deterministic master tick. Verify that the offset correctly models the RD strobe timing relative to the CPU tick slot start. If the current offset produces ±1 jitter, adjust to use the exact tick at which the RD strobe fires (which should be `scanline_tick_` at the time of the CPU tick + the RD strobe delay within the instruction).

**Function**: `write_register()`

**Specific Changes**:
7. **No change to write_register**: `write_register()` continues to write directly to `state_.registers[]`. The latching mechanism means these writes won't affect rendering until the next scanline latch. Writes during HBLANK/VBLANK will be captured by the latch at the start of the next visible scanline, preserving Req 3.1.

## Testing Strategy

### Validation Approach

The testing strategy follows a two-phase approach: first, surface counterexamples that demonstrate the bug on unfixed code, then verify the fix works correctly and preserves existing behavior.

### Exploratory Bug Condition Checking

**Goal**: Surface counterexamples that demonstrate the bug BEFORE implementing the fix. Confirm or refute the root cause analysis. If we refute, we will need to re-hypothesize.

**Test Plan**: Write tests that set up character/sprite positions, advance the VDC to mid-scanline, write new position values, then continue rendering and check the framebuffer for tearing artifacts. Run these tests on the UNFIXED code to observe failures.

**Test Cases**:
1. **Character Mid-Scanline Position Write**: Place a character at Y=50, X=40. Advance VDC to scanline 50, tick to mid-visible area, write X=60, continue rendering. Check that pixels after the write appear at X=60 (demonstrating the bug on unfixed code).
2. **Sprite Mid-Scanline Position Write**: Place sprite 0 at Y=30, X=20. Advance to scanline 30, tick to mid-visible, write X=40, continue. Check for split rendering (will show bug on unfixed code).
3. **Character Pattern Write Mid-Scanline**: Place character, advance to mid-scanline, write new pattern pointer. Check that pattern changes mid-scanline (will show bug on unfixed code).
4. **VBLANK Write Baseline**: Write character position during VBLANK, render next frame. Verify position is correct (should pass on both unfixed and fixed code — establishes baseline).

**Expected Counterexamples**:
- Framebuffer shows character/sprite pixels at BOTH old and new positions on the same scanline
- Possible cause confirmed: `is_character_pixel_at()` / `is_sprite_pixel_at()` reading live `state_.registers[]`

### Fix Checking

**Goal**: Verify that for all inputs where the bug condition holds, the fixed function produces the expected behavior.

**Pseudocode:**
```
FOR ALL input WHERE isBugCondition(input) DO
  // Set up VDC with random character/sprite configuration
  // Advance to mid-scanline
  // Write new register value
  // Continue rendering to end of scanline
  result := render_scanline_fixed(input)
  ASSERT all_pixels_use_pre_write_values(result)
END FOR
```

### Preservation Checking

**Goal**: Verify that for all inputs where the bug condition does NOT hold, the fixed function produces the same result as the original function.

**Pseudocode:**
```
FOR ALL input WHERE NOT isBugCondition(input) DO
  ASSERT render_original(input) == render_fixed(input)
END FOR
```

**Testing Approach**: Property-based testing is recommended for preservation checking because:
- It generates many random VDC register configurations and verifies rendering is identical
- It catches edge cases in character/sprite positioning that manual tests might miss
- It provides strong guarantees that the latching mechanism is transparent for non-mid-scanline-write scenarios

**Test Plan**: Observe rendering behavior on UNFIXED code for various register configurations written during VBLANK, then write property-based tests verifying the fixed code produces identical output.

**Test Cases**:
1. **VBLANK Write Preservation**: Generate random graphic register values, write during VBLANK, render next scanline. Verify output matches original code.
2. **HBLANK Write Preservation**: Generate random graphic register values, write during HBLANK, render next scanline. Verify output matches original code.
3. **No-Write Rendering Preservation**: Generate random VDC states (no mid-scanline writes), render full scanline. Verify pixel-for-pixel identical output.
4. **Non-Graphic Register Preservation**: Write to control, color, sound, grid registers at various times. Verify behavior unchanged.

### Unit Tests

- Test `latch_graphic_registers()` copies registers 0x00-0x9F correctly
- Test that writes during visible scanline don't affect `latched_registers[]` until next scanline
- Test that writes during HBLANK/VBLANK are captured by the next latch
- Test `get_beam_x_at_cpu_read()` returns deterministic values for same `scanline_tick_`
- Test character rendering uses latched position after mid-scanline write
- Test sprite rendering uses latched position after mid-scanline write
- Test collision detection uses latched positions

### Property-Based Tests

- Generate random character/sprite configurations and random mid-scanline write timings; verify no tearing (all pixels on a scanline use pre-write values)
- Generate random VDC states with VBLANK-only writes; verify rendering output identical to original code
- Generate random `scanline_tick_` values; verify `get_beam_x_at_cpu_read()` is a monotonically increasing deterministic function of tick position
- Generate random non-graphic register write sequences; verify behavior unchanged from original

### Integration Tests

- Run Satellite Attack ROM for 60 frames with sustained up input; verify ship Y position changes by constant delta per frame
- Run Satellite Attack ROM for 10 frames; capture framebuffer and verify no scanline shows character pixels at two different X positions
- Run Killer Bees ROM (writes during VBLANK) for 60 frames; verify display matches pre-fix output (regression check)
