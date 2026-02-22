# Master Clock Timing Audit - Design Document

## Overview

This design addresses the master clock timing system audit to ensure cycle-accurate emulation with correct clock frequencies for NTSC and PAL systems. The current implementation uses simplified clock frequencies that deviate from hardware specifications, causing timing drift and incorrect game speeds.

The audit will:
1. Update clock frequency constants to match verified hardware specifications
2. Implement separate NTSC and PAL clock configurations
3. Verify cycles-per-frame calculations
4. Validate the cycle debt tracking system
5. Add comprehensive tests to prevent timing regressions

## Architecture

### Current Architecture

The `MasterClock` class coordinates CPU and VDC execution using a cycle debt system:

```
MasterClock
├── VDC Clock (base unit): 3.54 MHz
├── CPU Clock: 1.79 MHz / 5 = 0.358 MHz
├── Cycle Ratio: ~9.9 VDC cycles per CPU instruction
└── Frame Timing: Based on scanlines × cycles per scanline
```

The debt system works as follows:
- VDC ticks accumulate CPU debt (CPU needs to catch up)
- CPU execution reduces CPU debt and adds VDC debt
- The component with higher debt executes next
- Debt is reset at frame boundaries to prevent jitter (per cycle-accurate-frame-timing spec)

**How Cycle Debt Works**:
1. **Debt Accumulation**: Each VDC tick adds 1/ratio to CPU debt (fractional instruction cycles)
2. **Debt Repayment**: When CPU debt ≥ ratio, CPU executes an instruction
3. **Carry-Over**: Fractional cycles accumulate until they "buy" a full instruction
4. **Frame Reset**: Debt resets to zero at frame boundaries to ensure deterministic raster effects

This mechanism ensures CPU and VDC stay synchronized despite their non-integer frequency ratio.

### Issues with Current Implementation

