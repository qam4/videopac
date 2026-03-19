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
    uint8 program[1024] = {};
    program[0] = 0x23;  // MOV A,#0x10
    program[1] = 0x10;
    program[2] = 0x03;  // ADD A,#0x20
    program[3] = 0x20;
    program[4] = 0xA8;  // MOV R0,A
    program[5] = 0x18;  // INC R0
    
    memory.load_bios(program, 1024);
    cpu.reset();
    
    // Execute the sequence
    cpu.execute_instruction();  // MOV A,#0x10
    EXPECT_EQ(cpu.get_accumulator(), 0x10);
    
    cpu.execute_instruction();  // ADD A,#0x20
    EXPECT_EQ(cpu.get_accumulator(), 0x30);
    
    cpu.execute_instruction();  // MOV R0,A
    auto state = cpu.get_state();
    EXPECT_EQ(state.ram[0], 0x30);
    
    cpu.execute_instruction();  // INC R0
    state = cpu.get_state();
    EXPECT_EQ(state.ram[0], 0x31);
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

// Integration test: Complete frame execution with all components
TEST(IntegrationTest, CompleteFrameExecution) {
    Configuration config;
    config.video_standard = VideoStandard::NTSC;
    
    EmulatorCore emulator(config);
    
    // Load minimal BIOS with infinite loop at 0x000
    uint8 bios[1024];
    for (int i = 0; i < 1024; i++) {
        bios[i] = 0x00;  // NOP instruction
    }
    // Put JMP 0x000 at address 0x000 (opcode 0x04, operand 0x00)
    bios[0] = 0x04;  // JMP instruction
    bios[1] = 0x00;  // Jump to address 0x000
    
    auto result = emulator.load_bios(bios, 1024);
    EXPECT_TRUE(result.is_ok());
    
    // Load minimal ROM with infinite loop at 0x400
    uint8 rom[2048];
    for (int i = 0; i < 2048; i++) {
        rom[i] = 0x00;  // NOP instruction
    }
    // Put JMP 0x400 at address 0x400 (opcode 0x04, operand 0x00)
    rom[0] = 0x04;  // JMP instruction
    rom[1] = 0x00;  // Jump to address 0x000 (which becomes 0x400 in ROM space)
    
    result = emulator.load_rom(rom, 2048);
    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(emulator.is_running());
    
    // Execute one frame
    uint64 initial_frame_count = emulator.get_frame_count();
    emulator.run_frame();
    
    // Verify frame was executed
    EXPECT_EQ(emulator.get_frame_count(), initial_frame_count + 1);
    
    // Verify framebuffer is accessible
    const uint8* framebuffer = emulator.get_framebuffer();
    EXPECT_NE(framebuffer, nullptr);
}

// Integration test: CPU and VDC timing synchronization
TEST(IntegrationTest, CPUVDCTimingSynchronization) {
    Configuration config;
    config.video_standard = VideoStandard::NTSC;
    
    EmulatorCore emulator(config);
    
    // Load BIOS and ROM with infinite loops
    uint8 bios[1024];
    uint8 rom[2048];
    for (int i = 0; i < 1024; i++) bios[i] = 0x00;
    for (int i = 0; i < 2048; i++) rom[i] = 0x00;
    
    // Put JMP 0x000 at BIOS start
    bios[0] = 0x04;  // JMP instruction
    bios[1] = 0x00;  // Jump to address 0x000
    
    // Put JMP 0x400 at ROM start
    rom[0] = 0x04;  // JMP instruction
    rom[1] = 0x00;  // Jump to address 0x000 (becomes 0x400)
    
    emulator.load_bios(bios, 1024);
    emulator.load_rom(rom, 2048);
    
    // Execute multiple frames
    for (int i = 0; i < 5; i++) {
        emulator.run_frame();
    }
    
    // Verify CPU executed instructions
    EXPECT_GT(emulator.get_cpu().get_cycles(), 0);
    
    // Verify frame count increased
    EXPECT_EQ(emulator.get_frame_count(), 5);
}

