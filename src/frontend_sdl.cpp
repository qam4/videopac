#include "frontend_sdl.h"
#include "types.h"
#include "ui/text_renderer.h"
#include "ui/menu_system.h"
#include "ui/config_manager.h"
#include "ui/file_browser.h"
#include "ui/zip_handler.h"
#include "ui/dialogs.h"
#include "ui/recent_files_list.h"
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
    
    // Initialize UI components
    config_manager_ = std::make_unique<ConfigManager>();
    config_manager_->load();  // Load saved configuration
    
    text_renderer_ = std::make_unique<TextRenderer>(renderer_);
    if (!text_renderer_->initialize()) {
        std::cerr << "Warning: Text renderer initialization failed" << std::endl;
    }
    
    menu_system_ = std::make_unique<MenuSystem>(renderer_, text_renderer_.get());
    menu_system_->build_main_menu();
    
    file_browser_ = std::make_unique<FileBrowser>(renderer_, text_renderer_.get(), config_manager_.get());
    zip_handler_ = std::make_unique<ZIPHandler>();
    message_dialog_ = std::make_unique<MessageDialog>(renderer_, text_renderer_.get());
    progress_dialog_ = std::make_unique<ProgressDialog>(renderer_, text_renderer_.get());
    recent_roms_ = std::make_unique<RecentFilesList>(10);
    recent_bios_ = std::make_unique<RecentFilesList>(10);
    
    // Initialize audio
    if (config_.audio_enabled && !init_audio()) {
        std::cerr << "Warning: Audio initialization failed, continuing without audio" << std::endl;
    }
    
    // Create emulator
    Configuration emu_config;
    emu_config.video_standard = config_.video_standard;
    emu_config.enable_profile = config_.enable_profile;
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
    
    // Initialize debugger if enabled
    if (config_.enable_debugger) {
        debugger_ = std::make_unique<Debugger>(emulator_.get());
        debugger_ui_ = std::make_unique<DebuggerUI>(debugger_.get());
        emulator_->set_debugger(debugger_.get());
        
        // Enable trace if requested (very expensive!)
        if (config_.enable_trace) {
            debugger_->enable_trace(true);
            std::cout << "Instruction trace enabled (performance will be slow)" << std::endl;
        }
        
        // Set breakpoints from config
        for (const auto& bp : config_.breakpoints) {
            if (bp.condition.empty()) {
                debugger_->add_breakpoint(bp.address);
                std::cout << "Breakpoint set at 0x" << std::hex << bp.address << std::dec << std::endl;
            } else {
                debugger_->add_breakpoint(bp.address, bp.condition);
                std::cout << "Conditional breakpoint set at 0x" << std::hex << bp.address << std::dec 
                          << " with condition: " << bp.condition << std::endl;
            }
        }
        
        // Set condition-only breakpoints (watch conditions)
        for (const auto& condition : config_.watch_conditions) {
            debugger_->add_breakpoint(condition);
            std::cout << "Watch condition set: " << condition << std::endl;
        }
        
        std::cout << "Debugger enabled - Press F9 to step, F5 to continue, F1 for help" << std::endl;
    }
    
    running_ = true;
    last_fps_time_ = SDL_GetTicks();
    
    std::cout << "SDL frontend initialized successfully" << std::endl;
    return true;
}

void SDLFrontend::shutdown() {
    std::cout << "SDLFrontend::shutdown() called" << std::endl;
    
    // Clean up temporary ZIP files
    if (zip_handler_) {
        zip_handler_->cleanup_temp_files();
    }
    
    // Save configuration
    if (config_manager_) {
        config_manager_->save();
    }
    
    // Write trace log if debugger is active
    if (debugger_) {
        std::cout << "Debugger exists" << std::endl;
        if (debugger_->is_trace_enabled()) {
            std::cout << "Trace is enabled" << std::endl;
            const auto& trace_log = debugger_->get_trace_log();
            std::cout << "Trace log size: " << trace_log.size() << " instructions" << std::endl;
            if (!trace_log.empty()) {
                std::cout << "Writing trace log to trace.log..." << std::endl;
                std::ofstream trace_file("trace.log");
                if (!trace_file) {
                    std::cerr << "ERROR: Failed to open trace.log for writing!" << std::endl;
                } else {
                    for (const auto& line : trace_log) {
                        trace_file << line << "\n";
                    }
                    trace_file.close();
                    std::cout << "Trace log written successfully (" << trace_log.size() << " instructions)" << std::endl;
                }
            } else {
                std::cout << "Trace log is empty, not writing file" << std::endl;
            }
        } else {
            std::cout << "Trace is NOT enabled" << std::endl;
        }
    } else {
        std::cout << "No debugger instance" << std::endl;
    }
    
    cleanup_audio();
    cleanup_video();
    
    emulator_.reset();
    
    SDL_Quit();
    running_ = false;
}

