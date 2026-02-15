#include "ui/config_manager.h"
#include "ui/recent_files_list.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <SDL.h>

ConfigManager::ConfigManager()
    : recent_roms_(10), recent_bios_(10) {
    // Get configuration file path using SDL_GetPrefPath
    char* pref_path = SDL_GetPrefPath("videopac", "emulator");
    if (pref_path) {
        config_path_ = std::string(pref_path) + "config.ini";
        SDL_free(pref_path);
    } else {
        // Fallback to current directory
        config_path_ = "config.ini";
    }
}

ConfigManager::~ConfigManager() {
}

std::string ConfigManager::get_config_path() const {
    return config_path_;
}

bool ConfigManager::load() {
    bool result = parse_ini_file(config_path_);
    if (result) {
        load_recent_files();
    }
    return result;
}

bool ConfigManager::save() {
    save_recent_files();
    return write_ini_file(config_path_);
}

bool ConfigManager::parse_ini_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    std::string current_section;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // Check for section header
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            current_section = line.substr(1, line.length() - 2);
            continue;
        }

        // Parse key=value pair
        size_t equals_pos = line.find('=');
        if (equals_pos != std::string::npos) {
            std::string key = trim(line.substr(0, equals_pos));
            std::string value = trim(line.substr(equals_pos + 1));
            
            if (!current_section.empty() && !key.empty()) {
                config_[current_section][key] = value;
            }
        }
    }

    file.close();
    return true;
}

bool ConfigManager::write_ini_file(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    // Write sections and key-value pairs
    for (const auto& section_pair : config_) {
        file << "[" << section_pair.first << "]\n";
        
        for (const auto& kv_pair : section_pair.second) {
            file << kv_pair.first << "=" << kv_pair.second << "\n";
        }
        
        file << "\n";
    }

    file.close();
    return true;
}

