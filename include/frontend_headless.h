#ifndef VIDEOPAC_FRONTEND_HEADLESS_H
#define VIDEOPAC_FRONTEND_HEADLESS_H

#include "frontend.h"
#include "emulator.h"
#include "debugger.h"
#include "debugger_ui.h"
#include <memory>
#include <chrono>

namespace videopac {

// Headless frontend for testing and remote operation
class HeadlessFrontend : public Frontend {
public:
    HeadlessFrontend();
    ~HeadlessFrontend() override;
    
    // Frontend interface
    bool initialize(const FrontendConfig& config) override;
    void shutdown() override;
    void run() override;
    bool is_running() const override { return running_; }
    
    void render_frame() override;
    void process_audio() override;
    void process_input() override;
    MenuAction process_menu() override;
    void show_message(const std::string& message) override;
    
    EmulatorCore* get_emulator() override { return emulator_.get(); }
    
    // Headless-specific
    void save_screenshot(const std::string& filename) override;
    void dump_framebuffer(const std::string& filename) override;
    
    // Control
    void set_frame_limit(int frames) { frame_limit_ = frames; }
    void set_auto_screenshot(bool enabled, int interval) {
        auto_screenshot_ = enabled;
        screenshot_interval_ = interval;
    }

private:
    FrontendConfig config_;
    std::unique_ptr<EmulatorCore> emulator_;
    std::unique_ptr<Debugger> debugger_;
    std::unique_ptr<DebuggerUI> debugger_ui_;
    
    bool running_;
    int frame_count_;
    int frame_limit_;  // 0 = unlimited
    
    // Screenshot support
    bool auto_screenshot_;
    int screenshot_interval_;
    int screenshot_count_;
    
    // Timing
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point last_frame_time_;
    
    // Helpers
    void write_ppm(const std::string& filename, const uint8* framebuffer, int width, int height);
    void update_timing();
};

} // namespace videopac

#endif // VIDEOPAC_FRONTEND_HEADLESS_H