bool SDLFrontend::init_video() {
    // Proper Videopac display: 320x240 output (2x horizontal scaling)
    // VDC framebuffer is 160x240 (full VDC height to capture status bars)
    int display_width = 320;   // 160 * 2
    int display_height = 240;  // Full VDC height
    int window_width = display_width * config_.display_scale;
    int window_height = display_height * config_.display_scale;
    
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
    
    // Set logical size to 320x240 for proper aspect ratio
    SDL_RenderSetLogicalSize(renderer_, display_width, display_height);
    
    // Use nearest-neighbor filtering for sharp pixels
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");  // 0=nearest (sharp), 1=linear, 2=best
    
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
    try {
        while (running_) {
            uint32 frame_start = SDL_GetTicks();
            
            // Process input
            process_input();
            
            // Run emulator frame if not paused and menu is not active
            bool emulator_paused = debugger_ && debugger_->is_paused();
            bool menu_active = menu_system_ && menu_system_->is_visible();
            
            if (!paused_ && !emulator_paused && !menu_active) {
                emulator_->run_frame();
                frame_count_++;
            }
            
            // Render
            render_frame();
            
            // Process audio (only if not paused)
            if (config_.audio_enabled && !menu_active) {
                process_audio();
            }
            
            // Update FPS counter
            fps_counter_++;
            uint32 current_time = SDL_GetTicks();
            if (current_time - last_fps_time_ >= 1000) {
                current_fps_ = fps_counter_ * 1000.0f / (current_time - last_fps_time_);
                fps_counter_ = 0;
                last_fps_time_ = current_time;
            }
            
            // Frame rate limiting to match video standard (60Hz NTSC / 50Hz PAL)
            uint32 target_frame_time = (config_.video_standard == VideoStandard::NTSC) ? 17 : 20;  // ms
            uint32 elapsed = SDL_GetTicks() - frame_start;
            if (elapsed < target_frame_time) {
                SDL_Delay(target_frame_time - elapsed);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "\n*** Exception caught in main loop: " << e.what() << std::endl;
        std::cerr << "Shutting down gracefully to save trace..." << std::endl;
        running_ = false;
    }
}

bool SDLFrontend::is_running() const {
    return running_;
}

void SDLFrontend::render_frame() {
    update_texture();
    
    // Clear renderer (black background)
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    
    // Render texture: 160x240 framebuffer to 320x240 display (2x horizontal scaling)
    SDL_Rect dest_rect;
    dest_rect.x = 0;
    dest_rect.y = 0;
    dest_rect.w = 320; // 160 * 2
    dest_rect.h = 240; // No vertical scaling
    
    SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
    
    // Render menu overlay if visible
    if (menu_system_ && menu_system_->is_visible()) {
        menu_system_->render();
    }
    
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
    // Calculate how many audio samples we need per video frame
    float frame_rate = (config_.video_standard == VideoStandard::NTSC) ? 60.0f : 50.0f;
    int samples_per_frame = static_cast<int>(config_.sample_rate / frame_rate);
    
    // Use the emulator's audio buffer method to get properly generated samples
    std::vector<int16> temp_buffer(samples_per_frame);
    emulator_->get_audio_buffer(temp_buffer.data(), samples_per_frame);
    
    SDL_LockAudioDevice(audio_device_);
    
    // Add samples to circular buffer
    for (int i = 0; i < samples_per_frame; i++) {
        // Apply volume
        int16 sample = static_cast<int16>(temp_buffer[i] * config_.master_volume);
        
        // Add to buffer (circular buffer)
        audio_buffer_[audio_write_pos_] = sample;
        audio_write_pos_ = (audio_write_pos_ + 1) % audio_buffer_.size();
    }
    
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
    
    // Ignore key repeat events (when holding a key down)
    if (event.repeat != 0) {
        return;
    }
    
    // If menu is active, route input to menu system
    if (menu_system_ && menu_system_->is_visible() && key_down) {
        MenuAction action = menu_system_->process_input(event.keysym.sym);
        
        // Handle menu actions
        if (action != MenuAction::None) {
            handle_menu_action(action);
        }
        return;
    }
    
    // Check for menu toggle keys (only on key down)
    // F10, F1, or backtick (`) to toggle menu
    if (key_down && (event.keysym.sym == SDLK_F10 || 
                     event.keysym.sym == SDLK_F1 ||
                     event.keysym.sym == SDLK_BACKQUOTE)) {
        if (menu_system_) {
            if (menu_system_->is_visible()) {
                menu_system_->hide();
            } else {
                menu_system_->show();
            }
        }
        return;
    }
    
    // Handle joystick keys (both press and release)
    switch (event.keysym.sym) {
        // Arrow keys + Space for joystick 1
        case SDLK_UP:
            emulator_->get_input_handler().set_joystick_state(0, Direction::Up, key_down);
            return;
        case SDLK_DOWN:
            emulator_->get_input_handler().set_joystick_state(0, Direction::Down, key_down);
            return;
        case SDLK_LEFT:
            emulator_->get_input_handler().set_joystick_state(0, Direction::Left, key_down);
            return;
        case SDLK_RIGHT:
            emulator_->get_input_handler().set_joystick_state(0, Direction::Right, key_down);
            return;
        case SDLK_SPACE:
            emulator_->get_input_handler().set_joystick_button(0, key_down);
            return;
            
        // WASD + Left Shift for joystick 2
        case SDLK_w:
            emulator_->get_input_handler().set_joystick_state(1, Direction::Up, key_down);
            return;
        case SDLK_s:
            emulator_->get_input_handler().set_joystick_state(1, Direction::Down, key_down);
            return;
        case SDLK_a:
            emulator_->get_input_handler().set_joystick_state(1, Direction::Left, key_down);
            return;
        case SDLK_d:
            emulator_->get_input_handler().set_joystick_state(1, Direction::Right, key_down);
            return;
        case SDLK_LSHIFT:
            emulator_->get_input_handler().set_joystick_button(1, key_down);
            return;
    }
    
    // Check for special keys (only on key down)
    if (key_down) {
        switch (event.keysym.sym) {
            case SDLK_ESCAPE:
                running_ = false;
                return;
                
            case SDLK_F5:
                if (debugger_ && debugger_->is_paused()) {
                    debugger_->continue_execution();
                    std::cout << "Continuing execution..." << std::endl;
                } else {
                    emulator_->reset();
                }
                return;
                
            case SDLK_F9:
                if (debugger_) {
                    debugger_->step();
                    debugger_ui_->display_cpu_state();
                    debugger_ui_->display_disassembly(2, 5);
                }
                return;
                
            case SDLK_F1:
                if (debugger_) {
                    debugger_ui_->print_help();
                }
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
    if (menu_system_ && menu_system_->is_visible()) {
        // Menu is being processed in handle_keyboard_event
        return MenuAction::None;
    }
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

// Menu action handlers
void SDLFrontend::handle_menu_action(videopac::MenuAction action) {
    switch (action) {
        case MenuAction::LoadBIOS:
            handle_load_bios();
            break;
        case MenuAction::LoadROM:
            handle_load_rom();
            break;
        case MenuAction::Reset:
            handle_reset();
            break;
        case MenuAction::Quit:
            handle_exit();
            break;
        default:
            std::cout << "Unhandled menu action: " << static_cast<int>(action) << std::endl;
            break;
    }
}

void SDLFrontend::handle_load_bios() {
    // Hide menu temporarily
    menu_system_->hide();
    
    // Open file browser for BIOS files
    file_browser_->open(".bin,.rom", FileBrowser::FileType::BIOS);
    
    // Process file browser until closed
    while (file_browser_->is_open() && running_) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running_ = false;
                file_browser_->close();
                return;
            }
            if (event.type == SDL_KEYDOWN) {
                file_browser_->process_input(event.key.keysym.sym);
            }
        }
        
        // Render
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        update_texture();
        SDL_Rect dest_rect = {0, 0, 320, 240};
        SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
        file_browser_->render();
        SDL_RenderPresent(renderer_);
        
        SDL_Delay(16);  // ~60 FPS
    }
    
    // Check if a file was selected
    if (file_browser_->was_file_selected()) {
        std::string file_path = file_browser_->get_selected_file();
        
        // Show progress dialog
        progress_dialog_->set_message("Loading BIOS...");
        progress_dialog_->show();
        
        // Render progress dialog
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        update_texture();
        SDL_Rect dest_rect = {0, 0, 320, 240};
        SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
        progress_dialog_->render();
        SDL_RenderPresent(renderer_);
        
        // Load the BIOS file
        bool success = load_bios_file(file_path);
        
        progress_dialog_->hide();
        
        // Show result message
        if (success) {
            message_dialog_->set_message("Success", "BIOS loaded successfully");
            recent_bios_->add(file_path);
            config_manager_->save();  // Save updated recent files
        } else {
            message_dialog_->set_message("Error", "Failed to load BIOS file");
        }
        
        message_dialog_->show();
        
        // Wait for user to dismiss message
        bool message_dismissed = false;
        uint32_t message_start_time = SDL_GetTicks();
        while (!message_dismissed && running_) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running_ = false;
                    return;
                }
                if (event.type == SDL_KEYDOWN) {
                    if (message_dialog_->process_input(event.key.keysym.sym)) {
                        message_dismissed = true;
                    }
                }
            }
            
            // Auto-dismiss success messages after 2 seconds
            if (success && (SDL_GetTicks() - message_start_time) >= 2000) {
                message_dismissed = true;
            }
            
            // Render
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            SDL_RenderClear(renderer_);
            update_texture();
            SDL_Rect dest_rect = {0, 0, 320, 240};
            SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
            message_dialog_->render();
            SDL_RenderPresent(renderer_);
            
            SDL_Delay(16);
        }
        
        message_dialog_->hide();
    }
    
    // Show menu again
    menu_system_->show();
}

