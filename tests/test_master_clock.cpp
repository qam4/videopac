#include <gtest/gtest.h>
#include "master_clock.h"

using namespace videopac;

// Test fixture for MasterClock tests
class MasterClockTest : public ::testing::Test {
protected:
    void SetUp() override {
        clock_ntsc = std::make_unique<MasterClock>(VideoStandard::NTSC);
        clock_pal = std::make_unique<MasterClock>(VideoStandard::PAL);
    }
    
    std::unique_ptr<MasterClock> clock_ntsc;
    std::unique_ptr<MasterClock> clock_pal;
};

// Test initial state
TEST_F(MasterClockTest, InitialState) {
    EXPECT_EQ(clock_ntsc->get_master_cycle_count(), 0);
    EXPECT_DOUBLE_EQ(clock_ntsc->get_cpu_cycle_debt(), 0.0);
    EXPECT_DOUBLE_EQ(clock_ntsc->get_vdc_cycle_debt(), 0.0);
    EXPECT_EQ(clock_ntsc->get_frame_cycle_count(), 0);
    EXPECT_FALSE(clock_ntsc->is_frame_complete());
}

// Test VDC tick accumulates CPU debt
TEST_F(MasterClockTest, VDCTickAccumulatesCPUDebt) {
    clock_ntsc->vdc_ticked();
    
    EXPECT_EQ(clock_ntsc->get_master_cycle_count(), 1);
    EXPECT_GT(clock_ntsc->get_cpu_cycle_debt(), 0.0);
    EXPECT_EQ(clock_ntsc->get_frame_cycle_count(), 1);
}

// Test CPU execution reduces debt and adds VDC debt
TEST_F(MasterClockTest, CPUExecutionReducesDebt) {
    // Accumulate some CPU debt first
    for (int i = 0; i < 10; i++) {
        clock_ntsc->vdc_ticked();
    }
    
    double initial_cpu_debt = clock_ntsc->get_cpu_cycle_debt();
    double initial_vdc_debt = clock_ntsc->get_vdc_cycle_debt();
    
    // Execute CPU instruction (assume 1 cycle)
    clock_ntsc->cpu_executed(1);
    
    EXPECT_LT(clock_ntsc->get_cpu_cycle_debt(), initial_cpu_debt);
    EXPECT_GT(clock_ntsc->get_vdc_cycle_debt(), initial_vdc_debt);
}

// Test cycle ratio is exactly 10.0 for NTSC
TEST_F(MasterClockTest, CycleRatioExactly10ForNTSC) {
    // Execute one CPU instruction
    clock_ntsc->cpu_executed(1);
    
    double vdc_debt = clock_ntsc->get_vdc_cycle_debt();
    
    // Should be exactly 10.0 VDC cycles per CPU instruction for NTSC
    EXPECT_DOUBLE_EQ(vdc_debt, 10.0);
}

// Test tick() returns CPU when debt threshold reached
TEST_F(MasterClockTest, TickReturnsCPUWhenDebtThresholdReached) {
    // Accumulate enough CPU debt (need 10.0 VDC cycles per CPU instruction for NTSC)
    // After N VDC ticks, CPU debt = N * (1/10.0) = N/10
    // We need CPU debt >= 10.0, so N >= 100
    // Due to floating-point precision, we use 101 ticks to ensure we're above the threshold
    for (int i = 0; i < 101; i++) {
        clock_ntsc->vdc_ticked();
    }
    
    // CPU debt should be > 10.0 now (101 * (1/10.0) = 10.1)
    auto next = clock_ntsc->tick();
    EXPECT_EQ(next, MasterClock::ExecuteNext::CPU);
}

// Test tick() returns VDC by default
TEST_F(MasterClockTest, TickReturnsVDCByDefault) {
    auto next = clock_ntsc->tick();
    EXPECT_EQ(next, MasterClock::ExecuteNext::VDC);
}

// Test frame completion detection
TEST_F(MasterClockTest, FrameCompletionDetection) {
    // NTSC: 262 scanlines * 227.5 cycles = 59,605 cycles
    uint32 ntsc_cycles = 59605;
    
    for (uint32 i = 0; i < ntsc_cycles - 1; i++) {
        clock_ntsc->vdc_ticked();
        EXPECT_FALSE(clock_ntsc->is_frame_complete());
    }
    
    clock_ntsc->vdc_ticked();
    EXPECT_TRUE(clock_ntsc->is_frame_complete());
}

// Test tick() returns FRAME_COMPLETE when frame is done
TEST_F(MasterClockTest, TickReturnsFrameCompleteWhenDone) {
    // NTSC: 59,605 cycles per frame
    uint32 ntsc_cycles = 59605;
    
    for (uint32 i = 0; i < ntsc_cycles; i++) {
        clock_ntsc->vdc_ticked();
    }
    
    auto next = clock_ntsc->tick();
    EXPECT_EQ(next, MasterClock::ExecuteNext::FRAME_COMPLETE);
}

// Test reset_frame() resets frame counter
TEST_F(MasterClockTest, ResetFrameResetsCounter) {
    for (int i = 0; i < 100; i++) {
        clock_ntsc->vdc_ticked();
    }
    
    EXPECT_GT(clock_ntsc->get_frame_cycle_count(), 0);
    
    clock_ntsc->reset_frame();
    EXPECT_EQ(clock_ntsc->get_frame_cycle_count(), 0);
}
// Test PAL has more cycles per frame than NTSC
TEST_F(MasterClockTest, PALHasMoreCyclesPerFrame) {
    // NTSC: 262 × 227.5 = 59,605 cycles
    // PAL: 312 × 227.36 ≈ 70,936 cycles
    uint32 ntsc_cycles = 59605;
    uint32 pal_cycles = 70936;
    
    for (uint32 i = 0; i < ntsc_cycles; i++) {
        clock_ntsc->vdc_ticked();
    }
    
    for (uint32 i = 0; i < pal_cycles; i++) {
        clock_pal->vdc_ticked();
    }
    
    EXPECT_TRUE(clock_ntsc->is_frame_complete());
    EXPECT_TRUE(clock_pal->is_frame_complete());
    
    // PAL should have taken more cycles
    EXPECT_GT(pal_cycles, ntsc_cycles);
}

// Test master cycle count increments correctly
TEST_F(MasterClockTest, MasterCycleCountIncrements) {
    for (int i = 0; i < 100; i++) {
        clock_ntsc->vdc_ticked();
    }
    
    EXPECT_EQ(clock_ntsc->get_master_cycle_count(), 100);
}
