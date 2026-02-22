#include "master_clock.h"

namespace videopac {

MasterClock::MasterClock(VideoStandard standard)
    : standard_(standard)
    , master_cycle_count_(0)
    , cpu_cycle_debt_(0.0)
    , vdc_cycle_debt_(0.0)
    , frame_cycle_count_(0)
    , cycles_per_frame_(0)
    , vdc_clock_mhz_(0.0)
    , cpu_instruction_mhz_(0.0)
    , cycles_per_cpu_instruction_(0.0)
{
    calculate_timing();
}

void MasterClock::calculate_timing() {
    // Calculate cycles per frame based on video standard
    // Use hardware-accurate clock frequencies for NTSC and PAL
    
    // Set standard-specific clock frequencies
    if (standard_ == VideoStandard::NTSC) {
        vdc_clock_mhz_ = NTSC_VDC_CLOCK_MHZ;
        cpu_instruction_mhz_ = NTSC_CPU_INSTRUCTION_MHZ;
        cycles_per_cpu_instruction_ = NTSC_CYCLES_PER_CPU_INSTRUCTION;
    } else {  // PAL
        vdc_clock_mhz_ = PAL_VDC_CLOCK_MHZ;
        cpu_instruction_mhz_ = PAL_CPU_INSTRUCTION_MHZ;
        cycles_per_cpu_instruction_ = PAL_CYCLES_PER_CPU_INSTRUCTION;
    }
    
    // Calculate cycles per scanline using fractional values for accuracy
    // NTSC: 3.579545 MHz / 59.94 Hz / 262 scanlines = 227.5 cycles/scanline
    // PAL: 3.546895 MHz / 50 Hz / 312 scanlines = 227.36 cycles/scanline
    uint32 scanlines_per_frame = (standard_ == VideoStandard::NTSC) ? 262 : 312;
    double cycles_per_scanline = (standard_ == VideoStandard::NTSC) ? 227.5 : 227.36;
    
    // Calculate total cycles per frame using fractional cycles per scanline
    // NTSC: 262 × 227.5 = 59,605 cycles
    // PAL: 312 × 227.36 ≈ 70,936 cycles
    double exact_cycles = scanlines_per_frame * cycles_per_scanline;
    cycles_per_frame_ = static_cast<uint32>(exact_cycles + 0.5);  // Round to nearest integer
}

MasterClock::ExecuteNext MasterClock::tick() {
    // Check if frame is complete
    if (frame_cycle_count_ >= cycles_per_frame_) {
        return ExecuteNext::FRAME_COMPLETE;
    }
    
    // Determine which component should execute next based on cycle debt
    // The component with higher debt should execute to catch up
    if (cpu_cycle_debt_ >= cycles_per_cpu_instruction_) {
        return ExecuteNext::CPU;
    }
    
    // VDC executes by default (it runs at the base clock frequency)
    return ExecuteNext::VDC;
}

void MasterClock::cpu_executed(uint8 instruction_cycles) {
    // CPU executed an instruction
    // Subtract the instruction cycles from CPU debt
    // Add equivalent VDC cycles to VDC debt (CPU runs slower than VDC)
    
    double vdc_cycles = instruction_cycles * cycles_per_cpu_instruction_;
    
    cpu_cycle_debt_ -= instruction_cycles;
    vdc_cycle_debt_ += vdc_cycles;
}

void MasterClock::vdc_ticked() {
    // VDC advanced one cycle
    master_cycle_count_++;
    frame_cycle_count_++;
    
    // Accumulate CPU debt (CPU needs to catch up)
    // For every VDC cycle, CPU accumulates 1/ratio of an instruction cycle
    cpu_cycle_debt_ += 1.0 / cycles_per_cpu_instruction_;
    
    // Reduce VDC debt
    vdc_cycle_debt_ -= 1.0;
}

bool MasterClock::is_frame_complete() const {
    return frame_cycle_count_ >= cycles_per_frame_;
}

void MasterClock::reset_frame() {
    frame_cycle_count_ = 0;
    
    // Debt carries over between frames to maintain long-term timing accuracy.
    // The CPU and VDC have independent clocks in hardware, and the fractional
    // cycle debt represents the real timing relationship. With exact integer
    // ratios (10.0 for NTSC, 9.0 for PAL), debt naturally stays bounded.
    // This approach prevents timing drift over extended periods.
}

void MasterClock::reset() {
    master_cycle_count_ = 0;
    frame_cycle_count_ = 0;
    // Initialize CPU with enough debt to execute immediately at cycle 0
    cpu_cycle_debt_ = cycles_per_cpu_instruction_;
    vdc_cycle_debt_ = 0.0;
}

} // namespace videopac
