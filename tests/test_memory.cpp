#include <gtest/gtest.h>
#include "memory.h"
#include <cstring>

using namespace videopac;

// ========== BIOS LOADING TESTS ==========

TEST(MemoryTest, LoadBIOSValidatesSize) {
    MemorySystem memory;
    
    uint8 invalid_bios[512] = {0};
    auto result = memory.load_bios(invalid_bios, 512);
    EXPECT_TRUE(result.is_err());
    
    uint8 valid_bios[1024] = {0};
    result = memory.load_bios(valid_bios, 1024);
    EXPECT_TRUE(result.is_ok());
}

TEST(MemoryTest, LoadBIOSStoresData) {
    MemorySystem memory;
    
    uint8 bios[1024];
    for (int i = 0; i < 1024; i++) {
        bios[i] = static_cast<uint8>(i & 0xFF);
    }
    
    auto result = memory.load_bios(bios, 1024);
    EXPECT_TRUE(result.is_ok());
    
    // Verify data was loaded correctly
    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(memory.read_program(i), static_cast<uint8>(i & 0xFF));
    }
}

// ========== ROM LOADING TESTS ==========

TEST(MemoryTest, LoadCartridgeValidatesSize) {
    MemorySystem memory;
    
    // Invalid sizes
    uint8 invalid_rom[1024] = {0};
    auto result = memory.load_cartridge(invalid_rom, 1024);
    EXPECT_TRUE(result.is_err());
    
    uint8 invalid_rom2[3072] = {0};
    result = memory.load_cartridge(invalid_rom2, 3072);
    EXPECT_TRUE(result.is_err());
    
    // Valid sizes
    uint8 valid_rom_2k[2048] = {0};
    result = memory.load_cartridge(valid_rom_2k, 2048);
    EXPECT_TRUE(result.is_ok());
    
    uint8 valid_rom_4k[4096] = {0};
    result = memory.load_cartridge(valid_rom_4k, 4096);
    EXPECT_TRUE(result.is_ok());
    
    uint8 valid_rom_8k[8192] = {0};
    result = memory.load_cartridge(valid_rom_8k, 8192);
    EXPECT_TRUE(result.is_ok());
}

TEST(MemoryTest, LoadCartridgeStoresData) {
    MemorySystem memory;
    
    // Load BIOS first
    uint8 bios[1024] = {0};
    memory.load_bios(bios, 1024);
    
    // Load 2KB ROM
    uint8 rom[2048];
    for (int i = 0; i < 2048; i++) {
        rom[i] = static_cast<uint8>((i + 0x42) & 0xFF);
    }
    
    auto result = memory.load_cartridge(rom, 2048);
    EXPECT_TRUE(result.is_ok());
    
    // Verify ROM data is accessible starting at 0x400
    for (int i = 0; i < 2048; i++) {
        uint16 addr = 0x400 + i;
        if (addr < 0x1000) {  // Within addressable range
            EXPECT_EQ(memory.read_program(addr), static_cast<uint8>((i + 0x42) & 0xFF));
        }
    }
}

// ========== BANK SWITCHING TESTS ==========

TEST(MemoryTest, BankSwitching2KB) {
    MemorySystem memory;
    
    uint8 bios[1024] = {0};
    memory.load_bios(bios, 1024);
    
    uint8 rom[2048];
    for (int i = 0; i < 2048; i++) {
        rom[i] = static_cast<uint8>(i & 0xFF);
    }
    memory.load_cartridge(rom, 2048);
    
    // 2KB ROM has only 1 bank
    auto state = memory.get_state();
    EXPECT_EQ(state.num_banks, 1);
    EXPECT_EQ(state.rom_size_kb, 2);
}

TEST(MemoryTest, BankSwitching4KB) {
    MemorySystem memory;
    
    uint8 bios[1024] = {0};
    memory.load_bios(bios, 1024);
    
    uint8 rom[4096];
    for (int i = 0; i < 4096; i++) {
        rom[i] = static_cast<uint8>(i & 0xFF);
    }
    memory.load_cartridge(rom, 4096);
    
    // 4KB ROM has 2 banks
    auto state = memory.get_state();
    EXPECT_EQ(state.num_banks, 2);
    EXPECT_EQ(state.rom_size_kb, 4);
    EXPECT_EQ(state.current_bank, 0);
    
    // Switch to bank 1
    memory.set_bank(1);
    state = memory.get_state();
    EXPECT_EQ(state.current_bank, 1);
}

