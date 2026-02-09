#include "emulator.h"

namespace videopac {

EmulatorCore::EmulatorCore(const Configuration& config)
    : config_(config), vdc_(config.video_standard), running_(false), paused_(false), frame_count_(0) {
    
    // Connect components
    cpu_.set_memory_system(&memory_);
    memory_.set_vdc(&vdc_);
    
    calculate_timing();
}

Result<void> EmulatorCore::load_bios(const std::string& path) {
    return memory_.load_bios(path);
}

Result<void> EmulatorCore::load_rom(const std::string& path) {
    auto result = memory_.load_cartridge(path);
    if (result.is_ok()) {
        reset();
        running_ = true;
    }
    return result;
}

void EmulatorCore::reset() {
    cpu_.reset();
    vdc_.reset();
    input_.reset();
    frame_count_ = 0;
    
    // According to doc/o2doc.md section 6.1:
    // The BIOS jumps to address 0x400 when the system is powered up or reset
    // The CPU reset() already sets PC to 0x000 (BIOS start)
    // The BIOS will then jump to 0x400 (cartridge entry point)
}

void EmulatorCore::run_frame() {
    if (!running_ || paused_) {
        return;
    }
    
    // Execute one complete frame
    // Frame consists of multiple scanlines, each with CPU execution and VDC rendering
    uint32 scanlines = (config_.video_standard == VideoStandard::NTSC) ? 
                       NTSC_SCANLINES : PAL_SCANLINES;
    
    for (uint32 scanline = 0; scanline < scanlines; ++scanline) {
        // Execute CPU for this scanline's worth of cycles
        uint32 cycles_executed = 0;
        while (cycles_executed < cycles_per_scanline_) {
            uint8 instruction_cycles = cpu_.execute_instruction();
            cycles_executed += instruction_cycles;
            
            // Advance VDC by the same number of cycles
            vdc_.tick(instruction_cycles);
        }
        
        // Render this scanline if not in VBLANK
        if (!vdc_.is_vblank()) {
            vdc_.render_scanline();
        }
        
        // Check for interrupts at specific points
        handle_interrupts();
    }
    
    frame_count_++;
}

void EmulatorCore::step() {
    if (!running_) {
        return;
    }
    
    cpu_.execute_instruction();
}

const uint8* EmulatorCore::get_framebuffer() const {
    return vdc_.get_framebuffer();
}

void EmulatorCore::get_audio_buffer(int16* buffer, size_t samples) {
    // TODO: Generate audio samples
    // This will be implemented in task 8
    for (size_t i = 0; i < samples; ++i) {
        buffer[i] = vdc_.get_audio_sample();
    }
}

void EmulatorCore::set_input(const InputState& input) {
    input_.set_state(input);
}

Result<void> EmulatorCore::save_state(const std::string& path) {
    (void)path;
    // TODO: Implement save state
    // This will be implemented in task 10
    return Result<void>::err("Not implemented");
}

Result<void> EmulatorCore::load_state(const std::string& path) {
    (void)path;
    // TODO: Implement load state
    // This will be implemented in task 10
    return Result<void>::err("Not implemented");
}

void EmulatorCore::calculate_timing() {
    if (config_.video_standard == VideoStandard::NTSC) {
        cycles_per_frame_ = (NTSC_CPU_CLOCK / CPU_CLOCK_DIVIDER) / NTSC_FRAME_RATE;
        cycles_per_scanline_ = cycles_per_frame_ / NTSC_SCANLINES;
    } else {
        cycles_per_frame_ = (PAL_CPU_CLOCK / CPU_CLOCK_DIVIDER) / PAL_FRAME_RATE;
        cycles_per_scanline_ = cycles_per_frame_ / PAL_SCANLINES;
    }
}

void EmulatorCore::handle_interrupts() {
    // Check for VBLANK interrupt (triggered at start of VBLANK)
    // According to doc/o2doc.md, cartridge vector 0x406 is the VBLANK service routine
    if (vdc_.is_vblank()) {
        // Trigger VBLANK interrupt to cartridge vector 0x406
        cpu_.trigger_interrupt(0x406);
    }
    
    // TODO: Add timer interrupt handling
    // TODO: Add external interrupt handling
    // TODO: Add horizontal line interrupt handling (if enabled in VDC control register)
}

} // namespace videopac