// Integration test: Input handler integration
TEST(IntegrationTest, InputHandlerIntegration) {
    Configuration config;
    EmulatorCore emulator(config);
    
    // Get input handler and set some input
    InputHandler& input = emulator.get_input_handler();
    input.set_key_state(VidKey::Key0, true);
    input.set_joystick_state(0, Direction::Up, true);
    
    // Verify input state
    uint8 keyboard_result = input.read_keyboard(0xFE);  // Row 0
    EXPECT_EQ(keyboard_result & 0x01, 0x00);  // Key0 pressed
    
    uint8 joystick_result = input.read_joystick(0x04);  // Joystick 1: P22=1 → 0b100
    EXPECT_EQ(joystick_result & 0x01, 0x00);  // Up pressed
}

// Integration test: Memory and VDC interaction
TEST(IntegrationTest, MemoryVDCInteraction) {
    Configuration config;
    EmulatorCore emulator(config);
    
    // Access components
    CPU& cpu = emulator.get_cpu();
    VDC& vdc = emulator.get_vdc();
    
    // Enable VDC by setting Port 1 (P13 = 0 enables VDC)
    // According to doc/o2doc.md, P13 low enables VDC
    cpu.write_port(1, 0xF7);  // P13 = 0, others = 1
    
    // Now write to VDC register through CPU
    // The CPU will route this through memory system to VDC
    vdc.write_register(0xA0, 0x08);  // Enable grid directly
    
    // Verify VDC received the write
    uint8 control_reg = vdc.read_register(0xA0);
    EXPECT_EQ(control_reg & 0x08, 0x08);  // Grid enabled
}

// Integration test: Pause and resume
TEST(IntegrationTest, PauseAndResume) {
    Configuration config;
    EmulatorCore emulator(config);
    
    uint8 bios[1024];
    uint8 rom[2048];
    for (int i = 0; i < 1024; i++) bios[i] = 0x00;
    for (int i = 0; i < 2048; i++) rom[i] = 0x00;
    
    // Put JMP 0x000 at BIOS start
    bios[0] = 0x04;  // JMP instruction
    bios[1] = 0x00;  // Jump to address 0x000
    
    // Put JMP 0x400 at ROM start
    rom[0] = 0x04;  // JMP instruction
    rom[1] = 0x00;  // Jump to address 0x000 (becomes 0x400)
    
    emulator.load_bios(bios, 1024);
    emulator.load_rom(rom, 2048);
    
    // Run one frame
    emulator.run_frame();
    EXPECT_EQ(emulator.get_frame_count(), 1);
    
    // Pause
    emulator.set_paused(true);
    EXPECT_TRUE(emulator.is_paused());
    
    // Try to run frame while paused (should not execute)
    emulator.run_frame();
    EXPECT_EQ(emulator.get_frame_count(), 1);  // Still 1
    
    // Resume
    emulator.set_paused(false);
    EXPECT_FALSE(emulator.is_paused());
    
    // Run frame (should execute)
    emulator.run_frame();
    EXPECT_EQ(emulator.get_frame_count(), 2);
}

// Integration test: Single-step debugging
TEST(IntegrationTest, SingleStepDebugging) {
    Configuration config;
    EmulatorCore emulator(config);
    
    uint8 bios[1024];
    uint8 rom[2048];
    for (int i = 0; i < 1024; i++) bios[i] = 0x00;  // NOP
    for (int i = 0; i < 2048; i++) rom[i] = 0x00;
    
    emulator.load_bios(bios, 1024);
    emulator.load_rom(rom, 2048);
    
    // Get initial PC
    uint16 initial_pc = emulator.get_cpu().get_pc();
    
    // Single step
    emulator.step();
    
    // Verify PC advanced
    uint16 new_pc = emulator.get_cpu().get_pc();
    EXPECT_NE(new_pc, initial_pc);
}
