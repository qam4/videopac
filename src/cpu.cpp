#include "cpu.h"
#include "memory.h"
#include <cstring>

namespace videopac {

CPU::CPU() : memory_(nullptr) {
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

uint8 CPU::execute_instruction() {
    // TODO: Implement instruction execution
    // This is a placeholder that will be implemented in task 3
    return 1;  // Return cycle count
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
        push_stack(state_.pc);
        push_stack(state_.psw);
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

} // namespace videopac
