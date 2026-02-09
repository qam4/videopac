#ifndef VIDEOPAC_CPU_H
#define VIDEOPAC_CPU_H

#include "types.h"

namespace videopac {

// Forward declarations
class MemorySystem;

// CPU state structure
struct CPUState {
    uint16 pc;                  // Program counter (12-bit, 0x000-0xFFF)
    uint8 a;                    // Accumulator
    uint8 psw;                  // Program Status Word
    uint8 r[16];                // Registers R0-R7 and R0'-R7'
    uint8 ram[64];              // Internal RAM (64 bytes)
    uint16 stack[8];            // 8-level stack
    uint8 sp;                   // Stack pointer (0-7)
    uint8 port1;                // Port 1 state
    uint8 port2;                // Port 2 state
    uint8 timer;                // Timer/counter register
    bool timer_running;         // Timer enabled flag
    bool interrupts_enabled;    // Interrupts enabled
    bool timer_interrupts_enabled;  // Timer interrupts enabled
    uint8 timer_prescaler;      // Timer prescaler counter (0-31)
                                // Reference: doc/mcs-48-assembly-language-manual.md, "Timer Flag" section
                                // The timer increments every 32 instruction cycles via this prescaler
    uint64 clock_cycles;        // Total cycles executed
    uint8 current_bank;         // Current register bank (0 or 1)
};

// Intel 8048 CPU emulation
class CPU {
public:
    CPU();
    ~CPU() = default;
    
    // Core interface
    void reset();
    uint8 execute_instruction();
    
    // Memory access (connected to memory system)
    void set_memory_system(MemorySystem* mem);
    uint8 read_memory(uint16 address);
    void write_memory(uint16 address, uint8 value);
    
    // I/O port access
    uint8 read_port(uint8 port);
    void write_port(uint8 port, uint8 value);
    
    // Interrupt handling
    void trigger_interrupt(uint16 vector);
    
    // State management
    CPUState get_state() const;
    void set_state(const CPUState& state);
    
    // Accessors
    uint16 get_pc() const { return state_.pc; }
    uint8 get_accumulator() const { return state_.a; }
    uint64 get_cycles() const { return state_.clock_cycles; }

private:
    CPUState state_;
    MemorySystem* memory_;
    
    // Instruction execution helpers
    uint8 fetch_byte();
    void push_stack(uint16 value);
    uint16 pop_stack();
    
    // Instruction implementations (to be implemented)
    void execute_mov();
    void execute_add();
    void execute_jmp();
    // ... more instruction handlers
};

} // namespace videopac

#endif // VIDEOPAC_CPU_H
