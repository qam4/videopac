#ifndef ZIP_HANDLER_H
#define ZIP_HANDLER_H

#include <string>
#include <vector>

// ZIP file handler for extracting ROM files from archives
// Uses miniz library for ZIP decompression
class ZIPHandler {
public:
    ZIPHandler();
    ~ZIPHandler();

    // Open a ZIP archive
    // Returns true on success, false if file doesn't exist or is not a valid ZIP
    bool open(const std::string& zip_path);

    // Close the currently open ZIP archive
    void close();

    // Get list of ROM files in the archive (.bin, .rom extensions)
    // Returns empty vector if no ROM files found
    std::vector<std::string> get_rom_files() const;

    // Extract a specific file from the archive to a temporary directory
    // Returns the path to the extracted file, or empty string on error
    std::string extract_file(const std::string& filename);

    // Extract all ROM files from the archive
    // Returns paths to extracted files
    std::vector<std::string> extract_all_roms();

    // Clean up all temporary extracted files
    void cleanup_temp_files();

    // Get the temporary directory used for extraction
    std::string get_temp_directory() const;

private:
    // Check if a filename has a ROM extension (.bin, .rom)
    bool is_rom_file(const std::string& filename) const;

    // Get temporary directory path (uses system temp + unique subdirectory)
    std::string create_temp_directory();

    // Extract a file to a specific path
    bool extract_to_path(const std::string& filename, const std::string& output_path);

    void* zip_archive_;  // mz_zip_archive pointer (opaque to avoid including miniz.h here)
    std::string zip_path_;
    std::string temp_dir_;
    std::vector<std::string> temp_files_;  // Track extracted files for cleanup
    bool is_open_;
};

#endif // ZIP_HANDLER_H
