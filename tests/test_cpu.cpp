#include <gtest/gtest.h>
#include "cpu.h"
#include "memory.h"
#include <cstring>

using namespace videopac;

class CPUTest : public ::testing::Test {
protected:
    void SetUp() override {
        cpu = std::make_unique<CPU>();
        memory = std::make_unique<MemorySystem>();
        cpu->set_memory_system(memory.get());
        
        // Load a simple test program into memory
        test_program.resize(4096, 0x00);  // Fill with NOP
    }
    
    // Helper to load program into memory
    void load_program(const std::vector<uint8>& program) {
        // Create a 1KB BIOS image filled with NOPs
        uint8 bios[1024];
        std::memset(bios, 0x00, 1024);  // Fill with NOP
        
        // Copy program into the beginning
        for (size_t i = 0; i < program.size() && i < 1024; ++i) {
            bios[i] = program[i];
        }
        
        // Load into memory system
        memory->load_bios(bios, 1024);
    }
    
    // Helper to get CPU state
    CPUState get_state() {
        return cpu->get_state();
    }
    
    std::unique_ptr<CPU> cpu;
    std::unique_ptr<MemorySystem> memory;
    std::vector<uint8> test_program;
};

// ========== BASIC TESTS ==========

TEST_F(CPUTest, ResetInitializesState) {
    cpu->reset();
    EXPECT_EQ(cpu->get_pc(), 0x000);
    EXPECT_EQ(cpu->get_accumulator(), 0);
}

TEST_F(CPUTest, NOP_DoesNothing) {
    load_program({0x00});  // NOP
    cpu->reset();
    
    uint16 pc_before = cpu->get_pc();
    uint8 cycles = cpu->execute_instruction();
    
    EXPECT_EQ(cycles, 1);
    EXPECT_EQ(cpu->get_pc(), pc_before + 1);
}

TEST_F(CPUTest, Debug_CheckMemoryLoading) {
    // Load a simple MOV A,#0x42 instruction
    load_program({0x23, 0x42});
    cpu->reset();
    
    // Check that memory was loaded correctly
    EXPECT_EQ(memory->read_program(0x000), 0x23);
    EXPECT_EQ(memory->read_program(0x001), 0x42);
    
    // Execute the instruction
    cpu->execute_instruction();
    
    // Check accumulator
    EXPECT_EQ(cpu->get_accumulator(), 0x42);
}

// ========== ARITHMETIC TESTS ==========

