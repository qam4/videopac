#include "disassembler.h"
#include <sstream>
#include <iomanip>
#include <map>

namespace videopac {

// Opcode information structure
struct OpcodeInfo {
    const char* mnemonic;
    uint8 size;        // 1 or 2 bytes
    uint8 cycles;      // Instruction cycles
    bool has_operand;  // Whether it has an immediate operand
};

// Intel 8048 opcode table
static const std::map<uint8, OpcodeInfo> opcode_table = {
    // Data transfer
    {0x23, {"MOV A,#", 2, 2, true}},
    {0xF8, {"MOV A,R0", 1, 1, false}},
    {0xF9, {"MOV A,R1", 1, 1, false}},
    {0xFA, {"MOV A,R2", 1, 1, false}},
    {0xFB, {"MOV A,R3", 1, 1, false}},
    {0xFC, {"MOV A,R4", 1, 1, false}},
    {0xFD, {"MOV A,R5", 1, 1, false}},
    {0xFE, {"MOV A,R6", 1, 1, false}},
    {0xFF, {"MOV A,R7", 1, 1, false}},
    {0xF0, {"MOV A,@R0", 1, 1, false}},
    {0xF1, {"MOV A,@R1", 1, 1, false}},
    {0xA8, {"MOV R0,A", 1, 1, false}},
    {0xA9, {"MOV R1,A", 1, 1, false}},
    {0xAA, {"MOV R2,A", 1, 1, false}},
    {0xAB, {"MOV R3,A", 1, 1, false}},
    {0xAC, {"MOV R4,A", 1, 1, false}},
    {0xAD, {"MOV R5,A", 1, 1, false}},
    {0xAE, {"MOV R6,A", 1, 1, false}},
    {0xAF, {"MOV R7,A", 1, 1, false}},
    {0xA0, {"MOV @R0,A", 1, 1, false}},
    {0xA1, {"MOV @R1,A", 1, 1, false}},
    {0xB8, {"MOV R0,#", 2, 2, true}},
    {0xB9, {"MOV R1,#", 2, 2, true}},
    {0xBA, {"MOV R2,#", 2, 2, true}},
    {0xBB, {"MOV R3,#", 2, 2, true}},
    {0xBC, {"MOV R4,#", 2, 2, true}},
    {0xBD, {"MOV R5,#", 2, 2, true}},
    {0xBE, {"MOV R6,#", 2, 2, true}},
    {0xBF, {"MOV R7,#", 2, 2, true}},
    {0xB0, {"MOV @R0,#", 2, 2, true}},
    {0xB1, {"MOV @R1,#", 2, 2, true}},
    
    // Arithmetic
    {0x03, {"ADD A,#", 2, 2, true}},
    {0x68, {"ADD A,R0", 1, 1, false}},
    {0x69, {"ADD A,R1", 1, 1, false}},
    {0x6A, {"ADD A,R2", 1, 1, false}},
    {0x6B, {"ADD A,R3", 1, 1, false}},
    {0x6C, {"ADD A,R4", 1, 1, false}},
    {0x6D, {"ADD A,R5", 1, 1, false}},
    {0x6E, {"ADD A,R6", 1, 1, false}},
    {0x6F, {"ADD A,R7", 1, 1, false}},
    {0x60, {"ADD A,@R0", 1, 1, false}},
    {0x61, {"ADD A,@R1", 1, 1, false}},
    {0x13, {"ADDC A,#", 2, 2, true}},
    {0x78, {"ADDC A,R0", 1, 1, false}},
    {0x79, {"ADDC A,R1", 1, 1, false}},
    {0x7A, {"ADDC A,R2", 1, 1, false}},
    {0x7B, {"ADDC A,R3", 1, 1, false}},
    {0x7C, {"ADDC A,R4", 1, 1, false}},
    {0x7D, {"ADDC A,R5", 1, 1, false}},
    {0x7E, {"ADDC A,R6", 1, 1, false}},
    {0x7F, {"ADDC A,R7", 1, 1, false}},
    {0x70, {"ADDC A,@R0", 1, 1, false}},
    {0x71, {"ADDC A,@R1", 1, 1, false}},
    {0x17, {"INC A", 1, 1, false}},
    {0x18, {"INC R0", 1, 1, false}},
    {0x19, {"INC R1", 1, 1, false}},
    {0x1A, {"INC R2", 1, 1, false}},
    {0x1B, {"INC R3", 1, 1, false}},
    {0x1C, {"INC R4", 1, 1, false}},
    {0x1D, {"INC R5", 1, 1, false}},
    {0x1E, {"INC R6", 1, 1, false}},
    {0x1F, {"INC R7", 1, 1, false}},
    {0x10, {"INC @R0", 1, 1, false}},
    {0x11, {"INC @R1", 1, 1, false}},
    {0x07, {"DEC A", 1, 1, false}},
    {0xC8, {"DEC R0", 1, 1, false}},
    {0xC9, {"DEC R1", 1, 1, false}},
    {0xCA, {"DEC R2", 1, 1, false}},
    {0xCB, {"DEC R3", 1, 1, false}},
    {0xCC, {"DEC R4", 1, 1, false}},
    {0xCD, {"DEC R5", 1, 1, false}},
    {0xCE, {"DEC R6", 1, 1, false}},
    {0xCF, {"DEC R7", 1, 1, false}},
    {0x57, {"DA A", 1, 1, false}},
    
    // Logical
    {0x53, {"ANL A,#", 2, 2, true}},
    {0x58, {"ANL A,R0", 1, 1, false}},
    {0x59, {"ANL A,R1", 1, 1, false}},
    {0x5A, {"ANL A,R2", 1, 1, false}},
    {0x5B, {"ANL A,R3", 1, 1, false}},
    {0x5C, {"ANL A,R4", 1, 1, false}},
    {0x5D, {"ANL A,R5", 1, 1, false}},
    {0x5E, {"ANL A,R6", 1, 1, false}},
    {0x5F, {"ANL A,R7", 1, 1, false}},
    {0x50, {"ANL A,@R0", 1, 1, false}},
    {0x51, {"ANL A,@R1", 1, 1, false}},
    {0x43, {"ORL A,#", 2, 2, true}},
    {0x48, {"ORL A,R0", 1, 1, false}},
    {0x49, {"ORL A,R1", 1, 1, false}},
    {0x4A, {"ORL A,R2", 1, 1, false}},
    {0x4B, {"ORL A,R3", 1, 1, false}},
    {0x4C, {"ORL A,R4", 1, 1, false}},
    {0x4D, {"ORL A,R5", 1, 1, false}},
    {0x4E, {"ORL A,R6", 1, 1, false}},
    {0x4F, {"ORL A,R7", 1, 1, false}},
    {0x40, {"ORL A,@R0", 1, 1, false}},
    {0x41, {"ORL A,@R1", 1, 1, false}},
    {0xD3, {"XRL A,#", 2, 2, true}},
    {0xD8, {"XRL A,R0", 1, 1, false}},
    {0xD9, {"XRL A,R1", 1, 1, false}},
    {0xDA, {"XRL A,R2", 1, 1, false}},
    {0xDB, {"XRL A,R3", 1, 1, false}},
    {0xDC, {"XRL A,R4", 1, 1, false}},
    {0xDD, {"XRL A,R5", 1, 1, false}},
    {0xDE, {"XRL A,R6", 1, 1, false}},
    {0xDF, {"XRL A,R7", 1, 1, false}},
    {0xD0, {"XRL A,@R0", 1, 1, false}},
    {0xD1, {"XRL A,@R1", 1, 1, false}},
    {0x27, {"CLR A", 1, 1, false}},
    {0x37, {"CPL A", 1, 1, false}},
    {0x47, {"SWAP A", 1, 1, false}},
    {0xF7, {"RLC A", 1, 1, false}},
    {0xE7, {"RL A", 1, 1, false}},
    {0x77, {"RRC A", 1, 1, false}},
    {0x67, {"RR A", 1, 1, false}},
    
    // Branch
    {0x04, {"JMP ", 2, 2, true}},  // Note: actual address calculation needed
    {0x24, {"JMP ", 2, 2, true}},
    {0x44, {"JMP ", 2, 2, true}},
    {0x64, {"JMP ", 2, 2, true}},
    {0x84, {"JMP ", 2, 2, true}},
    {0xA4, {"JMP ", 2, 2, true}},
    {0xC4, {"JMP ", 2, 2, true}},
    {0xE4, {"JMP ", 2, 2, true}},
    {0x12, {"JB0 ", 2, 2, true}},
    {0x32, {"JB1 ", 2, 2, true}},
    {0x52, {"JB2 ", 2, 2, true}},
    {0x72, {"JB3 ", 2, 2, true}},
    {0x92, {"JB4 ", 2, 2, true}},
    {0xB2, {"JB5 ", 2, 2, true}},
    {0xD2, {"JB6 ", 2, 2, true}},
    {0xF2, {"JB7 ", 2, 2, true}},
    {0xF6, {"JC ", 2, 2, true}},
    {0xE6, {"JNC ", 2, 2, true}},
    {0xC6, {"JZ ", 2, 2, true}},
    {0x96, {"JNZ ", 2, 2, true}},
    {0x36, {"JT0 ", 2, 2, true}},
    {0x26, {"JNT0 ", 2, 2, true}},
    {0x56, {"JT1 ", 2, 2, true}},
    {0x46, {"JNT1 ", 2, 2, true}},
    {0xB6, {"JF0 ", 2, 2, true}},
    {0x76, {"JF1 ", 2, 2, true}},
    {0x86, {"JNI ", 2, 2, true}},
    {0xE8, {"DJNZ R0,", 2, 2, true}},
    {0xE9, {"DJNZ R1,", 2, 2, true}},
    {0xEA, {"DJNZ R2,", 2, 2, true}},
    {0xEB, {"DJNZ R3,", 2, 2, true}},
    {0xEC, {"DJNZ R4,", 2, 2, true}},
    {0xED, {"DJNZ R5,", 2, 2, true}},
    {0xEE, {"DJNZ R6,", 2, 2, true}},
    {0xEF, {"DJNZ R7,", 2, 2, true}},
    
    // Subroutine
    {0x14, {"CALL ", 2, 2, true}},
    {0x34, {"CALL ", 2, 2, true}},
    {0x54, {"CALL ", 2, 2, true}},
    {0x74, {"CALL ", 2, 2, true}},
    {0x94, {"CALL ", 2, 2, true}},
    {0xB4, {"CALL ", 2, 2, true}},
    {0xD4, {"CALL ", 2, 2, true}},
    {0xF4, {"CALL ", 2, 2, true}},
    {0x83, {"RET", 1, 2, false}},
    {0x93, {"RETR", 1, 2, false}},
    
    // I/O and control
    {0x00, {"NOP", 1, 1, false}},
    {0x08, {"INS A,BUS", 1, 2, false}},
    {0x09, {"IN A,P1", 1, 2, false}},
    {0x0A, {"IN A,P2", 1, 2, false}},
    {0x02, {"OUTL BUS,A", 1, 2, false}},
    {0x39, {"OUTL P1,A", 1, 2, false}},
    {0x3A, {"OUTL P2,A", 1, 2, false}},
    {0x99, {"ANL P1,#", 2, 2, true}},
    {0x9A, {"ANL P2,#", 2, 2, true}},
    {0x89, {"ORL P1,#", 2, 2, true}},
    {0x8A, {"ORL P2,#", 2, 2, true}},
    {0x80, {"MOVX A,@R0", 1, 2, false}},
    {0x81, {"MOVX A,@R1", 1, 2, false}},
    {0x90, {"MOVX @R0,A", 1, 2, false}},
    {0x91, {"MOVX @R1,A", 1, 2, false}},
    {0xA3, {"MOVP A,@A", 1, 2, false}},
    {0xE3, {"MOVP3 A,@A", 1, 2, false}},
    {0x05, {"EN I", 1, 1, false}},
    {0x15, {"DIS I", 1, 1, false}},
    {0x25, {"EN TCNTI", 1, 1, false}},
    {0x35, {"DIS TCNTI", 1, 1, false}},
    {0x45, {"STRT CNT", 1, 1, false}},
    {0x55, {"STRT T", 1, 1, false}},
    {0x65, {"STOP TCNT", 1, 1, false}},
    {0x85, {"CLR F0", 1, 1, false}},
    {0x95, {"CPL F0", 1, 1, false}},
    {0xA5, {"CLR F1", 1, 1, false}},
    {0xB5, {"CPL F1", 1, 1, false}},
    {0x97, {"CLR C", 1, 1, false}},
    {0xA7, {"CPL C", 1, 1, false}},
    {0xC5, {"SEL RB0", 1, 1, false}},
    {0xD5, {"SEL RB1", 1, 1, false}},
    {0xE5, {"SEL MB0", 1, 1, false}},
    {0xF5, {"SEL MB1", 1, 1, false}},
    {0x20, {"XCH A,@R0", 1, 1, false}},
    {0x21, {"XCH A,@R1", 1, 1, false}},
    {0x28, {"XCH A,R0", 1, 1, false}},
    {0x29, {"XCH A,R1", 1, 1, false}},
    {0x2A, {"XCH A,R2", 1, 1, false}},
    {0x2B, {"XCH A,R3", 1, 1, false}},
    {0x2C, {"XCH A,R4", 1, 1, false}},
    {0x2D, {"XCH A,R5", 1, 1, false}},
    {0x2E, {"XCH A,R6", 1, 1, false}},
    {0x2F, {"XCH A,R7", 1, 1, false}},
    {0x30, {"XCHD A,@R0", 1, 1, false}},
    {0x31, {"XCHD A,@R1", 1, 1, false}},
};

Instruction Disassembler::disassemble_instruction(uint16 address, const uint8* memory) {
    Instruction instr;
    instr.address = address;
    instr.opcode = memory[0];
    instr.operand = 0;
    instr.has_operand = false;
    instr.cycles = 1;
    instr.size = 1;
    
    // Look up opcode
    auto it = opcode_table.find(instr.opcode);
    if (it != opcode_table.end()) {
        const OpcodeInfo& info = it->second;
        instr.mnemonic = info.mnemonic;
        instr.size = info.size;
        instr.cycles = info.cycles;
        instr.has_operand = info.has_operand;
        
        if (instr.has_operand && instr.size == 2) {
            instr.operand = memory[1];
            
            // Format operand based on instruction type
            char operand_buf[16];
            
            // Check if it's a jump/call instruction (needs address calculation)
            if (instr.mnemonic.find("JMP") == 0 || instr.mnemonic.find("CALL") == 0) {
                // JMP and CALL use bits 5-7 of opcode for address bits 8-10
                uint16 target = ((instr.opcode & 0xE0) << 3) | instr.operand;
                snprintf(operand_buf, sizeof(operand_buf), "0x%03x", target);
            } else if (instr.mnemonic.find("JB") == 0 || instr.mnemonic.find("JC") == 0 ||
                       instr.mnemonic.find("JNC") == 0 || instr.mnemonic.find("JZ") == 0 ||
                       instr.mnemonic.find("JNZ") == 0 || instr.mnemonic.find("JT") == 0 ||
                       instr.mnemonic.find("JNT") == 0 || instr.mnemonic.find("JF") == 0 ||
                       instr.mnemonic.find("JNI") == 0 || instr.mnemonic.find("DJNZ") == 0) {
                // Conditional jumps use current page (bits 8-11 of PC after increment)
                uint16 target = ((address + 2) & 0xF00) | instr.operand;
                snprintf(operand_buf, sizeof(operand_buf), "0x%03x", target);
            } else {
                // Immediate value
                snprintf(operand_buf, sizeof(operand_buf), "#0x%02x", instr.operand);
            }
            
            instr.operand_text = operand_buf;
        }
    } else {
        // Unknown opcode
        instr.mnemonic = "DB";
        char operand_buf[16];
        snprintf(operand_buf, sizeof(operand_buf), "0x%02x", instr.opcode);
        instr.operand_text = operand_buf;
    }
    
    return instr;
}

std::vector<Instruction> Disassembler::disassemble_range(const uint8* memory, uint16 start, uint16 end) {
    std::vector<Instruction> instructions;
    
    uint16 addr = start;
    while (addr <= end) {
        Instruction instr = disassemble_instruction(addr, &memory[addr]);
        instructions.push_back(instr);
        addr += instr.size;
        
        if (addr < start) break;  // Wrapped around
    }
    
    return instructions;
}

std::vector<Instruction> Disassembler::disassemble_rom(const uint8* rom, size_t size) {
    std::vector<Instruction> instructions;
    
    size_t addr = 0;
    while (addr < size) {
        Instruction instr = disassemble_instruction(static_cast<uint16>(addr), &rom[addr]);
        instructions.push_back(instr);
        addr += instr.size;
    }
    
    return instructions;
}

std::string Disassembler::format_instruction(const Instruction& instr) {
    char buffer[64];
    
    // Address and opcode bytes
    if (instr.has_operand && instr.size == 2) {
        snprintf(buffer, sizeof(buffer), "0x%03x: %02x %02x  %s%s",
                instr.address, instr.opcode, instr.operand,
                instr.mnemonic.c_str(), instr.operand_text.c_str());
    } else {
        snprintf(buffer, sizeof(buffer), "0x%03x: %02x     %s%s",
                instr.address, instr.opcode,
                instr.mnemonic.c_str(), instr.operand_text.c_str());
    }
    
    return std::string(buffer);
}

std::string Disassembler::get_label_name(uint16 address, bool add_prefix) {
    // Known BIOS and cartridge routine addresses with meaningful label names
    static const std::map<uint16, std::string> known_labels = {
        // BIOS ROM Routine Addresses (0x000-0x3FF)
        {0x000, "cold_boot"},
        {0x003, "external_t0_interrupt"},
        {0x007, "timer_clock_interrupt"},
        {0x009, "vblank_interrupt_routine_1"},
        {0x01A, "vblank_interrupt_routine_2"},
        {0x044, "vblank_interrupt_routine_3"},
        {0x089, "ram_to_vdc_vblank_copying_check"},
        {0x0A3, "copying_code"},
        {0x0B0, "keyboard_routine"},
        {0x0E7, "set_up_vdc_access"},
        {0x0EC, "set_up_ram_access"},
        {0x0F1, "reset"},
        {0x11C, "display_off"},
        {0x127, "display_on"},
        {0x132, "enable_data_copy_next_vsync"},
        {0x13D, "get_keystroke"},
        {0x14B, "character_colour_translation"},
        {0x16B, "clear_all_characters"},
        {0x176, "wait_for_interrupt"},
        {0x17C, "display_2_digit_bcd_characters"},
        {0x1A2, "start_tune"},
        {0x1B0, "up_down_counter"},
        {0x23A, "set_up_quad_score_characters"},
        {0x261, "translate_copy_character_colour"},
        {0x26A, "bit_test"},
        {0x280, "bit_clear"},
        {0x28A, "bit_set"},
        {0x293, "unknown_0293"},
        {0x2C3, "select_game"},
        {0x300, "frequency_data"},
        {0x34A, "tune_data"},
        {0x376, "keyboard_in_routine_end"},
        {0x37E, "misc_interrupt_handlers_banked_roms"},
        {0x38F, "read_joystick"},
        {0x3B1, "unknown_03b1"},
        {0x3CF, "unknown_03cf"},
        {0x3EA, "character_write"},
        // Vectors in Odyssey II ROM Cartridges (0x400-0x40A)
        {0x400, "restart"},
        {0x402, "vblank_external_interrupt"},
        {0x404, "timer_clock_interrupt_cart"},
        {0x406, "vblank_routine_vector"},
        {0x408, "end_of_select_game"},
        {0x40A, "continuation_of_vblank"}
    };
    
    auto it = known_labels.find(address);
    if (it != known_labels.end()) {
        // Add prefix based on address range (only if requested)
        if (add_prefix) {
            if (address < 0x400) {
                return "bios:" + it->second;
            } else {
                return "rom:" + it->second;
            }
        }
        return it->second;
    }
    
    return "";
}

} // namespace videopac
