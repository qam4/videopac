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

struct ScheduledKey {
    int key_code;
    int trigger_frame;
    int duration_frames;
    
    ScheduledKey() : key_code(0), trigger_frame(0), duration_frames(5) {}
    ScheduledKey(int key, int frame, int duration = 5) 
        : key_code(key), trigger_frame(frame), duration_frames(duration) {}
};

struct FrontendConfig {
    // Video settings
    VideoStandard video_standard;
    PaletteMode palette_mode;       // Separate from video timing
    int display_scale;              // 1x, 2x, 3x, 4x
    bool fullscreen;
    bool headless;                  // Run without window (for remote/testing)
    std::string aspect_ratio;       // "original", "4:3", "stretch"
    
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
    bool enable_vdc_trace;          // Enable VDC register write trace
    bool enable_profile;            // Enable performance profiling
    std::vector<BreakpointConfig> breakpoints;
    std::vector<std::string> watch_conditions;  // Condition-only breakpoints
    std::vector<ScheduledKey> scheduled_keys;   // Scheduled key presses for testing
    
    // File paths
    std::string bios_path;
    std::string rom_path;
    std::string save_state_path;
    std::string screenshot_path;    // For headless mode frame dumps
    
    FrontendConfig() 
        : video_standard(VideoStandard::NTSC)
        , palette_mode(PaletteMode::STANDARD)  // Default to Standard O2 palette
        , display_scale(2)
        , fullscreen(false)
        , headless(false)
        , aspect_ratio("4:3")       // Default to 4:3 (CRT aspect ratio)
        , sample_rate(48000)
        , audio_buffer_size(512)
        , master_volume(0.7f)
        , audio_enabled(true)
        , show_fps(true)
        , enable_debugger(true)  // Enable by default for ImGui debugger UI
        , trace_level("")        // Off by default
        , enable_vdc_trace(false)  // Off by default
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
    ToggleFPS,
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
