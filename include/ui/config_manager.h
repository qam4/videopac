#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include "ui/recent_files_list.h"

// Configuration manager for persistent settings
// Uses simple INI file format with sections and key=value pairs
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    // Load configuration from file
    // Returns true on success, false if file doesn't exist or is corrupted
    // If loading fails, default values are used
    bool load();

    // Save configuration to file
    // Returns true on success, false on I/O error
    bool save();

    // Get configuration file path (uses SDL_GetPrefPath)
    std::string get_config_path() const;

    // General settings
    std::string get_last_rom_directory() const;
    void set_last_rom_directory(const std::string& path);

    std::string get_last_bios_directory() const;
    void set_last_bios_directory(const std::string& path);

    std::string get_last_rom_path() const;
    void set_last_rom_path(const std::string& path);

    std::string get_last_bios_path() const;
    void set_last_bios_path(const std::string& path);

    bool get_auto_load_last_files() const;
    void set_auto_load_last_files(bool enabled);

    // Video settings
    std::string get_scaling_filter() const;  // "nearest" or "linear"
    void set_scaling_filter(const std::string& filter);

    std::string get_aspect_ratio() const;  // "original", "4:3", "stretch"
    void set_aspect_ratio(const std::string& ratio);

    bool get_fullscreen() const;
    void set_fullscreen(bool enabled);

    std::string get_crt_effect() const;  // "none", "light", "medium", "heavy"
    void set_crt_effect(const std::string& effect);

    int get_scanlines() const;  // 0, 25, 50, 75
    void set_scanlines(int percent);

    // Audio settings
    int get_volume() const;  // 0-100
    void set_volume(int volume);

    bool get_audio_muted() const;
    void set_audio_muted(bool muted);

    int get_audio_buffer_size() const;  // 512, 1024, 2048
    void set_audio_buffer_size(int size);

    // Input settings
    std::string get_keyboard_mapping(int player, const std::string& action) const;
    void set_keyboard_mapping(int player, const std::string& action, const std::string& key);

    int get_joystick_device(int player) const;  // -1 for none
    void set_joystick_device(int player, int device_id);

    // OSD settings
    std::string get_fps_position() const;  // "top-left", "top-right", "bottom-left", "bottom-right"
    void set_fps_position(const std::string& position);

    std::string get_notification_position() const;
    void set_notification_position(const std::string& position);

    std::string get_osd_font_size() const;  // "small", "medium", "large"
    void set_osd_font_size(const std::string& size);

    int get_osd_opacity() const;  // 25, 50, 75, 100
    void set_osd_opacity(int percent);

    bool get_fps_display_enabled() const;
    void set_fps_display_enabled(bool enabled);

    // Screenshot settings
    std::string get_screenshot_format() const;  // "png", "bmp", "tga"
    void set_screenshot_format(const std::string& format);

    // Speed settings
    int get_emulation_speed() const;  // 25, 50, 100, 200, 400, 0 (unlimited)
    void set_emulation_speed(int percent);

    // Recent files (up to 10 entries)
    std::vector<std::string> get_recent_roms() const;
    void add_recent_rom(const std::string& path);
    bool remove_recent_rom(const std::string& path);

    std::vector<std::string> get_recent_bios() const;
    void add_recent_bios(const std::string& path);
    bool remove_recent_bios(const std::string& path);

private:
    // INI file parsing
    bool parse_ini_file(const std::string& filepath);
    bool write_ini_file(const std::string& filepath);

    // Helper methods
    std::string get_value(const std::string& section, const std::string& key, const std::string& default_value) const;
    int get_value_int(const std::string& section, const std::string& key, int default_value) const;
    bool get_value_bool(const std::string& section, const std::string& key, bool default_value) const;

    void set_value(const std::string& section, const std::string& key, const std::string& value);
    void set_value_int(const std::string& section, const std::string& key, int value);
    void set_value_bool(const std::string& section, const std::string& key, bool value);

    std::string trim(const std::string& str) const;

    // Configuration storage: section -> (key -> value)
    std::map<std::string, std::map<std::string, std::string>> config_;

    // Configuration file path
    std::string config_path_;

    // Recent files lists
    RecentFilesList recent_roms_;
    RecentFilesList recent_bios_;

    // Helper to load recent files from config
    void load_recent_files();
    
    // Helper to save recent files to config
    void save_recent_files();
};

#endif // CONFIG_MANAGER_H
