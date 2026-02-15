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
    std::cout << "  --pal               Use PAL timing (default: NTSC)" << std::endl;
    std::cout << "  --headless          Run without display (for testing)" << std::endl;
    std::cout << "  --frames <n>        Run for N frames then exit (headless mode)" << std::endl;
    std::cout << "  --screenshot <n>    Save screenshot every N frames (headless mode)" << std::endl;
    std::cout << "  --extended-fb       Enable extended framebuffer mode (240x250, shows area beyond visible 160x200)" << std::endl;
    std::cout << "  --press-key <key> <frame>" << std::endl;
    std::cout << "                      Press key at frame N (headless mode)" << std::endl;
    std::cout << "                      Example: --press-key 1 60 (press '1' at frame 60)" << std::endl;
    std::cout << "  --debug             Enable debugger" << std::endl;
    std::cout << "  --trace             Enable instruction trace logging (very slow!)" << std::endl;
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
    bool enable_trace = false;
    bool enable_profile = false;
    std::vector<BreakpointConfig> breakpoints;
    std::vector<std::string> watch_conditions;  // Condition-only breakpoints
    
    struct ScheduledKeyPress {
        int key_code;
        int frame;
    };
    std::vector<ScheduledKeyPress> scheduled_keys;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--bios") == 0 && i + 1 < argc) {
            config.bios_path = argv[++i];
        } else if (strcmp(argv[i], "--pal") == 0) {
            config.video_standard = VideoStandard::PAL;
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
            force_headless = true;  // Key press implies headless
        } else if (strcmp(argv[i], "--debug") == 0) {
            config.enable_debugger = true;
        } else if (strcmp(argv[i], "--trace") == 0) {
            enable_trace = true;
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
    config.enable_trace = enable_trace;
    config.enable_profile = enable_profile;
    config.breakpoints = breakpoints;
    config.watch_conditions = watch_conditions;
    
    // Determine which frontend to use
#ifdef ENABLE_SDL
    if (!force_headless) {
        SDLFrontend frontend;
        
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

