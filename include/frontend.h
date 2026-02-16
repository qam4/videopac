#ifndef VIDEOPAC_FRONTEND_H
#define VIDEOPAC_FRONTEND_H

#include "types.h"
#include "emulator.h"
#include <string>
#include <map>

namespace videopac {

// Frontend configuration
struct BreakpointConfig {
    uint16 address;
    std::string condition;  // Empty string means unconditional
    
    BreakpointConfig(uint16 addr) : address(addr), condition("") {}
    BreakpointConfig(uint16 addr, const std::string& cond) : address(addr), condition(cond) {}
};

struct FrontendConfig {
    // Video settings
    VideoStandard video_standard;
    int display_scale;              // 1x, 2x, 3x, 4x
    bool fullscreen;
    bool vsync;
    bool headless;                  // Run without window (for remote/testing)
    
    // Audio settings
    int sample_rate;                // 44100 or 48000
    int audio_buffer_size;          // Samples per buffer
    float master_volume;            // 0.0 to 1.0
    bool audio_enabled;
    
    // Input settings
    std::map<int, VidKey> key_mappings;  // Host key code -> Videopac key
    
    // Debug settings
    bool show_fps;
    bool enable_debugger;
    std::string trace_level;        // "", "minimal", "normal", "full"
    bool enable_profile;            // Enable performance profiling
    std::vector<BreakpointConfig> breakpoints;
    std::vector<std::string> watch_conditions;  // Condition-only breakpoints
    
    // File paths
    std::string bios_path;
    std::string rom_path;
    std::string save_state_path;
    std::string screenshot_path;    // For headless mode frame dumps
    
    FrontendConfig() 
        : video_standard(VideoStandard::NTSC)
        , display_scale(2)
        , fullscreen(false)
        , vsync(true)
        , headless(false)
        , sample_rate(44100)
        , audio_buffer_size(512)
        , master_volume(0.7f)
        , audio_enabled(true)
        , show_fps(true)
        , enable_debugger(true)  // Enable by default for ImGui debugger UI
        , trace_level("")        // Off by default
        , enable_profile(false)
        , screenshot_path("screenshots")
    {}
};

// Menu actions
enum class MenuAction {
    None,
    LoadBIOS,
    LoadROM,
    Reset,
    Pause,
    Resume,
    SaveState,
    LoadState,
    Screenshot,
    ToggleDebugger,
    OpenDebugger,
    ToggleFullscreen,
    DisplayInfo,
    Quit,
    // Video Settings submenu
    VideoSettings,
    ScalingFilterNearest,
    ScalingFilterLinear,
    AspectRatioOriginal,
    AspectRatio4_3,
    AspectRatioStretch,
    ToggleVSync,
    CRTEffectNone,
    CRTEffectLight,
    CRTEffectMedium,
    CRTEffectHeavy,
    ScanlinesOff,
    Scanlines25,
    Scanlines50,
    Scanlines75,
    // Audio Settings submenu
    AudioSettings,
    VolumeDown,
    VolumeUp,
    Volume0,
    Volume10,
    Volume20,
    Volume30,
    Volume40,
    Volume50,
    Volume60,
    Volume70,
    Volume80,
    Volume90,
    Volume100,
    ToggleMute,
    AudioBufferSmall,
    AudioBufferMedium,
    AudioBufferLarge
};

// Abstract frontend interface
class Frontend {
public:
    virtual ~Frontend() = default;
    
    // Initialization
    virtual bool initialize(const FrontendConfig& config) = 0;
    virtual void shutdown() = 0;
    
    // Main loop
    virtual void run() = 0;
    virtual bool is_running() const = 0;
    
    // Frame processing
    virtual void render_frame() = 0;
    virtual void process_audio() = 0;
    virtual void process_input() = 0;
    
    // Menu/UI
    virtual MenuAction process_menu() = 0;
    virtual void show_message(const std::string& message) = 0;
    
    // Emulator access
    virtual EmulatorCore* get_emulator() = 0;
    
    // Headless mode support
    virtual void save_screenshot(const std::string& filename) = 0;
    virtual void dump_framebuffer(const std::string& filename) = 0;
};

} // namespace videopac

#endif // VIDEOPAC_FRONTEND_H
