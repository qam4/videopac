#include "savestate.h"
#include "utils.h"
#include <fstream>
#include <cstring>
#include <cstddef>  // for offsetof

namespace videopac {

Result<void> SaveStateManager::save(const std::string& path, const SaveState& state) {
    // Open file for binary writing
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return Result<void>::err("Failed to open file for writing: " + path);
    }
    
    // Write version
    uint32 version = CURRENT_VERSION;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    
    // Write CPU state
    file.write(reinterpret_cast<const char*>(&state.cpu_state), sizeof(state.cpu_state));
    
    // Write VDC state
    file.write(reinterpret_cast<const char*>(&state.vdc_state), sizeof(state.vdc_state));
    
    // Write Memory state (with special handling for vector)
    // Write fixed-size fields
    file.write(reinterpret_cast<const char*>(state.memory_state.bios_rom), sizeof(state.memory_state.bios_rom));
    file.write(reinterpret_cast<const char*>(state.memory_state.external_ram), sizeof(state.memory_state.external_ram));
    file.write(reinterpret_cast<const char*>(&state.memory_state.current_bank), sizeof(state.memory_state.current_bank));
    file.write(reinterpret_cast<const char*>(&state.memory_state.rom_size_kb), sizeof(state.memory_state.rom_size_kb));
    file.write(reinterpret_cast<const char*>(&state.memory_state.num_banks), sizeof(state.memory_state.num_banks));
    
    // Write cart_rom vector (size + data)
    uint32 cart_rom_size = static_cast<uint32>(state.memory_state.cart_rom.size());
    file.write(reinterpret_cast<const char*>(&cart_rom_size), sizeof(cart_rom_size));
    if (cart_rom_size > 0) {
        file.write(reinterpret_cast<const char*>(state.memory_state.cart_rom.data()), cart_rom_size);
    }
    
    // Write Input state
    file.write(reinterpret_cast<const char*>(&state.input_state), sizeof(state.input_state));
    
    // Write frame count
    file.write(reinterpret_cast<const char*>(&state.frame_count), sizeof(state.frame_count));
    
    // Calculate and write checksum over all written data
    // Close the file first
    file.close();
    
    // Reopen for reading to calculate checksum
    std::ifstream read_file(path, std::ios::binary);
    if (!read_file.is_open()) {
        return Result<void>::err("Failed to reopen file for checksum calculation");
    }
    
    // Read all data
    read_file.seekg(0, std::ios::end);
    std::streampos data_size = read_file.tellg();
    read_file.seekg(0, std::ios::beg);
    
    std::vector<char> buffer(static_cast<size_t>(data_size));
    read_file.read(buffer.data(), data_size);
    read_file.close();
    
    // Calculate checksum
    uint32 checksum = 0;
    for (size_t i = 0; i < buffer.size(); ++i) {
        checksum ^= static_cast<uint8>(buffer[i]);
        checksum = (checksum << 1) | (checksum >> 31);  // Rotate left
    }
    
    // Reopen for appending and write checksum
    std::ofstream append_file(path, std::ios::binary | std::ios::app);
    if (!append_file.is_open()) {
        return Result<void>::err("Failed to reopen file for checksum writing");
    }
    
    append_file.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));
    
    if (!append_file.good()) {
        return Result<void>::err("Failed to write checksum to file");
    }
    
    append_file.close();
    return Result<void>::ok();
}

Result<SaveState> SaveStateManager::load(const std::string& path) {
    // Open file for binary reading
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return Result<SaveState>::err("Failed to open file for reading: " + path);
    }
    
    SaveState state;
    
    // Read version
    uint32 version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (!file.good()) {
        return Result<SaveState>::err("Failed to read version from file");
    }
    
    // Verify version
    if (version != CURRENT_VERSION) {
        return Result<SaveState>::err("Incompatible save state version");
    }
    state.version = version;
    
    // Read CPU state
    file.read(reinterpret_cast<char*>(&state.cpu_state), sizeof(state.cpu_state));
    
    // Read VDC state
    file.read(reinterpret_cast<char*>(&state.vdc_state), sizeof(state.vdc_state));
    
    // Read Memory state (with special handling for vector)
    // Read fixed-size fields
    file.read(reinterpret_cast<char*>(state.memory_state.bios_rom), sizeof(state.memory_state.bios_rom));
    file.read(reinterpret_cast<char*>(state.memory_state.external_ram), sizeof(state.memory_state.external_ram));
    file.read(reinterpret_cast<char*>(&state.memory_state.current_bank), sizeof(state.memory_state.current_bank));
    file.read(reinterpret_cast<char*>(&state.memory_state.rom_size_kb), sizeof(state.memory_state.rom_size_kb));
    file.read(reinterpret_cast<char*>(&state.memory_state.num_banks), sizeof(state.memory_state.num_banks));
    
    // Read cart_rom vector (size + data)
    uint32 cart_rom_size;
    file.read(reinterpret_cast<char*>(&cart_rom_size), sizeof(cart_rom_size));
    if (!file.good()) {
        return Result<SaveState>::err("Failed to read cart ROM size from file");
    }
    
    // Sanity check: ROM size should be reasonable (max 8KB = 8192 bytes)
    if (cart_rom_size > 8192) {
        return Result<SaveState>::err("Invalid cart ROM size in save state");
    }
    
    if (cart_rom_size > 0) {
        state.memory_state.cart_rom.resize(cart_rom_size);
        file.read(reinterpret_cast<char*>(state.memory_state.cart_rom.data()), cart_rom_size);
    }
    
    // Read Input state
    file.read(reinterpret_cast<char*>(&state.input_state), sizeof(state.input_state));
    
    // Read frame count
    file.read(reinterpret_cast<char*>(&state.frame_count), sizeof(state.frame_count));
    
    // Read checksum
    uint32 stored_checksum;
    file.read(reinterpret_cast<char*>(&stored_checksum), sizeof(stored_checksum));
    
    if (!file.good()) {
        return Result<SaveState>::err("Failed to read save state from file");
    }
    
    // Calculate checksum over all data except the checksum itself
    file.seekg(0, std::ios::end);
    std::streampos total_size = file.tellg();
    std::streampos data_size = total_size - static_cast<std::streamoff>(sizeof(uint32));
    file.seekg(0, std::ios::beg);
    
    std::vector<char> buffer(static_cast<size_t>(data_size));
    file.read(buffer.data(), data_size);
    
    uint32 calculated_checksum = 0;
    for (size_t i = 0; i < buffer.size(); ++i) {
        calculated_checksum ^= static_cast<uint8>(buffer[i]);
        calculated_checksum = (calculated_checksum << 1) | (calculated_checksum >> 31);  // Rotate left
    }
    
    file.close();
    
    // Verify checksum
    if (stored_checksum != calculated_checksum) {
        return Result<SaveState>::err("Save state checksum verification failed");
    }
    
    state.checksum = stored_checksum;
    
    return Result<SaveState>::ok(state);
}

uint32 SaveStateManager::calculate_checksum(const SaveState& state) {
    // Calculate checksum over all fields except the checksum itself
    // The checksum field is at the end of the structure
    const uint8* data = reinterpret_cast<const uint8*>(&state);
    size_t size = offsetof(SaveState, checksum);  // Size up to (but not including) checksum
    
    return utils::calculate_checksum(data, size);
}

bool SaveStateManager::verify_checksum(const SaveState& state) {
    uint32 stored_checksum = state.checksum;
    uint32 calculated_checksum = calculate_checksum(state);
    return stored_checksum == calculated_checksum;
}

} // namespace videopac
