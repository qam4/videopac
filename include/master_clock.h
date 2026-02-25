#ifndef VIDEOPAC_MASTER_CLOCK_H
#define VIDEOPAC_MASTER_CLOCK_H

#include "types.h"

namespace videopac {

// Master clock synchronization for cycle-accurate emulation
// Uses true hardware master clock (7.15909 MHz NTSC, 17.734476 MHz PAL)
// Both CPU and VDC derive their timing from this master clock via integer divisors
// Reference: .kiro/specs/true-master-clock-timing/design.md
class MasterClock {
public:
    explicit MasterClock(VideoStandard standard);
    
    // Execution control
    enum class ExecuteNext {
        CPU,             // CPU should execute next instruction
        VDC,             // VDC should advance one cycle
        BOTH,            // Both CPU and VDC should execute (happens every LCM of divisors)
        NONE             // Neither ready (continue to next tick)
    };
    
    // Advance the master clock by 1 tick and determine what to execute next
    ExecuteNext tick();
    
    // Notify that component executed (no parameters needed with master clock approach)
    void cpu_executed();
    void vdc_executed();
    
    // Reset master clock (for emulator reset)
    void reset();
    
    // Accessors for testing and debugging
    uint64 get_master_tick_count() const { return master_tick_count_; }
    uint32 get_current_scanline() const { return current_scanline_; }
    uint32 get_scanline_tick() const { return scanline_tick_; }
    uint32 get_current_frame() const { return current_frame_; }
    
    // Get VDC cycle count (for compatibility with existing code)
    uint64 get_vdc_cycle_count() const { return vdc_cycle_count_; }
    
private:
    VideoStandard standard_;
    
    // Master clock state
    uint64 master_tick_count_;      // Total master ticks since start
    uint32 scanline_tick_;          // Tick within current scanline (0 to ticks_per_scanline-1)
    uint32 current_scanline_;       // Current scanline number (0 to total_scanlines-1)
    uint32 current_frame_;          // Current frame number
    
    // VDC cycle counter (for compatibility)
    uint64 vdc_cycle_count_;        // Total VDC cycles (increments every vdc_tick_divisor ticks)
    
    // Standard-specific timing constants
    uint32 ticks_per_scanline_;     // Master ticks per scanline (455 NTSC, 1135 PAL)
    uint32 vdc_tick_divisor_;       // VDC ticks every N master ticks (2 NTSC, 5 PAL)
    uint32 cpu_tick_divisor_;       // CPU ticks every M master ticks (20 NTSC, 45 PAL)
    uint32 total_scanlines_;        // Scanlines per frame (262 NTSC, 312/313 PAL)
    
    // NTSC timing constants
    static constexpr uint32 NTSC_MASTER_CLOCK_HZ = 7159090;      // 7.15909 MHz
    static constexpr uint32 NTSC_TICKS_PER_SCANLINE = 455;
    static constexpr uint32 NTSC_SCANLINES_PER_FRAME = 262;
    static constexpr uint32 NTSC_TICKS_PER_FRAME = 119210;       // 455 × 262
    static constexpr uint32 NTSC_VDC_TICK_DIVISOR = 2;           // VDC ticks every 2 master ticks
    static constexpr uint32 NTSC_CPU_TICK_DIVISOR = 20;          // CPU ticks every 20 master ticks
    
    // PAL timing constants
    static constexpr uint32 PAL_MASTER_CLOCK_HZ = 17734476;      // 17.734476 MHz
    static constexpr uint32 PAL_TICKS_PER_SCANLINE = 1135;
    static constexpr uint32 PAL_SCANLINES_PER_FRAME = 312;       // (or 313 for alternate frames)
    static constexpr uint32 PAL_TICKS_PER_FRAME = 354120;        // 1135 × 312
    static constexpr uint32 PAL_VDC_TICK_DIVISOR = 5;            // VDC ticks every 5 master ticks
    static constexpr uint32 PAL_CPU_TICK_DIVISOR = 45;           // CPU ticks every 45 master ticks
    
    // Derived constants (for documentation)
    // NTSC: VDC = 3.579545 MHz (master ÷ 2), CPU = 0.357954 MHz (master ÷ 20)
    // PAL:  VDC = 3.546895 MHz (master ÷ 5), CPU = 0.394099 MHz (master ÷ 45)
    
    // Initialize timing constants based on video standard
    void calculate_timing();
};

} // namespace videopac

#endif // VIDEOPAC_MASTER_CLOCK_H
