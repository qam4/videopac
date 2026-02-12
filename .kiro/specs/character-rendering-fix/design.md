# Design Document: Character Rendering Fix

## Overview

This design addresses the character rendering bug in the Videopac emulator by implementing a proper master clock synchronization architecture. The current "time-slice" approach (CPU executes for a scanline, then VDC renders that scanline) causes timing mismatches where the VDC's internal scanline counter advances during CPU execution, resulting in the wrong scanline being rendered.

The solution implements cycle-accurate emulation where CPU and VDC execute concurrently based on a unified master clock. The VDC will render continuously as the beam position advances, not in discrete scanline chunks. This ensures that register writes take effect immediately at the current beam position, matching real hardware behavior.

### Key Design Principles

1. **Master Clock as Source of Truth**: Use VDC 3.54 MHz clock as the base unit for all timing
2. **Cycle Debt Tracking**: Determine which component executes next based on accumulated cycle debt
3. **Continuous Rendering**: VDC renders pixels as beam advances, not in scanline chunks
4. **Immediate Register Effects**: VDC register writes take effect at current beam position
5. **Backward Compatibility**: Maintain existing interfaces for debugger, save state, and frontend

## Architecture

### Master Clock System

The emulator will use a master clock based on the VDC's 3.54 MHz clock frequency. Both CPU and VDC will advance based on this unified clock, with cycle debt tracking determining execution order.

```
Master Clock (3.54 MHz)
         |
         v
    +---------+
    |  Cycle  |
    |  Debt   |
    | Tracker |
    +---------+
      /     \
     /       \
    v         v
  CPU       VDC
(~358kHz)  (3.54MHz)
```

### Clock Frequency Relationships

- **VDC Clock**: 3.54 MHz (base unit)
- **CPU Instruction Cycle**: ~358 kHz (1.79 MHz / 5)
- **Cycles per CPU instruction**: ~9.9 VDC clock cycles (3.54 MHz / 358 kHz)
- **VDC cycles per scanline**: 
  - NTSC: ~227 cycles (3.54 MHz / 60 Hz / 262 scanlines)
  - PAL: ~227 cycles (3.54 MHz / 50 Hz / 312 scanlines)

### Modified Execution Loop

The current `run_frame()` loop will be replaced with a cycle-accurate loop:

**Current (Time-Slice) Approach:**
```
for each scanline:
    execute CPU for scanline_cycles
    render scanline
```

**New (Cycle-Accurate) Approach:**
```
while not end_of_frame:
    if cpu_debt >= cpu_instruction_threshold:
        execute one CPU instruction
        cpu_debt -= instruction_cycles
        vdc_debt += instruction_cycles
    
    while vdc_debt > 0:
        advance VDC by 1 cycle
        render pixel at current beam position
        vdc_debt -= 1
```

## Components and Interfaces

### 1. Master Clock Manager

A new component responsible for coordinating CPU and VDC execution.

```cpp
class MasterClock {
public:
    MasterClock(VideoStandard standard);
    
    // Advance the master clock and determine what to execute next
    enum class ExecuteNext { CPU, VDC, FRAME_COMPLETE };
    ExecuteNext tick();
    
    // Notify that CPU executed an instruction
    void cpu_executed(uint8 instruction_cycles);
    
    // Notify that VDC advanced one cycle
    void vdc_ticked();
    
    // Check if frame is complete
    bool is_frame_complete() const;
    
    // Reset for new frame
    void reset_frame();
    
private:
    VideoStandard standard_;
    uint64 master_cycle_count_;
    double cpu_cycle_debt_;
    double vdc_cycle_debt_;
    uint32 cycles_per_frame_;
    
    static constexpr double VDC_CLOCK_MHZ = 3.54;
    static constexpr double CPU_CLOCK_MHZ = 1.79 / 5.0;  // ~0.358 MHz
    static constexpr double CYCLES_PER_CPU_INSTRUCTION = VDC_CLOCK_MHZ / CPU_CLOCK_MHZ;  // ~9.9
};
```

### 2. Modified VDC Interface

The VDC will be modified to support continuous rendering instead of scanline-based rendering.

**Removed Methods:**
- `void render_scanline()` - No longer renders in scanline chunks

