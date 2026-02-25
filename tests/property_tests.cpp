#include <gtest/gtest.h>
#include <rapidcheck.h>
#include "cpu.h"
#include "memory.h"
#include "vdc.h"
#include "master_clock.h"

using namespace videopac;

// Property-based tests using RapidCheck
// Feature: true-master-clock-timing

// ============================================================================
// Property 1: NTSC VDC ticks every 2 master ticks
// Validates: Requirements R2
// ============================================================================
TEST(MasterClockProperties, NTSCVDCTickInterval) {
    rc::check("Property 1: NTSC VDC ticks every 2 master ticks",
              [](uint16 num_ticks) {
        RC_PRE(num_ticks >= 2 && num_ticks <= 1000);
        
        MasterClock clock(VideoStandard::NTSC);
        
        int vdc_tick_count = 0;
        for (uint16 i = 0; i < num_ticks; i++) {
            auto result = clock.tick();
            if (result == MasterClock::ExecuteNext::VDC) {
                vdc_tick_count++;
                clock.vdc_executed();
            }
        }
        
        // VDC should tick every 2 master ticks
        int expected_vdc_ticks = static_cast<int>(num_ticks) / 2;
        RC_ASSERT(vdc_tick_count == expected_vdc_ticks);
    });
}

// ============================================================================
// Property 2: NTSC CPU ticks every 20 master ticks
// Validates: Requirements R3
// ============================================================================
TEST(MasterClockProperties, NTSCCPUTickInterval) {
    rc::check("Property 2: NTSC CPU ticks every 20 master ticks",
              [](uint16 num_ticks) {
        RC_PRE(num_ticks >= 20 && num_ticks <= 1000);
        
        MasterClock clock(VideoStandard::NTSC);
        
        int cpu_tick_count = 0;
        for (uint16 i = 0; i < num_ticks; i++) {
            auto result = clock.tick();
            if (result == MasterClock::ExecuteNext::CPU) {
                cpu_tick_count++;
                clock.cpu_executed();
            }
        }
        
        // CPU should tick every 20 master ticks
        int expected_cpu_ticks = static_cast<int>(num_ticks) / 20;
        RC_ASSERT(cpu_tick_count == expected_cpu_ticks);
    });
}

// ============================================================================
// Property 3: PAL VDC ticks every 5 master ticks
// Validates: Requirements R2
// ============================================================================
TEST(MasterClockProperties, PALVDCTickInterval) {
    rc::check("Property 3: PAL VDC ticks every 5 master ticks",
              [](uint16 num_ticks) {
        RC_PRE(num_ticks >= 5 && num_ticks <= 1000);
        
        MasterClock clock(VideoStandard::PAL);
        
        int vdc_tick_count = 0;
        for (uint16 i = 0; i < num_ticks; i++) {
            auto result = clock.tick();
            if (result == MasterClock::ExecuteNext::VDC) {
                vdc_tick_count++;
                clock.vdc_executed();
            }
        }
        
        // VDC should tick every 5 master ticks
        int expected_vdc_ticks = static_cast<int>(num_ticks) / 5;
        RC_ASSERT(vdc_tick_count == expected_vdc_ticks);
    });
}

// ============================================================================
// Property 4: PAL CPU ticks every 45 master ticks
// Validates: Requirements R3
// ============================================================================
TEST(MasterClockProperties, PALCPUTickInterval) {
    rc::check("Property 4: PAL CPU ticks every 45 master ticks",
              [](uint16 num_ticks) {
        RC_PRE(num_ticks >= 45 && num_ticks <= 2000);
        
        MasterClock clock(VideoStandard::PAL);
        
        int cpu_tick_count = 0;
        for (uint16 i = 0; i < num_ticks; i++) {
            auto result = clock.tick();
            if (result == MasterClock::ExecuteNext::CPU) {
                cpu_tick_count++;
                clock.cpu_executed();
            }
        }
        
        // CPU should tick every 45 master ticks
        int expected_cpu_ticks = static_cast<int>(num_ticks) / 45;
        RC_ASSERT(cpu_tick_count == expected_cpu_ticks);
    });
}

// ============================================================================
// Property 5: Scanline boundaries occur at correct intervals
// Validates: Requirements R4
// ============================================================================
TEST(MasterClockProperties, ScanlineBoundaries) {
    rc::check("Property 5: NTSC scanline boundaries occur every 455 ticks",
              [](uint8 num_scanlines) {
        RC_PRE(num_scanlines >= 1 && num_scanlines <= 10);
        
        MasterClock clock(VideoStandard::NTSC);
        
        // Tick through N scanlines
        for (uint8 scanline = 0; scanline < num_scanlines; scanline++) {
            for (int tick = 0; tick < 455; tick++) {
                clock.tick();
            }
        }
        
        // Should be at scanline N
        RC_ASSERT(clock.get_current_scanline() == static_cast<uint32>(num_scanlines));
        RC_ASSERT(clock.get_scanline_tick() == 0u);
    });
}
