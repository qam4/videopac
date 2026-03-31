#include "frontend_sdl.h"
#include "types.h"
#include "version.h"
#include "ui/text_renderer.h"
#include "ui/menu_system.h"
#include "ui/config_manager.h"
#include "ui/input_mapper.h"
#include "ui/file_browser.h"
#include "ui/zip_handler.h"
#include "ui/dialogs.h"
#include "ui/recent_files_list.h"
#include "ui/save_state_manager.h"
#include "ui/osd_renderer.h"
#include "ui/imgui_debugger_ui.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <fstream>
#include <ctime>
#include <filesystem>
#include <chrono>

namespace videopac {

SDLFrontend::SDLFrontend()
    : window_(nullptr)
    , renderer_(nullptr)
    , texture_(nullptr)
    , audio_device_(0)
    , running_(false)
    , paused_(false)
    , frame_count_(0)
    , frame_limit_(0)  // No frame limit by default
    , last_fps_time_(0)
    , fps_counter_(0)
    , current_fps_(0.0f)
    , show_fps_(true)  // Show FPS by default
    , fps_position_(OSDRenderer::OSDPosition::TopRight)  // Default position
    , audio_muted_(false)
    , turbo_mode_(false)
    , swap_joysticks_(false)
    , is_fullscreen_(false)
    , windowed_width_(0)
    , windowed_height_(0)
    , windowed_x_(0)
    , windowed_y_(0)
    , current_rom_name_("")
    , current_bios_name_("")
    , disable_sdl_input_(false)
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
    std::cout << "Config file: " << config_manager_->get_config_path() << std::endl;
    
