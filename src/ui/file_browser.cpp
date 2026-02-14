#include "ui/file_browser.h"
#include "ui/text_renderer.h"
#include "ui/config_manager.h"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

FileBrowser::FileBrowser(SDL_Renderer* renderer, TextRenderer* text_renderer, ConfigManager* config)
    : renderer_(renderer)
    , text_renderer_(text_renderer)
    , config_(config)
    , selected_index_(0)
    , scroll_offset_(0)
    , visible_lines_(20)
    , is_open_(false)
    , file_selected_(false)
    , current_file_type_(FileType::Generic) {
}

FileBrowser::~FileBrowser() {
}

void FileBrowser::open(const std::string& extensions, FileType file_type) {
    extensions_filter_ = extensions;
    file_selected_ = false;
    selected_file_.clear();
    selected_index_ = 0;
    scroll_offset_ = 0;
    current_file_type_ = file_type;

    // Get last used directory from config based on file type
    if (file_type == FileType::BIOS) {
        current_directory_ = config_->get_last_bios_directory();
    } else if (file_type == FileType::ROM) {
        current_directory_ = config_->get_last_rom_directory();
    } else {
        // Generic: try ROM directory first, then current directory
        current_directory_ = config_->get_last_rom_directory();
    }
    
    // Verify directory exists, fall back to current directory if not
    if (!fs::exists(current_directory_) || !fs::is_directory(current_directory_)) {
        current_directory_ = fs::current_path().string();
    }

    scan_directory();
    is_open_ = true;
}

void FileBrowser::close() {
    is_open_ = false;
}

bool FileBrowser::matches_filter(const std::string& filename) const {
    if (extensions_filter_.empty()) {
        return true;
    }

    // Convert filename to lowercase
    std::string lower_filename = filename;
    std::transform(lower_filename.begin(), lower_filename.end(),
                   lower_filename.begin(), ::tolower);

    // Check each extension in the filter
    std::string filter = extensions_filter_;
    std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

    size_t pos = 0;
    while (pos < filter.length()) {
        size_t comma = filter.find(',', pos);
        if (comma == std::string::npos) {
            comma = filter.length();
        }

        std::string ext = filter.substr(pos, comma - pos);
        
        // Remove leading/trailing spaces
        ext.erase(0, ext.find_first_not_of(" \t"));
        ext.erase(ext.find_last_not_of(" \t") + 1);

        if (lower_filename.size() >= ext.size() &&
            lower_filename.substr(lower_filename.size() - ext.size()) == ext) {
            return true;
        }

        pos = comma + 1;
    }

    return false;
}

void FileBrowser::scan_directory() {
    entries_.clear();

    try {
        // Add parent directory entry if not at root
        fs::path current_path(current_directory_);
        if (current_path.has_parent_path() && current_path != current_path.root_path()) {
            FileEntry parent;
            parent.name = "..";
            parent.full_path = current_path.parent_path().string();
            parent.size = 0;
            parent.is_directory = true;
            entries_.push_back(parent);
        }

        // Scan directory
        for (const auto& entry : fs::directory_iterator(current_directory_)) {
            FileEntry file_entry;
            file_entry.name = entry.path().filename().string();
            file_entry.full_path = entry.path().string();
            file_entry.is_directory = entry.is_directory();

            if (file_entry.is_directory) {
                file_entry.size = 0;
                entries_.push_back(file_entry);
            } else if (matches_filter(file_entry.name)) {
                try {
                    file_entry.size = fs::file_size(entry.path());
                } catch (...) {
                    file_entry.size = 0;
                }
                entries_.push_back(file_entry);
            }
        }

        // Sort: directories first, then files, alphabetically
        std::sort(entries_.begin(), entries_.end(),
                  [](const FileEntry& a, const FileEntry& b) {
                      if (a.name == "..") return true;
                      if (b.name == "..") return false;
                      if (a.is_directory != b.is_directory) {
                          return a.is_directory;
                      }
                      return a.name < b.name;
                  });

    } catch (const fs::filesystem_error&) {
        // If scan fails, add error entry
        FileEntry error;
        error.name = "[Error reading directory]";
        error.full_path = "";
        error.size = 0;
        error.is_directory = false;
        entries_.push_back(error);
    }

    // Reset selection
    if (selected_index_ >= static_cast<int>(entries_.size())) {
        selected_index_ = entries_.empty() ? 0 : static_cast<int>(entries_.size()) - 1;
    }
}

void FileBrowser::navigate_up() {
    fs::path current_path(current_directory_);
    if (current_path.has_parent_path() && current_path != current_path.root_path()) {
        current_directory_ = current_path.parent_path().string();
        scan_directory();
        selected_index_ = 0;
        scroll_offset_ = 0;
    }
}

void FileBrowser::navigate_into(const std::string& dir_name) {
    fs::path new_path = fs::path(current_directory_) / dir_name;
    if (fs::exists(new_path) && fs::is_directory(new_path)) {
        current_directory_ = new_path.string();
        scan_directory();
        selected_index_ = 0;
        scroll_offset_ = 0;
    }
}

