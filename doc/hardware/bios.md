# Videopac/Odyssey 2 BIOS

The Videopac/Odyssey 2 BIOS is a 1KB ROM (addresses 0x000-0x3FF) that provides system initialization, interrupt handlers, and utility routines for game cartridges.

## BIOS Entry Points

### Interrupt Vectors

- **0x000**: Cold Boot - System initialization and reset
- **0x003**: External T0 Interrupt - Redirects to cartridge vector 0x402
- **0x007**: Timer/Clock Interrupt - Redirects to cartridge vector 0x404
- **0x009**: VBlank Interrupt Routine 1 - Timeable VBlank code
- **0x01A**: VBlank Interrupt Routine 2 - Collision & Clock update
- **0x044**: VBlank Interrupt Routine 3 - Tune player

### Utility Routines

- **0x089**: RAM to VDC VBlank Copying Check
- **0x0A3**: RAM to VDC Copying Code
- **0x0B0**: Keyboard Routine
- **0x0E7**: Set up VDC Access
- **0x0EC**: Set up RAM Access
- **0x0F1**: Reset System
- **0x11C**: Display Off
- **0x127**: Display On
- **0x132**: Enable Data Copy next VSYNC
- **0x13D**: Get Keystroke
- **0x14B**: Character/Colour Translation
- **0x16B**: Clear all Characters
- **0x176**: Wait for Interrupt
- **0x17C**: Display 2 digit BCD Characters
- **0x1A2**: Start Tune
- **0x1B0**: Up/Down Counter
- **0x23A**: Set up Quad Score Characters
- **0x261**: Translate & Copy Character & Colour
- **0x26A**: Bit Test
- **0x280**: Bit Clear
- **0x28A**: Bit Set
- **0x2C3**: Select Game
- **0x300**: Frequency Data (tune player)
- **0x34A**: Tune Data
- **0x376**: Keyboard In routine (end)
- **0x37E**: Miscellaneous Interrupt Handlers for Banked ROMs
- **0x38F**: Read Joystick
- **0x3EA**: Character Write

## Cartridge Vectors

Game cartridges must provide these vectors at the start of their ROM (0x400-0x40A):

- **0x400**: Restart - Game entry point
- **0x402**: VBlank (External) Interrupt - Called by BIOS 0x003
- **0x404**: Timer/Clock Interrupt - Called by BIOS 0x007
- **0x406**: VBlank Routine Vector - Called by BIOS 0x009 if 0x402 = JMP 0x009
- **0x408**: End of Select Game - Called with A = game selection
- **0x40A**: Continuation of VBlank - Called by BIOS 0x01A if 0x406 = JMP 0x01A

## BIOS-Maintained RAM Locations

The BIOS uses specific internal RAM locations (0x3D-0x3F) for system state:

### RAM[0x3D] - Collision Register
- Updated by VBlank interrupt handler from VDC register 0xA2
- Contains collision detection flags
- Read by games to detect sprite/character collisions

### RAM[0x3E] - Frame/Clock Counter
- **Incremented every VBlank interrupt** (60 times per second on NTSC, 50 times per second on PAL)
- **Wraps at 60 frames** (approximately 1 second on NTSC systems)
- Used by games for timing and animation

**VBlank Interrupt Handler Code** (BIOS 0x022-0x02E):
```assembly
0x022: INC R0          ; R0 now points to 0x3E
0x023: INC @R0         ; Increment RAM[0x3E] (Clock)
0x024: MOV A,@R0       ; Load counter value
0x025: ANL A,##0x3f    ; Mask with 0x3F (keep lower 6 bits, 0-63)
0x027: XRL A,##0x3c    ; Check if equals 60 (0x3C)
0x029: JNZ loc_002f    ; If not 60, skip wrap
0x02b: MOV A,@R0       ; Wrap counter back to 0
0x02c: ANL A,##0xc0    ; Keep upper 2 bits only
0x02e: MOV @R0,A       ; Store wrapped value
```

**PAL Compatibility Issue:**
- The counter wraps at 60 frames regardless of video standard
- **NTSC (60 FPS)**: Counter wraps every 1.0 second (60 frames ÷ 60 FPS) ✓
- **PAL (50 FPS)**: Counter wraps every 1.2 seconds (60 frames ÷ 50 FPS) ✗
- Games that use RAM[0x3E] for timing may have bugs in PAL mode
- See [road-movement-bug.md](../case-studies/road-movement-bug.md) for a real-world example