TEST_F(CPUTest, ADD_Immediate) {
    load_program({
        0x23, 0x42,  // MOV A,#0x42
        0x03, 0x10   // ADD A,#0x10
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0x42
    cpu->execute_instruction();  // ADD A,#0x10
    
    EXPECT_EQ(cpu->get_accumulator(), 0x52);
}

TEST_F(CPUTest, ADD_SetsCarryFlag) {
    load_program({
        0x23, 0xFF,  // MOV A,#0xFF
        0x03, 0x02   // ADD A,#0x02
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0xFF
    cpu->execute_instruction();  // ADD A,#0x02
    
    EXPECT_EQ(cpu->get_accumulator(), 0x01);
    auto state = get_state();
    EXPECT_TRUE(state.psw & 0x80);  // Carry flag set
}

TEST_F(CPUTest, ADD_SetsAuxiliaryCarryFlag) {
    load_program({
        0x23, 0x0F,  // MOV A,#0x0F
        0x03, 0x01   // ADD A,#0x01
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0x0F
    cpu->execute_instruction();  // ADD A,#0x01
    
    EXPECT_EQ(cpu->get_accumulator(), 0x10);
    auto state = get_state();
    EXPECT_TRUE(state.psw & 0x40);  // Auxiliary carry flag set
}

TEST_F(CPUTest, ADDC_AddsWithCarry) {
    load_program({
        0x23, 0x10,  // MOV A,#0x10
        0x03, 0xFF,  // ADD A,#0xFF  (sets carry)
        0x13, 0x00   // ADDC A,#0x00 (adds carry)
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0x10
    cpu->execute_instruction();  // ADD A,#0xFF
    cpu->execute_instruction();  // ADDC A,#0x00
    
    EXPECT_EQ(cpu->get_accumulator(), 0x10);  // 0x0F + 0x00 + 1(carry)
}

TEST_F(CPUTest, INC_Accumulator) {
    load_program({
        0x23, 0x42,  // MOV A,#0x42
        0x17         // INC A
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0x42
    cpu->execute_instruction();  // INC A
    
    EXPECT_EQ(cpu->get_accumulator(), 0x43);
}

TEST_F(CPUTest, DEC_Accumulator) {
    load_program({
        0x23, 0x42,  // MOV A,#0x42
        0x07         // DEC A
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0x42
    cpu->execute_instruction();  // DEC A
    
    EXPECT_EQ(cpu->get_accumulator(), 0x41);
}

TEST_F(CPUTest, DA_DecimalAdjust) {
    load_program({
        0x23, 0x09,  // MOV A,#0x09
        0x03, 0x08,  // ADD A,#0x08  (result = 0x11, should adjust to 0x17 BCD)
        0x57         // DA A
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0x09
    cpu->execute_instruction();  // ADD A,#0x08
    cpu->execute_instruction();  // DA A
    
    EXPECT_EQ(cpu->get_accumulator(), 0x17);  // BCD result
}

// ========== LOGICAL TESTS ==========

TEST_F(CPUTest, ANL_Immediate) {
    load_program({
        0x23, 0xFF,  // MOV A,#0xFF
        0x53, 0x0F   // ANL A,#0x0F
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0xFF
    cpu->execute_instruction();  // ANL A,#0x0F
    
    EXPECT_EQ(cpu->get_accumulator(), 0x0F);
}

TEST_F(CPUTest, ORL_Immediate) {
    load_program({
        0x23, 0x0F,  // MOV A,#0x0F
        0x43, 0xF0   // ORL A,#0xF0
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0x0F
    cpu->execute_instruction();  // ORL A,#0xF0
    
    EXPECT_EQ(cpu->get_accumulator(), 0xFF);
}

TEST_F(CPUTest, XRL_Immediate) {
    load_program({
        0x23, 0xFF,  // MOV A,#0xFF
        0xD3, 0xAA   // XRL A,#0xAA
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0xFF
    cpu->execute_instruction();  // XRL A,#0xAA
    
    EXPECT_EQ(cpu->get_accumulator(), 0x55);
}

TEST_F(CPUTest, CLR_Accumulator) {
    load_program({
        0x23, 0xFF,  // MOV A,#0xFF
        0x27         // CLR A
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0xFF
    cpu->execute_instruction();  // CLR A
    
    EXPECT_EQ(cpu->get_accumulator(), 0x00);
}

TEST_F(CPUTest, CPL_Accumulator) {
    load_program({
        0x23, 0xAA,  // MOV A,#0xAA
        0x37         // CPL A
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0xAA
    cpu->execute_instruction();  // CPL A
    
    EXPECT_EQ(cpu->get_accumulator(), 0x55);
}

// ========== DATA TRANSFER TESTS ==========

TEST_F(CPUTest, MOV_ImmediateToAccumulator) {
    load_program({0x23, 0x42});  // MOV A,#0x42
    cpu->reset();
    
    cpu->execute_instruction();
    
    EXPECT_EQ(cpu->get_accumulator(), 0x42);
}

TEST_F(CPUTest, MOV_AccumulatorToRegister) {
    load_program({
        0x23, 0x42,  // MOV A,#0x42
        0xA8         // MOV R0,A
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0x42
    cpu->execute_instruction();  // MOV R0,A
    
    auto state = get_state();
    EXPECT_EQ(state.r[0], 0x42);
}

TEST_F(CPUTest, MOV_RegisterToAccumulator) {
    load_program({
        0xB8, 0x42,  // MOV R0,#0x42
        0xF8         // MOV A,R0
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV R0,#0x42
    cpu->execute_instruction();  // MOV A,R0
    
    EXPECT_EQ(cpu->get_accumulator(), 0x42);
}

TEST_F(CPUTest, XCH_AccumulatorRegister) {
    load_program({
        0x23, 0xAA,  // MOV A,#0xAA
        0xB8, 0x55,  // MOV R0,#0x55
        0x28         // XCH A,R0
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0xAA
    cpu->execute_instruction();  // MOV R0,#0x55
    cpu->execute_instruction();  // XCH A,R0
    
    EXPECT_EQ(cpu->get_accumulator(), 0x55);
    auto state = get_state();
    EXPECT_EQ(state.r[0], 0xAA);
}

TEST_F(CPUTest, SWAP_Nibbles) {
    load_program({
        0x23, 0x12,  // MOV A,#0x12
        0x47         // SWAP A
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0x12
    cpu->execute_instruction();  // SWAP A
    
    EXPECT_EQ(cpu->get_accumulator(), 0x21);
}

// ========== BRANCH TESTS ==========

TEST_F(CPUTest, JMP_Unconditional) {
    load_program({
        0x04, 0x10,  // JMP 0x010
        0x00,        // (skipped)
        0x00         // (skipped)
    });
    cpu->reset();
    
    cpu->execute_instruction();  // JMP 0x010
    
    EXPECT_EQ(cpu->get_pc(), 0x010);
}

TEST_F(CPUTest, JZ_JumpsWhenZero) {
    load_program({
        0x27,        // CLR A
        0xC6, 0x10   // JZ 0x010
    });
    cpu->reset();
    
    cpu->execute_instruction();  // CLR A
    cpu->execute_instruction();  // JZ 0x010
    
    EXPECT_EQ(cpu->get_pc(), 0x010);
}

TEST_F(CPUTest, JZ_DoesNotJumpWhenNotZero) {
    load_program({
        0x23, 0x01,  // MOV A,#0x01
        0xC6, 0x10   // JZ 0x010
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0x01
    uint16 pc_before = cpu->get_pc();
    cpu->execute_instruction();  // JZ 0x010
    
    EXPECT_EQ(cpu->get_pc(), pc_before + 2);  // Did not jump
}

TEST_F(CPUTest, JC_JumpsWhenCarrySet) {
    load_program({
        0x23, 0xFF,  // MOV A,#0xFF
        0x03, 0x02,  // ADD A,#0x02  (sets carry)
        0xF6, 0x10   // JC 0x010
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0xFF
    cpu->execute_instruction();  // ADD A,#0x02
    cpu->execute_instruction();  // JC 0x010
    
    EXPECT_EQ(cpu->get_pc(), 0x010);
}

TEST_F(CPUTest, DJNZ_DecrementsAndJumps) {
    load_program({
        0xB8, 0x03,  // MOV R0,#0x03
        0xE8, 0x02   // DJNZ R0,0x002
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV R0,#0x03
    cpu->execute_instruction();  // DJNZ R0,0x002
    
    auto state = get_state();
    EXPECT_EQ(state.r[0], 0x02);
    EXPECT_EQ(cpu->get_pc(), 0x002);  // Jumped because R0 != 0
}

TEST_F(CPUTest, DJNZ_DoesNotJumpWhenZero) {
    load_program({
        0xB8, 0x01,  // MOV R0,#0x01
        0xE8, 0x02   // DJNZ R0,0x002
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV R0,#0x01
    uint16 pc_before = cpu->get_pc();
    cpu->execute_instruction();  // DJNZ R0,0x002
    
    auto state = get_state();
    EXPECT_EQ(state.r[0], 0x00);
    EXPECT_EQ(cpu->get_pc(), pc_before + 2);  // Did not jump
}

// ========== SUBROUTINE TESTS ==========

TEST_F(CPUTest, CALL_PushesPC) {
    load_program({
        0x14, 0x20   // CALL 0x020
    });
    cpu->reset();
    
    cpu->execute_instruction();  // CALL 0x020
    
    EXPECT_EQ(cpu->get_pc(), 0x020);
    auto state = get_state();
    EXPECT_EQ(state.sp, 1);  // Stack pointer incremented by 1 (PC + PSW combined in single 16-bit entry)
}

TEST_F(CPUTest, RET_PopsPC) {
    load_program({
        0x14, 0x20,  // CALL 0x020
        0x00,        // (return here)
        0x00
    });
    // Put RET at address 0x020
    uint8 bios[1024];
    std::memset(bios, 0x00, 1024);
    bios[0] = 0x14;  // CALL
    bios[1] = 0x20;
    bios[0x020] = 0x83;  // RET
    memory->load_bios(bios, 1024);
    
    cpu->reset();
    cpu->execute_instruction();  // CALL 0x020
    cpu->execute_instruction();  // RET
    
    EXPECT_EQ(cpu->get_pc(), 0x002);  // Returned to after CALL
    auto state = get_state();
    // CALL pushes 1 value (PC + PSW combined), RET pops 1 value (extracts PC), so SP = 0
    EXPECT_EQ(state.sp, 0);
}

// ========== FLAG TESTS ==========

TEST_F(CPUTest, CLR_C_ClearsCarryFlag) {
    load_program({
        0x23, 0xFF,  // MOV A,#0xFF
        0x03, 0x02,  // ADD A,#0x02  (sets carry)
        0x97         // CLR C
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0xFF
    cpu->execute_instruction();  // ADD A,#0x02
    cpu->execute_instruction();  // CLR C
    
    auto state = get_state();
    EXPECT_FALSE(state.psw & 0x80);  // Carry flag cleared
}

TEST_F(CPUTest, CPL_C_ComplementsCarryFlag) {
    load_program({
        0x97,  // CLR C
        0xA7   // CPL C
    });
    cpu->reset();
    
    cpu->execute_instruction();  // CLR C
    cpu->execute_instruction();  // CPL C
    
    auto state = get_state();
    EXPECT_TRUE(state.psw & 0x80);  // Carry flag set
}

// ========== CONTROL TESTS ==========

TEST_F(CPUTest, SEL_RB0_SelectsRegisterBank0) {
    load_program({0xC5});  // SEL RB0
    cpu->reset();
    
    cpu->execute_instruction();
    
    auto state = get_state();
    EXPECT_EQ(state.current_bank, 0);
}

TEST_F(CPUTest, SEL_RB1_SelectsRegisterBank1) {
    load_program({0xD5});  // SEL RB1
    cpu->reset();
    
    cpu->execute_instruction();
    
    auto state = get_state();
    EXPECT_EQ(state.current_bank, 1);
}

TEST_F(CPUTest, EN_I_EnablesInterrupts) {
    load_program({0x05});  // EN I
    cpu->reset();
    
    cpu->execute_instruction();
    
    auto state = get_state();
    EXPECT_TRUE(state.interrupts_enabled);
}

TEST_F(CPUTest, DIS_I_DisablesInterrupts) {
    load_program({
        0x05,  // EN I
        0x15   // DIS I
    });
    cpu->reset();
    
    cpu->execute_instruction();  // EN I
    cpu->execute_instruction();  // DIS I
    
    auto state = get_state();
    EXPECT_FALSE(state.interrupts_enabled);
}

// ========== ROTATE TESTS ==========

TEST_F(CPUTest, RR_RotateRight) {
    load_program({
        0x23, 0x81,  // MOV A,#0x81
        0x77         // RR A
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0x81
    cpu->execute_instruction();  // RR A
    
    EXPECT_EQ(cpu->get_accumulator(), 0xC0);  // 10000001 -> 11000000
}

TEST_F(CPUTest, RL_RotateLeft) {
    load_program({
        0x23, 0x81,  // MOV A,#0x81
        0xE7         // RL A
    });
    cpu->reset();
    
    cpu->execute_instruction();  // MOV A,#0x81
    cpu->execute_instruction();  // RL A
    
    EXPECT_EQ(cpu->get_accumulator(), 0x03);  // 10000001 -> 00000011
}

TEST_F(CPUTest, RRC_RotateRightThroughCarry) {
    load_program({
        0x97,        // CLR C
        0x23, 0x80,  // MOV A,#0x80
        0x67         // RRC A
    });
    cpu->reset();
    
    cpu->execute_instruction();  // CLR C
    cpu->execute_instruction();  // MOV A,#0x80
    cpu->execute_instruction();  // RRC A
    
    EXPECT_EQ(cpu->get_accumulator(), 0x40);  // 10000000 -> 01000000 (C was 0)
    auto state = get_state();
    EXPECT_FALSE(state.psw & 0x80);  // Carry still clear
}

TEST_F(CPUTest, RLC_RotateLeftThroughCarry) {
    load_program({
        0x97,        // CLR C
        0x23, 0x80,  // MOV A,#0x80
        0xF7         // RLC A
    });
    cpu->reset();
    
    cpu->execute_instruction();  // CLR C
    cpu->execute_instruction();  // MOV A,#0x80
    cpu->execute_instruction();  // RLC A
    
    EXPECT_EQ(cpu->get_accumulator(), 0x00);  // 10000000 -> 00000000 (bit 7 went to carry)
    auto state = get_state();
    EXPECT_TRUE(state.psw & 0x80);  // Carry set from bit 7
}


// Test timer control instructions
TEST_F(CPUTest, TimerControl) {
    // Test STRT T (Start Timer) - opcode 0x55
    load_program({0x55});
    cpu->reset();
    
    // Get state before starting timer
    CPUState state = get_state();
    state.timer_on = false;
    state.counter_on = false;
    state.timer_prescaler = 0;
    cpu->set_state(state);
    
    cpu->execute_instruction();
    state = get_state();
    EXPECT_TRUE(state.timer_on);
    EXPECT_FALSE(state.counter_on);
    // Note: prescaler will be 1 because STRT T clears it to 0, then the instruction
    // consumes 1 cycle which increments it to 1
    EXPECT_EQ(state.timer_prescaler, 1);
    
    // Test STOP TCNT (Stop Timer) - opcode 0x65
    load_program({0x55, 0x65});
    cpu->reset();
    cpu->execute_instruction();  // STRT T
    cpu->execute_instruction();  // STOP TCNT
    state = get_state();
    EXPECT_FALSE(state.timer_on);
    EXPECT_FALSE(state.counter_on);
    
    // Test STRT CNT (Start Event Counter) - opcode 0x45
    load_program({0x45});
    cpu->reset();
    
    state = get_state();
    state.timer_on = false;
    state.counter_on = false;
    state.timer_prescaler = 0;
    cpu->set_state(state);
    
    cpu->execute_instruction();
    state = get_state();
    EXPECT_TRUE(state.counter_on);
    EXPECT_FALSE(state.timer_on);
    // Counter mode doesn't use prescaler (increments per scanline, not per cycle)
    EXPECT_EQ(state.timer_prescaler, 0);
}

// Test timer increment logic
TEST_F(CPUTest, TimerIncrement) {
    // Load 100 NOP instructions
    std::vector<uint8> program(100, 0x00);
    load_program(program);
    
    cpu->reset();
    CPUState state = get_state();
    state.timer = 0x00;
    state.timer_on = true;
    state.counter_on = false;
    state.timer_prescaler = 0;
    cpu->set_state(state);
    
    // Execute 32 NOPs (32 cycles) - timer should increment once
    for (int i = 0; i < 32; i++) {
        cpu->execute_instruction();
    }
    
    state = get_state();
    EXPECT_EQ(state.timer, 0x01);  // Timer should have incremented
    
    // Execute 32 more NOPs - timer should increment again
    for (int i = 0; i < 32; i++) {
        cpu->execute_instruction();
    }
    
    state = get_state();
    EXPECT_EQ(state.timer, 0x02);
}

// Test timer overflow
TEST_F(CPUTest, TimerOverflow) {
    // Load 100 NOP instructions
    std::vector<uint8> program(100, 0x00);
    load_program(program);
    
    cpu->reset();
    CPUState state = get_state();
    state.timer = 0xFF;  // Set timer to maximum value
    state.timer_on = true;
    state.counter_on = false;
    state.timer_prescaler = 0;
    state.timer_interrupts_enabled = false;  // Disable interrupts for this test
    cpu->set_state(state);
    
    // Execute 32 NOPs - timer should overflow from 0xFF to 0x00
    for (int i = 0; i < 32; i++) {
        cpu->execute_instruction();
    }
    
    state = get_state();
    EXPECT_EQ(state.timer, 0x00);  // Timer should have overflowed to 0
}
