#include "memory.h"
#include "cpu.h"
#include "vdc.h"
#include "utils.h"
#include <fstream>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <iomanip>

namespace videopac {

MemorySystem::MemorySystem() 
    : vdc_(nullptr), cpu_(nullptr), test_port1_(0x08) {  // Default: P13=1, P14=0 (RAM enabled)
    // Initialize arrays to zero
    std::memset(state_.bios_rom, 0, sizeof(state_.bios_rom));
    std::memset(state_.external_ram, 0, sizeof(state_.external_ram));
    
    // Initialize scalar members
    state_.current_bank = 0;
    state_.num_banks = 1;
    state_.rom_size_kb = 0;
    
    // cart_rom vector is already default-initialized to empty
}

Result<void> MemorySystem::load_bios(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return Result<void>::err("Failed to open BIOS file: " + path);
    }
    
    file.read(reinterpret_cast<char*>(state_.bios_rom), 1024);
    if (file.gcount() != 1024) {
        return Result<void>::err("BIOS file must be exactly 1KB");
    }
    
    return Result<void>::ok();
}

Result<void> MemorySystem::load_bios(const uint8* data, size_t size) {
    if (size != 1024) {
        return Result<void>::err("BIOS must be exactly 1KB");
    }
    std::memcpy(state_.bios_rom, data, 1024);
    return Result<void>::ok();
}

Result<void> MemorySystem::load_cartridge(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return Result<void>::err("Failed to open ROM file: " + path);
    }
    
    size_t size = file.tellg();
    auto result = validate_rom_size(size);
    if (result.is_err()) {
        return result;
    }
    
    file.seekg(0);
    state_.cart_rom.resize(size);
    file.read(reinterpret_cast<char*>(state_.cart_rom.data()), size);
    
    detect_banking(size);
    return Result<void>::ok();
}

Result<void> MemorySystem::load_cartridge(const uint8* data, size_t size) {
    auto result = validate_rom_size(size);
    if (result.is_err()) {
        return result;
    }
    
    state_.cart_rom.assign(data, data + size);
    detect_banking(size);
    return Result<void>::ok();
}

uint8 MemorySystem::read_program(uint16 address) {
    // BIOS ROM: 0x000-0x3FF
    if (address < 0x400) {
        return state_.bios_rom[address];
    }
    
    // Cartridge ROM: 0x400-0xFFF (with banking and A10 mirroring)
    if (address >= 0x400 && !state_.cart_rom.empty()) {
        // A10 is not connected to the cartridge (doc/o2doc.md section 2.0)
        // "leaving off A10 will map this address to the first byte in the cartridge"
        // The cartridge only sees A0-A9 and A11 (11 bits total, not 12)
        //
        // Address bits:    A11 A10 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
        // Cartridge sees:  A11  -  A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
        // 
        // For 2KB ROM (0x800 bytes):
        // CPU 0x400-0x7FF (A11=0, A10=x): Cartridge sees 0x000-0x3FF → ROM[0x000-0x3FF]
        // CPU 0x800-0xBFF (A11=1, A10=0): Cartridge sees 0x400-0x7FF → ROM[0x400-0x7FF]
        // CPU 0xC00-0xFFF (A11=1, A10=1): Cartridge sees 0x400-0x7FF → ROM[0x400-0x7FF] (mirror)
        //
        // Implementation: Cartridge address = (A11 ? 0x400 : 0x000) | (A9-A0)
        // Formula: rom_offset = ((address & 0x800) >> 1) | (address & 0x3FF)
        //          Shift A11 right by 1 to get 0x400 when A11=1, then OR with A9-A0
        uint16 rom_offset = ((address & 0x800) >> 1) | (address & 0x3FF);
        
        // Apply banking
        if (state_.num_banks > 1) {
            rom_offset += state_.current_bank * 1024;  // 1KB per bank
        }
        
        // Check if accessing beyond ROM size
        if (rom_offset >= state_.cart_rom.size()) {
            // Return 0xFF for unmapped memory (NOP in 8048)
            return 0xFF;
        }
        
        return state_.cart_rom[rom_offset];
    }
    
    // No cartridge loaded - return 0xFF
    return 0xFF;
}

uint8 MemorySystem::read_external(uint8 address) {
    // Copy mode: P16=1, P13=0, P14=0
    // Reference: doc/o2doc.md section 1.1 "P16: Copy mode enable"
    // In copy mode, all external reads come from RAM
    if (copy_mode() && vdc_enabled() && ram_enabled()) {
        if (address < 128) {
            return state_.external_ram[address];
        }
        return 0xFF;  // Beyond RAM bounds
    }
    
    // VDC registers: 0x00-0xFF (when P13=0)
    // Reference: doc/o2doc.md section 4.0 "VDC"
    // Hardware behavior: VDC reads work when P13=0 (VDC chip select active)
    if (vdc_enabled() && vdc_) {
        return vdc_->read_register(address);
    }
    
    // External RAM: 0x00-0x7F (when P13=1, P14=0)
    // Reference: doc/o2doc.md section 3.0 "External RAM"
    // "To enable the external ram set P14 low and P13 high"
    if (!vdc_enabled() && ram_enabled() && address < 128) {
        return state_.external_ram[address];
    }
    
    // If neither VDC nor RAM is enabled, external reads return 0xFF
    // This represents floating bus / no device responding
    return 0xFF;
}

