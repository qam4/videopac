# Master Clock Timing Audit - Requirements

## VERIFIED: Clock Architecture

**Source**: Wikipedia and technical documentation analysis

**NTSC (Magnavox Odyssey²):**
- **Master Clock**: 7.15909 MHz
- **VDC (Intel 8244)**: 7.15909 MHz / 2 = **3.579545 MHz** (NTSC color subcarrier)
- **CPU (Intel 8048) Crystal Input**: 7.15909 MHz × 0.75 = **5.369317 MHz**
- **CPU Internal State Clock**: 5.369317 MHz / 3 = **1.789772 MHz** (this is what sources mean by "1.79 MHz")
- **CPU Instruction Rate**: 1.789772 MHz / 5 = **0.357954 MHz**
- **VDC/CPU Ratio**: 3.579545 / 0.357954 = **10.0** (exactly!)

**PAL (Philips Videopac G7000):**
- **Master Clock**: 17.734476 MHz (4× PAL color subcarrier 4.433619 MHz)
- **VDC (Intel 8245)**: 17.734476 MHz / 5 = **3.546895 MHz**
- **CPU (Intel 8048) Crystal Input**: 17.734476 MHz / 3 = **5.911492 MHz**
- **CPU Internal State Clock**: 5.911492 MHz / 3 = **1.970497 MHz**
- **CPU Instruction Rate**: 1.970497 MHz / 5 = **0.394099 MHz**
- **VDC/CPU Ratio**: 3.546895 / 0.394099 = **9.0** (exactly!)

**Key Insight**: The confusion in documentation comes from the 8048's internal /3 clock divider. When sources say "1.79 MHz", they refer to the internal state clock, not the crystal input frequency.

**Current Implementation Issues:**
- Uses 3.54 MHz for VDC (should be 3.579545 for NTSC, 3.546895 for PAL)
- Uses 1.79 / 5 = 0.358 MHz for CPU (should be 0.357954 for NTSC, 0.394099 for PAL)
- VDC/CPU ratio is 9.888 (should be 10.0 for NTSC, 9.0 for PAL)
- This 1.1% error in NTSC and 9.9% error in PAL could cause timing drift

**Impact Analysis:**
- **NTSC**: 1.1% too slow - games run slightly slower than real hardware
- **PAL**: 9.9% too slow - games run significantly slower, audio pitch is wrong
- Over 1 hour of gameplay: NTSC drifts ~40 seconds, PAL drifts ~6 minutes
- This could explain timing-sensitive bugs like the road movement issue

## Problem Statement

The emulator's master clock timing system needs verification to ensure cycle-accurate emulation. There are concerns about:

1. **Clock frequency accuracy**: Are we using the correct VDC and CPU clock frequencies?
2. **Cycles per frame**: Do we execute the exact number of VDC/CPU cycles per frame?
3. **Frame rate accuracy**: Do we achieve exactly 50 Hz (PAL) or 60 Hz (NTSC)?
4. **Debt accumulation**: Is the cycle debt system correctly tracking CPU/VDC synchronization?
5. **Frame boundary handling**: Are we properly resetting debt at frame boundaries?

These timing issues could manifest as:
- Games running too fast or too slow
- Audio pitch being incorrect
- Raster effects not aligning properly
- Input timing issues
- Animation speed problems

## User Stories

### US-1: Accurate Clock Frequencies
**As a** developer  
**I want** the emulator to use hardware-accurate clock frequencies  
**So that** games run at the correct speed and audio pitch is accurate

**Acceptance Criteria:**
- VDC clock frequency matches hardware specification (3.579545 MHz for NTSC, 3.54 MHz for PAL)
- CPU clock frequency matches hardware specification (1.79 MHz crystal / 5 = 0.358 MHz instruction rate)
- Clock ratio between VDC and CPU is correctly calculated (~9.888:1 for NTSC, ~9.888:1 for PAL)

### US-2: Correct Cycles Per Frame
**As a** developer  
**I want** each frame to execute the exact number of VDC cycles as real hardware  
**So that** frame-based timing is accurate

**Acceptance Criteria:**
- NTSC executes correct number of VDC cycles per frame
- PAL executes correct number of VDC cycles per frame
- Cycles per scanline matches hardware specification
- Total scanlines per frame matches hardware specification

### US-3: Accurate Frame Rate
**As a** developer  
**I want** the emulator to maintain exactly 50 Hz (PAL) or 60 Hz (NTSC)  
**So that** games run at the correct speed

**Acceptance Criteria:**
- NTSC maintains 59.94 Hz (or 60 Hz if using simplified timing)
- PAL maintains 50 Hz
- Frame rate is consistent over time (no drift)
- Frontend frame pacing matches emulator core frame rate

