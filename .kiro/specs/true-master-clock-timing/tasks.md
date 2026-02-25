# True Master Clock Timing - Tasks

## Status: Planning

## Tasks

### Phase 1: Foundation (Master Clock Implementation)

- [ ] **Task 1.1**: Create MasterClockV2 class with master tick counter
  - Add master_tick_count_ member
  - Add scanline_tick_ and current_scanline_ members
  - Implement tick() method with divisor logic
  - Add NTSC/PAL timing constants
  - **Estimate**: 2 hours

- [ ] **Task 1.2**: Add unit tests for MasterClockV2
  - Test tick counting
  - Test VDC tick generation (every 2/5 ticks)
  - Test CPU tick generation (every 20/45 ticks)
  - Test scanline boundaries (455/1135 ticks)
  - Test frame boundaries
  - **Estimate**: 2 hours

- [ ] **Task 1.3**: Add compile-time flag to switch implementations
  - Add USE_MASTER_CLOCK_V2 CMake option
  - Update emulator.cpp to use selected implementation
  - Ensure both compile successfully
  - **Estimate**: 1 hour

### Phase 2: Integration and Validation

- [ ] **Task 2.1**: Update emulator main loop for MasterClockV2
  - Modify tick/execute loop to handle NONE case
  - Remove debt tracking calls
  - Simplify CPU/VDC coordination
  - **Estimate**: 1 hour

- [ ] **Task 2.2**: Add integration tests
  - Test full NTSC frame (119,210 ticks = 59,605 VDC cycles)
  - Test full PAL frame (354,120 ticks = 70,824 VDC cycles)
  - Verify cycle counts match expected values
  - Test 1000-frame run for timing drift
  - **Estimate**: 3 hours

- [ ] **Task 2.3**: Run regression tests with both implementations
  - Run all existing game tests with old implementation
  - Run all existing game tests with new implementation
  - Compare results, document any differences
  - **Estimate**: 2 hours

- [ ] **Task 2.4**: Performance benchmarking
  - Benchmark old implementation (baseline)
  - Benchmark new implementation
  - Compare frame rates, CPU usage
  - Profile hot paths if needed
  - **Estimate**: 2 hours

### Phase 3: Migration and Cleanup

- [ ] **Task 3.1**: Handle save state compatibility
  - Document VDC total_cycles mapping
  - Add migration logic if needed
  - Test loading old save states with new implementation
  - **Estimate**: 2 hours

- [ ] **Task 3.2**: Replace old MasterClock with MasterClockV2
  - Remove old MasterClock class
  - Rename MasterClockV2 to MasterClock
  - Update all references
  - Remove compile-time flag
  - **Estimate**: 1 hour

- [ ] **Task 3.3**: Remove debt tracking code
  - Remove cpu_cycle_debt_* members
  - Remove vdc_cycle_debt_* members
  - Simplify cpu_executed() and vdc_ticked() methods
  - **Estimate**: 1 hour

- [ ] **Task 3.4**: Update timing constants in types.h/vdc.h
  - Add master clock frequency constants
  - Add tick divisor constants
  - Document 227.5 cycles/line for NTSC
  - Document 227 cycles/line for PAL
  - **Estimate**: 1 hour

### Phase 4: Documentation and Polish

- [ ] **Task 4.1**: Update master_clock.h documentation
  - Document master tick approach
  - Explain divisor logic
  - Add usage examples
  - Reference hardware specifications
  - **Estimate**: 1 hour

- [ ] **Task 4.2**: Update HACKING.md with timing information
  - Explain master clock architecture
  - Document NTSC vs PAL differences
  - Add timing diagrams
  - **Estimate**: 1 hour

- [ ] **Task 4.3**: Add master tick tracing (optional)
  - Add trace output for master ticks
  - Show VDC/CPU execution points
  - Useful for debugging timing issues
  - **Estimate**: 2 hours

- [ ] **Task 4.4**: Update debugger UI to show master ticks (optional)
  - Add master tick counter display
  - Show scanline tick position
  - Add timing visualization
  - **Estimate**: 3 hours

## Total Estimates

- **Phase 1**: 5 hours
- **Phase 2**: 8 hours
- **Phase 3**: 5 hours
- **Phase 4**: 7 hours (4 hours without optional tasks)
- **Total**: 25 hours (22 hours without optional tasks)

## Dependencies

- None (can start immediately)

## Risks

1. **Performance regression**: Master tick approach may be slower
   - Mitigation: Profile and optimize, target <5% regression
   
2. **Save state compatibility**: Old saves may not load correctly
   - Mitigation: Add migration logic, document mapping
   
3. **Subtle timing bugs**: Games may behave differently
   - Mitigation: Extensive testing, compare with old implementation
   
4. **Test failures**: Existing tests may need updates
   - Mitigation: Update tests to match new timing model

## Success Criteria

- [ ] All unit tests pass
- [ ] All integration tests pass
- [ ] All game regression tests pass
- [ ] Performance within 5% of old implementation
- [ ] Save states load correctly
- [ ] No timing drift over 1000 frames
- [ ] Code is simpler (fewer lines, no debt tracking)
- [ ] Documentation is complete and accurate

## Notes

- This is a foundational change that improves accuracy
- Benefits all future timing-sensitive features
- Worth the investment for long-term maintainability
- Can be done incrementally with compile-time flag
