#include "debugger_ui.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace videopac {

DebuggerUI::DebuggerUI(Debugger* debugger) : debugger_(debugger) {}

void DebuggerUI::process_command(const std::string& command_line) {
    auto args = parse_command(command_line);
    if (args.empty()) {
        return;
    }
    
    std::string cmd = args[0];
    
    // Convert to lowercase for case-insensitive matching
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    
    if (cmd == "help" || cmd == "h" || cmd == "?") {
        cmd_help(args);
    } else if (cmd == "continue" || cmd == "c") {
        cmd_continue(args);
    } else if (cmd == "step" || cmd == "s") {
        cmd_step(args);
    } else if (cmd == "break" || cmd == "b") {
        cmd_break(args);
    } else if (cmd == "delete" || cmd == "d") {
        cmd_delete(args);
    } else if (cmd == "list" || cmd == "l") {
        cmd_list(args);
    } else if (cmd == "registers" || cmd == "r") {
        cmd_registers(args);
    } else if (cmd == "memory" || cmd == "m") {
        cmd_memory(args);
    } else if (cmd == "vdc" || cmd == "v") {
        cmd_vdc(args);
    } else if (cmd == "disassemble" || cmd == "dis") {
        cmd_disassemble(args);
    } else if (cmd == "trace" || cmd == "t") {
        cmd_trace(args);
    } else if (cmd == "stats") {
        cmd_stats(args);
    } else {
        std::cout << "Unknown command: " << cmd << std::endl;
        std::cout << "Type 'help' for list of commands" << std::endl;
    }
}

void DebuggerUI::print_help() {
    std::cout << "Videopac Debugger Commands:" << std::endl;
    std::cout << "  help, h, ?              - Show this help message" << std::endl;
    std::cout << "  continue, c             - Continue execution" << std::endl;
    std::cout << "  step, s [count]         - Step one or more instructions" << std::endl;
    std::cout << "  break, b <addr>         - Set breakpoint at address" << std::endl;
    std::cout << "  delete, d <addr>        - Delete breakpoint at address" << std::endl;
    std::cout << "  list, l                 - List all breakpoints" << std::endl;
    std::cout << "  registers, r            - Display CPU registers" << std::endl;
    std::cout << "  memory, m <start> <end> - Display memory range" << std::endl;
    std::cout << "  vdc, v                  - Display VDC registers" << std::endl;
    std::cout << "  disassemble, dis [n]    - Disassemble around PC (n lines before/after)" << std::endl;
    std::cout << "  trace, t [on|off|show]  - Enable/disable/show instruction trace" << std::endl;
    std::cout << "  stats                   - Display frame timing statistics" << std::endl;
}

void DebuggerUI::display_cpu_state() {
    std::cout << debugger_->dump_cpu_state() << std::endl;
}

void DebuggerUI::display_memory(uint16 start, uint16 end) {
    std::cout << debugger_->dump_memory(start, end) << std::endl;
}

void DebuggerUI::display_vdc_state() {
    std::cout << debugger_->dump_vdc_registers() << std::endl;
}

void DebuggerUI::display_disassembly(int lines_before, int lines_after) {
    std::cout << debugger_->disassemble_at_pc(lines_before, lines_after) << std::endl;
}

void DebuggerUI::display_frame_stats() {
    FrameStats stats = debugger_->get_frame_stats();
    std::cout << "Frame Statistics:" << std::endl;
    std::cout << "  Total Cycles: " << stats.total_cycles << std::endl;
    std::cout << "  Frame Count: " << stats.frame_count << std::endl;
    std::cout << "  FPS: " << std::fixed << std::setprecision(2) << stats.fps << std::endl;
    std::cout << "  Avg Cycles/Frame: " << std::fixed << std::setprecision(2) 
              << stats.average_cycles_per_frame << std::endl;
}

void DebuggerUI::display_breakpoints() {
    const auto& breakpoints = debugger_->get_breakpoints();
    if (breakpoints.empty()) {
        std::cout << "No breakpoints set" << std::endl;
        return;
    }
    
    std::cout << "Breakpoints:" << std::endl;
    for (const auto& bp : breakpoints) {
        std::cout << "  0x" << std::hex << std::setw(4) << std::setfill('0') 
                  << bp.address << " " << (bp.enabled ? "[enabled]" : "[disabled]") 
                  << std::endl;
    }
}

