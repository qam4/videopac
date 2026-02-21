# Course de Voitures Timer Display Bug Investigation

## Bug Description

**Game**: Course de Voitures (Game 1 from Course de Voitures + Autodrome + Cryptogramme)
**Symptom**: After starting the game and driving for a few seconds, the timer display (which starts at 02:00 and counts down) becomes corrupted with random characters appearing at the top right of the screen.

## Reproduction Steps

1. Start emulator with Course de Voitures ROM
2. Press '1' to select Game 1 (Course de Voitures)
3. Press '1' again to select level
4. Press UP to accelerate
5. Wait 3-5 seconds
6. Observe timer at top right becoming corrupted

## Investigation Plan

### Phase 1: Evidence Collection ✅

- [x] Reproduce bug in headless mode with screenshots
- [x] Capture trace log during bug occurrence
- [ ] Identify exact frame when corruption starts
- [ ] Compare good vs corrupted screenshots

### Phase 2: Code Analysis

- [ ] Disassemble ROM to find timer display code
- [ ] Locate character data for digits 0-9 and colon
- [ ] Find timer update routine
- [ ] Identify VDC writes for timer display

### Phase 3: Trace Analysis

- [ ] Filter trace for VDC character RAM writes
- [ ] Identify what's being written to timer character positions
- [ ] Compare expected vs actual character codes
- [ ] Check if character ROM is being corrupted

### Phase 4: Root Cause

- [ ] Determine if bug is in:
  - Game code (wrong character codes)
  - Emulator VDC (character RAM corruption)
  - Emulator character ROM (wrong character data)
  - Timing issue (race condition)

### Phase 5: Fix and Verify

- [ ] Implement fix
- [ ] Verify with headless test
- [ ] Verify with SDL test
- [ ] Add regression test if needed

## Evidence

### Headless Test Run

**Command**:
```bash
videopac.exe --headless --screenshot 1 --frames 300 \
  --press-key 1 5 5 \
  --press-key 1 12 5 \
  --press-joystick 2 0 20 180 \
  --debug --trace \
  --bios "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" \
  "roms/Course de Voitures + Autodrome + Cryptogramme (1980)(Philips)(FR).bin"
```

**Results**:
- 300 frames captured
- Screenshots saved to `screenshots/frame_NNNNNN.ppm`
- Trace log saved to `trace.log` (49,595 instructions)

**Timeline**:
- Frame 5: Press '1' (select game)
- Frame 12: Press '1' (select level)
- Frame 20: Press UP (start driving)
- Frame 20-199: Holding UP
- Frame 200+: Released UP, coasting

### Key Frames to Examine

- Frame 20-30: Game start, timer should show 02:00
- Frame 60-90: Timer counting down (01:59, 01:58, etc.)
- Frame 100-150: Look for corruption starting
- Frame 200-300: Corruption should be visible

## Timer Display Technical Details

### Expected Behavior

The timer should:
1. Start at 02:00 when game begins
2. Count down every second (60 frames)
3. Display format: MM:SS (minutes:seconds)
4. Use character display at top right of screen

### Character Display

The Videopac uses:
- **Character RAM**: Stores which characters to display and where
- **Character ROM**: Stores the pixel patterns for each character
- Characters 0-9 for digits
- Character for colon (:)

### Possible Causes

1. **Wrong character codes**: Game writes incorrect character codes to VDC
2. **Character RAM corruption**: VDC character RAM gets corrupted
3. **Character ROM issue**: Character patterns are wrong
4. **Timing bug**: Race condition in timer update code
5. **VDC register corruption**: Display registers get wrong values

## Next Steps

1. Examine screenshots around frame 60-150 to find exact corruption frame
2. Generate disassembly of ROM to find timer code
3. Analyze trace log for VDC writes during corruption
4. Compare with working timer display in other games

## Notes

- The road scrolling works correctly (documented in course-de-voitures.md)
- This is a separate issue from the road movement bug
- Timer is likely using single characters, not quad characters
- Need to check if BIOS character ROM is correct for digits

