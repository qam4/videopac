# Course de Voitures Timer Bug Analysis

## Reproduction Successful

Successfully reproduced the bug in headless mode using `run_emulator.py`:
- 300 frames captured
- Game started correctly (key '1' pressed at frame 5 and 12)
- Car driving forward (joystick UP from frame 20-300)
- Trace log captured: 49,595 instructions

## Next Steps

1. **Visual Analysis**: Examine screenshots to identify exact frame where timer corruption begins
   - Timer should show "02:00" initially and count down
   - Look for frames 20-150 where corruption appears
   
2. **Trace Analysis**: Once corruption frame is identified, analyze trace.log at that point
   - Look for VDC character RAM writes (addresses 0x0000-0x03FF in VDC space)
   - Timer display is at top-right of screen
   - Character positions approximately: row 0, columns 26-30
   
3. **Disassembly Review**: Cross-reference with `course_de_voitures_full_disasm.txt`
   - Timer setup code at ROM 0x400-0x420
   - Uses BIOS `up_down_counter` routine
   
4. **Root Cause**: Determine if bug is in:
   - Game code (incorrect character codes being written)
   - Emulator VDC (character RAM corruption)
   - Emulator timing (race condition)

## Key Files
- Screenshots: `screenshots/frame_*.png` (300 frames)
- Trace log: `trace.log` (49,595 instructions)
- Disassembly: `course_de_voitures_full_disasm.txt`
- Investigation plan: `doc/case-studies/timer-display-bug.md`