**New/Modified Methods:**
```cpp
class VDC {
public:
    // Advance VDC by exactly 1 clock cycle
    void tick_one_cycle();
    
    // Render pixel at current beam position (if visible)
    void render_current_pixel();
    
    // Get current beam position
    uint16 get_beam_x() const { return state_.beam_x; }
    uint16 get_beam_y() const { return state_.beam_y; }
    
    // Check if beam is in visible area
    bool is_beam_visible() const;
    
    // Existing methods remain unchanged
    void write_register(uint8 address, uint8 value);
    uint8 read_register(uint8 address);
    bool is_vblank() const;
    bool is_hblank() const;
    // ...
};
```

### 3. Modified EmulatorCore

The emulator core will use the master clock to coordinate execution.

```cpp
class EmulatorCore {
public:
    // Modified run_frame() using master clock
    void run_frame();
    
private:
    MasterClock master_clock_;
    
    // Helper methods
    void execute_cpu_instruction();
    void advance_vdc_one_cycle();
};
```

## Data Models

### Master Clock State

```cpp
struct MasterClockState {
    uint64 master_cycle_count;      // Total VDC cycles since start
    double cpu_cycle_debt;           // Accumulated CPU cycles to execute
    double vdc_cycle_debt;           // Accumulated VDC cycles to execute
    uint32 frame_cycle_count;        // Cycles in current frame
    uint32 cycles_per_frame;         // Total cycles per frame
};
```

### Modified VDC State

The existing `VDCState` structure will be modified to support continuous rendering:

```cpp
struct VDCState {
    // Existing fields remain...
    uint8 registers[256];
    uint8 framebuffer[FRAMEBUFFER_HEIGHT][FRAMEBUFFER_WIDTH];
    
    // Modified timing fields
    uint16 beam_x;                   // Horizontal beam position (0-227 for full scanline)
    uint16 beam_y;                   // Vertical beam position (0-261 NTSC, 0-311 PAL)
    uint64 total_cycles;             // Total VDC cycles since reset
    
    // Removed fields:
    // - scanline (replaced by beam_y)
    // - cycle_counter (replaced by beam_x calculation)
    
    // Existing fields remain...
    VideoStandard video_standard;
    uint8 collision_state;
    bool collision_detected;
    bool display_enabled;
    bool grid_enabled;
    // ... audio fields ...
};
```

### Beam Position Calculation

The beam position is calculated from the total cycle count:

