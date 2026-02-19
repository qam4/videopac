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
        VDC,             // VDC should advance one cycle
        FRAME_COMPLETE   // Frame rendering is complete
    };
    
    // Advance the master clock and determine what to execute next
    ExecuteNext tick();
    
    // Notify that CPU executed an instruction
    void cpu_executed(uint8 instruction_cycles);
    
    // Notify that VDC advanced one cycle
    void vdc_ticked();
    
    // Frame management
    bool is_frame_complete() const;
    void reset_frame();
    
    // Reset master cycle count (for emulator reset)
    void reset();
    
    // Accessors for testing
    uint64 get_master_cycle_count() const { return master_cycle_count_; }
    double get_cpu_cycle_debt() const { return cpu_cycle_debt_; }
    double get_vdc_cycle_debt() const { return vdc_cycle_debt_; }
    uint32 get_frame_cycle_count() const { return frame_cycle_count_; }
    uint32 get_cycles_per_frame() const { return cycles_per_frame_; }
    
private:
    VideoStandard standard_;
    uint64 master_cycle_count_;      // Total VDC cycles since start
    double cpu_cycle_debt_;           // Accumulated CPU cycles to execute
    double vdc_cycle_debt_;           // Accumulated VDC cycles to execute
    uint32 frame_cycle_count_;        // Cycles in current frame
    uint32 cycles_per_frame_;         // Total cycles per frame
    
    // Clock frequency constants
    static constexpr double VDC_CLOCK_MHZ = 3.54;
    static constexpr double CPU_CLOCK_MHZ = 1.79 / 5.0;  // ~0.358 MHz
    static constexpr double CYCLES_PER_CPU_INSTRUCTION = VDC_CLOCK_MHZ / CPU_CLOCK_MHZ;  // ~9.9
    
    // Calculate cycles per frame based on video standard
    void calculate_timing();
};

} // namespace videopac

#endif // VIDEOPAC_MASTER_CLOCK_H
