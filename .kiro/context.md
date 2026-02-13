# Project Context and Guidelines

This document contains general instructions and guidelines for working on the Videopac emulator project.

## Build Instructions

To build the project, use CMake:

```bash
cmake --build build
```

This will compile all targets (emulator, tests, tools).

## Running the Emulator

To run the emulator with trace logging enabled:

```bash
./run_emulator_hl.sh "roms/Satellite Attack (1981)(Philips)(EU).bin"
```

Or with the French BIOS:

```bash
./run_emulator_hl.sh "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" "roms/Satellite Attack (1981)(Philips)(EU).bin"
```

The trace will be written to `trace.log`.

## Git Commit Workflow

When making commits, always follow this process:

1. **Test First**: Run the test suite to ensure all tests pass
   ```bash
   ./build/videopac_tests
   ```
   **CRITICAL**: All tests MUST pass before committing. Never commit with failing tests.

2. **Stage Relevant Files**: Add only the files related to the current change
   ```bash
   git add <specific-files>
   ```
   - Do NOT use `git add .` or `git add -A` without reviewing
   - Stage files that are logically related to the same change

3. **Review Changes**: Check what will be committed
   ```bash
   git diff --staged
   ```
   - Review every line to ensure no unintended changes
   - Verify no debug code, commented code, or temporary changes are included

4. **Draft Commit Message**: Follow conventional commit format
   ```
   <type>: <short summary>
   
   <detailed description if needed>
   
   <footer with references if applicable>
   ```

5. **Commit Types**:
   - `feat:` - New feature
   - `fix:` - Bug fix
   - `docs:` - Documentation changes
   - `style:` - Code style changes (formatting, no logic change)
   - `refactor:` - Code refactoring
   - `test:` - Adding or updating tests
   - `chore:` - Build process, dependencies, tooling
   - `perf:` - Performance improvements

6. **Commit Message Guidelines**:
   - Use imperative mood ("Add feature" not "Added feature")
   - Keep first line under 72 characters
   - Separate subject from body with blank line
   - Explain what and why, not how
   - Reference issue/task numbers when applicable

## Example Commit Messages

Good:
```
feat: implement Intel 8048 CPU core structure

Add CPU state structure with all registers, internal RAM, stack,
and I/O ports. Implement reset() and basic interface methods.

Implements task 3.1 from implementation plan.
```

```
test: add unit tests for memory bank switching

Verify correct behavior for 2KB, 4KB, and 8KB ROM cartridges
with proper bank selection via Port 1 pins.
```

```
chore: set up CMake build system with testing frameworks

Configure CMake to build core library, standalone executable,
and libretro core. Integrate Google Test and RapidCheck for
unit and property-based testing.

Completes task 1 from implementation plan.
```

Bad:
```
fixed stuff
```

```
WIP
```

```
Updated files
```

## Code Review Checklist

Before committing, verify:
- [ ] **All tests pass** (MANDATORY - never commit with failing tests)
- [ ] No compiler warnings
- [ ] Code follows C++17 standards
- [ ] No unused variables or parameters (use `(void)var` for intentional unused)
- [ ] No debug print statements
- [ ] No commented-out code
- [ ] Proper error handling
- [ ] Documentation updated if needed

## Build and Test Commands

```bash
# Configure build (use cmake3 on Amazon Linux 2)
cmake3 -B build -DCMAKE_BUILD_TYPE=Release

# Build all targets
make -C build

# Build specific target
make -C build videopac
make -C build videopac_tests

# Run tests
build/videopac_tests

# Run standalone emulator
build/videopac --bios <bios_file> <rom_file>
```

## Debugging and Development Tools

**See [doc/debugging.md](../doc/debugging.md) for comprehensive debugging documentation.**

**See [doc/trace_format.md](../doc/trace_format.md) for detailed trace log format.**

### Quick Reference

#### Command-Line Options

```bash
build/videopac [options] <rom_file>

Options:
  --bios <file>       Load BIOS from file (required)
  --pal               Use PAL timing (default: NTSC)
  --headless          Run without display (for testing)
  --frames <n>        Run for N frames then exit (headless mode)
  --screenshot <n>    Save screenshot every N frames (headless mode)
  --debug             Enable debugger and instruction trace
  --help              Show help message
```

#### Trace Logging

**Important**: To generate trace logs, you MUST use the `--debug` flag. The trace will be written to `trace.log`.

