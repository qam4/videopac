#ifndef UI_IMGUI_DEBUGGER_UI_H
#define UI_IMGUI_DEBUGGER_UI_H

#include "types.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <SDL2/SDL.h>

// Forward declarations
struct ImGuiContext;

namespace videopac {
    class Debugger;
    class EmulatorCore;
    class Disassembler;
    struct CPUState;
    struct VDCState;
    struct MemoryState;
}

// Display mode for debugger UI
enum class DisplayMode {
    Overlay,      // ImGui renders on top of emulation
    SplitScreen   // Emulation viewport shrinks
};

// Watch expression structure
struct WatchExpression {
    enum class Type {
        MemoryAddress,
        Register
    };
    
    Type type;
    std::string expression;  // e.g., "0x1234", "A", "R0"
    std::string label;       // User-defined label
    videopac::uint16 last_value;  // For change detection
    bool changed;            // Highlight if changed
    
    // Evaluation
    videopac::uint16 evaluate(const videopac::CPUState& cpu, const videopac::MemoryState& memory) const;
};

// Disassembly cache helper
struct DisassemblyCache {
    std::unordered_map<videopac::uint16, std::string> cache;
    
    // Get disassembly for an address, using cache if available
    std::string get_disassembly(videopac::uint16 address, videopac::Disassembler* disasm, const videopac::uint8* memory);
    
    // Clear the cache (call when memory is modified or ROM is loaded)
    void invalidate();
};

// ImGui-based in-game debugger UI
class ImGuiDebuggerUI {
public:
    // Limits for breakpoints and watch expressions
    static constexpr size_t MAX_BREAKPOINTS = 100;
    static constexpr size_t MAX_WATCH_EXPRESSIONS = 50;
    
    ImGuiDebuggerUI(videopac::Debugger* debugger, videopac::EmulatorCore* emulator, 
                    SDL_Window* window, SDL_Renderer* renderer);
    ~ImGuiDebuggerUI();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    
    // Activation
    void show();
    void hide();
    bool is_visible() const;
    
    // Rendering
    void render();  // Called every frame when visible
    void process_event(const SDL_Event& event);  // Forward SDL events to ImGui
    
    // Display mode
    void set_display_mode(DisplayMode mode);
    DisplayMode get_display_mode() const;
    
    // Context access (for frontend to set context before starting frame)
    ImGuiContext* get_context() const { return imgui_context_; }
    
    // State persistence
    void save_state();
    void load_state();
    
private:
    // Panel rendering methods
    void render_cpu_state_panel();
    void render_memory_panel();
    void render_internal_ram(const videopac::CPUState& cpu_state);
    void render_external_ram(const videopac::MemoryState& mem_state);
    void render_program_memory(const videopac::MemoryState& mem_state);
    void render_vdc_registers_panel();
    void render_breakpoints_panel();
    void render_disassembly_panel();
    void render_call_stack_panel();
    void render_watch_panel();
    void render_controls_panel();
    
    // Helper methods
    void render_register(const char* name, videopac::uint8 value);
    void render_register_16(const char* name, videopac::uint16 value);
    void render_psw_flags(videopac::uint8 psw);
    void render_memory_editor(videopac::uint16 start_address, size_t size);
    void render_hex_dump(const videopac::uint8* data, size_t size, videopac::uint16 base_address);
    
    // Validation methods
    bool validate_breakpoint_condition(const std::string& condition, std::string& error_msg);
    bool validate_watch_expression(const std::string& expression, std::string& error_msg);
    
    // Keyboard shortcuts
    void handle_shortcuts();
    
    // State
    videopac::Debugger* debugger_;
    videopac::EmulatorCore* emulator_;
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    
    bool visible_;
    DisplayMode display_mode_;
    
    // ImGui context
    ImGuiContext* imgui_context_;
    
    // Panel state
    struct MemoryViewerState {
        videopac::uint16 current_address;
        videopac::uint16 goto_address;
        bool goto_pending;
        std::string search_query;
        std::vector<videopac::uint16> search_results;
        int search_result_index;
        videopac::uint8 edit_value;
        videopac::uint16 edit_address;
        bool editing;
    } memory_state_;
    
    struct DisassemblyState {
        videopac::uint16 current_address;
        videopac::uint16 cursor_address;
        bool follow_pc;
    } disasm_state_;
    
    struct WatchState {
        std::vector<WatchExpression> expressions;
        char new_watch_buffer[256];
        char new_label_buffer[256];
        bool show_error;
        std::string error_message;
    } watch_state_;
    
    // Breakpoint UI state
    struct BreakpointUIState {
        char address_buffer[16];
        char condition_buffer[256];
        bool show_add_dialog;
        bool show_error;
        std::string error_message;
    } breakpoint_state_;
    
    // Panel visibility state
    struct PanelVisibility {
        bool cpu_state;
        bool memory;
        bool vdc_registers;
        bool breakpoints;
        bool disassembly;
        bool call_stack;
        bool watch;
        bool controls;
    } panel_visibility_;
    
    // Disassembly cache
    DisassemblyCache disasm_cache_;
};

#endif // UI_IMGUI_DEBUGGER_UI_H
