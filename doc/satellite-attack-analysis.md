# Satellite Attack Character Analysis

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

## Game Initialization and Main Loop

### Frame Timing
Each frame is approximately 59,470 CPU cycles (measured from trace.log).

### Main Loop Structure

The game has two main loops that alternate:

1. **main_loop2** (0x500):
   - Calls BIOS 0x0EC (labeled "extramenable" in game code, "Set up RAM Access" in BIOS - enables external RAM)
   - Calls increment_2a_2b_counters (0x792)
   - Calls CheckPlayerHit (0x53E)
   - Calls 0x663 (satellite rendering routine)
   - On first run (exram 0x2E == 0):
     - Increments exram 0x2E
     - Calls init_graphics_and_exram_to_vcd_table (0x75D)
     - Calls InitPlayerName (0x74D)
     - Waits for vsync
     - Updates status bar (0x6AB)
     - Calls check_for_keypress (0x603)
     - Jumps to main_loop1
   - On subsequent runs: jumps to 0x512, then continues to main_loop1

2. **main_loop1** (0x411):
   - Increments frame counter (intram 0x25)
   - Calls move_player (0x176 in memory bank 1)
   - Calls show_particles (0x02B in memory bank 1)
   - Calls 0x200 (satellite movement and related logic)
   - Calls ChangeFireDirection (0x4B0)
   - Calls test_for_collision (0x424)
   - Jumps back to main_loop2

### Key Press Handling (0x603)

The check_for_keypress function:
- Calls 0x0B0 to get key
- Disables interrupts
- Stores key in R2
- If bit 7 of key is SET (no key pressed): returns immediately
- If bit 7 is CLEAR (key pressed): processes the key
  - Adds character to player name at position (0x08 + name_length)
  - Sets intram 0x27 to 0x56

### Frame 5 Execution Timeline (Key Press Frame)

**Frame boundaries**: Absolute cycle 297379 (start) to 356837 (end) = 59,458 CPU cycles total

**Timing Constants (from emulator code):**
- VDC clock: 3.54 MHz
- CPU clock: 1.79 MHz / 5 = 0.358 MHz
- VDC cycles per CPU cycle: 3.54 / 0.358 = 9.9
- NTSC scanlines per frame: 262
- VDC cycles per scanline: 227
- Total VDC cycles per frame: 262 × 227 = 59,474
- VBLANK starts at scanline 240 = 240 × 227 = 54,480 VDC cycles into frame
- **VBLANK in CPU cycles: 54,480 / 9.9 ≈ 5,503 CPU cycles into frame**

**Trace log uses CPU cycle counts** (confirmed by examining debugger.cpp log_instruction function)

**Execution sequence (relative to frame start at cycle 0):**

1. **Relative cycle 0-2,621** (absolute 297379-300000): Frame starts, continues from previous frame execution
   - At relative cycle 2,621: JMP to 0x408 (game start check)
   - At relative cycle 2,621: JMP to 0x500 (main_loop2)

2. **Relative cycle 2,621-11,233** (absolute 300000-308612): main_loop2 initialization path
   - CALL extramenable (0x0EC) - enables external RAM
   - CALL increment_2a_2b_counters (0x792)
   - CALL CheckPlayerHit (0x53E)
   - CALL 0x663 (satellite rendering routine)
   - Check exram 0x2E (first run flag) - it's 0, so take initialization path
   - Increment exram 0x2E (mark as initialized)

3. **Relative cycle 11,233-14,220** (absolute 308612-311599): CALL init_graphics_and_exram_to_vcd_table (0x75D)
   - Calls clear_sprites_and_chars (0x76D)
     - Writes 0xF8 to external RAM 0x79-0x10 (clears sprites 1-3 and all 12 foreground characters)
     - Writes 0x00 to clear sprite attributes
   - Copies start_data to external RAM 0x7F-0x7A:
     - 0x7F: 0x40 (copy count = 64 bytes)
     - 0x7E: 0x00 (VDC destination address)
     - 0x7D: 0x60 (player sprite Y position)
     - 0x7C: 0x51 (player sprite X position)
     - 0x7B: 0x08 (player sprite color = RED)
     - 0x7A: 0x00 (player sprite byte 3)
   - **Note**: All writes are to external RAM, not directly to VDC

4. **Relative cycle 14,220-15,683** (absolute 311599-313062): CALL InitPlayerName (0x74D)
   - Initializes player name in external RAM

5. **Relative cycle 15,683-16,148** (absolute 313062-313527): CALL wait_until_scanline (0x716)
   - Waits for specific scanline (465 cycles)

6. **Relative cycle 16,148-22,328** (absolute 313527-319707): CALL update_status_bar (0x6AB)
   - Enables copy mode (ORL P1, 0x7C; ANL P1, 0xE7)
   - Calls set_location_of_quads
   - Calls put_score_in_quads (updates score display in RAM)
   - Calls vcdenable (0x0E7) - enables VDC access
   - Calls set_VDC_control_register_to_0xA8
   - **Duration**: 6,180 cycles

