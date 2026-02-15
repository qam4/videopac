#include "ui/save_state_manager.h"
#include "emulator.h"
#include "savestate.h"
#include "utils.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>

// Disable warnings for third-party header
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

// stb_image_write for PNG thumbnail saving
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../third_party/stb_image_write.h"

#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace fs = std::filesystem;

SaveStateManagerUI::SaveStateManagerUI(videopac::EmulatorCore* emulator, SDL_Renderer* renderer)
    : emulator_(emulator), renderer_(renderer) {
    
    // Get config directory using SDL
    char* pref_path = SDL_GetPrefPath("videopac", "emulator");
    if (pref_path) {
        saves_dir_ = std::string(pref_path) + "saves";
        SDL_free(pref_path);
    } else {
        // Fallback to current directory
        saves_dir_ = "saves";
    }
    
    // Ensure saves directory exists
    ensure_saves_directory();
}

SaveStateManagerUI::~SaveStateManagerUI() {
    // Nothing to clean up
}

videopac::Result<void> SaveStateManagerUI::save_state(int slot, const std::string& rom_name) {
    if (!emulator_) {
        return videopac::Result<void>::err("No emulator instance");
    }
    
    // Ensure saves directory exists
    if (!ensure_saves_directory()) {
        return videopac::Result<void>::err("Failed to create saves directory");
    }
    
    // Get filenames
    std::string state_file = get_state_filename(rom_name, slot);
    std::string thumbnail_file = get_thumbnail_filename(rom_name, slot);
    
    // Save the emulator state using the core's save_state method
    auto result = emulator_->save_state(state_file);
    if (!result.is_ok()) {
        return result;
    }
    
    // Capture and save thumbnail
    auto thumb_result = capture_thumbnail(thumbnail_file);
    if (!thumb_result.is_ok()) {
        // Thumbnail failure is not critical, just log it
        // The save state itself succeeded
    }
    
    return videopac::Result<void>::ok();
}

videopac::Result<void> SaveStateManagerUI::load_state(int slot, const std::string& rom_name) {
    if (!emulator_) {
        return videopac::Result<void>::err("No emulator instance");
    }
    
    // Get filename
    std::string state_file = get_state_filename(rom_name, slot);
    
    // Check if file exists
    if (!fs::exists(state_file)) {
        return videopac::Result<void>::err("Save state does not exist");
    }
    
    // Load the emulator state using the core's load_state method
    return emulator_->load_state(state_file);
}

videopac::Result<void> SaveStateManagerUI::delete_state(int slot, const std::string& rom_name) {
    // Get filenames
    std::string state_file = get_state_filename(rom_name, slot);
    std::string thumbnail_file = get_thumbnail_filename(rom_name, slot);
    
    bool deleted_any = false;
    
    // Delete state file if it exists
    if (fs::exists(state_file)) {
        try {
            fs::remove(state_file);
            deleted_any = true;
        } catch (const fs::filesystem_error& e) {
            return videopac::Result<void>::err("Failed to delete state file: " + std::string(e.what()));
        }
    }
    
    // Delete thumbnail file if it exists
    if (fs::exists(thumbnail_file)) {
        try {
            fs::remove(thumbnail_file);
        } catch (const fs::filesystem_error&) {
            // Thumbnail deletion failure is not critical
        }
    }
    
    if (!deleted_any) {
        return videopac::Result<void>::err("Save state does not exist");
    }
    
    return videopac::Result<void>::ok();
}

std::vector<SaveStateInfo> SaveStateManagerUI::list_states(const std::string& rom_name) {
    std::vector<SaveStateInfo> states;
    
    // Check slots 0-9
    for (int slot = 0; slot <= 9; ++slot) {
        SaveStateInfo info;
        info.slot = slot;
        info.filename = get_state_filename(rom_name, slot);
        info.thumbnail_path = get_thumbnail_filename(rom_name, slot);
        
        // Check if state file exists
        if (fs::exists(info.filename)) {
            info.exists = true;
            
            // Get file modification time
            try {
                auto ftime = fs::last_write_time(info.filename);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                info.timestamp = std::chrono::system_clock::to_time_t(sctp);
            } catch (const fs::filesystem_error&) {
                info.timestamp = 0;
            }
        } else {
            info.exists = false;
            info.timestamp = 0;
        }
        
        states.push_back(info);
    }
    
    return states;
}

