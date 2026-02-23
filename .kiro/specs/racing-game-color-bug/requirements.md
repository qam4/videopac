# Requirements: BIOS Timer Interrupt Vulnerability Fix

## Overview

Fix display corruption bug in Course de Voitures at frame 352 caused by timer interrupt firing during unprotected BIOS memory copy operation.

## Background

### Root Cause
The French BIOS contains a memory copy routine (0x089-0x0AC) that does NOT disable interrupts before copying data. When a timer interrupt fires during this copy operation, the game's timer interrupt handler (0x788-0x79C) modifies registers R0, R1, R2 without saving them (which is legal per 8048 convention). When the handler returns, the BIOS copy resumes with corrupted register values, causing it to copy 255 bytes of garbage data to VDC registers.

### Timer Interrupt Mechanism

The 8048 CPU has a built-in timer/counter that operates independently of all other system components:

**Timer Initialization:**
- **Who starts it**: The game ROM (not BIOS) enables and starts the timer
- **Where**: At address 0x45E in the game ROM initialization code
- **Instructions**:
  - `EN TCNTI` (0x25) at 0x45E - Enable timer interrupts
  - Timer is started by the interrupt handler itself using `STRT CNT` (0x45) at 0x39A

**How the Timer Works:**
1. **Prescaler**: A prescaler accumulates instruction cycles. Every 32 instruction cycles, the prescaler overflows and increments the 8-bit timer register
2. **Timer Register**: An 8-bit counter (0x00-0xFF) that increments every 32 instruction cycles when the timer is running
3. **Overflow**: When the timer register overflows (0xFF → 0x00), it triggers an interrupt at vector 0x007 if timer interrupts are enabled
4. **Full Cycle**: The timer overflows every 8192 instruction cycles (256 × 32)
5. **Period**: With PAL timing (1.79 MHz CPU clock), this is approximately 4.58ms per overflow

**Timer Interrupt Handler (0x788-0x79C):**
- Stops the timer with `STOP TCNT` (0x65)
- Reads joystick input
- Restarts the timer with `STRT CNT` (0x45)
- Returns with `RETR`
- **Critical Issue**: The handler modifies R0, R1, R2 without saving them (legal per 8048 convention, but dangerous if it interrupts code using those registers)

**Timer Period and Frame Relationship:**
- **Timer period**: 8192 instruction cycles = ~4.58ms (at 1.79 MHz PAL CPU clock)
- **Frame period**: 20ms (50 Hz PAL) or 16.67ms (60 Hz NTSC)
- **Interrupts per frame**: 
  - PAL: 20ms ÷ 4.58ms ≈ **4.4 timer interrupts per frame**
  - NTSC: 16.67ms ÷ 4.58ms ≈ **3.6 timer interrupts per frame**
- **NOT synchronized to frames**: The timer runs independently based on instruction cycles, not VBlank or frame boundaries
- **Phase drift**: Since the timer period doesn't evenly divide into frame period, the interrupt occurs at different points within each frame
- **Why frame 352 matters**: After 352 frames, the accumulated phase drift causes the interrupt to occur during the vulnerable BIOS copy window

**Key Characteristics:**
- **Purely cycle-based**: Timer increments based on instruction cycles executed, NOT synchronized to:
  - VBlank (vertical blanking interval)
  - Frame boundaries
  - Scanlines or HBlank
  - VDC timing
  - Any external events
- **Asynchronous**: Can fire after ANY instruction if timer is running and interrupts are enabled
- **Deterministic**: Given the same starting conditions and instruction sequence, timer will overflow at the same cycle count
- **Self-restarting**: The interrupt handler restarts the timer, creating a periodic interrupt approximately every 8192 cycles

**Implementation Reference:**
- See: `src/cpu.cpp` lines 1220-1250 (timer logic in `execute_instruction()`)
- See: `doc/reference/Reading-8048-Series-Code.md` (timer interrupt overview)
- See: `doc/reference/mcs-48-user-manual.md` (detailed timer/counter specification)
- See: `course_de_voitures_full_disasm.txt` lines 945-970 (timer initialization)
- See: `course_de_voitures_full_disasm.txt` lines 778-810 (timer interrupt handler)

### Why It Happens Now (and Possibly Before)
Recent timing changes (master-clock-timing-audit spec) removed debt reset at frame boundaries, causing debt to carry over between frames. This changed when timer interrupts fire relative to game code. At frame 352, the timer now overflows during the BIOS copy operation.

**Important Note**: According to FRAME352_BUG_SUMMARY.md, "The bug existed before but timing changes made it reproducible." This suggests the vulnerability was always present in the BIOS, but the timing changes made it occur consistently at frame 352. Before the timing changes, the interrupt may have fired at different times, sometimes avoiding the vulnerable BIOS copy window.

