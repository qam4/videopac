# Satellite Attack Frame Loop Analysis

## Frame Loop Structure

**Every frame follows the same pattern:**

1. **VBLANK interrupt fires** (0x01a) → sets F1=1
2. **Game executes logic** (one iteration)
3. **Game calls wait_for_interrupt** (0x176) → clears F1
4. **Game waits in JF1 loop** (0x178-0x17a) until next VBLANK
5. **Repeat**

The game executes **ONE iteration per frame**, not multiple loops within a frame.

### Frame Timing

Each frame takes approximately **59,500 CPU cycles**:
- Frame 0: 59,458 cycles
- Frame 1: 59,503 cycles  
- Frame 2: 59,503 cycles
- Frame 3: 59,503 cycles
- Frame 4: 59,503 cycles
- Frame 5: 59,468 cycles

### Key Addresses

- **0x01a**: VBLANK interrupt handler (CLR F1, CPL F1 to set F1=1)
- **0x176**: wait_for_interrupt (CLR F1, EN I)
- **0x178-0x17a**: JF1 wait loop (waits until F1=1)
- **0x2c3**: Game initialization (Frame 0 entry point)
- **0x500**: main_loop2 (main game loop entry)

### Timeline Example (Frame 0)

| Cycle Range | Location | Description |
|-------------|----------|-------------|
| 0-31,316 | Various | Game initialization and logic |
| 31,316 | 0x176 | CLR F1 (prepare for interrupt) |
| 31,345-54,771 | 0x178-0x17a | JF1 loop (waiting for VBLANK) |
| 54,771 | 0x01a | VBLANK interrupt fires (sets F1=1) |
| 54,771-59,458 | 0x178-0x17a | Continue JF1 loop until frame ends |
| 59,477 | - | Frame 1 begins (JF1 exits, game continues) |

### Frames 6-10: Black Screen (Tight Loop)

Frames 6-10 show a black screen. The game executes ONE iteration per frame, but that iteration is a tight loop at 0x71a-0x723 that repeats hundreds of times within the frame:

```
0x71a: MOV R1,##0xa5
0x71c: MOV A,R3        ; R3=0xaa
0x71d: MOVX @R0,A      ; Write 0xaa to VDC control (0xa0)
0x71e: MOVX A,@R1      ; Read VDC register 0xa5
0x71f: DEC R1
0x720: MOVX A,@R1      ; Read VDC register 0xa4
0x721: CLR C
0x722: ADD A,R2
0x723: JNC 0x71a       ; Loop back (no carry = always jumps)
```

This loop:
- Writes 0xAA to VDC Control register repeatedly
- Reads VDC registers 0xa5 and 0xa4
- Never exits (JNC always jumps because carry is never set)
- Executes ~3,866 times per frame (vs ~3,326 in frame 0)

The game is stuck in this loop, not progressing to character rendering or other game logic.

## Character Codes for Satellites

Based on the Satellite Attack disassembly (`doc/satellite-attack-disassembly.txt` lines 874-877):

```assembly
0729: [ BD 29 ] MOV R5, 29        ; 0x29 (char 'x') to R5
072B: [ 52 2F ] JB2, 2F            ; bit 2 of A set? jmp ret
072D: [ BD 10 ] MOV R5, 10        ; 0x10 (char '+') to R5
```

The game alternates between two characters based on bit 2 of internal RAM location 0x25:

- **Character 16 (0x10)**: '+' (plus sign)
  - ROM data: `0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00`
  - Visual pattern: vertical and horizontal bars forming a plus
  - Used when bit 2 of int_25 is CLEAR

- **Character 41 (0x29)**: 'x' (multiply symbol)
  - ROM data: `0x00,0x66,0x3C,0x18,0x3C,0x66,0x00,0x00`
  - Visual pattern: diagonal lines forming an X
  - Used when bit 2 of int_25 is SET

## Game Behavior

According to the disassembly (`doc/satellite-attack-disassembly.txt` line 151):
- The game sets R6 = 0x0C (12 decimal) - this is the number of satellites/characters
- Characters are written to VDC registers 0x10-0x3F (12 characters × 4 bytes each)
- Satellites alternate between '+' and 'X' patterns

## Character Rendering System

Characters in the Odyssey 2 are rendered using:
1. **Position**: Y (byte 0) and X (byte 1) coordinates
2. **Pattern Pointer**: 9-bit value (byte 2 bits 0-7, byte 3 bit 0) pointing into character ROM
3. **Color**: 3-bit color value (byte 3 bits 1-3)

