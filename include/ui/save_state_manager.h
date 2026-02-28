#ifndef UI_SAVE_STATE_MANAGER_H
#define UI_SAVE_STATE_MANAGER_H

#include "types.h"
#include <string>
#include <vector>
#include <ctime>
#include <SDL.h>

namespace videopac {
    class EmulatorCore;
}

// Save state information for UI display
struct SaveStateInfo {
    int slot;
    std::string filename;
    std::string thumbnail_path;
    std::time_t timestamp;
    bool exists;
};

// UI-level save state manager with thumbnail support
// Wraps the core SaveStateManager and adds UI features like thumbnails
class SaveStateManagerUI {
public:
    SaveStateManagerUI(videopac::EmulatorCore* emulator, SDL_Renderer* renderer);
    ~SaveStateManagerUI();

    // Save state operations
    videopac::Result<void> save_state(int slot, const std::string& rom_name);
    videopac::Result<void> load_state(int slot, const std::string& rom_name);
    videopac::Result<void> delete_state(int slot, const std::string& rom_name);

    // Query save states
    std::vector<SaveStateInfo> list_states(const std::string& rom_name);

    // Get saves directory path
    std::string get_saves_directory() const;

private:
    // Helper methods
    std::string get_state_filename(const std::string& rom_name, int slot);
    std::string get_thumbnail_filename(const std::string& rom_name, int slot);
    
    videopac::Result<void> capture_thumbnail(const std::string& filename);
    bool ensure_saves_directory();

    // Members
    videopac::EmulatorCore* emulator_;
    SDL_Renderer* renderer_;
    std::string saves_dir_;
};

#endif // UI_SAVE_STATE_MANAGER_H