std::string ConfigManager::trim(const std::string& str) const {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::string ConfigManager::get_value(const std::string& section, const std::string& key, const std::string& default_value) const {
    auto section_it = config_.find(section);
    if (section_it != config_.end()) {
        auto key_it = section_it->second.find(key);
        if (key_it != section_it->second.end()) {
            return key_it->second;
        }
    }
    return default_value;
}

int ConfigManager::get_value_int(const std::string& section, const std::string& key, int default_value) const {
    std::string value = get_value(section, key, "");
    if (value.empty()) {
        return default_value;
    }
    
    try {
        return std::stoi(value);
    } catch (...) {
        return default_value;
    }
}

bool ConfigManager::get_value_bool(const std::string& section, const std::string& key, bool default_value) const {
    std::string value = get_value(section, key, "");
    if (value.empty()) {
        return default_value;
    }
    
    // Convert to lowercase for comparison
    std::string lower_value = value;
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
    
    return (lower_value == "true" || lower_value == "1" || lower_value == "yes");
}

void ConfigManager::set_value(const std::string& section, const std::string& key, const std::string& value) {
    config_[section][key] = value;
}

void ConfigManager::set_value_int(const std::string& section, const std::string& key, int value) {
    config_[section][key] = std::to_string(value);
}

void ConfigManager::set_value_bool(const std::string& section, const std::string& key, bool value) {
    config_[section][key] = value ? "true" : "false";
}

// General settings
std::string ConfigManager::get_last_rom_directory() const {
    return get_value("General", "last_rom_directory", ".");
}

void ConfigManager::set_last_rom_directory(const std::string& path) {
    set_value("General", "last_rom_directory", path);
}

std::string ConfigManager::get_last_bios_directory() const {
    return get_value("General", "last_bios_directory", ".");
}

void ConfigManager::set_last_bios_directory(const std::string& path) {
    set_value("General", "last_bios_directory", path);
}

std::string ConfigManager::get_last_rom_path() const {
    return get_value("General", "last_rom_path", "");
}

void ConfigManager::set_last_rom_path(const std::string& path) {
    set_value("General", "last_rom_path", path);
}

std::string ConfigManager::get_last_bios_path() const {
    return get_value("General", "last_bios_path", "");
}

void ConfigManager::set_last_bios_path(const std::string& path) {
    set_value("General", "last_bios_path", path);
}

bool ConfigManager::get_auto_load_last_files() const {
    return get_value_bool("General", "auto_load_last_files", true);
}

void ConfigManager::set_auto_load_last_files(bool enabled) {
    set_value_bool("General", "auto_load_last_files", enabled);
}

// Video settings
std::string ConfigManager::get_scaling_filter() const {
    return get_value("Video", "scaling_filter", "nearest");
}

void ConfigManager::set_scaling_filter(const std::string& filter) {
    set_value("Video", "scaling_filter", filter);
}

std::string ConfigManager::get_aspect_ratio() const {
    return get_value("Video", "aspect_ratio", "original");
}

void ConfigManager::set_aspect_ratio(const std::string& ratio) {
    set_value("Video", "aspect_ratio", ratio);
}

bool ConfigManager::get_vsync_enabled() const {
    return get_value_bool("Video", "vsync", true);
}

void ConfigManager::set_vsync_enabled(bool enabled) {
    set_value_bool("Video", "vsync", enabled);
}

bool ConfigManager::get_fullscreen() const {
    return get_value_bool("Video", "fullscreen", false);
}

void ConfigManager::set_fullscreen(bool enabled) {
    set_value_bool("Video", "fullscreen", enabled);
}

std::string ConfigManager::get_crt_effect() const {
    return get_value("Video", "crt_effect", "none");
}

void ConfigManager::set_crt_effect(const std::string& effect) {
    set_value("Video", "crt_effect", effect);
}

int ConfigManager::get_scanlines() const {
    return get_value_int("Video", "scanlines", 0);
}

void ConfigManager::set_scanlines(int percent) {
    set_value_int("Video", "scanlines", percent);
}

// Audio settings
int ConfigManager::get_volume() const {
    return get_value_int("Audio", "volume", 100);
}

void ConfigManager::set_volume(int volume) {
    set_value_int("Audio", "volume", volume);
}

bool ConfigManager::get_audio_muted() const {
    return get_value_bool("Audio", "muted", false);
}

void ConfigManager::set_audio_muted(bool muted) {
    set_value_bool("Audio", "muted", muted);
}

int ConfigManager::get_audio_buffer_size() const {
    return get_value_int("Audio", "buffer_size", 1024);
}

void ConfigManager::set_audio_buffer_size(int size) {
    set_value_int("Audio", "buffer_size", size);
}

// Input settings
std::string ConfigManager::get_keyboard_mapping(int player, const std::string& action) const {
    std::string key = "player" + std::to_string(player) + "_" + action;
    
    // Default mappings
    if (player == 1) {
        if (action == "up") return get_value("Input", key, "Up");
        if (action == "down") return get_value("Input", key, "Down");
        if (action == "left") return get_value("Input", key, "Left");
        if (action == "right") return get_value("Input", key, "Right");
        if (action == "button") return get_value("Input", key, "Space");
    } else if (player == 2) {
        if (action == "up") return get_value("Input", key, "W");
        if (action == "down") return get_value("Input", key, "S");
        if (action == "left") return get_value("Input", key, "A");
        if (action == "right") return get_value("Input", key, "D");
        if (action == "button") return get_value("Input", key, "LShift");
    }
    
    return get_value("Input", key, "");
}

void ConfigManager::set_keyboard_mapping(int player, const std::string& action, const std::string& key) {
    std::string config_key = "player" + std::to_string(player) + "_" + action;
    set_value("Input", config_key, key);
}

int ConfigManager::get_joystick_device(int player) const {
    std::string key = "player" + std::to_string(player) + "_joystick";
    return get_value_int("Input", key, -1);
}

void ConfigManager::set_joystick_device(int player, int device_id) {
    std::string key = "player" + std::to_string(player) + "_joystick";
    set_value_int("Input", key, device_id);
}

// OSD settings
std::string ConfigManager::get_fps_position() const {
    return get_value("OSD", "fps_position", "top-left");
}

void ConfigManager::set_fps_position(const std::string& position) {
    set_value("OSD", "fps_position", position);
}

std::string ConfigManager::get_notification_position() const {
    return get_value("OSD", "notification_position", "top-right");
}

void ConfigManager::set_notification_position(const std::string& position) {
    set_value("OSD", "notification_position", position);
}

std::string ConfigManager::get_osd_font_size() const {
    return get_value("OSD", "font_size", "medium");
}

void ConfigManager::set_osd_font_size(const std::string& size) {
    set_value("OSD", "font_size", size);
}

int ConfigManager::get_osd_opacity() const {
    return get_value_int("OSD", "opacity", 100);
}

void ConfigManager::set_osd_opacity(int percent) {
    set_value_int("OSD", "opacity", percent);
}

bool ConfigManager::get_fps_display_enabled() const {
    return get_value_bool("OSD", "fps_enabled", false);
}

void ConfigManager::set_fps_display_enabled(bool enabled) {
    set_value_bool("OSD", "fps_enabled", enabled);
}

// Screenshot settings
std::string ConfigManager::get_screenshot_format() const {
    return get_value("Screenshot", "format", "png");
}

void ConfigManager::set_screenshot_format(const std::string& format) {
    set_value("Screenshot", "format", format);
}

// Speed settings
int ConfigManager::get_emulation_speed() const {
    return get_value_int("Speed", "emulation_speed", 100);
}

void ConfigManager::set_emulation_speed(int percent) {
    set_value_int("Speed", "emulation_speed", percent);
}

// Recent files
void ConfigManager::load_recent_files() {
    // Load recent ROMs
    recent_roms_.clear();
    for (int i = 0; i < 10; i++) {
        std::string key = "rom" + std::to_string(i);
        std::string path = get_value("RecentFiles", key, "");
        if (!path.empty()) {
            recent_roms_.add(path);
        }
    }
    
    // Load recent BIOS
    recent_bios_.clear();
    for (int i = 0; i < 10; i++) {
        std::string key = "bios" + std::to_string(i);
        std::string path = get_value("RecentFiles", key, "");
        if (!path.empty()) {
            recent_bios_.add(path);
        }
    }
}

void ConfigManager::save_recent_files() {
    // Save recent ROMs
    std::vector<std::string> roms = recent_roms_.get_all();
    for (size_t i = 0; i < roms.size(); i++) {
        std::string key = "rom" + std::to_string(i);
        set_value("RecentFiles", key, roms[i]);
    }
    
    // Save recent BIOS
    std::vector<std::string> bios = recent_bios_.get_all();
    for (size_t i = 0; i < bios.size(); i++) {
        std::string key = "bios" + std::to_string(i);
        set_value("RecentFiles", key, bios[i]);
    }
}

std::vector<std::string> ConfigManager::get_recent_roms() const {
    std::vector<std::string> result;
    std::vector<std::string> all_roms = recent_roms_.get_all();
    
    // Filter out files that no longer exist
    for (const auto& path : all_roms) {
        if (std::filesystem::exists(path)) {
            result.push_back(path);
        }
    }
    
    return result;
}

void ConfigManager::add_recent_rom(const std::string& path) {
    recent_roms_.add(path);
}

bool ConfigManager::remove_recent_rom(const std::string& path) {
    return recent_roms_.remove(path);
}

std::vector<std::string> ConfigManager::get_recent_bios() const {
    std::vector<std::string> result;
    std::vector<std::string> all_bios = recent_bios_.get_all();
    
    // Filter out files that no longer exist
    for (const auto& path : all_bios) {
        if (std::filesystem::exists(path)) {
            result.push_back(path);
        }
    }
    
    return result;
}

void ConfigManager::add_recent_bios(const std::string& path) {
    recent_bios_.add(path);
}

bool ConfigManager::remove_recent_bios(const std::string& path) {
    return recent_bios_.remove(path);
}
