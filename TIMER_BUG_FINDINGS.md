# Timer Bug - Key Findings

## Bug Reproduction
- Bug starts at frame 56
- Successfully captured in headless mode with trace log

## VDC Write Analysis

### Character RAM Address 0x01 Writes
The game writes to VDC character RAM address 0x01 every frame with changing values:

| Frame | PC    | Value Written | Notes |
|-------|-------|---------------|-------|
| 55    | 0xbaf | 0x36 ('6')    | ASCII '6' |
| 55    | 0xbc5 | 0x3c          | Not a digit |
| 55    | 0xbcc | 0x2b          | ASCII '+' |
| 56    | 0xbaf | 0x38 ('8')    | ASCII '8' |
| 56    | 0xbc5 | 0x3d          | Not a digit |
| 56    | 0xbcc | 0x2a          | ASCII '*' |
| 57    | 0xbaf | 0x3a (':')    | ASCII ':' |
| 57    | 0xbc5 | 0x3c          | Not a digit |
| 57    | 0xbcc | 0x2a          | ASCII '*' |

### Observations

1. **Three writes per frame** to address 0x01 from three different PC locations:
   - PC 0xbaf: Writes incrementing values (0x36 → 0x38 → 0x3a)
   - PC 0xbc5: Writes 0x3c or 0x3d
   - PC 0xbcc: Writes 0x2a or 0x2b

2. **Values are NOT proper timer digits**:
   - 0x36 = ASCII '6' (should be '0' = 0x30 for "02:00")
   - 0x38 = ASCII '8'
   - 0x3a = ASCII ':'
   - These are incrementing ASCII codes, not timer values!

3. **Multiple writes to same address**:
   - Writing to address 0x01 three times per frame
   - Last write wins, so only 0x2a/0x2b is actually displayed
   - This explains the "random characters" - it's showing '+' and '*'

## Root Cause Hypothesis

The game code at PC 0xbaf, 0xbc5, and 0xbcc is writing to the SAME VDC address (0x01) when it should be writing to DIFFERENT addresses for each timer digit.

Possible causes:
1. **Register corruption**: R0 is not being incremented properly between writes
2. **Logic error**: Game code is reusing the same address for multiple characters
3. **Timing issue**: Writes are happening too fast and R0 isn't updated

## Next Steps

1. Examine the game code at PC 0xbaf, 0xbc5, 0xbcc to see what they're doing
2. Check if R0 is supposed to be incremented between writes
3. Look at the disassembly to understand the intended logic
4. Compare with working timer display code (if any exists earlier in the game)