Trace format (see [doc/trace_format.md](../doc/trace_format.md) for details):
```
[F:5 C:297419] 0x171: 90 MOVX @R0,A | A=0xf8 PSW=0x90 P1=0x1c P2=0x00 [VDC:OFF RAM:OFF] RB0 VDC[beam:123,5 ctrl:0x00 stat:0x02 disp:OFF vbl:N] [WRITE @R0=0x10 val=0xf8 -> NOWHERE!]
```

**Note**: Traces log once per CPU instruction (not per VDC cycle). VDC runs ~10x faster than CPU, so VDC state shown is a snapshot at the moment the CPU instruction completes.

### Useful Debugging Addresses (BIOS)

From `doc/o2romsrc.txt`:
- `0x00B0` - Keyboard Check Routine
- `0x013D` - Get Keystroke routine
- `0x00E7` - Set up VDC Access
- `0x00EC` - Set up RAM Access
- `0x0127` - Display On
- `0x038F` - Read Joystick

### Running on Remote Systems (NICE DCV)

When running on AWS EC2 with NICE DCV:

```bash
# Set environment variables
export DISPLAY=:0
export SDL_RENDER_DRIVER=software

# Run emulator
build/videopac --bios <bios_file> <rom_file>

# Or use the helper script
./run_emulator.sh <rom_file>
```

**Note**: Audio may not work without ALSA, but the emulator continues without audio.

### Input System

The emulator supports:
- **Keyboard**: Full Videopac keyboard matrix (0-9, A-Z, special keys)
- **Joystick**: Not yet implemented (TODO: Task 14.5)

Keyboard input flow:
1. SDL frontend receives key events
2. Maps SDL keys to VidKey enum values
3. Sets key state in InputHandler keyboard matrix
4. CPU reads keyboard via Port 2 when BIOS calls keyboard routine
5. BIOS returns key code to game

### Keyboard Matrix Layout

From `include/input.h`:
- Row 0: Keys 0-7
- Row 1: Keys 8, 9, -, +, *, /, =, Yes
- Row 2: Q, W, E, R, T, Y, U, I
- Row 3: A, S, D, F, G, H, J, K
- Row 4: Z, X, C, V, B, N, M, .
- Row 5: Space, ?, L, P, O, Clear, Enter, No

### Common Issues

**Keyboard not working?**
- The F1 flag (PSW bit 4) must be set by `CPU::trigger_interrupt()` when VBlank interrupt occurs
- BIOS "Wait for Interrupt" routine (0x176) clears F1, enables interrupts, then loops checking F1
- When interrupt fires, F1 is set and the wait loop exits, allowing keyboard routine to execute
- Check that VBlank interrupt is being triggered by emulator core
- Verify BIOS is calling keyboard routine (0x00B0 or 0x013D)
- Use `--debug` and check trace.log for Port 2 reads
- Add debug output to `InputHandler::read_keyboard()` to see if keys are detected

**Display issues?**
- Verify VDC is enabled (P13 of Port 1)
- Check that display is turned on (BIOS routine at 0x0127)
- Use `--screenshot` to save framebuffer and inspect visually

**Build fails?**
- Use `cmake3` instead of `cmake` on Amazon Linux 2
- Ensure SDL2 is installed: `sudo yum install SDL2-devel`
- Check compiler supports C++17

## Recent Fixes

### Keyboard Input Fix (2026-02-10)

**Problem**: Keyboard input was detected by SDL but had no effect on the emulator.

**Root Cause**: The BIOS was stuck in an infinite loop at 0x176-0x17A waiting for the F1 flag to be set by the interrupt handler. The `CPU::trigger_interrupt()` method was not setting the F1 flag (PSW bit 4) when interrupts occurred.

**Solution**: Modified `CPU::trigger_interrupt()` to set F1 flag (PSW bit 4) when interrupt is triggered. This allows the BIOS "Wait for Interrupt" routine to exit and proceed to the keyboard check routine.

**Files Modified**:
- `src/cpu.cpp`: Added `state_.psw |= 0x10;` to set F1 flag in `trigger_interrupt()`
- `src/input.cpp`: Removed debug printf statements
- `src/frontend_sdl.cpp`: Removed debug printf statements (if any)


## BIOS Routine Analysis

### Select Game Routine (0x2C3)

The `select_game` routine is called by cartridges to display a game selection menu. Understanding this flow is critical for debugging keyboard and display issues.