void SDLFrontend::handle_load_rom() {
    // Hide menu temporarily
    menu_system_->hide();
    
    // Open file browser for ROM files (including ZIP)
    file_browser_->open(".bin,.rom,.zip", FileBrowser::FileType::ROM);
    
    // Process file browser until closed
    while (file_browser_->is_open() && running_) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running_ = false;
                file_browser_->close();
                return;
            }
            if (event.type == SDL_KEYDOWN) {
                file_browser_->process_input(event.key.keysym.sym);
            }
        }
        
        // Render
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        update_texture();
        SDL_Rect dest_rect = {0, 0, 320, 240};
        SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
        file_browser_->render();
        SDL_RenderPresent(renderer_);
        
        SDL_Delay(16);  // ~60 FPS
    }
    
    // Check if a file was selected
    if (file_browser_->was_file_selected()) {
        std::string file_path = file_browser_->get_selected_file();
        
        // Check if it's a ZIP file
        if (file_path.size() >= 4 && file_path.substr(file_path.size() - 4) == ".zip") {
            file_path = handle_zip_file(file_path);
            if (file_path.empty()) {
                // ZIP handling failed or was cancelled
                menu_system_->show();
                return;
            }
        }
        
        // Show progress dialog
        progress_dialog_->set_message("Loading ROM...");
        progress_dialog_->show();
        
        // Render progress dialog
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        update_texture();
        SDL_Rect dest_rect = {0, 0, 320, 240};
        SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
        progress_dialog_->render();
        SDL_RenderPresent(renderer_);
        
        // Load the ROM file
        bool success = load_rom_file(file_path);
        
        progress_dialog_->hide();
        
        // Show result message
        if (success) {
            message_dialog_->set_message("Success", "ROM loaded successfully");
            recent_roms_->add(file_path);
            config_manager_->save();  // Save updated recent files
        } else {
            message_dialog_->set_message("Error", "Failed to load ROM file");
        }
        
        message_dialog_->show();
        
        // Wait for user to dismiss message
        bool message_dismissed = false;
        uint32_t message_start_time = SDL_GetTicks();
        while (!message_dismissed && running_) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running_ = false;
                    return;
                }
                if (event.type == SDL_KEYDOWN) {
                    if (message_dialog_->process_input(event.key.keysym.sym)) {
                        message_dismissed = true;
                    }
                }
            }
            
            // Auto-dismiss success messages after 2 seconds
            if (success && (SDL_GetTicks() - message_start_time) >= 2000) {
                message_dismissed = true;
            }
            
            // Render
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            SDL_RenderClear(renderer_);
            update_texture();
            SDL_Rect dest_rect = {0, 0, 320, 240};
            SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
            message_dialog_->render();
            SDL_RenderPresent(renderer_);
            
            SDL_Delay(16);
        }
        
        message_dialog_->hide();
    }
    
    // Show menu again
    menu_system_->show();
}

