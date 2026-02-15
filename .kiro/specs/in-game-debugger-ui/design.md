# Design Document: In-Game Debugger UI

## Overview

The in-game debugger UI provides a comprehensive graphical debugging interface for the Videopac emulator, built using Dear ImGui. It overlays or splits the screen with the emulation display, offering real-time inspection and control of CPU state, memory, VDC registers, breakpoints, and execution flow. The debugger integrates seamlessly with the existing command-line Debugger and DebuggerUI classes while providing a modern, interactive interface.

The design follows an immediate-mode GUI paradigm using Dear ImGui, which renders UI elements every frame based on current application state. This approach is ideal for debugging tools where state changes frequently and UI needs to reflect real-time data.

## Architecture

### Component Structure

```
┌─────────────────────────────────────────────────────────────┐
│                      SDLFrontend                             │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              ImGuiDebuggerUI                         │   │
│  │  ┌────────────┐  ┌────────────┐  ┌────────────┐    │   │
│  │  │ CPU Panel  │  │ Memory     │  │ VDC Panel  │    │   │
│  │  │            │  │ Panel      │  │            │    │   │
│  │  └────────────┘  └────────────┘  └────────────┘    │   │
│  │  ┌────────────┐  ┌────────────┐  ┌────────────┐    │   │
│  │  │ Breakpoint │  │ Disassembly│  │ Call Stack │    │   │
│  │  │ Panel      │  │ Panel      │  │ Panel      │    │   │
│  │  └────────────┘  └────────────┘  └────────────┘    │   │
│  │  ┌────────────┐  ┌────────────┐                    │   │
│  │  │ Watch      │  │ Controls   │                    │   │
│  │  │ Panel      │  │ Panel      │                    │   │
│  │  └────────────┘  └────────────┘                    │   │
│  └──────────────────────────────────────────────────────┘   │
│                          │                                   │
│                          ↓                                   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │                    Debugger                          │   │
│  │  (Existing backend - breakpoints, stepping, etc.)   │   │
│  └──────────────────────────────────────────────────────┘   │
│                          │                                   │
│                          ↓                                   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │                 EmulatorCore                         │   │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐          │   │
│  │  │   CPU    │  │  Memory  │  │   VDC    │          │   │
│  │  └──────────┘  └──────────┘  └──────────┘          │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### Key Design Decisions

1. **Dear ImGui Integration**: Use Dear ImGui with SDL2 backend for rendering. ImGui provides dockable windows, rich widgets, and excellent performance for tool UIs.

2. **Immediate Mode Rendering**: All panels render every frame based on current emulator state. No need to manually track state changes or update UI elements.

3. **Separation of Concerns**: 
   - `ImGuiDebuggerUI`: Handles all ImGui rendering and user interaction
   - `Debugger`: Existing backend for breakpoint management and execution control
   - `EmulatorCore`: Provides state inspection methods

4. **Overlay vs Split-Screen**: Support both modes:
   - Overlay: ImGui renders on top of emulation framebuffer (default)
   - Split-Screen: Emulation viewport shrinks to make room for debugger panels

5. **State Persistence**: Use ImGui's built-in INI file system for window positions/sizes, plus custom JSON for breakpoints and watch expressions.

## Components and Interfaces

### ImGuiDebuggerUI Class

Main class responsible for rendering all debugger panels and handling user interaction.

```cpp
class ImGuiDebuggerUI {
public:
    ImGuiDebuggerUI(Debugger* debugger, EmulatorCore* emulator, 
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
    
    // State persistence
    void save_state();
    void load_state();
    
private:
    // Panel rendering methods
    void render_cpu_state_panel();
    void render_memory_panel();
    void render_vdc_registers_panel();
    void render_breakpoints_panel();
    void render_disassembly_panel();
    void render_call_stack_panel();
    void render_watch_panel();
    void render_controls_panel();
    
    // Helper methods
    void render_register(const char* name, uint8 value);
    void render_register_16(const char* name, uint16 value);
    void render_psw_flags(uint8 psw);
    void render_memory_editor(uint16 start_address, size_t size);
    void render_hex_dump(const uint8* data, size_t size, uint16 base_address);
    
    // Keyboard shortcuts
    void handle_shortcuts();
    
    // State
    Debugger* debugger_;
    EmulatorCore* emulator_;
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    
    bool visible_;
    DisplayMode display_mode_;
    
    // ImGui context
    ImGuiContext* imgui_context_;
    
    // Panel state
    struct MemoryViewerState {
        uint16 current_address;
        uint16 goto_address;
        bool goto_pending;
        std::string search_query;
        std::vector<uint16> search_results;
        int search_result_index;
        uint8 edit_value;
        uint16 edit_address;
        bool editing;
    } memory_state_;
    
    struct DisassemblyState {
        uint16 current_address;
        uint16 cursor_address;
        bool follow_pc;
    } disasm_state_;
    
    struct WatchState {
        std::vector<WatchExpression> expressions;
        char new_watch_buffer[256];
        char new_label_buffer[256];
    } watch_state_;
    
    // Breakpoint UI state
    struct BreakpointUIState {
        char address_buffer[16];
        char condition_buffer[256];
        bool show_add_dialog;
    } breakpoint_state_;
};
```

### WatchExpression Structure

```cpp
struct WatchExpression {
    enum class Type {
        MemoryAddress,
        Register
    };
    
