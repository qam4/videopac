#include <gtest/gtest.h>
#include <rapidcheck.h>
#include "cpu.h"
#include "memory.h"
#include "vdc.h"
#include "master_clock.h"

using namespace videopac;

// Helper: run the master clock for N ticks, properly handling BOTH
static void run_clock(MasterClock& clock, int num_ticks, int& vdc_count, int& cpu_count) {
    vdc_count = 0;
    cpu_count = 0;
    for (int i = 0; i < num_ticks; i++) {
        auto result = clock.tick();
        if (result == MasterClock::ExecuteNext::VDC ||
            result == MasterClock::ExecuteNext::BOTH) {
            vdc_count++;
            clock.vdc_executed();
        }
        if (result == MasterClock::ExecuteNext::CPU ||
            result == MasterClock::ExecuteNext::BOTH) {
            cpu_count++;
            clock.cpu_executed();
        }
    }
}

// Property 1: NTSC VDC ticks every 2 master ticks
TEST(MasterClockProperties, NTSCVDCTickInterval) {
    rc::check("Property 1: NTSC VDC ticks every 2 master ticks",
              []() {
        auto num_ticks = *rc::gen::inRange(2, 1001);
        MasterClock clock(VideoStandard::NTSC);
        int vdc_count = 0, cpu_count = 0;
        run_clock(clock, num_ticks, vdc_count, cpu_count);
        RC_ASSERT(vdc_count == num_ticks / 2);
    });
}

// Property 2: NTSC CPU ticks every 20 master ticks
TEST(MasterClockProperties, NTSCCPUTickInterval) {
    rc::check("Property 2: NTSC CPU ticks every 20 master ticks",
              []() {
        auto num_ticks = *rc::gen::inRange(20, 1001);
        MasterClock clock(VideoStandard::NTSC);
        int vdc_count = 0, cpu_count = 0;
        run_clock(clock, num_ticks, vdc_count, cpu_count);
        RC_ASSERT(cpu_count == num_ticks / 20);
    });
}

// Property 3: PAL VDC ticks every 5 master ticks
TEST(MasterClockProperties, PALVDCTickInterval) {
    rc::check("Property 3: PAL VDC ticks every 5 master ticks",
              []() {
        auto num_ticks = *rc::gen::inRange(5, 1001);
        MasterClock clock(VideoStandard::PAL);
        int vdc_count = 0, cpu_count = 0;
        run_clock(clock, num_ticks, vdc_count, cpu_count);
        RC_ASSERT(vdc_count == num_ticks / 5);
    });
}

// Property 4: PAL CPU ticks every 45 master ticks
TEST(MasterClockProperties, PALCPUTickInterval) {
    rc::check("Property 4: PAL CPU ticks every 45 master ticks",
              []() {
        auto num_ticks = *rc::gen::inRange(45, 2001);
        MasterClock clock(VideoStandard::PAL);
        int vdc_count = 0, cpu_count = 0;
        run_clock(clock, num_ticks, vdc_count, cpu_count);
        RC_ASSERT(cpu_count == num_ticks / 45);
    });
}

// Property 5: Scanline boundaries occur at correct intervals
TEST(MasterClockProperties, ScanlineBoundaries) {
    rc::check("Property 5: NTSC scanline boundaries occur every 455 ticks",
              []() {
        auto num_scanlines = *rc::gen::inRange(1, 11);
        MasterClock clock(VideoStandard::NTSC);

        for (int scanline = 0; scanline < num_scanlines; scanline++) {
            for (int tick = 0; tick < 455; tick++) {
                clock.tick();
            }
        }

        RC_ASSERT(clock.get_current_scanline() == static_cast<uint32>(num_scanlines));
        RC_ASSERT(clock.get_scanline_tick() == 0u);
    });
}