7. **Relative cycle 22,328-55,187** (absolute 319707-352566): CALL waitvsync (0x176) - **BLOCKS HERE**
   - Waits for vsync interrupt
   - **Duration**: 32,859 cycles (55% of frame!)
   - **VSYNC INTERRUPT FIRES at relative cycle 54,465** (absolute 351883)
     - This is 54,465 CPU cycles into frame
     - Expected VBLANK at ~5,503 cycles, but interrupt fires at 54,465 cycles
     - **PROBLEM: Interrupt fires 48,962 cycles LATE!**
   - BIOS handler at 0x009 executes:
     - Checks VDC status register 0xA1 for VBLANK bit (bit 3)
     - Jumps to cartridge vector 0x406 → 0x600 → 0x01A
     - Handler at 0x01A checks RAM 0x3F bit 7 for copy flag
     - **BIT 7 OF RAM 0x3F IS SET** (value 0x40 = 0b01000000)
     - Handler does NOT call copying_code (0x089/0x0A3) because bit 7 is CLEAR after complement
     - Returns from interrupt at relative cycle 55,127 (absolute 352506)

8. **Relative cycle 55,187-55,552** (absolute 352566-352931): Returns from waitvsync, continues execution
   - CALL 0x134 (365 cycles) - **SETS BIT 7 OF RAM 0x3F** (enables copy for NEXT vsync)
   - Writes to external RAM 0xA3, 0xA2

9. **Relative cycle 55,552-59,458** (absolute 352931-356837): CALL check_for_keypress (0x603)
   - Frame ends at cycle 356837 inside this function at address 0x606
   - **Never reaches main_loop1**

**CRITICAL FINDINGS:**

1. **ROOT CAUSE - VDC TIMING BUG**: `VideoTiming::NTSC_CYCLES_PER_SCANLINE = 23` should be 227
   - File: `include/vdc.h` line ~180
   - Current value: 23 cycles per scanline
   - Correct value: 227 cycles per scanline  
   - This causes VDC beam to advance 10× slower than it should
   - VBLANK (scanline 240) reached at VDC cycle 240×23=5,520 instead of 240×227=54,480
   - In CPU cycles: 5,520/9.9≈558 instead of 54,480/9.9≈5,503
   - **Wait, that's backwards...**

2. **ACTUAL ROOT CAUSE**: VDC beam calculation uses wrong constant
   - `tick_one_cycle()` calculates: `beam_y = frame_cycles / VideoTiming::CYCLES_PER_SCANLINE`
   - But `calculate_timing()` sets: `cycles_per_scanline_ = VideoTiming::NTSC_CYCLES_PER_SCANLINE`
   - The calculation uses the GENERIC constant (227) not the NTSC-specific one (23)
   - This mismatch causes beam_y to advance too slowly
   - With 23 cycles/scanline: scanline 240 reached at VDC cycle 240×23=5,520
   - But frame_cycles uses 227: so beam_y=240 when frame_cycles=240×227=54,480
   - In CPU cycles: 54,480/9.9≈5,503 - **THIS IS CORRECT!**

3. **REAL BUG**: VDC total_cycles doesn't advance correctly
   - VDC tick_one_cycle() increments state_.total_cycles by 1 per VDC cycle
   - But master clock calls vdc_ticked() which should sync with CPU
   - Need to verify VDC is being ticked at correct rate (9.9× CPU rate)
   - Interrupt fires at CPU cycle 54,465 = VDC cycle 54,465×9.9 = 539,204
   - But frame is only 59,474 VDC cycles!
   - **VDC is not being ticked enough times per CPU instruction**

4. **Copy flag timing issue**: 
   - RAM 0x3F bit 7 is checked BEFORE it's set
   - Game sets bit 7 at cycle 55,187 (after waitvsync returns)
   - But vsync interrupt already fired at cycle 54,465
   - Copy will happen on NEXT frame's vsync, not this frame

5. **Why "QUEL JEU!" still shows at end of frame 5:**
   - Characters were cleared in external RAM at cycle 11,233-14,220
   - But RAM→VDC copy didn't happen because bit 7 wasn't set yet
   - VDC still has old character data from previous frames
   - Screenshot at end of frame 5 shows VDC state, not RAM state

### Why 3 Frames Delay?

Frame 5: Initialization (clears screen, sets up player, waits for key)
Frame 6: First iteration of main_loop1 (moves player, shows particles)
Frame 7: Second iteration (continues game logic)
Frame 8: Third iteration - satellites finally appear

The 3-frame delay is because:
1. Frame 5 does initialization and clears the screen
2. Frames 6-7 run the game loop but satellites may not be positioned yet
3. Frame 8 is when satellites are finally written to VDC registers and rendered

## Current Issue

The character rendering fix spec (`.kiro/specs/character-rendering-fix/`) addresses the bug where:
- Game correctly writes character data to VDC registers
- Display enable bit is set (foreground ON)
- But satellites (characters) are not appearing on screen

The spec provides comprehensive requirements for fixing the character rendering system to match hardware behavior.
