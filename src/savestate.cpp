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
    
    // Create a copy with version and calculated checksum
    SaveState state_with_checksum = state;
    state_with_checksum.version = CURRENT_VERSION;
    // Calculate checksum (will only hash data before checksum field)
    state_with_checksum.checksum = calculate_checksum(state_with_checksum);
    
    // Write the entire structure
    file.write(reinterpret_cast<const char*>(&state_with_checksum), sizeof(SaveState));
    
    if (!file.good()) {
        return Result<void>::err("Failed to write save state to file");
    }
    
    file.close();
    return Result<void>::ok();
}

Result<SaveState> SaveStateManager::load(const std::string& path) {
    // Open file for binary reading
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return Result<SaveState>::err("Failed to open file for reading: " + path);
    }
    
    // Read the entire structure
    SaveState state;
    file.read(reinterpret_cast<char*>(&state), sizeof(SaveState));
    
    if (!file.good()) {
        return Result<SaveState>::err("Failed to read save state from file");
    }
    
    file.close();
    
    // Verify version
    if (state.version != CURRENT_VERSION) {
        return Result<SaveState>::err("Incompatible save state version");
    }
    
    // Verify checksum
    if (!verify_checksum(state)) {
        return Result<SaveState>::err("Save state checksum verification failed");
    }
    
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
