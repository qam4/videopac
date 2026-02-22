# Course de Voitures Timer Display Investigation

## Problem Statement

In Course de Voitures Game 1, the countdown timer displays "02:00" at the start of gameplay but becomes corrupted at approximately frame 56 (~1 second into gameplay), showing random/garbled characters instead of counting down to "01:59".

## Investigation Status

**Current Phase**: Understanding timer display mechanism and identifying corruption cause

**Key Questions to Answer**:
1. How is "02:00" stored and displayed? (BCD format? Binary? Where in RAM?)
2. How is the timer converted to display format? (120 seconds → "02:00")
3. Where does the countdown logic decrement the timer?
4. What causes the corruption at frame 56?
5. Is this a game bug or an emulator bug?

## Tools Implemented

### VDC Trace Capability
- Added VDC register write tracing to `vdc_trace.log`
- Captures all writes to VDC registers with frame correlation
- Fixed implementation to work in cycle-accurate execution loop
- Files modified: `include/vdc.h`, `src/vdc.cpp`, `src/debugger.cpp`, `src/emulator.cpp`

### Command-Line Support
- Added `--vdc-trace` flag to enable VDC tracing
- Integrated with headless mode for automated testing
- Files modified: `include/frontend.h`, `src/main.cpp`, `src/frontend_headless.cpp`, `run_emulator.py`

## Findings So Far

### 1. Initial Timer Display (Frame ~7)

**VDC Trace Evidence**:
```
Write #183-190 (~frame 6.9):
  0x49 = 0xF8 (11111000)
  0x48 = 0xF8 (11111000)
  0x47 = 0xF8 (11111000)
  0x46 = 0xF8 (11111000)
  0x45 = 0xF8 (11111000)
  0x44 = 0xF8 (11111000)
  0x43 = 0xF8 (11111000)
  0x42 = 0xF8 (11111000)
```

**Observations**:
- 8 consecutive writes to VDC quad character registers (0x49→0x42, backwards)
- All values are 0xF8 (binary: 11111000)
- This is the initial "02:00" timer display
- Uses RAM-to-VDC copy mechanism during VBlank
- Data prepared in RAM[0x7D-0x7A], copied to VDC during VBLANK interrupt

### 2. Corruption at Frame 56

**VDC Trace Evidence**:
```
Frame 56 VDC writes:
  0x3F = 0x02 (quad sub-character position)
  0x40 = 0x01 (quad sub-character 0 Y-position)
  NO writes to 0x42-0x49 (quad character data)
```

**Observations**:
- Timer character data (0x42-0x49) is NOT updated at frame 56
- Other quad position registers (0x3F, 0x40) ARE written
- Timer display remains at initial values from frame 7
- Corruption visible in screenshots at frame 56

### 3. Timer Update Routine

**BIOS Routine**: `up_down_counter` at 0x1B0

**Instruction Trace Evidence**:
```
[F:54 C:3214483] 0x1b0: b8 3e | A=28 PSW=10 P1=b7 P2=f0 RB1 F1=1
[F:55 C:3274342] 0x1b0: b8 3e | A=28 PSW=90 P1=b7 P2=f0 RB1 F1=1
[F:56 C:3332887] 0x1b0: b8 3e | A=28 PSW=10 P1=b7 P2=f0 RB1 F1=1
```

**Routine Logic** (from BIOS disassembly):
```assembly
0x1B0: MOV R0, #0x3E        ; R0 points to RAM[0x3E] (frame counter)
0x1B2: MOV A, @R0           ; A = RAM[0x3E]
0x1B3: JB7 0x13A            ; If bit 7 set, exit early
0x1B5: ANL A, #0x3F         ; A = A & 0x3F (keep lower 6 bits)
0x1B7: XRL A, #0x3B         ; A = A XOR 0x3B (check if == 59)
0x1B9: JNZ 0x13A            ; If not 59, exit early
0x1BB: CALL set_up_ram_access
0x1BD: MOV A, @R0           ; Continue with timer decrement...
```

**Observations**:
- Routine checks if frame counter (RAM[0x3E]) equals 59 (0x3B)
- Only decrements timer when counter == 59
- At frame 55: RAM[0x3E] = 0x31 (49 decimal) → exits early
- At frame 59: RAM[0x3E] = 0x35 (53 decimal) → exits early
- Timer should decrement every 60 frames (1 second at 60 FPS)

### 4. Frame Counter Behavior

**BIOS Maintenance**: RAM[0x3E] incremented every VBlank, wraps at 60

**Issue**: Running PAL (50 FPS) but counter wraps at 60 frames
- NTSC (60 FPS): Counter wraps every 1.0 second ✓
- PAL (50 FPS): Counter wraps every 1.2 seconds ✗

## Open Questions

