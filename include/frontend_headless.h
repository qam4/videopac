#ifndef VIDEOPAC_FRONTEND_HEADLESS_H
#define VIDEOPAC_FRONTEND_HEADLESS_H

#include "frontend.h"
#include "emulator.h"
#include "debugger.h"
#include "debugger_ui.h"
#include "input.h"
#include <memory>
#include <chrono>
#include <vector>

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
    
    // Extended framebuffer support
    void save_extended_screenshot(const std::string& filename);
    void set_extended_framebuffer_mode(bool enabled);
    
    // Control
    void set_frame_limit(int frames) { frame_limit_ = frames; }
    void set_auto_screenshot(bool enabled, int interval) {
        auto_screenshot_ = enabled;
        screenshot_interval_ = interval;
    }
    
    // Input simulation
    void press_key(VidKey key, int duration_frames = 5);
    void release_key(VidKey key);
    void schedule_key_press(VidKey key, int trigger_frame, int duration_frames = 5);
    int get_frame_count() const { return frame_count_; }

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
    bool enable_frame_pacing_;  // Enable frame pacing for accurate timing
    
    // Input simulation
    struct KeyPress {
        VidKey key;
        int frames_remaining;
    };
    std::vector<KeyPress> active_keys_;
    
    struct ScheduledKeyPress {
        VidKey key;
        int trigger_frame;
        int duration;
    };
    std::vector<ScheduledKeyPress> scheduled_keys_;
    
    // Helpers
    void write_ppm(const std::string& filename, const uint8* framebuffer, int width, int height);
    void update_timing();
};

} // namespace videopac

#endif // VIDEOPAC_FRONTEND_HEADLESS_H
