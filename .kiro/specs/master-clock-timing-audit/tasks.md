# Implementation Plan: Master Clock Timing Audit

## Overview

This implementation plan updates the master clock timing system to use hardware-accurate clock frequencies for NTSC and PAL systems. The key changes are:

1. Update clock frequency constants to match verified hardware specifications
2. Implement separate NTSC and PAL clock configurations with exact integer ratios
3. Remove debt reset at frame boundaries to allow proper debt carry-over
4. Add comprehensive tests to verify timing accuracy and bounded debt

The implementation will determine whether the jitter bug in Course de Voitures was caused by incorrect clock ratios or by the debt carry-over mechanism itself.

## Tasks

- [x] 1. Update clock frequency constants in MasterClock
  - [x] 1.1 Add NTSC timing constants to master_clock.h
    - Add `NTSC_MASTER_CLOCK_MHZ = 7.15909`
    - Add `NTSC_VDC_CLOCK_MHZ = 3.579545`
    - Add `NTSC_CPU_INSTRUCTION_MHZ = 0.357954`
    - Add `NTSC_CYCLES_PER_CPU_INSTRUCTION = 10.0`
    - _Requirements: US-1.1, US-1.2, US-1.3_
  
  - [x] 1.2 Add PAL timing constants to master_clock.h
    - Add `PAL_MASTER_CLOCK_MHZ = 17.734476`
    - Add `PAL_VDC_CLOCK_MHZ = 3.546895`
    - Add `PAL_CPU_INSTRUCTION_MHZ = 0.394099`
    - Add `PAL_CYCLES_PER_CPU_INSTRUCTION = 9.0`
    - _Requirements: US-1.1, US-1.2, US-1.3_
  
  - [x] 1.3 Add member variables for standard-specific timing
    - Add `double vdc_clock_mhz_`
    - Add `double cpu_instruction_mhz_`
    - Add `double cycles_per_cpu_instruction_`
    - Remove static `CYCLES_PER_CPU_INSTRUCTION` constant
    - _Requirements: US-1.1, US-1.2, US-1.3_

- [x] 2. Update calculate_timing() to set standard-specific values
  - [x] 2.1 Implement standard-specific clock frequency selection
    - Set `vdc_clock_mhz_` based on video standard (NTSC vs PAL)
    - Set `cpu_instruction_mhz_` based on video standard
    - Set `cycles_per_cpu_instruction_` based on video standard
    - Calculate `cycles_per_scanline` using fractional values (227.5 for NTSC, 227.36 for PAL)
    - _Requirements: US-1.1, US-1.2, US-1.3, US-2.3_
  
  - [x] 2.2 Update cycles per frame calculations
    - Use fractional cycles per scanline for accuracy
    - NTSC: 262 × 227.5 = 59,605 cycles
    - PAL: 312 × 227.36 ≈ 70,936 cycles
    - _Requirements: US-2.1, US-2.2, US-2.4_

- [x] 3. Update debt calculation methods to use member variables
  - [x] 3.1 Update tick() method
    - Replace `CYCLES_PER_CPU_INSTRUCTION` with `cycles_per_cpu_instruction_`
    - Ensure CPU executes when debt >= cycles_per_cpu_instruction_
    - _Requirements: US-4.1, US-4.2_
  
  - [x] 3.2 Update cpu_executed() method
    - Replace `CYCLES_PER_CPU_INSTRUCTION` with `cycles_per_cpu_instruction_`
    - Calculate VDC debt using standard-specific ratio
    - _Requirements: US-4.1, US-4.2_
  
  - [x] 3.3 Update vdc_ticked() method
    - Replace `CYCLES_PER_CPU_INSTRUCTION` with `cycles_per_cpu_instruction_`
    - Calculate CPU debt using standard-specific ratio
    - _Requirements: US-4.1, US-4.2_

- [x] 4. Remove debt reset from reset_frame()
  - [x] 4.1 Modify reset_frame() to preserve debt
    - Remove `cpu_cycle_debt_ = 0.0` line
    - Remove `vdc_cycle_debt_ = 0.0` line
    - Keep only `frame_cycle_count_ = 0`
    - Add comment explaining debt carry-over for accuracy
    - _Requirements: US-5.1, US-5.2_

