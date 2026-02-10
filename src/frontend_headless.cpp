#include "frontend_headless.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cstring>

namespace videopac {

HeadlessFrontend::HeadlessFrontend()
    : running_(false)
    , frame_count_(0)
    , frame_limit_(0)
    , auto_screenshot_(false)
    , screenshot_interval_(60)
    , screenshot_count_(0)
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
        debugger_->enable_trace(true);  // Enable trace logging
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
}

void HeadlessFrontend::process_audio() {
    // No audio output in headless mode
    // Could write to file if needed
}

void HeadlessFrontend::process_input() {
    // No input in headless mode
    // Could read from file or network if needed
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
    
    const uint8* framebuffer = emulator_->get_framebuffer();
    write_ppm(filename, framebuffer, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
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
    
    // Write PPM header
    file << "P6\n" << width << " " << height << "\n255\n";
    
    // Convert palette indices to RGB
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8 palette_index = framebuffer[y * width + x];
            Color color = PALETTE_BRIGHT[palette_index % 8];
            file.put(color.r);
            file.put(color.g);
            file.put(color.b);
        }
    }
    
    file.close();
}

void HeadlessFrontend::update_timing() {
    // Calculate frame time
    auto current_time = std::chrono::steady_clock::now();
    auto frame_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        current_time - last_frame_time_);
    
    // Target frame time (60 Hz for NTSC, 50 Hz for PAL)
    int target_fps = (config_.video_standard == VideoStandard::NTSC) ? 60 : 50;
    auto target_frame_time = std::chrono::microseconds(1000000 / target_fps);
    
    // Sleep if we're ahead of schedule (optional in headless mode)
    // Disabled by default to run as fast as possible
    // if (frame_duration < target_frame_time) {
    //     std::this_thread::sleep_for(target_frame_time - frame_duration);
    // }
    
    last_frame_time_ = current_time;
}

} // namespace videopac
