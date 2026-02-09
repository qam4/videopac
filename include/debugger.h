#ifndef VIDEOPAC_DEBUGGER_H
#define VIDEOPAC_DEBUGGER_H

#include "types.h"
#include <vector>
#include <string>
#include <cstdint>

namespace videopac {

// Forward declaration
class EmulatorCore;

// Breakpoint structure
struct Breakpoint {
    uint16 address;
    bool enabled;
    
    Breakpoint(uint16 addr, bool en = true) : address(addr), enabled(en) {}
};

// Debugger state
enum class DebuggerState {
    Running,
    Paused,
    StepMode,
    TraceMode
};

// Frame timing statistics
struct FrameStats {
    uint64 total_cycles;
    uint64 frame_count;
    double fps;
    double average_cycles_per_frame;
};

// Debugger class
class Debugger {
public:
    explicit Debugger(EmulatorCore* emulator);
    ~Debugger() = default;
    
    // Breakpoint management
    void add_breakpoint(uint16 address);
    void remove_breakpoint(uint16 address);
    void enable_breakpoint(uint16 address, bool enabled);
    void clear_all_breakpoints();
    bool check_breakpoint(uint16 address) const;
    const std::vector<Breakpoint>& get_breakpoints() const { return breakpoints_; }
    
    // Execution control
    void step();                    // Execute single instruction
    void continue_execution();      // Resume execution
    void pause();                   // Pause execution
    bool is_paused() const { return state_ == DebuggerState::Paused; }
    DebuggerState get_state() const { return state_; }
    
    // Inspection
    std::string dump_cpu_state() const;
    std::string dump_memory(uint16 start, uint16 end) const;
    std::string dump_vdc_registers() const;
    std::string disassemble_at_pc(int lines_before = 5, int lines_after = 5) const;
    
    // Trace logging
    void enable_trace(bool enabled);
    bool is_trace_enabled() const { return trace_enabled_; }
    void log_instruction();
    const std::vector<std::string>& get_trace_log() const { return trace_log_; }
    void clear_trace_log();
    
    // Frame timing statistics
    void update_frame_stats(uint64 cycles);
    FrameStats get_frame_stats() const { return frame_stats_; }
    void reset_frame_stats();

private:
    EmulatorCore* emulator_;
    std::vector<Breakpoint> breakpoints_;
    DebuggerState state_;
    bool trace_enabled_;
    std::vector<std::string> trace_log_;
    FrameStats frame_stats_;
    uint64 last_frame_time_;
};

} // namespace videopac

#endif // VIDEOPAC_DEBUGGER_H
