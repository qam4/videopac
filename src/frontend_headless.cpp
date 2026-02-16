#include "frontend_headless.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <thread>

namespace videopac {

HeadlessFrontend::HeadlessFrontend()
    : running_(false)
    , frame_count_(0)
    , frame_limit_(0)
    , auto_screenshot_(false)
    , screenshot_interval_(60)
    , screenshot_count_(0)
    , enable_frame_pacing_(false)
{}

HeadlessFrontend::~HeadlessFrontend() {
    shutdown();
}

bool HeadlessFrontend::initialize(const FrontendConfig& config) {
    config_ = config;
    
    std::cout << "Initializing headless frontend..." << std::endl;
    std::cout << "  Video: " << (config.video_standard == VideoStandard::NTSC ? "NTSC" : "PAL") << std::endl;
    std::cout << "  Audio: " << (config.audio_enabled ? "Enabled" : "Disabled") << std::endl;
    
    // Create emulator
    Configuration emu_config;
    emu_config.video_standard = config.video_standard;
    emu_config.bios_path = config.bios_path;
    emu_config.enable_profile = config.enable_profile;
    
    emulator_ = std::make_unique<EmulatorCore>(emu_config);
    
    // Load BIOS if specified
    if (!config.bios_path.empty()) {
        auto result = emulator_->load_bios(config.bios_path);
        if (!result.is_ok()) {
            std::cerr << "Failed to load BIOS: " << result.error << std::endl;
            return false;
        }
        std::cout << "  BIOS loaded: " << config.bios_path << std::endl;
    }
    
    // Load ROM if specified
    if (!config.rom_path.empty()) {
        auto result = emulator_->load_rom(config.rom_path);
        if (!result.is_ok()) {
            std::cerr << "Failed to load ROM: " << result.error << std::endl;
            return false;
        }
        std::cout << "  ROM loaded: " << config.rom_path << std::endl;
    }
    
    // Initialize debugger if enabled
    if (config.enable_debugger) {
        debugger_ = std::make_unique<Debugger>(emulator_.get());
        debugger_ui_ = std::make_unique<DebuggerUI>(debugger_.get());
        emulator_->set_debugger(debugger_.get());
        // Enable trace if requested
        if (!config_.trace_level.empty()) {
            TraceLevel level = TraceLevel::Full;  // Default
            if (config_.trace_level == "minimal") {
                level = TraceLevel::Minimal;
                std::cout << "Instruction trace enabled (minimal - fast)" << std::endl;
            } else if (config_.trace_level == "normal") {
                level = TraceLevel::Normal;
                std::cout << "Instruction trace enabled (normal - medium)" << std::endl;
            } else if (config_.trace_level == "full") {
                level = TraceLevel::Full;
                std::cout << "Instruction trace enabled (full - slow)" << std::endl;
            } else {
                std::cerr << "Unknown trace level: " << config_.trace_level << ", using 'full'" << std::endl;
            }
            debugger_->set_trace_level(level);
        }
        
        // Set breakpoints from config
        for (const auto& bp : config.breakpoints) {
            if (bp.condition.empty()) {
                debugger_->add_breakpoint(bp.address);
                std::cout << "  Breakpoint set at 0x" << std::hex << bp.address << std::dec << std::endl;
            } else {
                debugger_->add_breakpoint(bp.address, bp.condition);
                std::cout << "  Conditional breakpoint set at 0x" << std::hex << bp.address << std::dec 
                          << " with condition: " << bp.condition << std::endl;
            }
        }
        
        // Set condition-only breakpoints (watch conditions)
        for (const auto& condition : config.watch_conditions) {
            debugger_->add_breakpoint(condition);
            std::cout << "  Watch condition set: " << condition << std::endl;
        }
        
        std::cout << "  Debugger enabled (with trace)" << std::endl;
    }
    
    running_ = true;
    start_time_ = std::chrono::steady_clock::now();
    last_frame_time_ = start_time_;
    
    std::cout << "Headless frontend initialized successfully" << std::endl;
    return true;
}

void HeadlessFrontend::shutdown() {
    std::cout << "\nShutting down..." << std::endl;
    std::cout.flush();
    
    // Print statistics
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);
    double seconds = duration.count() / 1000.0;
    double fps = frame_count_ / seconds;
    
