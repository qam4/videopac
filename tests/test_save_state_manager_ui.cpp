#define _CRT_SECURE_NO_WARNINGS
#include <gtest/gtest.h>
#include "ui/save_state_manager.h"
#include "emulator.h"
#include <SDL2/SDL.h>
#include <filesystem>
#include <fstream>
#include <cstdio>

using namespace videopac;

namespace fs = std::filesystem;

class SaveStateManagerUITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize SDL for renderer
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            GTEST_SKIP() << "SDL initialization failed: " << SDL_GetError();
        }
        
        // Create a minimal window and renderer for testing
        window_ = SDL_CreateWindow("Test", 0, 0, 320, 240, SDL_WINDOW_HIDDEN);
        if (!window_) {
            SDL_Quit();
            GTEST_SKIP() << "Failed to create window: " << SDL_GetError();
        }
        
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
        if (!renderer_) {
            SDL_DestroyWindow(window_);
            SDL_Quit();
            GTEST_SKIP() << "Failed to create renderer: " << SDL_GetError();
        }
        
        // Create emulator with test configuration
        Configuration config;
        config.video_standard = VideoStandard::NTSC;
        emulator_ = std::make_unique<EmulatorCore>(config);
        
        // Load a minimal BIOS (just zeros for testing)
        std::vector<videopac::uint8> minimal_bios(1024, 0);
        emulator_->load_bios(minimal_bios.data(), minimal_bios.size());
        
        // Create save state manager
        manager_ = std::make_unique<SaveStateManagerUI>(emulator_.get(), renderer_);
        
        // Get test saves directory
        test_saves_dir_ = manager_->get_saves_directory();
    }
    
    void TearDown() override {
        // Clean up test save files
        cleanup_test_files();
        
        manager_.reset();
        emulator_.reset();
        
        if (renderer_) {
            SDL_DestroyRenderer(renderer_);
        }
        if (window_) {
            SDL_DestroyWindow(window_);
        }
        
        SDL_Quit();
    }
    
    void cleanup_test_files() {
        // Remove test save files
        for (int slot = 0; slot <= 9; ++slot) {
            std::string state_file = test_saves_dir_ + "/test_rom.state" + std::to_string(slot);
            std::string thumb_file = state_file + ".png";
            
            if (fs::exists(state_file)) {
                fs::remove(state_file);
            }
            if (fs::exists(thumb_file)) {
                fs::remove(thumb_file);
            }
        }
    }
    
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    std::unique_ptr<EmulatorCore> emulator_;
    std::unique_ptr<SaveStateManagerUI> manager_;
    std::string test_saves_dir_;
};

// Test: Saves directory auto-creation
TEST_F(SaveStateManagerUITest, SavesDirectoryAutoCreation) {
    // The saves directory should be created during manager construction
    EXPECT_TRUE(fs::exists(test_saves_dir_)) << "Saves directory not created";
    EXPECT_TRUE(fs::is_directory(test_saves_dir_)) << "Saves path is not a directory";
}

// Test: Save state creation
TEST_F(SaveStateManagerUITest, DISABLED_SaveStateCreation) {
    const std::string rom_name = "test_rom";
    const int slot = 1;
    
    // Save a state
    auto result = manager_->save_state(slot, rom_name);
    ASSERT_TRUE(result.is_ok()) << "Save failed: " << result.error;
    
    // Verify state file exists
    std::string state_file = test_saves_dir_ + "/test_rom.state1";
    EXPECT_TRUE(fs::exists(state_file)) << "State file not created";
    
    // Verify thumbnail file exists
    std::string thumb_file = state_file + ".png";
    EXPECT_TRUE(fs::exists(thumb_file)) << "Thumbnail file not created";
}

// Test: Save state loading
TEST_F(SaveStateManagerUITest, DISABLED_SaveStateLoading) {
    const std::string rom_name = "test_rom";
    const int slot = 2;
    
    // Save a state first
    auto save_result = manager_->save_state(slot, rom_name);
    ASSERT_TRUE(save_result.is_ok());
    
    // Load the state
    auto load_result = manager_->load_state(slot, rom_name);
    EXPECT_TRUE(load_result.is_ok()) << "Load failed: " << load_result.error;
}

// Test: Load non-existent state
TEST_F(SaveStateManagerUITest, LoadNonExistentState) {
    const std::string rom_name = "nonexistent_rom";
    const int slot = 5;
    
    // Try to load a state that doesn't exist
    auto result = manager_->load_state(slot, rom_name);
    EXPECT_FALSE(result.is_ok()) << "Should fail to load non-existent state";
}