### Q1: Timer Storage Format
**Question**: How is the timer value stored in RAM?
- Is it 120 (seconds)?
- Is it 0x0200 (BCD for "02:00")?
- Where in RAM is it stored?

**Investigation Needed**:
- Search ROM disassembly for timer initialization
- Find where 120 or 0x02/0x00 is loaded into RAM
- Trace RAM writes during game initialization

### Q2: Timer Display Conversion
**Question**: How is the timer converted to "02:00" format?
- Does BIOS `display_2_digit_bcd_characters` (0x17C) handle this?
- Is there game-specific conversion code?
- How are the 4 digits (0, 2, 0, 0) generated?

**Investigation Needed**:
- Examine BIOS routine 0x17C implementation
- Find calls to this routine in game code
- Understand BCD (Binary Coded Decimal) format used

### Q3: Quad Display Mechanism
**Question**: How do VDC quads work?
- What do registers 0x42-0x49 actually control?
- How do registers 0x3F, 0x40 affect display?
- Could there be a VDC emulation bug?

**Investigation Needed**:
- Review VDC documentation for quad/sprite system
- Check VDC emulator implementation for quad rendering
- Verify register 0x3F/0x40 behavior matches hardware

### Q4: Why Only One Call to up_down_counter?
**Question**: Why is 0x1B0 only called once at frame 106?
- Should it be called every frame?
- Is the game's main loop broken?
- Is there a condition preventing the call?

**Investigation Needed**:
- Find where game calls 0x1B0
- Check if there's a conditional that stops calling it
- Examine game's VBlank handler

## Next Steps

### 1. Understand Quad Register Layout ✓ COMPLETED
**Findings**:
- Quad characters use registers 0x40-0x7F (4 quads, 16 bytes each)
- Quad 0: 0x40-0x4F, Quad 1: 0x50-0x5F, Quad 2: 0x60-0x6F, Quad 3: 0x70-0x7F
- Each quad has 4 characters, each character uses 4 bytes:
  - Byte 0: Y position
  - Byte 1: X position
  - Byte 2: Character shape pointer (lower 8 bits)
  - Byte 3: Bit 0 = 9th bit of shape, Bits 1-3 = color

**Timer Display Writes (Frame 7)**:
```
0x42 = 0xF8 (Quad 0, Char 0, Byte 2 - shape pointer)
0x43 = 0xF8 (Quad 0, Char 0, Byte 3 - shape + color)
0x44 = 0xF8 (Quad 0, Char 1, Byte 0 - Y position)
0x45 = 0xF8 (Quad 0, Char 1, Byte 1 - X position)
0x46 = 0xF8 (Quad 0, Char 1, Byte 2 - shape pointer)
0x47 = 0xF8 (Quad 0, Char 1, Byte 3 - shape + color)
0x48 = 0xF8 (Quad 0, Char 2, Byte 0 - Y position)
0x49 = 0xF8 (Quad 0, Char 2, Byte 1 - X position)
```

**Problem**: All values are 0xF8 - this seems wrong for displaying "02:00"
- 0xF8 as Y/X position = 248 pixels (off-screen?)
- 0xF8 as shape pointer = character 248 (invalid?)
- Need to understand what 0xF8 actually means in this context

### 2. Investigate 0xF8 Values
- [ ] Check if 0xF8 is a special value or placeholder
- [ ] Verify VDC emulator handles 0xF8 correctly
- [ ] Look for subsequent writes that might complete the setup
- [ ] Check if RAM-to-VDC copy writes more than 8 bytes

### 3. Find Timer Storage Location
- [ ] Search ROM for timer initialization (120 seconds or 0x0200 BCD)
- [ ] Identify RAM location storing timer value
- [ ] Trace RAM writes during game initialization
- [ ] Find where timer is decremented

### 4. Understand Timer Display Mechanism
- [ ] Find timer initialization code in ROM
- [ ] Identify RAM location storing timer value
- [ ] Understand BCD format and conversion
- [ ] Document how "02:00" is generated from timer value

### 2. Review VDC Quad Documentation
- [ ] Read VDC documentation for quad/sprite registers
- [ ] Understand registers 0x10-0x7F layout
- [ ] Verify quad rendering in VDC emulator
- [ ] Check if registers 0x3F/0x40 could cause corruption

### 3. Trace Timer Decrement Logic
- [ ] Find where timer value is decremented
- [ ] Verify decrement happens when frame counter == 59
- [ ] Check if timer updates trigger display refresh
- [ ] Understand why display doesn't update

### 4. Identify Root Cause
- [ ] Determine if bug is in game ROM or emulator
- [ ] If emulator bug: identify incorrect behavior
- [ ] If game bug: document the issue
- [ ] Create test case to verify fix

## Test Environment