void FileBrowser::select_current_file() {
    if (entries_.empty() || selected_index_ < 0 || 
        selected_index_ >= static_cast<int>(entries_.size())) {
        return;
    }

    const FileEntry& entry = entries_[selected_index_];

    if (entry.name == "..") {
        navigate_up();
    } else if (entry.is_directory) {
        navigate_into(entry.name);
    } else {
        // File selected
        selected_file_ = entry.full_path;
        file_selected_ = true;
        
        // Save directory to config based on file type
        if (current_file_type_ == FileType::BIOS) {
            config_->set_last_bios_directory(current_directory_);
        } else if (current_file_type_ == FileType::ROM) {
            config_->set_last_rom_directory(current_directory_);
        } else {
            // Generic: save to ROM directory as default
            config_->set_last_rom_directory(current_directory_);
        }
        
        close();
    }
}

void FileBrowser::ensure_selection_visible() {
    if (selected_index_ < scroll_offset_) {
        scroll_offset_ = selected_index_;
    } else if (selected_index_ >= scroll_offset_ + visible_lines_) {
        scroll_offset_ = selected_index_ - visible_lines_ + 1;
    }
}

bool FileBrowser::process_input(SDL_Keycode key) {
    if (!is_open_) return false;

    switch (key) {
        case SDLK_UP:
            if (selected_index_ > 0) {
                selected_index_--;
                ensure_selection_visible();
            }
            return true;

        case SDLK_DOWN:
            if (selected_index_ < static_cast<int>(entries_.size()) - 1) {
                selected_index_++;
                ensure_selection_visible();
            }
            return true;

        case SDLK_RETURN:
            select_current_file();
            return true;

        case SDLK_ESCAPE:
            file_selected_ = false;
            close();
            return true;

        default:
            return false;
    }
}

void FileBrowser::render() {
    if (!is_open_) return;

    // Get screen dimensions (logical size for 320x240 coordinate system)
    int screen_width, screen_height;
    SDL_RenderGetLogicalSize(renderer_, &screen_width, &screen_height);
    if (screen_width == 0 || screen_height == 0) {
        SDL_GetRendererOutputSize(renderer_, &screen_width, &screen_height);
    }

    // Semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 180);
    SDL_Rect overlay = { 0, 0, screen_width, screen_height };
    SDL_RenderFillRect(renderer_, &overlay);

    // Browser box - scaled for 320x240 resolution
    int box_width = (screen_width * 9) / 10;  // 90% of screen
    int box_height = (screen_height * 9) / 10; // 90% of screen
    int box_x = (screen_width - box_width) / 2;
    int box_y = (screen_height - box_height) / 2;

    SDL_SetRenderDrawColor(renderer_, 40, 40, 40, 255);
    SDL_Rect box = { box_x, box_y, box_width, box_height };
    SDL_RenderFillRect(renderer_, &box);

    SDL_SetRenderDrawColor(renderer_, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer_, &box);

    // Title
    SDL_Color title_color = { 255, 255, 255, 255 };
    text_renderer_->render_text("Select File", box_x + 10, box_y + 5,
                                title_color, TextRenderer::FontSize::Large);

    // Current directory
    SDL_Color dir_color = { 150, 150, 255, 255 };
    text_renderer_->render_text(current_directory_, box_x + 10, box_y + 20,
                                dir_color, TextRenderer::FontSize::Small);

    // File list
    int list_y = box_y + 32;  // Reduced from 70
    int line_height = 12;     // Reduced from 20
    visible_lines_ = (box_height - 50) / line_height;  // Adjusted calculation

    for (int i = 0; i < visible_lines_ && (scroll_offset_ + i) < static_cast<int>(entries_.size()); i++) {
        int entry_index = scroll_offset_ + i;
        const FileEntry& entry = entries_[entry_index];

        bool is_selected = (entry_index == selected_index_);
        SDL_Color text_color = is_selected ? 
            SDL_Color{ 255, 255, 100, 255 } : 
            (entry.is_directory ? SDL_Color{ 100, 200, 255, 255 } : SDL_Color{ 200, 200, 200, 255 });

        std::string display_name = entry.name;
        if (entry.is_directory && entry.name != "..") {
            display_name = "[" + display_name + "]";
        }

        text_renderer_->render_text(display_name, box_x + 10, list_y + i * line_height,
                                    text_color, TextRenderer::FontSize::Medium);

        // Show file size for files
        if (!entry.is_directory && entry.name != "..") {
            std::string size_str;
            if (entry.size < 1024) {
                size_str = std::to_string(entry.size) + " B";
            } else if (entry.size < 1024 * 1024) {
                size_str = std::to_string(entry.size / 1024) + " KB";
            } else {
                size_str = std::to_string(entry.size / (1024 * 1024)) + " MB";
            }

            text_renderer_->render_text(size_str, box_x + box_width - 50, list_y + i * line_height,
                                        text_color, TextRenderer::FontSize::Small);
        }
    }

    // Hints
    SDL_Color hint_color = { 150, 150, 150, 255 };
    text_renderer_->render_text("Arrows: Move | Enter: Select | Esc: Cancel",
                                box_x + 10, box_y + box_height - 15,
                                hint_color, TextRenderer::FontSize::Small);
}
