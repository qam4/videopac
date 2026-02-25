#include "master_clock.h"

namespace videopac {

MasterClock::MasterClock(VideoStandard standard)
    : standard_(standard)
    , master_cycle_count_(0)
    , cpu_cycle_debt_numerator_(0)
    , cpu_cycle_debt_denominator_(1)
    , vdc_cycle_debt_numerator_(0)
    , vdc_cycle_debt_denominator_(1)
    , cycles_per_cpu_instruction_(0)
{
    calculate_timing();
}

void MasterClock::calculate_timing() {
    // Set standard-specific cycle ratios (exact integer ratios)
    if (standard_ == VideoStandard::NTSC) {
        cycles_per_cpu_instruction_ = NTSC_CYCLES_PER_CPU_INSTRUCTION;  // 10
    } else {  // PAL
        cycles_per_cpu_instruction_ = PAL_CYCLES_PER_CPU_INSTRUCTION;   // 9
    }
    
    // Set debt denominators to the cycle ratio
    // This allows us to track fractional progress using integer arithmetic
    cpu_cycle_debt_denominator_ = cycles_per_cpu_instruction_;
    vdc_cycle_debt_denominator_ = cycles_per_cpu_instruction_;
}

MasterClock::ExecuteNext MasterClock::tick() {
    // Determine which component should execute next based on cycle debt
    // CPU should execute when debt ratio >= 1.0 (numerator >= denominator)
    if (cpu_cycle_debt_numerator_ >= cpu_cycle_debt_denominator_) {
        return ExecuteNext::CPU;
    }
    
    // VDC executes by default (it runs at the base clock frequency)
    return ExecuteNext::VDC;
}

void MasterClock::cpu_executed(uint8 instruction_cycles) {
    // CPU executed an instruction
    // Subtract the instruction cycles from CPU debt
    // Add equivalent VDC cycles to VDC debt (CPU runs slower than VDC)
    
    // Each CPU instruction cycle consumes cycles_per_cpu_instruction_ VDC cycles
    int64 vdc_cycles = instruction_cycles * static_cast<int64>(cycles_per_cpu_instruction_);
    
    cpu_cycle_debt_numerator_ -= instruction_cycles * cpu_cycle_debt_denominator_;
    vdc_cycle_debt_numerator_ += vdc_cycles * vdc_cycle_debt_denominator_;
}

void MasterClock::vdc_ticked() {
    // VDC advanced one cycle
    master_cycle_count_++;
    
    // Accumulate CPU debt (CPU needs to catch up)
    // For every VDC cycle, CPU accumulates 1/cycles_per_cpu_instruction_ of an instruction
    // Using integer arithmetic: numerator += 1, execute when numerator >= denominator
    cpu_cycle_debt_numerator_ += 1;
    
    // Reduce VDC debt
    vdc_cycle_debt_numerator_ -= vdc_cycle_debt_denominator_;
}

void MasterClock::reset() {
    master_cycle_count_ = 0;
    // Initialize CPU with enough debt to execute immediately at cycle 0
    cpu_cycle_debt_numerator_ = cpu_cycle_debt_denominator_;
    vdc_cycle_debt_numerator_ = 0;
}

} // namespace videopac
