#ifndef VIDEOPAC_MASTER_CLOCK_H
#define VIDEOPAC_MASTER_CLOCK_H

#include "types.h"

namespace videopac {

// Master clock for cycle-accurate Odyssey² emulation.
//
// This is the SINGLE SOURCE OF TRUTH for all timing in the system.
// The real 8244/8245 has one master oscillator from which everything derives:
//   - X counter (beam horizontal position)
//   - Y counter (scanline number)
//   - Hblank, Vblank, Hsync, Vsync signals
//   - CPU clock (master ÷ 20 NTSC, master ÷ 45 PAL)
//   - VDC pixel clock (master ÷ 2 NTSC, master ÷ 5 PAL)
//
// Hardware reference: doc/hardware/odyssey2_timing.txt
//
// The internal counter counts 0-454 per scanline (455 master ticks).
// There is NO 227/228 alternation — every scanline is exactly 455 ticks.
// "doubling to 454 cycles will make calculating things easy" — odyssey2_timing.txt
//
// Y increments at tick 412 (not at scanline boundary).
// Y goes 0 through 262, then briefly 263, then resets to 0 at tick 365
// of scanline 263.
//
// Vblank goes high at tick 365 of scanline 242.
// Vblank goes low at tick 365 of scanline 263/0.
class MasterClock {
public:
    explicit MasterClock(VideoStandard standard);
    
    // Execution control
    enum class ExecuteNext {
        CPU,
        VDC,
        BOTH,
        NONE
    };
    
    // Advance master clock by 1 tick. Returns what should execute.
    ExecuteNext tick();
    
    // Notify that CPU executed (for multi-cycle instruction debt tracking)
    void cpu_executed(uint8 cycles = 1);
    void vdc_executed();
    
    void reset();
    
    // ── Beam position (derived from counter) ──
    
    // Current master tick within scanline (0-454)
    uint32 get_scanline_tick() const { return scanline_tick_; }
    
    // Current scanline number (Y counter). This is the hardware Y value.
    // Range: 0-263 (NTSC). Only lower 8 bits are visible to CPU.
    uint16 get_scanline() const { return current_scanline_; }
    uint16 get_current_scanline() const { return current_scanline_; }  // Alias for tests
    
    // X register value derived from counter (hardware formula).
    // Xvalue = (tick + ((tick > 412) ? 1 : 0)) >> 1
    // Same formula for NTSC and PAL — both use 455 ticks per scanline.
    // Reference: odyssey2_timing.txt (NTSC verified, PAL assumed same mapping)
    uint8 get_beam_x() const;
    
    // X register value at the point where CPU bus read samples data.
    // The 8048 MOVX RD strobe is active ~20 master ticks after instruction start.
    uint8 get_beam_x_at_cpu_read() const;
    
    // ── Blanking signals (derived from counter) ──
    
    // Hblank = (tick > 365) && (tick < 453)
    // Reference: odyssey2_timing.txt scanline timing table
    bool is_hblank() const;
    
    // Vblank state — tracks the hardware flip-flop.
    // Goes high at tick 365 of scanline 242.
    // Goes low at tick 365 of scanline 263/0.
    bool is_vblank() const { return vblank_; }
    
    // Vblank just transitioned from low to high THIS tick
    bool vblank_rising_edge() const { return vblank_rising_edge_; }
    
    // T1 = !(Hblank OR Vblank). HIGH during visible, LOW during blanking.
    bool get_t1_state() const { return !(is_hblank() || vblank_); }
    
    // Frame just completed (Y reset to 0) — true for one tick
    bool is_frame_complete() const { return frame_complete_; }
    
    // Clear frame_complete flag (called at start of new frame by emulator)
    void clear_frame_complete() { frame_complete_ = false; }
    
    // ── Timing constants ──
    
    uint32 get_ticks_per_scanline() const { return ticks_per_scanline_; }
    uint32 get_total_scanlines() const { return total_scanlines_; }
    uint32 get_vdc_tick_divisor() const { return vdc_tick_divisor_; }
    uint32 get_current_frame() const { return current_frame_; }
    uint64 get_master_tick_count() const { return master_tick_count_; }
    uint64 get_vdc_cycle_count() const { return vdc_cycle_count_; }
    
