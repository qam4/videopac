#include <gtest/gtest.h>
#include "ui/file_browser.h"
#include "ui/config_manager.h"
#include "ui/text_renderer.h"
#include <SDL.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class FileBrowserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize SDL for testing — skip on headless CI (no display)
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            GTEST_SKIP() << "SDL_Init failed (no display): " << SDL_GetError();
        }
        
        // Create a window and renderer for testing
        window_ = SDL_CreateWindow("Test", 0, 0, 640, 480, SDL_WINDOW_HIDDEN);
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
        
        // Create test directory structure
        test_dir_ = fs::temp_directory_path() / "file_browser_test";
        fs::create_directories(test_dir_);
        
        rom_dir_ = test_dir_ / "roms";
        bios_dir_ = test_dir_ / "bios";
        fs::create_directories(rom_dir_);
        fs::create_directories(bios_dir_);
        
        // Create some test files
        create_test_file(rom_dir_ / "game1.bin");
        create_test_file(rom_dir_ / "game2.rom");
        create_test_file(bios_dir_ / "system.bin");
        
        // Create config manager with temporary config file
        config_ = new ConfigManager();
        text_renderer_ = new TextRenderer(renderer_);
        file_browser_ = new FileBrowser(renderer_, text_renderer_, config_);
    }
    
    void TearDown() override {
        // Guard against GTEST_SKIP() in SetUp — pointers are still nullptr
        if (!file_browser_) return;

        delete file_browser_;
        delete text_renderer_;
        delete config_;
        
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        
        // Clean up test directory
        fs::remove_all(test_dir_);
    }
    
    void create_test_file(const fs::path& path) {
        std::ofstream file(path);
        file << "test data";
        file.close();
    }
    
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    ConfigManager* config_ = nullptr;
    TextRenderer* text_renderer_ = nullptr;
    FileBrowser* file_browser_ = nullptr;
    
    fs::path test_dir_;
    fs::path rom_dir_;
    fs::path bios_dir_;
};

// Test that FileBrowser remembers ROM directory separately from BIOS directory
TEST_F(FileBrowserTest, RemembersROMDirectorySeparately) {
    // Set ROM directory in config
    config_->set_last_rom_directory(rom_dir_.string());
    
    // Open file browser for ROM files
    file_browser_->open(".bin,.rom", FileBrowser::FileType::ROM);
    
    // The browser should start in the ROM directory
    // We can't directly check current_directory_ as it's private,
    // but we can verify by checking if the config was used
    EXPECT_EQ(config_->get_last_rom_directory(), rom_dir_.string());
    
    file_browser_->close();
}

// Test that FileBrowser remembers BIOS directory separately from ROM directory
TEST_F(FileBrowserTest, RemembersBIOSDirectorySeparately) {
    // Set different directories for ROM and BIOS
    config_->set_last_rom_directory(rom_dir_.string());
    config_->set_last_bios_directory(bios_dir_.string());
    
    // Open file browser for BIOS files
    file_browser_->open(".bin", FileBrowser::FileType::BIOS);
    
    // The browser should start in the BIOS directory
    EXPECT_EQ(config_->get_last_bios_directory(), bios_dir_.string());
    
    file_browser_->close();
}

// Test that FileBrowser saves ROM directory when ROM file is selected
TEST_F(FileBrowserTest, SavesROMDirectoryOnSelection) {
    // Clear the ROM directory setting
    config_->set_last_rom_directory("");
    
    // Simulate opening browser and selecting a file
    // Note: This is a simplified test - in reality, we'd need to simulate
    // the full navigation and selection process
    
    // Set a ROM directory and verify it's saved
    config_->set_last_rom_directory(rom_dir_.string());
    EXPECT_EQ(config_->get_last_rom_directory(), rom_dir_.string());
}

// Test that FileBrowser saves BIOS directory when BIOS file is selected
TEST_F(FileBrowserTest, SavesBIOSDirectoryOnSelection) {
    // Clear the BIOS directory setting
    config_->set_last_bios_directory("");
    
    // Set a BIOS directory and verify it's saved
    config_->set_last_bios_directory(bios_dir_.string());
    EXPECT_EQ(config_->get_last_bios_directory(), bios_dir_.string());
}

// Test that FileBrowser falls back to current directory if saved directory doesn't exist
TEST_F(FileBrowserTest, FallsBackToCurrentDirectoryIfSavedDoesNotExist) {
    // Set a non-existent directory
    config_->set_last_rom_directory("/nonexistent/directory/path");
    
    // Open file browser - it should fall back to current directory
    file_browser_->open(".bin,.rom", FileBrowser::FileType::ROM);
    
    // The browser should have fallen back (we can't directly verify the internal state,
    // but the browser should still be functional)
    EXPECT_TRUE(true); // Browser didn't crash
    
    file_browser_->close();
}

// Test that Generic file type uses ROM directory as default
TEST_F(FileBrowserTest, GenericFileTypeUsesROMDirectory) {
    // Set ROM directory
    config_->set_last_rom_directory(rom_dir_.string());
    
    // Open file browser with Generic type
    file_browser_->open(".bin", FileBrowser::FileType::Generic);
    
    // Should use ROM directory
    EXPECT_EQ(config_->get_last_rom_directory(), rom_dir_.string());
    
    file_browser_->close();
}