The ROM address formula is: `(char_ptr + (char_y / 2) + char_row) & 0x1FF`

## Current Issue

The character rendering fix spec (`.kiro/specs/character-rendering-fix/`) addresses the bug where:
- Game correctly writes character data to VDC registers
- Display enable bit is set (foreground ON)
- But satellites (characters) are not appearing on screen

The spec provides comprehensive requirements for fixing the character rendering system to match hardware behavior.

## Game Start Frame Analysis (Frame 5 Example)

This analysis examines one frame after game selection (when user presses '1'). **Every frame follows the same basic structure**, but the specific game logic executed varies based on game state.

### Frame Structure (All Frames)

Every frame has this structure:

1. **VBLANK interrupt** (0x01a): Sets F1=1, reads VDC status, updates game state
2. **Game logic execution**: One iteration through game code
3. **Wait for interrupt** (0x176): Clears F1, enables interrupts
4. **JF1 loop** (0x178-0x17a): Waits until next VBLANK sets F1=1
5. **Frame ends**: Next VBLANK fires, cycle repeats

### Frame 5 Timeline (59,468 cycles total)

This frame shows the transition from selection screen to gameplay.

#### Phase 1: Moving Characters Off-Screen (Cycles 0-2413)

**Cycle 0-20**: MOV R0,##0x10 / MOV R2,##0x30
- Sets up loop to write to 48 (0x30) VDC registers starting at 0x10

**Cycles 20-2413**: Loop at 0x171-0x173 (clear_all_characters)
- Writes 0xF8 to VDC registers 0x10-0x3F (48 bytes = 12 characters × 4 bytes)
- **What this does**: Each character has 4 bytes in VDC memory:
  - Byte 0: Y position
  - Byte 1: X position  
  - Byte 2: Character pattern pointer (low 8 bits)
  - Byte 3: Color + pattern pointer bit 8
- Writing 0xF8 to ALL bytes means:
  - Y positions become 0xF8 (248 decimal) - below visible screen area (screen height is ~192)
  - X positions become 0xF8 (248 decimal) - far right of screen (screen width is ~160)
  - Character patterns become 0xF8 (invalid character code)
  - Colors become 0xF8 (invalid color)
- **Effect**: All 12 characters (satellites + "QUEL JEU?" text) are moved off-screen and become invisible
- Loop executes 48 times, each iteration takes ~50 cycles

**Cycle 2413**: RET from clear_all_characters (0x175)

#### Phase 2: Display Enable (Cycles 2433-2591)

**Cycle 2433**: CALL 0x127 (display_on)
- Called from 0x2ED (end of select_game cleanup)

**Cycles 2452-2551**: Display On Routine
- Disables interrupts
- Reads VDC Control register (0xA0): value is 0xF8
- ORs with 0x28 (enable foreground + grid): result is 0xFF
- Writes 0xFF back to VDC Control
- **CRITICAL BUG**: 0xFF enables ALL bits including unused ones
- Re-enables interrupts

**Cycle 2591**: RET from display_on

#### Phase 3: Game Initialization (Cycles 2611-3016)

**Cycle 2611**: MOV A,R1 (restore keystroke value)
- R1 contains 0x10 (key '1' was pressed)

**Cycle 2621**: JMP 0x408 (end_of_select_game vector)

**Cycle 2640**: JMP 0x500 (main_loop2 - main game entry)

**Cycles 2660-2698**: CALL 0x0EC (set_up_ram_access)
- Configures Port 1 for RAM access

**Cycles 2719-3016**: CALL 0x792 (animation counter update)
- Increments RAM location 0x2A
- Masks with 0x03 (keeps value 0-3)
- Used for satellite animation (alternating between '+' and 'x' characters)

#### Phase 4: Character Update Loop (Cycles 3016-ongoing)

**Cycle 3016**: CALL 0x663 (character update routine)

**Cycles 3036-3056**: Setup
- MOV R2,##0x0C (12 satellites to process)
- MOV R0,##0x6D (start of character data in RAM mirror)

**Character Processing Loop** (12 iterations):

Each iteration processes one satellite character:

1. **Load Y-position** (Cycle offset +0): MOVX A,@R0 → A=0xFF
   - All Y-positions are 0xFF (off-screen from previous frame)

2. **Load character code** (Cycle offset +20-40): 
   - Loads character code 0x3C (60 decimal) = '<' symbol
   - This is NOT a satellite character (satellites use 0x10 '+' or 0x29 'x')
   - Character 0x3C is valid (points to ROM addresses 480-487, within 512-byte ROM)
   - This appears to be leftover/stale data from previous frame

