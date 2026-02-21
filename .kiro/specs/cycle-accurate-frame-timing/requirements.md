# Cycle-Accurate Frame Timing - Requirements

## Overview
The emulator currently has a cycle debt accumulation issue that causes frame-to-frame timing variations. This manifests as visual jitter in games that use raster effects (mid-frame register changes), such as the road scrolling in Course de Voitures.

## Problem Statement
The MasterClock maintains CPU and VDC cycle debt to ensure accurate timing between components. However, this debt carries over between frames, causing the CPU to execute at slightly different relative positions within each frame. This creates 1-scanline jitter in raster effects.

**Evidence:**
- Frame 6: CPU debt: 8.45098
- Frame 7: CPU debt: 8.8781  
- Frame 8: CPU debt: 8.30522
- Frame 9: CPU debt: 9.73234

This varying debt causes grid toggle operations to occur at different scanlines (Y=70 vs Y=71), creating visual scrolling artifacts.

## User Stories

### Story 1: Consistent Raster Effects
**As a** player  
**I want** raster effects to display consistently without jitter  
**So that** games like Course de Voitures display correctly without visual artifacts

**Acceptance Criteria:**
1. GIVEN a game uses raster effects (mid-frame register changes)
2. WHEN the game is running with speed=0 (stationary)
3. THEN the raster effect positions SHALL remain constant across frames
4. AND there SHALL be no visible jitter or scrolling artifacts

### Story 2: Frame Boundary Synchronization
**As an** emulator developer  
**I want** cycle debt to reset at frame boundaries  
**So that** each frame starts with consistent timing

**Acceptance Criteria:**
1. GIVEN the emulator is running
2. WHEN a frame completes
3. THEN the CPU cycle debt SHALL be reset to a consistent starting value
4. AND the VDC cycle debt SHALL be reset to a consistent starting value
5. AND the next frame SHALL start with predictable timing

### Story 3: Deterministic Execution
**As an** emulator developer  
**I want** identical input to produce identical output  
**So that** debugging and testing are reliable

**Acceptance Criteria:**
1. GIVEN two emulator runs with identical input
2. WHEN both runs execute the same number of frames
3. THEN the CPU state SHALL be identical at each frame boundary
4. AND the VDC state SHALL be identical at each frame boundary
5. AND the cycle debt SHALL be identical at each frame boundary

## Requirements

### Requirement 1: Frame Boundary Cycle Debt Reset
THE MasterClock SHALL reset cycle debt to consistent values at frame boundaries

**Acceptance Criteria:**
1.1 WHEN `reset_frame()` is called, THE CPU cycle debt SHALL be set to a fixed starting value  
1.2 WHEN `reset_frame()` is called, THE VDC cycle debt SHALL be set to a fixed starting value  
1.3 THE starting values SHALL ensure the CPU executes at the same relative position in each frame  
1.4 THE starting values SHALL maintain overall timing accuracy across multiple frames

### Requirement 2: Raster Effect Stability
THE emulator SHALL execute raster effects at consistent scanline positions when game state is unchanged

**Acceptance Criteria:**
2.1 GIVEN a game with raster effects and speed=0  
2.2 WHEN the game executes for multiple frames  
2.3 THEN VDC register writes SHALL occur at the same scanline positions each frame  
2.4 AND the beam Y position at each register write SHALL not vary by more than 0 scanlines

### Requirement 3: Timing Accuracy Preservation
THE cycle debt reset SHALL maintain long-term timing accuracy

**Acceptance Criteria:**
3.1 GIVEN the emulator runs for 1000 frames  
3.2 WHEN measuring total cycles executed  
3.3 THEN the total SHALL match expected cycles within 0.1% (1000 cycles for 1M total)  
3.4 AND there SHALL be no drift in frame timing over extended periods

### Requirement 4: Backward Compatibility
THE cycle debt changes SHALL not break existing functionality

**Acceptance Criteria:**
4.1 GIVEN the existing test suite  
4.2 WHEN all tests are run  
4.3 THEN all existing tests SHALL pass  
4.4 AND no regressions SHALL be introduced in game compatibility

## Technical Constraints

1. **Cycle Ratio**: CPU executes at 1/9.9 the speed of VDC (9.9 VDC cycles per CPU instruction)
2. **Frame Cycles**: 
   - NTSC: 59,474 cycles/frame (262 scanlines × 227 cycles)
   - PAL: 70,824 cycles/frame (312 scanlines × 227 cycles)
3. **Debt Precision**: Cycle debt uses floating-point for sub-cycle accuracy
4. **Performance**: Changes must not significantly impact emulation speed

## Out of Scope

- Changing the fundamental cycle-accurate execution model
- Modifying the CPU/VDC cycle ratio
- Implementing scanline-based execution (remains cycle-based)
- Adding VSync or display synchronization (separate concern)

## Success Metrics

1. **Jitter Elimination**: Grid toggle Y positions in Course de Voitures remain constant (±0 scanlines)
2. **Determinism**: 10 consecutive runs produce identical cycle debt values at each frame
3. **Accuracy**: Total cycle count over 1000 frames matches expected within 0.1%
4. **Performance**: Frame execution time increases by less than 5%

## References

- Case Study: `doc/case-studies/road-movement-bug.md`
- Master Clock Implementation: `src/master_clock.cpp`
- Emulator Core: `src/emulator.cpp`
- Test Suite: `tests/test_master_clock.cpp`
