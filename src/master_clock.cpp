#include "master_clock.h"

namespace videopac {

MasterClock::MasterClock(VideoStandard standard)
    : standard_(standard)
    , master_tick_count_(0)
    , scanline_tick_(0)
    , current_scanline_(0)
    , current_frame_(0)
    , vblank_(true)           // Start in vblank (hardware powers up in vblank)
    , vblank_rising_edge_(false)
    , frame_complete_(false)
    , vdc_cycle_count_(0)
    , cpu_cycles_remaining_(0)
    , ticks_per_scanline_(0)
    , y_increment_tick_(0)
    , hblank_start_tick_(0)
    , hblank_end_tick_(0)
    , vblank_transition_tick_(0)
    , vdc_tick_divisor_(0)
    , cpu_tick_divisor_(0)
    , total_scanlines_(0)
    , vblank_start_line_(0)
    , vblank_end_line_(0)
{
    calculate_timing();
}

void MasterClock::calculate_timing() {
    if (standard_ == VideoStandard::NTSC) {
        ticks_per_scanline_ = NTSC_TICKS_PER_SCANLINE;
        y_increment_tick_ = NTSC_Y_INCREMENT_TICK;
        hblank_start_tick_ = NTSC_HBLANK_START_TICK;
        hblank_end_tick_ = NTSC_HBLANK_END_TICK;
        vblank_transition_tick_ = NTSC_VBLANK_TRANSITION_TICK;
        vdc_tick_divisor_ = NTSC_VDC_DIVISOR;
        cpu_tick_divisor_ = NTSC_CPU_DIVISOR;
        total_scanlines_ = NTSC_TOTAL_SCANLINES;
        vblank_start_line_ = NTSC_VBLANK_START_LINE;
        vblank_end_line_ = NTSC_VBLANK_END_LINE;
    } else {
        ticks_per_scanline_ = PAL_TICKS_PER_SCANLINE;
        y_increment_tick_ = PAL_Y_INCREMENT_TICK;
        hblank_start_tick_ = PAL_HBLANK_START_TICK;
        hblank_end_tick_ = PAL_HBLANK_END_TICK;
        vblank_transition_tick_ = PAL_VBLANK_TRANSITION_TICK;
        vdc_tick_divisor_ = PAL_VDC_DIVISOR;
        cpu_tick_divisor_ = PAL_CPU_DIVISOR;
        total_scanlines_ = PAL_TOTAL_SCANLINES;
        vblank_start_line_ = PAL_VBLANK_START_LINE;
        vblank_end_line_ = total_scanlines_ - 1;
    }
}

MasterClock::ExecuteNext MasterClock::tick() {
    master_tick_count_++;
    scanline_tick_++;
    vblank_rising_edge_ = false;
    frame_complete_ = false;
    
    // ── Y increment at tick 412 (NTSC) / 1030 (PAL) ──
    if (scanline_tick_ == y_increment_tick_) {
        current_scanline_++;
    }
    
    // ── Vblank transitions at tick 365 (NTSC) / 912 (PAL) ──
    if (scanline_tick_ == vblank_transition_tick_) {
        if (current_scanline_ == vblank_start_line_) {
            if (!vblank_) {
                vblank_ = true;
                vblank_rising_edge_ = true;
            }
        }
        // Y was just incremented to 264 at tick 412 of previous scanline,
        // but the reset happens at tick 365 of the scanline where Y==263.
        // Actually: Y increments at tick 412. On the scanline where Y becomes 263,
        // at tick 365 of THAT scanline (which is before tick 412), Y is still 263
        // from the previous increment. Then at tick 365, Y resets to 0 and vblank goes low.
        //
        // Wait — re-reading the doc more carefully:
        // "It increments every scanline at cycle 412, from scanlines 0 through 106h."
        // "Then, at the next increment point (cycle 412 on scanline 106h) it increments to 107h!"
        // "This lasts until cycle 365, where it is reset to 0, along with the falling edge of vblank."
        //
        // So: at tick 412 of the scanline where Y was 262, Y becomes 263.
        // Then on the NEXT scanline (which is "scanline 263"), at tick 365, Y resets to 0.
        // But wait — Y already incremented to 263 at tick 412 of the previous scanline.
        // The "next scanline" starts at tick 0 after the previous scanline's tick 454.
        // So Y=263 from tick 412 of one scanline through tick 365 of the next.
        //
        // At tick 365 of the scanline where Y==263: reset Y to 0, vblank goes low.
        if (current_scanline_ == vblank_end_line_) {
            current_scanline_ = 0;
            vblank_ = false;
            current_frame_++;
            frame_complete_ = true;
        }
    }
    
    // ── Scanline wrap ──
    if (scanline_tick_ >= ticks_per_scanline_) {
        scanline_tick_ = 0;
    }
    
    // ── Determine what executes ──
    bool vdc_ready = (master_tick_count_ % vdc_tick_divisor_) == 0;
    
    // CPU debt tracking: a 2-cycle instruction sets cpu_cycles_remaining_=1.
    // On the next CPU tick slot, we decrement and the CPU is NOT ready (still paying debt).
    // The CPU becomes ready on the tick slot AFTER the debt is fully paid.
    //
    // Example for a 2-cycle instruction (NTSC, cpu_tick every 20 master ticks):
    //   Tick N:   cpu_ready=true, execute → cpu_cycles_remaining_=1
    //   Tick N+1: decrement to 0, cpu_ready=false (debt just paid)
    //   Tick N+2: remaining=0, cpu_ready=true → execute next instruction
    //
    // This gives 2 tick slots per 2-cycle instruction (40 master ticks NTSC).
    bool cpu_tick = (master_tick_count_ % cpu_tick_divisor_) == 0;
    bool cpu_ready = false;
    if (cpu_tick) {
        if (cpu_cycles_remaining_ > 0) {
            cpu_cycles_remaining_--;
            // Not ready — this slot is consumed by the multi-cycle instruction
        } else {
            cpu_ready = true;
        }
    }
    
    if (cpu_ready && vdc_ready) return ExecuteNext::BOTH;
    if (cpu_ready) return ExecuteNext::CPU;
    if (vdc_ready) return ExecuteNext::VDC;
    return ExecuteNext::NONE;
}

uint8 MasterClock::get_beam_x_at_cpu_read() const {
    // The 8048 MOVX instruction takes 2 machine cycles; the RD strobe is
    // active during the second cycle. Offset in master ticks depends on standard.
    // NTSC: ~20 master ticks. PAL: ~45 master ticks (one CPU cycle).
    uint32 bus_read_offset = cpu_tick_divisor_;
    uint32 tick = scanline_tick_ + bus_read_offset;
    if (tick >= ticks_per_scanline_) tick = ticks_per_scanline_ - 1;
    
    if (standard_ == VideoStandard::NTSC) {
        return static_cast<uint8>((tick + ((tick > NTSC_Y_INCREMENT_TICK) ? 1 : 0)) >> 1);
    } else {
        uint32 equiv_tick = (tick * 2 + 2) / 5;
        return static_cast<uint8>((equiv_tick + ((equiv_tick > NTSC_Y_INCREMENT_TICK) ? 1 : 0)) >> 1);
    }
}

void MasterClock::cpu_executed(uint8 cycles) {
    cpu_cycles_remaining_ = cycles - 1;
}

void MasterClock::vdc_executed() {
    vdc_cycle_count_++;
}

void MasterClock::reset() {
    master_tick_count_ = 0;
    scanline_tick_ = 0;
    current_scanline_ = 0;
    current_frame_ = 0;
    vblank_ = true;  // Start in vblank
    vblank_rising_edge_ = false;
    frame_complete_ = false;
    vdc_cycle_count_ = 0;
    cpu_cycles_remaining_ = 0;
}

} // namespace videopac
