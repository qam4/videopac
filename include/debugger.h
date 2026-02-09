#ifndef VIDEOPAC_DEBUGGER_H
#define VIDEOPAC_DEBUGGER_H

#include "types.h"
#include <vector>

namespace videopac {

// Forward declaration
class EmulatorCore;

// Breakpoint
struct Breakpoint {
    uint16 address;
    bool enabled;
};

// Debugger
class Debugger {
public:
    explicit Debugger(EmulatorCore* emulator);
    ~Debugger() = default;
    
    // Breakpoints
    void add_breakpoint(uint16 address);
    void remove_breakpoint(uint16 address);
    bool check_breakpoint(uint16 address);
    
    // Execution control
    void step();
    void continue_execution();
    void pause();
    
    // Inspection
    void dump_cpu_state();
    void dump_memory(uint16 start, uint16 end);
    void dump_vdc_registers();
    
    // Trace
    void enable_trace(bool enabled);
    void log_instruction();

private:
    EmulatorCore* emulator_;
    std::vector<Breakpoint> breakpoints_;
    bool trace_enabled_;
};

} // namespace videopac

#endif // VIDEOPAC_DEBUGGER_H
