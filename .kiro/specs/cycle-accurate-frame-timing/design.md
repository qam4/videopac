# Cycle-Accurate Frame Timing - Design

## Problem Analysis

### Root Cause
The `MasterClock::reset_frame()` function only resets cycle debt when it becomes excessively negative:

```cpp
void MasterClock::reset_frame() {
    frame_cycle_count_ = 0;
    
    // Only resets if debt < -9.9 or < -10.0
    if (cpu_cycle_debt_ < -CYCLES_PER_CPU_INSTRUCTION) {
        cpu_cycle_debt_ = 0.0;
    }
    if (vdc_cycle_debt_ < -10.0) {
        vdc_cycle_debt_ = 0.0;
    }
}
```

**Problem**: Positive debt carries over between frames, causing CPU to execute at different relative positions each frame.

### Impact on Raster Effects

Games like Course de Voitures use raster effects - they write to VDC registers mid-frame to create visual effects:

1. Game waits for VBlank interrupt
2. Game executes code to toggle grid enable bit
3. Grid toggle happens at specific scanline based on when CPU executes
4. If CPU timing varies, grid toggles at different scanlines
5. Result: 1-scanline jitter creates visual scrolling artifact

**Measured Jitter:**
- Frame 6: Grid toggle at Y=70, 82, 134, 146
- Frame 7: Grid toggle at Y=71, 83, 135, 147 (+1 scanline shift)

This jitter affects both SDL and headless modes identically. The cycle debt variation causes the CPU to execute at different relative positions within each frame, which shifts when the game's raster effect code runs.

## Solution Approach

### Option 1: Full Debt Reset (Recommended)

Reset cycle debt to zero at every frame boundary:

```cpp
void MasterClock::reset_frame() {
    frame_cycle_count_ = 0;
    cpu_cycle_debt_ = 0.0;
    vdc_cycle_debt_ = 0.0;
}
```

**Pros:**
- Simple and clear
- Eliminates frame-to-frame jitter completely
- Each frame starts with identical timing
- Deterministic execution

**Cons:**
- May lose sub-cycle timing accuracy over many frames
- Could accumulate small timing errors over extended periods

**Mitigation:**
- Monitor total cycle count over 1000+ frames
- Add test to verify long-term accuracy within 0.1%
- If drift occurs, implement Option 2

### Option 2: Controlled Debt Reset

Reset debt to a small, consistent starting value:

```cpp
void MasterClock::reset_frame() {
    frame_cycle_count_ = 0;
    
    // Reset to consistent starting values
    // CPU starts with enough debt to execute immediately
    cpu_cycle_debt_ = CYCLES_PER_CPU_INSTRUCTION;
    vdc_cycle_debt_ = 0.0;
}
```

**Pros:**
- Maintains sub-cycle accuracy
- CPU executes at cycle 0 of each frame (consistent)
- Preserves original design intent

**Cons:**
- Slightly more complex
- Still resets debt (may lose some accuracy)

### Option 3: Debt Clamping

Clamp debt to a small range instead of resetting:

```cpp
void MasterClock::reset_frame() {
    frame_cycle_count_ = 0;
    
    // Clamp debt to ±1.0 range
    if (cpu_cycle_debt_ > 1.0) cpu_cycle_debt_ = 1.0;
    if (cpu_cycle_debt_ < -1.0) cpu_cycle_debt_ = -1.0;
    if (vdc_cycle_debt_ > 1.0) vdc_cycle_debt_ = 1.0;
    if (vdc_cycle_debt_ < -1.0) vdc_cycle_debt_ = -1.0;
}
```

**Pros:**
- Preserves some debt for accuracy
- Limits jitter to smaller range

**Cons:**
- Doesn't eliminate jitter completely
- More complex logic
- Arbitrary clamp values

## Recommended Solution: Option 1 (Full Reset)

Start with the simplest solution that eliminates jitter. If long-term timing drift becomes an issue, we can switch to Option 2.

## Implementation Plan

### Phase 1: Modify MasterClock

**File**: `src/master_clock.cpp`

```cpp
void MasterClock::reset_frame() {
    frame_cycle_count_ = 0;
    
    // Reset cycle debt to zero at frame boundaries
    // This ensures each frame starts with consistent timing
    // and eliminates frame-to-frame jitter in raster effects
    cpu_cycle_debt_ = 0.0;
    vdc_cycle_debt_ = 0.0;
}
```

### Phase 2: Update Tests

**File**: `tests/test_master_clock.cpp`

Update `ResetFrameClearsExcessiveDebt` test to verify debt is always reset:

```cpp
TEST_F(MasterClockTest, ResetFrameResetsDebt) {
    // Accumulate some debt
    for (int i = 0; i < 100; i++) {
        clock_ntsc->vdc_ticked();
    }
    
    // Debt should be non-zero
    EXPECT_NE(clock_ntsc->get_cpu_cycle_debt(), 0.0);
    
    // Reset frame
    clock_ntsc->reset_frame();
    
    // Debt should be zero
    EXPECT_EQ(clock_ntsc->get_cpu_cycle_debt(), 0.0);
    EXPECT_EQ(clock_ntsc->get_vdc_cycle_debt(), 0.0);
}
```

### Phase 3: Add Raster Effect Test

**File**: `tests/test_raster_effects.cpp` (new)

