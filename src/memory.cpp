#include "memory.h"
#include "vdc.h"
#include "utils.h"
#include <fstream>
#include <cstring>

namespace videopac {

MemorySystem::MemorySystem() 
    : vdc_(nullptr), vdc_enabled_(false), ram_enabled_(false), copy_mode_(false) {
    std::memset(&state_, 0, sizeof(state_));
    state_.current_bank = 0;
    state_.num_banks = 1;
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
    
    // Cartridge ROM: 0x400-0xFFF (with banking)
    if (address >= 0x400 && !state_.cart_rom.empty()) {
        // Skip address line A10 (address 0x400 in ROM space)
        uint16 rom_offset = (address - 0x400) & 0xBFF;  // Mask out A10
        
        // Apply banking
        if (state_.num_banks > 1) {
            rom_offset += state_.current_bank * 1024;  // 1KB per bank
        }
        
        if (rom_offset < state_.cart_rom.size()) {
            return state_.cart_rom[rom_offset];
        }
    }
    
    return 0xFF;
}

uint8 MemorySystem::read_external(uint8 address) {
    // VDC registers: 0x00-0xFF (when P13 = 0)
    if (vdc_enabled_ && vdc_) {
        return vdc_->read_register(address);
    }
    
    // External RAM: 0x00-0x7F (when P14 = 0)
    if (ram_enabled_ && address < 128) {
        return state_.external_ram[address];
    }
    
    return 0xFF;
}

void MemorySystem::write_external(uint8 address, uint8 value) {
    // VDC registers: 0x00-0xFF (when P13 = 0)
    if (vdc_enabled_ && vdc_) {
        vdc_->write_register(address, value);
    }
    
    // External RAM: 0x00-0x7F (when P14 = 0)
    if (ram_enabled_ && address < 128) {
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

void MemorySystem::update_control_signals(uint8 port1_value) {
    // P13 = 0: VDC enabled
    vdc_enabled_ = !utils::get_bit(port1_value, 3);
    
    // P14 = 0: External RAM enabled
    ram_enabled_ = !utils::get_bit(port1_value, 4);
    
    // P16 = 1, P13 = 0, P14 = 0: Copy mode
    bool p16 = utils::get_bit(port1_value, 6);
    copy_mode_ = p16 && !utils::get_bit(port1_value, 3) && !utils::get_bit(port1_value, 4);
    
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