**Flow**:
1. **Initialize** (0x2C3-0x2D4):
   - Sets R7 = 0xFF (key state)
   - Calls `reset` (0x0F1) to clear memory and VDC
   - Sets up text display parameters:
     - R1 = 0xF2 (pointer to "QUEL JEU?" text at 0x2F2)
     - R0 = 0x10 (VDC character register start)
     - R2 = 0x0B (11 characters to display)
     - R3 = 0x28 (X position)
     - R4 = 0x70 (Y position)
     - R6 = 0x06 (color - note: French BIOS uses 0x06, US uses 0x04)

2. **Display Text** (0x2D6-0x2DE):
   - Turns off display
   - Loops through characters, calling `character_write` (0x3EA) for each
   - Increments color for each character (creates rainbow effect)
   - Turns display back on

3. **Play Tune** (0x2E2-0x2E4):
   - Calls `start_tune` (0x1A2) with tune data at 0x34A

4. **Wait for Keystroke** (0x2E6):
   - Calls `get_keystroke` (0x13D)
   - **This blocks until a key is pressed**
   - `get_keystroke` internally:
     - Calls `wait_for_interrupt` (0x176) - waits for VBlank
     - Calls `keyboard_routine` (0x0B0) - scans keyboard
     - Loops until a key is detected (bit 7 of R7 clear)

5. **Cleanup and Return** (0x2E8-0x2F0):
   - Saves keystroke in R1
   - Turns off display
   - Clears all characters
   - Turns display back on
   - Moves keystroke to A
   - Jumps to `end_of_select_game` (0x408) - cartridge vector

**Key Points**:
- `select_game` is called **once** per game selection
- It **blocks** in `get_keystroke` until user presses a key
- The routine does NOT loop - cartridge must call it again for multiple selections
- Keystroke value (0-9) is returned in accumulator A

**Debugging select_game**:

```bash
# Set breakpoint at select_game entry
(gdb) break CPU::execute_instruction if state_.pc == 0x2c3

# Set breakpoint at get_keystroke
(gdb) break CPU::execute_instruction if state_.pc == 0x13d

# Set breakpoint at keyboard_routine
(gdb) break CPU::execute_instruction if state_.pc == 0x0b0

# Watch for keystroke return (when A != 0xFF after get_keystroke)
(gdb) watch state_.a if state_.pc == 0x2e8

# Enable trace logging
./build/videopac --debug --bios <bios> <rom>
# Then check trace.log for execution flow
```

**Common Issues**:
- **Text not displayed**: Check VDC is enabled, display is on, characters written to VDC registers 0x10-0x1A
- **Stuck in get_keystroke**: Verify VBlank interrupts are firing, F1 flag is being set, keyboard matrix is populated
- **Wrong key detected**: Check keyboard matrix mapping in `InputHandler::read_keyboard()`

### Get Keystroke Routine (0x13D)

Blocks until a valid key is pressed.

**Flow**:
1. Calls `wait_for_interrupt` (0x176) - waits for VBlank
2. Calls `keyboard_routine` (0x0B0) - scans keyboard matrix
3. Checks bit 7 of R7 (key valid flag)
4. If bit 7 set (no key), loops back to step 1
5. If bit 7 clear (key pressed), plays beep and returns key code in A

**Important**: This routine **blocks** until a key is pressed. The emulator must:
- Generate VBlank interrupts regularly
- Set F1 flag in interrupt handler
- Populate keyboard matrix from SDL input events

### Keyboard Routine (0x0B0)

Scans the keyboard matrix and returns key code.

**Flow**:
1. Disables interrupts
2. Saves Port 1 state in R6
3. Configures Port 1 for keyboard scanning
4. Scans 6 rows (0xF0, 0xE0, 0xD0, 0xC0, 0xB0, 0xA0)
5. For each row:
   - Writes row value to Port 2
   - Reads Port 2 to get column data
   - If bit 4 clear, key is pressed
6. Debounces key (reads multiple times)
7. Converts row/column to key code
8. Stores in R7 with bit 7 clear if valid key
9. Returns with A = R7 | 0xC0

**Debugging keyboard**:
- Set breakpoint at 0x0B0 to see when keyboard is scanned
- Watch Port 2 writes/reads to see row scanning
- Check R7 value after routine - bit 7 clear means key detected
- Verify `InputHandler::read_keyboard()` returns correct values for each row

### Disassembly Tools

The project includes tools for analyzing BIOS and ROM code:

**Generate disassembly with labels**:
```bash
./build/disasm_tool "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" > french_bios_disasm.txt
```

**Generate annotated disassembly** (with comments from US BIOS):
```bash
python3 tools/annotate_disasm.py french_bios_disasm.txt doc/o2romsrc.txt > french_bios_annotated.txt
```