3. **CALL 0x14B** (character_colour_translation - BIOS routine):
   - Converts character code and Y-position to VDC format
   - Input: R4 = Y-position (0xFF), R5 = Character code (0x3C), R6 = Color byte
   - Process:
     - Multiplies character code by 8: 0x3C * 8 = 0x1E0 (480 decimal)
     - Subtracts Y/2 from character pointer: 480 - (255/2) = 480 - 127 = 353
     - Handles 9th bit of character pointer (bit 0 of R6)
   - Output: R5 = byte 2 (low 8 bits of pointer), R6 = byte 3 (color + bit 8 of pointer)
   - Result is a valid character ROM address within 0-511 range

4. **Write to VDC mirror**:
   - Writes translated values back to RAM mirror
   - These will be copied to VDC during next VBlank

**Loop continues for all 12 characters**, each taking ~200-250 cycles.

#### Phase 5: Wait for Next Frame (Cycles ~56,000-59,468)

**Cycle ~56,000**: CALL 0x176 (wait_for_interrupt)
- CLR F1 (clear flag)
- EN I (enable interrupts)

**Cycles ~56,000-59,468**: JF1 loop at 0x178-0x17a
- 0x178: JF1 0x175 (test if F1=1)
- 0x17a: JMP 0x178 (loop back if F1=0)
- Repeats ~170 times until VBLANK

**Cycle ~58,000**: VBLANK interrupt fires (during JF1 loop)
- Interrupt handler at 0x01a executes
- Sets F1=1 to signal interrupt occurred
- Returns to JF1 loop

**Cycle 59,468**: Frame 5 ends, Frame 6 begins
- JF1 detects F1=1 and exits loop
- Game continues execution in Frame 6

### Key Findings

1. **Characters are moved off-screen at frame start**: The clear_all_characters routine moves all characters off-screen by writing 0xF8 to all character data bytes. This makes the "QUEL JEU?" text from the selection screen disappear, along with any other characters.

2. **Display is enabled**: The display_on routine is called and sets foreground+grid bits in VDC Control.

3. **Character data is stale**: The character update loop at 0x663 reads Y-positions of 0xFF (off-screen) and character codes of 0x3C (the '<' symbol, not a satellite).

4. **No character repositioning**: There is NO code in frame 5 that writes new Y/X positions to place satellites on-screen.

5. **VDC Control bug**: The display_on routine reads 0xF8, ORs with 0x28, gets 0xFF. This may cause issues with VDC behavior.

6. **Why no display at end of frame 5**: All 12 characters have Y-position = 0xF8 (248 decimal), which is far below the visible screen area (0-191). Even though the VDC is rendering characters, they are all positioned off-screen and therefore invisible. The satellites should be repositioned to visible coordinates in subsequent frames (frame 6+).

### Root Cause Analysis

