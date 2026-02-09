#ifndef VIDEOPAC_MEMORY_H
#define VIDEOPAC_MEMORY_H

#include "types.h"
#include <vector>

namespace videopac {

// Forward declaration
class VDC;

// Memory system state
struct MemoryState {
    uint8 bios_rom[1024];       // BIOS ROM (1KB)
    std::vector<uint8> cart_rom; // Cartridge ROM (2KB-8KB)
    uint8 external_ram[128];    // External RAM (128 bytes)
    uint8 current_bank;         // Current ROM bank
    uint8 rom_size_kb;          // ROM size in KB (2, 4, or 8)
    uint8 num_banks;            // Number of banks (1, 2, or 4)
};

// Memory system
class MemorySystem {
public:
    MemorySystem();
    ~MemorySystem() = default;
    
    // ROM loading
    Result<void> load_bios(const std::string& path);
    Result<void> load_bios(const uint8* data, size_t size);
    Result<void> load_cartridge(const std::string& path);
    Result<void> load_cartridge(const uint8* data, size_t size);
    
    // Memory access
    uint8 read_program(uint16 address);
    uint8 read_external(uint8 address);
    void write_external(uint8 address, uint8 value);
    
    // Bank switching
    void set_bank(uint8 bank);
    uint8 get_current_bank() const { return state_.current_bank; }
    
    // VDC connection
    void set_vdc(VDC* vdc);
    
    // State management
    MemoryState get_state() const;
    void set_state(const MemoryState& state);
    
    // Port 1 control signals
    void update_control_signals(uint8 port1_value);

private:
    MemoryState state_;
    VDC* vdc_;
    
    // Control signals from Port 1
    bool vdc_enabled_;      // P13 = 0
    bool ram_enabled_;      // P14 = 0
    bool copy_mode_;        // P16 = 1, P13 = 0, P14 = 0
    
    // Helper functions
    Result<void> validate_rom_size(size_t size);
    void detect_banking(size_t size);
};

} // namespace videopac

#endif // VIDEOPAC_MEMORY_H
