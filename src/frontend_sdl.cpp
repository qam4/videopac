#include "frontend_sdl.h"
#include "types.h"
#include <iostream>
#include <cstring>
#include <fstream>

namespace videopac {

SDLFrontend::SDLFrontend()
    : window_(nullptr)
    , renderer_(nullptr)
    , texture_(nullptr)
    , audio_device_(0)
    , running_(false)
    , paused_(false)
    , frame_count_(0)
    , last_fps_time_(0)
    , fps_counter_(0)
    , current_fps_(0.0f)
    , audio_write_pos_(0)
    , audio_read_pos_(0)
{
}

SDLFrontend::~SDLFrontend() {
    shutdown();
}

bool SDLFrontend::initialize(const FrontendConfig& config) {
    config_ = config;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Initialize video
    if (!init_video()) {
        shutdown();
        return false;
    }
    
    // Initialize audio
    if (config_.audio_enabled && !init_audio()) {
        std::cerr << "Warning: Audio initialization failed, continuing without audio" << std::endl;
    }
    
    // Create emulator
    Configuration emu_config;
    emu_config.video_standard = config_.video_standard;
    emulator_ = std::make_unique<EmulatorCore>(emu_config);
    
    // Load BIOS
    if (!config_.bios_path.empty()) {
        auto result = emulator_->load_bios(config_.bios_path);
        if (result.is_err()) {
            std::cerr << "Failed to load BIOS: " << result.error << std::endl;
            shutdown();
            return false;
        }
    }
    
    // Load ROM
    if (!config_.rom_path.empty()) {
        auto result = emulator_->load_rom(config_.rom_path);
        if (result.is_err()) {
            std::cerr << "Failed to load ROM: " << result.error << std::endl;
            shutdown();
            return false;
        }
    }
    
    // Reset emulator
    emulator_->reset();
    
    running_ = true;
    last_fps_time_ = SDL_GetTicks();
    
    std::cout << "SDL frontend initialized successfully" << std::endl;
    return true;
}

void SDLFrontend::shutdown() {
    cleanup_audio();
    cleanup_video();
    
    emulator_.reset();
    
    SDL_Quit();
    running_ = false;
}

bool SDLFrontend::init_video() {
    // Calculate window size
    int window_width = FRAMEBUFFER_WIDTH * config_.display_scale;
    int window_height = FRAMEBUFFER_HEIGHT * config_.display_scale;
    
    // Create window
    uint32 window_flags = SDL_WINDOW_SHOWN;
    if (config_.fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    
    window_ = SDL_CreateWindow(
        "Videopac Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        window_flags
    );
    
    if (!window_) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create renderer
    uint32 renderer_flags = SDL_RENDERER_ACCELERATED;
    if (config_.vsync) {
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
    }
    
    renderer_ = SDL_CreateRenderer(window_, -1, renderer_flags);
    if (!renderer_) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Set logical size for automatic scaling
    SDL_RenderSetLogicalSize(renderer_, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    
    // Create texture for framebuffer
    texture_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        FRAMEBUFFER_WIDTH,
        FRAMEBUFFER_HEIGHT
    );
    
    if (!texture_) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        return false;
    }
    
    return true;
}

bool SDLFrontend::init_audio() {
    // Allocate audio buffer (1 second worth of samples)
    audio_buffer_.resize(config_.sample_rate * 2);  // Stereo
    
    // Set up audio specification
    SDL_AudioSpec desired_spec, obtained_spec;
    SDL_zero(desired_spec);
    
    desired_spec.freq = config_.sample_rate;
    desired_spec.format = AUDIO_S16SYS;
    desired_spec.channels = 1;  // Mono
    desired_spec.samples = config_.audio_buffer_size;
    desired_spec.callback = audio_callback;
    desired_spec.userdata = this;
    
    audio_device_ = SDL_OpenAudioDevice(nullptr, 0, &desired_spec, &obtained_spec, 0);
    if (audio_device_ == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Start audio playback
    SDL_PauseAudioDevice(audio_device_, 0);
    
    return true;
}

void SDLFrontend::cleanup_video() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

void SDLFrontend::cleanup_audio() {
    if (audio_device_ != 0) {
        SDL_CloseAudioDevice(audio_device_);
        audio_device_ = 0;
    }
}

void SDLFrontend::run() {
    while (running_) {
        // Process input
        process_input();
        
        // Run emulator frame if not paused
        if (!paused_) {
            emulator_->run_frame();
            frame_count_++;
        }
        
        // Render
        render_frame();
        
        // Process audio
        if (config_.audio_enabled) {
            process_audio();
        }
        
        // Update FPS counter
        fps_counter_++;
        uint32 current_time = SDL_GetTicks();
        if (current_time - last_fps_time_ >= 1000) {
            current_fps_ = fps_counter_ * 1000.0f / (current_time - last_fps_time_);
            fps_counter_ = 0;
            last_fps_time_ = current_time;
            
            if (config_.show_fps) {
                std::cout << "FPS: " << current_fps_ << std::endl;
            }
        }
    }
}

bool SDLFrontend::is_running() const {
    return running_;
}

void SDLFrontend::render_frame() {
    update_texture();
    
    // Clear renderer
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    
    // Render texture
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    
    // Present
    SDL_RenderPresent(renderer_);
}

void SDLFrontend::update_texture() {
    const uint8* framebuffer = emulator_->get_framebuffer();
    
    // Lock texture for writing
    void* pixels;
    int pitch;
    if (SDL_LockTexture(texture_, nullptr, &pixels, &pitch) < 0) {
        std::cerr << "Failed to lock texture: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Convert palette indices to RGB
    uint8* rgb_pixels = static_cast<uint8*>(pixels);
    for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
        for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
            uint8 palette_index = framebuffer[y * FRAMEBUFFER_WIDTH + x];
            Color color = PALETTE_BRIGHT[palette_index % 8];
            
            int pixel_offset = (y * pitch) + (x * 3);
            rgb_pixels[pixel_offset + 0] = color.r;
            rgb_pixels[pixel_offset + 1] = color.g;
            rgb_pixels[pixel_offset + 2] = color.b;
        }
    }
    
    SDL_UnlockTexture(texture_);
}

void SDLFrontend::process_audio() {
    // Get audio sample from emulator
    // For now, just get one sample per frame
    // TODO: Generate proper number of samples based on frame rate
    int16 sample = emulator_->get_vdc().get_audio_sample();
    
    // Apply volume
    sample = static_cast<int16>(sample * config_.master_volume);
    
    // Add to buffer (circular buffer)
    SDL_LockAudioDevice(audio_device_);
    audio_buffer_[audio_write_pos_] = sample;
    audio_write_pos_ = (audio_write_pos_ + 1) % audio_buffer_.size();
    SDL_UnlockAudioDevice(audio_device_);
}

void SDLFrontend::process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                handle_quit_event();
                break;
                
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                handle_keyboard_event(event.key);
                break;
        }
    }
}

