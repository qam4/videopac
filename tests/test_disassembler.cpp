#include <gtest/gtest.h>
#include "disassembler.h"

using namespace videopac;

TEST(DisassemblerTest, DisassembleNOP) {
    Disassembler disasm;
    uint8 code[] = {0x00};  // NOP
    
    Instruction instr = disasm.disassemble_instruction(0x000, code);
    
    EXPECT_EQ(instr.address, 0x000);
    EXPECT_EQ(instr.opcode, 0x00);
    EXPECT_EQ(instr.mnemonic, "NOP");
    EXPECT_EQ(instr.size, 1);
    EXPECT_EQ(instr.cycles, 1);
    EXPECT_FALSE(instr.has_operand);
}

TEST(DisassemblerTest, DisassembleMOVImmediate) {
    Disassembler disasm;
    uint8 code[] = {0x23, 0x42};  // MOV A,#0x42
    
    Instruction instr = disasm.disassemble_instruction(0x000, code);
    
    EXPECT_EQ(instr.opcode, 0x23);
    EXPECT_EQ(instr.mnemonic, "MOV A,#");
    EXPECT_EQ(instr.size, 2);
    EXPECT_EQ(instr.cycles, 2);
    EXPECT_TRUE(instr.has_operand);
    EXPECT_EQ(instr.operand, 0x42);
    EXPECT_EQ(instr.operand_text, "#0x42");
}

TEST(DisassemblerTest, DisassembleADD) {
    Disassembler disasm;
    uint8 code[] = {0x03, 0x10};  // ADD A,#0x10
    
    Instruction instr = disasm.disassemble_instruction(0x000, code);
    
    EXPECT_EQ(instr.mnemonic, "ADD A,#");
    EXPECT_EQ(instr.operand, 0x10);
    EXPECT_EQ(instr.operand_text, "#0x10");
}

TEST(DisassemblerTest, DisassembleJMP) {
    Disassembler disasm;
    uint8 code[] = {0x04, 0x50};  // JMP 0x050
    
    Instruction instr = disasm.disassemble_instruction(0x000, code);
    
    EXPECT_EQ(instr.mnemonic, "JMP ");
    EXPECT_EQ(instr.operand, 0x50);
    // Target address should be calculated: (0x000 + 2) & 0xF00 | 0x50 = 0x050
    EXPECT_EQ(instr.operand_text, "0x050");
}

TEST(DisassemblerTest, DisassembleCALL) {
    Disassembler disasm;
    uint8 code[] = {0x14, 0xE7};  // CALL 0x0E7 (Enable VDC)
    
    Instruction instr = disasm.disassemble_instruction(0x000, code);
    
    EXPECT_EQ(instr.mnemonic, "CALL ");
    EXPECT_EQ(instr.operand, 0xE7);
    EXPECT_EQ(instr.operand_text, "0x0e7");
}

TEST(DisassemblerTest, DisassembleRET) {
    Disassembler disasm;
    uint8 code[] = {0x83};  // RET
    
    Instruction instr = disasm.disassemble_instruction(0x000, code);
    
    EXPECT_EQ(instr.mnemonic, "RET");
    EXPECT_EQ(instr.size, 1);
    EXPECT_FALSE(instr.has_operand);
}

TEST(DisassemblerTest, DisassembleRange) {
    Disassembler disasm;
    uint8 code[] = {
        0x23, 0x10,    // MOV A,#0x10
        0x03, 0x20,    // ADD A,#0x20
        0xA8,          // MOV R0,A
        0x18,          // INC R0
        0x00           // NOP
    };
    
    auto instructions = disasm.disassemble_range(code, 0x000, 0x006);
    
    EXPECT_EQ(instructions.size(), 5);
    EXPECT_EQ(instructions[0].mnemonic, "MOV A,#");
    EXPECT_EQ(instructions[1].mnemonic, "ADD A,#");
    EXPECT_EQ(instructions[2].mnemonic, "MOV R0,A");
    EXPECT_EQ(instructions[3].mnemonic, "INC R0");
    EXPECT_EQ(instructions[4].mnemonic, "NOP");
}

TEST(DisassemblerTest, FormatInstruction) {
    Disassembler disasm;
    uint8 code[] = {0x23, 0x42};  // MOV A,#0x42
    
    Instruction instr = disasm.disassemble_instruction(0x100, code);
    std::string formatted = disasm.format_instruction(instr);
    
    // Should contain address, opcode bytes, and mnemonic
    EXPECT_NE(formatted.find("0x100"), std::string::npos);
    EXPECT_TRUE(formatted.find("23") != std::string::npos || formatted.find("23") != std::string::npos);
    EXPECT_NE(formatted.find("MOV"), std::string::npos);
}

TEST(DisassemblerTest, IdentifyBIOSCall) {
    Disassembler disasm;
    
    EXPECT_EQ(disasm.identify_bios_call(0x0E7), "Enable VDC");
    EXPECT_EQ(disasm.identify_bios_call(0x38F), "Read Joystick");
    EXPECT_EQ(disasm.identify_bios_call(0x400), "Cartridge entry point");
    EXPECT_EQ(disasm.identify_bios_call(0x406), "VBLANK service routine");
    EXPECT_EQ(disasm.identify_bios_call(0x999), "");  // Unknown address
}

TEST(DisassemblerTest, UnknownOpcode) {
    Disassembler disasm;
    uint8 code[] = {0x01};  // Invalid opcode (not in 8048 set)
    
    Instruction instr = disasm.disassemble_instruction(0x000, code);
    
    EXPECT_EQ(instr.mnemonic, "DB");  // Data byte
    EXPECT_EQ(instr.operand_text, "0x01");
}

TEST(DisassemblerTest, BranchTargetCalculation) {
    Disassembler disasm;
    
    // JMP from 0x000 to 0x050
    uint8 code1[] = {0x04, 0x50};
    Instruction instr1 = disasm.disassemble_instruction(0x000, code1);
    // Target: (0x000 + 2) & 0xF00 | 0x50 = 0x002 & 0xF00 | 0x50 = 0x050
    EXPECT_EQ(instr1.mnemonic, "JMP ");
    EXPECT_EQ(instr1.operand, 0x50);
    EXPECT_EQ(instr1.operand_text, "0x050");
    
    // DJNZ from 0x100 to 0x150
    uint8 code2[] = {0xE8, 0x50};  // DJNZ R0,0x150
    Instruction instr2 = disasm.disassemble_instruction(0x100, code2);
    // Target: (0x100 + 2) & 0xF00 | 0x50 = 0x102 & 0xF00 | 0x50 = 0x150
    EXPECT_EQ(instr2.mnemonic, "DJNZ R0,");
    EXPECT_EQ(instr2.operand, 0x50);
    EXPECT_EQ(instr2.operand_text, "0x150");
}