    Type type;
    std::string expression;  // e.g., "0x1234", "A", "R0"
    std::string label;       // User-defined label
    uint16 last_value;       // For change detection
    bool changed;            // Highlight if changed
    
    // Evaluation
    uint16 evaluate(const CPUState& cpu, const MemoryState& memory) const;
};
```

### DisplayMode Enum

```cpp
enum class DisplayMode {
    Overlay,      // ImGui renders on top of emulation
    SplitScreen   // Emulation viewport shrinks
};
```

### Integration with SDLFrontend

The SDLFrontend class will be extended to manage the ImGuiDebuggerUI:

```cpp
class SDLFrontend : public Frontend {
    // ... existing members ...
    
    // Add ImGui debugger UI
    std::unique_ptr<ImGuiDebuggerUI> imgui_debugger_ui_;
    
    // Modified render_frame() to handle debugger overlay
    void render_frame() override;
    
    // Modified process_input() to forward events to ImGui
    void process_input() override;
    
    // Handle F12 hotkey to toggle debugger
    void handle_keyboard_event(const SDL_KeyboardEvent& event) override;
};
```

## Data Models

### CPU State Display

The CPU state panel displays data from `CPUState` structure:

```cpp
struct CPUState {
    uint16 pc;                  // Program counter
    uint8 a;                    // Accumulator
    uint8 psw;                  // Program Status Word
    uint8 r[16];                // Registers R0-R7 and R0'-R7'
    uint8 sp;                   // Stack pointer
    uint16 stack[8];            // 8-level stack
    uint8 current_bank;         // Current register bank (0 or 1)
    bool f1_flag;               // F1 flag
    bool memory_bank;           // Memory bank flag (DBF)
    // ... other fields ...
};
```

Display format:
```
PC: 0x0234    A: 0x5A    PSW: 0b10110100
SP: 3         Bank: 0    F1: 0    DBF: 0

Flags: [C] [AC] [F0] [BS]

R0: 0x12  R1: 0x34  R2: 0x56  R3: 0x78
R4: 0x9A  R5: 0xBC  R6: 0xDE  R7: 0xF0

Current Instruction: MOV A, R0
```

### Memory Viewer Layout

Memory displayed in hex dump format with ASCII sidebar:

```
Address   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  ASCII
0x0000    48 65 6C 6C 6F 20 57 6F 72 6C 64 21 00 00 00 00  Hello World!....
0x0010    FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF  ................
...
```

Features:
- Scrollable view
- Click to edit bytes
- Search functionality
- Goto address
- Highlight modified bytes

### VDC Register Display

VDC registers organized by category:

```
Sprites:
  Sprite 0: X=120 Y=100 Color=3 Pattern=0x80
  Sprite 1: X=140 Y=110 Color=5 Pattern=0x88
  ...

Control (0xA0): 0b00101000
  [✓] Display Enable
  [✓] Grid Enable
  [ ] HBlank Interrupt
  ...

Status (0xA1): 0b00001000
  [✓] VBlank
  [ ] HBlank
  ...

Collision (0xA2): 0b00000011
  [✓] Sprite 0
  [✓] Sprite 1
  ...

Audio:
  Shift Register: 0x123456
  Volume: 12
  Frequency: 983 Hz
  [✓] Enabled
  [ ] Loop
  [ ] Noise
```

### Breakpoint List

```
Address    Condition              Enabled
0x0234     -                      [✓]
0x0456     A==0xFF                [✓]
-          R0>0x80                [✓]
0x0789     -                      [ ]
```

### Disassembly View

```
Address   Opcode    Instruction
0x0230    23        MOV A, #0x23
0x0231    F0        MOV A, @R0
0x0232    A8        MOV R0, A
→ 0x0233  04 56     JMP 0x0456      ← Current PC (highlighted)
0x0234    17        INC A
0x0235    96        JNZ 0x0235
...
```

Features:
- Highlight current PC
- Show breakpoint indicators
- Click to set cursor
- Scroll to follow PC or manual navigation

### Call Stack Display

```
Depth  Return Address  Instruction
0      0x0456          CALL 0x0234
1      0x0123          CALL 0x0450
2      0x0890          CALL 0x0100
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*


### Property 1: Debugger Activation Preserves Emulator State

*For any* emulator state (CPU registers, memory contents, VDC state), activating the debugger UI should preserve all state values unchanged.

**Validates: Requirements 1.3**

### Property 2: Debugger Prevents Emulation While Active

*For any* emulator state, while the debugger UI is active and not explicitly continuing execution, the emulator should not advance its state (PC should not change, no instructions executed).

**Validates: Requirements 1.4**

### Property 3: CPU State Display Completeness

*For any* CPU state, the CPU state viewer should display all required fields: PC, A, PSW (in hex and binary), SP, all working registers (R0-R7), current bank, F1 flag, DBF flag, individual PSW flags, and the disassembled current instruction.

**Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9**

### Property 4: Memory Viewer Format Correctness

*For any* memory contents, the memory viewer should display data in hexadecimal format with exactly 16 bytes per row, and include ASCII representation alongside hex values.

**Validates: Requirements 3.1, 3.2**

### Property 5: Memory Viewer Navigation Range

*For any* address in the range 0x0000-0xFFFF, the memory viewer should allow scrolling to and displaying that address.

**Validates: Requirements 3.3**

### Property 6: Memory Modification Propagation

*For any* memory address and any byte value, modifying the value through the memory viewer should immediately update the corresponding location in the emulator's memory system.

**Validates: Requirements 3.5**

### Property 7: Memory Search Completeness

*For any* byte sequence that exists in memory, the memory viewer search function should locate and return all occurrences of that sequence.

**Validates: Requirements 3.6**

### Property 8: Memory Goto Navigation

*For any* valid memory address, using the goto function should navigate the memory viewer to display that address.

**Validates: Requirements 3.7**

### Property 9: Memory Change Highlighting

*For any* memory location that changes value during execution, the memory viewer should highlight that location in a distinct color.

**Validates: Requirements 3.8**

### Property 10: VDC Register Display Completeness

*For any* VDC state, the VDC register viewer should display all required data: all sprite control registers (position, color, pattern) for sprites 0-3, control register with decoded flags, status register with decoded flags, collision register with decoded bits, color register, beam position, audio registers, and grid control registers.

**Validates: Requirements 4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 4.8, 4.9**

### Property 11: Breakpoint Creation and Storage

*For any* valid address, condition string, or combination thereof, adding a breakpoint through the UI should create a corresponding breakpoint in the Debugger backend that is checked during execution.

**Validates: Requirements 5.1, 5.2, 5.3, 11.2**

### Property 12: Breakpoint Display Completeness

*For any* set of breakpoints, the breakpoint manager should display all breakpoints with their address (or "-" for condition-only), condition (or "-" for address-only), and enabled status.

**Validates: Requirements 5.4**

### Property 13: Breakpoint State Modification

*For any* breakpoint, toggling its enabled status or deleting it through the UI should immediately update the breakpoint state in the Debugger backend.

**Validates: Requirements 5.5, 5.6**

### Property 14: Breakpoint Triggering

*For any* enabled breakpoint, when execution reaches the breakpoint address (or when a condition evaluates to true), the emulator should pause and the breakpoint should be highlighted in the UI.

**Validates: Requirements 5.7, 5.8**

### Property 15: Single Step Execution

*For any* emulator state, executing a single step (F9) should advance the PC by exactly one instruction and update all viewer panels to reflect the new state.

**Validates: Requirements 6.1**

### Property 16: Step Over Call Instructions

*For any* CALL instruction, step over (F8) should execute until the instruction immediately following the CALL completes (after the called function returns).

**Validates: Requirements 6.3**

### Property 17: Step Out of Function

*For any* function call context (non-empty call stack), step out (Shift+F8) should execute until the current function returns and the next instruction in the caller executes.

**Validates: Requirements 6.4**

### Property 18: Run to Cursor

*For any* cursor address in the disassembly viewer, run to cursor should execute instructions until the PC reaches that address.

**Validates: Requirements 6.5**

### Property 19: Execution Status Display

*For any* emulator state, the execution controls should display "Paused" when execution is paused and "Running" when execution is running.

**Validates: Requirements 6.7, 6.8**

### Property 20: Watch Expression Evaluation

*For any* valid watch expression (memory address or register name), the debugger UI should display the current value of that expression evaluated against the current emulator state.

**Validates: Requirements 7.1, 7.2, 11.6, 11.7**

### Property 21: Watch Expression Label Display

*For any* watch expression with a custom label, the debugger UI should display the label alongside the evaluated value.

**Validates: Requirements 7.3**

### Property 22: Watch Value Change Detection

*For any* watch expression, if the evaluated value changes between execution steps, the debugger UI should highlight the changed value in a distinct color.

**Validates: Requirements 7.4**

### Property 23: Watch Expression Modification

*For any* watch expression, deleting or editing it through the UI should immediately update the watch list and display the new state.

**Validates: Requirements 7.5, 7.6**

### Property 24: Disassembly Display Format

*For any* address range around the PC, the disassembly viewer should display instructions with their address in hexadecimal, opcode bytes in hexadecimal, and mnemonic with operands, with the current PC instruction highlighted.

**Validates: Requirements 8.1, 8.2, 8.3, 8.4, 8.5, 8.6**

### Property 25: Disassembly Breakpoint Indicators

*For any* address with a breakpoint, the disassembly viewer should display a breakpoint indicator at that address.

**Validates: Requirements 8.7**

### Property 26: Disassembly Navigation Range

*For any* address in the valid address space, the disassembly viewer should allow scrolling to and displaying disassembled instructions at that address.

**Validates: Requirements 8.8**

### Property 27: Call Stack Display Completeness

*For any* CPU state with a non-empty stack, the call stack viewer should display all return addresses from the stack with their depth (0-7), address in hexadecimal, and disassembled instruction at each return address.

**Validates: Requirements 9.1, 9.2, 9.3, 9.4, 9.6**

### Property 28: Display Mode Toggle

*For any* current display mode (Overlay or Split-Screen), toggling the display mode should switch to the other mode.

**Validates: Requirements 10.2**

### Property 29: Panel Visibility Toggle

*For any* panel, toggling its visibility should change its display state (hidden panels should become visible, visible panels should become hidden).

**Validates: Requirements 10.4**

### Property 30: State Persistence Round-Trip

*For any* debugger UI state (watch expressions, breakpoints, layout configuration, window positions/sizes), saving the state to disk and then loading it should restore an equivalent state.

**Validates: Requirements 7.7, 7.8, 10.5, 10.6, 10.7, 10.8, 12.7**

### Property 31: Memory Modification Through Debugger

*For any* memory address and value, modifying memory through the debugger UI should update the emulator's MemorySystem state, which should be reflected in subsequent memory reads.

**Validates: Requirements 11.3**

### Property 32: Debugger Backend Integration

*For any* debugger operation (step, continue, breakpoint management), the ImGuiDebuggerUI should use the existing Debugger class methods rather than directly manipulating emulator state.

**Validates: Requirements 11.1, 11.4, 11.5**

## Error Handling

### Invalid Input Handling

1. **Invalid Memory Addresses**: When user enters an address outside valid range (0x0000-0xFFFF), display error message and do not navigate.

2. **Invalid Breakpoint Conditions**: When user enters a malformed condition expression, display error message and do not create breakpoint.

3. **Invalid Watch Expressions**: When user enters an invalid expression (non-existent register, malformed address), display error message and do not create watch.

4. **Memory Edit Validation**: When user enters non-hexadecimal value for memory edit, reject input and display error.

### Resource Exhaustion

1. **Too Many Breakpoints**: Limit breakpoints to reasonable number (e.g., 100). Display warning when limit reached.

2. **Too Many Watch Expressions**: Limit watch expressions to reasonable number (e.g., 50). Display warning when limit reached.

3. **Memory Search Timeout**: If memory search takes too long (>1 second), cancel search and display timeout message.

### State Persistence Errors

1. **Failed to Save State**: If saving state to disk fails (permissions, disk full), display error dialog with specific error message.

2. **Failed to Load State**: If loading state fails (file not found, corrupted), display error dialog and use default state.

3. **Corrupted State File**: If state file is corrupted, display warning and use default state rather than crashing.

### ImGui Initialization Errors

1. **Failed to Initialize ImGui**: If ImGui initialization fails, log error and disable debugger UI (fall back to command-line debugger).

2. **Failed to Load Fonts**: If font loading fails, use ImGui default font as fallback.

3. **Failed to Create ImGui Context**: If context creation fails, log error and disable debugger UI.

## Testing Strategy

### Dual Testing Approach

The testing strategy employs both unit tests and property-based tests to ensure comprehensive coverage:

- **Unit tests**: Verify specific examples, edge cases, error conditions, and integration points
- **Property tests**: Verify universal properties across all inputs through randomization

### Unit Testing Focus

Unit tests should focus on:
- Specific UI interaction examples (F12 activation, menu selection, button clicks)
- Edge cases (empty memory, empty call stack, no breakpoints)
- Error conditions (invalid addresses, malformed conditions, file I/O errors)
- Integration points (ImGui initialization, SDL event handling, Debugger backend calls)

### Property-Based Testing Configuration

- **Library**: Use RapidCheck (already in project) for C++ property-based testing
- **Iterations**: Minimum 100 iterations per property test
- **Tagging**: Each property test must reference its design document property
- **Tag format**: `// Feature: in-game-debugger-ui, Property N: [property text]`

### Test Organization

```cpp
// Unit tests
TEST(ImGuiDebuggerUI, ActivateWithF12) {
    // Test specific F12 activation
}

TEST(ImGuiDebuggerUI, HandleInvalidMemoryAddress) {
    // Test error handling for invalid address
}

// Property tests
TEST(ImGuiDebuggerUI, PropertyActivationPreservesState) {
    // Feature: in-game-debugger-ui, Property 1: Debugger Activation Preserves Emulator State
    rc::check([](const CPUState& cpu, const MemoryState& mem, const VDCState& vdc) {
        // Generate random emulator state
        // Activate debugger
        // Verify state unchanged
    });
}

TEST(ImGuiDebuggerUI, PropertyMemoryModificationPropagation) {
    // Feature: in-game-debugger-ui, Property 6: Memory Modification Propagation
    rc::check([](uint16 address, uint8 value) {
        // Modify memory through UI
        // Verify emulator memory updated
    });
}
```

### Integration Testing

Integration tests should verify:
- ImGui renders correctly with SDL2 backend
- Debugger UI integrates with existing Debugger class
- State persistence works end-to-end (save/load cycle)
- Keyboard shortcuts work correctly
- Panel interactions work correctly

### Manual Testing

Some aspects require manual testing:
- Visual appearance and layout
- Panel resizing and docking behavior
- Performance with large memory dumps
- Usability and workflow efficiency

## Implementation Notes

### Dear ImGui Integration

1. **Initialization**: Initialize ImGui in `SDLFrontend::initialize()` after SDL initialization
2. **Event Handling**: Forward SDL events to ImGui in `SDLFrontend::process_input()`
3. **Rendering**: Render ImGui in `SDLFrontend::render_frame()` after emulation framebuffer
4. **Cleanup**: Shutdown ImGui in `SDLFrontend::shutdown()`

### CMake Configuration

Add Dear ImGui to the project:

```cmake
# Find or fetch Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.0
)
FetchContent_MakeAvailable(imgui)

# Add ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

# Link ImGui to videopac executable
target_sources(videopac PRIVATE ${IMGUI_SOURCES})
target_include_directories(videopac PRIVATE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
```

### State Persistence Format

Use JSON for custom state (breakpoints, watch expressions):

```json
{
  "breakpoints": [
    {"address": 564, "condition": "", "enabled": true},
    {"address": 1110, "condition": "A==0xFF", "enabled": true},
    {"address": 0, "condition": "R0>0x80", "enabled": true, "condition_only": true}
  ],
  "watch_expressions": [
    {"type": "memory", "expression": "0x1234", "label": "Player X Position"},
    {"type": "register", "expression": "A", "label": "Accumulator"}
  ],
  "display_mode": "overlay",
  "panel_visibility": {
    "cpu_state": true,
    "memory": true,
    "vdc_registers": true,
    "breakpoints": true,
    "disassembly": true,
    "call_stack": true,
    "watch": true,
    "controls": true
  }
}
```

ImGui will automatically save window positions/sizes to `imgui.ini`.

### Performance Considerations

1. **Lazy Evaluation**: Only evaluate watch expressions and render visible panels
2. **Caching**: Cache disassembly results to avoid re-disassembling every frame
3. **Throttling**: Update memory change highlighting only when execution steps, not every frame
4. **Efficient Rendering**: Use ImGui's clipping to avoid rendering off-screen content

### Keyboard Shortcut Handling

Keyboard shortcuts should be handled in `ImGuiDebuggerUI::handle_shortcuts()`:

```cpp
void ImGuiDebuggerUI::handle_shortcuts() {
    ImGuiIO& io = ImGui::GetIO();
    
    // F9: Step
    if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
        debugger_->step();
    }
    
    // F5: Continue
    if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
        debugger_->continue_execution();
    }
    
    // F8: Step Over
    if (ImGui::IsKeyPressed(ImGuiKey_F8) && !io.KeyShift) {
        // Implement step over logic
    }
    
    // Shift+F8: Step Out
    if (ImGui::IsKeyPressed(ImGuiKey_F8) && io.KeyShift) {
        // Implement step out logic
    }
    
    // F2: Toggle Breakpoint
    if (ImGui::IsKeyPressed(ImGuiKey_F2)) {
        // Toggle breakpoint at current PC or cursor
    }
    
    // Escape or F12: Close Debugger
    if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_F12)) {
        hide();
        debugger_->continue_execution();
    }
}
```

### Memory Editor Widget

Use ImGui's memory editor widget for the memory viewer:

```cpp
void ImGuiDebuggerUI::render_memory_panel() {
    if (ImGui::Begin("Memory Viewer")) {
        // Get memory state
        MemoryState mem_state = emulator_->get_memory_state();
        
        // Render memory editor
        static MemoryEditor mem_edit;
        mem_edit.DrawContents(mem_state.ram, sizeof(mem_state.ram), 0x0000);
        
        // Search functionality
        if (ImGui::InputText("Search", memory_state_.search_query, 256)) {
            // Perform search
        }
        
        // Goto functionality
        if (ImGui::InputText("Goto", memory_state_.goto_address_str, 16)) {
            // Parse and navigate to address
        }
    }
    ImGui::End();
}
```

### Disassembly Caching

Cache disassembly results to improve performance:

```cpp
struct DisassemblyCache {
    std::unordered_map<uint16, std::string> cache;
    
    std::string get_disassembly(uint16 address, Disassembler* disasm) {
        auto it = cache.find(address);
        if (it != cache.end()) {
            return it->second;
        }
        
        // Disassemble and cache
        std::string result = disasm->disassemble_instruction(address);
        cache[address] = result;
        return result;
    }
    
    void invalidate() {
        cache.clear();
    }
};
```

Invalidate cache when memory is modified or ROM is loaded.