Create property-based test to verify raster effect stability:

```cpp
TEST(RasterEffectStability, GridTogglePositionsConstant) {
    // Test that grid toggle Y positions remain constant
    // when game state is unchanged (speed=0)
    
    Configuration config;
    config.video_standard = VideoStandard::PAL;
    EmulatorCore emulator(config);
    
    // Load Course de Voitures ROM and BIOS
    // ... (setup code)
    
    // Run to level select screen
    // ... (run frames, press '1' twice)
    
    // Capture grid toggle Y positions for 10 frames
    std::vector<std::vector<int>> toggle_positions;
    for (int frame = 0; frame < 10; frame++) {
        std::vector<int> positions;
        
        // Hook VDC register writes to capture Y positions
        // when control register (0xA0) is written
        // ... (capture logic)
        
        emulator.run_frame();
        toggle_positions.push_back(positions);
    }
    
    // Verify all frames have identical positions
    for (size_t i = 1; i < toggle_positions.size(); i++) {
        EXPECT_EQ(toggle_positions[i], toggle_positions[0])
            << "Frame " << i << " has different toggle positions";
    }
}
```

### Phase 4: Add Long-Term Accuracy Test

**File**: `tests/test_master_clock.cpp`

```cpp
TEST_F(MasterClockTest, LongTermTimingAccuracy) {
    // Run for 1000 frames and verify total cycles
    uint32 expected_cycles_per_frame = 262 * 227; // NTSC
    uint32 num_frames = 1000;
    uint64 expected_total = expected_cycles_per_frame * num_frames;
    
    uint64 actual_total = 0;
    for (uint32 frame = 0; frame < num_frames; frame++) {
        uint64 frame_start = clock_ntsc->get_master_cycle_count();
        
        // Run frame
        while (!clock_ntsc->is_frame_complete()) {
            auto next = clock_ntsc->tick();
            if (next == MasterClock::ExecuteNext::CPU) {
                clock_ntsc->cpu_executed(1);
            } else if (next == MasterClock::ExecuteNext::VDC) {
                clock_ntsc->vdc_ticked();
            }
        }
        
        uint64 frame_end = clock_ntsc->get_master_cycle_count();
        actual_total += (frame_end - frame_start);
        
        clock_ntsc->reset_frame();
    }
    
    // Verify accuracy within 0.1%
    double error_percent = std::abs((double)actual_total - expected_total) / expected_total * 100.0;
    EXPECT_LT(error_percent, 0.1) << "Timing drift: " << error_percent << "%";
}
```

## Correctness Properties

### Property 1: Frame Boundary Debt Reset
**For any** frame completion, the cycle debt **must** be reset to zero.

```
∀ frame: reset_frame() → (cpu_cycle_debt = 0.0 ∧ vdc_cycle_debt = 0.0)
```

**Validates**: Requirements 1.1, 1.2

### Property 2: Deterministic Frame Start
**For any** two frames with identical starting state, the CPU **must** execute at the same cycle.

```
∀ frame₁, frame₂: (state₁ = state₂ ∧ debt₁ = debt₂) → first_cpu_cycle₁ = first_cpu_cycle₂
```

**Validates**: Requirements 1.3, 2.3

### Property 3: Raster Effect Consistency
**For any** game with raster effects and unchanged state, register writes **must** occur at identical scanlines.

```
∀ frame₁, frame₂: (game_state₁ = game_state₂) → register_write_scanlines₁ = register_write_scanlines₂
```

**Validates**: Requirements 2.1, 2.2, 2.3, 2.4

### Property 4: Long-Term Accuracy
**For any** N frames, total cycles **must** equal N × cycles_per_frame within 0.1%.

```
∀ N: |total_cycles - (N × cycles_per_frame)| / (N × cycles_per_frame) < 0.001
```

**Validates**: Requirements 3.1, 3.2, 3.3, 3.4

## Testing Strategy

### Unit Tests
1. `ResetFrameResetsDebt` - Verify debt is zero after reset
2. `LongTermTimingAccuracy` - Verify no drift over 1000 frames
3. `DeterministicFrameStart` - Verify consistent CPU execution timing

### Integration Tests
1. `RasterEffectStability` - Verify Course de Voitures grid positions
2. `MultipleRunsDeterminism` - Run same input 10 times, verify identical output

### Manual Testing
1. Run Course de Voitures in SDL mode
2. Select Game 1, select level
3. Observe road - should be perfectly stationary (no jitter)
4. Press UP - road should scroll smoothly

## Rollback Plan

If the change causes issues:

1. **Timing Drift**: If long-term accuracy test fails, switch to Option 2 (controlled reset)
2. **Game Compatibility**: If games break, add per-game timing profiles
3. **Performance**: If frame time increases >5%, optimize debt calculation

## Success Criteria

1. ✅ Course de Voitures road is stationary without jitter
2. ✅ Grid toggle Y positions are identical across frames (±0 scanlines)
3. ✅ All existing tests pass
4. ✅ Long-term timing accuracy within 0.1%
5. ✅ No performance regression >5%

## References

- MasterClock: `src/master_clock.cpp`
- Emulator Core: `src/emulator.cpp`
- Case Study: `doc/case-studies/road-movement-bug.md`
- VDC Implementation: `src/vdc.cpp`
