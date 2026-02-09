#include <gtest/gtest.h>
#include "emulator.h"
#include "cpu.h"
#include "memory.h"

using namespace videopac;

TEST(IntegrationTest, EmulatorCreation) {
    Configuration config;
    config.video_standard = VideoStandard::NTSC;
    
    EmulatorCore emulator(config);
    EXPECT_FALSE(emulator.is_running());
}

// Checkpoint test: Verify CPU can execute basic instruction sequences
TEST(IntegrationTest, CPUExecutesInstructionSequence) {
    CPU cpu;
    MemorySystem memory;
    cpu.set_memory_system(&memory);
    
    // Create a simple program:
    // MOV A,#0x10    ; Load 0x10 into accumulator
    // ADD A,#0x20    ; Add 0x20 to accumulator (result: 0x30)
    // MOV R0,A       ; Move accumulator to R0
    // INC R0         ; Increment R0 (result: 0x31)
    uint8 program[] = {
        0x23, 0x10,    // MOV A,#0x10
        0x03, 0x20,    // ADD A,#0x20
        0xA8,          // MOV R0,A
        0x18           // INC R0
    };
    
    memory.load_bios(program, 1024);
    cpu.reset();
    
    // Execute the sequence
    cpu.execute_instruction();  // MOV A,#0x10
    EXPECT_EQ(cpu.get_accumulator(), 0x10);
    
    cpu.execute_instruction();  // ADD A,#0x20
    EXPECT_EQ(cpu.get_accumulator(), 0x30);
    
    cpu.execute_instruction();  // MOV R0,A
    auto state = cpu.get_state();
    EXPECT_EQ(state.r[0], 0x30);
    
    cpu.execute_instruction();  // INC R0
    state = cpu.get_state();
    EXPECT_EQ(state.r[0], 0x31);
}

// Checkpoint test: Verify memory system loads ROMs and handles banking
TEST(IntegrationTest, MemorySystemLoadsAndBanks) {
    MemorySystem memory;
    
    // Load BIOS
    uint8 bios[1024];
    for (int i = 0; i < 1024; i++) {
        bios[i] = static_cast<uint8>(i & 0xFF);
    }
    auto result = memory.load_bios(bios, 1024);
    EXPECT_TRUE(result.is_ok());
    
    // Verify BIOS is accessible
    EXPECT_EQ(memory.read_program(0x000), 0x00);
    EXPECT_EQ(memory.read_program(0x100), 0x00);  // Wraps at 256
    EXPECT_EQ(memory.read_program(0x3FF), 0xFF);
    
    // Load 4KB ROM (2 banks)
    uint8 rom[4096];
    for (int i = 0; i < 4096; i++) {
        rom[i] = static_cast<uint8>((i >> 8) & 0xFF);  // Bank number in high byte
    }
    result = memory.load_cartridge(rom, 4096);
    EXPECT_TRUE(result.is_ok());
    
    // Verify banking
    auto state = memory.get_state();
    EXPECT_EQ(state.num_banks, 2);
    EXPECT_EQ(state.current_bank, 0);
    
    // Switch to bank 1
    memory.set_bank(1);
    state = memory.get_state();
    EXPECT_EQ(state.current_bank, 1);
}

// TODO: Add more integration tests
// This will be implemented in task 8