    // Apply saved scaling filter preference
    std::string scaling_filter = config_manager_->get_scaling_filter();
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scaling_filter == "linear" ? "1" : "0");
    
    // Requirement 12.7: Restore fullscreen state on startup
    bool saved_fullscreen = config_manager_->get_fullscreen();
    if (saved_fullscreen != is_fullscreen_) {
        // Apply saved fullscreen preference
        if (saved_fullscreen) {
            SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
            is_fullscreen_ = true;
        }
        // If saved preference is windowed and we're already windowed, no action needed
    }
    
    text_renderer_ = std::make_unique<TextRenderer>(renderer_);
    if (!text_renderer_->initialize()) {
        std::cerr << "Warning: Text renderer initialization failed" << std::endl;
    }
    
    osd_renderer_ = std::make_unique<OSDRenderer>(renderer_);
    if (!osd_renderer_->initialize()) {
        std::cerr << "Warning: OSD renderer initialization failed" << std::endl;
    }
    
    // Load FPS display position from config (Requirement 19.3)
    fps_position_ = string_to_osd_position(config_manager_->get_fps_position());
    
    // If position is TopLeft (which overlaps with game graphics), move to BottomRight
    if (fps_position_ == OSDRenderer::OSDPosition::TopLeft) {
        fps_position_ = OSDRenderer::OSDPosition::BottomRight;
        config_manager_->set_fps_position("bottom-right");
        config_manager_->save();
    }
    
    // Load FPS display enabled state
    show_fps_ = config_manager_->get_fps_display_enabled();
    
    // Load audio mute state (Requirement 14.12)
    audio_muted_ = config_manager_->get_audio_muted();
    
    // Load input settings
    swap_joysticks_ = config_manager_->get_swap_joysticks();
    input_mapper_.load_from_config(*config_manager_);
    
    // Log input mappings
    std::cout << "Input mappings:" << std::endl;
    const char* action_names[] = {"Up", "Down", "Left", "Right", "Button"};
    for (int p = 0; p < 2; p++) {
        for (int a = 0; a < 5; a++) {
            SDL_Keycode key = input_mapper_.get_keyboard_mapping(p, static_cast<Action>(a));
            std::cout << "  Player " << (p+1) << " " << action_names[a] << ": " << SDL_GetKeyName(key) << std::endl;
        }
    }
    if (swap_joysticks_) {
        std::cout << "  Joystick ports swapped" << std::endl;
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
    
    // Apply saved mute state to audio device
    if (audio_device_ != 0 && audio_muted_) {
        SDL_PauseAudioDevice(audio_device_, 1);
    }
    
    // Create emulator
    Configuration emu_config;
    emu_config.video_standard = config_.video_standard;
    emu_config.enable_profile = config_.enable_profile;
    emulator_ = std::make_unique<EmulatorCore>(emu_config);
    
    // Set audio sample rate on VDC so it captures samples at the correct rate during run_frame()
    emulator_->get_vdc().set_audio_sample_rate(config_.sample_rate);
    // Create save state manager (needs emulator and renderer)
    save_state_manager_ = std::make_unique<SaveStateManagerUI>(emulator_.get(), renderer_);
    
    // Load BIOS
    if (!config_.bios_path.empty()) {
        auto result = emulator_->load_bios(config_.bios_path);
        if (result.is_err()) {
            std::cerr << "Failed to load BIOS: " << result.error << std::endl;
            shutdown();
            return false;
        }
        current_bios_name_ = config_.bios_path;
        std::cout << "Loaded BIOS: " << config_.bios_path << std::endl;
    } else if (config_manager_->get_auto_load_last_files()) {
        // Auto-load last BIOS if no command line arg provided
        std::string last_bios = config_manager_->get_last_bios_path();
        if (!last_bios.empty()) {
            std::string bios_path = extract_if_zip(last_bios);
            
            if (!bios_path.empty()) {
                auto result = emulator_->load_bios(bios_path);
                if (result.is_ok()) {
                    std::cout << "Auto-loaded last BIOS: " << last_bios << std::endl;
                    current_bios_name_ = last_bios;
                } else {
                    std::cerr << "Failed to auto-load last BIOS (" << last_bios << "): " << result.error << std::endl;
                    std::cout << "Use F10 to open menu and load BIOS manually" << std::endl;
                }
            } else {
                std::cerr << "Failed to extract BIOS from ZIP: " << last_bios << std::endl;
                std::cout << "Use F10 to open menu and load BIOS manually" << std::endl;
            }
        } else {
            std::cout << "No previous BIOS found. Use F10 to open menu and load BIOS" << std::endl;
        }
    } else {
        std::cout << "Auto-load disabled. Use F10 to open menu and load BIOS" << std::endl;
    }
    
    // Load ROM
    if (!config_.rom_path.empty()) {
        std::string rom_path = extract_if_zip(config_.rom_path);
        
        if (!rom_path.empty()) {
            auto result = emulator_->load_rom(rom_path);
            if (result.is_err()) {
                std::cerr << "Failed to load ROM: " << result.error << std::endl;
                shutdown();
                return false;
            }
            
            // Extract filename from path for save state tracking
            size_t last_slash = config_.rom_path.find_last_of("/\\");
            current_rom_name_ = (last_slash != std::string::npos) ? 
                config_.rom_path.substr(last_slash + 1) : config_.rom_path;
            std::cout << "Loaded ROM: " << config_.rom_path << std::endl;
        }
    } else if (config_manager_->get_auto_load_last_files()) {
        // Auto-load last ROM if no command line arg provided
        std::string last_rom = config_manager_->get_last_rom_path();
        if (!last_rom.empty()) {
            std::string rom_path = extract_if_zip(last_rom);
            
            if (!rom_path.empty()) {
                auto result = emulator_->load_rom(rom_path);
                if (result.is_ok()) {
                    std::cout << "Auto-loaded last ROM: " << last_rom << std::endl;
                    // Extract filename from path for save state tracking
                    size_t last_slash = last_rom.find_last_of("/\\");
                    current_rom_name_ = (last_slash != std::string::npos) ? 
                        last_rom.substr(last_slash + 1) : last_rom;
                } else {
                    std::cerr << "Failed to auto-load last ROM (" << last_rom << "): " << result.error << std::endl;
                    std::cout << "Use F10 to open menu and load ROM manually" << std::endl;
                }
            } else {
                std::cerr << "Failed to extract ROM from ZIP: " << last_rom << std::endl;
                std::cout << "Use F10 to open menu and load ROM manually" << std::endl;
            }
        } else {
            std::cout << "No previous ROM found. Use F10 to open menu and load ROM" << std::endl;
        }
    } else {
        std::cout << "Auto-load disabled. Use F10 to open menu and load ROM" << std::endl;
    }
    
    // Reset emulator
    emulator_->reset();
    
    // Initialize debugger if enabled
    if (config_.enable_debugger) {
        debugger_ = std::make_unique<Debugger>(emulator_.get());
        debugger_ui_ = std::make_unique<DebuggerUI>(debugger_.get());
        emulator_->set_debugger(debugger_.get());
        
        // Initialize ImGui debugger UI
        imgui_debugger_ui_ = std::make_unique<ImGuiDebuggerUI>(
            debugger_.get(), 
            emulator_.get(), 
            window_, 
            renderer_
        );
        if (!imgui_debugger_ui_->initialize()) {
            std::cerr << "Warning: ImGui debugger UI initialization failed" << std::endl;
            imgui_debugger_ui_.reset();  // Clean up on failure
        }
        
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
            debugger_->set_trace_limit(false);  // Disable trace limit for full capture
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
    // Guard against double shutdown
    if (shutdown_done_) return;
    shutdown_done_ = true;
    
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
                std::cout << "Writing trace log to trace_cpu.log..." << std::endl;
                std::ofstream trace_file("trace_cpu.log");
                if (!trace_file) {
                    std::cerr << "ERROR: Failed to open trace_cpu.log for writing!" << std::endl;
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
    // Native resolution rendering: VDC outputs 160x240, GPU handles scaling
    // No manual pixel doubling - SDL renderer scales to display
    int display_width = FRAMEBUFFER_WIDTH;   // 160 (native VDC width)
    int display_height = FRAMEBUFFER_HEIGHT;  // 240 (full VDC height)
    int window_width = display_width * 2 * config_.display_scale;  // Default 2x scale for visibility
    int window_height = display_height * config_.display_scale;
    
    // Store windowed dimensions
    windowed_width_ = window_width;
    windowed_height_ = window_height;
    
    // Create window
    uint32 window_flags = SDL_WINDOW_SHOWN;
    if (config_.fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        is_fullscreen_ = true;
    } else {
        is_fullscreen_ = false;
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
    
    // Store initial window position (for windowed mode)
    if (!is_fullscreen_) {
        SDL_GetWindowPosition(window_, &windowed_x_, &windowed_y_);
    }
    
    // Create renderer without VSync
    // VSync is not appropriate for emulators that need precise frame timing
    // (PAL 50Hz vs NTSC 60Hz) on monitors that typically run at 60Hz
    uint32 renderer_flags = SDL_RENDERER_ACCELERATED;
    
    renderer_ = SDL_CreateRenderer(window_, -1, renderer_flags);
    if (!renderer_) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Set scaling quality hint before texture creation
    // Default to nearest neighbor (sharp pixels) for retro aesthetic
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");  // 0=nearest (sharp), 1=linear (smooth)
    
    // No logical size - we'll use GPU scaling with calculated viewports
    // This allows flexible aspect ratio handling
    
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
    
    // Cache performance counter frequency for precise frame pacing
    perf_frequency_ = SDL_GetPerformanceFrequency();
    
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

void SDLFrontend::toggle_fullscreen() {
    // Requirements 12.1, 12.2, 12.3, 12.4, 12.5
    if (!window_) {
        return;
    }
    
    if (is_fullscreen_) {
        // Exit fullscreen - restore windowed mode
        // Requirement 12.5: Restore window size and position
        SDL_SetWindowFullscreen(window_, 0);
        SDL_SetWindowSize(window_, windowed_width_, windowed_height_);
        SDL_SetWindowPosition(window_, windowed_x_, windowed_y_);
        is_fullscreen_ = false;
    } else {
        // Enter fullscreen
        // Requirement 12.5: Store current window size and position
        SDL_GetWindowSize(window_, &windowed_width_, &windowed_height_);
        SDL_GetWindowPosition(window_, &windowed_x_, &windowed_y_);
        
        // Requirement 12.2: Use desktop resolution with fullscreen desktop mode
        // Requirement 12.3, 12.4: SDL_WINDOW_FULLSCREEN_DESKTOP maintains aspect ratio with letterboxing
        SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
        is_fullscreen_ = true;
    }
    
    // Requirement 12.6: Persist fullscreen preference to config
    if (config_manager_) {
        config_manager_->set_fullscreen(is_fullscreen_);
    }
}

void SDLFrontend::run() {
    try {
        // BUGFIX: Clear SDL event queue before starting main loop
        // SDL can generate spurious events during window initialization (focus, mouse enter, etc.)
        // or may have buffered keyboard events. Clear the queue to ensure clean start.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Discard all events
        }
        
        while (running_) {
            frame_start_counter_ = (frame_start_counter_ == 0) ? SDL_GetPerformanceCounter() : frame_start_counter_;
            
            // Check for scheduled key presses that should trigger this frame
            // Note: frame_count_ is the current frame, so check frame_count_ + 1 for next frame
            int next_frame = frame_count_ + 1;
            if (!active_keys_.empty() || !config_.scheduled_keys.empty()) {
                for (auto it = config_.scheduled_keys.begin(); it != config_.scheduled_keys.end(); ) {
                    if (next_frame >= it->trigger_frame) {
                        // Trigger the key press
                        InputHandler& input = emulator_->get_input_handler();
                        VidKey key = static_cast<VidKey>(it->key_code);
                        input.set_key_state(key, true);
                        active_keys_.push_back({key, it->duration_frames});
                        it = config_.scheduled_keys.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
            
            // Check for scheduled joystick presses
            for (auto it = scheduled_joystick_.begin(); it != scheduled_joystick_.end(); ) {
                if (next_frame >= it->trigger_frame) {
                    InputHandler& input = emulator_->get_input_handler();
                    if (it->direction == static_cast<Direction>(4)) {
                        input.set_joystick_button(it->joystick, true);
                    } else {
                        input.set_joystick_state(it->joystick, it->direction, true);
                    }
                    active_joystick_.push_back({it->joystick, it->direction, it->duration});
                    const char* dir_names[] = {"UP", "DOWN", "LEFT", "RIGHT", "FIRE"};
                    std::cout << "\n[Frame " << next_frame << "] Pressing joystick " << (it->joystick + 1)
                              << " " << dir_names[static_cast<int>(it->direction)]
                              << " for " << it->duration << " frames" << std::endl;
                    it = scheduled_joystick_.erase(it);
                } else {
                    ++it;
                }
            }
            
            // Update active joystick presses and release expired ones
            for (auto it = active_joystick_.begin(); it != active_joystick_.end(); ) {
                it->frames_remaining--;
                if (it->frames_remaining <= 0) {
                    InputHandler& input = emulator_->get_input_handler();
                    if (it->direction == static_cast<Direction>(4)) {
                        input.set_joystick_button(it->joystick, false);
                    } else {
                        input.set_joystick_state(it->joystick, it->direction, false);
                    }
                    const char* dir_names[] = {"UP", "DOWN", "LEFT", "RIGHT", "FIRE"};
                    std::cout << "[Frame " << next_frame << "] Releasing joystick " << (it->joystick + 1)
                              << " " << dir_names[static_cast<int>(it->direction)] << std::endl;
                    it = active_joystick_.erase(it);
                } else {
                    ++it;
                }
            }
            
            // Process input
            process_input();
            
            // Run emulator frame if not paused and menu is not active
            bool emulator_paused = debugger_ && debugger_->is_paused();
            bool menu_active = menu_system_ && menu_system_->is_visible();
            
            if (!paused_ && !emulator_paused && !menu_active) {
                emulator_->run_frame();
                frame_count_++;
                
                // Check frame limit
                if (frame_limit_ > 0 && frame_count_ >= frame_limit_) {
                    std::cout << "\nReached frame limit (" << frame_limit_ << " frames)" << std::endl;
                    running_ = false;
                }
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
                
                // Update window title with FPS
                char title[256];
                snprintf(title, sizeof(title), "Videopac Emulator - %.1f FPS", current_fps_);
                SDL_SetWindowTitle(window_, title);
            }
            
            // Precise frame pacing using performance counter
            // Skip pacing in turbo mode for maximum speed (Requirement 18.1)
            if (!turbo_mode_) {
                uint64_t target_ticks = perf_frequency_ / ((config_.video_standard == VideoStandard::PAL) ? 50 : 60);
                uint64_t frame_end = frame_start_counter_ + target_ticks;
                
                // Spin-wait for sub-millisecond precision
                while (SDL_GetPerformanceCounter() < frame_end) {
                    // Busy wait for precise timing
                }
            }
            frame_start_counter_ = SDL_GetPerformanceCounter();
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
    
    // Clear renderer with dark gray background for letterboxing
    // This provides contrast with the game's black background
    SDL_SetRenderDrawColor(renderer_, 32, 32, 32, 255);
    SDL_RenderClear(renderer_);
    
    // Disable logical size - we use GPU scaling with viewports
    SDL_RenderSetLogicalSize(renderer_, 0, 0);
    
    // === STEP 1: GAME RENDERING WITH GPU SCALING ===
    // Determine if we're in split screen mode
    bool split_screen = imgui_debugger_ui_ && imgui_debugger_ui_->is_visible() && 
                        imgui_debugger_ui_->get_display_mode() == DisplayMode::SplitScreen;
    
    SDL_Rect viewport;
    if (split_screen) {
        // Split Screen mode: render game to left half
        int window_width, window_height;
        SDL_GetRendererOutputSize(renderer_, &window_width, &window_height);
        
        // Game uses left half of window
        viewport = {0, 0, window_width / 2, window_height};
    } else {
        // Normal mode: use calculated viewport for aspect ratio handling
        viewport = calculate_viewport();
    }
    
    // Render the framebuffer texture to the viewport (GPU scaling)
    SDL_RenderCopy(renderer_, texture_, nullptr, &viewport);
    
    // Apply post-scaling effects to the viewport
    render_crt_effects(viewport);
    render_scanlines(viewport);
    
    // === STEP 3: RENDER ALL UI AT NATIVE RESOLUTION ===
    // Render unified status bar at bottom with FPS and all indicators
    if (osd_renderer_ && !menu_system_->is_visible()) {
        // Build status bar text with all info
        std::string status_bar;
        
        // Add FPS if enabled
        if (show_fps_) {
            char fps_text[32];
            snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", current_fps_);
            status_bar += fps_text;
        }
        
        // Add status indicators
        if (audio_muted_) {
            if (!status_bar.empty()) status_bar += " | ";
            status_bar += "MUTE";
        }
        if (turbo_mode_) {
            if (!status_bar.empty()) status_bar += " | ";
            status_bar += "TURBO";
        }
        if (paused_) {
            if (!status_bar.empty()) status_bar += " | ";
            status_bar += "PAUSED";
        }
        
        // Render the unified status bar at bottom
        if (!status_bar.empty()) {
            osd_renderer_->render_status_bar(status_bar);
        }
        
        // Update and render notifications
        osd_renderer_->update(SDL_GetTicks());
    }
    
    // Render menu overlay if visible
    if (menu_system_ && menu_system_->is_visible()) {
        menu_system_->render();
    }
    
    // Start and render ImGui frame if debugger UI exists
    if (imgui_debugger_ui_) {
        // CRITICAL: Set ImGui context BEFORE starting the frame
        ImGui::SetCurrentContext(imgui_debugger_ui_->get_context());
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        // Only render panels if visible
        if (imgui_debugger_ui_->is_visible()) {
            imgui_debugger_ui_->render();
        }
        
        // Always finish the ImGui frame if we started one
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    }
    
    // === STEP 4: PRESENT ===
    SDL_RenderPresent(renderer_);
}

OSDRenderer::OSDPosition SDLFrontend::string_to_osd_position(const std::string& position) const {
    if (position == "top-left") {
        return OSDRenderer::OSDPosition::TopLeft;
    } else if (position == "top-right") {
        return OSDRenderer::OSDPosition::TopRight;
    } else if (position == "bottom-left") {
        return OSDRenderer::OSDPosition::BottomLeft;
    } else if (position == "bottom-right") {
        return OSDRenderer::OSDPosition::BottomRight;
    }
    // Default to top-right if invalid
    return OSDRenderer::OSDPosition::TopRight;
}

SDL_Rect SDLFrontend::calculate_viewport() const {
    // Get window/renderer size
    int window_width, window_height;
    SDL_GetRendererOutputSize(renderer_, &window_width, &window_height);
    
    // Handle edge case: zero or negative window dimensions
    if (window_width <= 0 || window_height <= 0) {
        // Return default viewport matching framebuffer
        return {0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT};
    }
    
    // Original framebuffer dimensions
    const int fb_width = FRAMEBUFFER_WIDTH;   // 160
    const int fb_height = FRAMEBUFFER_HEIGHT; // 240
    
    SDL_Rect viewport;
    std::string aspect_ratio = config_manager_ ? config_manager_->get_aspect_ratio() : "4:3";
    
    if (aspect_ratio == "original") {
        // Original 2:3 aspect ratio (160:240 native VDC ratio)
        // Maintain the native pixel aspect ratio with letterboxing
        float target_aspect = static_cast<float>(fb_width) / fb_height;  // 160/240 = 2/3
        float window_aspect = static_cast<float>(window_width) / window_height;
        
        if (window_aspect > target_aspect) {
            // Window is wider than 2:3, fit to height
            viewport.h = window_height;
            viewport.w = static_cast<int>(window_height * target_aspect);
            viewport.x = (window_width - viewport.w) / 2;
            viewport.y = 0;
        } else {
            // Window is taller than 2:3, fit to width
            viewport.w = window_width;
            viewport.h = static_cast<int>(window_width / target_aspect);
            viewport.x = 0;
            viewport.y = (window_height - viewport.h) / 2;
        }
        
    } else if (aspect_ratio == "4:3") {
        // 4:3 aspect ratio (CRT television aspect ratio)
        // Horizontally stretch the image to achieve 4:3 display ratio
        float target_aspect = 4.0f / 3.0f;
        float window_aspect = static_cast<float>(window_width) / window_height;
        
        if (window_aspect > target_aspect) {
            // Window is wider than 4:3, fit to height
            viewport.h = window_height;
            viewport.w = static_cast<int>(window_height * target_aspect);
            viewport.x = (window_width - viewport.w) / 2;
            viewport.y = 0;
        } else {
            // Window is taller than 4:3, fit to width
            viewport.w = window_width;
            viewport.h = static_cast<int>(window_width / target_aspect);
            viewport.x = 0;
            viewport.y = (window_height - viewport.h) / 2;
        }
        
    } else { // "stretch"
        // Stretch to fill entire window (no aspect ratio preservation)
        viewport.x = 0;
        viewport.y = 0;
        viewport.w = window_width;
        viewport.h = window_height;
    }
    
    return viewport;
}

void SDLFrontend::render_crt_effects(const SDL_Rect& viewport) {
    // CRT effects disabled - edge vignetting was causing visual artifacts
    // Future implementation could add proper CRT simulation effects
    (void)viewport;  // Unused
    return;
}

void SDLFrontend::render_scanlines(const SDL_Rect& viewport) {
    if (!config_manager_) {
        return;
    }
    
    int scanline_percent = config_manager_->get_scanlines();
    
    if (scanline_percent == 0) {
        return;
    }
    
    // Calculate alpha based on percentage
    uint8_t alpha = static_cast<uint8_t>((scanline_percent * 255) / 100);
    
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, alpha);
    
    // Draw horizontal scanlines (every other line)
    // Scale scanline spacing based on viewport height
    int scanline_spacing = 2;  // Draw every 2 pixels
    
    for (int y = viewport.y; y < viewport.y + viewport.h; y += scanline_spacing) {
        SDL_RenderDrawLine(renderer_, viewport.x, y, viewport.x + viewport.w, y);
    }
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
            // Use Standard or Videopac+ palette based on palette mode
            const Color* palette = (config_.palette_mode == PaletteMode::STANDARD) ? PALETTE_STANDARD : PALETTE_VIDEOPAC_PLUS;
            Color color = palette[palette_index % 16];
            
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
    
    // Check for overflow: if more than 3 frames queued, skip ahead
    int max_queued = 3 * samples_per_frame;
    
    // Calculate queued samples
    size_t queued = (audio_write_pos_ >= audio_read_pos_) 
        ? (audio_write_pos_ - audio_read_pos_)
        : (audio_buffer_.size() - audio_read_pos_ + audio_write_pos_);
    
    if (queued > static_cast<size_t>(max_queued)) {
        // Skip ahead to 1 frame worth
        size_t target = static_cast<size_t>(samples_per_frame);
        audio_read_pos_ = (audio_write_pos_ >= target) 
            ? (audio_write_pos_ - target)
            : (audio_buffer_.size() - target + audio_write_pos_);
    }
    
    SDL_UnlockAudioDevice(audio_device_);
}

void SDLFrontend::process_input() {
    // Process scheduled key presses first
    if (!emulator_) {
        return;
    }
    
    InputHandler& input = emulator_->get_input_handler();
    
    // Update active keys and release expired ones
    for (auto it = active_keys_.begin(); it != active_keys_.end(); ) {
        it->frames_remaining--;
        
        if (it->frames_remaining <= 0) {
            // Release the key
            input.set_key_state(it->key, false);
            it = active_keys_.erase(it);
        } else {
            ++it;
        }
    }
    
    // If SDL input is disabled, skip SDL event processing
    if (disable_sdl_input_) {
        return;
    }
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Handle F9 (screenshot) BEFORE ImGui
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F9 && event.key.repeat == 0) {
            // Generate filename with timestamp
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::tm* tm = std::localtime(&time_t);
            char filename[64];
            std::strftime(filename, sizeof(filename), "screenshot_%Y%m%d_%H%M%S.ppm", tm);
            save_screenshot(filename);
            continue;
        }
        
        // Handle F12 (debugger toggle) BEFORE ImGui to prevent double-processing
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F12 && event.key.repeat == 0) {
            if (imgui_debugger_ui_) {
                if (imgui_debugger_ui_->is_visible()) {
                    imgui_debugger_ui_->hide();
                    if (debugger_) {
                        debugger_->continue_execution();
                    }
                } else {
                    if (debugger_) {
                        debugger_->pause();
                        // Force immediate pause by stepping once
                        debugger_->step();
                    }
                    imgui_debugger_ui_->show();
                    std::cout << "Debugger opened - PC: 0x" << std::hex << emulator_->get_cpu_state().pc << std::dec << std::endl;
                }
            }
            continue;  // Skip further processing of this event
        }
        
        // Forward events to ImGui debugger UI if visible
        if (imgui_debugger_ui_ && imgui_debugger_ui_->is_visible()) {
            // Set ImGui context before accessing ImGui functions
            ImGui::SetCurrentContext(imgui_debugger_ui_->get_context());
            
            imgui_debugger_ui_->process_event(event);
            
            // Check if ImGui wants to capture input
            ImGuiIO& io = ImGui::GetIO();
            bool imgui_wants_keyboard = io.WantCaptureKeyboard;
            bool imgui_wants_mouse = io.WantCaptureMouse;
            
            // Skip processing keyboard events if ImGui wants them
            if (imgui_wants_keyboard && (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)) {
                continue;
            }
            
            // Skip processing mouse events if ImGui wants them
            if (imgui_wants_mouse && (event.type == SDL_MOUSEBUTTONDOWN || 
                                      event.type == SDL_MOUSEBUTTONUP || 
                                      event.type == SDL_MOUSEMOTION || 
                                      event.type == SDL_MOUSEWHEEL)) {
                continue;
            }
        }
        
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
                // Update menu values before showing menu
                menu_system_->update_menu_values(config_manager_.get());
                // Update save state slot information before showing menu
                menu_system_->update_save_state_slots(save_state_manager_.get(), current_rom_name_);
                menu_system_->show();
            }
        }
        return;
    }
    
    // Handle joystick keys via InputMapper (both press and release)
    int player;
    Action joy_action;
    if (is_joystick_key(event.keysym.sym, player, joy_action)) {
        // Apply swap: player 0 maps to Videopac joystick index based on swap setting
        int joy_index = swap_joysticks_ ? (1 - player) : player;
        if (joy_action == Action::Button) {
            emulator_->get_input_handler().set_joystick_button(joy_index, key_down);
        } else {
            Direction dir;
            switch (joy_action) {
                case Action::Up:    dir = Direction::Up; break;
                case Action::Down:  dir = Direction::Down; break;
                case Action::Left:  dir = Direction::Left; break;
                case Action::Right: dir = Direction::Right; break;
                default: return;
            }
            emulator_->get_input_handler().set_joystick_state(joy_index, dir, key_down);
        }
        return;
    }
    
    // Check for special keys (only on key down)
    if (key_down) {
        switch (event.keysym.sym) {
            case SDLK_ESCAPE:
                // Exit application when menu is not active (Requirement 8.7)
                if (!menu_system_ || !menu_system_->is_visible()) {
                    running_ = false;
                }
                return;
                
            case SDLK_F3:
                // Toggle FPS display (Requirement 8.2)
                show_fps_ = !show_fps_;
                if (config_manager_) {
                    config_manager_->set_fps_display_enabled(show_fps_);
                }
                if (osd_renderer_) {
                    std::string message = show_fps_ ? "FPS Display: ON" : "FPS Display: OFF";
                    osd_renderer_->show_notification(message, 2000);
                }
                return;
                
            case SDLK_F4:
                // Toggle audio mute (Requirement 14.5)
                audio_muted_ = !audio_muted_;
                if (config_manager_) {
                    config_manager_->set_audio_muted(audio_muted_);
                }
                if (audio_device_ != 0) {
                    SDL_PauseAudioDevice(audio_device_, audio_muted_ ? 1 : 0);
                }
                if (osd_renderer_) {
                    std::string message = audio_muted_ ? "Audio: MUTED" : "Audio: UNMUTED";
                    osd_renderer_->show_notification(message, 2000);
                }
                return;
                
            case SDLK_F5:
                // Reset emulator when debugger not active (Requirement 8.3)
                if (debugger_ && debugger_->is_paused()) {
                    debugger_->continue_execution();
                    std::cout << "Continuing execution..." << std::endl;
                } else {
                    emulator_->reset();
                    if (osd_renderer_) {
                        osd_renderer_->show_notification("Emulator Reset", 2000);
                    }
                }
                return;
                
            case SDLK_F6:
                // Quick-save to slot 0 (Requirement 8.4)
                handle_save_state(0);
                return;
                
            case SDLK_F7:
                // Quick-load from slot 0 (Requirement 8.5)
                handle_load_state(0);
                return;
                
            case SDLK_F9:
                if (debugger_) {
                    debugger_->step();
                    debugger_ui_->display_cpu_state();
                    debugger_ui_->display_disassembly(2, 5);
                }
                return;
                
            case SDLK_F11:
                // Toggle fullscreen (Requirement 12.1)
                toggle_fullscreen();
                if (osd_renderer_) {
                    std::string message = is_fullscreen_ ? "Fullscreen Mode" : "Windowed Mode";
                    osd_renderer_->show_notification(message, 2000);
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
        }
        
        // Check for Alt+Enter (alternative fullscreen toggle) (Requirement 12.1)
        if (event.keysym.sym == SDLK_RETURN && (event.keysym.mod & KMOD_ALT)) {
            toggle_fullscreen();
            if (osd_renderer_) {
                std::string message = is_fullscreen_ ? "Fullscreen Mode" : "Windowed Mode";
                osd_renderer_->show_notification(message, 2000);
            }
            return;
        }
    }
    
    // Handle Tab key for turbo mode (both press and release) (Requirement 18.1)
    if (event.keysym.sym == SDLK_TAB) {
        turbo_mode_ = key_down;
        return;
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
            frontend->last_audio_sample_ = output[i];  // Track for underrun
            frontend->audio_read_pos_ = (frontend->audio_read_pos_ + 1) % frontend->audio_buffer_.size();
        } else {
            output[i] = frontend->last_audio_sample_;  // Hold last sample on underrun
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
    // Create screenshots directory if it doesn't exist
    std::filesystem::path screenshots_dir("screenshots");
    if (!std::filesystem::exists(screenshots_dir)) {
        std::filesystem::create_directory(screenshots_dir);
    }
    
    // Build full path with screenshots directory
    std::filesystem::path full_path = screenshots_dir / filename;
    
    dump_framebuffer(full_path.string());
    std::cout << "Screenshot saved to " << full_path.string() << std::endl;
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
            // Use Standard or Videopac+ palette based on palette mode
            const Color* palette = (config_.palette_mode == PaletteMode::STANDARD) ? PALETTE_STANDARD : PALETTE_VIDEOPAC_PLUS;
            Color color = palette[palette_index % 16];
            file.put(color.r);
            file.put(color.g);
            file.put(color.b);
        }
    }
}

bool SDLFrontend::is_joystick_key(SDL_Keycode key, int& player, Action& action) const {
    for (int p = 0; p < 2; ++p) {
        for (int a = 0; a < 5; ++a) {
            Action act = static_cast<Action>(a);
            if (input_mapper_.get_keyboard_mapping(p, act) == key) {
                player = p;
                action = act;
                return true;
            }
        }
    }
    return false;
}

VidKey SDLFrontend::map_sdl_key(SDL_Keycode key) {
    // Map SDL keys to Videopac keyboard matrix
    // Layout matches o2em vmachine.c key_map[6][8]
    switch (key) {
        // Row 0: Number keys 0-7
        case SDLK_0: return VidKey::Key0;
        case SDLK_1: return VidKey::Key1;
        case SDLK_2: return VidKey::Key2;
        case SDLK_3: return VidKey::Key3;
        case SDLK_4: return VidKey::Key4;
        case SDLK_5: return VidKey::Key5;
        case SDLK_6: return VidKey::Key6;
        case SDLK_7: return VidKey::Key7;
        
        // Row 1: 8, 9, SPACE, /, L, P
        case SDLK_8: return VidKey::Key8;
        case SDLK_9: return VidKey::Key9;
        // Note: SDLK_SPACE may be handled by joystick mapping and won't reach here
        // SDLK_l and SDLK_p may be intercepted by special key handlers
        case SDLK_l: return VidKey::KeyL;
        
        // Row 2: +, W, E, R, T, U, I, O
        case SDLK_w: return VidKey::KeyW;
        case SDLK_e: return VidKey::KeyE;
        case SDLK_r: return VidKey::KeyR;
        case SDLK_t: return VidKey::KeyT;
        case SDLK_u: return VidKey::KeyU;
        case SDLK_i: return VidKey::KeyI;
        case SDLK_o: return VidKey::KeyO;
        
        // Row 3: Q, S, D, F, G, H, J, K
        case SDLK_q: return VidKey::KeyQ;
        case SDLK_s: return VidKey::KeyS;
        case SDLK_d: return VidKey::KeyD;
        case SDLK_f: return VidKey::KeyF;
        case SDLK_g: return VidKey::KeyG;
        case SDLK_h: return VidKey::KeyH;
        case SDLK_j: return VidKey::KeyJ;
        case SDLK_k: return VidKey::KeyK;
        
        // Row 4: A, Z, X, C, V, B, M, .
        case SDLK_a: return VidKey::KeyA;
        case SDLK_z: return VidKey::KeyZ;
        case SDLK_x: return VidKey::KeyX;
        case SDLK_c: return VidKey::KeyC;
        case SDLK_v: return VidKey::KeyV;
        case SDLK_b: return VidKey::KeyB;
        case SDLK_m: return VidKey::KeyM;
        case SDLK_PERIOD: return VidKey::Period;
        
        // Row 5: -, *, /, =, Y, N, DEL/Clear, ENTER
        case SDLK_MINUS: return VidKey::Minus;
        case SDLK_KP_MULTIPLY: return VidKey::Multiply;
        case SDLK_SLASH: return VidKey::Divide;
        case SDLK_EQUALS: return VidKey::Equal;
        case SDLK_y: return VidKey::KeyY;
        case SDLK_n: return VidKey::KeyN;
        case SDLK_DELETE: return VidKey::Clear;
        case SDLK_BACKSPACE: return VidKey::Clear;
        case SDLK_RETURN: return VidKey::Enter;
        
        // Numpad plus for + key (row 2 col 0)
        case SDLK_KP_PLUS: return VidKey::Plus;
        
        default:
            return static_cast<VidKey>(0xFF);
    }
}

// Menu action handlers
void SDLFrontend::handle_menu_action(videopac::MenuAction action) {
    int slot = menu_system_->get_selected_slot();
    
    switch (action) {
        case MenuAction::LoadBIOS:
            handle_load_bios();
            break;
        case MenuAction::LoadROM:
            handle_load_rom();
            break;
        case MenuAction::SaveState:
            if (slot >= 0) {
                handle_save_state(slot);
            }
            break;
        case MenuAction::LoadState:
            if (slot >= 0) {
                handle_load_state(slot);
            }
            break;
        case MenuAction::Reset:
            handle_reset();
            break;
        case MenuAction::DisplayInfo:
            handle_display_info();
            break;
        case MenuAction::ToggleFullscreen:
            toggle_fullscreen();
            if (osd_renderer_) {
                std::string message = is_fullscreen_ ? "Fullscreen Mode" : "Windowed Mode";
                osd_renderer_->show_notification(message, 2000);
            }
            break;
        case MenuAction::Screenshot:
            {
                // Generate timestamp-based filename
                time_t now = time(nullptr);
                struct tm* timeinfo = localtime(&now);
                char filename[256];
                strftime(filename, sizeof(filename), "videopac_%Y%m%d_%H%M%S.ppm", timeinfo);
                save_screenshot(filename);
                if (osd_renderer_) {
                    osd_renderer_->show_notification("Screenshot Saved", 2000);
                }
            }
            break;
        case MenuAction::ToggleFPS:
            show_fps_ = !show_fps_;
            if (config_manager_) {
                config_manager_->set_fps_display_enabled(show_fps_);
                config_manager_->save();
            }
            break;
            
        case MenuAction::ToggleDebugger:
            if (imgui_debugger_ui_) {
                if (imgui_debugger_ui_->is_visible()) {
                    imgui_debugger_ui_->hide();
                    if (debugger_) {
                        debugger_->continue_execution();
                    }
                } else {
                    imgui_debugger_ui_->show();
                    if (debugger_) {
                        debugger_->pause();
                    }
                }
                menu_system_->hide();
            } else if (osd_renderer_) {
                osd_renderer_->show_notification("Debugger not available", 2000);
            }
            break;
            if (imgui_debugger_ui_) {
                imgui_debugger_ui_->show();
                if (debugger_) {
                    debugger_->pause();
                }
                menu_system_->hide();
            } else if (osd_renderer_) {
                osd_renderer_->show_notification("Debugger not available", 2000);
            }
            break;
        case MenuAction::Quit:
            handle_exit();
            break;
        // Video Settings
        case MenuAction::ScalingFilterNearest:
            config_manager_->set_scaling_filter("nearest");
            config_manager_->save();  // Persist setting
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
            if (osd_renderer_) {
                osd_renderer_->show_notification("Scaling: Nearest", 2000);
            }
            break;
        case MenuAction::ScalingFilterLinear:
            config_manager_->set_scaling_filter("linear");
            config_manager_->save();  // Persist setting
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
            if (osd_renderer_) {
                osd_renderer_->show_notification("Scaling: Linear", 2000);
            }
            break;
        case MenuAction::AspectRatioOriginal:
            config_manager_->set_aspect_ratio("original");
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("Aspect Ratio: Original", 2000);
            }
            break;
        case MenuAction::AspectRatio4_3:
            config_manager_->set_aspect_ratio("4:3");
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("Aspect Ratio: 4:3", 2000);
            }
            break;
        case MenuAction::AspectRatioStretch:
            config_manager_->set_aspect_ratio("stretch");
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("Aspect Ratio: Stretch", 2000);
            }
            break;
        case MenuAction::CRTEffectNone:
            config_manager_->set_crt_effect("none");
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("CRT Effect: None", 2000);
            }
            break;
        case MenuAction::CRTEffectLight:
            config_manager_->set_crt_effect("light");
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("CRT Effect: Light", 2000);
            }
            break;
        case MenuAction::CRTEffectMedium:
            config_manager_->set_crt_effect("medium");
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("CRT Effect: Medium", 2000);
            }
            break;
        case MenuAction::CRTEffectHeavy:
            config_manager_->set_crt_effect("heavy");
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("CRT Effect: Heavy", 2000);
            }
            break;
        case MenuAction::ScanlinesOff:
            config_manager_->set_scanlines(0);
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("Scanlines: Off", 2000);
            }
            break;
        case MenuAction::Scanlines25:
            config_manager_->set_scanlines(25);
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("Scanlines: 25%", 2000);
            }
            break;
        case MenuAction::Scanlines50:
            config_manager_->set_scanlines(50);
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("Scanlines: 50%", 2000);
            }
            break;
        case MenuAction::Scanlines75:
            config_manager_->set_scanlines(75);
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("Scanlines: 75%", 2000);
            }
            break;
        // Audio Settings
        case MenuAction::Volume0:
        case MenuAction::Volume10:
        case MenuAction::Volume20:
        case MenuAction::Volume30:
        case MenuAction::Volume40:
        case MenuAction::Volume50:
        case MenuAction::Volume60:
        case MenuAction::Volume70:
        case MenuAction::Volume80:
        case MenuAction::Volume90:
        case MenuAction::Volume100:
            {
                // Map action to volume value
                int volume = 0;
                switch (action) {
                    case MenuAction::Volume0: volume = 0; break;
                    case MenuAction::Volume10: volume = 10; break;
                    case MenuAction::Volume20: volume = 20; break;
                    case MenuAction::Volume30: volume = 30; break;
                    case MenuAction::Volume40: volume = 40; break;
                    case MenuAction::Volume50: volume = 50; break;
                    case MenuAction::Volume60: volume = 60; break;
                    case MenuAction::Volume70: volume = 70; break;
                    case MenuAction::Volume80: volume = 80; break;
                    case MenuAction::Volume90: volume = 90; break;
                    case MenuAction::Volume100: volume = 100; break;
                    default: volume = 70; break;
                }
                
                config_manager_->set_volume(volume);
                config_manager_->save();  // Persist setting
                
                // Update master volume in config
                config_.master_volume = volume / 100.0f;
                
                if (osd_renderer_) {
                    osd_renderer_->show_notification("Volume: " + std::to_string(volume) + "%", 2000);
                }
                
                // Update menu values to reflect change
                menu_system_->update_menu_values(config_manager_.get());
            }
            break;
        case MenuAction::ToggleMute:
            {
                bool current_mute = config_manager_->get_audio_muted();
                audio_muted_ = !current_mute;
                config_manager_->set_audio_muted(audio_muted_);
                config_manager_->save();  // Persist setting
                
                // Apply mute immediately
                if (audio_device_ != 0) {
                    SDL_PauseAudioDevice(audio_device_, audio_muted_ ? 1 : 0);
                }
                
                if (osd_renderer_) {
                    std::string message = audio_muted_ ? "Audio: MUTED" : "Audio: UNMUTED";
                    osd_renderer_->show_notification(message, 2000);
                }
                
                // Update menu values to reflect change
                menu_system_->update_menu_values(config_manager_.get());
            }
            break;
        case MenuAction::AudioBufferSmall:
            config_manager_->set_audio_buffer_size(512);
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("Buffer: Small (Restart Required)", 3000);
            }
            // Update menu values to reflect change
            menu_system_->update_menu_values(config_manager_.get());
            break;
        case MenuAction::AudioBufferMedium:
            config_manager_->set_audio_buffer_size(1024);
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("Buffer: Medium (Restart Required)", 3000);
            }
            // Update menu values to reflect change
            menu_system_->update_menu_values(config_manager_.get());
            break;
        case MenuAction::AudioBufferLarge:
            config_manager_->set_audio_buffer_size(2048);
            config_manager_->save();  // Persist setting
            if (osd_renderer_) {
                osd_renderer_->show_notification("Buffer: Large (Restart Required)", 3000);
            }
            // Update menu values to reflect change
            menu_system_->update_menu_values(config_manager_.get());
            break;
        case MenuAction::SwapJoysticks:
            swap_joysticks_ = !swap_joysticks_;
            if (config_manager_) {
                config_manager_->set_swap_joysticks(swap_joysticks_);
                config_manager_->save();
            }
            if (osd_renderer_) {
                std::string message = swap_joysticks_ ? "Joysticks: Swapped" : "Joysticks: Normal";
                osd_renderer_->show_notification(message, 2000);
            }
            menu_system_->update_menu_values(config_manager_.get());
            break;
        default:
            std::cout << "Unhandled menu action: " << static_cast<int>(action) << std::endl;
            break;
    }
}

