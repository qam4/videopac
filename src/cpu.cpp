#include "cpu.h"
#include "memory.h"
#include "input.h"
#include <cstring>

namespace videopac {

CPU::CPU() : memory_(nullptr), input_(nullptr) {
    reset();
}

void CPU::reset() {
    std::memset(&state_, 0, sizeof(state_));
    state_.pc = 0x000;
    state_.sp = 0;
}

void CPU::set_memory_system(MemorySystem* mem) {
    memory_ = mem;
}

void CPU::set_input_handler(InputHandler* input) {
    input_ = input;
}

uint8 CPU::read_memory(uint16 address) {
    if (memory_) {
        return memory_->read_program(address);
    }
    return 0xFF;
}

void CPU::write_memory(uint16 address, uint8 value) {
    if (memory_) {
        memory_->write_external(static_cast<uint8>(address), value);
    }
}

uint8 CPU::read_port(uint8 port) {
    if (port == 1) {
        return state_.port1;
    } else if (port == 2) {
        // Port 2 reads keyboard input
        // P12 (bit 2 of Port 1) must be 0 to enable keyboard
        // P20-P22 (bits 0-2 of Port 2) select the row
        if (input_ && !(state_.port1 & 0x04)) {  // P12 == 0
            uint8 selected_row = state_.port2 & 0x07;  // P20-P22
            uint8 result = input_->read_keyboard(selected_row);
            return result;
        }
        // Keyboard disabled or no input handler - return last written value
        return state_.port2;
    }
    return 0xFF;
}

void CPU::write_port(uint8 port, uint8 value) {
    if (port == 1) {
        state_.port1 = value;
        if (memory_) {
            memory_->update_control_signals(value);
        }
    } else if (port == 2) {
        state_.port2 = value;
    }
}

void CPU::trigger_interrupt(uint16 vector) {
    if (state_.interrupts_enabled) {
        // Push PC (12 bits) and PSW bits 4-7 (4 bits) as a single 16-bit value
        uint16 stack_value = (state_.pc & 0x0FFF) | ((state_.psw & 0xF0) << 8);
        push_stack(stack_value);
        state_.pc = vector;
    }
}

CPUState CPU::get_state() const {
    return state_;
}

void CPU::set_state(const CPUState& state) {
    state_ = state;
}

uint8 CPU::fetch_byte() {
    uint8 byte = read_memory(state_.pc);
    state_.pc = (state_.pc + 1) & 0xFFF;  // 12-bit PC
    return byte;
}

void CPU::push_stack(uint16 value) {
    state_.stack[state_.sp] = value;
    state_.sp = (state_.sp + 1) & 0x07;  // Wrap at 8 levels
}

uint16 CPU::pop_stack() {
    state_.sp = (state_.sp - 1) & 0x07;
    return state_.stack[state_.sp];
}

// Helper function to get the correct register index based on current bank
// The 8048 has two register banks (RB0 and RB1), each with 8 registers (R0-R7)
// RB0: registers 0-7, RB1: registers 8-15
inline uint8 get_register_index(uint8 reg_num, uint8 current_bank) {
    return current_bank * 8 + (reg_num & 0x07);
}

// Intel 8048 Instruction Execution
// This function decodes and executes a single 8048 instruction.
// The 8048 has 96 instructions organized into several categories:
// - Data Transfer (MOV, MOVX, MOVP, XCH, XCHD, SWAP)
// - Arithmetic (ADD, ADDC, INC, DEC, DA)
// - Logical (ANL, ORL, XRL, CLR, CPL)
// - Branch (JMP, DJNZ, JC, JNC, JZ, JNZ, JT0, JNT0, JT1, JNT1, JF0, JF1, JTF, JBb)
// - Subroutine (CALL, RET, RETR)
// - I/O (IN, OUT, INS, OUTL, ANL, ORL)
// - Control (NOP, EN I, DIS I, EN TCNTI, DIS TCNTI, SEL RB0, SEL RB1, SEL MB0, SEL MB1)
//
// Returns: Number of machine cycles consumed by the instruction
uint8 CPU::execute_instruction() {
    uint8 opcode = fetch_byte();
    uint8 cycles = 1;  // Default cycle count (most instructions are 1 cycle)
    
    // Decode and execute instruction based on opcode
    switch (opcode) {
        // ========== CONTROL INSTRUCTIONS ==========
        
        // NOP - No Operation (0x00)
        // Does nothing, consumes 1 cycle
        case 0x00:
            break;
            
        // ========== ARITHMETIC INSTRUCTIONS ==========
        
        // ADD A,@Rr - Add data memory to accumulator (0x60-0x61)
        // Operation: (A) <- (A) + ((Rr))
        // Flags affected: C (Carry), AC (Auxiliary Carry)
        // Cycles: 1
        // The contents of the data memory location addressed by R0 or R1 are added to the accumulator.
        // The carry and auxiliary carry flags are set if there is a carry out of bit 7 or bit 3 respectively.
        case 0x60: case 0x61: {
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            uint8 value = (addr < 64) ? state_.ram[addr] : 0xFF;
            uint16 result = state_.a + value;
            // Update PSW: C (bit 7) = 1 if result > 255, AC (bit 6) = 1 if lower nibble overflows
            state_.psw = (state_.psw & 0x3F) | ((result & 0x100) ? 0x80 : 0) | ((((state_.a & 0x0F) + (value & 0x0F)) & 0x10) ? 0x40 : 0);
            state_.a = result & 0xFF;
            break;
        }
        
        // ADD A,Rr - Add register to accumulator (0x68-0x6F)
        // Operation: (A) <- (A) + (Rr)
        // Flags affected: C (Carry), AC (Auxiliary Carry)
        // Cycles: 1
        // The contents of the specified working register (R0-R7) are added to the accumulator.
        case 0x68: case 0x69: case 0x6A: case 0x6B:
        case 0x6C: case 0x6D: case 0x6E: case 0x6F: {
            uint8 reg_idx = get_register_index(opcode & 0x07, state_.current_bank);
            uint8 value = state_.r[reg_idx];
            uint16 result = state_.a + value;
            // Update PSW: C (bit 7) = 1 if result > 255, AC (bit 6) = 1 if lower nibble overflows
            state_.psw = (state_.psw & 0x3F) | ((result & 0x100) ? 0x80 : 0) | ((((state_.a & 0x0F) + (value & 0x0F)) & 0x10) ? 0x40 : 0);
            state_.a = result & 0xFF;
            break;
        }
        
        // ADD A,#data - Add immediate data to accumulator (0x03)
        // Operation: (A) <- (A) + data
        // Flags affected: C (Carry), AC (Auxiliary Carry)
        // Cycles: 2
        // The immediate data byte following the opcode is added to the accumulator.
        case 0x03: {
            uint8 value = fetch_byte();
            uint16 result = state_.a + value;
            // Update PSW: C (bit 7) = 1 if result > 255, AC (bit 6) = 1 if lower nibble overflows
            state_.psw = (state_.psw & 0x3F) | ((result & 0x100) ? 0x80 : 0) | ((((state_.a & 0x0F) + (value & 0x0F)) & 0x10) ? 0x40 : 0);
            state_.a = result & 0xFF;
            cycles = 2;
            break;
        }
        
        // ADDC A,@Rr - Add data memory and carry to accumulator (0x70-0x71)
        // Operation: (A) <- (A) + ((Rr)) + (C)
        // Flags affected: C (Carry), AC (Auxiliary Carry)
        // Cycles: 1
        // The contents of the data memory location and the carry bit are added to the accumulator.
        case 0x70: case 0x71: {
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            uint8 value = (addr < 64) ? state_.ram[addr] : 0xFF;
            uint8 carry = (state_.psw & 0x80) ? 1 : 0;
            uint16 result = state_.a + value + carry;
            // Update PSW: C (bit 7) = 1 if result > 255, AC (bit 6) = 1 if lower nibble overflows
            state_.psw = (state_.psw & 0x3F) | ((result & 0x100) ? 0x80 : 0) | ((((state_.a & 0x0F) + (value & 0x0F) + carry) & 0x10) ? 0x40 : 0);
            state_.a = result & 0xFF;
            break;
        }
        
        // ADDC A,Rr - Add register and carry to accumulator (0x78-0x7F)
        // Operation: (A) <- (A) + (Rr) + (C)
        // Flags affected: C (Carry), AC (Auxiliary Carry)
        // Cycles: 1
        case 0x78: case 0x79: case 0x7A: case 0x7B:
        case 0x7C: case 0x7D: case 0x7E: case 0x7F: {
            uint8 reg_idx = get_register_index(opcode & 0x07, state_.current_bank);
            uint8 value = state_.r[reg_idx];
            uint8 carry = (state_.psw & 0x80) ? 1 : 0;
            uint16 result = state_.a + value + carry;
            // Update PSW: C (bit 7) = 1 if result > 255, AC (bit 6) = 1 if lower nibble overflows
            state_.psw = (state_.psw & 0x3F) | ((result & 0x100) ? 0x80 : 0) | ((((state_.a & 0x0F) + (value & 0x0F) + carry) & 0x10) ? 0x40 : 0);
            state_.a = result & 0xFF;
            break;
        }
        
        // ADDC A,#data - Add immediate data and carry to accumulator (0x13)
        // Operation: (A) <- (A) + data + (C)
        // Flags affected: C (Carry), AC (Auxiliary Carry)
        // Cycles: 2
        case 0x13: {
            uint8 value = fetch_byte();
            uint8 carry = (state_.psw & 0x80) ? 1 : 0;
            uint16 result = state_.a + value + carry;
            // Update PSW: Set C (bit 7) if carry from bit 7, AC (bit 6) if carry from bit 3
            state_.psw = (state_.psw & 0x3F) | ((result & 0x100) ? 0x80 : 0) | ((((state_.a & 0x0F) + (value & 0x0F) + carry) & 0x10) ? 0x40 : 0);
            state_.a = result & 0xFF;
            cycles = 2;
            break;
        }
        
        // ========== LOGICAL INSTRUCTIONS ==========
        
        // ANL A,@Rr - AND data memory with accumulator (0x50-0x51)
        // Operation: (A) <- (A) AND ((Rr))
        // Flags affected: None
        // Cycles: 1
        // Performs bitwise AND between accumulator and data memory location.
        case 0x50: case 0x51: {
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            uint8 value = (addr < 64) ? state_.ram[addr] : 0xFF;
            state_.a &= value;
            break;
        }
        
        // ANL A,Rr - AND register with accumulator (0x58-0x5F)
        // Operation: (A) <- (A) AND (Rr)
        // Flags affected: None
        // Cycles: 1
        case 0x58: case 0x59: case 0x5A: case 0x5B:
        case 0x5C: case 0x5D: case 0x5E: case 0x5F: {
            uint8 reg_idx = get_register_index(opcode & 0x07, state_.current_bank);
            state_.a &= state_.r[reg_idx];
            break;
        }
            
        // ANL A,#data - AND immediate data with accumulator (0x53)
        // Operation: (A) <- (A) AND data
        // Flags affected: None
        // Cycles: 2
        case 0x53:
            state_.a &= fetch_byte();
            cycles = 2;
            break;
            
        // ANL BUS,#data - AND immediate data with BUS (0x98)
        // Operation: (BUS) <- (BUS) AND data
        // Flags affected: None
        // Cycles: 2
        // Performs bitwise AND between external BUS and immediate data mask.
        case 0x98: {
            (void)fetch_byte();  // Read mask (BUS operations would interact with external bus)
            cycles = 2;
            break;
        }
        
        // ANL Pp,#data - AND immediate data with port (0x99-0x9A)
        // Operation: (Pp) <- (Pp) AND data
        // Flags affected: None
        // Cycles: 2
        // Performs bitwise AND between port P1 or P2 and immediate data mask.
        case 0x99: case 0x9A: {
            uint8 mask = fetch_byte();
            uint8 port = (opcode & 0x03);
            if (port == 1) state_.port1 &= mask;
            else if (port == 2) state_.port2 &= mask;
            cycles = 2;
            break;
        }
        
        // ========== SUBROUTINE CALL/RETURN INSTRUCTIONS ==========
        
        // CALL addr - Call subroutine (0x14, 0x34, 0x54, 0x74, 0x94, 0xB4, 0xD4, 0xF4)
        // Operation: ((SP)) <- (PC) | (PSW4-7 << 12), (PC) <- addr
        // Flags affected: None
        // Cycles: 2
        // Stores the current PC (12 bits) and PSW bits 4-7 (4 bits) in a single 16-bit stack entry.
        // Format: bits 0-11 = PC, bits 12-15 = PSW bits 4-7
        // The address is formed by combining bits 5-7 of the opcode with the following byte.
        // Stack pointer is incremented after storing.
        case 0x14: case 0x34: case 0x54: case 0x74:
        case 0x94: case 0xB4: case 0xD4: case 0xF4: {
            uint8 addr_low = fetch_byte();
            uint16 addr = ((opcode & 0xE0) << 3) | addr_low;
            // Push PC (12 bits) and PSW bits 4-7 (4 bits) as a single 16-bit value
            uint16 stack_value = (state_.pc & 0x0FFF) | ((state_.psw & 0xF0) << 8);
            push_stack(stack_value);
            state_.pc = addr;
            cycles = 2;
            break;
        }
        
        // ========== FLAG AND ACCUMULATOR MANIPULATION ==========
        
        // CLR A - Clear accumulator (0x27)
        // Operation: (A) <- 0
        // Flags affected: None
        // Cycles: 1
        case 0x27:
            state_.a = 0;
            break;
            
        // CLR C - Clear carry flag (0x97)
        // Operation: (C) <- 0
        // Flags affected: C
        // Cycles: 1
        case 0x97:
            state_.psw &= 0x7F;  // Clear bit 7 (carry flag)
            break;
            
        // CLR F0 - Clear flag 0 (0x85)
        // Operation: (F0) <- 0
        // Flags affected: F0
        // Cycles: 1
        case 0x85:
            state_.psw &= 0xDF;  // Clear bit 5 (F0 flag)
            break;
            
        // CLR F1 - Clear flag 1 (0xA5)
        // Operation: (F1) <- 0
        // Flags affected: F1
        // Cycles: 1
        case 0xA5:
            state_.psw &= 0xEF;  // Clear bit 4 (F1 flag)
            state_.current_bank = 0;  // F1=0 means Bank 0
            break;
            
        // CPL A - Complement accumulator (0x37)
        // Operation: (A) <- NOT (A)
        // Flags affected: None
        // Cycles: 1
        // Performs one's complement (bitwise NOT) on the accumulator.
        case 0x37:
            state_.a = ~state_.a;
            break;
            
        // CPL C - Complement carry flag (0xA7)
        // Operation: (C) <- NOT (C)
        // Flags affected: C
        // Cycles: 1
        case 0xA7:
            state_.psw ^= 0x80;  // Toggle bit 7 (carry flag)
            break;
            
        // CPL F0 - Complement flag 0 (0x95)
        // Operation: (F0) <- NOT (F0)
        // Flags affected: F0
        // Cycles: 1
        case 0x95:
            state_.psw ^= 0x20;  // Toggle bit 5 (F0 flag)
            break;
            
        // CPL F1 - Complement flag 1 (0xB5)
        // Operation: (F1) <- NOT (F1)
        // Flags affected: F1
        // Cycles: 1
        case 0xB5:
            state_.psw ^= 0x10;  // Toggle bit 4 (F1 flag)
            state_.current_bank = (state_.psw & 0x10) ? 1 : 0;  // Update bank
            break;
            
        // DA A - Decimal Adjust Accumulator (0x57)
        // Operation: Adjust accumulator for BCD addition
        // Flags affected: C
        // Cycles: 1
        // Adjusts the 8-bit value in the accumulator to form two 4-bit BCD digits.
        // Used after ADD or ADDC instructions when performing BCD arithmetic.
        // If lower nibble > 9 or AC set, adds 0x06 to lower nibble.
        // If upper nibble > 9 or C set, adds 0x60 to upper nibble and sets carry.
        case 0x57: {
            uint8 correction = 0;
            if ((state_.a & 0x0F) > 9 || (state_.psw & 0x40)) {
                correction = 0x06;
            }
            if ((state_.a & 0xF0) > 0x90 || (state_.psw & 0x80) || ((state_.a & 0xF0) >= 0x90 && (state_.a & 0x0F) > 9)) {
                correction |= 0x60;
                state_.psw |= 0x80;  // Set carry
            } else {
                state_.psw &= 0x7F;  // Clear carry
            }
            state_.a += correction;
            break;
        }
        
        // DEC A - Decrement accumulator (0x07)
        // Operation: (A) <- (A) - 1
        // Flags affected: None
        // Cycles: 1
        case 0x07:
            state_.a--;
            break;
            
        // DEC Rr - Decrement register (0xC8-0xCF)
        // Operation: (Rr) <- (Rr) - 1
        // Flags affected: None
        // Cycles: 1
        case 0xC8: case 0xC9: case 0xCA: case 0xCB:
        case 0xCC: case 0xCD: case 0xCE: case 0xCF: {
            uint8 reg_idx = get_register_index(opcode & 0x07, state_.current_bank);
            state_.r[reg_idx]--;
            break;
        }
            
        // ========== INTERRUPT AND TIMER CONTROL ==========
        
        // DIS I - Disable external interrupt (0x15)
        // Operation: Disable external interrupt input
        // Flags affected: None
        // Cycles: 1
        // Disables the external interrupt input pin from generating interrupts.
        case 0x15:
            state_.interrupts_enabled = false;
            break;
            
        // DIS TCNTI - Disable timer/counter interrupt (0x35)
        // Operation: Disable timer/counter interrupt
        // Flags affected: None
        // Cycles: 1
        // Disables timer/counter overflow from generating interrupts.
        case 0x35:
            state_.timer_interrupts_enabled = false;
            break;
            
        // DJNZ Rr,addr - Decrement register and jump if not zero (0xE8-0xEF)
        // Operation: (Rr) <- (Rr) - 1, if (Rr) != 0 then (PC) <- addr
        // Flags affected: None
        // Cycles: 2
        // Decrements the specified register and jumps to the address if result is not zero.
        // Useful for implementing counted loops.
        case 0xE8: case 0xE9: case 0xEA: case 0xEB:
        case 0xEC: case 0xED: case 0xEE: case 0xEF: {
            uint8 addr = fetch_byte();
            uint8 reg_idx = get_register_index(opcode & 0x07, state_.current_bank);
            state_.r[reg_idx]--;
            if (state_.r[reg_idx] != 0) {
                state_.pc = (state_.pc & 0xF00) | addr;
            }
            cycles = 2;
            break;
        }
        
        // EN I - Enable external interrupt (0x05)
        // Operation: Enable external interrupt input
        // Flags affected: None
        // Cycles: 1
        // Enables the external interrupt input pin to generate interrupts.
        case 0x05:
            state_.interrupts_enabled = true;
            break;
            
        // EN TCNTI - Enable timer/counter interrupt (0x25)
        // Operation: Enable timer/counter interrupt
        // Flags affected: None
        // Cycles: 1
        // Enables timer/counter overflow to generate interrupts.
        case 0x25:
            state_.timer_interrupts_enabled = true;
            break;
            
        // STRT T - Start timer (0x55)
        // Operation: Start timer, clear prescaler
        // Flags affected: None
        // Cycles: 1
        // Initiates timer accumulation. Timer is incremented every 32 instruction cycles.
        // The prescaler is cleared but the timer register is not.
        // Reference: doc/mcs-48-assembly-language-manual.md, "Start Timer" section
        case 0x55:
            state_.timer_running = true;
            state_.timer_prescaler = 0;  // Clear prescaler
            break;
            
        // STRT CNT - Start event counter (0x45)
        // Operation: Enable T1 pin as event counter input and start
        // Flags affected: None
        // Cycles: 1
        // Enables the T1 pin as event counter input. Counter increments on high-to-low transitions.
        // For this emulator, we treat it the same as STRT T since we don't emulate external pins.
        // Reference: doc/mcs-48-assembly-language-manual.md, "Start Event Counter" section
        case 0x45:
            state_.timer_running = true;
            state_.timer_prescaler = 0;  // Clear prescaler
            break;
            
        // STOP TCNT - Stop timer/event counter (0x65)
        // Operation: Stop timer or disable event counter
        // Flags affected: None
        // Cycles: 1
        // Stops both time accumulation and event counting.
        // Reference: doc/mcs-48-assembly-language-manual.md, "Stop Timer/Event Counter" section
        case 0x65:
            state_.timer_running = false;
            break;
            
        // ========== INPUT/OUTPUT INSTRUCTIONS ==========
        
        // IN A,Pp - Input port to accumulator (0x09-0x0A)
        // Operation: (A) <- (Pp)
        // Flags affected: None
        // Cycles: 2
        // Reads data from port P1 or P2 into the accumulator.
        case 0x09: case 0x0A:
            state_.a = read_port(opcode & 0x03);
            cycles = 2;
            break;
            
        // INC A - Increment accumulator (0x17)
        // Operation: (A) <- (A) + 1
        // Flags affected: None
        // Cycles: 1
        case 0x17:
            state_.a++;
            break;
            
        // INC Rr - Increment register (0x18-0x1F)
        // Operation: (Rr) <- (Rr) + 1
        // Flags affected: None
        // Cycles: 1
        case 0x18: case 0x19: case 0x1A: case 0x1B:
        case 0x1C: case 0x1D: case 0x1E: case 0x1F: {
            uint8 reg_idx = get_register_index(opcode & 0x07, state_.current_bank);
            state_.r[reg_idx]++;
            break;
        }
            
        // INC @Rr - Increment data memory (0x10-0x11)
        // Operation: ((Rr)) <- ((Rr)) + 1
        // Flags affected: None
        // Cycles: 1
        // Increments the data memory location addressed by R0 or R1.
        case 0x10: case 0x11: {
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            if (addr < 64) state_.ram[addr]++;
            break;
        }
        
        // INS A,BUS - Input BUS to accumulator with strobe (0x08)
        // Operation: (A) <- (BUS)
        // Flags affected: None
        // Cycles: 2
        // Reads data from the external BUS with RD strobe active.
        case 0x08:
            // BUS input with RD strobe
            cycles = 2;
            break;
            
        // ========== CONDITIONAL JUMP INSTRUCTIONS ==========
        
        // JBb addr - Jump if accumulator bit is set (0x12, 0x32, 0x52, 0x72, 0x92, 0xB2, 0xD2, 0xF2)
        // Operation: If (A.b) = 1 then (PC) <- addr
        // Flags affected: None
        // Cycles: 2
        // Tests bit b (0-7) of the accumulator. If the bit is 1, jumps to the address within the current page.
        // Bit number is encoded in bits 5-7 of the opcode.
        case 0x12: case 0x32: case 0x52: case 0x72:
        case 0x92: case 0xB2: case 0xD2: case 0xF2: {
            uint8 addr = fetch_byte();
            uint8 bit = (opcode >> 5) & 0x07;
            if (state_.a & (1 << bit)) {
                state_.pc = (state_.pc & 0xF00) | addr;
            }
            cycles = 2;
            break;
        }
        
        // JC addr - Jump if carry is set (0xF6)
        // Operation: If (C) = 1 then (PC) <- addr
        // Flags affected: None
        // Cycles: 2
        // Jumps to the address within the current page if the carry flag is set.
        case 0xF6: {
            uint8 addr = fetch_byte();
            if (state_.psw & 0x80) {
                state_.pc = (state_.pc & 0xF00) | addr;
            }
            cycles = 2;
            break;
        }
        
        // JF0 addr - Jump if flag 0 is set (0xB6)
        // Operation: If (F0) = 1 then (PC) <- addr
        // Flags affected: None
        // Cycles: 2
        // Jumps to the address within the current page if flag F0 is set.
        case 0xB6: {
            uint8 addr = fetch_byte();
            if (state_.psw & 0x20) {
                state_.pc = (state_.pc & 0xF00) | addr;
            }
            cycles = 2;
            break;
        }
        
        // JF1 addr - Jump if flag 1 is set (0x76)
        // Operation: If (F1) = 1 then (PC) <- addr
        // Flags affected: None
        // Cycles: 2
        // Jumps to the address within the current page if flag F1 is set.
        case 0x76: {
            uint8 addr = fetch_byte();
            if (state_.psw & 0x10) {  // Test bit 4 (F1 flag)
                state_.pc = (state_.pc & 0xF00) | addr;
            }
            cycles = 2;
            break;
        }
        
        // JMP addr - Unconditional jump (0x04, 0x24, 0x44, 0x64, 0x84, 0xA4, 0xC4, 0xE4)
        // Operation: (PC) <- addr
        // Flags affected: None
        // Cycles: 2
        // Unconditionally jumps to the 11-bit address formed by combining bits 5-7 of the opcode
        // with the following byte. Can jump anywhere within the current 2K memory bank.
        case 0x04: case 0x24: case 0x44: case 0x64:
        case 0x84: case 0xA4: case 0xC4: case 0xE4: {
            uint8 addr_low = fetch_byte();
            state_.pc = ((opcode & 0xE0) << 3) | addr_low;
            cycles = 2;
            break;
        }
        
        // JMPP @A - Indirect jump via accumulator (0xB3)
        // Operation: (PC0-7) <- ((PC0-7) + (A))
        // Flags affected: None
        // Cycles: 2
        // Uses the accumulator as an offset into a jump table in the current page.
        // Reads the byte at (current_page | A) and uses it as the low 8 bits of the new PC.
        case 0xB3: {
            uint16 addr = (state_.pc & 0xF00) | state_.a;
            state_.a = read_memory(addr);
            state_.pc = (state_.pc & 0xF00) | state_.a;
            cycles = 2;
            break;
        }
        
        // JNC addr - Jump if carry is not set (0xE6)
        // Operation: If (C) = 0 then (PC) <- addr
        // Flags affected: None
        // Cycles: 2
        // Jumps to the address within the current page if the carry flag is clear.
        case 0xE6: {
            uint8 addr = fetch_byte();
            if (!(state_.psw & 0x80)) {
                state_.pc = (state_.pc & 0xF00) | addr;
            }
            cycles = 2;
            break;
        }
        
        // JNZ addr - Jump if accumulator is not zero (0x96)
        // Operation: If (A) != 0 then (PC) <- addr
        // Flags affected: None
        // Cycles: 2
        // Jumps to the address within the current page if the accumulator is not zero.
        case 0x96: {
            uint8 addr = fetch_byte();
            if (state_.a != 0) {
                state_.pc = (state_.pc & 0xF00) | addr;
            }
            cycles = 2;
            break;
        }
        
        // JTF addr - Jump if timer flag is set (0x16)
        // Operation: If (TF) = 1 then (PC) <- addr, (TF) <- 0
        // Flags affected: TF (cleared if jump taken)
        // Cycles: 2
        // Jumps to the address within the current page if the timer overflow flag is set.
        // The timer flag is automatically cleared when the jump is taken.
        case 0x16: {
            (void)fetch_byte();  // Read address (timer flag check would go here)
            cycles = 2;
            break;
        }
        
        // JZ addr - Jump if accumulator is zero (0xC6)
        // Operation: If (A) = 0 then (PC) <- addr
        // Flags affected: None
        // Cycles: 2
        // Jumps to the address within the current page if the accumulator is zero.
        case 0xC6: {
            uint8 addr = fetch_byte();
            if (state_.a == 0) {
                state_.pc = (state_.pc & 0xF00) | addr;
            }
            cycles = 2;
            break;
        }
        
        // ========== DATA TRANSFER INSTRUCTIONS (MOV) ==========
        
        // MOV A,#data - Move immediate data to accumulator (0x23)
        // Operation: (A) <- data
        // Flags affected: None
        // Cycles: 2
        case 0x23:
            state_.a = fetch_byte();
            cycles = 2;
            break;
            
        // MOV A,PSW - Move PSW to accumulator (0xC7)
        // Operation: (A) <- (PSW)
        // Flags affected: None
        // Cycles: 1
        // Copies the Program Status Word to the accumulator for inspection or saving.
        case 0xC7:
            state_.a = state_.psw;
            break;
            
        // MOV A,Rr - Move register to accumulator (0xF8-0xFF)
        // Operation: (A) <- (Rr)
        // Flags affected: None
        // Cycles: 1
        case 0xF8: case 0xF9: case 0xFA: case 0xFB:
        case 0xFC: case 0xFD: case 0xFE: case 0xFF: {
            uint8 reg_idx = get_register_index(opcode & 0x07, state_.current_bank);
            state_.a = state_.r[reg_idx];
            break;
        }
            
        // MOV A,@Rr - Move data memory to accumulator (0xF0-0xF1)
        // Operation: (A) <- ((Rr))
        // Flags affected: None
        // Cycles: 1
        // Moves the contents of the data memory location addressed by R0 or R1 to the accumulator.
        case 0xF0: case 0xF1: {
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            state_.a = (addr < 64) ? state_.ram[addr] : 0xFF;
            break;
        }
        
        // MOV A,T - Move timer to accumulator (0x42)
        // Operation: (A) <- (T)
        // Flags affected: None
        // Cycles: 1
        // Reads the current value of the timer/counter register into the accumulator.
        case 0x42:
            state_.a = state_.timer;
            break;
            
        // MOV PSW,A - Move accumulator to PSW (0xD7)
        // Operation: (PSW) <- (A)
        // Flags affected: All (PSW is replaced)
        // Cycles: 1
        // Copies the accumulator to the Program Status Word. Use with caution as this
        // affects all flags, stack pointer, and register bank selection.
        case 0xD7:
            state_.psw = state_.a;
            state_.current_bank = (state_.psw & 0x10) ? 1 : 0;  // Update bank from PSW bit 4
            break;
            
        // MOV Rr,A - Move accumulator to register (0xA8-0xAF)
        // Operation: (Rr) <- (A)
        // Flags affected: None
        // Cycles: 1
        case 0xA8: case 0xA9: case 0xAA: case 0xAB:
        case 0xAC: case 0xAD: case 0xAE: case 0xAF: {
            uint8 reg_idx = get_register_index(opcode & 0x07, state_.current_bank);
            state_.r[reg_idx] = state_.a;
            break;
        }
            
        // MOV Rr,#data - Move immediate data to register (0xB8-0xBF)
        // Operation: (Rr) <- data
        // Flags affected: None
        // Cycles: 2
        case 0xB8: case 0xB9: case 0xBA: case 0xBB:
        case 0xBC: case 0xBD: case 0xBE: case 0xBF: {
            uint8 reg_idx = get_register_index(opcode & 0x07, state_.current_bank);
            state_.r[reg_idx] = fetch_byte();
            cycles = 2;
            break;
        }
            
        // MOV @Rr,A - Move accumulator to data memory (0xA0-0xA1)
        // Operation: ((Rr)) <- (A)
        // Flags affected: None
        // Cycles: 1
        // Moves the accumulator to the data memory location addressed by R0 or R1.
        case 0xA0: case 0xA1: {
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            if (addr < 64) state_.ram[addr] = state_.a;
            break;
        }
        
        // MOV @Rr,#data - Move immediate data to data memory (0xB0-0xB1)
        // Operation: ((Rr)) <- data
        // Flags affected: None
        // Cycles: 2
        // Moves immediate data to the data memory location addressed by R0 or R1.
        case 0xB0: case 0xB1: {
            uint8 data = fetch_byte();
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            if (addr < 64) state_.ram[addr] = data;
            cycles = 2;
            break;
        }
        
        // MOV T,A - Move accumulator to timer (0x62)
        // Operation: (T) <- (A)
        // Flags affected: None
        // Cycles: 1
        // Loads the accumulator value into the timer/counter register.
        case 0x62:
            state_.timer = state_.a;
            break;
            
        // MOVP A,@A - Move program memory to accumulator (current page) (0xA3)
        // Operation: (A) <- ((PC8-11) | (A))
        // Flags affected: None
        // Cycles: 2
        // Uses the accumulator as an index into a table in the current 256-byte page.
        // Reads the byte at that location and stores it in the accumulator.
        // Useful for implementing lookup tables.
        case 0xA3: {
            uint16 addr = (state_.pc & 0xF00) | state_.a;
            state_.a = read_memory(addr);
            cycles = 2;
            break;
        }
        
        // MOVP3 A,@A - Move program memory to accumulator (page 3) (0xE3)
        // Operation: (A) <- ((0x300) | (A))
        // Flags affected: None
        // Cycles: 2
        // Similar to MOVP but always accesses page 3 (addresses 0x300-0x3FF).
        // Useful for accessing constant tables stored in a fixed location.
        case 0xE3: {
            uint16 addr = 0x300 | state_.a;
            state_.a = read_memory(addr);
            cycles = 2;
            break;
        }
        
        // MOVX A,@Rr - Move external RAM to accumulator (0x80-0x81)
        // Operation: (A) <- ((Rr))
        // Flags affected: None
        // Cycles: 2
        // Reads from external data memory addressed by R0 or R1.
        // The full 8-bit value in Rr is used as the external address.
        case 0x80: case 0x81: {
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            state_.a = (memory_) ? memory_->read_external(addr) : 0xFF;
            cycles = 2;
            break;
        }
            
        // MOVX @Rr,A - Move accumulator to external RAM (0x90-0x91)
        // Operation: ((Rr)) <- (A)
        // Flags affected: None
        // Cycles: 2
        // Writes the accumulator to external data memory addressed by R0 or R1.
        case 0x90: case 0x91: {
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            if (memory_) {
                memory_->write_external(addr, state_.a);
            }
            cycles = 2;
            break;
        }
            
        // ========== LOGICAL OR INSTRUCTIONS ==========
        
        // ORL A,@Rr - OR data memory with accumulator (0x40-0x41)
        // Operation: (A) <- (A) OR ((Rr))
        // Flags affected: None
        // Cycles: 1
        case 0x40: case 0x41: {
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            uint8 value = (addr < 64) ? state_.ram[addr] : 0xFF;
            state_.a |= value;
            break;
        }
        
        // ORL A,Rr - OR register with accumulator (0x48-0x4F)
        // Operation: (A) <- (A) OR (Rr)
        // Flags affected: None
        // Cycles: 1
        case 0x48: case 0x49: case 0x4A: case 0x4B:
        case 0x4C: case 0x4D: case 0x4E: case 0x4F: {
            uint8 reg_idx = get_register_index(opcode & 0x07, state_.current_bank);
            state_.a |= state_.r[reg_idx];
            break;
        }
            
        // ORL A,#data - OR immediate data with accumulator (0x43)
        // Operation: (A) <- (A) OR data
        // Flags affected: None
        // Cycles: 2
        case 0x43:
            state_.a |= fetch_byte();
            cycles = 2;
            break;
            
        // ORL BUS,#data - OR immediate data with BUS (0x88)
        // Operation: (BUS) <- (BUS) OR data
        // Flags affected: None
        // Cycles: 2
        case 0x88:
            fetch_byte();  // Read mask
            cycles = 2;
            break;
            
        // ORL Pp,#data - OR immediate data with port (0x89-0x8A)
        // Operation: (Pp) <- (Pp) OR data
        // Flags affected: None
        // Cycles: 2
        // Useful for setting specific bits in a port without affecting others.
        case 0x89: case 0x8A: {
            uint8 mask = fetch_byte();
            uint8 port = (opcode & 0x03);
            if (port == 1) state_.port1 |= mask;
            else if (port == 2) state_.port2 |= mask;
            cycles = 2;
            break;
        }
        
        // ========== OUTPUT INSTRUCTIONS ==========
        
        // OUTL BUS,A - Output accumulator to BUS (0x02)
        // Operation: (BUS) <- (A)
        // Flags affected: None
        // Cycles: 2
        // Outputs the accumulator to the external BUS and latches it.
        case 0x02:
            // Output to BUS
            cycles = 2;
            break;
            
        // OUTL Pp,A - Output accumulator to port (0x38-0x3A)
        // Operation: (Pp) <- (A)
        // Flags affected: None
        // Cycles: 2
        // Outputs the accumulator to port P1 or P2 and latches it.
        case 0x38: case 0x39: case 0x3A:
            write_port(opcode & 0x03, state_.a);
            cycles = 2;
            break;
            
        // ========== RETURN INSTRUCTIONS ==========
        
        // RET - Return from subroutine (0x83)
        // Operation: (PC) <- ((SP)) & 0x0FFF, (SP) <- (SP) - 1
        // Flags affected: None
        // Cycles: 2
        // Restores the program counter from the stack (lower 12 bits) and returns to the calling routine.
        // Does NOT restore PSW bits (use RETR for that).
        case 0x83: {
            uint16 stack_value = pop_stack();
            state_.pc = stack_value & 0x0FFF;  // Extract PC (12 bits)
            cycles = 2;
            break;
        }
            
        // RETR - Return from interrupt and restore PSW (0x93)
        // Restores PC and PSW bits 5-7 (F0, AC, C) from stack.
        // Bit 4 (F1/BS) is NOT restored - keeps current value.
        // Cycles: 2
        // Used when returning from interrupt service routines.
        // Reference: doc/mcs-48-assembly-language-manual.md: "F1 is not restored by RETR"
        case 0x93: {
            uint16 stack_value = pop_stack();
            state_.pc = stack_value & 0x0FFF;  // Extract PC (12 bits)
            // Restore PSW bits 5-7 (F0, AC, C) but NOT bit 4 (F1/BS)
            // Per Intel: "F1 is not restored by RETR" - bit 4 keeps its current value
            // This allows ISR to signal to main program via F1, and preserves bank selection
            state_.psw = (state_.psw & 0x1F) | ((stack_value >> 8) & 0xE0);  // Preserve bits 0-4, restore 5-7
            state_.current_bank = (state_.psw & 0x10) ? 1 : 0;  // Update bank from current bit 4
            cycles = 2;
            break;
        }
            
        // ========== ROTATE INSTRUCTIONS ==========
        
        // RL A - Rotate accumulator left (0xE7)
        // Operation: (An+1) <- (An), (A0) <- (A7)
        // Flags affected: None (carry not affected)
        // Cycles: 1
        // Rotates the accumulator left one bit position. Bit 7 wraps around to bit 0.
        case 0xE7: {
            uint8 msb = state_.a & 0x80;
            state_.a = (state_.a << 1) | (msb >> 7);
            break;
        }
        
        // RLC A - Rotate accumulator left through carry (0xF7)
        // Operation: (An+1) <- (An), (A0) <- (C), (C) <- (A7)
        // Flags affected: C
        // Cycles: 1
        // Rotates the accumulator left through the carry flag. Bit 7 goes to carry,
        // carry goes to bit 0. Useful for multi-byte shifts.
        case 0xF7: {
            uint8 msb = state_.a & 0x80;
            uint8 carry = (state_.psw & 0x80) ? 1 : 0;
            state_.a = (state_.a << 1) | carry;
            state_.psw = (state_.psw & 0x7F) | (msb ? 0x80 : 0);
            break;
        }
        
        // RR A - Rotate accumulator right (0x77)
        // Operation: (An) <- (An+1), (A7) <- (A0)
        // Flags affected: None (carry not affected)
        // Cycles: 1
        // Rotates the accumulator right one bit position. Bit 0 wraps around to bit 7.
        case 0x77: {
            uint8 lsb = state_.a & 0x01;
            state_.a = (state_.a >> 1) | (lsb << 7);
            break;
        }
        
        // RRC A - Rotate accumulator right through carry (0x67)
        // Operation: (An) <- (An+1), (A7) <- (C), (C) <- (A0)
        // Flags affected: C
        // Cycles: 1
        // Rotates the accumulator right through the carry flag. Bit 0 goes to carry,
        // carry goes to bit 7. Useful for multi-byte shifts.
        case 0x67: {
            uint8 lsb = state_.a & 0x01;
            uint8 carry = (state_.psw & 0x80) ? 1 : 0;
            state_.a = (state_.a >> 1) | (carry << 7);
            state_.psw = (state_.psw & 0x7F) | (lsb ? 0x80 : 0);
            break;
        }
        
        // ========== BANK SELECTION INSTRUCTIONS ==========
        
        // SEL MB0 - Select memory bank 0 (0xE5)
        // Operation: (DBF) <- 0
        // Flags affected: None
        // Cycles: 1
        // Selects the lower 2K of program memory (addresses 0x000-0x7FF).
        // Takes effect on the next jump or call instruction.
        case 0xE5:
            state_.pc &= 0x7FF;  // Clear bit 11
            break;
            
        // SEL MB1 - Select memory bank 1 (0xF5)
        // Operation: (DBF) <- 1
        // Flags affected: None
        // Cycles: 1
        // Selects the upper 2K of program memory (addresses 0x800-0xFFF).
        // Takes effect on the next jump or call instruction.
        case 0xF5:
            state_.pc |= 0x800;  // Set bit 11
            break;
            
        // SEL RB0 - Select register bank 0 (0xC5)
        // Operation: (BS) <- 0
        // Flags affected: BS (in PSW bit 4)
        // Cycles: 1
        // Selects register bank 0 (RAM locations 0-7 as R0-R7).
        case 0xC5:
            state_.current_bank = 0;
            state_.psw &= 0xEF;  // Clear PSW bit 4 (BS)
            break;
            
        // SEL RB1 - Select register bank 1 (0xD5)
        // Operation: (BS) <- 1
        // Flags affected: BS (in PSW bit 4)
        // Cycles: 1
        // Selects register bank 1 (RAM locations 24-31 as R0-R7).
        // Useful for preserving registers during interrupt service routines.
        case 0xD5:
            state_.current_bank = 1;
            state_.psw |= 0x10;  // Set PSW bit 4 (BS)
            break;
            
        // ========== DATA EXCHANGE INSTRUCTIONS ==========
        
        // SWAP A - Swap accumulator nibbles (0x47)
        // Operation: (A0-3) <-> (A4-7)
        // Flags affected: None
        // Cycles: 1
        // Exchanges the upper and lower 4-bit nibbles of the accumulator.
        // Useful for BCD operations and nibble manipulation.
        case 0x47:
            state_.a = ((state_.a & 0x0F) << 4) | ((state_.a & 0xF0) >> 4);
            break;
            
        // XCH A,Rr - Exchange accumulator and register (0x28-0x2F)
        // Operation: (A) <-> (Rr)
        // Flags affected: None
        // Cycles: 1
        // Exchanges the contents of the accumulator and the specified register.
        case 0x28: case 0x29: case 0x2A: case 0x2B:
        case 0x2C: case 0x2D: case 0x2E: case 0x2F: {
            uint8 reg_idx = get_register_index(opcode & 0x07, state_.current_bank);
            uint8 temp = state_.a;
            state_.a = state_.r[reg_idx];
            state_.r[reg_idx] = temp;
            break;
        }
        
        // XCH A,@Rr - Exchange accumulator and data memory (0x20-0x21)
        // Operation: (A) <-> ((Rr))
        // Flags affected: None
        // Cycles: 1
        // Exchanges the accumulator with the data memory location addressed by R0 or R1.
        case 0x20: case 0x21: {
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            if (addr < 64) {
                uint8 temp = state_.a;
                state_.a = state_.ram[addr];
                state_.ram[addr] = temp;
            }
            break;
        }
        
        // XCHD A,@Rr - Exchange accumulator and data memory digit (0x30-0x31)
        // Operation: (A0-3) <-> ((Rr)0-3)
        // Flags affected: None
        // Cycles: 1
        // Exchanges only the lower 4 bits (nibble) between the accumulator and data memory.
        // Upper nibbles remain unchanged. Useful for BCD operations.
        case 0x30: case 0x31: {
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            if (addr < 64) {
                uint8 temp = state_.a & 0x0F;
                state_.a = (state_.a & 0xF0) | (state_.ram[addr] & 0x0F);
                state_.ram[addr] = (state_.ram[addr] & 0xF0) | temp;
            }
            break;
        }
        
        // ========== EXCLUSIVE OR INSTRUCTIONS ==========
        
        // XRL A,@Rr - XOR data memory with accumulator (0xD0-0xD1)
        // Operation: (A) <- (A) XOR ((Rr))
        // Flags affected: None
        // Cycles: 1
        case 0xD0: case 0xD1: {
            uint8 reg_idx = get_register_index(opcode & 0x01, state_.current_bank);
            uint8 addr = state_.r[reg_idx];
            uint8 value = (addr < 64) ? state_.ram[addr] : 0xFF;
            state_.a ^= value;
            break;
        }
        
        // XRL A,Rr - XOR register with accumulator (0xD8-0xDF)
        // Operation: (A) <- (A) XOR (Rr)
        // Flags affected: None
        // Cycles: 1
        case 0xD8: case 0xD9: case 0xDA: case 0xDB:
        case 0xDC: case 0xDD: case 0xDE: case 0xDF: {
            uint8 reg_idx = get_register_index(opcode & 0x07, state_.current_bank);
            state_.a ^= state_.r[reg_idx];
            break;
        }
            
        // XRL A,#data - XOR immediate data with accumulator (0xD3)
        // Operation: (A) <- (A) XOR data
        // Flags affected: None
        // Cycles: 2
        // Useful for toggling specific bits or complementing values.
        case 0xD3:
            state_.a ^= fetch_byte();
            cycles = 2;
            break;
            
        default:
            // Unknown opcode - treat as NOP
            break;
    }
    
    state_.clock_cycles += cycles;
    
    // Timer/counter logic: increment timer every 32 cycles when running
    // The Intel 8048 timer increments every 32 instruction cycles via a prescaler.
    // Reference: doc/mcs-48-assembly-language-manual.md, "Timer Flag" section
    // Quote: "incremented by a prescaler having a periodic duration equivalent to 32 instruction cycles"
    if (state_.timer_running) {
        state_.timer_prescaler += cycles;
        if (state_.timer_prescaler >= 32) {
            state_.timer_prescaler -= 32;
            uint8 old_timer = state_.timer;
            state_.timer++;
            
            // Check for timer overflow (0xFF -> 0x00)
            // Reference: doc/mcs-48-assembly-language-manual.md, "Timer Flag" section
            // Quote: "the 8-bit timer register will overflow every 8192 cycles (256 x 32)"
            if (old_timer == 0xFF && state_.timer == 0x00) {
                // Timer overflowed - trigger interrupt if enabled
                if (state_.timer_interrupts_enabled && state_.interrupts_enabled) {
                    trigger_interrupt(0x007);  // Timer interrupt vector
                }
            }
        }
    }
    
    return cycles;
}

} // namespace videopac
