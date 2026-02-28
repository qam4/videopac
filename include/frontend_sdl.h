#ifndef VIDEOPAC_FRONTEND_SDL_H
#define VIDEOPAC_FRONTEND_SDL_H

#include "frontend.h"
#include "emulator.h"
#include "input.h"
#include "debugger.h"
#include "debugger_ui.h"
#include "ui/osd_renderer.h"  // Need full definition for OSDPosition enum
#include "ui/imgui_debugger_ui.h"
#include <SDL.h>
#include <memory>
#include <vector>

// Forward declarations for UI components
class TextRenderer;
class MenuSystem;
class ConfigManager;
class FileBrowser;
class ZIPHandler;
class MessageDialog;
class ProgressDialog;
class RecentFilesList;
class SaveStateManagerUI;

namespace videopac {

class SDLFrontend : public Frontend {
public:
    SDLFrontend();
    ~SDLFrontend() override;
    
    // Frontend interface implementation
    bool initialize(const FrontendConfig& config) override;
    void shutdown() override;
    void run() override;
    bool is_running() const override;
    
    void render_frame() override;
    void process_audio() override;
    void process_input() override;
    
    MenuAction process_menu() override;
    void show_message(const std::string& message) override;
    
    EmulatorCore* get_emulator() override;
    
    void save_screenshot(const std::string& filename) override;
    void dump_framebuffer(const std::string& filename) override;
    
    // Testing/debugging methods
    void schedule_key_press(VidKey key, int trigger_frame, int duration_frames);
    void schedule_joystick_press(int joystick, Direction direction, int trigger_frame, int duration_frames);
    void set_disable_sdl_input(bool disable) { disable_sdl_input_ = disable; }
    void set_frame_limit(int frames) { frame_limit_ = frames; }
    
private:
    // SDL resources
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
    SDL_AudioDeviceID audio_device_;
    
    // Emulator
    std::unique_ptr<EmulatorCore> emulator_;
    
    // Debugger
    std::unique_ptr<Debugger> debugger_;
    std::unique_ptr<DebuggerUI> debugger_ui_;
    std::unique_ptr<ImGuiDebuggerUI> imgui_debugger_ui_;
    
    // UI components
    std::unique_ptr<TextRenderer> text_renderer_;
    std::unique_ptr<MenuSystem> menu_system_;
    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<FileBrowser> file_browser_;
    std::unique_ptr<ZIPHandler> zip_handler_;
    std::unique_ptr<MessageDialog> message_dialog_;
    std::unique_ptr<ProgressDialog> progress_dialog_;
    std::unique_ptr<RecentFilesList> recent_roms_;
    std::unique_ptr<RecentFilesList> recent_bios_;
    std::unique_ptr<SaveStateManagerUI> save_state_manager_;
    std::unique_ptr<OSDRenderer> osd_renderer_;
    
    // Configuration
    FrontendConfig config_;
    
    // State
    bool running_;
    bool paused_;
    uint32_t frame_count_;
    uint32_t frame_limit_;  // Frame limit for testing (0 = no limit)
    uint32_t last_fps_time_;
    uint32_t fps_counter_;
    float current_fps_;
    
    // UI state
    bool show_fps_;
    OSDRenderer::OSDPosition fps_position_;  // Configurable FPS display position
    bool audio_muted_;
    bool turbo_mode_;
    
    // Fullscreen state
    bool is_fullscreen_;
    int windowed_width_;
    int windowed_height_;
    int windowed_x_;
    int windowed_y_;
    
    // Current file names
    std::string current_rom_name_;
    std::string current_bios_name_;
    
    // Scheduled key presses (for testing/debugging)
    struct KeyPress {
        VidKey key;
        int frames_remaining;
    };
    std::vector<KeyPress> active_keys_;
    bool disable_sdl_input_;  // Flag to disable SDL input processing
    
    // Scheduled joystick presses (for testing/debugging)
    struct ScheduledJoystick {
        int joystick;
        Direction direction;
        int trigger_frame;
        int duration;
    };
    struct ActiveJoystick {
        int joystick;
        Direction direction;
        int frames_remaining;
    };
    std::vector<ScheduledJoystick> scheduled_joystick_;
    std::vector<ActiveJoystick> active_joystick_;
    
    // Audio buffer
    std::vector<int16> audio_buffer_;
    size_t audio_write_pos_;
    size_t audio_read_pos_;
    
    // Helper methods
    bool init_video();
    bool init_audio();
    void cleanup_video();
    void cleanup_audio();
    
    void update_texture();
    void handle_keyboard_event(const SDL_KeyboardEvent& event);
    void handle_quit_event();
    
    // Fullscreen helper
    void toggle_fullscreen();
    
    // OSD helper
    OSDRenderer::OSDPosition string_to_osd_position(const std::string& position) const;
    
    // Video rendering helpers
    SDL_Rect calculate_viewport() const;
    void render_crt_effects(const SDL_Rect& viewport);
    void render_scanlines(const SDL_Rect& viewport);
    
    // Menu action handlers
    void handle_menu_action(videopac::MenuAction action);
    void handle_load_bios();
    void handle_load_rom();
    void handle_save_state(int slot);
    void handle_load_state(int slot);
    void handle_delete_state(int slot);
    void handle_reset();
    void handle_display_info();
    void handle_exit();
    
    // File loading helpers
    bool load_bios_file(const std::string& path);
    bool load_rom_file(const std::string& path);
    std::string extract_if_zip(const std::string& path);  // Helper to extract ZIP files
    std::string handle_zip_file(const std::string& zip_path);
    
    static void audio_callback(void* userdata, uint8* stream, int len);
    
    // Input mapping
    VidKey map_sdl_key(SDL_Keycode key);
};

} // namespace videopac

#endif // VIDEOPAC_FRONTEND_SDL_H
