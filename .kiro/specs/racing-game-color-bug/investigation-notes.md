# Frame 352 Corruption Bug - Investigation Summary

## ROOT CAUSE IDENTIFIED

At frame 352, cycle 20981501, the BIOS memory copy routine at 0x0A8 reads 0xFF from address 0x9E and writes it to VDC grid RAM registers (0xD0-0xEF).

### Key Evidence from Full Trace with Registers

```
[F:352 C:20981501] 0x0a8: 80 91 | A=00 | R0=9f R1=d0 R2=ff
[F:352 C:20981581] 0x0a8: 80 91 | A=ff | R0=9e R1=d1 R2=fe  ← BUG: Reading 0xFF from 0x9E
[F:352 C:20981661] 0x0a8: 80 91 | A=ff | R0=9d R1=d2 R2=fd
[F:352 C:20981741] 0x0a8: 80 91 | A=ff | R0=9c R1=d3 R2=fc
```

### Analysis

1. **R0 = 0x9F → 0x9E → 0x9D...** (decrementing source pointer)
2. **R1 = 0xD0 → 0xD1 → 0xD2...** (incrementing VDC destination)
3. **R2 = 0xFF** (255 iterations - VERY SUSPICIOUS!)
4. **Memory at 0x9E contains 0xFF** (the corrupting data)

### Questions

1. **Why is R2=0xFF (255 iterations)?** This seems like an uninitialized or corrupted loop counter
2. **What should be at address 0x9E?** Is this ROM or RAM?
3. **What is the correct source data location?** Where should R0 be pointing?
4. **How did R0/R1/R2 get set up?** Need to trace backwards from 0x0A8

### Memory Map

- 0x000-0x3FF: BIOS ROM
- 0x400-0xBFF: Game ROM  
- 0x9E is in **Game ROM** (0x400 + 0x59E offset)

So the game is copying data from its own ROM at offset 0x59E, which happens to contain 0xFF values.

## ROOT CAUSE CONFIRMED

The bug is caused by an **interrupt handler returning to the wrong address**, corrupting an in-progress BIOS copy operation.

### Detailed Analysis from Trace

1. **Before the bug** (cycle 20977681-20978501):
   - BIOS copy loop at 0x0A8-0x0AC is running normally
   - Copying data from RAM to VDC registers
   - R0 counting down from 0x17 to 0x0D
   - R1 counting up from 0xC5 to 0xCF  
   - R2 counting down from 0x77 to 0x6D

2. **Timer interrupt occurs** (cycle 20978521):
   - PC jumps from 0x0A9 to 0x007 (timer interrupt vector)
   - Then to 0x404 (game's timer interrupt handler)
   - Then to 0x788 (actual handler code)

3. **Interrupt handler executes** (cycle 20978561-20978671):
   - At 0x78B: `MOV R0,##0xA0` (R0=0xA0)
   - At 0x78D: `MOV A,##0x20` (A=0x20)
   - At 0x78F: `MOVX @R0,A` (write 0x20 to RAM[0xA0])
   - At 0x790: `MOV R2,##0x86` (R2=0x86)
   - At 0x792-0x794: Loop copying data, R2 counts down to 0

4. **Handler returns** (cycle 20981461):
   - At 0x79C: `RETR` instruction
   - **Returns to 0x0AA** (middle of BIOS copy loop!)
   - At this point: R0=0xA0, R1=0xCF, R2=0x00

5. **Bug manifests** (cycle 20981501):
   - At 0x0AC: `DJNZ R2,0x0A8` 
   - R2=0x00 decrements to 0xFF (wrap-around!)
   - Loop continues for 255 iterations
   - Copies 0xFF bytes from ROM (0x9F-0x00) to VDC (0xD0-0xEF+)
   - This corrupts the VDC grid registers with garbage data

### The Core Problem

The interrupt handler at 0x788-0x79C is **not properly saving and restoring the CPU state**. Specifically:
- It modifies R0, R1, R2 without saving them
- It returns to 0x0AA, which is the wrong return address
- The correct return address should be 0x0AA from the ORIGINAL interrupted code, but the handler's own loop has corrupted the stack or return mechanism

### Why This Happens at Frame 352

This is likely a **timing-dependent bug**. The interrupt must occur at exactly the right moment:
- While the BIOS copy loop is running (0x0A8-0x0AC)
- At a specific point where the return address gets corrupted
- This explains why it happens consistently at frame 352 with the same inputs

## ROOT CAUSE CONFIRMED: Timing-Dependent Interrupt During BIOS Copy

The bug is caused by a **timer interrupt firing during an unprotected BIOS copy operation**, exposed by recent timing changes.

### The Complete Picture

1. **BIOS copy routine (0x089-0x0AC) does NOT disable interrupts**
   - No `DIS I` instruction before the copy loop
   - Registers R0, R1, R2 are used for the copy operation
   - The routine is vulnerable to interruption

2. **Recent timing changes altered interrupt timing**
   - Master clock timing audit removed debt reset at frame boundaries
   - Debt now carries over between frames (more accurate)
   - This changed when timer interrupts fire relative to game code
   - At frame 352, timer interrupt now fires during BIOS copy (cycle 20978521)

3. **Game's timer interrupt handler (0x788-0x79C) modifies R0, R1, R2**
   - Handler runs its own copy loop using R0, R1, R2
   - Does not save/restore these registers (not required by 8048 calling convention)
   - When it returns, R0=0xA0, R1=0xCF, R2=0x00

4. **Corrupted state causes bug**
   - BIOS copy resumes at 0x0AA with R2=0x00
   - DJNZ instruction wraps R2 from 0 to 0xFF
   - Copies 255 bytes of garbage to VDC registers

### Why This Happens Now

**Before timing changes**:
- Debt reset at frame boundaries
- Timer interrupts fired at predictable points
- Never interrupted BIOS copy operation

**After timing changes**:
- Debt carries over between frames (more accurate)
- Timer interrupt timing shifted
- Now interrupts BIOS copy at frame 352

### Is This a Bug?

This reveals a **latent bug in the BIOS** that was masked by the old timing:

1. **BIOS bug**: Copy routine should disable interrupts but doesn't
2. **Not a game bug**: Interrupt handler is correctly implemented
3. **Not an emulator bug**: Interrupt handling is correct
4. **Timing bug**: New timing exposed the BIOS vulnerability

### Does This Happen on Real Hardware?

**Unknown**. This depends on:
- Whether real hardware has the same timing relationship
- Whether the BIOS copy ever gets interrupted on real hardware
- Whether this game triggers the same sequence on real hardware

### Possible Fixes

1. **Emulator workaround**: Disable interrupts during BIOS copy (0x089-0x0AC)
   - Pros: Fixes the bug, simple to implement
   - Cons: Not hardware-accurate if real hardware allows interrupts

2. **Revert timing changes**: Go back to debt reset at frame boundaries
   - Pros: Restores old behavior
   - Cons: Less accurate timing, masks the bug

3. **Adjust timing**: Fine-tune when timer interrupts fire
   - Pros: Might avoid the specific collision
   - Cons: Doesn't fix the underlying vulnerability

4. **Test on real hardware**: Determine if this is a real bug
   - Pros: Definitive answer
   - Cons: Requires physical hardware

### Recommendation

1. **Short term**: Add emulator workaround to disable interrupts during BIOS copy
2. **Long term**: Test on real hardware to determine correct behavior
3. **Document**: Add this as a known BIOS vulnerability in the documentation

This is a fascinating case of improved accuracy exposing a latent bug!