void SDLFrontend::handle_reset() {
    if (emulator_) {
        emulator_->reset();
        message_dialog_->set_message("Reset", "Emulator reset successfully");
        message_dialog_->show();
        
        // Auto-dismiss after 2 seconds
        uint32_t start_time = SDL_GetTicks();
        while (message_dialog_->is_visible() && running_) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running_ = false;
                    return;
                }
                if (event.type == SDL_KEYDOWN) {
                    message_dialog_->process_input(event.key.keysym.sym);
                }
            }
            
            if ((SDL_GetTicks() - start_time) >= 2000) {
                message_dialog_->hide();
            }
            
            // Render
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            SDL_RenderClear(renderer_);
            update_texture();
            SDL_Rect dest_rect = {0, 0, 320, 240};
            SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
            message_dialog_->render();
            SDL_RenderPresent(renderer_);
            
            SDL_Delay(16);
        }
    }
}

void SDLFrontend::handle_display_info() {
    // TODO: Implement display info dialog
    std::cout << "Display info not yet implemented" << std::endl;
}

void SDLFrontend::handle_exit() {
    running_ = false;
}

// File loading helpers
bool SDLFrontend::load_bios_file(const std::string& path) {
    if (!emulator_) {
        return false;
    }
    
    auto result = emulator_->load_bios(path);
    if (result.is_err()) {
        std::cerr << "Failed to load BIOS: " << result.error << std::endl;
        return false;
    }
    
    return true;
}

