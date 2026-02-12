#ifndef VIDEOPAC_DEBUGGER_H
#define VIDEOPAC_DEBUGGER_H

#include "types.h"
#include <vector>
#include <string>
#include <cstdint>

namespace videopac {

// Forward declarations
class EmulatorCore;
struct CPUState;

// Breakpoint structure
struct Breakpoint {
    uint16 address;
    bool enabled;
    
    // Conditional breakpoint support
    bool has_condition;
    std::string condition;  // e.g., "A==0xFF", "R0>0x80", "PSW&0x10"
    
    // Condition-only breakpoint (no specific address)
    bool condition_only;
    
    Breakpoint(uint16 addr, bool en = true) 
        : address(addr), enabled(en), has_condition(false), condition(""), condition_only(false) {}
    
    Breakpoint(uint16 addr, const std::string& cond, bool en = true)
        : address(addr), enabled(en), has_condition(true), condition(cond), condition_only(false) {}
    
    // Constructor for condition-only breakpoint
    explicit Breakpoint(const std::string& cond, bool en = true)
        : address(0), enabled(en), has_condition(true), condition(cond), condition_only(true) {}
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
    void add_breakpoint(uint16 address, const std::string& condition);
    void add_breakpoint(const std::string& condition);  // Condition-only breakpoint
    void remove_breakpoint(uint16 address);
    void enable_breakpoint(uint16 address, bool enabled);
    void clear_all_breakpoints();
    bool check_breakpoint(uint16 address) const;
    bool check_condition_breakpoints() const;  // Check condition-only breakpoints
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
    void log_instruction(uint64 current_cycles = 0);
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
    
    // Helper functions
    bool evaluate_condition(const std::string& condition, const CPUState& cpu) const;
};

} // namespace videopac

#endif // VIDEOPAC_DEBUGGER_H
