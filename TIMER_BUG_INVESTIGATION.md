# Timer Display Bug - Quick Investigation Guide

## Current Status

✅ Reproduced in headless mode (300 frames captured)
✅ Trace log captured (49,595 instructions)
✅ Disassembly generated
🔍 Next: Find exact frame of corruption

## Quick Commands

### View specific screenshot frames
```bash
# Check frames 60-150 for corruption
ls screenshots/frame_0000{60..99}.ppm
ls screenshots/frame_000{100..150}.ppm
```

### Search trace for VDC character writes
```bash
# Character RAM is at 0x40-0x7F in VDC
grep "MOVX.*0x[4-7][0-9a-f]" trace.log | head -100
```

### Key Code Locations

**Timer Setup (0x400-0x420)**:
```
0x405: MOV A,##0x02      ; Minutes = 2
0x407: MOVX @R0,A        ; Write to RAM
0x409: MOV A,##0x01      
0x40b: MOVX @R0,A        
0x418: MOV R0,##0x3e     
0x41a: MOV @R0,##0x3b    ; Seconds = 59 (0x3B)
0x420: CALL bios:up_down_counter  ; Display timer
```

**BIOS up_down_counter**: This routine likely displays the timer using characters

## Investigation Steps

1. **Find corruption frame**:
   - Examine screenshots/frame_000060.ppm through frame_000150.ppm
   - Look for when timer changes from normal digits to garbage
   - Note exact frame number

2. **Analyze trace at corruption**:
   - Find VDC writes around that frame
   - Check what character codes are being written
   - Compare with expected digit characters (0-9, colon)

3. **Check character ROM**:
   - Verify BIOS has correct character patterns for digits
   - Check if character codes match expected values

4. **Root cause**:
   - Is game writing wrong character codes?
   - Is VDC corrupting character RAM?
   - Is character ROM data incorrect?

## Expected Timer Format

```
Position: Top right of screen
Format: MM:SS
Start: 02:00
Countdown: Every 60 frames (1 second)

Character codes (typical):
'0' = 0x00
'1' = 0x01
'2' = 0x02
...
'9' = 0x09
':' = 0x0A or similar
```

## Next Actions

1. Open screenshots in image viewer to find corruption
2. Note frame number when it starts
3. Search trace.log for VDC writes near that frame
4. Compare with working frames

