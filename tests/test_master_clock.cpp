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
    EXPECT_EQ(clock_ntsc->get_master_tick_count(), 0);
    EXPECT_EQ(clock_ntsc->get_current_scanline(), 0);
    EXPECT_EQ(clock_ntsc->get_scanline_tick(), 0);
    EXPECT_EQ(clock_ntsc->get_vdc_cycle_count(), 0);
}

// Test NTSC timing - VDC ticks every 2 master ticks
TEST_F(MasterClockTest, NTSCVDCTicksEvery2MasterTicks) {
    // First tick - should return VDC (tick 1, divisible by 2? No, but we start at 0)
    // Actually, after first tick(), master_tick_count_ becomes 1
    // 1 % 2 = 1 (not divisible), so no VDC
    // Let's tick twice to get VDC
    
    auto result1 = clock_ntsc->tick();  // tick 1
    EXPECT_EQ(result1, MasterClock::ExecuteNext::NONE);
    
    auto result2 = clock_ntsc->tick();  // tick 2
    EXPECT_EQ(result2, MasterClock::ExecuteNext::VDC);
    
    // VDC should have executed once
    clock_ntsc->vdc_executed();
    EXPECT_EQ(clock_ntsc->get_vdc_cycle_count(), 1);
}

// Test NTSC timing - CPU ticks every 20 master ticks
TEST_F(MasterClockTest, NTSCCPUTicksEvery20MasterTicks) {
    // Tick 20 times to get CPU
    for (int i = 0; i < 19; i++) {
        clock_ntsc->tick();
        // Should get VDC on even ticks (2, 4, 6, ..., 18)
        // Should get NONE on odd ticks (1, 3, 5, ..., 19)
    }
    
    auto result = clock_ntsc->tick();  // tick 20
    // At tick 20, both CPU (20 % 20 == 0) and VDC (20 % 2 == 0) are ready
    EXPECT_EQ(result, MasterClock::ExecuteNext::BOTH);
}

// Test PAL timing - VDC ticks every 5 master ticks
TEST_F(MasterClockTest, PALVDCTicksEvery5MasterTicks) {
    // Tick 5 times
    for (int i = 0; i < 4; i++) {
        clock_pal->tick();
    }
    
    auto result = clock_pal->tick();  // tick 5
    EXPECT_EQ(result, MasterClock::ExecuteNext::VDC);
}

// Test PAL timing - CPU ticks every 45 master ticks
TEST_F(MasterClockTest, PALCPUTicksEvery45MasterTicks) {
    // Tick 45 times
    for (int i = 0; i < 44; i++) {
        clock_pal->tick();
    }
    
    auto result = clock_pal->tick();  // tick 45
    // At tick 45, both CPU (45 % 45 == 0) and VDC (45 % 5 == 0) are ready
    EXPECT_EQ(result, MasterClock::ExecuteNext::BOTH);
}

// Test scanline boundaries - NTSC has 455 ticks per scanline
TEST_F(MasterClockTest, NTSCScanlineBoundary) {
    EXPECT_EQ(clock_ntsc->get_current_scanline(), 0);
    
    // Tick 455 times (one full scanline)
    for (int i = 0; i < 455; i++) {
        clock_ntsc->tick();
    }
    
    EXPECT_EQ(clock_ntsc->get_current_scanline(), 1);
    EXPECT_EQ(clock_ntsc->get_scanline_tick(), 0);
}

// Test scanline boundaries - PAL has 1135 ticks per scanline
TEST_F(MasterClockTest, PALScanlineBoundary) {
    EXPECT_EQ(clock_pal->get_current_scanline(), 0);
    
    // Tick 1135 times (one full scanline)
    for (int i = 0; i < 1135; i++) {
        clock_pal->tick();
    }
    
    EXPECT_EQ(clock_pal->get_current_scanline(), 1);
    EXPECT_EQ(clock_pal->get_scanline_tick(), 0);
}

// Test frame boundaries - NTSC has 262 scanlines
TEST_F(MasterClockTest, NTSCFrameBoundary) {
    EXPECT_EQ(clock_ntsc->get_current_frame(), 0);
    
    // Tick through entire frame (455 * 262 = 119,210 ticks)
    for (int i = 0; i < 119210; i++) {
        clock_ntsc->tick();
    }
    
    EXPECT_EQ(clock_ntsc->get_current_frame(), 1);
    EXPECT_EQ(clock_ntsc->get_current_scanline(), 0);
}

// Test reset functionality
TEST_F(MasterClockTest, ResetClearsState) {
    // Advance clock
    for (int i = 0; i < 1000; i++) {
        clock_ntsc->tick();
    }
    
    // Reset
    clock_ntsc->reset();
    
    EXPECT_EQ(clock_ntsc->get_master_tick_count(), 0);
    EXPECT_EQ(clock_ntsc->get_current_scanline(), 0);
    EXPECT_EQ(clock_ntsc->get_scanline_tick(), 0);
    EXPECT_EQ(clock_ntsc->get_vdc_cycle_count(), 0);
}

