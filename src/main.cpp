#include "frontend_headless.h"
#ifdef ENABLE_SDL
#include "frontend_sdl.h"
#endif
#include <iostream>
#include <cstring>

using namespace videopac;

void print_usage(const char* program_name) {
    std::cout << "Videopac Emulator v1.0.0" << std::endl;
    std::cout << "Usage: " << program_name << " [options] [rom_file]" << std::endl;
    std::cout << "\nArguments:" << std::endl;
    std::cout << "  rom_file            ROM file to load (optional)" << std::endl;
    std::cout << "\nIf no ROM file is specified, the emulator will:" << std::endl;
    std::cout << "  1. Auto-load the last used BIOS and ROM (if available)" << std::endl;
    std::cout << "  2. Otherwise, start with the menu (press F10)" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  --bios <file>       Load BIOS from file (optional)" << std::endl;
    std::cout << "  --region <name>     Hardware region: usa, europe, france (default: usa)" << std::endl;
    std::cout << "  --headless          Run without display (for testing)" << std::endl;
    std::cout << "  --frames <n>        Run for N frames then exit (headless mode)" << std::endl;
    std::cout << "  --screenshot <n>    Save screenshot every N frames (headless mode)" << std::endl;
    std::cout << "  --extended-fb       Enable extended framebuffer mode (240x250, shows area beyond visible 160x200)" << std::endl;
    std::cout << "  --press-key <key> <frame>" << std::endl;
    std::cout << "                      Press key at frame N (headless mode)" << std::endl;
    std::cout << "                      Example: --press-key 1 60 (press '1' at frame 60)" << std::endl;
    std::cout << "  --press-joystick <joy> <dir> <frame> [duration]" << std::endl;
    std::cout << "                      Press joystick direction at frame N for D frames (headless mode)" << std::endl;
    std::cout << "                      joy: 1 or 2, dir: 0=up 1=down 2=left 3=right 4=fire" << std::endl;
    std::cout << "                      duration: frames to hold (default: 5)" << std::endl;
    std::cout << "                      Example: --press-joystick 2 0 60 100 (joy2 up at frame 60 for 100 frames)" << std::endl;
    std::cout << "  --debug             Enable debugger" << std::endl;
    std::cout << "  --trace[=level]     Enable instruction trace logging" << std::endl;
    std::cout << "                      Levels: minimal, normal, full (default: full)" << std::endl;
    std::cout << "                      minimal = PC + instruction + A register only (fast)" << std::endl;
    std::cout << "                      normal  = + PSW, ports, memory ops (medium)" << std::endl;
    std::cout << "                      full    = all registers, VDC state (slow)" << std::endl;
    std::cout << "  --profile           Enable performance profiling" << std::endl;
    std::cout << "  --break <addr>      Set breakpoint at address (hex, e.g. 0x00B0)" << std::endl;
    std::cout << "  --condition <expr>  Add condition to previous breakpoint" << std::endl;
    std::cout << "                      Examples: \"cpu.A == 0xFF\", \"vdc.registers[0xA0] & 0x20\"" << std::endl;
    std::cout << "  --watch <expr>      Set condition-only breakpoint (no address)" << std::endl;
    std::cout << "                      Examples: \"memory.external_ram[0x7F]==0xF8\"" << std::endl;
    std::cout << "  --help              Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    // Allow running without arguments - will use menu to load files
    if (argc >= 2 && strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    
    // Parse command line arguments
    FrontendConfig config;
    std::string rom_path;
    bool force_headless = false;
    int frame_limit = 0;
    int screenshot_interval = 0;
    bool extended_framebuffer = false;
    std::string trace_level_str = "";  // empty = off, "minimal", "normal", "full"
    bool enable_profile = false;
    std::vector<BreakpointConfig> breakpoints;
    std::vector<std::string> watch_conditions;  // Condition-only breakpoints
    
    struct ScheduledKeyPress {
        int key_code;
        int frame;
    };
    std::vector<ScheduledKeyPress> scheduled_keys;
    
    struct ScheduledJoystickPress {
        int joystick;  // 0 or 1 (for joy1 or joy2)
        int direction; // 0=up, 1=down, 2=left, 3=right, 4=fire
        int frame;
        int duration;  // frames to hold (default: 5)
    };
    std::vector<ScheduledJoystickPress> scheduled_joystick;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--bios") == 0 && i + 1 < argc) {
            config.bios_path = argv[++i];
        } else if (strcmp(argv[i], "--region") == 0 && i + 1 < argc) {
            std::string region = argv[++i];
            if (region == "usa") {
                // USA: NTSC timing (60Hz) + Standard O2 palette - Odyssey 2
                config.video_standard = VideoStandard::NTSC;
                config.palette_mode = PaletteMode::STANDARD;
            } else if (region == "europe") {
                // Europe: PAL timing (50Hz) + Standard O2 palette - Videopac G7000
                config.video_standard = VideoStandard::PAL;
                config.palette_mode = PaletteMode::STANDARD;
            } else if (region == "france") {
                // France: PAL timing (50Hz) + Standard O2 palette - C52 SECAM
                config.video_standard = VideoStandard::PAL;
                config.palette_mode = PaletteMode::STANDARD;
            } else {
                std::cerr << "Unknown region: " << region << " (valid: usa, europe, france)" << std::endl;
                return 1;
            }
        } else if (strcmp(argv[i], "--headless") == 0) {
            force_headless = true;
        } else if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            frame_limit = std::atoi(argv[++i]);
            force_headless = true;  // Frame limit implies headless
        } else if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
            screenshot_interval = std::atoi(argv[++i]);
        } else if (strcmp(argv[i], "--extended-fb") == 0) {
            extended_framebuffer = true;
        } else if (strcmp(argv[i], "--press-key") == 0 && i + 2 < argc) {
            int key_code = std::atoi(argv[++i]);
            int frame = std::atoi(argv[++i]);
            scheduled_keys.push_back({key_code, frame});
            // Don't force headless - allow SDL mode with programmatic keys for testing
        } else if (strcmp(argv[i], "--press-joystick") == 0 && i + 3 < argc) {
            int joystick = std::atoi(argv[++i]) - 1;  // Convert 1-based to 0-based
            int direction = std::atoi(argv[++i]);
            int frame = std::atoi(argv[++i]);
            int duration = 5;  // Default duration
            // Check if duration is provided (optional 4th argument)
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                duration = std::atoi(argv[++i]);
            }
            if (joystick < 0 || joystick > 1) {
                std::cerr << "Error: joystick must be 1 or 2" << std::endl;
                return 1;
            }
            if (direction < 0 || direction > 4) {
                std::cerr << "Error: direction must be 0-4 (0=up, 1=down, 2=left, 3=right, 4=fire)" << std::endl;
                return 1;
            }
            scheduled_joystick.push_back({joystick, direction, frame, duration});
        } else if (strcmp(argv[i], "--debug") == 0) {
            config.enable_debugger = true;
        } else if (strncmp(argv[i], "--trace", 7) == 0) {
            // Parse --trace or --trace=level
            if (argv[i][7] == '=') {
                trace_level_str = &argv[i][8];  // Get level after '='
            } else if (argv[i][7] == '\0') {
                trace_level_str = "full";  // Default to full if no level specified
            }
            config.enable_debugger = true;  // Trace requires debugger
        } else if (strcmp(argv[i], "--profile") == 0) {
            enable_profile = true;
        } else if (strcmp(argv[i], "--break") == 0 && i + 1 < argc) {
            uint16 addr = static_cast<uint16>(std::strtol(argv[++i], nullptr, 16));
            breakpoints.emplace_back(addr);
            config.enable_debugger = true;  // Auto-enable debugger if breakpoints are set
        } else if (strcmp(argv[i], "--condition") == 0 && i + 1 < argc) {
            if (breakpoints.empty()) {
                std::cerr << "Error: --condition must follow --break" << std::endl;
                return 1;
            }
            // Add condition to the last breakpoint
            breakpoints.back().condition = argv[++i];
        } else if (strcmp(argv[i], "--watch") == 0 && i + 1 < argc) {
            // Add condition-only breakpoint
            watch_conditions.push_back(argv[++i]);
            config.enable_debugger = true;  // Auto-enable debugger
        } else if (argv[i][0] != '-') {
            rom_path = argv[i];
        } else {
            std::cerr << "Unknown option: " << argv[i] << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // ROM path is optional - if not provided, emulator will start with menu
    // or auto-load last files if configured
    if (!rom_path.empty()) {
        config.rom_path = rom_path;
    }
    config.trace_level = trace_level_str;
    config.enable_profile = enable_profile;
    config.breakpoints = breakpoints;
    config.watch_conditions = watch_conditions;
    
    // Convert scheduled_keys to ScheduledKey format
    for (const auto& sk : scheduled_keys) {
        config.scheduled_keys.push_back(ScheduledKey(sk.key_code, sk.frame, 5));
    }
    
    // Determine which frontend to use
#ifdef ENABLE_SDL
    if (!force_headless) {
        SDLFrontend frontend;
        
        // If there are scheduled keys, disable SDL input to avoid interference
        if (!scheduled_keys.empty()) {
            frontend.set_disable_sdl_input(true);
            std::cout << "SDL input disabled - using scheduled key presses" << std::endl;
        }
        
        if (!frontend.initialize(config)) {
            std::cerr << "Failed to initialize SDL frontend" << std::endl;
            return 1;
        }
        
        frontend.run();
        frontend.shutdown();
        return 0;
    }
#else
    force_headless = true;
#endif
    
    // Create and run frontend
    if (force_headless) {
        config.headless = true;
        HeadlessFrontend frontend;
        
        if (frame_limit > 0) {
            frontend.set_frame_limit(frame_limit);
        }
        
        if (screenshot_interval > 0) {
            frontend.set_auto_screenshot(true, screenshot_interval);
        }
        
        if (!frontend.initialize(config)) {
            std::cerr << "Failed to initialize frontend" << std::endl;
            return 1;
        }
        
        if (extended_framebuffer) {
            frontend.set_extended_framebuffer_mode(true);
        }
        
        // Schedule key presses
        for (const auto& key_press : scheduled_keys) {
            VidKey key = static_cast<VidKey>(key_press.key_code);
            frontend.schedule_key_press(key, key_press.frame, 5);  // Press for 5 frames to ensure detection
        }
        
        // Schedule joystick presses
        for (const auto& joy_press : scheduled_joystick) {
            Direction dir = static_cast<Direction>(joy_press.direction);
            frontend.schedule_joystick_press(joy_press.joystick, dir, joy_press.frame, joy_press.duration);
        }
        
        frontend.run();
        frontend.shutdown();
    }
#ifdef ENABLE_SDL
    else {
        // SDL frontend will be implemented here
        std::cerr << "SDL frontend not yet implemented" << std::endl;
        return 1;
    }
#endif
    
    return 0;
}