**Compare French vs US BIOS**:
```bash
python3 compare_disasm.py
```

The disassembler automatically:
- Uses meaningful labels for known BIOS routines (e.g., `select_game`, `get_keystroke`)
- Generates `loc_XXXX` labels for other jump targets
- Replaces hex addresses with label names in JMP/CALL operands
- Handles both BIOS (0x000-0x3FF) and cartridge (0x400+) code

**Key files**:
- `french_bios_annotated.txt` - Fully annotated French BIOS disassembly
- `doc/o2romsrc.txt` - US BIOS disassembly with detailed comments
- `tools/disasm_tool.cpp` - Disassembler with label generation
- `tools/annotate_disasm.py` - Merges disassembly with comments


## Recent Fixes and Debugging Progress

### SEL MB0/MB1 Bug Fix (2026-02-11)

**Problem**: The `SEL MB1` (0xF5) and `SEL MB0` (0xE5) instructions were incorrectly modifying the Program Counter immediately, causing PC to jump to invalid addresses. When `SEL MB1` was executed at 0x414, it changed PC from 0x415 to 0xC15 (0x415 | 0x800), causing execution beyond ROM bounds.

**Root Cause**: The implementation was setting/clearing bit 11 of PC directly (`state_.pc |= 0x800` or `state_.pc &= 0x7FF`), but according to Intel 8048 specification, these instructions should set a Data Bank Flag (DBF) that affects **future** JMP/CALL instructions, not the current PC.

**Solution**:
1. Added `memory_bank` flag to `CPUState` to track the DBF
2. Modified `SEL MB0` to set `state_.memory_bank = false`
3. Modified `SEL MB1` to set `state_.memory_bank = true`
4. Updated all `JMP` instructions (0x04, 0x24, 0x44, 0x64, 0x84, 0xA4, 0xC4, 0xE4) to apply the memory bank flag:
   ```cpp
   uint16 addr = ((opcode & 0xE0) << 3) | addr_low;
   if (state_.memory_bank) {
       addr |= 0x800;  // Set bit 11 for MB1
   }
   state_.pc = addr;
   ```
5. Updated all `CALL` instructions (0x14, 0x34, 0x54, 0x74, 0x94, 0xB4, 0xD4, 0xF4) similarly

**Files Modified**:
- `include/cpu.h`: Added `memory_bank` flag to CPUState
- `src/cpu.cpp`: Fixed SEL MB0/MB1, JMP, and CALL instructions
- `src/memory.cpp`: Added PC bounds checking with detailed error reporting
- `src/frontend_sdl.cpp`: Added exception handling to save trace before exit

**Testing**: Satellite Attack ROM now runs past the select_game screen without crashing.

### Memory Bounds Checking (2026-02-11)

Added comprehensive bounds checking in `MemorySystem::read_program()` to detect when PC goes beyond valid ROM space:

```cpp
if (rom_offset >= state_.cart_rom.size()) {
    std::cerr << "\n*** FATAL ERROR: PC out of bounds ***" << std::endl;
    std::cerr << "PC = 0x" << std::hex << address << std::dec << std::endl;
    std::cerr << "ROM size: " << state_.rom_size_kb << "KB (" << state_.num_banks << " banks)" << std::endl;
    std::cerr << "Current bank: " << static_cast<int>(state_.current_bank) << std::endl;
    // ... more diagnostic info ...
    throw std::runtime_error("PC out of bounds - halting emulator");
}
```

This helps catch emulation bugs early by halting execution with detailed diagnostics when PC goes to unmapped memory.

### Current Status: Satellite Attack Debugging

**What Works**:
- ✅ BIOS loads and executes correctly
- ✅ Select game screen displays "QUEL JEU?" text
- ✅ Keyboard input works (can press keys 0-9)
- ✅ Game starts after pressing '1'
- ✅ No crashes or PC out of bounds errors
- ✅ Trace logging works (2.5M+ instructions captured)
- ✅ VDC register dumps working
- ✅ Conditional breakpoints implemented

**Current Issue**:
- ❌ Screen goes all white after pressing '1' and game starts
- The game reads VDC Control register (0xA0), ORs with 0x28, gets 0xFF
- This 0xFF value propagates through the system, setting all VDC registers to 0xFF
- Background color becomes white (palette index 7)