```cpp
// Cycles per scanline (including HBLANK)
constexpr uint32 CYCLES_PER_SCANLINE = 227;

// Calculate beam position from total cycles
uint32 frame_cycles = total_cycles % cycles_per_frame;
beam_y = frame_cycles / CYCLES_PER_SCANLINE;
beam_x = frame_cycles % CYCLES_PER_SCANLINE;

// Visible area
constexpr uint16 VISIBLE_X_START = 0;
constexpr uint16 VISIBLE_X_END = 160;
constexpr uint16 VISIBLE_Y_START = 0;
constexpr uint16 VISIBLE_Y_END = 192;  // NTSC: 192, PAL: 240
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*


### Property Reflection

After analyzing all acceptance criteria, I've identified several areas where properties can be consolidated:

**Redundancy Analysis:**

1. **Master Clock Properties (1.1-1.5)**: These can be consolidated into fewer properties about clock synchronization behavior
2. **Display Enable (3.1-3.2)**: These are inverse conditions that can be combined into one property
3. **Position Bounds Checking (4.1-4.3, 11.1-11.4)**: Multiple properties about bounds checking can be consolidated
4. **ROM Address Calculation (5.1-5.5)**: These are all about the same calculation and can be combined
5. **Pattern Rendering (6.1-6.6)**: Multiple properties about pixel rendering can be consolidated
6. **Character Types (8.1-8.4, 9.1-9.5)**: Properties about single vs quad characters have similar patterns
7. **Layer Priority (12.1-12.4)**: All about rendering order, can be consolidated

**Consolidated Properties:**

After reflection, the following properties provide unique validation value without redundancy:

- Master clock synchronization and cycle debt tracking
- Immediate register effect at beam position
- Continuous rendering with beam advancement
- Beam position calculation from cycle count
- Display enable controls character visibility
- Character position bounds checking
- ROM address calculation and validation
- Pattern byte rendering with correct bit order
- Color attribute extraction
- Single and quad character support
- Rendering layer priority

### Correctness Properties

Property 1: Master clock cycle ratio
*For any* CPU instruction execution, the VDC cycle debt should increase by approximately 9.9 cycles (3.54 MHz / 358 kHz ratio)
**Validates: Requirements 1.2**

Property 2: Cycle debt determines execution order
*For any* master clock state, the component (CPU or VDC) with the higher cycle debt should be selected to execute next
**Validates: Requirements 1.3**

Property 3: Immediate register effect
*For any* VDC register write during frame rendering, the register value should be reflected at the current beam position, not deferred to the end of the scanline
**Validates: Requirements 1.7**

Property 4: Beam position advances with VDC cycles
*For any* VDC clock tick, the beam position (beam_x, beam_y) should advance by exactly one position in the raster scan pattern
**Validates: Requirements 2.3, 2.4**

Property 5: Beam position calculation from cycle count
*For any* total cycle count, the beam position should be calculated as: beam_y = (cycles % cycles_per_frame) / cycles_per_scanline, beam_x = (cycles % cycles_per_frame) % cycles_per_scanline
**Validates: Requirements 2.4**

Property 6: No rendering during blanking
*For any* beam position during HBLANK or VBLANK periods, no pixels should be written to the framebuffer
**Validates: Requirements 2.6**

Property 7: Display enable controls character rendering
*For any* frame, characters should be rendered if and only if bit 5 of register 0xA0 is set to 1
**Validates: Requirements 3.1, 3.2, 3.3**

Property 8: Character position bounds checking
*For any* character with position (x, y), the character should be rendered if and only if y is in range [0, 191] and x is in range [0, 159]
**Validates: Requirements 4.1, 4.2, 4.3, 11.1, 11.2**

Property 9: Single character position extraction
*For any* single character at index i (0-11), the X position should be read from register (0x10 + i*4 + 1) and Y position from register (0x10 + i*4 + 0)
**Validates: Requirements 4.4**

Property 10: Quad character position calculation
*For any* quad character group q (0-3) and sub-character s (0-3), the X position should be calculated as: base_x + (s * 8), where base_x is from register (0x40 + q*16 + 13)
**Validates: Requirements 4.5, 9.4, 9.5**

Property 11: ROM address calculation
*For any* character with char_ptr, char_y, and char_row, the ROM address should be calculated as: (char_ptr + (char_y / 2) + char_row) & 0x1FF
**Validates: Requirements 5.1, 5.4, 5.5**

Property 12: ROM address bounds validation
*For any* calculated ROM address, if the address is >= 512, the character should not be rendered
**Validates: Requirements 5.2, 11.3**

Property 13: Character pointer extraction
*For any* character attribute bytes (byte2, byte3), the 9-bit character pointer should be extracted as: (byte2 & 0xFF) | ((byte3 & 0x01) << 8)
**Validates: Requirements 5.3**

Property 14: Pattern byte pixel rendering
*For any* pattern byte and pixel position x (0-7), a pixel should be drawn if and only if bit (7-x) of the pattern byte is 1
**Validates: Requirements 6.2, 6.3, 6.4**

Property 15: Character width and height
*For any* character, exactly 8 pixels should be rendered horizontally per scanline, and each character row should span exactly 2 scanlines vertically
**Validates: Requirements 6.5, 6.6**

Property 16: Color attribute extraction
*For any* character attribute byte, the color index should be extracted as: (attribute_byte >> 1) & 0x07, yielding a value in range [0, 7]
**Validates: Requirements 7.1, 7.2**

Property 17: Single character capacity
*For any* frame, up to 12 single characters (indices 0-11) should be renderable simultaneously, with control data read from registers 0x10-0x3F (4 bytes per character)
**Validates: Requirements 8.1, 8.2**

Property 18: Quad character capacity
*For any* frame, up to 4 quad character groups (indices 0-3) should be renderable simultaneously, with control data read from registers 0x40-0x7F (16 bytes per quad)
**Validates: Requirements 9.1, 9.2**

Property 19: Quad character spacing
*For any* quad character group, all 4 sub-characters should be rendered with exactly 8 pixels of horizontal spacing between them
**Validates: Requirements 9.3**

Property 20: Character ROM structure
*For any* character index c (0-63), the character pattern should occupy exactly 8 bytes at ROM addresses [c*8, c*8+7], for a total ROM size of 512 bytes
**Validates: Requirements 10.2, 10.4**

Property 21: Rendering layer priority
*For any* pixel position, the visible color should follow priority order: sprites (highest) > characters > grid > background (lowest)
**Validates: Requirements 12.1, 12.2, 12.3, 12.4**

## Error Handling

### Invalid ROM Addresses

When a calculated ROM address is >= 512, the character rendering should be skipped entirely for that character. This prevents buffer overruns and undefined behavior.

```cpp
uint16 rom_addr = (char_ptr + (char_y / 2) + char_row) & 0x1FF;
if (rom_addr >= 512) {
    return;  // Skip this character
}
```

### Out-of-Bounds Positions

Characters with positions outside the visible area should be skipped during rendering. Bounds checking should occur before any framebuffer writes.

```cpp
if (char_y >= FRAMEBUFFER_HEIGHT || char_x >= FRAMEBUFFER_WIDTH) {
    continue;  // Skip this character
}

