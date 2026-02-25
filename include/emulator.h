#ifndef VIDEOPAC_EMULATOR_H
#define VIDEOPAC_EMULATOR_H

#include "types.h"
#include "cpu.h"
#include "vdc.h"
#include "memory.h"
#include "input.h"
#include "master_clock.h"
#include <memory>

namespace videopac {

// Forward declaration
class Debugger;

// Configuration
struct Configuration {
    VideoStandard video_standard;
    std::string bios_path;
    bool enable_profile;
    
    Configuration() : video_standard(VideoStandard::NTSC), enable_profile(false) {}
};

// Emulator core
class EmulatorCore {
public:
    explicit EmulatorCore(const Configuration& config);
    ~EmulatorCore() = default;
    
    // ROM loading
    Result<void> load_bios(const std::string& path);
    Result<void> load_bios(const uint8* data, size_t size);
    Result<void> load_rom(const std::string& path);
    Result<void> load_rom(const uint8* data, size_t size);
    
    // Emulation control
    void reset();
    void run_frame();
    void step();  // Single instruction for debugging
    
    // Output
    const uint8* get_framebuffer() const;
    void get_audio_buffer(int16* buffer, size_t samples);
    
    // Input
    void set_input(const InputState& input);
    InputHandler& get_input_handler() { return input_; }
    
    // State management
    Result<void> save_state(const std::string& path);
    Result<void> load_state(const std::string& path);
    
    // Status
    bool is_running() const { return running_; }
    bool is_paused() const { return paused_; }
    void set_paused(bool paused) { paused_ = paused; }
    uint64 get_frame_count() const { return frame_count_; }
    
    // Component access (for debugging)
    CPU& get_cpu() { return cpu_; }
    VDC& get_vdc() { return vdc_; }
    MemorySystem& get_memory() { return memory_; }
    MasterClock& get_master_clock() { return master_clock_; }
    const MasterClock& get_master_clock() const { return master_clock_; }
    
    // State access (for debugger inspection)
    CPUState get_cpu_state() const { return cpu_.get_state(); }
    VDCState get_vdc_state() const { return vdc_.get_state(); }
    MemoryState get_memory_state() const { return memory_.get_state(); }
    
    // Debugger integration
    void set_debugger(Debugger* debugger) { debugger_ = debugger; }
    Debugger* get_debugger() { return debugger_; }

private:
    Configuration config_;
    
    // Components
    CPU cpu_;
    VDC vdc_;
    MemorySystem memory_;
    InputHandler input_;
    MasterClock master_clock_;
    
    // Debugger (optional)
    Debugger* debugger_;
    
    // State
    bool running_;
    bool paused_;
    uint64 frame_count_;
    bool vblank_interrupt_triggered_;  // Track if VBlank interrupt fired this frame
    uint16 prev_scanline_;             // Track previous scanline for counter mode
    
    // Helpers
    void handle_interrupts();
    void check_debugger_breakpoint();
    void log_debugger_trace();
};

} // namespace videopac

#endif // VIDEOPAC_EMULATOR_H