bool SDLFrontend::load_rom_file(const std::string& path) {
    if (!emulator_) {
        return false;
    }
    
    auto result = emulator_->load_rom(path);
    if (result.is_err()) {
        std::cerr << "Failed to load ROM: " << result.error << std::endl;
        return false;
    }
    
    // Reset emulator after loading ROM
    emulator_->reset();
    
    return true;
}

std::string SDLFrontend::handle_zip_file(const std::string& zip_path) {
    // Show progress dialog
    progress_dialog_->set_message("Extracting ZIP...");
    progress_dialog_->show();
    
    // Render progress dialog
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    update_texture();
    SDL_Rect dest_rect = {0, 0, 320, 240};
    SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
    progress_dialog_->render();
    SDL_RenderPresent(renderer_);
    
    // Open ZIP file
    if (!zip_handler_->open(zip_path)) {
        progress_dialog_->hide();
        message_dialog_->set_message("Error", "Failed to open ZIP file");
        message_dialog_->show();
        
        // Wait for dismissal
        while (message_dialog_->is_visible() && running_) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running_ = false;
                    return "";
                }
                if (event.type == SDL_KEYDOWN) {
                    message_dialog_->process_input(event.key.keysym.sym);
                }
            }
            
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            SDL_RenderClear(renderer_);
            update_texture();
            SDL_Rect dest_rect = {0, 0, 320, 240};
            SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
            message_dialog_->render();
            SDL_RenderPresent(renderer_);
            SDL_Delay(16);
        }
        
        message_dialog_->hide();
        return "";
    }
    
    // Get list of ROM files
    std::vector<std::string> rom_files = zip_handler_->get_rom_files();
    
    progress_dialog_->hide();
    
    if (rom_files.empty()) {
        message_dialog_->set_message("Error", "No ROM files found in ZIP archive");
        message_dialog_->show();
        
        // Wait for dismissal
        while (message_dialog_->is_visible() && running_) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running_ = false;
                    return "";
                }
                if (event.type == SDL_KEYDOWN) {
                    message_dialog_->process_input(event.key.keysym.sym);
                }
            }
            
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            SDL_RenderClear(renderer_);
            update_texture();
            SDL_Rect dest_rect = {0, 0, 320, 240};
            SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
            message_dialog_->render();
            SDL_RenderPresent(renderer_);
            SDL_Delay(16);
        }
        
        message_dialog_->hide();
        zip_handler_->close();
        return "";
    }
    
    // If only one ROM file, extract it automatically
    std::string selected_file;
    if (rom_files.size() == 1) {
        selected_file = rom_files[0];
    } else {
        // TODO: Show selection dialog for multiple ROMs
        // For now, just use the first one
        selected_file = rom_files[0];
        std::cout << "Multiple ROMs found in ZIP, using first: " << selected_file << std::endl;
    }
    
    // Extract the selected file
    std::string extracted_path = zip_handler_->extract_file(selected_file);
    zip_handler_->close();
    
    if (extracted_path.empty()) {
        message_dialog_->set_message("Error", "Failed to extract ROM from ZIP");
        message_dialog_->show();
        
        // Wait for dismissal
        while (message_dialog_->is_visible() && running_) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running_ = false;
                    return "";
                }
                if (event.type == SDL_KEYDOWN) {
                    message_dialog_->process_input(event.key.keysym.sym);
                }
            }
            
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            SDL_RenderClear(renderer_);
            update_texture();
            SDL_Rect dest_rect = {0, 0, 320, 240};
            SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
            message_dialog_->render();
            SDL_RenderPresent(renderer_);
            SDL_Delay(16);
        }
        
        message_dialog_->hide();
        return "";
    }
    
    return extracted_path;
}

} // namespace videopac
