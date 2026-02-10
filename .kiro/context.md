# Project Context and Guidelines

This document contains general instructions and guidelines for working on the Videopac emulator project.

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

### Command-Line Options

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

### Debugger Features

The emulator includes a built-in debugger with:
- **Breakpoints**: Set breakpoints at specific addresses
- **Single-step execution**: Step through instructions one at a time
- **Instruction trace**: Log every instruction executed (requires `--debug` flag)
- **Memory inspection**: View CPU registers, RAM, and VDC state
- **Disassembly**: View disassembled code around current PC

**Important**: To generate trace logs, you MUST use the `--debug` flag. The trace will be written to `trace.log`.

### Trace Log

When running with `--debug`, the emulator generates a trace log showing:
- Program counter (PC)
- Instruction opcode and mnemonic
- Accumulator (A) and PSW register values

Example trace output:
```
0x0a8: 80     MOVX A,@R0 | A=0x43 PSW=0x90
0x0a9: 91     MOVX @R1,A | A=0xf0 PSW=0x90
```

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
