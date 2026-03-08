#include "emulator.h"
#include "debugger.h"
#include "savestate.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <chrono>

namespace videopac {

EmulatorCore::EmulatorCore(const Configuration& config)
    : config_(config), vdc_(config.video_standard), master_clock_(config.video_standard),
      debugger_(nullptr), running_(false), paused_(false), frame_count_(0) {
    
    // Connect components
    cpu_.set_memory_system(&memory_);
    cpu_.set_input_handler(&input_);
    memory_.set_vdc(&vdc_);
    memory_.set_cpu(&cpu_);  // Allow memory system to read Port 1 from CPU
    vdc_.set_master_clock(&master_clock_);  // VDC queries master clock for beam position
}

Result<void> EmulatorCore::load_bios(const std::string& path) {
    auto result = memory_.load_bios(path);
    if (result.is_ok()) {
        // If ROM is already loaded, start emulation
        if (memory_.get_cart_rom() != nullptr) {
            running_ = true;
        }
    }
    return result;
}

Result<void> EmulatorCore::load_bios(const uint8* data, size_t size) {
    auto result = memory_.load_bios(data, size);
    if (result.is_ok()) {
        // If ROM is already loaded, start emulation
        if (memory_.get_cart_rom() != nullptr) {
            running_ = true;
        }
    }
    return result;
}

Result<void> EmulatorCore::load_rom(const std::string& path) {
    auto result = memory_.load_cartridge(path);
    if (result.is_ok()) {
        reset();
        // Only start emulation if BIOS is also loaded
        const uint8* bios = memory_.get_bios_rom();
        if (bios) {
            running_ = true;
        }
    }
    return result;
}

Result<void> EmulatorCore::load_rom(const uint8* data, size_t size) {
    auto result = memory_.load_cartridge(data, size);
    if (result.is_ok()) {
        reset();
        // Only start emulation if BIOS is also loaded
        const uint8* bios = memory_.get_bios_rom();
        if (bios) {
            running_ = true;
        }
    }
    return result;
}

void EmulatorCore::reset() {
    cpu_.reset();
    vdc_.reset();
    input_.reset();
    master_clock_.reset();
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
    
    // Reset VBlank flag in status register at start of frame
    // (it will be latched again when vblank_rising_edge fires)
    
    // Clear frame_complete flag at start of frame
    // The master clock will set it again when Y resets to 0
    vdc_.clear_frame_complete();
    master_clock_.clear_frame_complete();
    
    // Reset audio sample buffer for this frame
    vdc_.reset_audio_sample_buffer();
    
    // ── Fast path: no debugger, no profiling ──
    // Eliminates all debugger checks, trace logging, and chrono calls
    // from the inner loop (~119K iterations per NTSC frame).
    if (!debugger_ && !config_.enable_profile) {
        while (!vdc_.is_frame_complete()) {
            MasterClock::ExecuteNext next = master_clock_.tick();
            
            switch (next) {
                case MasterClock::ExecuteNext::BOTH: {
                    uint8 cycles = cpu_.execute_instruction();
                    master_clock_.cpu_executed(cycles);
                    vdc_.tick_one_cycle();
                    cpu_.update_counter(vdc_.get_t1_state());
                    master_clock_.vdc_executed();
                    break;
                }
                case MasterClock::ExecuteNext::CPU: {
                    uint8 cycles = cpu_.execute_instruction();
                    master_clock_.cpu_executed(cycles);
                    break;
                }
                case MasterClock::ExecuteNext::VDC: {
                    vdc_.tick_one_cycle();
                    cpu_.update_counter(vdc_.get_t1_state());
                    master_clock_.vdc_executed();
                    break;
                }
                case MasterClock::ExecuteNext::NONE:
                    break;
            }
            
            handle_interrupts();
        }
        
        frame_count_++;
        return;
    }
    
    // ── Slow path: debugger and/or profiling active ──
    
    // Track VDC cycles at start of frame for statistics
    uint64 frame_start_vdc_cycles = vdc_.get_total_cycles();
    
    // Diagnostic counters (only if profiling enabled)
    static uint64 frame_counter = 0;
    uint64 cpu_calls_this_frame = 0;
    uint64 vdc_calls_this_frame = 0;
    
    // Timing measurements (in microseconds, only if profiling enabled)
    uint64 cpu_time_this_frame = 0;
    uint64 vdc_time_this_frame = 0;
    uint64 overhead_time_this_frame = 0;
    
    bool profiling = config_.enable_profile;
    
    // Cycle-accurate execution loop
    // CPU and VDC execute interleaved based on master clock cycle debt
    while (!vdc_.is_frame_complete()) {
        auto loop_start = profiling ? std::chrono::high_resolution_clock::now() : std::chrono::high_resolution_clock::time_point();
        
        MasterClock::ExecuteNext next = master_clock_.tick();
        
        if (profiling) {
            auto tick_end = std::chrono::high_resolution_clock::now();
            overhead_time_this_frame += std::chrono::duration_cast<std::chrono::microseconds>(tick_end - loop_start).count();
        }
        
        switch (next) {
            case MasterClock::ExecuteNext::BOTH: {
                // Both CPU and VDC execute at this tick
                auto cpu_start = profiling ? std::chrono::high_resolution_clock::now() : std::chrono::high_resolution_clock::time_point();
                
                if (profiling) cpu_calls_this_frame++;
                
                // Check for breakpoint before executing instruction
                check_debugger_breakpoint();
                
                // If paused by debugger, stop execution
                if (paused_) {
                    return;
                }
                
                // Log instruction trace BEFORE executing (so we capture the PC before it changes)
                if (debugger_ && debugger_->is_trace_enabled()) {
                    debugger_->log_instruction(master_clock_.get_vdc_cycle_count());
                }
                
                // Execute one CPU instruction
                uint8 cycles = cpu_.execute_instruction();
                
                // Log VDC trace if enabled (after instruction execution, in case it wrote to VDC)
                if (debugger_ && debugger_->is_vdc_trace_enabled()) {
                    debugger_->log_vdc_write();
                }
                
                // Notify master clock that CPU executed (cycles include interrupt overhead from execute_instruction)
                master_clock_.cpu_executed(cycles);
                
                if (profiling) {
                    auto cpu_end = std::chrono::high_resolution_clock::now();
                    cpu_time_this_frame += std::chrono::duration_cast<std::chrono::microseconds>(cpu_end - cpu_start).count();
                }
                
                // Now execute VDC
                auto vdc_start = profiling ? std::chrono::high_resolution_clock::now() : std::chrono::high_resolution_clock::time_point();
                
                if (profiling) vdc_calls_this_frame++;
                
                // Advance VDC by one cycle
                vdc_.tick_one_cycle();
                
                // Update CPU T1 pin with current VDC blanking state
                cpu_.update_counter(vdc_.get_t1_state());
                
                // Notify master clock that VDC ticked
                master_clock_.vdc_executed();
                
                if (profiling) {
                    auto vdc_end = std::chrono::high_resolution_clock::now();
                    vdc_time_this_frame += std::chrono::duration_cast<std::chrono::microseconds>(vdc_end - vdc_start).count();
                }
                break;
            }
            
            case MasterClock::ExecuteNext::CPU: {
                auto cpu_start = profiling ? std::chrono::high_resolution_clock::now() : std::chrono::high_resolution_clock::time_point();
                
                if (profiling) cpu_calls_this_frame++;
                
                // Check for breakpoint before executing instruction
                check_debugger_breakpoint();
                
                // If paused by debugger, stop execution
                if (paused_) {
                    return;
                }
                
                // Log instruction trace BEFORE executing (so we capture the PC before it changes)
                if (debugger_ && debugger_->is_trace_enabled()) {
                    debugger_->log_instruction(master_clock_.get_vdc_cycle_count());
                }
                
                // Execute one CPU instruction
                uint8 cycles = cpu_.execute_instruction();
                
                // Log VDC trace if enabled (after instruction execution, in case it wrote to VDC)
                if (debugger_ && debugger_->is_vdc_trace_enabled()) {
                    debugger_->log_vdc_write();
                }
                
                // Notify master clock that CPU executed (cycles include interrupt overhead from execute_instruction)
                master_clock_.cpu_executed(cycles);
                
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
                
                // Update CPU T1 pin with current VDC blanking state
                cpu_.update_counter(vdc_.get_t1_state());
                
                // Notify master clock that VDC ticked
                master_clock_.vdc_executed();
                
                if (profiling) {
                    auto vdc_end = std::chrono::high_resolution_clock::now();
                    vdc_time_this_frame += std::chrono::duration_cast<std::chrono::microseconds>(vdc_end - vdc_start).count();
                }
                break;
            }
            
            case MasterClock::ExecuteNext::NONE:
                break;
        }
        
        // Check for interrupts on EVERY tick, not just VDC ticks.
        // The vblank_rising_edge is a master clock event that fires at tick 365
        // of scanline 242. Since 455 is odd, the parity of master_tick_count
        // at that point alternates each frame — so if we only check on VDC ticks
        // (even master_tick_count for NTSC), we'd miss vblank on ~half the frames.
        handle_interrupts();
    }
    
    // Update diagnostics
    if (profiling) {
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
        uint64 frame_end_vdc_cycles = vdc_.get_total_cycles();
        uint64 cycles_this_frame = frame_end_vdc_cycles - frame_start_vdc_cycles;
        debugger_->update_frame_stats(cycles_this_frame);
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
    // Read pre-captured audio samples from VDC's ring buffer.
    // Samples were captured at the correct rate during run_frame() via
    // tick_one_cycle() -> capture_audio_sample().
    uint16 available = vdc_.get_audio_sample_count();
    const int16* src = vdc_.get_audio_sample_buffer();
    
    if (available == 0) {
        // No samples captured (e.g. first frame) - fill with silence
        std::memset(buffer, 0, samples * sizeof(int16));
        return;
    }
    
    // Copy available samples, stretching or truncating to fit requested count
    for (size_t i = 0; i < samples; ++i) {
        // Map output sample index to source buffer index
        size_t src_idx = (i * available) / samples;
        if (src_idx >= available) src_idx = available - 1;
        buffer[i] = src[src_idx];
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

void EmulatorCore::handle_interrupts() {
    // VBLANK interrupt: edge-triggered at the exact tick when vblank goes high.
    // Hardware: /INT goes low and A1.3 goes high simultaneously at tick 365 of scanline F2h.
    // Reference: doc/hardware/odyssey2_timing.txt Test 52
    //
    // The master clock tracks the vblank flip-flop and sets vblank_rising_edge()
    // for exactly one tick when the transition occurs. No manual edge detection needed.
    if (master_clock_.vblank_rising_edge()) {
        cpu_.set_external_interrupt_pending(true);
        vdc_.set_vblank_flag();  // Latch A1.3 high (cleared when CPU reads status register)
    }
    
    // Note: Timer interrupt (vector 0x007) is handled in CPU::execute_instruction()
    // when the timer overflows. BIOS at 0x007 jumps to ROM at 0x404.
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
        std::cout << "\n*** BREAKPOINT HIT at 0x" << std::hex << pc << std::dec 
                  << " (Frame " << frame_count_ << ") ***" << std::endl;
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