    std::cout << "\nEmulation statistics:" << std::endl;
    std::cout << "  Frames: " << frame_count_ << std::endl;
    std::cout << "  Time: " << std::fixed << std::setprecision(2) << seconds << " seconds" << std::endl;
    std::cout << "  Average FPS: " << std::fixed << std::setprecision(2) << fps << std::endl;
    std::cout.flush();
    
    if (debugger_) {
        std::cout << "  Debugger is active" << std::endl;
        std::cout.flush();
        FrameStats stats = debugger_->get_frame_stats();
        std::cout << "  Total Cycles: " << stats.total_cycles << std::endl;
        std::cout << "  Avg Cycles/Frame: " << std::fixed << std::setprecision(2) 
                  << stats.average_cycles_per_frame << std::endl;
        std::cout.flush();
        
        // Dump trace log to file
        const auto& trace_log = debugger_->get_trace_log();
        std::cout << "  Trace log size: " << trace_log.size() << " instructions" << std::endl;
        std::cout.flush();
        if (!trace_log.empty()) {
            std::ofstream trace_file("trace.log");
            std::cout << "  Writing trace log..." << std::endl;
            std::cout.flush();
            for (const auto& line : trace_log) {
                trace_file << line << "\n";
            }
            trace_file.close();
            std::cout << "  Trace log written to trace.log" << std::endl;
            std::cout.flush();
        }
    } else {
        std::cout << "  No debugger" << std::endl;
        std::cout.flush();
    }
    
    running_ = false;
    std::cout << "Headless frontend shutdown" << std::endl;
    std::cout.flush();
}

void HeadlessFrontend::run() {
    std::cout << "Starting emulation..." << std::endl;
    if (frame_limit_ > 0) {
        std::cout << "  Frame limit: " << frame_limit_ << " frames" << std::endl;
    }
    if (auto_screenshot_) {
        std::cout << "  Auto-screenshot every " << screenshot_interval_ << " frames" << std::endl;
    }
    std::cout << "  Press Ctrl+C to stop" << std::endl;
    
    while (running_) {
        // Check frame limit
        if (frame_limit_ > 0 && frame_count_ >= frame_limit_) {
            std::cout << "\nReached frame limit (" << frame_limit_ << " frames)" << std::endl;
            running_ = false;
            break;
        }
        
        // Process input (minimal in headless mode)
        process_input();
        
        // Run one frame
        render_frame();
        
        // Check if emulator hit a breakpoint
        if (emulator_ && emulator_->is_paused()) {
            std::cout << "\n*** Breakpoint hit - executing 50 steps ***" << std::endl;
            
            // Execute 50 single steps and print state after each
            for (int step = 1; step <= 50; step++) {
                if (debugger_) {
                    debugger_->step();
                    
                    // Print every 5th step, or when we return to 0x687 (after calcchar23)
                    CPUState cpu = emulator_->get_cpu_state();
                    if (step % 5 == 0 || cpu.pc == 0x687) {
                        std::cout << "\n=== Step " << step << " ===" << std::endl;
                        std::cout << debugger_->dump_cpu_state() << std::endl;
                        std::cout << debugger_->disassemble_at_pc(0, 2) << std::endl;
                        
                        // Stop if we've returned from calcchar23
                        if (cpu.pc == 0x687) {
                            std::cout << "\n*** Returned from calcchar23 - R5 now contains byte 2 (char_ptr) ***" << std::endl;
                            break;
                        }
                    }
                }
            }
            
            running_ = false;
            break;
        }
        
        // Process audio (no-op in headless mode)
        process_audio();
        
        // Auto-screenshot if enabled
        if (auto_screenshot_ && (frame_count_ % screenshot_interval_ == 0)) {
            std::stringstream ss;
            ss << config_.screenshot_path << "/frame_" 
               << std::setw(6) << std::setfill('0') << frame_count_ << ".ppm";
            save_screenshot(ss.str());
        }
        
        // Update timing
        update_timing();
        
        // Progress indicator every 60 frames
        if (frame_count_ % 60 == 0) {
            std::cout << "." << std::flush;
        }
    }
    
    std::cout << std::endl;
}