// Per-pixel bounds checking
if (screen_x < 0 || screen_x >= FRAMEBUFFER_WIDTH) {
    continue;  // Skip this pixel
}
```

### Cycle Debt Overflow

The cycle debt accumulators use floating-point arithmetic to maintain precision. To prevent overflow, the debt values should be clamped or reset at frame boundaries.

```cpp
// Reset debt at frame boundary
if (is_frame_complete()) {
    cpu_cycle_debt_ = 0.0;
    vdc_cycle_debt_ = 0.0;
}
```

### HBLANK/VBLANK Rendering

During blanking periods, no pixels should be written to the framebuffer. The rendering functions should check the beam position before writing.

```cpp
bool VDC::is_beam_visible() const {
    return beam_x < VISIBLE_X_END && 
           beam_y < VISIBLE_Y_END &&
           !is_hblank() && 
           !is_vblank();
}
```

## Testing Strategy

### Dual Testing Approach

This design requires both unit tests and property-based tests for comprehensive coverage:

**Unit Tests** focus on:
- Specific examples of ROM address calculation
- Edge cases (boundary positions, maximum values)
- Error conditions (invalid ROM addresses, out-of-bounds positions)
- Integration points (CPU-VDC synchronization, register writes)

**Property-Based Tests** focus on:
- Universal properties across all inputs (cycle ratios, beam calculations)
- Randomized character positions and attributes
- Comprehensive input coverage through generation

Together, unit tests catch concrete bugs while property tests verify general correctness.

### Property-Based Testing Configuration

All property-based tests should:
- Use the fast-check library for C++ (or similar PBT framework)
- Run minimum 100 iterations per test
- Tag each test with the property number and text
- Reference the design document property

Example test structure:
```cpp
// Feature: character-rendering-fix, Property 11: ROM address calculation
TEST(CharacterRenderingPBT, ROMAddressCalculation) {
    fc::check(fc::property([](uint16 char_ptr, uint8 char_y, uint8 char_row) {
        // Constrain inputs to valid ranges
        char_ptr = char_ptr & 0x1FF;  // 9-bit
        char_y = char_y % 192;         // Visible range
        char_row = char_row % 8;       // 8 rows per character
        
        uint16 rom_addr = (char_ptr + (char_y / 2) + char_row) & 0x1FF;
        
        // Property: ROM address should be in valid range
        return rom_addr < 512;
    }), fc::numRuns(100));
}
```

### Test Coverage Requirements

1. **Master Clock Tests**:
   - Unit: Test specific cycle counts and debt calculations
   - Property: Verify cycle ratio holds for all instruction types

2. **Beam Position Tests**:
   - Unit: Test specific cycle counts map to expected positions
   - Property: Verify beam position calculation for all cycle counts

3. **Character Rendering Tests**:
   - Unit: Test specific character patterns render correctly
   - Property: Verify ROM address calculation for all valid inputs
   - Property: Verify bounds checking for all positions
   - Property: Verify color extraction for all attribute values

4. **Register Write Tests**:
   - Unit: Test specific register writes at specific beam positions
   - Property: Verify immediate effect for all register addresses

5. **Integration Tests**:
   - Test complete frame rendering with known character data
   - Test Satellite Attack game scenario (the original bug)
   - Test mid-scanline register writes

## Implementation Impact

### Files to Modify

1. **include/emulator.h** - Add MasterClock member, modify run_frame() signature
2. **src/emulator.cpp** - Rewrite run_frame() to use master clock
3. **include/vdc.h** - Add tick_one_cycle(), remove render_scanline(), modify state
4. **src/vdc.cpp** - Implement continuous rendering, modify tick() behavior
5. **include/types.h** - Add MasterClockState structure

### Files to Create

1. **include/master_clock.h** - MasterClock class definition
2. **src/master_clock.cpp** - MasterClock implementation
3. **tests/test_master_clock.cpp** - Unit tests for master clock
4. **tests/test_character_rendering_pbt.cpp** - Property-based tests

### Backward Compatibility

**Maintained Interfaces:**
- `VDC::write_register()` - No change
- `VDC::read_register()` - No change
- `VDC::get_framebuffer()` - No change
- `VDC::get_state()` / `set_state()` - Modified state structure, but interface unchanged
- `EmulatorCore::run_frame()` - No signature change
- Debugger interface - No changes required
- Save state - State structure modified, but serialization handles it

**Breaking Changes:**
- `VDC::render_scanline()` - Removed (internal method, not used by external code)
- `VDC::tick(uint8 cycles)` - Modified to `tick_one_cycle()` (internal method)
- `VDCState` structure - Fields modified (scanline → beam_y, cycle_counter removed)

**Migration Path:**
- Existing save states will need version migration to convert old VDCState to new format
- Debugger display of VDC state will show beam_x/beam_y instead of scanline/cycle_counter
- No changes required to frontend or input handling

### Performance Considerations

**Potential Performance Impact:**
- More frequent function calls (tick_one_cycle vs tick(N))
- Floating-point arithmetic for cycle debt tracking
- Per-pixel rendering checks instead of per-scanline

**Mitigation Strategies:**
- Inline tick_one_cycle() for performance
- Use fast beam position calculation (modulo arithmetic)
- Cache visibility checks per scanline
- Profile and optimize hot paths

**Expected Performance:**
- Modern CPUs should handle the increased call frequency
- Cycle-accurate emulation is standard in modern emulators
- The correctness benefit outweighs minor performance cost

## Alternative Designs Considered

### Alternative 1: Hybrid Time-Slice with Finer Granularity

Instead of full cycle-accurate emulation, use smaller time slices (e.g., per-pixel instead of per-scanline).

**Pros:**
- Less invasive changes to existing code
- Better performance than full cycle-accurate

**Cons:**
- Still has timing mismatches, just smaller ones
- Doesn't fully solve the mid-scanline register write problem
- More complex to reason about correctness

**Decision:** Rejected - Doesn't fully solve the root cause

### Alternative 2: Scanline Buffering with Register Snapshots

Keep scanline-based rendering but snapshot register state at scanline start.

**Pros:**
- Minimal changes to rendering code
- Good performance

**Cons:**
- Doesn't support mid-scanline register writes
- Doesn't match hardware behavior
- Won't fix timing-sensitive games

**Decision:** Rejected - Doesn't match hardware behavior

### Alternative 3: Event-Based Scheduling

Use an event queue to schedule CPU and VDC events.

**Pros:**
- Very flexible for adding new components
- Clean separation of concerns
- Easy to add timing-sensitive features

**Cons:**
- More complex implementation
- Higher memory overhead
- Overkill for current needs

**Decision:** Rejected - Too complex for current requirements, but could be future enhancement

## Future Enhancements

1. **Event-Based Scheduling**: Migrate to event queue system for better extensibility
2. **Cycle-Accurate Audio**: Apply same master clock approach to audio generation
3. **Horizontal Line Interrupts**: Support mid-scanline interrupts using beam position
4. **Performance Profiling**: Add instrumentation to measure emulation performance
5. **Regression Test Suite**: Build comprehensive test suite using property-based testing

## References

- **doc/o2doc.md**: Videopac/Odyssey2 hardware documentation
- **doc/8245.md**: Intel 8245 VDC chip documentation
- **doc/satellite-attack-disassembly.txt**: Game code analysis showing character usage
- **Existing Implementation**: src/vdc.cpp, src/emulator.cpp
- **Intel 8245 Datasheet**: Original hardware specifications
- **Cycle-Accurate Emulation**: Standard practice in modern emulators (MAME, Higan, etc.)
