#include "debugger.h"
#include "emulator.h"
#include "disassembler.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace videopac {

Debugger::Debugger(EmulatorCore* emulator)
    : emulator_(emulator)
    , state_(DebuggerState::Running)
    , trace_enabled_(false)
    , last_frame_time_(0) {
    frame_stats_.total_cycles = 0;
    frame_stats_.frame_count = 0;
    frame_stats_.fps = 0.0;
    frame_stats_.average_cycles_per_frame = 0.0;
}

// Breakpoint management
void Debugger::add_breakpoint(uint16 address) {
    // Check if breakpoint already exists
    for (const auto& bp : breakpoints_) {
        if (bp.address == address) {
            return;  // Already exists
        }
    }
    breakpoints_.emplace_back(address, true);
}

void Debugger::remove_breakpoint(uint16 address) {
    breakpoints_.erase(
        std::remove_if(breakpoints_.begin(), breakpoints_.end(),
            [address](const Breakpoint& bp) { return bp.address == address; }),
        breakpoints_.end()
    );
}

void Debugger::enable_breakpoint(uint16 address, bool enabled) {
    for (auto& bp : breakpoints_) {
        if (bp.address == address) {
            bp.enabled = enabled;
            return;
        }
    }
}

void Debugger::clear_all_breakpoints() {
    breakpoints_.clear();
}

bool Debugger::check_breakpoint(uint16 address) const {
    for (const auto& bp : breakpoints_) {
        if (bp.address == address && bp.enabled) {
            return true;
        }
    }
    return false;
}

// Execution control
void Debugger::step() {
    state_ = DebuggerState::StepMode;
    emulator_->step();
    if (trace_enabled_) {
        log_instruction();
    }
    state_ = DebuggerState::Paused;
}

void Debugger::continue_execution() {
    state_ = DebuggerState::Running;
}

void Debugger::pause() {
    state_ = DebuggerState::Paused;
}

// Inspection
std::string Debugger::dump_cpu_state() const {
    std::stringstream ss;
    CPUState cpu = emulator_->get_cpu_state();
    
    ss << "CPU State:\n";
    ss << "  PC:  0x" << std::hex << std::setw(3) << std::setfill('0') << cpu.pc << "\n";
    ss << "  A:   0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.a << "\n";
    ss << "  PSW: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.psw;
    ss << " [C=" << ((cpu.psw & 0x80) ? "1" : "0");
    ss << " AC=" << ((cpu.psw & 0x40) ? "1" : "0");
    ss << " F0=" << ((cpu.psw & 0x20) ? "1" : "0");
    ss << " BS=" << ((cpu.psw & 0x10) ? "1" : "0") << "]\n";
    
    ss << "  Registers (Bank " << ((cpu.psw & 0x10) ? "1" : "0") << "):\n";
    int bank_offset = (cpu.psw & 0x10) ? 8 : 0;
    for (int i = 0; i < 8; i++) {
        ss << "    R" << i << ": 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.r[bank_offset + i];
        if (i % 4 == 3) ss << "\n";
    }
    
    ss << "  Stack Pointer: " << std::dec << (int)cpu.sp << "\n";
    ss << "  Interrupts: " << (cpu.interrupts_enabled ? "Enabled" : "Disabled") << "\n";
    ss << "  Timer: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.timer << "\n";
    
    return ss.str();
}

std::string Debugger::dump_memory(uint16 start, uint16 end) const {
    std::stringstream ss;
    CPUState cpu = emulator_->get_cpu_state();
    
    ss << "Internal RAM (0x00-0x3F):\n";
    for (uint16 addr = start; addr <= end && addr < 64; addr++) {
        if (addr % 16 == 0) {
            ss << "  0x" << std::hex << std::setw(2) << std::setfill('0') << addr << ": ";
        }
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.ram[addr] << " ";
        if (addr % 16 == 15 || addr == end) {
            ss << "\n";
        }
    }
    
    return ss.str();
}