void SDLFrontend::handle_load_bios() {
    // Hide menu temporarily
    menu_system_->hide();
    
    // Open file browser for BIOS files (including ZIP)
    file_browser_->open(".bin,.rom,.zip", FileBrowser::FileType::BIOS);
    
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
        std::string original_file_path = file_browser_->get_selected_file();
        std::string file_path = original_file_path;
        
        // Check if it's a ZIP file
        bool is_zip = (file_path.size() >= 4 && file_path.substr(file_path.size() - 4) == ".zip");
        if (is_zip) {
            file_path = handle_zip_file(file_path);
            if (file_path.empty()) {
                // ZIP handling failed or was cancelled
                menu_system_->show();
                return;
            }
        }
        
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
            // Save the original ZIP path, not the extracted temp file
            recent_bios_->add(original_file_path);
            config_manager_->set_last_bios_path(original_file_path);  // Save last BIOS path
            config_manager_->save();  // Save updated recent files and last path
            current_bios_name_ = original_file_path;
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
    
    // Don't show menu again - let emulator start running
    // User can press F10 to open menu if needed
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
        std::string original_file_path = file_browser_->get_selected_file();
        std::string file_path = original_file_path;
        
        // Check if it's a ZIP file
        bool is_zip = (file_path.size() >= 4 && file_path.substr(file_path.size() - 4) == ".zip");
        if (is_zip) {
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
            // Save the original ZIP path, not the extracted temp file
            recent_roms_->add(original_file_path);
            config_manager_->set_last_rom_path(original_file_path);
            config_manager_->save();  // Save updated recent files and last path
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
    
    // Don't show menu again - let emulator start running
    // User can press F10 to open menu if needed
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
    menu_system_->hide();
    
    // Build info message
    std::string rom_info = current_rom_name_.empty() ? "None" : current_rom_name_;
    std::string bios_info = current_bios_name_.empty() ? "None" : current_bios_name_;
    std::string version_info = "v" + std::string(VIDEOPAC_VERSION);
    
    std::string message = "ROM: " + rom_info + "\n" +
                         "BIOS: " + bios_info + "\n" +
                         "Version: " + version_info;
    
    message_dialog_->set_message("System Information", message);
    message_dialog_->show();
    
    // Wait for user to dismiss the dialog
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
        
        // Render the paused game screen with dialog overlay
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

void SDLFrontend::handle_exit() {
    // Save configuration before exit (Requirement 16.4)
    if (config_manager_) {
        std::cout << "Saving configuration..." << std::endl;
        if (!config_manager_->save()) {
            std::cerr << "Warning: Failed to save configuration" << std::endl;
        }
    }
    
    // Clean up temporary files (Requirement 3.7)
    if (zip_handler_) {
        std::cout << "Cleaning up temporary files..." << std::endl;
        zip_handler_->cleanup_temp_files();
    }
    
    // Terminate application (Requirement 4.12)
    std::cout << "Exiting application..." << std::endl;
    running_ = false;
}

void SDLFrontend::handle_save_state(int slot) {
    menu_system_->hide();
    
    // Use tracked ROM name, or "unknown" if no ROM loaded
    std::string rom_name = current_rom_name_.empty() ? "unknown" : current_rom_name_;
    
    // Show progress dialog
    progress_dialog_->set_message("Saving state...");
    progress_dialog_->show();
    
    // Render progress dialog
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    update_texture();
    SDL_Rect dest_rect = {0, 0, 320, 240};
    SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
    progress_dialog_->render();
    SDL_RenderPresent(renderer_);
    
    // Save the state
    auto result = save_state_manager_->save_state(slot, rom_name);
    
    progress_dialog_->hide();
    
    // Show result message
    if (result.is_ok()) {
        message_dialog_->set_message("Success", "State saved to slot " + std::to_string(slot));
    } else {
        message_dialog_->set_message("Error", "Failed to save state: " + result.error);
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
        if (result.is_ok() && (SDL_GetTicks() - message_start_time) >= 2000) {
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
    menu_system_->show();
}

void SDLFrontend::handle_load_state(int slot) {
    menu_system_->hide();
    
    // Use tracked ROM name, or "unknown" if no ROM loaded
    std::string rom_name = current_rom_name_.empty() ? "unknown" : current_rom_name_;
    
    // Show progress dialog
    progress_dialog_->set_message("Loading state...");
    progress_dialog_->show();
    
    // Render progress dialog
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    update_texture();
    SDL_Rect dest_rect = {0, 0, 320, 240};
    SDL_RenderCopy(renderer_, texture_, nullptr, &dest_rect);
    progress_dialog_->render();
    SDL_RenderPresent(renderer_);
    
    // Load the state
    auto result = save_state_manager_->load_state(slot, rom_name);
    
    progress_dialog_->hide();
    
    // Show result message
    if (result.is_ok()) {
        message_dialog_->set_message("Success", "State loaded from slot " + std::to_string(slot));
    } else {
        message_dialog_->set_message("Error", "Failed to load state: " + result.error);
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
        if (result.is_ok() && (SDL_GetTicks() - message_start_time) >= 2000) {
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
    menu_system_->show();
}

void SDLFrontend::handle_delete_state(int slot) {
    (void)slot;  // Unused for now
    menu_system_->hide();
    
    // Use tracked ROM name, or "unknown" if no ROM loaded
    std::string rom_name = current_rom_name_.empty() ? "unknown" : current_rom_name_;
    
    // Show confirmation dialog
    // TODO: Implement ConfirmDialog usage
    
    // For now, just show a message
    message_dialog_->set_message("Delete State", "Delete state feature not yet fully implemented");
    message_dialog_->show();
    
    // Wait for user to dismiss message
    bool message_dismissed = false;
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
    menu_system_->show();
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
    
    // Extract filename from path
    size_t last_slash = path.find_last_of("/\\");
    current_bios_name_ = (last_slash != std::string::npos) ? path.substr(last_slash + 1) : path;
    
    // Reset emulator after loading BIOS
    emulator_->reset();
    
    return true;
}

// Helper function to extract file from ZIP if needed
std::string SDLFrontend::extract_if_zip(const std::string& path) {
    // Check if it's a ZIP file
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".zip") {
        // Extract ZIP to temp file
        if (zip_handler_->open(path)) {
            auto rom_files = zip_handler_->get_rom_files();
            if (!rom_files.empty()) {
                std::string extracted = zip_handler_->extract_file(rom_files[0]);
                zip_handler_->close();
                return extracted;
            }
            zip_handler_->close();
        }
        return "";  // ZIP extraction failed
    }
    return path;  // Not a ZIP, return original path
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
    
    // Extract filename from path
    size_t last_slash = path.find_last_of("/\\");
    current_rom_name_ = (last_slash != std::string::npos) ? path.substr(last_slash + 1) : path;
    
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

void SDLFrontend::schedule_key_press(VidKey key, int trigger_frame, int duration_frames) {
    ScheduledKey sk;
    sk.key_code = static_cast<int>(key);
    sk.trigger_frame = trigger_frame;
    sk.duration_frames = duration_frames;
    config_.scheduled_keys.push_back(sk);
    std::cout << "Scheduled key " << sk.key_code 
              << " to be pressed at frame " << trigger_frame 
              << " for " << duration_frames << " frames" << std::endl;
}

void SDLFrontend::schedule_joystick_press(int joystick, Direction direction, int trigger_frame, int duration_frames) {
    scheduled_joystick_.push_back({joystick, direction, trigger_frame, duration_frames});
    const char* dir_names[] = {"UP", "DOWN", "LEFT", "RIGHT", "FIRE"};
    std::cout << "Scheduled joystick " << (joystick + 1) << " " << dir_names[static_cast<int>(direction)]
              << " to be pressed at frame " << trigger_frame
              << " for " << duration_frames << " frames" << std::endl;
}

} // namespace videopac
