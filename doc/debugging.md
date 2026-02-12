# Debugging Guide

## Overview

The emulator includes a built-in debugger with breakpoints, single-stepping, state inspection, and trace logging capabilities.

## VDC Display Coordinate System

### Display Dimensions

The Intel 8245 VDC (Video Display Controller) generates a 160×200 pixel display with the following coordinate system:

```
CRT Scanline Timing (NTSC: 262 lines, PAL: 312 lines):

Scanline 0 (top) ──────────────────────────────────────────────
                 ┌─────────────────────────────────────────┐
                 │                                         │
                 │      VISIBLE DISPLAY AREA               │
                 │      Scanlines 0-239 (NTSC)             │
                 │      Scanlines 0-287 (PAL)              │
                 │                                         │
                 │  Framebuffer: 160×200 pixels            │
                 │  X: 0-159 (160 pixels wide)             │
                 │  Y: 0-199 (200 pixels tall)             │
                 │                                         │
                 │  Coordinate System:                     │
                 │  • Origin (0,0) at TOP-LEFT             │
                 │  • X increases rightward →              │
                 │  • Y increases downward ↓               │
                 │                                         │
                 │  Character Visibility:                  │
                 │  • Y: 0-186 = fully visible             │
                 │  • Y: 187-199 = partially visible       │
                 │  • Y: 200+ = off-screen (below)         │
                 │                                         │
                 └─────────────────────────────────────────┘
Scanline 240 (NTSC) ────────────────────────────────────────────
Scanline 288 (PAL)
                 ╔═════════════════════════════════════════╗
                 ║                                         ║
                 ║         VBLANK REGION                   ║
                 ║    (Vertical Blanking Interval)         ║
                 ║                                         ║
                 ║  NTSC: Scanlines 240-261 (22 lines)     ║
                 ║  PAL:  Scanlines 288-311 (24 lines)     ║
                 ║                                         ║
                 ║  Electron beam returns to top           ║
                 ║  No visible output during this period   ║
                 ║                                         ║
                 ╚═════════════════════════════════════════╝
Scanline 261 (NTSC) ────────────────────────────────────────────
Scanline 311 (PAL)
                 ↻ Beam returns to scanline 0, next frame

Beam Position Registers (0xA4=X, 0xA5=Y):
• Track electron beam position across entire frame
• X (0xA4): 0-226 (0-159 visible, 160-226 HBLANK)
• Y (0xA5): 0-261 (NTSC) or 0-311 (PAL)
  - NTSC: 0-239 visible, 240-261 VBLANK
  - PAL:  0-287 visible, 288-311 VBLANK

Timing Details:
- Cycles per scanline: 227 VDC cycles (~63.4 microseconds)
- HBLANK: ~12 microseconds per scanline (X: 160-226)
- Frame rate: 60 Hz (NTSC) / 50 Hz (PAL)
```

### Coordinate System Details

#### Framebuffer Coordinates
- **Width**: 160 pixels (X: 0-159)
- **Height**: 200 pixels (Y: 0-199)
- **Origin**: Top-left corner (0, 0)
- **Pixel format**: 8-bit palette index (0-7 for colors)

#### Character/Sprite Coordinates
- **X position**: 0-159 (visible), 160-255 (off-screen right)
- **Y position**: 
  - 0-186: Fully visible (character + 14 pixels fits in framebuffer)
  - 187-199: Partially visible (character extends beyond Y=199, clipped)
  - 200-255: Completely off-screen below
- **Character height**: 14 pixels (7 rows × 2 scanlines per row)
- **Character width**: 8 pixels

#### Off-Screen Positioning
Games often use specific Y-values to hide objects:
- **Y = 0xF8 (248)**: Common "hidden" position (48 pixels below framebuffer)
- **Y = 0xC7 (199)**: Barely visible (only top pixel shows, rest clipped)
- **Y = 0xC8 (200)**: Just off-screen (first completely invisible position)
- **Y = 0xFF (255)**: Maximum Y-value (55 pixels below framebuffer)

**Why Y=199 is (mostly) off-screen:**
- Framebuffer height: 200 pixels (Y = 0-199)
- Character height: 14 pixels
- A character at Y=199 renders pixels Y=199 to Y=212
- Only pixel Y=199 is in the framebuffer (the top pixel of the character)
- Pixels Y=200-212 are clipped (below the framebuffer)
- Therefore, a character at Y=199 shows only **1 pixel** (effectively invisible)
- Y=200 or higher is **completely off-screen**