std::string Debugger::dump_vdc_registers() const {
    std::stringstream ss;
    VDCState vdc = emulator_->get_vdc_state();
    
    ss << "VDC Registers:\n";
    ss << "  Control (0xA0): 0x" << std::hex << std::setw(2) << std::setfill('0') 
       << (int)vdc.registers[0xA0] << "\n";
    ss << "  Status  (0xA1): 0x" << std::hex << std::setw(2) << std::setfill('0') 
       << (int)vdc.registers[0xA1] << "\n";
    ss << "  Collision (0xA2): 0x" << std::hex << std::setw(2) << std::setfill('0') 
       << (int)vdc.registers[0xA2] << "\n";
    ss << "  Color   (0xA3): 0x" << std::hex << std::setw(2) << std::setfill('0') 
       << (int)vdc.registers[0xA3] << "\n";
    
    ss << "  Sprites:\n";
    for (int i = 0; i < 4; i++) {
        uint8 x = vdc.registers[0x10 + i * 4];
        uint8 y = vdc.registers[0x10 + i * 4 + 1];
        uint8 color = vdc.registers[0x10 + i * 4 + 2];
        uint8 pattern = vdc.registers[0x10 + i * 4 + 3];
        ss << "    Sprite " << i << ": X=" << std::dec << (int)x 
           << " Y=" << (int)y 
           << " Color=0x" << std::hex << (int)color 
           << " Pattern=0x" << (int)pattern << "\n";
    }
    
    ss << "  Scanline: " << std::dec << vdc.scanline;
    ss << " (VBLANK: " << (vdc.scanline >= (vdc.video_standard == VideoStandard::NTSC ? 240 : 288) ? "Yes" : "No") << ")\n";
    
    return ss.str();
}

std::string Debugger::disassemble_at_pc(int lines_before, int lines_after) const {
    std::stringstream ss;
    CPUState cpu = emulator_->get_cpu_state();
    Disassembler disasm;
    
    // Get ROM from memory system
    MemoryState mem = emulator_->get_memory_state();
    
    // Disassemble around PC
    uint16 start_addr = (cpu.pc > lines_before * 2) ? cpu.pc - lines_before * 2 : 0;
    uint16 end_addr = cpu.pc + lines_after * 2;
    
    ss << "Disassembly around PC (0x" << std::hex << std::setw(3) << std::setfill('0') 
       << cpu.pc << "):\n";
    
    uint16 addr = start_addr;
    while (addr <= end_addr && addr < 2048) {
        // Read instruction from ROM
        const uint8* code = &mem.bios_rom[addr];
        if (addr >= 1024 && !mem.cart_rom.empty()) {
            // Cartridge ROM
            code = &mem.cart_rom[addr - 1024];
        }
        
        Instruction instr = disasm.disassemble_instruction(addr, code);
        std::string formatted = disasm.format_instruction(instr);
        
        // Mark current PC
        if (addr == cpu.pc) {
            ss << " -> ";
        } else {
            ss << "    ";
        }
        ss << formatted << "\n";
        
        addr += instr.size;
    }
    
    return ss.str();
}

// Trace logging
void Debugger::enable_trace(bool enabled) {
    trace_enabled_ = enabled;
}

void Debugger::log_instruction() {
    CPUState cpu = emulator_->get_cpu_state();
    Disassembler disasm;
    MemoryState mem = emulator_->get_memory_state();
    
    // Read instruction from ROM
    const uint8* code = &mem.bios_rom[cpu.pc];
    if (cpu.pc >= 1024 && !mem.cart_rom.empty()) {
        code = &mem.cart_rom[cpu.pc - 1024];
    }
    
    Instruction instr = disasm.disassemble_instruction(cpu.pc, code);
    std::string formatted = disasm.format_instruction(instr);
    
    // Add CPU state
    std::stringstream ss;
    ss << formatted << " | A=0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.a;
    ss << " PSW=0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.psw;
    
    trace_log_.push_back(ss.str());
    
    // Limit trace log size to prevent memory issues (disabled for debugging)
    // if (trace_log_.size() > 10000) {
    //     trace_log_.erase(trace_log_.begin());
    // }
}

void Debugger::clear_trace_log() {
    trace_log_.clear();
}

// Frame timing statistics
void Debugger::update_frame_stats(uint64 cycles) {
    frame_stats_.total_cycles += cycles;
    frame_stats_.frame_count++;
    
    if (frame_stats_.frame_count > 0) {
        frame_stats_.average_cycles_per_frame = 
            static_cast<double>(frame_stats_.total_cycles) / frame_stats_.frame_count;
    }
    
    // Calculate FPS (assuming 60Hz target)
    // This is a simple approximation
    frame_stats_.fps = 60.0;  // Will be updated by frontend with real timing
}

void Debugger::reset_frame_stats() {
    frame_stats_.total_cycles = 0;
    frame_stats_.frame_count = 0;
    frame_stats_.fps = 0.0;
    frame_stats_.average_cycles_per_frame = 0.0;
}

} // namespace videopac