// Test VDC cycle count accumulation
TEST_F(MasterClockTest, VDCCycleCountAccumulates) {
    // Run for 100 VDC cycles (200 master ticks for NTSC)
    int vdc_count = 0;
    for (int i = 0; i < 200; i++) {
        auto result = clock_ntsc->tick();
        if (result == MasterClock::ExecuteNext::VDC) {
            clock_ntsc->vdc_executed();
            vdc_count++;
        } else if (result == MasterClock::ExecuteNext::BOTH) {
            // Both CPU and VDC execute
            clock_ntsc->cpu_executed();
            clock_ntsc->vdc_executed();
            vdc_count++;
        } else if (result == MasterClock::ExecuteNext::CPU) {
            clock_ntsc->cpu_executed();
        }
    }
    
    EXPECT_EQ(clock_ntsc->get_vdc_cycle_count(), vdc_count);
    EXPECT_EQ(vdc_count, 100);  // 200 ticks / 2 = 100 VDC cycles
}

// ============================================================================
// Integration Tests - Full Frame Validation
// Task 2.2: Test full NTSC/PAL frames for timing accuracy
// ============================================================================

// Test full NTSC frame produces correct VDC cycle count
TEST_F(MasterClockTest, NTSCFullFrameVDCCycles) {
    // NTSC: 119,210 master ticks = 59,605 VDC cycles (119,210 / 2)
    int vdc_count = 0;
    int cpu_count = 0;
    
    for (int i = 0; i < 119210; i++) {
        auto result = clock_ntsc->tick();
        if (result == MasterClock::ExecuteNext::VDC) {
            clock_ntsc->vdc_executed();
            vdc_count++;
        } else if (result == MasterClock::ExecuteNext::CPU) {
            clock_ntsc->cpu_executed();
            cpu_count++;
        } else if (result == MasterClock::ExecuteNext::BOTH) {
            clock_ntsc->cpu_executed();
            clock_ntsc->vdc_executed();
            vdc_count++;
            cpu_count++;
        }
    }
    
    // Verify VDC cycle count
    EXPECT_EQ(vdc_count, 59605);  // 119,210 / 2
    EXPECT_EQ(clock_ntsc->get_vdc_cycle_count(), 59605);
    
    // Verify CPU instruction count
    EXPECT_EQ(cpu_count, 5960);  // 119,210 / 20 (with rounding)
    
    // Verify frame boundary
    EXPECT_EQ(clock_ntsc->get_current_frame(), 1);
    EXPECT_EQ(clock_ntsc->get_current_scanline(), 0);
}

// Test full PAL frame produces correct VDC cycle count
TEST_F(MasterClockTest, PALFullFrameVDCCycles) {
    // PAL: 354,120 master ticks = 70,824 VDC cycles (354,120 / 5)
    int vdc_count = 0;
    int cpu_count = 0;
    
    for (int i = 0; i < 354120; i++) {
        auto result = clock_pal->tick();
        if (result == MasterClock::ExecuteNext::VDC) {
            clock_pal->vdc_executed();
            vdc_count++;
        } else if (result == MasterClock::ExecuteNext::CPU) {
            clock_pal->cpu_executed();
            cpu_count++;
        } else if (result == MasterClock::ExecuteNext::BOTH) {
            clock_pal->cpu_executed();
            clock_pal->vdc_executed();
            vdc_count++;
            cpu_count++;
        }
    }
    
    // Verify VDC cycle count
    EXPECT_EQ(vdc_count, 70824);  // 354,120 / 5
    EXPECT_EQ(clock_pal->get_vdc_cycle_count(), 70824);
    
    // Verify CPU instruction count
    EXPECT_EQ(cpu_count, 7869);  // 354,120 / 45 (with rounding)
    
    // Verify frame boundary
    EXPECT_EQ(clock_pal->get_current_frame(), 1);
    EXPECT_EQ(clock_pal->get_current_scanline(), 0);
}

// Test 1000-frame run for timing drift (NTSC)
TEST_F(MasterClockTest, NTSCNoTimingDriftOver1000Frames) {
    const int FRAMES = 1000;
    const int TICKS_PER_FRAME = 119210;
    const int EXPECTED_VDC_CYCLES_PER_FRAME = 59605;
    
    for (int frame = 0; frame < FRAMES; frame++) {
        uint64 vdc_start = clock_ntsc->get_vdc_cycle_count();
        
        for (int i = 0; i < TICKS_PER_FRAME; i++) {
            auto result = clock_ntsc->tick();
            if (result == MasterClock::ExecuteNext::VDC) {
                clock_ntsc->vdc_executed();
            } else if (result == MasterClock::ExecuteNext::CPU) {
                clock_ntsc->cpu_executed();
            } else if (result == MasterClock::ExecuteNext::BOTH) {
                clock_ntsc->cpu_executed();
                clock_ntsc->vdc_executed();
            }
        }
        
        uint64 vdc_end = clock_ntsc->get_vdc_cycle_count();
        uint64 vdc_this_frame = vdc_end - vdc_start;
        
        // Each frame should have exactly the same number of VDC cycles
        EXPECT_EQ(vdc_this_frame, EXPECTED_VDC_CYCLES_PER_FRAME) 
            << "Frame " << frame << " had incorrect VDC cycle count";
    }
    
    // Verify total counts
    EXPECT_EQ(clock_ntsc->get_current_frame(), FRAMES);
    EXPECT_EQ(clock_ntsc->get_vdc_cycle_count(), EXPECTED_VDC_CYCLES_PER_FRAME * FRAMES);
}

