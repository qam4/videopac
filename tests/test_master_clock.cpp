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
}

// Test VDC tick accumulates CPU debt
TEST_F(MasterClockTest, VDCTickAccumulatesCPUDebt) {
    clock_ntsc->vdc_ticked();
    
    EXPECT_EQ(clock_ntsc->get_master_cycle_count(), 1);
    EXPECT_GT(clock_ntsc->get_cpu_cycle_debt(), 0.0);
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

// Note: Frame completion tracking has been moved to VDC class.
// MasterClock no longer tracks frame cycles or completion.

// Test master cycle count increments correctly
TEST_F(MasterClockTest, MasterCycleCountIncrements) {
    for (int i = 0; i < 100; i++) {
        clock_ntsc->vdc_ticked();
    }
    
    EXPECT_EQ(clock_ntsc->get_master_cycle_count(), 100);
}
