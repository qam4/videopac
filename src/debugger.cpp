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

void Debugger::add_breakpoint(uint16 address, const std::string& condition) {
    // Check if breakpoint already exists
    for (auto& bp : breakpoints_) {
        if (bp.address == address) {
            // Update existing breakpoint with condition
            bp.has_condition = true;
            bp.condition = condition;
            return;
        }
    }
    breakpoints_.emplace_back(address, condition, true);
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
            // If no condition, break immediately
            if (!bp.has_condition) {
                return true;
            }
            
            // Evaluate condition
            CPUState cpu = emulator_->get_cpu_state();
            if (evaluate_condition(bp.condition, cpu)) {
                return true;
            }
        }
    }
    return false;
}

// Helper function to evaluate breakpoint conditions
// Supports simple expressions like: cpu.A==0xFF, vdc.registers[0xA0]&0x20, memory.copy_mode==true
bool Debugger::evaluate_condition(const std::string& condition, const CPUState& cpu) const {
    if (condition.empty()) {
        return true;
    }
    
    // Get all emulator state
    VDCState vdc = emulator_->get_vdc_state();
    MemoryState mem = emulator_->get_memory_state();
    
    // Find operator
    size_t op_pos = std::string::npos;
    std::string op;
    
    if ((op_pos = condition.find("==")) != std::string::npos) {
        op = "==";
    } else if ((op_pos = condition.find("!=")) != std::string::npos) {
        op = "!=";
    } else if ((op_pos = condition.find(">=")) != std::string::npos) {
        op = ">=";
    } else if ((op_pos = condition.find("<=")) != std::string::npos) {
        op = "<=";
    } else if ((op_pos = condition.find(">")) != std::string::npos) {
        op = ">";
    } else if ((op_pos = condition.find("<")) != std::string::npos) {
        op = "<";
    } else if ((op_pos = condition.find("&")) != std::string::npos) {
        op = "&";
    } else if ((op_pos = condition.find("|")) != std::string::npos) {
        op = "|";
    } else {
        return false;  // Invalid condition
    }
    
    // Extract left side (variable path) and right side (value)
    std::string var_path = condition.substr(0, op_pos);
    std::string value_str = condition.substr(op_pos + op.length());
    
    // Trim whitespace
    var_path.erase(0, var_path.find_first_not_of(" \t"));
    var_path.erase(var_path.find_last_not_of(" \t") + 1);
    value_str.erase(0, value_str.find_first_not_of(" \t"));
    value_str.erase(value_str.find_last_not_of(" \t") + 1);
    
    // Parse value (supports hex with 0x prefix, or boolean true/false)
    uint16 value = 0;
    bool is_bool = false;
    bool bool_value = false;
    
    if (value_str == "true") {
        is_bool = true;
        bool_value = true;
    } else if (value_str == "false") {
        is_bool = true;
        bool_value = false;
    } else if (value_str.find("0x") == 0 || value_str.find("0X") == 0) {
        value = std::stoi(value_str, nullptr, 16);
    } else {
        value = std::stoi(value_str);
    }
    
    // Parse variable path and get value
    uint16 var_value = 0;
    bool var_is_bool = false;
    bool var_bool_value = false;
    
    // Split by '.' to get namespace and field
    size_t dot_pos = var_path.find('.');
    if (dot_pos == std::string::npos) {
        return false;  // Must have namespace (cpu., vdc., memory.)
    }
    
    std::string ns = var_path.substr(0, dot_pos);
    std::string field = var_path.substr(dot_pos + 1);
    
    // Handle array indexing like registers[0xA0]
    size_t bracket_pos = field.find('[');
    std::string array_name;
    int array_index = -1;
    
    if (bracket_pos != std::string::npos) {
        array_name = field.substr(0, bracket_pos);
        size_t close_bracket = field.find(']');
        if (close_bracket == std::string::npos) {
            return false;  // Invalid array syntax
        }
        std::string index_str = field.substr(bracket_pos + 1, close_bracket - bracket_pos - 1);
        // Trim whitespace
        index_str.erase(0, index_str.find_first_not_of(" \t"));
        index_str.erase(index_str.find_last_not_of(" \t") + 1);
        
        if (index_str.find("0x") == 0 || index_str.find("0X") == 0) {
            array_index = std::stoi(index_str, nullptr, 16);
        } else {
            array_index = std::stoi(index_str);
        }
        field = array_name;
    }
    
    // Get variable value based on namespace
    if (ns == "cpu") {
        if (field == "A") {
            var_value = cpu.a;
        } else if (field == "PSW") {
            var_value = cpu.psw;
        } else if (field == "PC") {
            var_value = cpu.pc;
        } else if (field == "SP") {
            var_value = cpu.sp;
        } else if (field.length() == 2 && field[0] == 'R' && field[1] >= '0' && field[1] <= '7') {
            int r = field[1] - '0';
            var_value = cpu.r[r];
        } else if (field == "ram" && array_index >= 0 && array_index < 64) {
            var_value = cpu.ram[array_index];
        } else {
            return false;  // Unknown CPU field
        }
    } else if (ns == "vdc") {
        if (field == "registers" && array_index >= 0 && array_index < 256) {
            var_value = vdc.registers[array_index];
        } else if (field == "scanline") {
            var_value = vdc.scanline;
        } else if (field == "display_enabled") {
            var_is_bool = true;
            var_bool_value = vdc.display_enabled;
        } else if (field == "grid_enabled") {
            var_is_bool = true;
            var_bool_value = vdc.grid_enabled;
        } else {
            return false;  // Unknown VDC field
        }
    } else if (ns == "memory") {
        if (field == "external_ram" && array_index >= 0 && array_index < 128) {
            var_value = mem.external_ram[array_index];
        } else if (field == "current_bank") {
            var_value = mem.current_bank;
        } else if (field == "rom_size_kb") {
            var_value = mem.rom_size_kb;
        } else {
            return false;  // Unknown memory field
        }
    } else {
        return false;  // Unknown namespace
    }
    
    // Evaluate condition
    if (var_is_bool && is_bool) {
        // Boolean comparison
        if (op == "==") {
            return var_bool_value == bool_value;
        } else if (op == "!=") {
            return var_bool_value != bool_value;
        }
        return false;
    }
    
    // Numeric comparison
    if (op == "==") {
        return var_value == value;
    } else if (op == "!=") {
        return var_value != value;
    } else if (op == ">") {
        return var_value > value;
    } else if (op == "<") {
        return var_value < value;
    } else if (op == ">=") {
        return var_value >= value;
    } else if (op == "<=") {
        return var_value <= value;
    } else if (op == "&") {
        return (var_value & value) != 0;
    } else if (op == "|") {
        return (var_value | value) != 0;
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

void Debugger::log_instruction(uint64 current_cycles) {
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
    
    // Use provided current_cycles if non-zero, otherwise use frame_stats total
    uint64 cycles_to_log = (current_cycles > 0) ? current_cycles : frame_stats_.total_cycles;
    
    // Add frame and cycle info, then CPU state
    std::stringstream ss;
    ss << "[F:" << std::dec << frame_stats_.frame_count 
       << " C:" << cycles_to_log << "] ";
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