#### Beam Position (Registers 0xA4, 0xA5)
- **0xA4 (BEAM_X)**: Horizontal position (0-226)
  - 0-159: Visible area
  - 160-226: HBLANK (horizontal blanking)
- **0xA5 (BEAM_Y)**: Vertical position (0-261 NTSC, 0-311 PAL)
  - 0-239: Visible area (NTSC)
  - 240-261: VBLANK (vertical blanking, NTSC)

#### VBLANK Timing
VBLANK (Vertical Blanking) occurs at the **bottom** of the screen after all visible scanlines have been drawn. This is when the electron beam returns from the bottom-right to the top-left to start the next frame.

- **NTSC**: Starts at scanline 240 (after Y=239)
  - VDC cycle: 240 × 227 = 54,480 cycles into frame
  - CPU cycle: 54,480 ÷ 9.9 ≈ 5,503 cycles into frame
  - Duration: 22 scanlines (240-261)
- **PAL**: Starts at scanline 288 (after Y=287)
  - VDC cycle: 288 × 227 = 65,376 cycles into frame
  - CPU cycle: 65,376 ÷ 9.9 ≈ 6,603 cycles into frame
  - Duration: 24 scanlines (288-311)

**Note**: Games typically write to VDC registers during VBLANK to avoid visual artifacts (tearing, flickering) that can occur when updating registers during active display.

### Common Debugging Scenarios

#### Finding Off-Screen Objects
If characters or sprites aren't visible, check their Y-positions:
```cpp
// Character 0 Y-position is at register 0x10
uint8 char_y = vdc.registers[0x10];
if (char_y > 186) {
    // Character is partially or completely off-screen
    // Y=187-199: partially visible (clipped)
    // Y=200+: completely off-screen
}
if (char_y == 0xF8 || char_y == 0xFF || char_y == 0xC7) {
    // Common "hidden" positions
}
```

#### Checking Quad Character Positions
Quad characters (0x40-0x7F) have a different structure:
```cpp
// Quad 0 base X position is at register 0x4D (byte 13 of quad)
uint8 quad_x = vdc.registers[0x4D];

// Each sub-character Y-position (4 characters per quad)
uint8 char0_y = vdc.registers[0x40];  // Sub-char 0
uint8 char1_y = vdc.registers[0x44];  // Sub-char 1
uint8 char2_y = vdc.registers[0x48];  // Sub-char 2
uint8 char3_y = vdc.registers[0x4C];  // Sub-char 3
```

## Enabling the Debugger

### Command Line
```bash
./videopac --bios <bios_file> <rom_file> --debug
```

### Programmatically
```cpp
Debugger debugger(&emulator);
emulator.set_debugger(&debugger);
```

## Debugger Features

### 1. Breakpoints

#### Unconditional Breakpoints
Break execution when PC reaches a specific address:

```cpp
debugger.add_breakpoint(0x16B);  // Break at BIOS clear_all_characters
```

#### Conditional Breakpoints
Break only when a condition is met:

```cpp
// Break at 0x171 when A register equals 0xF8
debugger.add_breakpoint(0x171, "cpu.A==0xF8");

// Break when Port 1 has VDC disabled
debugger.add_breakpoint(0x171, "cpu.P1&0x08");

// Break when VDC register 0xA0 has display enabled
debugger.add_breakpoint(0x100, "vdc.registers[0xA0]&0x20");
```

#### Condition Syntax
Conditions support:
- **Operators**: `==`, `!=`, `>`, `<`, `>=`, `<=`, `&` (bitwise AND), `|` (bitwise OR)
- **CPU fields**: `cpu.A`, `cpu.PSW`, `cpu.PC`, `cpu.SP`, `cpu.R0`-`cpu.R7`, `cpu.ram[index]`
- **VDC fields**: `vdc.registers[index]`, `vdc.scanline`, `vdc.display_enabled`, `vdc.grid_enabled`
- **Memory fields**: `memory.external_ram[index]`, `memory.current_bank`, `memory.rom_size_kb`
- **Values**: Hex (`0xFF`), decimal (`255`), boolean (`true`, `false`)

