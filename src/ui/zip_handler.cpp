#include "ui/zip_handler.h"
#include <miniz.h>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

ZIPHandler::ZIPHandler() 
    : zip_archive_(nullptr), is_open_(false) {
    // Allocate mz_zip_archive structure
    zip_archive_ = new mz_zip_archive();
    std::memset(zip_archive_, 0, sizeof(mz_zip_archive));
}

ZIPHandler::~ZIPHandler() {
    cleanup_temp_files();
    close();
    
    if (zip_archive_) {
        delete static_cast<mz_zip_archive*>(zip_archive_);
        zip_archive_ = nullptr;
    }
}

bool ZIPHandler::open(const std::string& zip_path) {
    if (is_open_) {
        close();
    }

    zip_path_ = zip_path;
    mz_zip_archive* archive = static_cast<mz_zip_archive*>(zip_archive_);
    
    // Initialize and open the ZIP archive
    if (!mz_zip_reader_init_file(archive, zip_path.c_str(), 0)) {
        return false;
    }

    is_open_ = true;
    return true;
}

void ZIPHandler::close() {
    if (is_open_) {
        mz_zip_archive* archive = static_cast<mz_zip_archive*>(zip_archive_);
        mz_zip_reader_end(archive);
        is_open_ = false;
    }
}

bool ZIPHandler::is_rom_file(const std::string& filename) const {
    // Convert to lowercase for case-insensitive comparison
    std::string lower_filename = filename;
    std::transform(lower_filename.begin(), lower_filename.end(), 
                   lower_filename.begin(), ::tolower);

    return (lower_filename.size() > 4 && 
            (lower_filename.substr(lower_filename.size() - 4) == ".bin" ||
             lower_filename.substr(lower_filename.size() - 4) == ".rom"));
}

std::vector<std::string> ZIPHandler::get_rom_files() const {
    std::vector<std::string> rom_files;

    if (!is_open_) {
        return rom_files;
    }

    mz_zip_archive* archive = static_cast<mz_zip_archive*>(zip_archive_);
    int num_files = mz_zip_reader_get_num_files(archive);

    for (int i = 0; i < num_files; i++) {
        mz_zip_archive_file_stat file_stat;
        if (mz_zip_reader_file_stat(archive, i, &file_stat)) {
            std::string filename = file_stat.m_filename;
            
            // Skip directories
            if (filename.empty() || filename.back() == '/' || filename.back() == '\\') {
                continue;
            }

            if (is_rom_file(filename)) {
                rom_files.push_back(filename);
            }
        }
    }

    return rom_files;
}

std::string ZIPHandler::create_temp_directory() {
    if (!temp_dir_.empty()) {
        return temp_dir_;
    }

    // Get system temp directory
    fs::path temp_path = fs::temp_directory_path();
    
    // Create unique subdirectory for this ZIP extraction
    std::string unique_name = "videopac_zip_" + std::to_string(std::time(nullptr));
    temp_path /= unique_name;

    // Create the directory
    try {
        fs::create_directories(temp_path);
        temp_dir_ = temp_path.string();
    } catch (const fs::filesystem_error&) {
        return "";
    }

    return temp_dir_;
}

std::string ZIPHandler::get_temp_directory() const {
    return temp_dir_;
}

bool ZIPHandler::extract_to_path(const std::string& filename, const std::string& output_path) {
    if (!is_open_) {
        return false;
    }

    mz_zip_archive* archive = static_cast<mz_zip_archive*>(zip_archive_);

    // Find the file in the archive
    int file_index = mz_zip_reader_locate_file(archive, filename.c_str(), nullptr, 0);
    if (file_index < 0) {
        return false;
    }

    // Extract the file
    if (!mz_zip_reader_extract_to_file(archive, file_index, output_path.c_str(), 0)) {
        return false;
    }

    return true;
}

std::string ZIPHandler::extract_file(const std::string& filename) {
    if (!is_open_) {
        return "";
    }

    // Create temp directory if needed
    std::string temp_dir = create_temp_directory();
    if (temp_dir.empty()) {
        return "";
    }

    // Get just the filename without path
    fs::path file_path(filename);
    std::string base_filename = file_path.filename().string();

    // Build output path
    fs::path output_path = fs::path(temp_dir) / base_filename;
    std::string output_str = output_path.string();

    // Extract the file
    if (!extract_to_path(filename, output_str)) {
        return "";
    }

    // Track for cleanup
    temp_files_.push_back(output_str);

    return output_str;
}

std::vector<std::string> ZIPHandler::extract_all_roms() {
    std::vector<std::string> extracted_files;
    std::vector<std::string> rom_files = get_rom_files();

    for (const auto& rom_file : rom_files) {
        std::string extracted_path = extract_file(rom_file);
        if (!extracted_path.empty()) {
            extracted_files.push_back(extracted_path);
        }
    }

    return extracted_files;
}

void ZIPHandler::cleanup_temp_files() {
    // Delete all extracted files
    for (const auto& file_path : temp_files_) {
        try {
            fs::remove(file_path);
        } catch (const fs::filesystem_error&) {
            // Ignore errors during cleanup
        }
    }
    temp_files_.clear();

    // Delete temp directory if it exists
    if (!temp_dir_.empty()) {
        try {
            fs::remove_all(temp_dir_);
        } catch (const fs::filesystem_error&) {
            // Ignore errors during cleanup
        }
        temp_dir_.clear();
    }
}
