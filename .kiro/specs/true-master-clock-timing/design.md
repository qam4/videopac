# True Master Clock Timing - Design

## Architecture Overview

Replace VDC-cycle-based timing with true master clock ticks. Both CPU and VDC derive their timing from a single master clock counter, eliminating the need for debt tracking and manual alternation.

## Current vs Proposed Architecture

### Current (VDC-Cycle Based)
```
MasterClock (counts VDC cycles)
    ├─> VDC (ticks every cycle)
    └─> CPU (ticks every 10 cycles via debt tracking)
```

### Proposed (Master Clock Based)
```
MasterClock (counts master ticks)
    ├─> VDC (ticks every 2 ticks for NTSC, 5 for PAL)
    └─> CPU (ticks every 20 ticks for NTSC, 45 for PAL)
```

## Component Changes

### 1. MasterClock Class

**New Interface:**
```cpp
class MasterClock {
public:
    explicit MasterClock(VideoStandard standard);
    
    enum class ExecuteNext {
        CPU,
        VDC,
        NONE  // Neither ready yet
    };
    
    // Advance master clock by 1 tick, return what should execute
    ExecuteNext tick();
    
    // Notify that component executed
    void cpu_executed();
    void vdc_executed();
    
    // Accessors
    uint64 get_master_tick_count() const;
    uint32 get_current_scanline() const;
    uint32 get_scanline_tick() const;
    
private:
    VideoStandard standard_;
    uint64 master_tick_count_;      // Total master ticks since start
    uint32 scanline_tick_;          // Tick within current scanline (0-454 NTSC, 0-1134 PAL)
    uint32 current_scanline_;       // Current scanline number
    
    // Standard-specific constants
    uint32 ticks_per_scanline_;     // 455 NTSC, 1135 PAL
    uint32 vdc_tick_divisor_;       // 2 NTSC, 5 PAL
    uint32 cpu_tick_divisor_;       // 20 NTSC, 45 PAL
    uint32 total_scanlines_;        // 262 NTSC, 312/313 PAL
};
```

**Implementation:**
```cpp
MasterClock::ExecuteNext MasterClock::tick() {
    master_tick_count_++;
    scanline_tick_++;
    
    // Check for scanline boundary
    if (scanline_tick_ >= ticks_per_scanline_) {
        scanline_tick_ = 0;
        current_scanline_++;
        
        if (current_scanline_ >= total_scanlines_) {
            current_scanline_ = 0;  // Frame boundary
        }
    }
    
    // Determine what should execute at this tick
    bool vdc_ready = (master_tick_count_ % vdc_tick_divisor_) == 0;
    bool cpu_ready = (master_tick_count_ % cpu_tick_divisor_) == 0;
    
    // Priority: CPU > VDC (if both ready, CPU goes first)
    if (cpu_ready) return ExecuteNext::CPU;
    if (vdc_ready) return ExecuteNext::VDC;
    return ExecuteNext::NONE;
}
```

### 2. VDC Class Changes

**Minimal changes required:**
- `tick_one_cycle()` remains the same
- Beam position calculation changes to use master ticks:

```cpp
void VDC::tick_one_cycle() {
    state_.total_cycles++;  // Still count VDC cycles internally
    
    // Calculate beam position from VDC cycles (unchanged)
    uint32 cycles_per_frame = total_scanlines_ * cycles_per_scanline_;
    uint32 frame_cycles = state_.total_cycles % cycles_per_frame;
    state_.beam_y = frame_cycles / cycles_per_scanline_;
    state_.beam_x = frame_cycles % cycles_per_scanline_;
    
    // Rest of implementation unchanged
    // ...
}
```

**Note:** VDC continues to count its own cycles internally. The master clock just determines WHEN it ticks.

### 3. CPU Class Changes

**No changes required:**
- CPU continues to execute instructions normally
- Master clock determines WHEN it executes
- Instruction cycle counts remain the same

### 4. Emulator Main Loop

**Current:**
```cpp
while (running) {
    auto next = master_clock.tick();
    if (next == MasterClock::ExecuteNext::CPU) {
        uint8 cycles = cpu.execute_one_instruction();
        master_clock.cpu_executed(cycles);
    } else {
        vdc.tick_one_cycle();
        master_clock.vdc_ticked();
    }
}
```

**Proposed:**
```cpp
while (running) {
    auto next = master_clock.tick();
    
    switch (next) {
        case MasterClock::ExecuteNext::CPU:
            cpu.execute_one_instruction();
            master_clock.cpu_executed();
            break;
            
        case MasterClock::ExecuteNext::VDC:
            vdc.tick_one_cycle();
            master_clock.vdc_executed();
            break;
            
        case MasterClock::ExecuteNext::NONE:
            // Neither ready, continue to next tick
            break;
    }
}
```

## Timing Constants