void HeadlessFrontend::render_frame() {
    if (!emulator_) {
        return;
    }
    
    emulator_->run_frame();
    frame_count_++;
    
    // Debug: Dump VDC registers at frame 8 (after game starts)
    if (frame_count_ == 8) {
        std::cout << "\n";
        emulator_->get_vdc().dump_registers();
    }
}

void HeadlessFrontend::process_audio() {
    // No audio output in headless mode
    // Could write to file if needed
}

void HeadlessFrontend::process_input() {
    if (!emulator_) {
        return;
    }
    
    // Process simulated key presses
    InputHandler& input = emulator_->get_input_handler();
    
    // Check for scheduled key presses that should trigger this frame
    // Note: frame_count_ is the NEXT frame that will be rendered
    int next_frame = frame_count_ + 1;
    for (auto it = scheduled_keys_.begin(); it != scheduled_keys_.end(); ) {
        if (next_frame >= it->trigger_frame) {
            // Trigger the key press
            input.set_key_state(it->key, true);
            active_keys_.push_back({it->key, it->duration});
            std::cout << "\n[Frame " << next_frame << "] Pressing key " 
                      << static_cast<int>(it->key) << " for " << it->duration << " frames" << std::endl;
            it = scheduled_keys_.erase(it);
        } else {
            ++it;
        }
    }
    
    // Update active keys and release expired ones
    for (auto it = active_keys_.begin(); it != active_keys_.end(); ) {
        it->frames_remaining--;
        
        if (it->frames_remaining <= 0) {
            // Release the key
            input.set_key_state(it->key, false);
            std::cout << "[Frame " << next_frame << "] Releasing key " 
                      << static_cast<int>(it->key) << std::endl;
            it = active_keys_.erase(it);
        } else {
            ++it;
        }
    }
}

void HeadlessFrontend::press_key(VidKey key, int duration_frames) {
    if (!emulator_) {
        return;
    }
    
    InputHandler& input = emulator_->get_input_handler();
    
    // Press the key immediately
    input.set_key_state(key, true);
    
    // Schedule release after duration_frames
    if (duration_frames > 0) {
        active_keys_.push_back({key, duration_frames});
    }
}

void HeadlessFrontend::release_key(VidKey key) {
    if (!emulator_) {
        return;
    }
    
    InputHandler& input = emulator_->get_input_handler();
    
    // Release immediately
    input.set_key_state(key, false);
    
    // Remove from active keys if present
    active_keys_.erase(
        std::remove_if(active_keys_.begin(), active_keys_.end(),
            [key](const KeyPress& kp) { return kp.key == key; }),
        active_keys_.end()
    );
}

MenuAction HeadlessFrontend::process_menu() {
    return MenuAction::None;
}

void HeadlessFrontend::show_message(const std::string& message) {
    std::cout << "Message: " << message << std::endl;
}