- [ ] 5. Add unit tests for clock frequencies
  - [ ]* 5.1 Test NTSC clock frequencies
    - Verify VDC clock = 3.579545 MHz
    - Verify CPU instruction rate = 0.357954 MHz
    - Verify cycles per CPU instruction = 10.0
    - _Requirements: US-1.1, US-1.2, US-1.3_
  
  - [ ]* 5.2 Test PAL clock frequencies
    - Verify VDC clock = 3.546895 MHz
    - Verify CPU instruction rate = 0.394099 MHz
    - Verify cycles per CPU instruction = 9.0
    - _Requirements: US-1.1, US-1.2, US-1.3_
  
  - [ ]* 5.3 Test frame timing calculations
    - Verify NTSC cycles per frame = 59,605
    - Verify PAL cycles per frame ≈ 70,936
    - Verify NTSC scanlines = 262
    - Verify PAL scanlines = 312
    - _Requirements: US-2.1, US-2.2, US-2.3, US-2.4_
  
  - [ ]* 5.4 Test frame rate calculations
    - Verify NTSC frame rate ≈ 59.94 Hz
    - Verify PAL frame rate = 50 Hz
    - _Requirements: US-3.1, US-3.2_

- [x] 6. Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 7. Add property-based tests for timing accuracy
  - [ ]* 7.1 Write property test for frame cycle consistency
    - **Property 1: Frame Cycle Consistency**
    - **Validates: Requirements US-3.3, US-5.2**
    - Test that N frames execute exactly N × cycles_per_frame cycles
    - Run with random N from 1 to 1000
    - Verify no accumulation of timing errors
  
  - [ ]* 7.2 Write property test for VDC/CPU ratio maintenance
    - **Property 2: VDC/CPU Ratio Maintenance**
    - **Validates: Requirements US-4.1, US-4.4**
    - Test that ratio of VDC cycles to CPU instructions equals expected ratio
    - Run random sequences of operations (100+ iterations)
    - Verify ratio is maintained within small tolerance (< 0.01%)
  
  - [ ]* 7.3 Write property test for debt accumulation correctness
    - **Property 3: Debt Accumulation Correctness**
    - **Validates: Requirements US-4.2**
    - Test that VDC tick increases CPU debt by 1/ratio
    - Test that CPU execution increases VDC debt by cycles × ratio
    - Run with random instruction cycle counts (100+ iterations)
  
  - [ ]* 7.4 Write property test for bounded debt
    - **Property 4: Bounded Debt**
    - **Validates: Requirements US-4.3**
    - Simulate extended emulation (1 hour equivalent)
    - Verify CPU and VDC debt remain below threshold (e.g., 100 cycles)
    - Test with both NTSC and PAL
  
  - [ ]* 7.5 Write property test for debt carry-over accuracy
    - **Property 5: Debt Carry-Over Maintains Accuracy**
    - **Validates: Requirements US-5.2**
    - Compare timing accuracy with debt carry-over vs debt reset
    - Run 1000+ frames with both approaches
    - Verify carry-over has lower cumulative error

- [ ] 8. Update existing tests for new behavior
  - [ ]* 8.1 Update ResetFrameClearsExcessiveDebt test
    - Rename to `ResetFramePreservesDebt`
    - Verify debt is NOT reset to zero
    - Verify debt carries over between frames
    - _Requirements: US-5.1_
  
  - [ ]* 8.2 Update CycleRatioApproximately9Point9 test
    - Update for exact ratios (10.0 for NTSC, 9.0 for PAL)
    - Test both NTSC and PAL separately
    - _Requirements: US-1.3_

- [ ] 9. Test with Course de Voitures to verify jitter fix
  - [ ]* 9.1 Add integration test for raster effect stability
    - Load Course de Voitures ROM
    - Run to level select screen
    - Capture grid toggle Y positions for 10 frames
    - Verify positions are consistent (no jitter)
    - _Requirements: US-5.3_

- [x] 10. Final checkpoint - Verify all tests pass and timing is correct
  - Ensure all tests pass, ask the user if questions arise.
  - Verify no regressions in existing functionality
  - Manual testing: Course de Voitures road should be stationary
  - Manual testing: Games should run at correct speed

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- The critical decision point is whether debt carry-over fixes the jitter bug
- If jitter persists after fixing clock ratios, investigate root cause before deciding on debt reset strategy
- All timing constants are based on verified hardware specifications from Wikipedia and technical documentation
- The exact integer ratios (10.0 for NTSC, 9.0 for PAL) are key to bounded debt behavior