#### Managing Breakpoints
```cpp
// Remove breakpoint
debugger.remove_breakpoint(0x16B);

// Disable/enable breakpoint (keeps it in list)
debugger.enable_breakpoint(0x16B, false);
debugger.enable_breakpoint(0x16B, true);

// Clear all breakpoints
debugger.clear_all_breakpoints();

// List breakpoints
const auto& breakpoints = debugger.get_breakpoints();
for (const auto& bp : breakpoints) {
    std::cout << "0x" << std::hex << bp.address;
    if (bp.has_condition) {
        std::cout << " if " << bp.condition;
    }
    std::cout << (bp.enabled ? " [enabled]" : " [disabled]") << "\n";
}
```

### 2. Execution Control

#### Single Stepping
Execute one CPU instruction at a time:

```cpp
debugger.step();
```

#### Continue Execution
Resume normal execution after a breakpoint or pause:

```cpp
debugger.continue_execution();
```

#### Pause
Pause execution:

```cpp
debugger.pause();
```

#### Check State
```cpp
if (debugger.is_paused()) {
    // Debugger is paused
}

DebuggerState state = debugger.get_state();
// States: Running, Paused, StepMode, TraceMode
```

### 3. State Inspection

#### CPU State
```cpp
std::string cpu_dump = debugger.dump_cpu_state();
```

Output:
```
CPU State:
  PC:  0x171
  A:   0xf8
  PSW: 0x90 [C=1 AC=0 F0=0 BS=1]
  Registers (Bank 1):
    R0: 0x10 R1: 0x20 R2: 0x30 R3: 0x00
    R4: 0x00 R5: 0x00 R6: 0x00 R7: 0xff
  Stack Pointer: 2
  Interrupts: Enabled
  Timer: 0x00
```

#### Memory Dump
```cpp
// Dump internal RAM addresses 0x00-0x3F
std::string mem_dump = debugger.dump_memory(0x00, 0x3F);
```

Output:
```
Internal RAM (0x00-0x3F):
  0x00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x10: f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8
  ...
```

#### VDC Registers
```cpp
std::string vdc_dump = debugger.dump_vdc_registers();
```

Output:
```
VDC Registers:
  Control (0xA0): 0x20
  Status  (0xA1): 0x02
  Collision (0xA2): 0x00
  Color   (0xA3): 0x00
  Sprites:
    Sprite 0: X=0 Y=0 Color=0x00 Pattern=0x00
    ...
  Beam Position: (123, 5) (VBLANK: No)
```

#### Disassembly
```cpp
// Disassemble 5 lines before and after PC
std::string disasm = debugger.disassemble_at_pc(5, 5);
```

Output:
```
Disassembly around PC (0x171):
    0x16b: 23 f8  MOV A,##0xf8
    0x16d: b8 10  MOV R0,##0x10
    0x16f: ba 30  MOV R2,##0x30
 -> 0x171: 90     MOVX @R0,A
    0x172: 18     INC R0
    0x173: ea 71  DJNZ R2,0x171
```

### 4. Trace Logging

Trace logging records every CPU instruction execution with full system state.

#### Enable Trace
```cpp
debugger.enable_trace(true);
```

#### Get Trace Log
```cpp
const auto& trace = debugger.get_trace_log();
for (const auto& line : trace) {
    std::cout << line << "\n";
}
```

#### Clear Trace
```cpp
debugger.clear_trace_log();
```

#### Trace Format
See [trace_format.md](trace_format.md) for detailed documentation of the trace log format.

Example trace line:
```
[F:5 C:297419] 0x171: 90     MOVX @R0,A | A=0xf8 PSW=0x90 P1=0x1c P2=0x00 [VDC:OFF RAM:OFF] RB0 VDC[beam:123,5 ctrl:0x00 stat:0x02 disp:OFF vbl:N] [WRITE @R0=0x10 val=0xf8 -> NOWHERE!]
```

### 5. Frame Statistics

Track frame timing and cycle counts:

```cpp
FrameStats stats = debugger.get_frame_stats();
std::cout << "Frame: " << stats.frame_count << "\n";
std::cout << "Total cycles: " << stats.total_cycles << "\n";
std::cout << "Avg cycles/frame: " << stats.average_cycles_per_frame << "\n";
std::cout << "FPS: " << stats.fps << "\n";

// Reset statistics
debugger.reset_frame_stats();
```

## Common Debugging Workflows

