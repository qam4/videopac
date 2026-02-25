# True Master Clock Timing - Requirements

## Overview

Migrate from VDC-cycle-based timing to true hardware master clock timing for improved accuracy. This eliminates fractional cycle handling and naturally produces the 227.5 VDC cycles per scanline behavior required for NTSC color phase shift.

## Current Implementation Issues

1. **VDC-cycle base unit**: We count VDC cycles (3.579545 MHz) instead of master clock ticks (7.15909 MHz)
2. **Integer approximation**: Using 227 cycles/scanline instead of 227.5 causes timing drift
3. **Manual ratio tracking**: 10:1 VDC-to-CPU ratio requires debt tracking
4. **No NTSC phase shift**: Can't naturally produce the 0.5-cycle offset for color subcarrier

## Goals

1. Use true master clock as base unit (7.15909 MHz NTSC, 17.734476 MHz PAL)
2. Eliminate fractional cycle handling - let it emerge naturally from integer math
3. Simplify CPU/VDC coordination - both derive from master clock
4. Support accurate NTSC color phase shift (227.5 cycles/line)
5. Maintain cycle-accurate CPU/VDC interleaving

## Hardware Specifications

### NTSC (Intel 8244)
- Master clock: 7.15909 MHz
- Scanline period: 455 master ticks (exact)
- VDC clock: Master ÷ 2 = 3.579545 MHz (227.5 cycles/line)
- CPU instruction: Master ÷ 20 = 0.357954 MHz (22.75 instructions/line)
- Frame: 262 scanlines = 119,210 master ticks

### PAL (Intel 8245)
- Master clock: 17.734476 MHz
- Scanline period: 1,135 master ticks (exact)
- VDC clock: Master ÷ 5 = 3.546895 MHz (227 cycles/line)
- CPU instruction: Master ÷ 45 = 0.394099 MHz (25.22 instructions/line)
- Frame: 312 scanlines = 354,120 master ticks (or 313 = 355,255)

## Requirements

### R1: Master Clock Counter
- System shall use master clock ticks as the base time unit
- Counter shall increment by 1 for each master clock tick
- NTSC: 455 ticks per scanline
- PAL: 1,135 ticks per scanline

### R2: VDC Tick Derivation
- VDC shall tick every N master ticks (N=2 for NTSC, N=5 for PAL)
- No manual alternation logic required
- 227.5 cycles/line emerges naturally from 455 ÷ 2

### R3: CPU Instruction Derivation
- CPU shall execute every M master ticks (M=20 for NTSC, M=45 for PAL)
- No debt tracking required
- Fractional instructions/line handled naturally

### R4: Scanline Boundaries
- Scanline increments when master_tick reaches scanline period
- NTSC: master_tick >= 455 → scanline++, master_tick = 0
- PAL: master_tick >= 1135 → scanline++, master_tick = 0

### R5: Frame Boundaries
- Frame increments when scanline reaches total scanlines
- NTSC: scanline >= 262 → frame++, scanline = 0
- PAL: scanline >= 312 (or 313) → frame++, scanline = 0

### R6: Backward Compatibility
- Existing save states should remain compatible
- VDC state (beam_x, beam_y, total_cycles) should map correctly
- Tests should continue to pass with updated timing

### R7: Performance
- Master clock approach should not significantly impact performance
- Target: <5% performance regression vs current implementation

## Non-Goals

- Simulating actual NTSC/PAL RF encoding (color subcarrier generation)
- Cycle-accurate bus contention (8048/VDC bus timing)
- Sub-master-tick precision (e.g., analog signal timing)

## Success Criteria

1. All existing tests pass with new timing system
2. Games run correctly on both NTSC and PAL
3. Frame timing matches hardware specifications exactly
4. No timing drift over extended play sessions
5. Code is simpler (no debt tracking, no alternation logic)

## References

- doc/case-studies/killer-bees-banking.md - Complete timing documentation
- include/master_clock.h - Current MasterClock implementation
- src/master_clock.cpp - Current timing logic
- doc/hardware/8245.md - Intel 8245 VDC specifications
