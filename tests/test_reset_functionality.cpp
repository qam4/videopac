#include <gtest/gtest.h>
#include "emulator.h"

using namespace videopac;

// Test that reset preserves loaded BIOS and ROM (Requirement 4.6)
TEST(ResetFunctionalityTest, ResetPreservesBIOSAndROM) {
    Configuration config;
    config.video_standard = VideoStandard::NTSC;
    
    EmulatorCore emulator(config);
    
    // Create test BIOS data
    uint8 bios_data[1024];
    for (int i = 0; i < 1024; i++) {
        bios_data[i] = static_cast<uint8>(i & 0xFF);
    }
    
    // Create test ROM data
    uint8 rom_data[2048];
    for (int i = 0; i < 2048; i++) {
        rom_data[i] = static_cast<uint8>((i + 100) & 0xFF);
    }
    
    // Load BIOS and ROM
    auto bios_result = emulator.load_bios(bios_data, 1024);
    ASSERT_TRUE(bios_result.is_ok()) << "Failed to load BIOS";
    
    auto rom_result = emulator.load_rom(rom_data, 2048);
    ASSERT_TRUE(rom_result.is_ok()) << "Failed to load ROM";
    
    // Get memory state before reset
    auto& memory = emulator.get_memory();
    
    // Read some BIOS bytes before reset
    uint8 bios_byte_0 = memory.read_program(0x0000);
    uint8 bios_byte_100 = memory.read_program(0x0064);
    
    // Read some ROM bytes before reset (ROM is mapped at 0x0400)
    uint8 rom_byte_0 = memory.read_program(0x0400);
    uint8 rom_byte_100 = memory.read_program(0x0464);
    
    // Reset the emulator
    emulator.reset();
    
    // Verify BIOS is still loaded (same bytes at same addresses)
    EXPECT_EQ(memory.read_program(0x0000), bios_byte_0) 
        << "BIOS data at 0x0000 should be preserved after reset";
    EXPECT_EQ(memory.read_program(0x0064), bios_byte_100) 
        << "BIOS data at 0x0064 should be preserved after reset";
    
    // Verify ROM is still loaded (same bytes at same addresses)
    EXPECT_EQ(memory.read_program(0x0400), rom_byte_0) 
        << "ROM data at 0x0400 should be preserved after reset";
    EXPECT_EQ(memory.read_program(0x0464), rom_byte_100) 
        << "ROM data at 0x0464 should be preserved after reset";
    
    // Verify CPU state was reset (PC should be at 0x0000)
    EXPECT_EQ(emulator.get_cpu().get_pc(), 0x0000) 
        << "CPU PC should be reset to 0x0000";
    
    // Verify frame count was reset
    EXPECT_EQ(emulator.get_frame_count(), 0) 
        << "Frame count should be reset to 0";
}