TEST(MemoryTest, BankSwitching8KB) {
    MemorySystem memory;
    
    uint8 bios[1024] = {0};
    memory.load_bios(bios, 1024);
    
    uint8 rom[8192];
    for (int i = 0; i < 8192; i++) {
        rom[i] = static_cast<uint8>(i & 0xFF);
    }
    memory.load_cartridge(rom, 8192);
    
    // 8KB ROM has 4 banks
    auto state = memory.get_state();
    EXPECT_EQ(state.num_banks, 4);
    EXPECT_EQ(state.rom_size_kb, 8);
    
    // Test switching through all banks
    for (uint8 bank = 0; bank < 4; bank++) {
        memory.set_bank(bank);
        state = memory.get_state();
        EXPECT_EQ(state.current_bank, bank);
    }
}

TEST(MemoryTest, BankSwitchingViaPort1) {
    MemorySystem memory;
    
    uint8 bios[1024] = {0};
    memory.load_bios(bios, 1024);
    
    uint8 rom[4096];
    for (int i = 0; i < 4096; i++) {
        rom[i] = static_cast<uint8>(i & 0xFF);
    }
    memory.load_cartridge(rom, 4096);
    
    // Bank switching via Port 1 pins P10 and P11
    // Bank 0: P10=0, P11=0
    memory.update_control_signals(0x00);
    EXPECT_EQ(memory.get_state().current_bank, 0);
    
    // Bank 1: P10=1, P11=0
    memory.update_control_signals(0x01);
    EXPECT_EQ(memory.get_state().current_bank, 1);
}

// ========== EXTERNAL RAM TESTS ==========

TEST(MemoryTest, ExternalRAMAccess) {
    MemorySystem memory;
    
    // Enable external RAM via Port 1 (P14 = 0)
    memory.update_control_signals(0x00);  // All pins low
    
    // Write to external RAM
    for (uint8 addr = 0; addr < 128; addr++) {
        memory.write_external(addr, addr + 0x10);
    }
    
    // Read back from external RAM
    for (uint8 addr = 0; addr < 128; addr++) {
        EXPECT_EQ(memory.read_external(addr), addr + 0x10);
    }
}

TEST(MemoryTest, ExternalRAMDisabled) {
    MemorySystem memory;
    
    // Disable external RAM via Port 1 (P14 = 1)
    memory.update_control_signals(0x10);  // P14 high
    
    // Try to write to external RAM (should not work)
    memory.write_external(0x00, 0x42);
    
    // Enable RAM and check it wasn't written
    memory.update_control_signals(0x00);
    EXPECT_NE(memory.read_external(0x00), 0x42);
}

// ========== MEMORY ACCESS CONTROL TESTS ==========

TEST(MemoryTest, Port1ControlSignals) {
    MemorySystem memory;
    
    // Test P13 (VDC enable) - bit 3
    // P13 = 0: VDC enabled
    memory.update_control_signals(0x00);
    // VDC would be accessible here (tested with actual VDC)
    
    // P13 = 1: VDC disabled
    memory.update_control_signals(0x08);
    // VDC would not be accessible
    
    // Test P14 (RAM enable) - bit 4
    // P14 = 0: RAM enabled
    memory.update_control_signals(0x00);
    memory.write_external(0x00, 0x42);
    EXPECT_EQ(memory.read_external(0x00), 0x42);
    
    // P14 = 1: RAM disabled
    memory.update_control_signals(0x10);
    memory.write_external(0x01, 0x99);
    memory.update_control_signals(0x00);
    EXPECT_NE(memory.read_external(0x01), 0x99);
}

// ========== STATE MANAGEMENT TESTS ==========

TEST(MemoryTest, GetSetState) {
    MemorySystem memory;
    
    // Load some data
    uint8 bios[1024];
    for (int i = 0; i < 1024; i++) {
        bios[i] = static_cast<uint8>(i & 0xFF);
    }
    memory.load_bios(bios, 1024);
    
    uint8 rom[2048];
    for (int i = 0; i < 2048; i++) {
        rom[i] = static_cast<uint8>((i + 0x42) & 0xFF);
    }
    memory.load_cartridge(rom, 2048);
    
    // Write to external RAM
    memory.update_control_signals(0x00);
    for (uint8 i = 0; i < 128; i++) {
        memory.write_external(i, i + 0x10);
    }
    
    // Get state
    auto state = memory.get_state();
    
    // Create new memory system and restore state
    MemorySystem memory2;
    memory2.set_state(state);
    
    // Verify BIOS
    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(memory2.read_program(i), static_cast<uint8>(i & 0xFF));
    }
    
    // Verify ROM
    for (int i = 0; i < 2048; i++) {
        uint16 addr = 0x400 + i;
        if (addr < 0x1000) {
            EXPECT_EQ(memory2.read_program(addr), static_cast<uint8>((i + 0x42) & 0xFF));
        }
    }
    
    // Verify external RAM
    memory2.update_control_signals(0x00);
    for (uint8 i = 0; i < 128; i++) {
        EXPECT_EQ(memory2.read_external(i), i + 0x10);
    }
}