### NTSC Constants
```cpp
namespace NTSCTiming {
    constexpr uint32 MASTER_CLOCK_HZ = 7159090;      // 7.15909 MHz
    constexpr uint32 TICKS_PER_SCANLINE = 455;
    constexpr uint32 SCANLINES_PER_FRAME = 262;
    constexpr uint32 TICKS_PER_FRAME = 119210;       // 455 × 262
    
    constexpr uint32 VDC_TICK_DIVISOR = 2;           // VDC ticks every 2 master ticks
    constexpr uint32 CPU_TICK_DIVISOR = 20;          // CPU ticks every 20 master ticks
    
    // Derived values (for documentation)
    constexpr double VDC_CLOCK_HZ = 3579545.0;       // Master ÷ 2
    constexpr double CPU_CLOCK_HZ = 357954.5;        // Master ÷ 20
    constexpr double VDC_CYCLES_PER_LINE = 227.5;    // 455 ÷ 2
    constexpr double CPU_INSTRUCTIONS_PER_LINE = 22.75;  // 455 ÷ 20
}
```

### PAL Constants
```cpp
namespace PALTiming {
    constexpr uint32 MASTER_CLOCK_HZ = 17734476;     // 17.734476 MHz
    constexpr uint32 TICKS_PER_SCANLINE = 1135;
    constexpr uint32 SCANLINES_PER_FRAME = 312;      // (or 313)
    constexpr uint32 TICKS_PER_FRAME = 354120;       // 1135 × 312
    
    constexpr uint32 VDC_TICK_DIVISOR = 5;           // VDC ticks every 5 master ticks
    constexpr uint32 CPU_TICK_DIVISOR = 45;          // CPU ticks every 45 master ticks
    
    // Derived values (for documentation)
    constexpr double VDC_CLOCK_HZ = 3546895.2;       // Master ÷ 5
    constexpr double CPU_CLOCK_HZ = 394099.47;       // Master ÷ 45
    constexpr double VDC_CYCLES_PER_LINE = 227.0;    // 1135 ÷ 5 (exact!)
    constexpr double CPU_INSTRUCTIONS_PER_LINE = 25.22;  // 1135 ÷ 45
}
```

## Benefits

### 1. Eliminates Fractional Cycle Handling
- No more 227.5 approximation to 227
- NTSC naturally produces 227/228 alternation
- PAL naturally produces 227 every line

### 2. Simplifies Code
- No debt tracking numerator/denominator
- No manual alternation logic
- Single counter drives everything

### 3. Perfect Accuracy
- Matches hardware exactly
- No timing drift over time
- Correct NTSC color phase shift

### 4. Easier to Understand
- Clear relationship: master tick → VDC/CPU
- Divisors match hardware documentation
- No complex ratio calculations

## Migration Strategy

### Phase 1: Add Master Clock Support (Parallel)
1. Add new `MasterClockV2` class with master tick implementation
2. Keep existing `MasterClock` class unchanged
3. Add compile-time flag to switch between implementations
4. Run tests with both implementations

### Phase 2: Validate Accuracy
1. Compare frame timing between old and new implementations
2. Verify VDC cycle counts match
3. Verify CPU instruction counts match
4. Test with multiple games on both NTSC and PAL

### Phase 3: Replace Old Implementation
1. Remove old `MasterClock` class
2. Rename `MasterClockV2` to `MasterClock`
3. Update all references
4. Remove compile-time flag

### Phase 4: Cleanup
1. Remove debt tracking code
2. Simplify VDC timing constants
3. Update documentation
4. Update tests

## Testing Strategy

### Unit Tests
- Test master clock tick counting
- Test VDC tick generation (every 2/5 ticks)
- Test CPU tick generation (every 20/45 ticks)
- Test scanline boundaries
- Test frame boundaries

### Integration Tests
- Run full frame, verify cycle counts
- Test NTSC: 119,210 master ticks = 59,605 VDC cycles = 5,960.5 CPU instructions
- Test PAL: 354,120 master ticks = 70,824 VDC cycles = 7,869.3 CPU instructions
- Verify no timing drift over 1000 frames

### Regression Tests
- All existing game tests must pass
- Save state compatibility
- Performance benchmarks

## Performance Considerations

### Potential Overhead
- More frequent tick() calls (every master tick vs every VDC cycle)
- NTSC: 2× more calls (119,210 vs 59,605)
- PAL: 5× more calls (354,120 vs 70,824)

### Optimizations
- Inline tick() method
- Use modulo with power-of-2 divisors where possible
- Cache divisor results
- Profile and optimize hot paths

### Expected Impact
- Modern CPUs handle modulo efficiently
- Branch prediction helps with switch statement
- Overall: <5% performance impact expected

## Open Questions

1. Should we expose master tick count in debugger UI?
2. How to handle save state migration (VDC total_cycles)?
3. Should we add master tick tracing for debugging?
4. Do we need sub-tick precision for any edge cases?

## References

- doc/case-studies/killer-bees-banking.md - Complete timing documentation
- Intel 8244/8245 datasheets
- Kevtris's Odyssey² hardware documentation
- Dan Boris's technical notes