### Evidence
- Full trace analysis shows timer interrupt at cycle 20978521 during BIOS copy
- Handler modifies R0=0xA0, R1=0xCF, R2=0x00
- DJNZ instruction wraps R2 from 0x00 to 0xFF
- Copies 255 bytes of 0xFF values to VDC grid registers (0xD0-0xEF)
- See: .kiro/specs/racing-game-color-bug/investigation-notes.md, FRAME352_BUG_SUMMARY.md

## User Stories

### 1. As a user, I want Course de Voitures to display correctly at frame 352
**Acceptance Criteria:**
- 1.1 Background remains correct color (not grey)
- 1.2 Sprites display correctly (not big squares)
- 1.3 No white grid appears on screen
- 1.4 Game remains playable after frame 352

### 2. As a developer, I want to understand if this bug exists on real hardware
**Acceptance Criteria:**
- 2.1 Document whether real Videopac hardware exhibits this bug
- 2.2 If unknown, document that physical testing is required
- 2.3 Provide rationale for chosen fix approach

### 3. As a developer, I want the emulator to handle BIOS vulnerabilities appropriately
**Acceptance Criteria:**
- 3.1 Fix approach is documented with clear rationale
- 3.2 Fix does not break other games
- 3.3 Fix can be toggled or adjusted if needed for accuracy testing

## Functional Requirements

### FR1: Fix Display Corruption
The emulator shall prevent display corruption at frame 352 in Course de Voitures.

### FR2: Maintain Timing Accuracy
The fix shall not revert the improved timing accuracy from master-clock-timing-audit spec.

### FR3: Preserve Interrupt Behavior
The fix shall preserve correct timer interrupt behavior for games that rely on it.

### FR4: Document Approach
The fix shall be clearly documented with rationale for the chosen approach.

## Non-Functional Requirements

### NFR1: Performance
The fix shall not measurably impact emulator performance.

### NFR2: Compatibility
The fix shall not break any other games in the test suite.

### NFR3: Maintainability
The fix shall be clearly commented and easy to understand.

## Fix Options

### Option 1: Emulator Workaround - Disable Interrupts During BIOS Copy
**Approach:** Detect when PC is in range 0x089-0x0AC and temporarily disable timer interrupts.

**Pros:**
- Fixes the bug immediately
- Simple to implement
- Preserves timing accuracy
- Protects against BIOS vulnerability

**Cons:**
- Not hardware-accurate if real hardware allows interrupts during BIOS copy
- Requires special-case code in emulator
- May mask other timing-dependent bugs

### Option 2: Revert Timing Changes
**Approach:** Restore debt reset at frame boundaries from before master-clock-timing-audit.

**Pros:**
- Restores old behavior
- No special-case code needed

**Cons:**
- Less accurate timing (defeats purpose of timing audit)
- Masks the bug rather than fixing it
- May cause other timing-dependent issues

### Option 3: Adjust Timer Timing
**Approach:** Fine-tune when timer interrupts fire to avoid the specific collision.

**Pros:**
- Might avoid this specific bug
- Preserves most timing accuracy

**Cons:**
- Doesn't fix the underlying vulnerability
- May cause bugs in other games
- Arbitrary adjustment without hardware verification

### Option 4: Test on Real Hardware First
**Approach:** Determine if this bug exists on real Videopac before implementing a fix.

**Pros:**
- Definitive answer on correct behavior
- Ensures fix matches hardware

**Cons:**
- Requires physical hardware and testing setup
- May take significant time
- Bug remains unfixed in the meantime

## Recommended Approach

**Option 1** (Emulator Workaround) is recommended for the following reasons:

1. **Immediate fix** - Resolves the bug now without waiting for hardware testing
2. **Preserves accuracy** - Maintains improved timing from master-clock-timing-audit
3. **Protects BIOS** - Prevents exploitation of BIOS vulnerability
4. **Reversible** - Can be removed or adjusted if hardware testing shows different behavior
5. **Documented** - Clear comments explain why the workaround exists

The workaround should be implemented with:
- Clear comments explaining the BIOS vulnerability
- Configuration option to disable the workaround for testing
- Logging when the workaround is triggered (debug mode only)

## Out of Scope

- Testing on real Videopac hardware (future work)
- Fixing other potential BIOS vulnerabilities
- Modifying game ROM code
- Implementing cycle-perfect timer interrupt timing

## Dependencies

- None (fix is self-contained in CPU emulation)

## Constraints

- Must not break existing games
- Must maintain timing accuracy from master-clock-timing-audit
- Must be clearly documented for future maintainers

## Success Metrics

- Course de Voitures displays correctly at frame 352
- All other games in test suite continue to work
- No measurable performance impact
- Code is well-documented and maintainable
