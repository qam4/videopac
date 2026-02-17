# Case Study: Racing Game Road Segment Color Bug

## Game Information
- **Game**: Course de Voitures + Autodrome + Cryptogramme (1980)(Philips)(FR)
- **Region**: French (C52 SECAM)
- **BIOS**: Philips C52 BIOS (19xx)(Philips)(FR).bin
- **Expected Palette**: PAL timing + NTSC colors
- **Note**: This cartridge contains 2 games (select with '1' or '2' at start)

## Summary
Multiple rendering and gameplay bugs affect both games in this cartridge. The primary issue is that grid-based elements (road segments in game 1, racing circuit in game 2) appear dark red instead of white. Investigation reveals the game sets VDC register 0xA3 (background/grid color) to 0x69, which should render as white but appears as red.

## Symptoms

### Game 1 (Course de Voitures - Road Racing)
- **Observed**: Road segments appear dark red (RGB: 0xFF, 0x49, 0x49)
- **Expected**: Road segments should appear white
- **Affected Elements**: Grid-based road markings (vertical segments)
- **Unaffected**: Player car (red), timer text (blue), other UI elements

### Game 2 (Autodrome - Top-Down Circuit Racing)
Multiple issues observed:

1. **Grid Color Bug** (Same as Game 1)
   - **Observed**: Racing circuit appears dark red
   - **Expected**: Circuit should appear white

2. **Grid Rendering Bug**
   - **Observed**: Circuit shape is incorrect/malformed
   - **Expected**: Proper circuit layout
   - **Note**: Difficult to describe without visual reference

3. **Input Handling Bug**
   - **Observed**: Both cars (blue and red) move when using keyboard arrows
   - **Expected**: Only one car should respond to player input

4. **Sprite Direction Bug**
   - **Observed**: Pressing left arrow makes cars face right, and vice versa
   - **Expected**: Cars should face the direction they're moving
   - **Note**: Cars do move in the correct direction despite facing wrong way

5. **Collision Detection Bug**
   - **Observed**: No collision between cars and circuit walls
   - **Expected**: Cars should collide with circuit boundaries

### Common Elements (Both Games)
- Timer display works correctly (blue text)
- Lap counters display correctly
- Checkpoints render correctly (blue vertical segments)

## Investigation Process

### Step 1: Identify Rendering System
Road segments are rendered using the VDC grid system, not characters or sprites.

### Step 2: Trace Color Register Writes
Captured execution trace at frame 5 (game initialization after pressing '1'):

```
[F:5 C:361980] 0x831: b8 a3 MOV R0,##0xa3
[F:5 C:362000] 0x833: 23 69 MOV A,##0x69
[F:5 C:362020] 0x835: 90 f5 MOVX @R0,A | A=69
```

### Step 3: Analyze Color Value
ROM address 0x833 loads value 0x69 into accumulator, which is written to VDC register 0xA3 (background/grid color).

**Color Value Breakdown:**
- 0x69 = 0b01101001
- Bits 0-3 (color index) = 0b1001 = 9
- Color 9 in NTSC palette = RED (high-intensity: 0xFF, 0x49, 0x49)

**Expected for White:**
- 0x6F (color 15) = White (0xFF, 0xFF, 0xFF)
- 0x67 (color 7) = Grey (0xCE, 0xCE, 0xCE)

### Step 4: Verify ROM Code
Disassembly confirms the value 0x69 is hardcoded in the ROM:

```assembly
0x831: b8 a3  MOV R0,##0xa3    ; R0 = VDC register 0xA3 (background/grid color)
0x833: 23 69  MOV A,##0x69     ; A = 0x69 (color 9 = red in NTSC)
0x835: 90     MOVX @R0,A       ; Write to VDC register
```

## Root Cause Analysis

Since the ROM and BIOS are confirmed not bugged, this indicates an **emulator bug** in color interpretation.

### Hypothesis 1: Color Bit Interpretation
The emulator may be incorrectly interpreting bits in the 0x69 value:
- Possible bit ordering issue
- Possible palette index calculation error
- Possible high/low intensity bit handling

### Hypothesis 2: Palette Selection
The French C52 uses PAL timing but NTSC colors. The emulator may be:
- Applying the wrong palette
- Mixing PAL and NTSC color mappings
- Incorrectly handling the special French C52 configuration

