#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H

#include <string>
#include <vector>
#include <SDL.h>

// Forward declarations
class TextRenderer;
class ConfigManager;

// File entry structure
struct FileEntry {
    std::string name;
    std::string full_path;
    size_t size;
    bool is_directory;
};

// File browser for selecting files from the filesystem
class FileBrowser {
public:
    FileBrowser(SDL_Renderer* renderer, TextRenderer* text_renderer, ConfigManager* config);
    ~FileBrowser();

    // File type for directory memory
    enum class FileType {
        ROM,
        BIOS,
        Generic
    };

    // Open the file browser with specified file extensions filter
    // extensions: comma-separated list (e.g., ".bin,.rom,.zip")
    // file_type: type of file being browsed (for directory memory)
    void open(const std::string& extensions, FileType file_type = FileType::Generic);

    // Close the file browser
    void close();

    // Check if browser is open
    bool is_open() const { return is_open_; }

    // Process keyboard input
    // Returns true if input was handled
    bool process_input(SDL_Keycode key);

    // Render the file browser
    void render();

    // Get the selected file path (empty if cancelled)
    std::string get_selected_file() const { return selected_file_; }

    // Check if a file was selected (vs cancelled)
    bool was_file_selected() const { return file_selected_; }

private:
    // Scan current directory and populate file list
    void scan_directory();

    // Filter files by extension
    bool matches_filter(const std::string& filename) const;

    // Navigate to parent directory
    void navigate_up();

    // Navigate into selected directory
    void navigate_into(const std::string& dir_name);

    // Select current file
    void select_current_file();

    // Scroll handling
    void ensure_selection_visible();

    SDL_Renderer* renderer_;
    TextRenderer* text_renderer_;
    ConfigManager* config_;

    std::string current_directory_;
    std::string extensions_filter_;
    std::vector<FileEntry> entries_;
    
    int selected_index_;
    int scroll_offset_;
    int visible_lines_;

    bool is_open_;
    bool file_selected_;
    std::string selected_file_;
    FileType current_file_type_;
};

#endif // FILE_BROWSER_H
