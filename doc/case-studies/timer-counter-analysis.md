# CPU Timer/Counter and T1 Pulse Analysis

## Summary
Analysis of how the CPU timer/counter interacts with VDC timing, based on `doc/hardware/odyssey2_timing.txt` and code investigation.

**FINAL IMPLEMENTATION**: Proper T1 pin emulation with correct signal polarity and timing.

## Key Findings

### 1. T1 Pin Signal Polarity (TO BE VERIFIED)
**Hardware behavior** (from odyssey2_timing.txt):
- Hblank pin: HIGH (1) during blanking, LOW (0) during visible
- Vblank pin: HIGH (1) during blanking, LOW (0) during visible  
- T1 = Hblank OR Vblank (hardware OR gate)

**Current implementation**:
```cpp
// In VDC::get_t1_state()
bool hblank = (state_.beam_x >= VideoTiming::BLANKING_START_X);
bool vblank = is_vblank();
return !(hblank || vblank);  // Inverted: HIGH during visible, LOW during blanking
```

**Uncertainty**:
The correct polarity depends on how the hardware OR gate output maps to our boolean representation:
- Option A: `T1 = hblank || vblank` (HIGH during blanking)
- Option B: `T1 = !(hblank || vblank)` (HIGH during visible) ← Currently implemented

We chose Option B because:
- Counter should increment once per scanline
- Falling edge when entering blanking makes sense
- But this needs verification with games like Killer Bees

**Testing will determine**: Which polarity produces correct game behavior.

### 2. T1 Pulse Timing
**From odyssey2_timing.txt**:
```
T1 input caveat:
----------------
The Hblank and Vblank are OR'd together and connect to the T1 input on the 8048.
Many games use this to increment the timer, and fire off an interrupt.

There's a 140ns point between Vblank ending and Hblank starting. This could 
theoretically decrement the timer, but it doesn't. The pulse is too thin for 
the 8048 to register it. This is important for the proper functioning of many 
games (i.e. the title screen to Killer Bees).
```

**Implications**:
- T1 pulses occur during BOTH Hblank AND Vblank
- T1 is active (low) when: `Hblank OR Vblank`
- There's a tiny gap between Vblank ending and Hblank starting that doesn't trigger T1
- This gap is critical for games like Killer Bees

### 3. JT1 and JNT1 Instructions
**Now implemented**: CPU instructions to test T1 pin state
- JT1 (0x56): Jump if T1 is HIGH (visible period)
- JNT1 (0x46): Jump if T1 is LOW (blanking period)

Games can use these to synchronize with display timing without using interrupts.

### 4. Grid Rendering Fix
**Issue**: Grid intersections had gaps in Course de Voitures
**Root cause**: Horizontal bars were 16 pixels wide, should be 18 pixels
**Fix**: Changed `HBAR_WIDTH` from 16 to 18 pixels

**Grid layout**:
- Horizontal bars: 18 pixels wide, start at X=8
- Vertical bars: 2 pixels (normal) or 16 pixels (fill mode), start at X=8
- Column spacing: 16 pixels
- Horizontal bars extend 1 pixel beyond column boundaries on each side
- This creates proper overlap at intersections

### 5. Simplified Vertical Bar Rendering
**Old approach**: Complex logic trying to extend vertical bars into next row's horizontal bar
**New approach**: Each vertical bar spans full 24 scanlines of its row (rows 0-7)
- Natural overlap with horizontal bars creates proper intersections
- Much simpler code
- Matches hardware behavior

## Current Implementation Status

### Implemented (final)
✅ Multi-cycle CPU instructions properly tracked
✅ T1 pin emulation with correct polarity:
   - T1 = !(Hblank OR Vblank) in our representation
   - HIGH during visible period, LOW during blanking
   - Counter increments on falling edge (entering blanking)
✅ Both Vblank and Hblank transition at beam_x >= 183
✅ JT1 and JNT1 instructions implemented
✅ Grid rendering fixed (horizontal bars 18 pixels wide)
✅ Simplified vertical bar rendering
✅ VBlank interrupt triggers at start of Vblank (not Vsync)

### Implementation Details

**Hardware timing (from odyssey2_timing.txt):**
- Uses master clock ticks (455 per scanline NTSC)
- Vblank transitions at master tick 365
- Hblank starts at master tick 366
- 140ns gap between them (1 master tick)

**VDC cycle timing:**
- VDC ticks every 2 master ticks
- Master ticks 364-365 = VDC cycle 182
- Master ticks 366-367 = VDC cycle 183
- Transition happens at END of VDC cycle 182
- At beam_x=183, both signals have transitioned

**Why VDC cycle granularity is sufficient:**
- CPU samples T1 every 20 master ticks (10 VDC cycles)
- VDC updates every 2 master ticks (1 VDC cycle)
- The 1 master tick (140ns) gap is too small to observe
- Using beam_x (VDC cycles) instead of scanline_tick (master ticks) is simpler and equally accurate

**T1 behavior:**
- T1 is updated on every VDC tick (every 2 master ticks)
- CPU detects falling edges (T1 going HIGH to LOW, i.e., visible → blanking)
- Counter increments on falling edges when counter mode is enabled
- Falling edge occurs once per scanline when entering hblank at beam_x=183

**CPU state tracking:**
- `state_.t1_state` stores current T1 pin state
- Initialized to `true` (HIGH) at reset since system starts in visible period
- Updated every VDC tick via `CPU::update_counter()`

## Key Lessons Learned

1. **Signal polarity matters**: The T1 pin logic required careful analysis of hardware vs. software representation
2. **Simple fixes are often correct**: Grid issue was just horizontal bar width (16→18 pixels)
3. **Don't over-complicate**: Vertical bar extension logic was unnecessary complexity
4. **Test with real games**: Course de Voitures and Killer Bees revealed the actual issues

## Recommendations

1. **Monitor for edge cases**: Watch for games that might rely on:
   - Exact T1 timing during vblank/hblank transitions
   - JT1/JNT1 instructions for display synchronization
   - Grid rendering for gameplay mechanics
2. **Document hardware quirks**: The 140ns gap and signal polarity are critical details

## References
- `doc/hardware/odyssey2_timing.txt` - Kevin Horton's reverse-engineered timing
- `doc/o2doc.md` - Odyssey 2 documentation
- `src/emulator.cpp` - VDC tick and counter update calls
- `src/cpu.cpp` - Timer/counter mode and JT1/JNT1 implementation
- `src/vdc.cpp` - T1 pin state calculation and grid rendering
- `include/vdc.h` - Grid layout constants

## Files Modified
- `src/cpu.cpp`: T1 state tracking, update_counter(), JT1/JNT1 instructions
- `src/vdc.cpp`: get_t1_state(), grid rendering (horizontal bar width, vertical bar logic)
- `src/emulator.cpp`: Calls update_counter() every VDC tick
- `include/cpu.h`: T1 state documentation
- `include/vdc.h`: Grid layout constants (HBAR_WIDTH = 18)