### Hypothesis 3: Grid Color Register Handling
VDC register 0xA3 may have special handling that differs from sprite/character colors:
- Different bit layout
- Different palette mapping
- Additional color transformation

## Data Collection

### VDC Register State (Frame 7)
```
Control (0xA0): 0x28 [Display:ON Grid:ON]
Status (0xA1): 0x2
Collision (0xA2): 0x3
Color (0xA3): 0x69 (should be white, renders as red)
```

### Palette Configuration
- Video Standard: PAL (50Hz)
- Palette Mode: NTSC colors
- Region: france

### Color Palette Reference (NTSC)
```cpp
// From include/types.h
constexpr Color PALETTE_NTSC[16] = {
    // Low-intensity (0-7)
    {0x00, 0x00, 0x00},  // 0: Black
    {0xC6, 0x00, 0x08},  // 1: DARK RED
    {0x00, 0x9C, 0x18},  // 2: Dark Green
    {0x00, 0xBD, 0xDE},  // 3: Light Blue
    {0x08, 0x39, 0xD6},  // 4: DARK BLUE
    {0xCE, 0x10, 0xB5},  // 5: Violet
    {0x9C, 0x84, 0x10},  // 6: Orange/Gold
    {0xCE, 0xCE, 0xCE},  // 7: Grey
    // High-intensity (8-15)
    {0x49, 0x49, 0x49},  // 8: Light Grey
    {0xFF, 0x49, 0x49},  // 9: RED (High-intensity red) <-- 0x69 maps here
    {0x49, 0xFF, 0x49},  // 10: Green
    {0x49, 0xFF, 0xFF},  // 11: Cyan
    {0x49, 0x49, 0xFF},  // 12: BLUE (High-intensity blue)
    {0xFF, 0x49, 0xFF},  // 13: Magenta
    {0xFF, 0xFF, 0x49},  // 14: Yellow
    {0xFF, 0xFF, 0xFF}   // 15: White <-- Expected
};
```

## Next Steps for Investigation

1. **Check VDC Color Register Implementation**
   - Review `src/vdc.cpp` register 0xA3 handling
   - Verify bit extraction for color index
   - Check if grid colors use different palette mapping

2. **Compare with Reference Emulator**
   - Test same ROM on o2em or other emulator
   - Capture screenshots for comparison
   - Verify expected behavior

3. **Test Color Value Variations**
   - Modify ROM to test different color values
   - Verify 0x6F renders as white
   - Check if other color indices work correctly

4. **Review French C52 Specifications**
   - Verify NTSC color palette is correct
   - Check for any special C52 color handling
   - Review hardware documentation

## Files Referenced
- ROM: `roms/Course de Voitures + Autodrome + Cryptogramme (1980)(Philips)(FR).bin`
- BIOS: `roms/Philips C52 BIOS (19xx)(Philips)(FR).bin`
- Disassembly: `racing_game_disasm.txt` (address 0x833)
- Trace: `trace_annotated.log` (frame 5, cycle 362020)
- Palette: `include/types.h` (PALETTE_NTSC)

## Conclusion

The emulator has multiple bugs affecting this game cartridge:

1. **Primary Bug - Grid Color**: The emulator incorrectly renders color value 0x69 as red (color 9) when it should render as white. Since the ROM and BIOS are confirmed correct, this is an emulator bug in color interpretation, likely in the VDC register 0xA3 (background/grid color) handling or palette index calculation.

2. **Grid Rendering Bug**: The circuit shape in game 2 is malformed, suggesting issues with grid pattern generation or display.

3. **Input Handling Bug**: Both players respond to the same input, indicating a problem with input routing or player selection logic.

4. **Sprite Direction Bug**: Sprites are horizontally mirrored (facing opposite direction), suggesting an issue with sprite pattern rendering or flip bit handling.

5. **Collision Detection Bug**: Collision between sprites and grid elements is not working, indicating issues with the collision detection system.

**Priority**: The grid color bug affects both games and is the most visible issue. The other bugs in game 2 make it unplayable.

**Status**: Bugs identified and documented. Root causes require investigation of:
- VDC color handling (register 0xA3)
- Grid pattern rendering
- Input system
- Sprite rendering (horizontal flip)
- Collision detection system