std::string SaveStateManagerUI::get_saves_directory() const {
    return saves_dir_;
}

std::string SaveStateManagerUI::get_state_filename(const std::string& rom_name, int slot) {
    // Extract base name from rom_name (remove path and extension)
    std::string base_name = rom_name;
    
    // Remove path
    size_t last_slash = base_name.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        base_name = base_name.substr(last_slash + 1);
    }
    
    // Remove extension
    size_t last_dot = base_name.find_last_of('.');
    if (last_dot != std::string::npos) {
        base_name = base_name.substr(0, last_dot);
    }
    
    // Build filename: {saves_dir}/{base_name}.state{slot}
    std::ostringstream oss;
    oss << saves_dir_ << "/" << base_name << ".state" << slot;
    return oss.str();
}

std::string SaveStateManagerUI::get_thumbnail_filename(const std::string& rom_name, int slot) {
    // Same as state filename but with .png extension
    return get_state_filename(rom_name, slot) + ".png";
}

videopac::Result<void> SaveStateManagerUI::capture_thumbnail(const std::string& filename) {
    if (!emulator_ || !renderer_) {
        return videopac::Result<void>::err("No emulator or renderer instance");
    }
    
    // Get the current framebuffer from the emulator
    const videopac::uint8* framebuffer = nullptr;
    try {
        framebuffer = emulator_->get_framebuffer();
    } catch (...) {
        return videopac::Result<void>::err("Exception getting framebuffer");
    }
    
    if (!framebuffer) {
        return videopac::Result<void>::err("Failed to get framebuffer");
    }
    
    // Videopac framebuffer is 160x200 pixels with palette indices (0-7)
    const int src_width = 160;
    const int src_height = 200;
    const int thumb_width = 160;  // Keep original width
    const int thumb_height = 120; // Scale down height
    
    // Videopac palette (bright colors)
    const videopac::uint8 palette[8][3] = {
        {0, 0, 0},       // 0: Black
        {0, 0, 255},     // 1: Blue
        {0, 255, 0},     // 2: Green
        {0, 255, 255},   // 3: Cyan
        {255, 0, 0},     // 4: Red
        {255, 0, 255},   // 5: Magenta
        {255, 255, 0},   // 6: Yellow
        {255, 255, 255}  // 7: White
    };
    
    // Allocate buffer for thumbnail (RGB24 format)
    std::vector<videopac::uint8> thumbnail(thumb_width * thumb_height * 3);
    
    // Scale down height (200 -> 120) while keeping width (160)
    // Use simple nearest-neighbor sampling
    try {
        for (int y = 0; y < thumb_height; ++y) {
            for (int x = 0; x < thumb_width; ++x) {
                // Map thumbnail coordinates to source coordinates
                int src_x = x;  // Width stays the same
                int src_y = (y * src_height) / thumb_height;  // Scale height
                
                // Bounds check
                if (src_x >= src_width || src_y >= src_height) {
                    continue;
                }
                
                // Get palette index from framebuffer
                int src_idx = src_y * src_width + src_x;
                videopac::uint8 palette_index = framebuffer[src_idx] % 8;
                
                // Convert to RGB using palette
                int dst_idx = (y * thumb_width + x) * 3;
                thumbnail[dst_idx + 0] = palette[palette_index][0];  // R
                thumbnail[dst_idx + 1] = palette[palette_index][1];  // G
                thumbnail[dst_idx + 2] = palette[palette_index][2];  // B
            }
        }
    } catch (...) {
        return videopac::Result<void>::err("Exception during thumbnail capture");
    }
    
    // Save as PNG using stb_image_write
    int result = 0;
    try {
        result = stbi_write_png(filename.c_str(), thumb_width, thumb_height, 3, 
                                     thumbnail.data(), thumb_width * 3);
    } catch (...) {
        return videopac::Result<void>::err("Exception writing thumbnail PNG");
    }
    
    if (result == 0) {
        return videopac::Result<void>::err("Failed to write thumbnail PNG");
    }
    
    return videopac::Result<void>::ok();
}

bool SaveStateManagerUI::ensure_saves_directory() {
    try {
        if (!fs::exists(saves_dir_)) {
            fs::create_directories(saves_dir_);
        }
        return true;
    } catch (const fs::filesystem_error&) {
        return false;
    }
}