The problem is that this transition frame does NOT reposition the satellites. The game:
1. Moves all characters off-screen (hides the "QUEL JEU?" selection text)
2. Enables display
3. Processes character data (but doesn't update positions)
4. Continues to main loop

The satellites should be repositioned during game initialization, but this is not happening in the transition frame. Either:
- The repositioning happens in subsequent frames (frame 6+)
- There's a bug in the game logic that prevents repositioning
- The emulator is not correctly handling some aspect of the character system that prevents the game from seeing that characters need repositioning

### Critical Discovery: RAM-to-VDC Copy Mechanism

**The Odyssey 2 uses an indirect character update system:**

1. **Games write to RAM** (external RAM 0x00-0xFF), not directly to VDC registers
2. **Games set bit 7 of internal RAM location 0x3F** to request a copy
3. **During VBlank interrupt**, BIOS routine `ram_to_vdc_vblank_copying_check` (0x089) copies data from RAM to VDC
4. **The copy uses a descriptor** in RAM locations 0x7F (count), 0x7E (VDC address), 0x7D-backwards (data)

**Why "QUEL JEU!" is still visible:**

The game writes 0xF8 to RAM locations to move characters off-screen, but:
- Either the RAM-to-VDC copy isn't happening
- Or the copy is happening but the VDC isn't using the updated values
- Or the emulator's VDC is reading stale data from before the copy

**The VDC render_characters() method reads from `state_.registers[]`**, which should contain the VDC register values. If those registers still contain the old "QUEL JEU!" character positions (not 0xF8), then the text will still render.

**Root cause**: The emulator may not be properly implementing the RAM-to-VDC copy mechanism during VBlank, or there's a timing issue where the VDC renders before the copy completes.

### Next Steps

1. **Verify character rendering implementation** - The spec (`.kiro/specs/character-rendering-fix/`) provides requirements for fixing the VDC character rendering
2. **Check RAM-to-VDC copy timing** - Ensure the BIOS interrupt handler properly copies character data from RAM to VDC during VBLANK
3. **Analyze frames 6-10** - Determine why these frames show black screen (game waiting state?)
4. **Test with corrected VDC Control** - The 0xFF value may cause issues; should be 0x28 (foreground + grid only)

## Frame-by-Frame Summary

| Frame | Cycles | Calls/Jumps | Description | Display |
|-------|--------|-------------|-------------|---------|
| 0 | 59,458 | 55 | Game initialization, BIOS setup, 12 character updates | "QUEL JEU?" text |
| 1-4 | ~59,500 | ~8 | Selection screen active, waiting for input | "QUEL JEU?" text |
| 5 | 59,468 | ~50 | User presses '1', clear screen, game init | Transition (clearing) |
| 6-10 | ~59,500 | 0 | **BROKEN**: Stuck in infinite loop at 0x71a-0x723 | Black screen |
| 11+ | ~59,500 | 0 | (Still stuck in same loop) | Black screen |

**Pattern**: 
- **Frame 0**: Complex initialization (55 function calls)
- **Frames 1-5**: Normal game loop (~8 calls per frame: VBLANK → game logic → wait)
- **Frame 6+**: BROKEN - stuck in tight loop with NO function calls, never reaches wait_for_interrupt

The game enters a broken state in frame 6 where it gets stuck in an infinite loop and never progresses. See `FRAME_CALL_TREES.md` for detailed call trees.


## Frame 9 Analysis - Big White Square

Frame 9 shows extensive RAM-to-VDC copying during VBLANK, which creates the "big white square" visible in the screenshot.

### What's Happening

The BIOS routine `ram_to_vdc_vblank_copying_check` (at address 0x0a8-0x0ac) is copying data from RAM to VDC registers:

1. **Character Shape Data (0x20-0x3F)**: Being filled with 0xff values
   - 0xff = all bits set = all pixels white
   - This creates white character shapes

2. **Grid Data (0xC0-0xE9)**: Being filled with character references
   - Grid registers tell the VDC which characters to display at each position
   - Values like 0x08, 0xf8, 0x69, 0xf9 are character codes

3. **Display State**: Display is ON (ctrl:0xff, disp:ON)
   - The VDC is actively rendering during this copy
   - White characters on the grid create the visible white square

### Why This Happens

This is normal game initialization:
- The game is setting up the playfield/grid structure
- Character shapes are initialized with placeholder data (0xff)
- The grid is populated with character references
- Later frames will update these with actual game graphics

### Performance Impact

Frame 9 spends most of its time (17,641 + 19,539 + 16,375 = 53,555 cycles) in the RAM-to-VDC copy loop:
- This is the BIOS's automatic VBLANK data transfer mechanism
- The game queues data in RAM, and the BIOS copies it to VDC during VBLANK
- This is efficient because it happens during the vertical blanking interval

### Trace Evidence

From trace.log starting at line 30168 (frame 9 begins):
```
[F:9 C:535281] 0x0a8: MOVX A,@R0  | [READ @R0=0x07 <- RAM]
[F:9 C:535301] 0x0a9: MOVX @R1,A  | [WRITE @R1=0x23 val=0xff -> VDC]
[F:9 C:535380] 0x0a9: MOVX @R1,A  | [WRITE @R1=0x24 val=0xff -> VDC]
[F:9 C:535459] 0x0a9: MOVX @R1,A  | [WRITE @R1=0x25 val=0xff -> VDC]
...
[F:9 C:547720] 0x0a9: MOVX @R1,A  | [WRITE @R1=0xc0 val=0x08 -> VDC]
[F:9 C:547800] 0x0a9: MOVX @R1,A  | [WRITE @R1=0xc1 val=0xf8 -> VDC]
[F:9 C:547879] 0x0a9: MOVX @R1,A  | [WRITE @R1=0xc2 val=0xf8 -> VDC]
```

### Conclusion

The "big white square" is not a bug - it's the game initializing its display grid with white placeholder characters before loading the actual game graphics. This is standard initialization behavior for Videopac/Odyssey² games.
