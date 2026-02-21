# Timer Bug - Working Hypothesis

## Summary
The timer display corruption in Course de Voitures is likely caused by an issue in the emulator's RAM-to-VDC copy mechanism used during VBLANK, not a game bug.

## How Timer Display Works

### BIOS Routine: display_2_digit_bcd_characters (0x17C)
1. Prepares 8 bytes of data in external RAM starting at 0x7D (backwards)
2. Writes VDC target address to RAM[0x7E]
3. Writes byte count (8) to RAM[0x7F]
4. Sets RAM[0x3F] bit 7 to request copy during next VBLANK
5. Returns to game code

### VBLANK Interrupt Handler (BIOS 0x01A)
1. Checks RAM[0x3F] bit 7
2. If set, calls copying_code at 0x0A3
3. Reads count from RAM[0x7F]
4. Reads target address from RAM[0x7E]
5. Copies data from RAM[0x7D, 0x7C, 0x7B...] to VDC[target, target+1, target+2...]
6. Clears RAM[0x3F] bit 7

## Evidence from Trace Analysis

### Observation 1: No Direct VDC Character Writes
- The game does NOT write directly to VDC character RAM (0x00-0x3F)
- All timer display goes through the BIOS RAM-to-VDC copy mechanism
- This is why we don't see VDC character writes in the trace during normal execution

### Observation 2: Code at 0xbad-0xbcc is NOT Timer Display
- This code writes to VDC address 0x01 three times
- But it's writing sprite/quad positioning data, not timer characters
- The values (0x36, 0x38, 0x3a = '6', '8', ':') are coincidentally ASCII-like
- This is unrelated to the timer bug

### Observation 3: Timer Updates Once Per Second
- BIOS up_down_counter (0x1B0) is called every frame
- But it only updates when RAM[0x3E] & 0x3F == 0x3B (frame 59 of 60)
- Timer display should only change once per second

## Hypothesis: RAM-to-VDC Copy Bug

The corruption starting at frame 56 suggests:

1. **Timing Issue**: The RAM-to-VDC copy during VBLANK may not be synchronized properly
   - Copy might start too late (after VBLANK ends)
   - Copy might be interrupted
   - VDC might not be in the right state

2. **Copy Mode Issue**: The copy uses special P1 settings (P16=1, P13=0, P14=0)
   - Emulator might not handle copy mode correctly
   - Writes might go to wrong destination
   - Reads might come from wrong source

3. **Address Increment Issue**: The copy loop increments VDC address (R1)
   - VDC might not auto-increment addresses correctly
   - Address wrapping might be wrong
   - Character RAM addressing might be incorrect

## Next Steps

1. **Examine VBLANK Copy Implementation**
   - Check `src/memory.cpp` copy_mode() handling
   - Verify VDC register auto-increment
   - Check timing of copy relative to VBLANK

2. **Add Logging to Copy Mechanism**
   - Log when RAM[0x3F] bit 7 is set
   - Log what data is being copied
   - Log VDC target addresses

3. **Compare with Working Games**
   - Check if other games use the same mechanism
   - See if they have similar issues

4. **Test Copy Mode Directly**
   - Create unit test for RAM-to-VDC copy
   - Verify copy mode enables/disables correctly
   - Test address increment behavior

## Key Code Locations

### Emulator
- `src/memory.cpp::write_external()` - Handles copy mode
- `src/vdc.cpp::write_register()` - VDC register writes
- `src/cpu.cpp` - MOVX instruction execution

### BIOS
- `0x17C`: display_2_digit_bcd_characters
- `0x0A3`: copying_code (RAM-to-VDC copy loop)
- `0x01A`: VBLANK interrupt handler

### Game
- Calls to 0x17C at: 0x52D, 0x6B1, 0x9E4, 0xA9D