    // Hardware constants (from odyssey2_timing.txt — NTSC verified)
    // PAL values are scaled by the ratio 1135/455 (PAL master clock is ~2.5x NTSC)
    static constexpr uint32 NTSC_TICKS_PER_SCANLINE = 455;
    static constexpr uint32 PAL_TICKS_PER_SCANLINE = 1135;
    
    // NTSC scanline event ticks (from odyssey2_timing.txt)
    static constexpr uint32 NTSC_Y_INCREMENT_TICK = 412;
    static constexpr uint32 NTSC_HBLANK_START_TICK = 366;
    static constexpr uint32 NTSC_HBLANK_END_TICK = 453;
    static constexpr uint32 NTSC_VBLANK_TRANSITION_TICK = 365;
    
    // PAL scanline event ticks (scaled from NTSC: multiply by 1135/455 ≈ 2.4945)
    // These are at the same VDC-cycle positions, just in PAL master ticks.
    // VDC cycle = master_tick / divisor. NTSC divisor=2, PAL divisor=5.
    // So PAL_tick = (NTSC_tick / 2) * 5 = NTSC_tick * 5 / 2
    static constexpr uint32 PAL_Y_INCREMENT_TICK = 1030;     // 412 * 5 / 2
    static constexpr uint32 PAL_HBLANK_START_TICK = 915;     // 366 * 5 / 2
    static constexpr uint32 PAL_HBLANK_END_TICK = 1133;      // 453 * 5 / 2 ≈ 1132.5, round up
    static constexpr uint32 PAL_VBLANK_TRANSITION_TICK = 912; // 365 * 5 / 2 = 912.5, round down
    
    // Clock divisors (master ticks per component tick)
    static constexpr uint32 NTSC_VDC_DIVISOR = 2;   // VDC pixel clock = master / 2
    static constexpr uint32 NTSC_CPU_DIVISOR = 20;   // CPU clock = master / 20
    static constexpr uint32 PAL_VDC_DIVISOR = 5;     // VDC pixel clock = master / 5
    static constexpr uint32 PAL_CPU_DIVISOR = 45;    // CPU clock = master / 45
    
    // NTSC scanline constants
    static constexpr uint16 NTSC_VBLANK_START_LINE = 242;
    static constexpr uint16 NTSC_VBLANK_END_LINE = 263;
    static constexpr uint16 NTSC_TOTAL_SCANLINES = 264;
    
    // PAL scanline constants
    static constexpr uint16 PAL_VBLANK_START_LINE = 290;
    static constexpr uint16 PAL_TOTAL_SCANLINES = 313;
    
private:
    VideoStandard standard_;
    
    // Master clock state
    uint64 master_tick_count_;
    uint32 scanline_tick_;       // 0 to TICKS_PER_SCANLINE-1
    uint16 current_scanline_;    // Hardware Y counter (0-263 NTSC)
    uint32 current_frame_;
    
    // Vblank flip-flop (hardware state, not derived)
    bool vblank_;
    bool vblank_rising_edge_;    // True for exactly one tick at vblank start
    
    // Frame completion flag (set when Y resets to 0)
    bool frame_complete_;
    
    // VDC cycle counter (for compatibility/debugging)
    uint64 vdc_cycle_count_;
    
    // CPU cycle debt tracking
    uint8 cpu_cycles_remaining_;
    
    // Countdown counters — replace modulo operations in tick()
    uint32 vdc_countdown_;
    uint32 cpu_countdown_;
    
    // Standard-specific constants (set in calculate_timing)
    uint32 ticks_per_scanline_;  // 455 NTSC, 1135 PAL
    uint32 y_increment_tick_;
    uint32 hblank_start_tick_;
    uint32 hblank_end_tick_;
    uint32 vblank_transition_tick_;
    uint32 vdc_tick_divisor_;    // 2 NTSC, 5 PAL
    uint32 cpu_tick_divisor_;    // 20 NTSC, 45 PAL
    uint16 total_scanlines_;     // 264 NTSC, 313 PAL
    uint16 vblank_start_line_;   // 242 NTSC
    uint16 vblank_end_line_;     // 263 NTSC
    
    void calculate_timing();
};

} // namespace videopac

#endif // VIDEOPAC_MASTER_CLOCK_H
