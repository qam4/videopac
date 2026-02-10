#include "frontend_headless.h"
#ifdef ENABLE_SDL
#include "frontend_sdl.h"
#endif
#include <iostream>
#include <cstring>

using namespace videopac;

void print_usage(const char* program_name) {
    std::cout << "Videopac Emulator v1.0.0" << std::endl;
    std::cout << "Usage: " << program_name << " [options] <rom_file>" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  --bios <file>       Load BIOS from file" << std::endl;
    std::cout << "  --pal               Use PAL timing (default: NTSC)" << std::endl;
    std::cout << "  --headless          Run without display (for testing)" << std::endl;
    std::cout << "  --frames <n>        Run for N frames then exit (headless mode)" << std::endl;
    std::cout << "  --screenshot <n>    Save screenshot every N frames (headless mode)" << std::endl;
    std::cout << "  --debug             Enable debugger" << std::endl;
    std::cout << "  --break <addr>      Set breakpoint at address (hex, e.g. 0x00B0)" << std::endl;
    std::cout << "  --help              Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Parse command line arguments
    FrontendConfig config;
    std::string rom_path;
    bool force_headless = false;
    int frame_limit = 0;
    int screenshot_interval = 0;
    std::vector<uint16> breakpoints;
    
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
        } else if (strcmp(argv[i], "--debug") == 0) {
            config.enable_debugger = true;
        } else if (strcmp(argv[i], "--break") == 0 && i + 1 < argc) {
            uint16 addr = static_cast<uint16>(std::strtol(argv[++i], nullptr, 16));
            breakpoints.push_back(addr);
            config.enable_debugger = true;  // Auto-enable debugger if breakpoints are set
        } else if (argv[i][0] != '-') {
            rom_path = argv[i];
        } else {
            std::cerr << "Unknown option: " << argv[i] << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (rom_path.empty()) {
        std::cerr << "Error: No ROM file specified" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    
    config.rom_path = rom_path;
    config.breakpoints = breakpoints;
    
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