### Finding Why Characters Don't Clear

1. **Set breakpoint at clear routine**:
   ```cpp
   debugger.add_breakpoint(0x16B);  // BIOS clear_all_characters
   ```

2. **Enable trace logging**:
   ```cpp
   debugger.enable_trace(true);
   ```

3. **Run until breakpoint**:
   ```cpp
   emulator.run_frame();
   ```

4. **Inspect Port 1 state**:
   ```cpp
   std::string cpu = debugger.dump_cpu_state();
   // Check Port 1 value - should have VDC enabled (P13=0)
   ```

5. **Step through MOVX instructions**:
   ```cpp
   debugger.step();  // Execute one instruction
   // Check trace log to see where writes go
   ```

6. **Analyze trace**:
   ```cpp
   const auto& trace = debugger.get_trace_log();
   // Look for "[WRITE ... -> NOWHERE!]" indicating disabled VDC/RAM
   ```

### Tracking VDC Register Changes

1. **Set conditional breakpoint on VDC writes**:
   ```cpp
   // Break when writing to VDC control register (0xA0)
   debugger.add_breakpoint(0x100, "vdc.registers[0xA0]!=0x00");
   ```

2. **Monitor VDC state in trace**:
   - Enable trace logging
   - Look for `VDC[...]` state in each line
   - Track `ctrl:0x__` value changes

### Finding Timing Issues

1. **Enable trace with frame stats**:
   ```cpp
   debugger.enable_trace(true);
   ```

2. **Run specific number of frames**:
   ```cpp
   for (int i = 0; i < 10; i++) {
       emulator.run_frame();
   }
   ```

3. **Analyze cycle counts**:
   ```cpp
   FrameStats stats = debugger.get_frame_stats();
   // Check if cycles per frame match expected values
   // NTSC: ~59,474 cycles/frame (262 scanlines * 227 cycles)
   // PAL: ~70,824 cycles/frame (312 scanlines * 227 cycles)
   ```

### Debugging Memory Access

1. **Set breakpoint on specific memory address**:
   ```cpp
   // Break when R0 points to character register 0x10
   debugger.add_breakpoint(0x171, "cpu.R0==0x10");
   ```

2. **Check Port 1 configuration**:
   ```cpp
   // Break when Port 1 disables VDC
   debugger.add_breakpoint(0x100, "cpu.P1&0x08");
   ```

3. **Trace memory operations**:
   - Look for `[WRITE ...]` and `[READ ...]` in trace
   - Check destination: `-> VDC`, `-> RAM`, `-> VDC+RAM`, or `-> NOWHERE!`

## Performance Considerations

### Trace Logging
- Generates ~60,000-120,000 lines per second
- Can consume significant memory
- Use `clear_trace_log()` periodically
- Consider filtering to specific frame ranges

### Breakpoints
- Conditional breakpoints have overhead (condition evaluation)
- Use unconditional breakpoints when possible
- Disable unused breakpoints instead of removing them

### Headless Mode
For automated debugging, use headless mode:
```bash
./videopac --bios <bios> <rom> --headless --frames 10 --debug
```

## Tips and Tricks

1. **Use conditional breakpoints to narrow down issues**:
   - Instead of breaking at every iteration, break only when a specific condition occurs

2. **Combine breakpoints with trace logging**:
   - Set breakpoint near problem area
   - Enable trace when breakpoint hits
   - Analyze trace for detailed execution flow

3. **Check Port 1 state for I/O issues**:
   - Port 1 controls VDC/RAM access
   - Look for `[VDC:OFF RAM:OFF]` in trace (indicates bug)

4. **Monitor VDC beam position**:
   - Writes during visible area can cause artifacts
   - Prefer writes during VBLANK (Y >= 240 for NTSC)

5. **Use frame statistics to verify timing**:
   - Incorrect cycle counts indicate timing bugs
   - Compare against expected values for NTSC/PAL

## Call Tree Analysis

The `build_call_tree.py` tool analyzes trace logs to generate hierarchical call trees showing function calls, returns, jumps, and loops for each frame.

### Usage

```bash
python3 build_call_tree.py <frame_numbers...>
```

Example:
```bash
# Analyze frames 0-10
python3 build_call_tree.py 0 1 2 3 4 5 6 7 8 9 10 > FRAME_CALL_TREES.md
```

### Features

