#ifndef VIDEOPAC_SAVESTATE_H
#define VIDEOPAC_SAVESTATE_H

#include "types.h"
#include "cpu.h"
#include "vdc.h"
#include "memory.h"
#include "input.h"

namespace videopac {

// Save state structure
struct SaveState {
    uint32 version;
    CPUState cpu_state;
    VDCState vdc_state;
    MemoryState memory_state;
    InputState input_state;
    uint64 frame_count;
    uint32 checksum;
};

// Save state management
class SaveStateManager {
public:
    static Result<void> save(const std::string& path, const SaveState& state);
    static Result<SaveState> load(const std::string& path);
    
private:
    static constexpr uint32 CURRENT_VERSION = 1;
    static uint32 calculate_checksum(const SaveState& state);
    static bool verify_checksum(const SaveState& state);
};

} // namespace videopac

#endif // VIDEOPAC_SAVESTATE_H