void HeadlessFrontend::save_screenshot(const std::string& filename) {
    if (!emulator_) {
        return;
    }
    
    VDC& vdc = emulator_->get_vdc();
    
    // If extended framebuffer mode is enabled, save extended screenshot
    if (vdc.is_extended_framebuffer_mode()) {
        const uint8* framebuffer = vdc.get_extended_framebuffer();
        int width = vdc.get_extended_framebuffer_width();
        int height = vdc.get_extended_framebuffer_height();
        write_ppm(filename, framebuffer, width, height);
    } else {
        // Normal screenshot
        const uint8* framebuffer = emulator_->get_framebuffer();
        write_ppm(filename, framebuffer, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    }
    
    screenshot_count_++;
}

void HeadlessFrontend::dump_framebuffer(const std::string& filename) {
    save_screenshot(filename);
}

void HeadlessFrontend::write_ppm(const std::string& filename, const uint8* framebuffer, 
                                  int width, int height) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }
    
    // Check if this is extended framebuffer (240x250) or normal (160x200)
    bool is_extended = (width == EXTENDED_FB_WIDTH && height == EXTENDED_FB_HEIGHT);
    
    if (is_extended) {
        // Extended framebuffer: output as-is with 2x horizontal scaling
        // This shows the full VDC area including overscan
        int output_width = width * 2;   // 240 * 2 = 480
        int output_height = height;     // 250
        
        file << "P6\n" << output_width << " " << output_height << "\n255\n";
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                uint8 palette_index = framebuffer[y * width + x];
                Color color = PALETTE_BRIGHT[palette_index % 8];
                // Write each pixel twice (2x horizontal scaling)
                file.put(color.r);
                file.put(color.g);
                file.put(color.b);
                file.put(color.r);
                file.put(color.g);
                file.put(color.b);
            }
        }
    } else {
        // Normal framebuffer (160x240): O2EM-style 320x240 output
        // - 2x horizontal scaling for square-looking pixels
        // - No vertical borders needed since framebuffer is already 240 lines
        int output_width = 320;   // 160 * 2
        int output_height = 240;  // Same as framebuffer height
        
        file << "P6\n" << output_width << " " << output_height << "\n255\n";
        
        // Framebuffer area (240 lines, each pixel 2x wide)
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                uint8 palette_index = framebuffer[y * width + x];
                Color color = PALETTE_BRIGHT[palette_index % 8];
                // Write each pixel twice (2x horizontal scaling)
                file.put(color.r);
                file.put(color.g);
                file.put(color.b);
                file.put(color.r);
                file.put(color.g);
                file.put(color.b);
            }
        }
    }
    
    file.close();
}

void HeadlessFrontend::update_timing() {
    // Target frame time (60 Hz for NTSC, 50 Hz for PAL)
    int target_fps = (config_.video_standard == VideoStandard::NTSC) ? 60 : 50;
    auto target_frame_time = std::chrono::microseconds(1000000 / target_fps);
    
    // Use frame pacing when enabled (set when scheduled keys are present)
    if (enable_frame_pacing_) {
        auto current_time = std::chrono::steady_clock::now();
        auto frame_duration = std::chrono::duration_cast<std::chrono::microseconds>(
            current_time - last_frame_time_);
        
        if (frame_duration < target_frame_time) {
            std::this_thread::sleep_for(target_frame_time - frame_duration);
        }
    }
    
    last_frame_time_ = std::chrono::steady_clock::now();
}

void HeadlessFrontend::set_extended_framebuffer_mode(bool enabled) {
    if (!emulator_) {
        return;
    }
    
    // Access VDC through emulator and enable extended framebuffer mode
    VDC& vdc = emulator_->get_vdc();
    vdc.set_extended_framebuffer_mode(enabled);
}

void HeadlessFrontend::save_extended_screenshot(const std::string& filename) {
    if (!emulator_) {
        return;
    }
    
    VDC& vdc = emulator_->get_vdc();
    if (!vdc.is_extended_framebuffer_mode()) {
        std::cerr << "Extended framebuffer mode is not enabled" << std::endl;
        return;
    }
    
    const uint8* framebuffer = vdc.get_extended_framebuffer();
    int width = vdc.get_extended_framebuffer_width();
    int height = vdc.get_extended_framebuffer_height();
    
    write_ppm(filename, framebuffer, width, height);
    std::cout << "Saved extended screenshot (" << width << "x" << height << "): " << filename << std::endl;
}

void HeadlessFrontend::schedule_key_press(VidKey key, int trigger_frame, int duration_frames) {
    scheduled_keys_.push_back({key, trigger_frame, duration_frames});
    enable_frame_pacing_ = true;  // Enable frame pacing for accurate key timing
    std::cout << "Scheduled key " << static_cast<int>(key) 
              << " to be pressed at frame " << trigger_frame 
              << " for " << duration_frames << " frames" << std::endl;
}

} // namespace videopac
