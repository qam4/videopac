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
    
    // Port 1 value (control signals for memory/VDC access)
    // Reference: doc/port1_bits.md for bit definitions
    uint8 port1_;
    
    // Port 1 bit masks (from BIOS naming convention)
    // Reference: doc/french_bios_annotated.txt
    static constexpr uint8 P1_KBSCAN = 0x04;  // P12: Keyboard scan enable (active low)
    static constexpr uint8 P1_VDCEN  = 0x08;  // P13: VDC enable (active low)
    static constexpr uint8 P1_RAMEN  = 0x10;  // P14: RAM enable (active low)
    static constexpr uint8 P1_COPYEN = 0x40;  // P16: Copy mode enable (active high)
    static constexpr uint8 P1_LUMEN  = 0x80;  // P17: Luminance enable (active high)
    
    // Helper methods to check Port 1 control signals
    // These return true when the device is ENABLED (active-low signals inverted for clarity)
    inline bool vdc_enabled() const { return !(port1_ & P1_VDCEN); }  // P13=0 enables VDC
    inline bool ram_enabled() const { return !(port1_ & P1_RAMEN); }  // P14=0 enables RAM
    inline bool copy_mode() const { return (port1_ & P1_COPYEN) != 0; }  // P16=1 enables copy mode
    
    // Helper functions
    Result<void> validate_rom_size(size_t size);
    void detect_banking(size_t size);
};

} // namespace videopac

#endif // VIDEOPAC_MEMORY_H
