#include "emulator.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Videopac Emulator v1.0.0" << std::endl;
    
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <rom_file>" << std::endl;
        return 1;
    }
    
    // TODO: Implement main loop with SDL2 frontend
    // This will be implemented in task 14
    
    std::cout << "Frontend not yet implemented" << std::endl;
    return 0;
}
