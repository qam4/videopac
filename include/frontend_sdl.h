#ifndef VIDEOPAC_FRONTEND_SDL_H
#define VIDEOPAC_FRONTEND_SDL_H

#include "frontend.h"
#include "emulator.h"
#include "debugger.h"
#include "debugger_ui.h"
#include <SDL2/SDL.h>
#include <memory>
#include <vector>

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
    
    // Configuration
    FrontendConfig config_;
    
    // State
    bool running_;
    bool paused_;
    uint32_t frame_count_;
    uint32_t last_fps_time_;
    uint32_t fps_counter_;
    float current_fps_;
    
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
    
    static void audio_callback(void* userdata, uint8* stream, int len);
    
    // Input mapping
    VidKey map_sdl_key(SDL_Keycode key);
};

} // namespace videopac

#endif // VIDEOPAC_FRONTEND_SDL_H