void MemorySystem::write_external(uint8 address, uint8 value) {
    // Copy mode: P16=1, P13=0, P14=0
    // Reference: doc/o2doc.md section 1.1 "P16: Copy mode enable"
    // In copy mode: reads from RAM, writes to VDC only (EXRAM writes disabled)
    if (copy_mode() && vdc_enabled() && ram_enabled()) {
        if (vdc_) {
            vdc_->write_register(address, value);
        }
        return;  // Don't write to RAM in copy mode (P16=1 disables EXRAM writes)
    }
    
    // VDC writes: Work when P13=0 (regardless of P14 state)
    // Reference: Hardware behavior confirmed via internet research
    // "VDC writes generally work with P13=0 regardless of the state of P14, provided that P16 is 0"
    // Note: If P13=0 AND P14=0, data writes to BOTH VDC and EXRAM simultaneously
    if (vdc_enabled() && vdc_) {
        vdc_->write_register(address, value);
    }
    
    // External RAM: 0x00-0x7F (when P14=0)
    // Reference: doc/o2doc.md section 3.0 "External RAM"
    // Note: If P13=0 AND P14=0, data writes to BOTH VDC and EXRAM simultaneously
    if (ram_enabled() && address < 128) {
        // Log when RAM[0x3F] bit 7 is set (copy request)
        if (address == 0x3F && (value & 0x80)) {
            uint8 old_value = state_.external_ram[address];
            if (!(old_value & 0x80)) {
                // Copy request being set - dump the copy data
                uint8 count = state_.external_ram[0x7F];
                uint8 target = state_.external_ram[0x7E];
                std::cout << "[COPY REQUEST] RAM[0x3F] bit 7 set. Count=0x" << std::hex << std::setw(2) << std::setfill('0') 
                          << (int)count << " Target=0x" << (int)target << std::dec << std::endl;
                // Dump the data bytes
                std::cout << "[COPY DATA] ";
                for (int i = 0; i < 8 && i < count; i++) {
                    uint8 addr = 0x7D - i;
                    uint8 data = state_.external_ram[addr];
                    std::cout << "RAM[0x" << std::hex << std::setw(2) << std::setfill('0') << (int)addr 
                              << "]=0x" << (int)data << " ";
                }
                std::cout << std::dec << std::endl;
                std::cout.flush();
            }
        }
        state_.external_ram[address] = value;
    }
}

void MemorySystem::set_bank(uint8 bank) {
    if (bank < state_.num_banks) {
        state_.current_bank = bank;
    }
}

void MemorySystem::set_vdc(VDC* vdc) {
    vdc_ = vdc;
}

void MemorySystem::set_cpu(CPU* cpu) {
    cpu_ = cpu;
}

uint8 MemorySystem::get_port1() const {
    if (!cpu_) {
        // Use test value when no CPU is connected (for unit testing)
        return test_port1_;
    }
    return cpu_->get_state().port1;
}

bool MemorySystem::vdc_enabled() const {
    return !(get_port1() & P1_VDCEN);  // P13=0 enables VDC
}

bool MemorySystem::ram_enabled() const {
    return !(get_port1() & P1_RAMEN);  // P14=0 enables RAM
}

bool MemorySystem::copy_mode() const {
    return (get_port1() & P1_COPYEN) != 0;  // P16=1 enables copy mode
}

void MemorySystem::update_control_signals(uint8 port1_value) {
    // Store Port 1 value for testing (when no CPU is connected)
    test_port1_ = port1_value;
    
    // Bank switching: P10 and P11
    if (state_.num_banks > 1) {
        uint8 bank = 0;
        if (utils::get_bit(port1_value, 0)) bank |= 1;  // P10
        if (utils::get_bit(port1_value, 1)) bank |= 2;  // P11
        set_bank(bank);
    }
}

MemoryState MemorySystem::get_state() const {
    return state_;
}

void MemorySystem::set_state(const MemoryState& state) {
    state_ = state;
}

Result<void> MemorySystem::validate_rom_size(size_t size) {
    if (size != 2048 && size != 4096 && size != 8192) {
        return Result<void>::err("ROM size must be 2KB, 4KB, or 8KB");
    }
    return Result<void>::ok();
}

void MemorySystem::detect_banking(size_t size) {
    state_.rom_size_kb = static_cast<uint8>(size / 1024);
    
    if (size == 2048) {
        state_.num_banks = 1;
    } else if (size == 4096) {
        state_.num_banks = 2;
    } else if (size == 8192) {
        state_.num_banks = 4;
    }
    
    state_.current_bank = 0;
}

} // namespace videopac
