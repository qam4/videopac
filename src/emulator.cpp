#include "emulator.h"
#include "debugger.h"
#include "savestate.h"
#include <iostream>
#include <chrono>

namespace videopac {

EmulatorCore::EmulatorCore(const Configuration& config)
    : config_(config), vdc_(config.video_standard), master_clock_(config.video_standard),
      debugger_(nullptr), running_(false), paused_(false), frame_count_(0), 
      vblank_interrupt_triggered_(false) {
    
    // Connect components
    cpu_.set_memory_system(&memory_);
    cpu_.set_input_handler(&input_);
    memory_.set_vdc(&vdc_);
    memory_.set_cpu(&cpu_);  // Allow memory system to read Port 1 from CPU
    
    calculate_timing();
}

Result<void> EmulatorCore::load_bios(const std::string& path) {
    return memory_.load_bios(path);
}

Result<void> EmulatorCore::load_bios(const uint8* data, size_t size) {
    return memory_.load_bios(data, size);
}

Result<void> EmulatorCore::load_rom(const std::string& path) {
    auto result = memory_.load_cartridge(path);
    if (result.is_ok()) {
        reset();
        running_ = true;
    }
    return result;
}

Result<void> EmulatorCore::load_rom(const uint8* data, size_t size) {
    auto result = memory_.load_cartridge(data, size);
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
    master_clock_.reset_frame();
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
    
    // Reset VBlank interrupt flag at start of frame
    vblank_interrupt_triggered_ = false;
    
    // Reset master clock for new frame
    master_clock_.reset_frame();
    
    // Diagnostic counters (only if profiling enabled)
    static uint64 total_cpu_calls = 0;
    static uint64 total_vdc_calls = 0;
    static uint64 frame_counter = 0;
    uint64 cpu_calls_this_frame = 0;
    uint64 vdc_calls_this_frame = 0;
    
    // Timing measurements (in microseconds, only if profiling enabled)
    static uint64 total_cpu_time = 0;
    static uint64 total_vdc_time = 0;
    static uint64 total_overhead_time = 0;
    uint64 cpu_time_this_frame = 0;
    uint64 vdc_time_this_frame = 0;
    uint64 overhead_time_this_frame = 0;
    
    bool profiling = config_.enable_profile;
    
    // Cycle-accurate execution loop
    // CPU and VDC execute interleaved based on master clock cycle debt
    while (!master_clock_.is_frame_complete()) {
        auto loop_start = profiling ? std::chrono::high_resolution_clock::now() : std::chrono::high_resolution_clock::time_point();
        
        MasterClock::ExecuteNext next = master_clock_.tick();
        
        if (profiling) {
            auto tick_end = std::chrono::high_resolution_clock::now();
            overhead_time_this_frame += std::chrono::duration_cast<std::chrono::microseconds>(tick_end - loop_start).count();
        }
        
        switch (next) {
            case MasterClock::ExecuteNext::CPU: {
                auto cpu_start = profiling ? std::chrono::high_resolution_clock::now() : std::chrono::high_resolution_clock::time_point();
                
                if (profiling) cpu_calls_this_frame++;
                
                // Check for interrupts BEFORE executing next instruction
                handle_interrupts();
                
                // Check for breakpoint before executing instruction
                check_debugger_breakpoint();
                
                // If paused by debugger, stop execution
                if (paused_) {
                    return;
                }
                
                // Execute one CPU instruction
                uint8 instruction_cycles = cpu_.execute_instruction();
                
                // Notify master clock that CPU executed
                master_clock_.cpu_executed(instruction_cycles);
                
                // Log instruction trace AFTER executing
                if (debugger_ && debugger_->is_trace_enabled()) {
                    debugger_->log_instruction(master_clock_.get_master_cycle_count());
                }
                
                if (profiling) {
                    auto cpu_end = std::chrono::high_resolution_clock::now();
                    cpu_time_this_frame += std::chrono::duration_cast<std::chrono::microseconds>(cpu_end - cpu_start).count();
                }
                break;
            }
            
            case MasterClock::ExecuteNext::VDC: {
                auto vdc_start = profiling ? std::chrono::high_resolution_clock::now() : std::chrono::high_resolution_clock::time_point();
                
                if (profiling) vdc_calls_this_frame++;
                
                // Advance VDC by one cycle
                vdc_.tick_one_cycle();
                
                // Notify master clock that VDC ticked
                master_clock_.vdc_ticked();
                
                if (profiling) {
                    auto vdc_end = std::chrono::high_resolution_clock::now();
                    vdc_time_this_frame += std::chrono::duration_cast<std::chrono::microseconds>(vdc_end - vdc_start).count();
                }
                break;
            }
            
            case MasterClock::ExecuteNext::FRAME_COMPLETE: {
                // Frame is complete, exit loop
                break;
            }
        }
    }
    
    // Update diagnostics
    if (profiling) {
        total_cpu_calls += cpu_calls_this_frame;
        total_vdc_calls += vdc_calls_this_frame;
        total_cpu_time += cpu_time_this_frame;
        total_vdc_time += vdc_time_this_frame;
        total_overhead_time += overhead_time_this_frame;
        frame_counter++;
        
        // Print diagnostics every 60 frames
        if (frame_counter % 60 == 0) {
            uint64 total_time = cpu_time_this_frame + vdc_time_this_frame + overhead_time_this_frame;
            std::cout << "Frame " << frame_counter << " timing (microseconds):" << std::endl;
            std::cout << "  CPU time: " << cpu_time_this_frame << " us (" 
                      << (100.0 * cpu_time_this_frame / total_time) << "%)" << std::endl;
            std::cout << "  VDC time: " << vdc_time_this_frame << " us (" 
                      << (100.0 * vdc_time_this_frame / total_time) << "%)" << std::endl;
            std::cout << "  Overhead time: " << overhead_time_this_frame << " us (" 
                      << (100.0 * overhead_time_this_frame / total_time) << "%)" << std::endl;
            std::cout << "  Total frame time: " << total_time << " us" << std::endl;
            std::cout << "  Target frame time: 16667 us (60 FPS)" << std::endl;
            std::cout << "  CPU calls: " << cpu_calls_this_frame << std::endl;
            std::cout << "  VDC calls: " << vdc_calls_this_frame << std::endl;
        }
    }
    
    // Update debugger frame statistics
    if (debugger_) {
        debugger_->update_frame_stats(master_clock_.get_master_cycle_count());
    }
    
    frame_count_++;
}

void EmulatorCore::step() {
    if (!running_) {
        return;
    }
    
    // Check for breakpoint before executing instruction
    check_debugger_breakpoint();
    
    // Log instruction trace if enabled
    log_debugger_trace();
    
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
    // Gather state from all components
    SaveState state;
    state.cpu_state = cpu_.get_state();
    state.vdc_state = vdc_.get_state();
    state.memory_state = memory_.get_state();
    state.input_state = input_.get_state();
    state.frame_count = frame_count_;
    
    // Save to file
    return SaveStateManager::save(path, state);
}

Result<void> EmulatorCore::load_state(const std::string& path) {
    // Load from file
    auto result = SaveStateManager::load(path);
    if (!result.is_ok()) {
        return Result<void>::err(result.error);
    }
    
    SaveState state = result.value.value();
    
    // Restore state to all components
    cpu_.set_state(state.cpu_state);
    vdc_.set_state(state.vdc_state);
    memory_.set_state(state.memory_state);
    input_.set_state(state.input_state);
    frame_count_ = state.frame_count;
    
    return Result<void>::ok();
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
    // Check for VBLANK interrupt (triggered ONCE at start of VBLANK)
    // According to doc/o2doc.md and BIOS disassembly:
    // - External interrupt vector is at 0x003 in BIOS
    // - BIOS jumps to 0x402 in cartridge
    // - Cartridge should jump to 0x009 in BIOS (VBlank handler)
    
    if (vdc_.is_vblank() && !vblank_interrupt_triggered_) {
        // Trigger external interrupt (VBlank) to BIOS vector 0x003
        cpu_.trigger_interrupt(0x003);
        vblank_interrupt_triggered_ = true;
    }
    
    // TODO: Add timer interrupt handling (vector 0x007)
    // TODO: Add other external interrupts
    // TODO: Add horizontal line interrupt handling (if enabled in VDC control register)
}

void EmulatorCore::check_debugger_breakpoint() {
    if (!debugger_) {
        return;
    }
    
    // Get current PC
    CPUState cpu_state = cpu_.get_state();
    uint16 pc = cpu_state.pc;
    
    // Check if breakpoint is hit (address-based or condition-only)
    if (debugger_->check_breakpoint(pc) || debugger_->check_condition_breakpoints()) {
        paused_ = true;
        debugger_->pause();
        std::cout << "\n*** BREAKPOINT HIT at 0x" << std::hex << pc << std::dec << " ***" << std::endl;
        std::cout << "CPU State:" << std::endl;
        std::cout << debugger_->dump_cpu_state() << std::endl;
        std::cout << "Press F9 to step, F5 to continue" << std::endl;
    }
}

void EmulatorCore::log_debugger_trace() {
    if (!debugger_ || !debugger_->is_trace_enabled()) {
        return;
    }
    
    debugger_->log_instruction();
}

} // namespace videopac
