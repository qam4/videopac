#include "master_clock.h"

namespace videopac {

MasterClock::MasterClock(VideoStandard standard)
    : standard_(standard)
    , master_tick_count_(0)
    , scanline_tick_(0)
    , current_scanline_(0)
    , current_frame_(0)
    , vdc_cycle_count_(0)
    , cpu_cycles_remaining_(0)
    , ticks_per_scanline_(0)
    , vdc_tick_divisor_(0)
    , cpu_tick_divisor_(0)
    , total_scanlines_(0)
{
    calculate_timing();
}

void MasterClock::calculate_timing() {
    // Set standard-specific timing constants
    if (standard_ == VideoStandard::NTSC) {
        ticks_per_scanline_ = NTSC_TICKS_PER_SCANLINE;  // 455
        vdc_tick_divisor_ = NTSC_VDC_TICK_DIVISOR;      // 2
        cpu_tick_divisor_ = NTSC_CPU_TICK_DIVISOR;      // 20
        total_scanlines_ = NTSC_SCANLINES_PER_FRAME;    // 262
    } else {  // PAL
        ticks_per_scanline_ = PAL_TICKS_PER_SCANLINE;   // 1135
        vdc_tick_divisor_ = PAL_VDC_TICK_DIVISOR;       // 5
        cpu_tick_divisor_ = PAL_CPU_TICK_DIVISOR;       // 45
        total_scanlines_ = PAL_SCANLINES_PER_FRAME;     // 312
    }
}

MasterClock::ExecuteNext MasterClock::tick() {
    // Advance master clock by 1 tick
    master_tick_count_++;
    scanline_tick_++;
    
    // Check for scanline boundary
    if (scanline_tick_ >= ticks_per_scanline_) {
        scanline_tick_ = 0;
        current_scanline_++;
        
        // Check for frame boundary
        if (current_scanline_ >= total_scanlines_) {
            current_scanline_ = 0;
            current_frame_++;
        }
    }
    
    // Determine what should execute at this tick
    // Both CPU and VDC can be ready at the same tick (every LCM of divisors)
    // CPU is only ready if it has no remaining cycles from a multi-cycle instruction
    bool cpu_ready = (cpu_cycles_remaining_ == 0) && ((master_tick_count_ % cpu_tick_divisor_) == 0);
    bool vdc_ready = (master_tick_count_ % vdc_tick_divisor_) == 0;
    
    // Decrement CPU cycle debt if we're at a CPU tick boundary
    if ((master_tick_count_ % cpu_tick_divisor_) == 0 && cpu_cycles_remaining_ > 0) {
        cpu_cycles_remaining_--;
    }
    
    if (cpu_ready && vdc_ready) {
        return ExecuteNext::BOTH;
    }
    
    if (cpu_ready) {
        return ExecuteNext::CPU;
    }
    
    if (vdc_ready) {
        return ExecuteNext::VDC;
    }
    
    // Neither ready, continue to next tick
    return ExecuteNext::NONE;
}

void MasterClock::cpu_executed(uint8 cycles) {
    // CPU executed an instruction that consumed 'cycles' CPU cycles (1 or 2)
    // Set remaining cycles so CPU won't execute again until debt is paid
    // Subtract 1 because the current cycle is already consumed
    cpu_cycles_remaining_ = cycles - 1;
}

void MasterClock::vdc_executed() {
    // VDC advanced one cycle
    vdc_cycle_count_++;
}

void MasterClock::reset() {
    master_tick_count_ = 0;
    scanline_tick_ = 0;
    current_scanline_ = 0;
    current_frame_ = 0;
    vdc_cycle_count_ = 0;
    cpu_cycles_remaining_ = 0;
}

} // namespace videopac
