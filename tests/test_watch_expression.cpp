#include <gtest/gtest.h>
#include "ui/imgui_debugger_ui.h"
#include "cpu.h"
#include "memory.h"

using namespace videopac;

class WatchExpressionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize CPU state with test values
        cpu_state.pc = 0x0234;
        cpu_state.a = 0x5A;
        cpu_state.psw = 0xB4;
        cpu_state.sp = 3;
        cpu_state.current_bank = 0;
        cpu_state.f1_flag = false;
        cpu_state.memory_bank = false;
        cpu_state.port1 = 0x12;
        cpu_state.port2 = 0x34;
        cpu_state.timer = 0x56;
        
        // Initialize registers R0-R7
        for (int i = 0; i < 8; i++) {
            cpu_state.r[i] = 0x10 + i;  // R0=0x10, R1=0x11, etc.
        }
        
        // Initialize CPU internal RAM with test pattern
        for (int i = 0; i < 64; i++) {
            cpu_state.ram[i] = i;
        }
        
        // Initialize memory state with test values
        for (int i = 0; i < 128; i++) {
            memory_state.external_ram[i] = 0x80 + i;
        }
        
        for (int i = 0; i < 1024; i++) {
            memory_state.bios_rom[i] = 0xFF - (i % 256);
        }
        
        memory_state.cart_rom.resize(2048);
        for (size_t i = 0; i < memory_state.cart_rom.size(); i++) {
            memory_state.cart_rom[i] = (i & 0xFF);
        }
    }
    
    CPUState cpu_state;
    MemoryState memory_state;
};

// ========== REGISTER TESTS ==========

TEST_F(WatchExpressionTest, EvaluateAccumulator) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::Register;
    watch.expression = "A";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x5A);
}

TEST_F(WatchExpressionTest, EvaluateAccumulatorLowercase) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::Register;
    watch.expression = "a";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x5A);
}

TEST_F(WatchExpressionTest, EvaluateProgramCounter) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::Register;
    watch.expression = "PC";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x0234);
}

TEST_F(WatchExpressionTest, EvaluatePSW) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::Register;
    watch.expression = "PSW";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0xB4);
}

TEST_F(WatchExpressionTest, EvaluateStackPointer) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::Register;
    watch.expression = "SP";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 3);
}

TEST_F(WatchExpressionTest, EvaluateRegisterR0) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::Register;
    watch.expression = "R0";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x10);
}

TEST_F(WatchExpressionTest, EvaluateRegisterR7) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::Register;
    watch.expression = "R7";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x17);
}

TEST_F(WatchExpressionTest, EvaluateRegisterLowercase) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::Register;
    watch.expression = "r3";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x13);
}

TEST_F(WatchExpressionTest, EvaluatePort1) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::Register;
    watch.expression = "P1";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x12);
}

TEST_F(WatchExpressionTest, EvaluatePort2) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::Register;
    watch.expression = "P2";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x34);
}

TEST_F(WatchExpressionTest, EvaluateTimer) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::Register;
    watch.expression = "TIMER";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x56);
}

TEST_F(WatchExpressionTest, EvaluateInvalidRegister) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::Register;
    watch.expression = "INVALID";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0);
}

// ========== MEMORY ADDRESS TESTS ==========

TEST_F(WatchExpressionTest, EvaluateMemoryAddressInternalRAM) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::MemoryAddress;
    watch.expression = "0x0010";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x10);  // cpu_state.ram[0x10] = 0x10
}

TEST_F(WatchExpressionTest, EvaluateMemoryAddressExternalRAM) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::MemoryAddress;
    watch.expression = "0x0050";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x90);  // memory_state.external_ram[0x10] = 0x90
}

TEST_F(WatchExpressionTest, EvaluateMemoryAddressBIOSROM) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::MemoryAddress;
    watch.expression = "0x0100";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0xFF);  // memory_state.bios_rom[0x100] = 0xFF
}

TEST_F(WatchExpressionTest, EvaluateMemoryAddressCartridgeROM) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::MemoryAddress;
    watch.expression = "0x0500";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x00);  // memory_state.cart_rom[0x100] = 0x00
}

TEST_F(WatchExpressionTest, EvaluateMemoryAddressWithoutPrefix) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::MemoryAddress;
    watch.expression = "20";  // Hex 0x20
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x20);  // cpu_state.ram[0x20] = 0x20
}

TEST_F(WatchExpressionTest, EvaluateMemoryAddressUppercaseX) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::MemoryAddress;
    watch.expression = "0X0015";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0x15);  // cpu_state.ram[0x15] = 0x15
}

TEST_F(WatchExpressionTest, EvaluateMemoryAddressUppercaseHex) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::MemoryAddress;
    watch.expression = "0x00AB";
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0xEB);  // memory_state.external_ram[0x6B] = 0x80 + 0x6B = 0xEB
}

TEST_F(WatchExpressionTest, EvaluateMemoryAddressInvalid) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::MemoryAddress;
    watch.expression = "0xGGGG";  // Invalid hex
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0);
}

TEST_F(WatchExpressionTest, EvaluateMemoryAddressOutOfRange) {
    WatchExpression watch;
    watch.type = WatchExpression::Type::MemoryAddress;
    watch.expression = "0xFFFF";  // Out of range
    
    uint16 value = watch.evaluate(cpu_state, memory_state);
    EXPECT_EQ(value, 0);
}