- **Call/Return Tracking**: Shows CALL and RET instructions with proper indentation
- **Loop Detection**: Identifies loops using backward jumps (DJNZ, JNZ, JZ, JNC, JC, JF0, JF1, JT0, JNT0, JT1, JNT1, JTF, JNI, JB0-JB7)
- **Cycle Counting**: Reports cycle ranges and iteration counts for loops
- **Label Resolution**: Uses BIOS and ROM labels for readable output
- **Frame Boundary Handling**: Properly splits loops that span frame boundaries
- **Indentation Tracking**: Maintains call stack depth across frames

### Supported Jump Instructions

The tool detects loops using all 8048 conditional and unconditional jump instructions:

- **Unconditional**: JMP
- **Carry tests**: JC, JNC
- **Zero tests**: JZ, JNZ
- **Flag tests**: JF0, JF1
- **Test pin tests**: JT0, JNT0, JT1, JNT1
- **Timer flag**: JTF
- **Interrupt**: JNI
- **Bit tests**: JB0, JB1, JB2, JB3, JB4, JB5, JB6, JB7
- **Decrement and jump**: DJNZ

### Output Format

```
=== Frame N Call Tree ===

[  cycle] address → CALL target_label
  [  cycle] target_label → CALL nested_label
  [  cycle] nested_label ← RET
[  cycle] address ← RET
[  start..   end] LOOP from_addr → to_addr: N iterations (M cycles)
```

Example:
```
[ 15604] 0x520 → CALL rom:wait_until_scanline
  [ 15614] rom:wait_until_scanline → CALL bios:set_up_vdc_access
  [ 15673] 0x0eb ← RET
  [ 15841.. 49343] LOOP 0x723 → 0x71a: 243 iterations (33502 cycles)
[ 49353] 0x725 ← RET
```

### Label Sources

The tool loads labels from:
1. **BIOS**: `doc/french_bios_annotated.txt`
   - Function labels (e.g., `wait_for_interrupt`, `character_write`)
   - Labels extracted from CALL/JMP operands
2. **ROM**: `doc/satellite-attack-disassembly.txt`
   - Function labels (e.g., `wait_until_scanline`, `main_loop2`)
   - Handles address wrapping after 0x7FF

### ROM Address Translation

The ROM disassembly has addresses that wrap after 0x7FF:
- Addresses 0x400-0x7FF map directly to memory 0x400-0x7FF
- After wrap, addresses 0x000-0x3FF map to memory 0xC00-0xFFF

The tool automatically detects and handles this wrapping.

### Loop Detection

Loops are detected by identifying backward jumps where:
- Target address < current address
- Distance < 0x30 bytes (prevents false positives from long jumps)

Loop regions show:
- Start and end cycles (relative to frame start)
- Source and target addresses
- Number of iterations (back-edge count)
- Total cycles consumed

### Frame Boundary Handling

When a loop spans multiple frames:
1. The loop is split into separate regions per frame
2. Indentation depth is preserved across frame boundaries
3. Loops continuing from previous frame start at cycle 0

### Common Use Cases

1. **Performance Analysis**: Identify hot loops and cycle consumption
2. **Control Flow Understanding**: See call hierarchy and execution paths
3. **Timing Verification**: Check cycle counts match expectations
4. **Bug Investigation**: Track execution flow through problematic code

### Example Analysis

To investigate why `wait_until_scanline` takes so long:

```bash
python3 build_call_tree.py 5 7 | grep -A 5 wait_until_scanline
```

Output shows:
```
[ 15604] 0x520 → CALL rom:wait_until_scanline
  [ 15614] rom:wait_until_scanline → CALL bios:set_up_vdc_access
  [ 15673] 0x0eb ← RET
  [ 15841.. 49343] LOOP 0x723 → 0x71a: 243 iterations (33502 cycles)
[ 49353] 0x725 ← RET
```

This reveals the function contains a JNC loop at 0x723 that runs 243 times, consuming 33,502 cycles waiting for the scanline position.

## See Also

- [trace_format.md](trace_format.md) - Detailed trace log format documentation
- [port1_bits.md](port1_bits.md) - Port 1 bit definitions
- [o2doc.md](o2doc.md) - Videopac hardware documentation
- [FRAME_CALL_TREES.md](../FRAME_CALL_TREES.md) - Generated call tree analysis
