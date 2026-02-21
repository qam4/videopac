# Course de Voitures + Autodrome + Cryptogramme

**ROM**: `Course de Voitures + Autodrome + Cryptogramme (1980)(Philips)(FR).bin`

## Game Selection

The cartridge contains 3 games:
1. **Course de Voitures** (Road Racing) - Press '1'
2. **Autodrome** (Top-down Circuit Racing) - Press '2'
3. **Cryptogramme** (Word Game) - Press '3'

## Course de Voitures (Game 1) - Road Racing

### Start Sequence

1. Press '1' to select Course de Voitures
2. Press '1' again to select level
3. Press UP (joystick) to start accelerating

### RAM Locations

- **RAM[0x30]**: Speed register
  - 0x00 = stopped
  - 0x01 = moving (increments when UP pressed, decrements when released)
  - Controls road scrolling speed
  
- **RAM[0x3F] bit 7**: UP button state
  - Bit 7 = 1: UP is pressed (accelerate)
  - Bit 7 = 0: UP not pressed (decelerate)

### Road Rendering - RASTER EFFECTS TECHNIQUE

**IMPORTANT**: The road animation uses **mid-frame register updates** (raster effects), NOT moving quad characters.

The road is created using:

1. **2 vertical blue grid lines** (always present):
   - VDC register E1 = 0xFF (left side)
   - VDC register E6 = 0xFF (right side)
   - Grid color: Blue (from color register 0xA3)

2. **Road gaps created by toggling grid enable bit**:
   - Game calls BIOS `display_on` (0x127) to enable grid at specific scanlines
   - Game calls BIOS `display_off` (0x11C) to disable grid at other scanlines
   - VDC control register 0xA0 bit 3 controls grid enable
   - When grid is OFF, the vertical lines disappear, creating gaps

3. **Pattern repeats down the screen**:
   - Grid ON for ~52 scanlines → vertical blue lines visible
   - Grid OFF for ~12 scanlines → gap appears (no vertical lines)
   - Repeat 4-6 times per frame to create multiple road segments

4. **Animation by shifting Y positions**:
   - Each frame, the scanline Y positions where grid toggles happen shift
   - Amount of shift controlled by speed (RAM[0x30])
   - This makes the gaps appear to scroll toward the player

**Why this technique?**
- No need to move actual graphics or update character positions
- Minimal CPU overhead (just a few BIOS calls per frame)
- Creates smooth scrolling with very little data
- The vertical grid lines (E1/E6) never change - only when they're visible changes

**Characters are for UI, not road**:
- The 8 characters (Char0-Char7) are used for score, timer, and UI elements
- They are NOT used to create the road segments

### Scrolling Mechanism

The road scrolling effect is created by shifting the scanline Y positions where the grid enable bit is toggled. The speed (RAM[0x30]) controls how much these Y positions shift each frame.

**How it works:**
- Speed = 0: Grid toggles at same Y positions every frame → road stationary
- Speed = 1: Grid toggle Y positions shift by 1 scanline per frame → slow scroll
- Speed = 5: Grid toggle Y positions shift by 5 scanlines per frame → fast scroll

The game calculates: `toggle_Y_position = base_Y + (frame_counter * speed)`

#### Speed Control (0x4a3-0x4b9)

This code runs every frame and updates RAM[0x30] based on joystick input:

```assembly
0x4a3: b9 30  MOV R1,##0x30      ; R1 = address of speed register
0x4a5: f2 ae  JB7 loc_04ae       ; Jump if RAM[0x3F] bit 7 set (UP pressed)

; If UP NOT pressed (bit 7 = 0):
0x4a7: f1     MOV A,@R1          ; Load speed from RAM[0x30]
0x4a8: c6 d2  JZ loc_04d2        ; If speed = 0, skip (already stopped)
0x4aa: 07     DEC A              ; Decrement speed (deceleration)
0x4ab: a1     MOV @R1,A          ; Store back to RAM[0x30]
0x4ac: 76 ba  JF1 loc_04ba       ; Jump to sound effect code

; If UP pressed (bit 7 = 1):
loc_04ae:
0x4ae: bb fa  MOV R3,##0xfa      ; R3 = -6 (used for speed limit check)
0x4b0: f0     MOV A,@R0          ; Load RAM[0x3F] (game state flags)
0x4b1: 12 b5  JB0 loc_04b5       ; Check bit 0 (affects speed limit)
0x4b3: bb f8  MOV R3,##0xf8      ; R3 = -8 (different speed limit)

loc_04b5:
0x4b5: f1     MOV A,@R1          ; Load current speed from RAM[0x30]
0x4b6: 6b     ADD A,R3           ; Add negative offset (check if at limit)
0x4b7: f6 d2  JC loc_04d2        ; If carry (speed >= limit), skip increment
0x4b9: 11     INC @R1            ; INCREMENT SPEED in RAM[0x30]
```

**Key Points:**
- RAM[0x3F] bit 7 = 1 means UP is pressed (accelerate)
- RAM[0x3F] bit 7 = 0 means UP not pressed (decelerate)
- Speed is capped by checking if adding a negative offset causes carry
- When speed = 0, road doesn't move