### US-4: Correct Cycle Debt Tracking
**As a** developer  
**I want** the cycle debt system to accurately track CPU/VDC synchronization  
**So that** both components execute in proper proportion

**Acceptance Criteria:**
- CPU executes approximately 1 instruction per 9.9 VDC cycles
- Debt accumulation is mathematically correct
- Debt never grows unbounded
- CPU and VDC stay synchronized over long periods

### US-5: Proper Frame Boundary Handling
**As a** developer  
**I want** cycle debt to be handled correctly at frame boundaries  
**So that** there's no timing jitter or drift between frames

**Acceptance Criteria:**
- Debt is reset or carried over correctly at frame boundaries
- No accumulation of timing errors across frames
- Raster effects align consistently frame-to-frame
- No visible jitter in animations

## Hardware Specifications

### Clock Frequencies

**NTSC (Intel 8244):**
- VDC Clock: 3.579545 MHz (NTSC color subcarrier frequency)
- CPU Crystal: 1.79 MHz (independent oscillator)
- CPU Instruction Rate: 1.79 MHz / 5 = 0.358 MHz
- VDC/CPU Ratio: 3.579545 / 0.358 = 9.998 (~10:1)

**PAL (Intel 8245):**
- VDC Clock: 3.54 MHz (approximate, varies by region)
- CPU Crystal: 1.79 MHz (independent oscillator)  
- CPU Instruction Rate: 1.79 MHz / 5 = 0.358 MHz
- VDC/CPU Ratio: 3.54 / 0.358 = 9.888 (~10:1)

**Important**: The CPU and VDC have **independent crystal oscillators**. They are NOT derived from the same clock source. The ratio is approximately 10:1 but not exactly.

### Frame Timing

**NTSC:**
- Frame Rate: 59.94 Hz (or 60 Hz simplified)
- Scanlines per Frame: 262
- Cycles per Scanline: ~227 (needs verification)
- Total VDC Cycles per Frame: 262 × 227 = 59,474
- Total CPU Cycles per Frame: 59,474 / 5 = 11,894.8

**PAL:**
- Frame Rate: 50 Hz
- Scanlines per Frame: 312
- Cycles per Scanline: ~227 (needs verification)
- Total VDC Cycles per Frame: 312 × 227 = 70,824
- Total CPU Cycles per Frame: 70,824 / 5 = 14,164.8

### Questions to Answer

1. **Is 227 cycles per scanline correct for both NTSC and PAL?**
   - NTSC: 3.579545 MHz / 59.94 Hz / 262 scanlines = 227.5 cycles/scanline
   - PAL: 3.54 MHz / 50 Hz / 312 scanlines = 227.0 cycles/scanline
   - Current implementation uses 227 for both - is this accurate enough?

2. **Should we use 59.94 Hz or 60 Hz for NTSC?**
   - Real NTSC is 59.94 Hz (color subcarrier / 1.001)
   - Many emulators simplify to 60 Hz
   - What's the impact on timing accuracy?

3. **What's the correct PAL VDC clock frequency?**
   - 3.54 MHz (current implementation)
   - 3.547 MHz (some sources)
   - 3.58 MHz (other sources)
   - Need to verify against hardware documentation

4. **Is the cycle debt system mathematically correct?**
   - Current ratio: VDC_CLOCK_MHZ / CPU_CLOCK_MHZ = 3.54 / (1.79/5) = 9.888
   - This matches the hardware specification (independent clocks)
   - Need to verify the debt accumulation math is correct

5. **Should debt be reset at frame boundaries?**
   - Current implementation resets debt to zero
   - Alternative: carry over fractional debt to next frame
   - Which approach is more accurate for independent clocks?

## Success Criteria

1. All clock frequencies match hardware specifications within acceptable tolerance
2. Cycles per frame calculations are verified against hardware documentation
3. Frame rate is stable and matches target (50 Hz or 60 Hz)
4. Cycle debt system is mathematically proven correct
5. No timing drift over extended periods (hours of emulation)
6. Existing games continue to work correctly (no regressions)
7. All existing tests pass
8. New tests added to verify timing accuracy

## Out of Scope

- Audio synchronization (separate concern)
- Frontend-specific frame pacing (SDL vsync, etc.)
- Save state timing (handled separately)
- Debugger timing (may run slower intentionally)

## References

- Intel 8244/8245 datasheets
- doc/o2doc.md - Videopac/Odyssey 2 documentation
- doc/8245.md - VDC register documentation
- .kiro/specs/cycle-accurate-frame-timing/ - Previous timing work
- tests/test_master_clock.cpp - Existing timing tests
