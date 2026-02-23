#include "disassembler.h"
#include "ui/zip_handler.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <filesystem>

using namespace videopac;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <rom_file> [--rom]" << std::endl;
        std::cerr << "  rom_file can be .bin or .zip (will extract first .bin file)" << std::endl;
        std::cerr << "  --rom: disassemble as ROM (base address 0x0400)" << std::endl;
        std::cerr << "  If --rom is not specified, assumes BIOS (base address 0x0000)" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Examples:" << std::endl;
        std::cerr << "  " << argv[0] << " bios.bin" << std::endl;
        std::cerr << "  " << argv[0] << " game.bin --rom" << std::endl;
        std::cerr << "  " << argv[0] << " game.zip --rom" << std::endl;
        return 1;
    }
    
    std::vector<uint8_t> rom;
    std::string filename = argv[1];
    
    // Parse command line arguments for --rom flag
    uint16_t base_addr = 0x0000;  // Default to BIOS
    
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--rom") {
            base_addr = 0x0400;
        }
    }
    
    // Check if it's a zip file
    if (fs::path(filename).extension() == ".zip") {
        ZIPHandler zip;
        if (!zip.open(filename)) {
            std::cerr << "Failed to open zip file: " << filename << std::endl;
            return 1;
        }
        
        auto rom_files = zip.get_rom_files();
        if (rom_files.empty()) {
            std::cerr << "No ROM files found in zip" << std::endl;
            zip.close();
            return 1;
        }
        
        // Extract first ROM file
        std::string extracted_path = zip.extract_file(rom_files[0]);
        if (extracted_path.empty()) {
            std::cerr << "Failed to extract ROM from zip" << std::endl;
            zip.close();
            return 1;
        }
        
        std::cerr << "Extracted " << rom_files[0] << " from zip" << std::endl;
        
        // Read the extracted file
        std::ifstream file(extracted_path, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to read extracted file" << std::endl;
            zip.cleanup_temp_files();
            zip.close();
            return 1;
        }
        rom = std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                     std::istreambuf_iterator<char>());
        file.close();
        
        std::cerr << "ROM size: " << rom.size() << " bytes" << std::endl;
        
        // Cleanup
        zip.cleanup_temp_files();
        zip.close();
    } else {
        // Read ROM file directly
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return 1;
        }
        rom = std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                     std::istreambuf_iterator<char>());
    }
    
    uint16_t end = rom.size();
    
    std::cerr << "Disassembling with base address: 0x" << std::hex << std::setw(4) 
              << std::setfill('0') << base_addr << std::endl;
    std::cerr << "ROM size: " << std::dec << rom.size() << " bytes" << std::endl;
    
    Disassembler disasm;
    
    // First pass: disassemble and collect jump/call targets
    // Disassemble from file offset 0, but label addresses starting at base_addr
    std::vector<Instruction> instructions;
    uint16_t addr = base_addr;
    size_t offset = 0;
    while (offset < rom.size() && offset < end) {
        Instruction instr = disasm.disassemble_instruction(addr, &rom[offset]);
        instructions.push_back(instr);
        offset += instr.size;
        addr += instr.size;
    }
    std::set<uint16_t> jump_targets;
    
    // Add all known addresses as jump targets so they get labels
    for (const auto& instr : instructions) {
        std::string known_label = disasm.get_label_name(instr.address);
        if (!known_label.empty()) {
            jump_targets.insert(instr.address);
        }
    }
    
    for (const auto& instr : instructions) {
        // Check if this is a JMP or CALL instruction
        if (instr.mnemonic.find("JMP") == 0 || instr.mnemonic.find("CALL") == 0 ||
            instr.mnemonic.find("JB") == 0 || instr.mnemonic.find("JC") == 0 ||
            instr.mnemonic.find("JNC") == 0 || instr.mnemonic.find("JZ") == 0 ||
            instr.mnemonic.find("JNZ") == 0 || instr.mnemonic.find("JT") == 0 ||
            instr.mnemonic.find("JNT") == 0 || instr.mnemonic.find("JF") == 0 ||
            instr.mnemonic.find("JNI") == 0 || instr.mnemonic.find("DJNZ") == 0) {
            
            // Extract target address from operand_text (format: "0xXXX")
            if (instr.operand_text.find("0x") == 0) {
                uint16_t target = std::strtol(instr.operand_text.c_str() + 2, nullptr, 16);
                // Only add label if target is within our disassembled range
                if (target >= base_addr && target < base_addr + end) {
                    jump_targets.insert(target);
                }
            }
        }
    }
    
    // Second pass: output with labels
    for (const auto& instr : instructions) {
        // Check if this address has a label
        if (jump_targets.find(instr.address) != jump_targets.end()) {
            // Add blank line before label for readability
            std::cout << std::endl;
            
            // Use known label name if available, otherwise use loc_XXXX
            // Don't add prefix for label definitions
            std::string label = disasm.get_label_name(instr.address, false);
            if (!label.empty()) {
                std::cout << label << ":" << std::endl;
            } else {
                std::cout << "loc_" << std::hex << std::setw(4) << std::setfill('0') 
                          << instr.address << ":" << std::endl;
            }
        }
        
        // Format instruction
        std::string formatted = disasm.format_instruction(instr);
        
        // Replace hex addresses with labels in the operand ONLY (not in the address prefix)
        // Check if this instruction has a jump/call target
        if (instr.operand_text.find("0x") == 0) {
            uint16_t target = std::strtol(instr.operand_text.c_str() + 2, nullptr, 16);
            
            // First check if it's a known address
            // Add prefix for label references in instructions
            std::string label = disasm.get_label_name(target, true);
            
            // If not a known address, check if it's in jump_targets
            if (label.empty() && jump_targets.find(target) != jump_targets.end()) {
                std::stringstream ss;
                ss << "loc_" << std::hex << std::setw(4) << std::setfill('0') << target;
                label = ss.str();
            }
            
            // Replace the address with the label if we found one
            // IMPORTANT: Only replace in the operand part (after the mnemonic), not the address prefix
            if (!label.empty()) {
                // Find the operand text in the formatted string (it appears after the mnemonic)
                // The format is: "0xADDR: OP OP  MNEMONIC OPERAND"
                // We want to replace OPERAND, not the address prefix
                size_t mnemonic_pos = formatted.find(instr.mnemonic);
                if (mnemonic_pos != std::string::npos) {
                    size_t operand_pos = formatted.find(instr.operand_text, mnemonic_pos);
                    if (operand_pos != std::string::npos) {
                        formatted.replace(operand_pos, instr.operand_text.length(), label);
                    }
                }
            }
        }
        
        std::cout << formatted << std::endl;
    }
    
    return 0;
}