// Test 1000-frame run for timing drift (PAL)
TEST_F(MasterClockTest, PALNoTimingDriftOver1000Frames) {
    const int FRAMES = 1000;
    const int TICKS_PER_FRAME = 354120;
    const int EXPECTED_VDC_CYCLES_PER_FRAME = 70824;
    
    for (int frame = 0; frame < FRAMES; frame++) {
        uint64 vdc_start = clock_pal->get_vdc_cycle_count();
        
        for (int i = 0; i < TICKS_PER_FRAME; i++) {
            auto result = clock_pal->tick();
            if (result == MasterClock::ExecuteNext::VDC) {
                clock_pal->vdc_executed();
            } else if (result == MasterClock::ExecuteNext::CPU) {
                clock_pal->cpu_executed();
            } else if (result == MasterClock::ExecuteNext::BOTH) {
                clock_pal->cpu_executed();
                clock_pal->vdc_executed();
            }
        }
        
        uint64 vdc_end = clock_pal->get_vdc_cycle_count();
        uint64 vdc_this_frame = vdc_end - vdc_start;
        
        // Each frame should have exactly the same number of VDC cycles
        EXPECT_EQ(vdc_this_frame, EXPECTED_VDC_CYCLES_PER_FRAME) 
            << "Frame " << frame << " had incorrect VDC cycle count";
    }
    
    // Verify total counts
    EXPECT_EQ(clock_pal->get_current_frame(), FRAMES);
    EXPECT_EQ(clock_pal->get_vdc_cycle_count(), EXPECTED_VDC_CYCLES_PER_FRAME * FRAMES);
}

// Test NTSC scanline VDC cycle count (227.5 average)
TEST_F(MasterClockTest, NTSCScanlineVDCCycles) {
    // NTSC: 455 ticks per scanline = 227.5 VDC cycles average
    // Should alternate between 227 and 228 VDC cycles per scanline
    
    for (int scanline = 0; scanline < 10; scanline++) {
        uint64 vdc_start = clock_ntsc->get_vdc_cycle_count();
        
        for (int i = 0; i < 455; i++) {
            auto result = clock_ntsc->tick();
            if (result == MasterClock::ExecuteNext::VDC) {
                clock_ntsc->vdc_executed();
            } else if (result == MasterClock::ExecuteNext::CPU) {
                clock_ntsc->cpu_executed();
            } else if (result == MasterClock::ExecuteNext::BOTH) {
                clock_ntsc->cpu_executed();
                clock_ntsc->vdc_executed();
            }
        }
        
        uint64 vdc_end = clock_ntsc->get_vdc_cycle_count();
        uint64 vdc_this_scanline = vdc_end - vdc_start;
        
        // Should be either 227 or 228 VDC cycles
        EXPECT_TRUE(vdc_this_scanline == 227 || vdc_this_scanline == 228)
            << "Scanline " << scanline << " had " << vdc_this_scanline << " VDC cycles";
    }
}

// Test PAL scanline VDC cycle count (exactly 227)
TEST_F(MasterClockTest, PALScanlineVDCCycles) {
    // PAL: 1135 ticks per scanline = exactly 227 VDC cycles
    
    for (int scanline = 0; scanline < 10; scanline++) {
        uint64 vdc_start = clock_pal->get_vdc_cycle_count();
        
        for (int i = 0; i < 1135; i++) {
            auto result = clock_pal->tick();
            if (result == MasterClock::ExecuteNext::VDC) {
                clock_pal->vdc_executed();
            } else if (result == MasterClock::ExecuteNext::CPU) {
                clock_pal->cpu_executed();
            } else if (result == MasterClock::ExecuteNext::BOTH) {
                clock_pal->cpu_executed();
                clock_pal->vdc_executed();
            }
        }
        
        uint64 vdc_end = clock_pal->get_vdc_cycle_count();
        uint64 vdc_this_scanline = vdc_end - vdc_start;
        
        // Should be exactly 227 VDC cycles (perfect integer ratio)
        EXPECT_EQ(vdc_this_scanline, 227)
            << "Scanline " << scanline << " had " << vdc_this_scanline << " VDC cycles";
    }
}
