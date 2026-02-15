#ifndef VIDEOPAC_DISASSEMBLER_H
#define VIDEOPAC_DISASSEMBLER_H

#include "types.h"
#include <string>
#include <vector>

namespace videopac {

// Instruction representation
struct Instruction {
    uint16 address;
    uint8 opcode;
    uint8 operand;
    bool has_operand;
    std::string mnemonic;
    std::string operand_text;
    uint8 cycles;
    uint8 size;
};

// Disassembler
class Disassembler {
public:
    Disassembler() = default;
    ~Disassembler() = default;
    
    // Disassembly
    Instruction disassemble_instruction(uint16 address, const uint8* memory);
    std::vector<Instruction> disassemble_range(const uint8* memory, uint16 start, uint16 end);
    std::vector<Instruction> disassemble_rom(const uint8* rom, size_t size);
    
    // Formatting
    std::string format_instruction(const Instruction& instr);
    std::string get_label_name(uint16 address, bool add_prefix = false);  // Get label name for known addresses
};

} // namespace videopac

#endif // VIDEOPAC_DISASSEMBLER_H