1. **Incorrect Clock Frequencies**:
   - Uses 3.54 MHz for VDC (should be 3.579545 MHz for NTSC, 3.546895 MHz for PAL)
   - Uses 1.79 MHz / 5 for CPU (should account for 8048's internal /3 divider)
   - VDC/CPU ratio is 9.888 (should be 10.0 for NTSC, 9.0 for PAL)

2. **No NTSC/PAL Differentiation**:
   - Both standards use the same VDC clock frequency
   - PAL runs 9.9% too slow due to incorrect clock

3. **Timing Drift**:
   - NTSC: 1.1% too slow (~40 seconds per hour)
   - PAL: 9.9% too slow (~6 minutes per hour)

### Proposed Architecture

Update the `MasterClock` class to use hardware-accurate frequencies:

```
MasterClock (NTSC)
├── Master Clock: 7.15909 MHz
├── VDC Clock: 3.579545 MHz (master / 2)
├── CPU Crystal: 5.369317 MHz (master × 0.75)
├── CPU State Clock: 1.789772 MHz (crystal / 3)
├── CPU Instruction Rate: 0.357954 MHz (state / 5)
└── VDC/CPU Ratio: 10.0 (exact)

MasterClock (PAL)
├── Master Clock: 17.734476 MHz
├── VDC Clock: 3.546895 MHz (master / 5)
├── CPU Crystal: 5.911492 MHz (master / 3)
├── CPU State Clock: 1.970497 MHz (crystal / 3)
├── CPU Instruction Rate: 0.394099 MHz (state / 5)
└── VDC/CPU Ratio: 9.0 (exact)
```

## Components and Interfaces

### MasterClock Class Updates

**Constants to Update**:

```cpp
// NTSC timing constants
static constexpr double NTSC_MASTER_CLOCK_MHZ = 7.15909;
static constexpr double NTSC_VDC_CLOCK_MHZ = 3.579545;      // master / 2
static constexpr double NTSC_CPU_INSTRUCTION_MHZ = 0.357954; // (master × 0.75 / 3) / 5
static constexpr double NTSC_CYCLES_PER_CPU_INSTRUCTION = 10.0; // exact ratio

// PAL timing constants
static constexpr double PAL_MASTER_CLOCK_MHZ = 17.734476;
static constexpr double PAL_VDC_CLOCK_MHZ = 3.546895;       // master / 5
static constexpr double PAL_CPU_INSTRUCTION_MHZ = 0.394099;  // (master / 3 / 3) / 5
static constexpr double PAL_CYCLES_PER_CPU_INSTRUCTION = 9.0; // exact ratio
```

**Member Variables to Add**:

```cpp
double vdc_clock_mhz_;              // VDC clock frequency for current standard
double cpu_instruction_mhz_;        // CPU instruction rate for current standard
double cycles_per_cpu_instruction_; // VDC cycles per CPU instruction
```

**Methods to Update**:

- `calculate_timing()`: Set clock frequencies based on video standard
- `tick()`: Use standard-specific cycle ratio
- `cpu_executed()`: Use standard-specific cycle ratio
- `vdc_ticked()`: Use standard-specific cycle ratio

### Frame Timing Calculations

**NTSC Frame Timing**:
```
VDC Clock: 3.579545 MHz
Frame Rate: 59.94 Hz (NTSC color standard)
Scanlines: 262
Cycles per scanline: 3.579545 MHz / 59.94 Hz / 262 = 227.5
Total cycles per frame: 262 × 227.5 = 59,605
```

**PAL Frame Timing**:
```
VDC Clock: 3.546895 MHz
Frame Rate: 50 Hz
Scanlines: 312
Cycles per scanline: 3.546895 MHz / 50 Hz / 312 = 227.36
Total cycles per frame: 312 × 227.36 = 70,936
```

**Implementation Decision**: Use fractional cycles per scanline (227.5 for NTSC, 227.36 for PAL) and track fractional debt to maintain accuracy over time.

## Data Models

### Timing Configuration Structure

```cpp
struct TimingConfig {
    double vdc_clock_mhz;
    double cpu_instruction_mhz;
    double cycles_per_cpu_instruction;
    uint32 scanlines_per_frame;
    double cycles_per_scanline;
    uint32 cycles_per_frame;
    double frame_rate_hz;
};
```

This structure encapsulates all timing parameters for a video standard, making it easier to verify correctness and switch between standards.

### Cycle Debt Tracking

The cycle debt system remains fundamentally the same but uses accurate ratios:

```cpp
// Current debt tracking (unchanged)
double cpu_cycle_debt_;  // Accumulated CPU cycles to execute
double vdc_cycle_debt_;  // Accumulated VDC cycles to execute

// Debt accumulation (updated with correct ratios)
// VDC tick: cpu_cycle_debt_ += 1.0 / cycles_per_cpu_instruction_
// CPU exec: vdc_cycle_debt_ += instruction_cycles * cycles_per_cpu_instruction_
```

The key insight is that with exact integer ratios (10.0 for NTSC, 9.0 for PAL), the debt system will have less floating-point error accumulation.


## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Frame Cycle Consistency

*For any* number of complete frames N, the total VDC cycles executed should equal N × cycles_per_frame with no accumulation of timing errors across frames.

**Validates: Requirements US-3.3, US-5.2**

This property ensures that frame timing is consistent over extended periods. Each frame should execute exactly the same number of cycles, preventing drift that would cause games to speed up or slow down over time.

### Property 2: VDC/CPU Ratio Maintenance

*For any* sequence of VDC ticks and CPU instruction executions over a long period, the ratio of total VDC cycles to total CPU instruction cycles should equal the configured cycles_per_cpu_instruction (10.0 for NTSC, 9.0 for PAL) within a small tolerance.

**Validates: Requirements US-4.1, US-4.4**

This property ensures that the CPU and VDC remain synchronized over time. The debt system should maintain the correct execution ratio regardless of the specific sequence of operations.

### Property 3: Debt Accumulation Correctness

*For any* VDC tick operation, the CPU debt should increase by exactly 1.0 / cycles_per_cpu_instruction, and *for any* CPU execution of N instruction cycles, the VDC debt should increase by exactly N × cycles_per_cpu_instruction.

**Validates: Requirements US-4.2**

This property verifies the mathematical correctness of the debt accumulation formulas. The debt changes must be precise to prevent gradual desynchronization.

### Property 4: Bounded Debt

*For any* sequence of operations over an extended period (e.g., 1 hour of emulation), the absolute value of both CPU debt and VDC debt should remain below a reasonable threshold (e.g., 100 cycles).

**Validates: Requirements US-4.3**

This property ensures that the debt system doesn't accumulate unbounded errors. With exact integer ratios (10.0 for NTSC, 9.0 for PAL) and debt carry-over, debt should naturally oscillate around zero. If debt grows unbounded, it indicates a bug in the debt calculation logic.

### Property 5: Debt Carry-Over Maintains Accuracy

*For any* sequence of frames, carrying debt over between frames should result in more accurate long-term timing than resetting debt to zero.

**Validates: Requirements US-5.2**

This property verifies that debt carry-over is beneficial. We can test this by comparing two implementations: one that carries debt over and one that resets. The carry-over version should have lower cumulative timing error over 1000+ frames.

## Error Handling

### Invalid Video Standard

If an invalid video standard is provided to the constructor, the system should default to NTSC and log a warning. This prevents crashes while making the error visible.

### Floating-Point Precision

The debt system uses double-precision floating-point arithmetic. While this provides sufficient precision for emulation (error < 1 cycle per million operations), we should:

1. Use exact integer ratios where possible (10.0 for NTSC, 9.0 for PAL)
2. Reset debt at frame boundaries to prevent long-term accumulation
3. Add tests to verify precision over extended periods

### Frame Boundary Edge Cases

The debt system must balance two competing concerns:

1. **Long-term accuracy**: Debt should carry over between frames to maintain precise timing ratios
2. **Frame-to-frame consistency**: For deterministic raster effects

**Correct Approach** (based on emulation research):

Debt should **carry over** between frames. This is how cycle-accurate emulators maintain synchronization:

```cpp
void MasterClock::reset_frame() {
    frame_cycle_count_ = 0;
    // Debt carries over - DO NOT reset
    // This maintains long-term accuracy
}
```

**Why Carry-Over is Correct**:
- The CPU and VDC have independent clocks in hardware
- The fractional cycle debt represents the real timing relationship
- Resetting debt throws away timing information and causes drift
- With exact integer ratios (10.0 for NTSC, 9.0 for PAL), debt should naturally stay bounded

**Current Implementation Issue**: The cycle-accurate-frame-timing spec changed this to full reset to fix a jitter bug, but this may have been treating a symptom rather than the root cause. The jitter might have been caused by incorrect clock ratios (9.888 instead of 10.0/9.0).

**Recommendation for This Audit**: 
1. Fix the clock ratios to exact integers (10.0 for NTSC, 9.0 for PAL)
2. Remove the debt reset at frame boundaries (let it carry over)
3. Test if the jitter still occurs with correct ratios
4. If jitter persists, investigate the root cause rather than masking it with debt reset

This audit should determine whether the jitter was caused by incorrect ratios or by the debt carry-over itself.

## Testing Strategy

### Dual Testing Approach

We will use both unit tests and property-based tests to ensure comprehensive coverage:

**Unit Tests** focus on:
- Specific clock frequency values for NTSC and PAL
- Cycles per frame calculations
- Frame boundary behavior
- Debt reset functionality
- Edge cases (first frame, after reset, etc.)

**Property-Based Tests** focus on:
- Frame cycle consistency over many frames
- VDC/CPU ratio maintenance over random operation sequences
- Debt accumulation correctness with random instruction cycles
- Bounded debt over extended simulations

### Property-Based Testing Configuration

We will use **Google Test with custom property test helpers** (or integrate a C++ property testing library like RapidCheck if available). Each property test will:

- Run a minimum of 100 iterations with randomized inputs
- Tag each test with a comment referencing the design property
- Format: `// Feature: master-clock-timing-audit, Property N: [property text]`

### Test Coverage Requirements

1. **Clock Frequency Tests**:
   - Verify NTSC VDC clock = 3.579545 MHz
   - Verify PAL VDC clock = 3.546895 MHz
   - Verify NTSC CPU instruction rate = 0.357954 MHz
   - Verify PAL CPU instruction rate = 0.394099 MHz
   - Verify NTSC ratio = 10.0
   - Verify PAL ratio = 9.0

2. **Frame Timing Tests**:
   - Verify NTSC cycles per frame = 59,605
   - Verify PAL cycles per frame ≈ 70,936
   - Verify NTSC scanlines = 262
   - Verify PAL scanlines = 312
   - Verify NTSC frame rate ≈ 59.94 Hz
   - Verify PAL frame rate = 50 Hz

3. **Debt System Tests**:
   - Property test: Frame cycle consistency (100+ iterations)
   - Property test: VDC/CPU ratio maintenance (100+ iterations)
   - Property test: Debt accumulation correctness (100+ iterations)
   - Property test: Bounded debt (100+ iterations)
   - Unit test: Debt reset at frame boundaries
   - Unit test: Initial debt state

4. **Regression Tests**:
   - All existing tests must continue to pass
   - No changes to public API (only internal constants)
   - Verify existing games still run correctly

### Testing Tools

- **Google Test**: Primary testing framework
- **Custom property test helpers**: For randomized testing
- **Floating-point comparison**: Use `EXPECT_NEAR` with appropriate epsilon
- **Long-running tests**: Simulate hours of emulation in seconds

### Success Criteria

- All unit tests pass
- All property tests pass with 100+ iterations
- No timing drift detected over simulated 1-hour sessions
- Existing games run at correct speed (verified manually)
- Audio pitch is correct (verified manually)

## Implementation Notes

### Backward Compatibility

The changes are internal to the `MasterClock` class and do not affect the public API. Existing code that uses `MasterClock` will automatically benefit from the corrected timing without modifications.

### Performance Considerations

The updated calculations use the same algorithmic complexity as before. The only changes are:
- Different constant values
- Standard-specific constants selected at construction time

There should be no measurable performance impact.

### Migration Path

1. Update constants in `master_clock.h` to use exact hardware frequencies
2. Update `calculate_timing()` to set standard-specific values
3. Update all references to `CYCLES_PER_CPU_INSTRUCTION` to use member variable
4. **Remove debt reset from `reset_frame()`** - let debt carry over naturally
5. Add new unit tests for clock frequencies
6. Add property-based tests for timing properties and bounded debt
7. Test Course de Voitures to verify jitter is fixed by correct ratios
8. Run full test suite to verify no regressions
9. Manual testing with known games to verify correct speed

**Critical Decision Point**: After implementing correct clock ratios, we need to determine if debt should carry over or reset:
- If jitter is gone with correct ratios → keep debt carry-over (more accurate)
- If jitter persists → investigate root cause before deciding on reset

### Future Enhancements

After this audit, potential future improvements include:

1. **Fractional cycle tracking**: Use fractional cycles per scanline for even higher accuracy
2. **Cycle-accurate CPU/VDC interleaving**: Model the exact cycle-by-cycle behavior
3. **Audio synchronization**: Ensure audio output matches the corrected timing
4. **Save state timing**: Verify timing state is correctly saved/restored

These are out of scope for this audit but may be addressed in future work.
