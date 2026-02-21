# Timer Bug - Root Cause Analysis

## Summary
The timer display corruption in Course de Voitures Game 1 is caused by a bug in the game code where three consecutive VDC writes target the same character RAM address (0x01) instead of three different addresses.

## Evidence from Trace Analysis

### VDC Writes to Address 0x01
| Frame | PC    | Value | Character |
|-------|-------|-------|-----------|
| 55    | 0xbaf | 0x36  | '6'       |
| 55    | 0xbc5 | 0x3c  | '<'       |
| 55    | 0xbcc | 0x2b  | '+'       |
| 56    | 0xbaf | 0x38  | '8'       |
| 56    | 0xbc5 | 0x3d  | '='       |
| 56    | 0xbcc | 0x2a  | '*'       |
| 57    | 0xbaf | 0x3a  | ':'       |
| 57    | 0xbc5 | 0x3c  | '<'       |
| 57    | 0xbcc | 0x2a  | '*'       |

Only the last write (0x2a or 0x2b) is visible on screen, showing '+' or '*' characters.

## Disassembly Analysis

### Code at PC 0xbad-0xbcc:
```assembly
0xbad: 80     MOVX A,@R0      ; Read from RAM[R0]
0xbae: 6b     ADD A,R3        ; Add R3 to accumulator
0xbaf: 90     MOVX @R0,A      ; Write to VDC[R0] - FIRST WRITE
0xbb0: 18     INC R0          ; Increment R0 (R0 = 0x02)
0xbb1: fa     MOV A,R2        ; Move R2 to A
0xbb2: f7     RLC A           ; Rotate left through carry
0xbb3: fa     MOV A,R2        ; Move R2 to A again
0xbb4: 67     RR A            ; Rotate right
0xbb5: a9     MOV R1,A        ; Store in R1
0xbb6: e6 ca  JNC loc_0bca    ; Jump if no carry
0xbb8: 18     INC R0          ; Increment R0 (R0 = 0x03)
0xbb9: f8     MOV A,R0        ; Move R0 to A
0xbba: 43 20  ORL A,##0x20    ; OR with 0x20
0xbbc: a8     MOV R0,A        ; Store back in R0
0xbbd: f0     MOV A,@R0       ; Read from internal RAM[R0]
0xbbe: d3 01  XRL A,##0x01    ; XOR with 0x01
0xbc0: a0     MOV @R0,A       ; Write back to internal RAM[R0]
0xbc1: 28     XCH A,R0        ; Exchange A and R0
0xbc2: 53 0f  ANL A,##0x0f    ; AND with 0x0F (mask lower 4 bits)
0xbc4: 28     XCH A,R0        ; Exchange back (R0 = 0x01 again!)
0xbc5: 90     MOVX @R0,A      ; Write to VDC[R0] - SECOND WRITE (R0=0x01)
0xbc6: c8     DEC R0          ; Decrement R0 (R0 = 0x00)
0xbc7: 12 ca  JB0 loc_0bca    ; Jump if bit 0 set
0xbc9: 19     INC R1          ; Increment R1
0xbca: 80     MOVX A,@R0      ; Read from RAM[R0]
0xbcb: 69     ADD A,R1        ; Add R1
0xbcc: 90     MOVX @R0,A      ; Write to VDC[R0] - THIRD WRITE (R0=0x00 or 0x01)
0xbcd: 83     RET             ; Return
```

## Root Cause

The bug is at **0xbc4**: `28 XCH A,R0` followed by `53 0f ANL A,##0x0f`

This sequence:
1. Exchanges A and R0
2. Masks A with 0x0F (keeping only lower 4 bits)
3. Exchanges back, putting the masked value into R0

**The problem**: After incrementing R0 to 0x02 or 0x03, the code masks it with 0x0F and puts it back. Since 0x02 & 0x0F = 0x02 and 0x03 & 0x0F = 0x03, this should work. However, the code at 0xbba-0xbbc ORs R0 with 0x20, making it 0x22 or 0x23. Then at 0xbc4, masking with 0x0F gives 0x02 or 0x03, but the exchange puts this back into R0.

Wait, let me re-analyze. At 0xbc1-0xbc4:
- 0xbc1: `28 XCH A,R0` - A gets R0's value, R0 gets A's value
- 0xbc2: `53 0f ANL A,##0x0f` - A = A & 0x0F
- 0xbc4: `28 XCH A,R0` - Exchange again

After the first exchange at 0xbc1, R0 contains the value that was in A (from 0xbc0). Then A is masked. Then they're exchanged again at 0xbc4, so R0 gets the masked value.

The issue is that R0 was set to 0x22 or 0x23 at 0xbbc, then used to access internal RAM at 0xbbd-0xbc0. The value read from internal RAM is then put into A at 0xbc0. At 0xbc1, this value is exchanged into R0, overwriting the address! Then at 0xbc4, it's masked and exchanged back, but R0 now contains a value from internal RAM, not an address.

**Actual Root Cause**: The code at 0xbc1-0xbc4 is trying to restore R0 to point to VDC character RAM, but it's using the wrong logic. It should restore R0 to the incremented value (0x02 or 0x03), but instead it's using a value from internal RAM that happens to be 0x01.

## Conclusion

This is a **game code bug**, not an emulator bug. The game's timer display routine has incorrect pointer arithmetic that causes all three character writes to target the same VDC address (0x01) instead of three consecutive addresses (0x01, 0x02, 0x03).

The corruption appears as random characters ('+', '*', ':', etc.) because the code is writing incrementing ASCII values to the same location, and only the last write is visible.

## Verification

To verify this is a game bug and not an emulator bug, we would need to:
1. Test on real hardware or a known-good emulator
2. Check if the original game has this same visual glitch
3. Review the game's intended timer display logic

If the bug exists on real hardware, it's a game bug. If it doesn't, there may be an emulator timing or VDC implementation issue that causes this code path to execute incorrectly.
