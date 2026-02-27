#ifndef VIDEOPAC_CPU_H
#define VIDEOPAC_CPU_H

#include "types.h"

namespace videopac {

// Forward declarations
class MemorySystem;
class InputHandler;

// CPU state structure
// Internal RAM layout (matches Intel 8048 / o2em):
//   intRAM[0..7]   = Register bank 0 (R0-R7)
//   intRAM[8..23]  = Stack (8 levels × 2 bytes each)
//   intRAM[24..31] = Register bank 1 (R0'-R7')
//   intRAM[32..63] = General purpose RAM
// All registers, stack, and RAM share the same 64-byte array.
// This is critical because game code can read/write stack entries via RAM addressing,
// and register writes are visible to anything reading those RAM addresses.
struct CPUState {
    uint16 pc;                  // Program counter (12-bit, 0x000-0xFFF)
    uint8 a;                    // Accumulator
    uint8 psw;                  // Program Status Word
    uint8 ram[64];              // Unified internal RAM (registers + stack + general RAM)
    uint8 sp;                   // Stack pointer (8-23, byte index into ram[])
    uint8 port1;                // Port 1 state
    uint8 port2;                // Port 2 state
    uint8 timer;                // Timer/counter register
    bool timer_on;              // Timer mode enabled (increments every 32 cycles)
    bool counter_on;            // Counter mode enabled (increments once per scanline)
    bool interrupts_enabled;    // Interrupts enabled
    bool timer_interrupts_enabled;  // Timer interrupts enabled
    bool timer_flag;            // Timer overflow flag (TF) - set when timer overflows from 0xFF to 0x00
                                // Can be tested with JTF instruction, which also clears it
    uint8 timer_prescaler;      // Timer prescaler counter (0-31)
                                // Reference: doc/mcs-48-assembly-language-manual.md, "Timer Flag" section
                                // The timer increments every 32 instruction cycles via this prescaler
    uint64 clock_cycles;        // Total cycles executed
    uint8 current_bank;         // Current register bank (0 or 1)
                                // Controlled by SEL RB0/RB1 instructions (PSW bit 4 = BS)
    bool f1_flag;               // F1 flag (separate from PSW) - user flag, can be tested with JF1
                                // NOTE: F1 is NOT in PSW! F0 is in PSW bit 5, but F1 is separate.
                                // NOTE: F1 is NOT the same as Bank Select (BS)! They are independent.
    bool memory_bank;           // Memory bank flag (DBF): false=MB0 (0x000-0x7FF), true=MB1 (0x800-0xFFF)
                                // Set by SEL MB0/MB1 instructions, affects JMP/CALL target addresses
    bool memory_bank_saved;     // Saved memory bank flag (A11ff in o2em)
                                // On interrupt entry, memory_bank is saved here and cleared to 0
                                // On RETR, memory_bank is restored from this field
                                // Reference: Intel 8048 spec - interrupts force A11=0
    
    // T1 pin state (for counter mode)
    // Reference: doc/hardware/odyssey2_timing.txt "T1 input caveat" section
    // T1 is HIGH during visible period, LOW during blanking
    // true = T1 HIGH (visible), false = T1 LOW (blanking)
    bool t1_state;              // Current T1 pin state
    
    // Pending interrupt flags
    // Reference: Intel 8048 User Manual, page 3107:
    // "The Interrupt line is sampled every machine cycle during ALE and when detected
    //  causes a 'jump to subroutine' at location 3 in program memory as soon as all
    //  cycles of the current instruction are complete."
    // This means interrupts are sampled every cycle but only processed between instructions.
    bool timer_interrupt_pending;   // Timer interrupt pending (will be processed after current instruction)
    bool external_interrupt_pending; // External interrupt pending (will be processed after current instruction)
    bool in_interrupt;              // Currently executing an interrupt handler (irq_ex in o2em)
                                    // Set when interrupt fires, cleared by RETR
                                    // Prevents nested interrupts and affects SEL MB1 behavior
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
    void set_input_handler(InputHandler* input);
    uint8 read_memory(uint16 address);
    void write_memory(uint16 address, uint8 value);
    
    // I/O port access
    uint8 read_port(uint8 port);
    void write_port(uint8 port, uint8 value);
    
    // Interrupt handling
    // Returns number of cycles consumed (2 if interrupt fired, 0 if interrupts disabled)
    uint8 trigger_interrupt(uint16 vector);
    void set_external_interrupt_pending(bool pending) { state_.external_interrupt_pending = pending; }
    
    // Timer/counter management
    // Hardware: Simulates T1 pin pulse from VDC
    // Counter mode increments on T1 falling edges (T1 going from active to inactive)
    // Reference: doc/hardware/odyssey2_timing.txt "T1 input caveat" section
    void update_counter(bool t1_state);
    
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
    InputHandler* input_;
    
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
