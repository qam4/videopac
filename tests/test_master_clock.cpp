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

// Test cycle ratio is approximately 9.9
TEST_F(MasterClockTest, CycleRatioApproximately9Point9) {
    // Execute one CPU instruction
    clock_ntsc->cpu_executed(1);
    
    double vdc_debt = clock_ntsc->get_vdc_cycle_debt();
    
    // Should be approximately 9.9 VDC cycles per CPU instruction
    EXPECT_NEAR(vdc_debt, 9.9, 0.1);
}

// Test tick() returns CPU when debt threshold reached
TEST_F(MasterClockTest, TickReturnsCPUWhenDebtThresholdReached) {
    // Accumulate enough CPU debt (need ~9.9 VDC cycles per CPU instruction)
    for (int i = 0; i < 10; i++) {
        clock_ntsc->vdc_ticked();
    }
    
    // CPU debt should be >= 9.9 now (10 * (1/9.9) ≈ 1.01, which is < 9.9)
    // We need about 100 VDC ticks to get 10 CPU cycles of debt
    for (int i = 0; i < 90; i++) {
        clock_ntsc->vdc_ticked();
    }
    
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
    // NTSC: 262 scanlines * 227 cycles = 59474 cycles
    uint32 ntsc_cycles = 262 * 227;
    
    for (uint32 i = 0; i < ntsc_cycles - 1; i++) {
        clock_ntsc->vdc_ticked();
        EXPECT_FALSE(clock_ntsc->is_frame_complete());
    }
    
    clock_ntsc->vdc_ticked();
    EXPECT_TRUE(clock_ntsc->is_frame_complete());
}

// Test tick() returns FRAME_COMPLETE when frame is done
TEST_F(MasterClockTest, TickReturnsFrameCompleteWhenDone) {
    uint32 ntsc_cycles = 262 * 227;
    
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

// Test reset_frame() clears excessive debt
TEST_F(MasterClockTest, ResetFrameClearsExcessiveDebt) {
    // Create large negative debt
    for (int i = 0; i < 100; i++) {
        clock_ntsc->cpu_executed(1);
    }
    
    clock_ntsc->reset_frame();
    
    // Debt should be cleared or minimal
    EXPECT_GT(clock_ntsc->get_cpu_cycle_debt(), -20.0);
    EXPECT_GT(clock_ntsc->get_vdc_cycle_debt(), -20.0);
}

// Test PAL has more cycles per frame than NTSC
TEST_F(MasterClockTest, PALHasMoreCyclesPerFrame) {
    // Run both to completion
    uint32 ntsc_cycles = 262 * 227;
    uint32 pal_cycles = 312 * 227;
    
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