### RAM[0x3F] - Status Register
- **Bit 7**: Request RAM-to-VDC copy during next VBlank
  - Set by game code to trigger data copy
  - Cleared by BIOS after copy completes
- **Bit 6**: Delay VBlank completion (used by tune player)
  - Prevents VBlank handler from calling cartridge vector 0x408
  - Used to synchronize tune playback with frame timing

## VBlank Interrupt Flow

The BIOS provides three VBlank interrupt routines that can be chained together:

### Routine 1 (0x009) - Timeable VBlank Code
- Saves A register in R5
- Reads Port 1 status and saves in R6
- Sets up VDC access
- Checks if in VSYNC (VDC register 0xA1 bit 3)
- If in VSYNC, jumps to cartridge vector 0x406
- Otherwise, restores registers and returns

### Routine 2 (0x01A) - Collision & Clock Update
- Sets F1 flag (signals interrupt occurred)
- Reads collision register from VDC 0xA2 → RAM[0x3D]
- Increments frame counter RAM[0x3E], wraps at 60
- Checks RAM[0x3F] bit 7 for copy request
- If set, calls RAM-to-VDC copy routine and clears bit 7
- Checks RAM[0x3F] bit 6 for delay
- If clear, jumps to cartridge vector 0x408
- If set, decrements R3 delay counter before calling 0x408

### Routine 3 (0x044) - Tune Player
- Reads tune data from table at 0x300 + R4
- Processes tune commands:
  - Bit 7 set: Load new note data
  - Bit 6 set: Set delay counter
  - Bit 5 set: Silence (volume = 0)
  - Bit 4 set: Jump to new position (tune complete)
- Updates VDC sound registers 0xA7-0xAA
- Continues to cartridge vector 0x40A when complete

## RAM-to-VDC Copy Mechanism

The BIOS provides an efficient mechanism to copy data from external RAM to VDC registers during VBlank:

### Setup (Game Code)
1. Write data to external RAM addresses (backwards from 0x7D, 0x7C, 0x7B...)
2. Write VDC target address to RAM[0x7E]
3. Write byte count to RAM[0x7F]
4. Set bit 7 of internal RAM[0x3F] to request copy

### Execution (BIOS During VBlank)
1. Check RAM[0x3F] bit 7 - if clear, skip copy
2. Read RAM[0x7F] to get byte count
3. Read RAM[0x7E] to get VDC target address
4. Enable copy mode (Port 1: P16=1, P13=0, P14=0)
5. Loop:
   - Read from RAM[0x7D], RAM[0x7C], RAM[0x7B]... (backwards)
   - Write to VDC[target], VDC[target+1], VDC[target+2]... (forwards)
6. Clear RAM[0x3F] bit 7
7. Re-enable VDC and turn on display

**Why Backwards?**
The data is stored backwards in RAM because the copy loop decrements the RAM pointer (DEC R0) while incrementing the VDC pointer (INC R1). This allows efficient copying without needing to calculate end addresses.

## Keyboard Routine

The BIOS keyboard routine (0x0B0) scans the keyboard matrix:

1. Disable interrupts
2. Set up keyboard scan mode (Port 1)
3. Scan 6 rows (0xF0-0xF5)
4. For each row:
   - Output row number to Port 2
   - Read Port 2 to get key status
   - If key pressed, debounce by reading 48 times
   - Convert row/column to key code
5. Store result in R7 (0xFF = no key, 0x00-0x3F = key code)
6. Restore Port 1 and return

**Key Code Format:**
- Bits 0-2: Column (0-7)
- Bits 3-5: Row (0-5)
- Bit 6: Always 0
- Bit 7: 0 = key pressed, 1 = no key

## Wait for Interrupt Routine

The BIOS provides a routine (0x176) that blocks until an interrupt occurs:

```assembly
0x176: CLR F1          ; Clear F1 flag
0x177: EN I            ; Enable interrupts
0x178: JF1 0x175       ; Loop until F1 is set
0x17a: JMP 0x178       ; Continue loop
```

This is used by games to synchronize with VBlank:
1. Game calls 0x176 to wait
2. VBlank interrupt fires, sets F1 flag
3. Loop exits, game continues

## References

- [french_bios_annotated.txt](../french_bios_annotated.txt) - Complete annotated BIOS disassembly
- [memory-architecture.md](memory-architecture.md) - Memory map and RAM usage
- [8048.txt](8048.txt) - Intel 8048 CPU reference
- [8245.md](8245.md) - Intel 8245 VDC reference