**Recent Fixes (2026-02-11)**:
1. **Copy Mode Implementation**: Added copy mode support in `read_external()` to allow reading ROM data via MOVX
2. **Error Handling**: Changed `read_memory()` and copy mode bounds checks to throw exceptions instead of returning 0xFF
3. **VDC Debug Logging**: Added `dump_registers()` method to VDC for runtime inspection
4. **Conditional Breakpoints**: Implemented comprehensive conditional breakpoint system supporting:
   - CPU state: `cpu.A == 0xFF`, `cpu.R0 > 0x80`, `cpu.PSW & 0x10`
   - VDC state: `vdc.registers[0xA0] & 0x20`, `vdc.display_enabled == true`
   - Memory state: `memory.external_ram[0x20] == 0xFF`, `memory.current_bank == 1`
   - Array indexing with hex values: `vdc.registers[0xA0]`, `cpu.ram[0x10]`

**Next Debugging Steps**:
1. Use conditional breakpoints to track when A becomes 0xFF:
   ```bash
   # Break at ORL instruction when result is 0xFF
   --break 0x09F --condition "cpu.A == 0xFF"
   ```
2. Check if VDC Control register is being written with wrong value before the read
3. Verify copy mode is working correctly for ROM data reads
4. Check if there's an issue with how VDC registers are initialized

**Useful Debugging Commands**:
```bash
# Run with debugger and trace
./run_emulator.sh "roms/Satellite Attack (1981)(Philips)(EU).bin"

# Run with conditional breakpoint (breaks at 0x09F when A == 0xFF)
./build/videopac --bios <bios> --break 0x09F --condition "cpu.A == 0xFF" <rom>

# Multiple breakpoints with different conditions
./build/videopac --bios <bios> \
  --break 0x09F --condition "cpu.A == 0xFF" \
  --break 0x0A8 --condition "memory.external_ram[0x20] > 0x80" \
  <rom>

# Break when VDC display is enabled
./build/videopac --bios <bios> --break 0x127 --condition "vdc.display_enabled == true" <rom>

# Check trace around game start (after key press)
grep -A 50 "0x500:" trace.log | head -60

# Check for VDC writes (register 0xA0 is control register)
grep "MOVX @R" trace.log | grep -E "0x[a-f][0-9a-f]"

# Find when A becomes 0xFF after ORL
grep -B2 "0x09f: 43 28.*ORL.*A=0xff" trace.log

# Take screenshots in headless mode
./build/videopac --bios <bios> <rom> --headless --frames 500 --screenshot 100
```

**Conditional Breakpoint Syntax**:
- CPU: `cpu.A`, `cpu.R0`, `cpu.PSW`, `cpu.PC`, `cpu.SP`, `cpu.ram[index]`
- VDC: `vdc.registers[index]`, `vdc.scanline`, `vdc.display_enabled`, `vdc.grid_enabled`
- Memory: `memory.external_ram[index]`, `memory.current_bank`, `memory.rom_size_kb`
- Operators: `==`, `!=`, `>`, `<`, `>=`, `<=`, `&` (bitwise), `|` (bitwise)
- Values: Decimal (`255`) or hex (`0xFF`), boolean (`true`/`false`)

**Known ROM Addresses**:
- `0x400`: Cartridge entry point (JMP to select_game)
- `0x408`: Game start (JMP to 0x500)
- `0x500`: main_loop2 - main game loop
- `0x411`: main_loop1 - inner game loop
- `0x09E-0x0A1`: VDC Control register read/modify/write sequence



## Known Issues

### Satellite Attack Rendering Bugs

**FIXED (2026-02-13)**:
1. ✅ **UFO sprite too small** - Fixed by correcting framebuffer height to 240 lines and implementing 2x horizontal scaling (320x240 output)
2. ✅ **Collision with UFO not working** - Fixed by implementing per-scanline collision detection that sets collision bits for both objects
3. ✅ **Status bar wrong/out of screen** - Fixed by extending framebuffer to 240 lines (status bar is at Y=199-207) and correcting quad character spacing from 8 pixels to 16 pixels (8 pixels character + 8 pixels space between each, per doc/o2doc.md section 4.5)

**Remaining Issues**:
4. **Enemy saucer looks incorrect** - The enemy saucer that appears periodically doesn't render correctly. This might be a sprite rendering issue (pattern data, double-size) or a character rendering issue depending on how the game implements it.

**Investigation needed**:
- Examine enemy saucer sprite/character configuration
- Check if it uses sprites or characters
- Verify pattern data and rendering attributes

**Related code**:
- `src/vdc.cpp`: `render_sprites()`, `render_characters()`
- `include/vdc.h`: Sprite and character register definitions
