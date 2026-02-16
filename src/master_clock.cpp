#include "master_clock.h"

namespace videopac {

MasterClock::MasterClock(VideoStandard standard)
    : standard_(standard)
    , master_cycle_count_(0)
    , cpu_cycle_debt_(0.0)
    , vdc_cycle_debt_(0.0)
    , frame_cycle_count_(0)
    , cycles_per_frame_(0)
{
    calculate_timing();
}

void MasterClock::calculate_timing() {
    // Calculate cycles per frame based on video standard
    // VDC clock: 3.54 MHz
    // NTSC: 60 Hz, 262 scanlines, ~227 cycles per scanline
    // PAL: 50 Hz, 312 scanlines, ~227 cycles per scanline
    
    uint32 scanlines_per_frame = (standard_ == VideoStandard::NTSC) ? 262 : 312;
    uint32 cycles_per_scanline = 227;  // Approximate for both standards
    
    cycles_per_frame_ = scanlines_per_frame * cycles_per_scanline;
}

MasterClock::ExecuteNext MasterClock::tick() {
    // Check if frame is complete
    if (frame_cycle_count_ >= cycles_per_frame_) {
        return ExecuteNext::FRAME_COMPLETE;
    }
    
    // Determine which component should execute next based on cycle debt
    // The component with higher debt should execute to catch up
    if (cpu_cycle_debt_ >= CYCLES_PER_CPU_INSTRUCTION) {
        return ExecuteNext::CPU;
    }
    
    // VDC executes by default (it runs at the base clock frequency)
    return ExecuteNext::VDC;
}

void MasterClock::cpu_executed(uint8 instruction_cycles) {
    // CPU executed an instruction
    // Subtract the instruction cycles from CPU debt
    // Add equivalent VDC cycles to VDC debt (CPU runs slower than VDC)
    
    double vdc_cycles = instruction_cycles * CYCLES_PER_CPU_INSTRUCTION;
    
    cpu_cycle_debt_ -= instruction_cycles;
    vdc_cycle_debt_ += vdc_cycles;
}

void MasterClock::vdc_ticked() {
    // VDC advanced one cycle
    master_cycle_count_++;
    frame_cycle_count_++;
    
    // Accumulate CPU debt (CPU needs to catch up)
    // For every VDC cycle, CPU accumulates 1/9.9 of an instruction cycle
    cpu_cycle_debt_ += 1.0 / CYCLES_PER_CPU_INSTRUCTION;
    
    // Reduce VDC debt
    vdc_cycle_debt_ -= 1.0;
}

bool MasterClock::is_frame_complete() const {
    return frame_cycle_count_ >= cycles_per_frame_;
}

void MasterClock::reset_frame() {
    frame_cycle_count_ = 0;
    
    // Reset debt accumulators to prevent overflow
    // Keep small residual debt to maintain timing accuracy
    if (cpu_cycle_debt_ < -CYCLES_PER_CPU_INSTRUCTION) {
        cpu_cycle_debt_ = 0.0;
    }
    if (vdc_cycle_debt_ < -10.0) {
        vdc_cycle_debt_ = 0.0;
    }
}

void MasterClock::reset() {
    master_cycle_count_ = 0;
    frame_cycle_count_ = 0;
    // Initialize CPU with enough debt to execute immediately at cycle 0
    cpu_cycle_debt_ = CYCLES_PER_CPU_INSTRUCTION;
    vdc_cycle_debt_ = 0.0;
}

} // namespace videopac