void DebuggerUI::display_trace_log(int lines) {
    const auto& trace = debugger_->get_trace_log();
    if (trace.empty()) {
        std::cout << "Trace log is empty" << std::endl;
        return;
    }
    
    int start = std::max(0, static_cast<int>(trace.size()) - lines);
    std::cout << "Instruction Trace (last " << lines << " entries):" << std::endl;
    for (int i = start; i < static_cast<int>(trace.size()); ++i) {
        std::cout << trace[i] << std::endl;
    }
}

// Command handlers

void DebuggerUI::cmd_help(const std::vector<std::string>& /*args*/) {
    print_help();
}

void DebuggerUI::cmd_continue(const std::vector<std::string>& /*args*/) {
    debugger_->continue_execution();
    std::cout << "Continuing execution..." << std::endl;
}

void DebuggerUI::cmd_step(const std::vector<std::string>& args) {
    int count = 1;
    if (args.size() > 1) {
        try {
            count = std::stoi(args[1]);
        } catch (...) {
            std::cout << "Invalid step count" << std::endl;
            return;
        }
    }
    
    for (int i = 0; i < count; ++i) {
        debugger_->step();
    }
    
    std::cout << "Stepped " << count << " instruction(s)" << std::endl;
    display_cpu_state();
    display_disassembly(2, 2);
}

void DebuggerUI::cmd_break(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: break <address>" << std::endl;
        return;
    }
    
    uint16 addr = parse_address(args[1]);
    debugger_->add_breakpoint(addr);
    std::cout << "Breakpoint set at 0x" << std::hex << std::setw(4) 
              << std::setfill('0') << addr << std::endl;
}

void DebuggerUI::cmd_delete(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: delete <address>" << std::endl;
        return;
    }
    
    uint16 addr = parse_address(args[1]);
    debugger_->remove_breakpoint(addr);
    std::cout << "Breakpoint removed at 0x" << std::hex << std::setw(4) 
              << std::setfill('0') << addr << std::endl;
}

void DebuggerUI::cmd_list(const std::vector<std::string>& /*args*/) {
    display_breakpoints();
}

void DebuggerUI::cmd_registers(const std::vector<std::string>& /*args*/) {
    display_cpu_state();
}

void DebuggerUI::cmd_memory(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: memory <start> <end>" << std::endl;
        return;
    }
    
    uint16 start = parse_address(args[1]);
    uint16 end = parse_address(args[2]);
    display_memory(start, end);
}

void DebuggerUI::cmd_vdc(const std::vector<std::string>& /*args*/) {
    display_vdc_state();
}

void DebuggerUI::cmd_disassemble(const std::vector<std::string>& args) {
    int lines = 5;
    if (args.size() > 1) {
        try {
            lines = std::stoi(args[1]);
        } catch (...) {
            std::cout << "Invalid line count" << std::endl;
            return;
        }
    }
    
    display_disassembly(lines, lines);
}

void DebuggerUI::cmd_trace(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: trace [on|off|show]" << std::endl;
        return;
    }
    
    std::string subcmd = args[1];
    std::transform(subcmd.begin(), subcmd.end(), subcmd.begin(), ::tolower);
    
    if (subcmd == "on") {
        debugger_->enable_trace(true);
        std::cout << "Instruction trace enabled" << std::endl;
    } else if (subcmd == "off") {
        debugger_->enable_trace(false);
        std::cout << "Instruction trace disabled" << std::endl;
    } else if (subcmd == "show") {
        int lines = 20;
        if (args.size() > 2) {
            try {
                lines = std::stoi(args[2]);
            } catch (...) {}
        }
        display_trace_log(lines);
    } else {
        std::cout << "Usage: trace [on|off|show]" << std::endl;
    }
}

void DebuggerUI::cmd_stats(const std::vector<std::string>& /*args*/) {
    display_frame_stats();
}

// Helper methods

std::vector<std::string> DebuggerUI::parse_command(const std::string& command_line) {
    std::vector<std::string> args;
    std::istringstream iss(command_line);
    std::string arg;
    
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    return args;
}

uint16 DebuggerUI::parse_address(const std::string& addr_str) {
    uint16 addr = 0;
    
    try {
        // Support both hex (0x prefix) and decimal
        if (addr_str.size() > 2 && addr_str[0] == '0' && 
            (addr_str[1] == 'x' || addr_str[1] == 'X')) {
            addr = static_cast<uint16>(std::stoul(addr_str, nullptr, 16));
        } else {
            addr = static_cast<uint16>(std::stoul(addr_str, nullptr, 0));
        }
    } catch (...) {
        std::cout << "Invalid address: " << addr_str << std::endl;
    }
    
    return addr;
}

} // namespace videopac
