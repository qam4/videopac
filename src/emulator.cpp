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
    
    // Jump to cartridge entry point
    // TODO: Set PC to 0x400 after BIOS initialization
}

void EmulatorCore::run_frame() {
    if (!running_ || paused_) {
        return;
    }
    
    // TODO: Implement frame execution loop
    // This will be implemented in task 8
    // For now, just increment frame counter
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
    // TODO: Implement interrupt handling
    // This will be implemented in task 8
}

} // namespace videopac