void SDLFrontend::handle_keyboard_event(const SDL_KeyboardEvent& event) {
    bool key_down = (event.type == SDL_KEYDOWN);
    
    // Check for special keys
    if (key_down) {
        switch (event.keysym.sym) {
            case SDLK_ESCAPE:
                running_ = false;
                return;
                
            case SDLK_F5:
                emulator_->reset();
                return;
                
            case SDLK_PAUSE:
            case SDLK_p:
                paused_ = !paused_;
                return;
                
            case SDLK_F12:
                save_screenshot("screenshot.ppm");
                return;
        }
    }
    
    // Map to Videopac key
    VidKey vid_key = map_sdl_key(event.keysym.sym);
    if (static_cast<uint8>(vid_key) != 0xFF) {
        emulator_->get_input_handler().set_key_state(vid_key, key_down);
    }
}

void SDLFrontend::handle_quit_event() {
    running_ = false;
}

void SDLFrontend::audio_callback(void* userdata, uint8* stream, int len) {
    SDLFrontend* frontend = static_cast<SDLFrontend*>(userdata);
    int16* output = reinterpret_cast<int16*>(stream);
    int samples = len / sizeof(int16);
    
    for (int i = 0; i < samples; i++) {
        if (frontend->audio_read_pos_ != frontend->audio_write_pos_) {
            output[i] = frontend->audio_buffer_[frontend->audio_read_pos_];
            frontend->audio_read_pos_ = (frontend->audio_read_pos_ + 1) % frontend->audio_buffer_.size();
        } else {
            output[i] = 0;  // Silence if buffer is empty
        }
    }
}