// Test: Thumbnail capture
TEST_F(SaveStateManagerUITest, DISABLED_ThumbnailCapture) {
    const std::string rom_name = "test_rom";
    const int slot = 3;
    
    // Save a state (which should capture thumbnail)
    auto result = manager_->save_state(slot, rom_name);
    ASSERT_TRUE(result.is_ok());
    
    // Verify thumbnail file exists and has content
    std::string thumb_file = test_saves_dir_ + "/test_rom.state3.png";
    ASSERT_TRUE(fs::exists(thumb_file));
    
    // Check file size is reasonable (should be > 0)
    auto file_size = fs::file_size(thumb_file);
    EXPECT_GT(file_size, 0) << "Thumbnail file is empty";
}

// Test: Save state deletion
TEST_F(SaveStateManagerUITest, DISABLED_SaveStateDeletion) {
    const std::string rom_name = "test_rom";
    const int slot = 4;
    
    // Save a state first
    auto save_result = manager_->save_state(slot, rom_name);
    ASSERT_TRUE(save_result.is_ok());
    
    // Verify files exist
    std::string state_file = test_saves_dir_ + "/test_rom.state4";
    std::string thumb_file = state_file + ".png";
    ASSERT_TRUE(fs::exists(state_file));
    ASSERT_TRUE(fs::exists(thumb_file));
    
    // Delete the state
    auto delete_result = manager_->delete_state(slot, rom_name);
    EXPECT_TRUE(delete_result.is_ok()) << "Delete failed: " << delete_result.error;
    
    // Verify files are deleted
    EXPECT_FALSE(fs::exists(state_file)) << "State file not deleted";
    EXPECT_FALSE(fs::exists(thumb_file)) << "Thumbnail file not deleted";
}

// Test: Delete non-existent state
TEST_F(SaveStateManagerUITest, DeleteNonExistentState) {
    const std::string rom_name = "nonexistent_rom";
    const int slot = 7;
    
    // Try to delete a state that doesn't exist
    auto result = manager_->delete_state(slot, rom_name);
    EXPECT_FALSE(result.is_ok()) << "Should fail to delete non-existent state";
}

// Test: Listing save states with timestamps
TEST_F(SaveStateManagerUITest, DISABLED_ListSaveStatesWithTimestamps) {
    const std::string rom_name = "test_rom";
    
    // Save states in slots 0, 2, and 5
    manager_->save_state(0, rom_name);
    manager_->save_state(2, rom_name);
    manager_->save_state(5, rom_name);
    
    // List all states
    auto states = manager_->list_states(rom_name);
    
    // Should return info for all 10 slots (0-9)
    ASSERT_EQ(states.size(), 10);
    
    // Check slot 0 exists
    EXPECT_TRUE(states[0].exists);
    EXPECT_EQ(states[0].slot, 0);
    EXPECT_GT(states[0].timestamp, 0) << "Timestamp should be set";
    
    // Check slot 1 doesn't exist
    EXPECT_FALSE(states[1].exists);
    EXPECT_EQ(states[1].slot, 1);
    
    // Check slot 2 exists
    EXPECT_TRUE(states[2].exists);
    EXPECT_EQ(states[2].slot, 2);
    
    // Check slot 5 exists
    EXPECT_TRUE(states[5].exists);
    EXPECT_EQ(states[5].slot, 5);
}

// Test: Empty save slots indicated
TEST_F(SaveStateManagerUITest, EmptySaveSlots) {
    const std::string rom_name = "empty_test_rom";
    
    // List states for a ROM that has no saves
    auto states = manager_->list_states(rom_name);
    
    // All slots should be marked as not existing
    for (const auto& state : states) {
        EXPECT_FALSE(state.exists) << "Slot " << state.slot << " should be empty";
        EXPECT_EQ(state.timestamp, 0) << "Empty slot should have zero timestamp";
    }
}

// Test: Save state file naming format
TEST_F(SaveStateManagerUITest, DISABLED_SaveStateFileNaming) {
    const std::string rom_name = "my_game.bin";
    const int slot = 6;
    
    // Save a state
    auto result = manager_->save_state(slot, rom_name);
    ASSERT_TRUE(result.is_ok());
    
    // Verify filename format: {rom_name}.state{slot}
    std::string expected_state = test_saves_dir_ + "/my_game.state6";
    std::string expected_thumb = expected_state + ".png";
    
    EXPECT_TRUE(fs::exists(expected_state)) << "State file has wrong name";
    EXPECT_TRUE(fs::exists(expected_thumb)) << "Thumbnail file has wrong name";
    
    // Clean up
    fs::remove(expected_state);
    fs::remove(expected_thumb);
}

// Test: ROM name with path is handled correctly
TEST_F(SaveStateManagerUITest, DISABLED_RomNameWithPath) {
    const std::string rom_name = "/path/to/roms/game.bin";
    const int slot = 7;
    
    // Save a state
    auto result = manager_->save_state(slot, rom_name);
    ASSERT_TRUE(result.is_ok());
    
    // Verify filename uses only the base name (no path)
    std::string expected_state = test_saves_dir_ + "/game.state7";
    EXPECT_TRUE(fs::exists(expected_state)) << "State file should use base name only";
    
    // Clean up
    fs::remove(expected_state);
    fs::remove(expected_state + ".png");
}