#### Grid Toggle Timing Example (from actual trace)

```
Frame N:
  Control #13: Grid ON  at Y=18  → scanlines 18-70 show vertical lines (52 scanlines)
  Control #14: Grid OFF at Y=70  → scanlines 70-82 show gap (12 scanlines)
  Control #15: Grid ON  at Y=82  → scanlines 82-213 show vertical lines (131 scanlines)
  Control #16: Grid OFF at Y=213 → scanlines 213-225 show gap (12 scanlines)
  Control #17: Grid ON  at Y=225 → scanlines 225-277 show vertical lines (52 scanlines)
  Control #18: Grid OFF at Y=277 → scanlines 277+ show gap

Frame N+1 (speed=1):
  Control #13: Grid ON  at Y=19  → Y positions shifted by 1 scanline
  Control #14: Grid OFF at Y=71
  ... (pattern continues with +1 offset)
```

The Y offset increases each frame based on speed, creating the scrolling effect.

### Timing

From headless mode testing:
- **Frame 10**: Game selected
- **Frame 14**: Level selected
- **Frame 20**: UP pressed
- **Frame 22**: Speed increases to 1, grid toggle Y positions start shifting
- **Frame 24**: Road scrolling visible as gaps move

### Known Issues

**Bug**: In SDL mode, the road scrolls immediately after level selection WITHOUT pressing UP. This suggests RAM[0x3F] bit 7 is being set incorrectly, or the grid toggle timing is being triggered when it shouldn't be.

## Main Game Loop Structure

Understanding the game loop helps trace how RAM[0x30] affects rendering:

```
Main Loop (simplified):
1. Read joystick input (0x495: CALL bios:read_joystick)
2. Update speed based on input (0x4a3-0x4b9: speed control logic)
3. Calculate grid toggle Y offset based on RAM[0x30]
4. During frame rendering:
   - Call display_on/display_off at calculated Y positions
   - Grid enable bit toggles mid-frame at specific scanlines
   - Creates gaps that appear to scroll
5. Wait for VBlank interrupt
6. Repeat
```

**Critical Addresses for Debugging:**
- 0x495: Joystick read (sets RAM[0x3F] bit 7 based on UP button)
- 0x4a3-0x4b9: Speed control (updates RAM[0x30])
- 0x11C (BIOS display_off): Clears grid enable bit (0xA0 bit 3)
- 0x127 (BIOS display_on): Sets grid enable bit (0xA0 bit 3)
- RAM[0x30]: Speed register (0 = stopped, >0 = moving)
- RAM[0x3F] bit 7: UP button state (1 = pressed, 0 = not pressed)
- VDC 0xA0: Control register (bit 3 = grid enable)

## Disassembly Reference

See `course_de_voitures_disasm.txt` for complete disassembly.

### Key Routines

**Speed and Input:**
- 0x495: Read joystick (CALL bios:read_joystick)
- 0x497-0x49c: Check frame counter for input timing
- 0x4a3-0x4b9: Speed control (acceleration/deceleration based on RAM[0x3F] bit 7)
- 0x4ba-0x4d0: Sound effects based on speed changes
- 0x4d2-0x4f8: Speed-based game logic (collision checks, etc.)

**Road Rendering:**
- 0x11C (BIOS display_off): Turn off grid (clear bit 3 of 0xA0)
- 0x127 (BIOS display_on): Turn on grid (set bit 3 of 0xA0)
- 0x422-0x453: Grid line setup (VDC registers E1, E6 = 0xFF for vertical lines)
- Game calls display_on/off multiple times per frame at calculated Y positions

**BIOS Routines Used:**
- 0x08F (bios:read_joystick): Read joystick state into R3
- 0x0E7 (bios:set_up_vdc_access): Configure Port 1 for VDC access
- 0x0EC (bios:set_up_ram_access): Configure Port 1 for RAM access
- 0x121 (via 0x353): Turn off display (clear foreground/grid bits)
- 0x127 (bios:display_on): Turn on display (set foreground/grid bits)
- 0x176 (bios:wait_for_interrupt): Wait for VBlank interrupt

### Memory Map

**Internal RAM:**
- 0x30: Speed register (0 = stopped, increments with UP, decrements without)
- 0x32: Derived speed value (used in calculations)
- 0x33-0x34: Game state flags
- 0x35-0x37: Score/timer related
- 0x3E: BIOS frame counter (incremented by VBlank interrupt)
- 0x3F: Game state flags (bit 7 = UP pressed, bit 0 = affects speed limit)

**VDC Registers (External RAM via MOVX):**
- 0xA0: VDC Control (bit 5 = grid enable, bit 3 = foreground enable)
- 0xA2: Collision register
- 0xA3: Color register
- 0xC0-0xC7: Quad character registers (8 road segments)
  - Each quad: Y position, X position, pattern, color
- 0xE1, 0xE6: Grid vertical line registers (0xFF = full line)