MenuAction SDLFrontend::process_menu() {
    // TODO: Implement menu system
    return MenuAction::None;
}

void SDLFrontend::show_message(const std::string& message) {
    std::cout << message << std::endl;
}

EmulatorCore* SDLFrontend::get_emulator() {
    return emulator_.get();
}

void SDLFrontend::save_screenshot(const std::string& filename) {
    dump_framebuffer(filename);
    std::cout << "Screenshot saved to " << filename << std::endl;
}

void SDLFrontend::dump_framebuffer(const std::string& filename) {
    const uint8* framebuffer = emulator_->get_framebuffer();
    
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }
    
    // Write PPM header
    file << "P6\n" << FRAMEBUFFER_WIDTH << " " << FRAMEBUFFER_HEIGHT << "\n255\n";
    
    // Write pixel data
    for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
        for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
            uint8 palette_index = framebuffer[y * FRAMEBUFFER_WIDTH + x];
            Color color = PALETTE_BRIGHT[palette_index % 8];
            file.put(color.r);
            file.put(color.g);
            file.put(color.b);
        }
    }
}

VidKey SDLFrontend::map_sdl_key(SDL_Keycode key) {
    // Map SDL keys to Videopac keyboard
    switch (key) {
        // Number keys
        case SDLK_0: return VidKey::Key0;
        case SDLK_1: return VidKey::Key1;
        case SDLK_2: return VidKey::Key2;
        case SDLK_3: return VidKey::Key3;
        case SDLK_4: return VidKey::Key4;
        case SDLK_5: return VidKey::Key5;
        case SDLK_6: return VidKey::Key6;
        case SDLK_7: return VidKey::Key7;
        case SDLK_8: return VidKey::Key8;
        case SDLK_9: return VidKey::Key9;
        
        // Letter keys
        case SDLK_a: return VidKey::KeyA;
        case SDLK_b: return VidKey::KeyB;
        case SDLK_c: return VidKey::KeyC;
        case SDLK_d: return VidKey::KeyD;
        case SDLK_e: return VidKey::KeyE;
        case SDLK_f: return VidKey::KeyF;
        case SDLK_g: return VidKey::KeyG;
        case SDLK_h: return VidKey::KeyH;
        case SDLK_i: return VidKey::KeyI;
        case SDLK_j: return VidKey::KeyJ;
        case SDLK_k: return VidKey::KeyK;
        case SDLK_l: return VidKey::KeyL;
        case SDLK_m: return VidKey::KeyM;
        case SDLK_n: return VidKey::KeyN;
        case SDLK_o: return VidKey::KeyO;
        case SDLK_p: return VidKey::KeyP;
        case SDLK_q: return VidKey::KeyQ;
        case SDLK_r: return VidKey::KeyR;
        case SDLK_s: return VidKey::KeyS;
        case SDLK_t: return VidKey::KeyT;
        case SDLK_u: return VidKey::KeyU;
        case SDLK_v: return VidKey::KeyV;
        case SDLK_w: return VidKey::KeyW;
        case SDLK_x: return VidKey::KeyX;
        case SDLK_y: return VidKey::KeyY;
        case SDLK_z: return VidKey::KeyZ;
        
        // Special keys
        case SDLK_SPACE: return VidKey::Space;
        case SDLK_RETURN: return VidKey::Enter;
        case SDLK_PERIOD: return VidKey::Period;
        case SDLK_PLUS: return VidKey::Plus;
        case SDLK_MINUS: return VidKey::Minus;
        case SDLK_ASTERISK: return VidKey::Multiply;
        case SDLK_SLASH: return VidKey::Divide;
        case SDLK_EQUALS: return VidKey::Equal;
        case SDLK_QUESTION: return VidKey::Question;
        
        default:
            // Return a sentinel value - use Key0 with invalid state
            // The caller will check if the key is valid
            return static_cast<VidKey>(0xFF);
    }
}

} // namespace videopac