**ROM**: Course de Voitures + Autodrome + Cryptogramme (1980)(Philips)(FR).bin
**BIOS**: Philips C52 BIOS (19xx)(Philips)(FR).bin
**Video Standard**: PAL (50 FPS)
**Test Sequence**:
1. Press '1' at frame 5 (select game)
2. Press '1' at frame 12 (select level)
3. Press joystick UP at frame 20-70 (gameplay)
4. Observe timer at frame 56 (corruption occurs)

**Trace Files**:
- `trace.log`: Instruction trace with CPU state
- `vdc_trace.log`: VDC register write trace
- `screenshots/frame_*.png`: Visual evidence

## References

- [doc/hardware/bios.md](../hardware/bios.md) - BIOS routines and RAM usage
- [doc/hardware/8245.md](../hardware/8245.md) - VDC register documentation
- [doc/french_bios_annotated.txt](../french_bios_annotated.txt) - Complete BIOS disassembly
- [doc/games/course-de-voitures.md](../games/course-de-voitures.md) - Game documentation

## Investigation Log

### 2024-02-21: VDC Trace Implementation
- Implemented VDC register write tracing
- Fixed trace to work in cycle-accurate execution loop
- Added command-line support for VDC tracing
- Captured initial timer display at frame 7
- Identified no quad character updates at frame 56

### 2024-02-21: Timer Routine Analysis
- Found `up_down_counter` (0x1B0) only called once
- Identified frame counter check logic
- Discovered PAL timing mismatch (wraps at 60, runs at 50 FPS)
- Need to understand timer storage and display mechanism

### Next Session: Process of Elimination
- **NOT single characters** - Debugger shows "P" and "L", not numbers
- **NOT sprites** - Sprites are the cars
- **NOT grid** - Grid draws grid lines
- **MUST BE QUADS** - Only display system left!

**Key Insight**: Timer "02:00" uses 4 quad characters, not 5 separate characters
- Colon ":" must be part of one character's shape
- Likely: "0", "2:", "0", "0" or similar arrangement
- The 8 writes to 0x42-0x49 with 0xF8 at frame 7 ARE the timer setup

**Critical Question**: What does 0xF8 mean in quad character registers?
- All 8 bytes are 0xF8 at frame 7
- This sets up the timer display somehow
- Need to understand quad character data format
- 0xF8 might be setting character shapes, not positions

**BREAKTHROUGH**: Found complete Quad 0 setup sequence!
```
Initial writes (frame ~7): All 0x40-0x4F = 0xF8 (hide/init)
Then setup:
  0x40 = 0x50 (Char 0 Y-pos = 80)
  0x41 = 0x70 (Char 0 X-pos = 112)
  0x42 = 0xD8 (Char 0 Shape = 216)
  0x43 = 0x09 (Char 0 Color = 1, Shape bit 9 = 0)
  0x46 = 0x28 (Char 1 Shape = 40)
  0x47 = 0x08 (Char 1 Color = 0, Shape bit 9 = 1)
  0x4E = 0x38 (Char 3 Shape = 56)
  0x4F = 0x08 (Char 3 Color = 0, Shape bit 9 = 1)
  ... and more
```

**Timer confirmed to use Quad 0 (registers 0x40-0x4F)**

**Next Steps**:
1. Map all Quad 0 register writes in chronological order
2. Understand which writes happen at frame 7 (working) vs frame 56 (corrupted)
3. Identify what changes at frame 56 that causes corruption
4. Check if corruption is due to missing writes or wrong values
1. Understand what registers 0x42-0x49 control in Quad 0
2. Decode what 0xF8 values mean (shape pointers? special values?)
3. Check if there are MORE writes after 0x42-0x49 that complete the setup
4. Look at quad character shapes in ROM/VDC to see digit patterns
5. Find where quad character shapes are defined (internal ROM or external?)
- **Timer is 5 characters "02:00", NOT 4** - Cannot be a simple quad (quads show 4 chars)
- **Analyzed single character registers (0x10-0x3F)** - Characters 0-3 are used for gameplay graphics, updated frequently around frame 50-56
- **Characters 4-11 only used for title screen** - Hidden at frame 12-13, never updated during gameplay
- **0xF8 value means "hide"** - Y-position 248 is off-screen
- **Timer display mechanism still unknown** - Not in single chars 0-11, not clearly in quads
- **Possible explanations**:
  1. Timer uses grid characters (background grid system)
  2. Timer uses quads with colon ":" as part of background
  3. Timer uses a combination of display methods
  4. We're looking at the wrong VDC registers

**Next**: Need to find where "02:00" is actually displayed - check grid system, check if colon is separate, verify quad usage
- Review VDC quad documentation
- Find timer storage location in RAM
- Understand BCD conversion and display
- Determine if VDC emulator has quad rendering bug
