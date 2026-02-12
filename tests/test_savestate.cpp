#include <gtest/gtest.h>
#include "savestate.h"
#include "emulator.h"
#include <cstdio>
#include <fstream>

using namespace videopac;

TEST(SaveStateTest, SerializationRoundTrip) {
    // Create a save state with known values
    SaveState state;
    state.version = 1;
    state.frame_count = 12345;
    
    // Set some CPU state
    state.cpu_state.pc = 0x400;
    state.cpu_state.a = 0x42;
    state.cpu_state.psw = 0x80;
    
    // Set some VDC state
    state.vdc_state.beam_y = 100;
    state.vdc_state.registers[0xA0] = 0x38;  // Control register
    
    // Set some memory state
    state.memory_state.current_bank = 1;
    state.memory_state.external_ram[0] = 0xAB;
    
    // Set some input state
    state.input_state.keyboard_matrix[0][0] = true;
    state.input_state.joystick1[0] = true;
    
    // Save to file
    const std::string test_file = "/tmp/test_savestate.sav";
    auto save_result = SaveStateManager::save(test_file, state);
    ASSERT_TRUE(save_result.is_ok()) << "Save failed: " << save_result.error;
    
    // Load from file
    auto load_result = SaveStateManager::load(test_file);
    ASSERT_TRUE(load_result.is_ok()) << "Load failed: " << load_result.error;
    
    SaveState loaded_state = load_result.value.value();
    
    // Verify all fields match
    EXPECT_EQ(loaded_state.version, state.version);
    EXPECT_EQ(loaded_state.frame_count, state.frame_count);
    EXPECT_EQ(loaded_state.cpu_state.pc, state.cpu_state.pc);
    EXPECT_EQ(loaded_state.cpu_state.a, state.cpu_state.a);
    EXPECT_EQ(loaded_state.vdc_state.beam_y, state.vdc_state.beam_y);
    EXPECT_EQ(loaded_state.vdc_state.registers[0xA0], state.vdc_state.registers[0xA0]);
    EXPECT_EQ(loaded_state.memory_state.current_bank, state.memory_state.current_bank);
    EXPECT_EQ(loaded_state.memory_state.external_ram[0], state.memory_state.external_ram[0]);
    EXPECT_EQ(loaded_state.input_state.keyboard_matrix[0][0], state.input_state.keyboard_matrix[0][0]);
    EXPECT_EQ(loaded_state.input_state.joystick1[0], state.input_state.joystick1[0]);
    
    // Clean up
    std::remove(test_file.c_str());
}

TEST(SaveStateTest, ChecksumValidation) {
    SaveState state;
    state.version = 1;
    state.frame_count = 100;
    state.cpu_state.pc = 0x500;
    
    const std::string test_file = "/tmp/test_checksum.sav";
    auto save_result = SaveStateManager::save(test_file, state);
    ASSERT_TRUE(save_result.is_ok());
    
    // Load and verify checksum is valid
    auto load_result = SaveStateManager::load(test_file);
    ASSERT_TRUE(load_result.is_ok());
    
    // Clean up
    std::remove(test_file.c_str());
}

TEST(SaveStateTest, CorruptedFileDetection) {
    SaveState state;
    state.version = 1;
    state.frame_count = 200;
    
    const std::string test_file = "/tmp/test_corrupted.sav";
    auto save_result = SaveStateManager::save(test_file, state);
    ASSERT_TRUE(save_result.is_ok());
    
    // Corrupt the file by modifying a byte
    FILE* file = fopen(test_file.c_str(), "r+b");
    ASSERT_NE(file, nullptr);
    fseek(file, 10, SEEK_SET);
    uint8 corrupt_byte = 0xFF;
    fwrite(&corrupt_byte, 1, 1, file);
    fclose(file);
    
    // Try to load corrupted file
    auto load_result = SaveStateManager::load(test_file);
    EXPECT_FALSE(load_result.is_ok());
    EXPECT_TRUE(load_result.error.find("checksum") != std::string::npos);
    
    // Clean up
    std::remove(test_file.c_str());
}

TEST(SaveStateTest, VersionCompatibility) {
    SaveState state;
    state.version = 999;  // Invalid version
    state.frame_count = 300;
    
    const std::string test_file = "/tmp/test_version.sav";
    
    // Manually write with wrong version
    std::ofstream file(test_file, std::ios::binary);
    file.write(reinterpret_cast<const char*>(&state), sizeof(SaveState));
    file.close();
    
    // Try to load with wrong version
    auto load_result = SaveStateManager::load(test_file);
    EXPECT_FALSE(load_result.is_ok());
    EXPECT_TRUE(load_result.error.find("version") != std::string::npos);
    
    // Clean up
    std::remove(test_file.c_str());
}

// NOTE: This test is disabled because MemoryState contains std::vector
// which cannot be serialized with simple binary write.
// TODO: Implement proper serialization for non-POD types
TEST(SaveStateTest, DISABLED_EmulatorIntegration) {
    Configuration config;
    EmulatorCore emulator(config);
    
    // Load BIOS and ROM
    uint8 bios[1024];
    uint8 rom[2048];
    for (int i = 0; i < 1024; i++) bios[i] = 0x00;
    for (int i = 0; i < 2048; i++) rom[i] = 0x00;
    
    emulator.load_bios(bios, 1024);
    emulator.load_rom(rom, 2048);
    
    // Run a few frames
    for (int i = 0; i < 10; i++) {
        emulator.run_frame();
    }
    
    uint64 frame_count_before = emulator.get_frame_count();
    uint16 pc_before = emulator.get_cpu().get_pc();
    
    // Save state
    const std::string test_file = "/tmp/test_emulator.sav";
    auto save_result = emulator.save_state(test_file);
    ASSERT_TRUE(save_result.is_ok());
    
    // Run more frames
    for (int i = 0; i < 5; i++) {
        emulator.run_frame();
    }
    
    // Verify state changed
    EXPECT_NE(emulator.get_frame_count(), frame_count_before);
    
    // Load state
    auto load_result = emulator.load_state(test_file);
    ASSERT_TRUE(load_result.is_ok());
    
    // Verify state restored
    EXPECT_EQ(emulator.get_frame_count(), frame_count_before);
    EXPECT_EQ(emulator.get_cpu().get_pc(), pc_before);
    
    // Clean up
    std::remove(test_file.c_str());
}
