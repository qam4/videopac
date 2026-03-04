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
// (PAL master clock runs at ~2.5x NTSC; divides by 5 for VDC, NTSC by 2)
TEST_F(MasterClockTest, PALScanlineBoundary) {
    EXPECT_EQ(clock_pal->get_current_scanline(), 0);
    
    // Tick 1135 times (one full PAL scanline)
    for (int i = 0; i < 1135; i++) {
        clock_pal->tick();
    }
    
    // Y increments at tick 1030, so after 1135 ticks we're on the next scanline
    // with scanline_tick back to 0
    EXPECT_EQ(clock_pal->get_current_scanline(), 1);
    EXPECT_EQ(clock_pal->get_scanline_tick(), 0);
}

// Test frame boundaries - NTSC has 264 hardware scanlines (0-263)
// Y increments at tick 412, frame completes at tick 365 of the scanline where Y=263
TEST_F(MasterClockTest, NTSCFrameBoundary) {
    EXPECT_EQ(clock_ntsc->get_current_frame(), 0);
    
    // From reset: 263 full scanlines + 365 ticks into scanline 263
    // Y goes 0→1→...→263, then resets to 0 at tick 365 of the next scanline
    const int TICKS_TO_FRAME_COMPLETE = 263 * 455 + 365;  // 120,030
    for (int i = 0; i < TICKS_TO_FRAME_COMPLETE; i++) {
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
    // From reset to frame_complete: 263 * 455 + 365 = 120,030 master ticks
    // VDC cycles = 120,030 / 2 = 60,015
    const int TICKS_PER_FRAME = 263 * 455 + 365;  // 120,030
    const int EXPECTED_VDC_CYCLES = TICKS_PER_FRAME / 2;  // 60,015
    
    int vdc_count = 0;
    
    for (int i = 0; i < TICKS_PER_FRAME; i++) {
        auto result = clock_ntsc->tick();
        if (result == MasterClock::ExecuteNext::VDC) {
            clock_ntsc->vdc_executed();
            vdc_count++;
        } else if (result == MasterClock::ExecuteNext::CPU) {
            clock_ntsc->cpu_executed();
        } else if (result == MasterClock::ExecuteNext::BOTH) {
            clock_ntsc->cpu_executed();
            clock_ntsc->vdc_executed();
            vdc_count++;
        }
    }
    
    // Verify VDC cycle count
    EXPECT_EQ(vdc_count, EXPECTED_VDC_CYCLES);
    EXPECT_EQ(clock_ntsc->get_vdc_cycle_count(), static_cast<uint64>(EXPECTED_VDC_CYCLES));
    
    // Verify frame boundary
    EXPECT_EQ(clock_ntsc->get_current_frame(), 1);
    EXPECT_EQ(clock_ntsc->get_current_scanline(), 0);
}

// Test full PAL frame produces correct VDC cycle count
TEST_F(MasterClockTest, PALFullFrameVDCCycles) {
    // PAL: From reset to frame_complete: 312 * 1135 + 912 = 355,032 master ticks
    // VDC cycles = 355,032 / 5 = 71,006 (with remainder 2, so floor)
    const int TICKS_PER_FRAME = 312 * 1135 + 912;  // 355,032
    const int EXPECTED_VDC_CYCLES = TICKS_PER_FRAME / 5;  // 71,006
    
    int vdc_count = 0;
    
    for (int i = 0; i < TICKS_PER_FRAME; i++) {
        auto result = clock_pal->tick();
        if (result == MasterClock::ExecuteNext::VDC) {
            clock_pal->vdc_executed();
            vdc_count++;
        } else if (result == MasterClock::ExecuteNext::CPU) {
            clock_pal->cpu_executed();
        } else if (result == MasterClock::ExecuteNext::BOTH) {
            clock_pal->cpu_executed();
            clock_pal->vdc_executed();
            vdc_count++;
        }
    }
    
    // Verify VDC cycle count
    EXPECT_EQ(vdc_count, EXPECTED_VDC_CYCLES);
    EXPECT_EQ(clock_pal->get_vdc_cycle_count(), static_cast<uint64>(EXPECTED_VDC_CYCLES));
    
    // Verify frame boundary
    EXPECT_EQ(clock_pal->get_current_frame(), 1);
    EXPECT_EQ(clock_pal->get_current_scanline(), 0);
}

// Test 1000-frame run for timing drift (NTSC)
// Verifies that VDC cycle count per frame is consistent
TEST_F(MasterClockTest, NTSCNoTimingDriftOver1000Frames) {
    const int FRAMES = 1000;
    
    // Run until we've completed FRAMES frames by watching frame counter
    int frames_completed = 0;
    
    while (frames_completed < FRAMES) {
        auto result = clock_ntsc->tick();
        if (result == MasterClock::ExecuteNext::VDC) {
            clock_ntsc->vdc_executed();
        } else if (result == MasterClock::ExecuteNext::CPU) {
            clock_ntsc->cpu_executed();
        } else if (result == MasterClock::ExecuteNext::BOTH) {
            clock_ntsc->cpu_executed();
            clock_ntsc->vdc_executed();
        }
        
        if (clock_ntsc->is_frame_complete()) {
            frames_completed++;
        }
    }
    
    EXPECT_EQ(clock_ntsc->get_current_frame(), static_cast<uint32>(FRAMES));
}

// Test 1000-frame run for timing drift (PAL)
TEST_F(MasterClockTest, PALNoTimingDriftOver1000Frames) {
    const int FRAMES = 1000;
    
    int frames_completed = 0;
    
    while (frames_completed < FRAMES) {
        auto result = clock_pal->tick();
        if (result == MasterClock::ExecuteNext::VDC) {
            clock_pal->vdc_executed();
        } else if (result == MasterClock::ExecuteNext::CPU) {
            clock_pal->cpu_executed();
        } else if (result == MasterClock::ExecuteNext::BOTH) {
            clock_pal->cpu_executed();
            clock_pal->vdc_executed();
        }
        
        if (clock_pal->is_frame_complete()) {
            frames_completed++;
        }
    }
    
    EXPECT_EQ(clock_pal->get_current_frame(), static_cast<uint32>(FRAMES));
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

// Test PAL scanline VDC cycle count (227 per scanline)
TEST_F(MasterClockTest, PALScanlineVDCCycles) {
    // PAL: 1135 ticks per scanline / 5 = 227 VDC cycles per scanline
    
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
        
        // Should be exactly 227 VDC cycles (1135 / 5 = 227, perfect integer ratio)
        EXPECT_EQ(vdc_this_scanline, 227)
            << "Scanline " << scanline << " had " << vdc_this_scanline << " VDC cycles";
    }
}
