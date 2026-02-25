#ifndef VIDEOPAC_MASTER_CLOCK_H
#define VIDEOPAC_MASTER_CLOCK_H

#include "types.h"

namespace videopac {

// Master clock synchronization for cycle-accurate emulation
// Uses VDC 3.54 MHz clock as base unit, coordinates CPU and VDC execution
// Reference: .kiro/specs/character-rendering-fix/design.md
class MasterClock {
public:
    explicit MasterClock(VideoStandard standard);
    
    // Execution control
    enum class ExecuteNext {
        CPU,             // CPU should execute next instruction
        VDC              // VDC should advance one cycle
    };
    
    // Advance the master clock and determine what to execute next
    ExecuteNext tick();
    
    // Notify that CPU executed an instruction
    void cpu_executed(uint8 instruction_cycles);
    
    // Notify that VDC advanced one cycle
    void vdc_ticked();
    
    // Reset master cycle count (for emulator reset)
    void reset();
    
    // Accessors for testing
    uint64 get_master_cycle_count() const { return master_cycle_count_; }
    double get_cpu_cycle_debt() const { return static_cast<double>(cpu_cycle_debt_numerator_) / cpu_cycle_debt_denominator_; }
    double get_vdc_cycle_debt() const { return static_cast<double>(vdc_cycle_debt_numerator_) / vdc_cycle_debt_denominator_; }
    double get_cycles_per_cpu_instruction() const { return static_cast<double>(cycles_per_cpu_instruction_); }
    
private:
    VideoStandard standard_;
    uint64 master_cycle_count_;      // Total VDC cycles since start
    
    // Integer-based debt tracking (eliminates floating-point rounding errors)
    // CPU debt: tracks when CPU should execute (numerator/denominator >= 1.0 means execute)
    int64 cpu_cycle_debt_numerator_;     // Accumulated CPU cycle debt (numerator)
    int64 cpu_cycle_debt_denominator_;   // Debt threshold (denominator)
    
    // VDC debt: tracks VDC cycles that CPU execution "owes" to VDC
    int64 vdc_cycle_debt_numerator_;     // Accumulated VDC cycle debt (numerator)
    int64 vdc_cycle_debt_denominator_;   // Debt threshold (denominator)
    
    // Standard-specific timing (set by calculate_timing())
    uint32 cycles_per_cpu_instruction_;  // VDC cycles per CPU instruction (10 for NTSC, 9 for PAL)
    
    // Hardware reference constants (NOT used in emulation, for documentation only)
    // The actual hardware has a master crystal oscillator that gets divided down.
    // Our emulator uses VDC cycles as the base unit instead of simulating the crystal.
    
    // NTSC hardware clock frequencies (reference only)
    static constexpr double NTSC_HARDWARE_MASTER_CLOCK_MHZ = 7.15909;   // Hardware crystal oscillator
    static constexpr double NTSC_HARDWARE_VDC_CLOCK_MHZ = 3.579545;     // master / 2
    static constexpr double NTSC_HARDWARE_CPU_CLOCK_MHZ = 0.357954;     // (master × 0.75 / 3) / 5
    static constexpr uint32 NTSC_CYCLES_PER_CPU_INSTRUCTION = 10;       // USED: VDC cycles per CPU instruction
    
    // PAL hardware clock frequencies (reference only)
    static constexpr double PAL_HARDWARE_MASTER_CLOCK_MHZ = 17.734476;  // Hardware crystal oscillator
    static constexpr double PAL_HARDWARE_VDC_CLOCK_MHZ = 3.546895;      // master / 5
    static constexpr double PAL_HARDWARE_CPU_CLOCK_MHZ = 0.394099;      // (master × 0.8 / 4) / 9
    static constexpr uint32 PAL_CYCLES_PER_CPU_INSTRUCTION = 9;         // USED: VDC cycles per CPU instruction
    
    // Note: Our emulator's "master clock" (master_cycle_count_) counts VDC cycles, not hardware crystal cycles.
    // This is simpler and sufficient for cycle-accurate emulation.
    
    // Calculate cycles per frame based on video standard
    void calculate_timing();
};

} // namespace videopac

#endif // VIDEOPAC_MASTER_CLOCK_H
