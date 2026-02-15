# Implementation Plan: In-Game Debugger UI

## Overview

This implementation plan breaks down the in-game debugger UI feature into discrete coding tasks. The debugger UI will be built using Dear ImGui with SDL2 backend, integrating with the existing Debugger class. Tasks are organized to build incrementally, with early validation through tests and checkpoints.

## Tasks

- [x] 1. Set up Dear ImGui integration with SDL2
  - Add Dear ImGui to CMakeLists.txt using FetchContent
  - Include ImGui source files (imgui.cpp, imgui_draw.cpp, imgui_widgets.cpp, imgui_tables.cpp)
  - Include ImGui SDL2 backend files (imgui_impl_sdl2.cpp, imgui_impl_sdlrenderer2.cpp)
  - Add ImGui include directories to videopac target
  - _Requirements: 12.1_

- [ ] 2. Create ImGuiDebuggerUI class skeleton
  - [x] 2.1 Create include/ui/imgui_debugger_ui.h header file
    - Define ImGuiDebuggerUI class with constructor, destructor
    - Define public methods: initialize(), shutdown(), show(), hide(), is_visible(), render(), process_event()
    - Define private panel rendering methods (render_cpu_state_panel, render_memory_panel, etc.)
    - Define internal state structures (MemoryViewerState, DisassemblyState, WatchState, BreakpointUIState)
    - Define WatchExpression structure with Type enum, evaluation method
    - Define DisplayMode enum (Overlay, SplitScreen)
    - _Requirements: 1.1, 1.2, 10.1, 10.2, 12.1_
  
  - [x] 2.2 Create src/ui/imgui_debugger_ui.cpp implementation file
    - Implement constructor and destructor
    - Implement initialize() to set up ImGui context with SDL2 backend
    - Implement shutdown() to clean up ImGui context
    - Implement show() and hide() to control visibility
    - Implement empty render() method (will be filled in later tasks)
    - Implement process_event() to forward SDL events to ImGui
    - _Requirements: 12.1, 12.3, 12.4_

- [ ] 3. Integrate ImGuiDebuggerUI into SDLFrontend
  - [x] 3.1 Add ImGuiDebuggerUI member to SDLFrontend class
    - Add #include "ui/imgui_debugger_ui.h" to frontend_sdl.h
    - Add std::unique_ptr<ImGuiDebuggerUI> imgui_debugger_ui_ member
    - _Requirements: 11.1_
  
  - [x] 3.2 Initialize ImGuiDebuggerUI in SDLFrontend::initialize()
    - Create ImGuiDebuggerUI instance after debugger creation
    - Call imgui_debugger_ui_->initialize()
    - Pass debugger, emulator, window, and renderer to constructor
    - _Requirements: 12.1_
  
  - [x] 3.3 Integrate ImGui rendering into SDLFrontend::render_frame()
    - Call ImGui_ImplSDLRenderer2_NewFrame() at start of frame
    - Call ImGui_ImplSDL2_NewFrame() after SDL renderer new frame
    - Call ImGui::NewFrame() to start ImGui frame
    - Call imgui_debugger_ui_->render() if visible
    - Call ImGui::Render() to finalize ImGui rendering
    - Call ImGui_ImplSDLRenderer2_RenderDrawData() to render ImGui
    - _Requirements: 12.2, 12.5, 12.6_
  
  - [x] 3.4 Forward SDL events to ImGui in SDLFrontend::process_input()
    - Call imgui_debugger_ui_->process_event() for each SDL event
    - Check ImGui::GetIO().WantCaptureMouse and WantCaptureKeyboard to prevent event passthrough
    - _Requirements: 12.3_
  
  - [x] 3.5 Add F12 hotkey to toggle debugger UI
    - In handle_keyboard_event(), check for SDLK_F12
    - Toggle imgui_debugger_ui_ visibility on F12 press
    - Pause emulator when debugger activates, resume when it closes
    - _Requirements: 1.1, 10.9_
  
  - [x] 3.6 Add "Open Debugger" menu item to F10 menu
    - Add MenuAction::OpenDebugger to MenuAction enum in frontend.h
    - Add "Open Debugger" menu item to MenuSystem::build_main_menu()
    - Handle MenuAction::OpenDebugger in SDLFrontend::handle_menu_action()
    - _Requirements: 1.2_

- [x] 4. Checkpoint - Verify ImGui integration
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 5. Implement CPU State Viewer panel
  - [x] 5.1 Implement render_cpu_state_panel() method
    - Create ImGui window titled "CPU State"
    - Get CPU state from emulator using get_cpu_state()
    - Display PC in hexadecimal format using ImGui::Text()
    - Display accumulator (A) in hexadecimal format
    - Display PSW in hexadecimal and binary formats
    - Display stack pointer (SP) value
    - Display all working registers (R0-R7) in hexadecimal format
    - Display current register bank indicator (0 or 1)
    - Display F1 flag and DBF flag
    - Display individual PSW flags (Carry, Auxiliary Carry, F0, Register Bank Select)
    - Get current instruction disassembly from Disassembler
    - Display disassembled current instruction
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 11.6_
  
  - [ ]* 5.2 Write property test for CPU state display completeness
    - **Property 3: CPU State Display Completeness**
    - **Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9**

- [ ] 6. Implement Memory Viewer panel
  - [x] 6.1 Implement render_memory_panel() method
    - Create ImGui window titled "Memory Viewer"
    - Get memory state from emulator using get_memory_state()
    - Implement hex dump display with 16 bytes per row
    - Display address column in hexadecimal format
    - Display hex values for each byte
    - Display ASCII representation column
    - Implement scrolling through address space using ImGui::BeginChild()
    - Implement click-to-edit functionality using ImGui::InputScalar()
    - Implement search functionality with ImGui::InputText()
    - Implement goto address functionality with ImGui::InputText()
    - Track modified bytes and highlight in distinct color
    - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 11.3_
  
  - [ ]* 6.2 Write property test for memory viewer format
    - **Property 4: Memory Viewer Format Correctness**
    - **Validates: Requirements 3.1, 3.2**
  
  - [ ]* 6.3 Write property test for memory modification propagation
    - **Property 6: Memory Modification Propagation**
    - **Validates: Requirements 3.5, 11.3**
  
  - [ ]* 6.4 Write property test for memory search completeness
    - **Property 7: Memory Search Completeness**
    - **Validates: Requirements 3.6**

- [ ] 7. Implement VDC Register Viewer panel
  - [x] 7.1 Implement render_vdc_registers_panel() method
    - Create ImGui window titled "VDC Registers"
    - Get VDC state from emulator using get_vdc_state()
    - Display sprite control registers for sprites 0-3 (position, color, pattern)
    - Display VDC control register (0xA0) with decoded bit flags
    - Display VDC status register (0xA1) with decoded bit flags
    - Display collision register (0xA2) with decoded collision bits
    - Display color register (0xA3) value
    - Display beam position registers (X and Y)
    - Display audio registers (shift register bytes, control)
    - Display grid control registers
    - Use ImGui::TreeNode() for organized display by category
    - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 4.8, 4.9, 11.7_
  
  - [ ]* 7.2 Write property test for VDC register display completeness
    - **Property 10: VDC Register Display Completeness**
    - **Validates: Requirements 4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 4.8, 4.9**

- [ ] 8. Implement Breakpoint Manager panel
  - [x] 8.1 Implement render_breakpoints_panel() method
    - Create ImGui window titled "Breakpoints"
    - Get breakpoints from debugger using get_breakpoints()
    - Display breakpoint list using ImGui::BeginTable()
    - Show columns: Address, Condition, Enabled
    - Display address in hexadecimal (or "-" for condition-only)
    - Display condition string (or "-" for address-only)
    - Display enabled checkbox using ImGui::Checkbox()
    - Implement toggle enabled functionality
    - Implement delete button for each breakpoint using ImGui::Button()
    - Implement "Add Breakpoint" button to show add dialog
    - Implement add breakpoint dialog with address and condition inputs
    - Call debugger->add_breakpoint() when adding breakpoints
    - Call debugger->remove_breakpoint() when deleting breakpoints
    - Call debugger->enable_breakpoint() when toggling enabled status
    - Highlight breakpoint if execution is paused at that address
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7, 5.8, 11.1, 11.2_
  
  - [ ]* 8.2 Write property test for breakpoint creation and storage
    - **Property 11: Breakpoint Creation and Storage**
    - **Validates: Requirements 5.1, 5.2, 5.3, 11.2**
  
  - [ ]* 8.3 Write property test for breakpoint state modification
    - **Property 13: Breakpoint State Modification**
    - **Validates: Requirements 5.5, 5.6**
  
  - [ ]* 8.4 Write unit test for F2 breakpoint toggle
    - Test F2 hotkey toggles breakpoint at current PC
    - _Requirements: 5.9, 6.6_

- [ ] 9. Implement Execution Controls panel
  - [x] 9.1 Implement render_controls_panel() method
    - Create ImGui window titled "Execution Controls"
    - Display current execution status ("Paused" or "Running")
    - Implement "Step (F9)" button that calls debugger->step()
    - Implement "Continue (F5)" button that calls debugger->continue_execution()
    - Implement "Step Over (F8)" button with step over logic
    - Implement "Step Out (Shift+F8)" button with step out logic
    - Display current PC value
    - Display instruction count or cycle count
    - _Requirements: 6.1, 6.2, 6.7, 6.8, 11.4_
  
  - [x] 9.2 Implement handle_shortcuts() method
    - Check for F9 key press and call debugger->step()
    - Check for F5 key press and call debugger->continue_execution()
    - Check for F8 key press (without Shift) and implement step over
    - Check for F8 key press (with Shift) and implement step out
    - Check for F2 key press and toggle breakpoint at current PC or cursor
    - Check for Escape or F12 key press and call hide() + continue_execution()
    - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.6, 10.9_
  
  - [ ]* 9.3 Write property test for single step execution
    - **Property 15: Single Step Execution**
    - **Validates: Requirements 6.1**
  
  - [ ]* 9.4 Write property test for step over call instructions
    - **Property 16: Step Over Call Instructions**
    - **Validates: Requirements 6.3**
  
  - [ ]* 9.5 Write property test for step out of function
    - **Property 17: Step Out of Function**
    - **Validates: Requirements 6.4**

- [x] 10. Checkpoint - Verify core panels work
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 11. Implement Watch Expressions panel
  - [x] 11.1 Implement WatchExpression::evaluate() method
    - Parse expression string to determine type (memory address or register)
    - For memory addresses, read value from MemoryState
    - For registers, read value from CPUState
    - Return evaluated value as uint16
    - _Requirements: 7.1, 7.2_
  
  - [x] 11.2 Implement render_watch_panel() method
    - Create ImGui window titled "Watch Expressions"
    - Display watch expression list using ImGui::BeginTable()
    - Show columns: Label, Expression, Value
    - Display custom label if present
    - Display expression string
    - Evaluate and display current value in hexadecimal
    - Highlight value in distinct color if changed since last step
    - Implement delete button for each watch expression
    - Implement edit functionality using ImGui::InputText()
    - Implement "Add Watch" button with input fields for expression and label
    - Update last_value and changed flag after each evaluation
    - _Requirements: 7.1, 7.2, 7.3, 7.4, 7.5, 7.6_
  
  - [ ]* 11.3 Write property test for watch expression evaluation
    - **Property 20: Watch Expression Evaluation**
    - **Validates: Requirements 7.1, 7.2, 11.6, 11.7**
  
  - [ ]* 11.4 Write property test for watch value change detection
    - **Property 22: Watch Value Change Detection**
    - **Validates: Requirements 7.4**

- [ ] 12. Implement Disassembly Viewer panel
  - [x] 12.1 Create DisassemblyCache helper class
    - Define cache as std::unordered_map<uint16, std::string>
    - Implement get_disassembly() method that checks cache first
    - Implement invalidate() method to clear cache
    - _Requirements: 8.1, 8.2, 8.3, 8.4, 8.5, 8.6_
  
  - [x] 12.2 Implement render_disassembly_panel() method
    - Create ImGui window titled "Disassembly"
    - Get current PC from CPU state
    - Disassemble instructions starting 10 lines before PC
    - Disassemble instructions ending 10 lines after PC
    - Display address column in hexadecimal format
    - Display opcode bytes column in hexadecimal format
    - Display mnemonic and operands column
    - Highlight current PC instruction with distinct background color
    - Display breakpoint indicator (e.g., red dot) for addresses with breakpoints
    - Implement scrolling using ImGui::BeginChild()
    - Implement cursor positioning by clicking on instructions
    - Implement "Run to Cursor" functionality
    - Implement follow PC mode that auto-scrolls to PC
    - _Requirements: 8.1, 8.2, 8.3, 8.4, 8.5, 8.6, 8.7, 8.8, 8.9, 6.5_
  
  - [ ]* 12.3 Write property test for disassembly display format
    - **Property 24: Disassembly Display Format**
    - **Validates: Requirements 8.1, 8.2, 8.3, 8.4, 8.5, 8.6**
  
  - [ ]* 12.4 Write property test for run to cursor
    - **Property 18: Run to Cursor**
    - **Validates: Requirements 6.5**

- [ ] 13. Implement Call Stack Viewer panel
  - [x] 13.1 Implement render_call_stack_panel() method
    - Create ImGui window titled "Call Stack"
    - Get CPU state to access stack
    - Display stack using ImGui::BeginTable()
    - Show columns: Depth, Return Address, Instruction
    - Display depth (0-7 for Intel 8048)
    - Display return address in hexadecimal format
    - Disassemble instruction at return address
    - Implement click handler to navigate disassembly to return address
    - Handle empty stack case gracefully
    - _Requirements: 9.1, 9.2, 9.3, 9.4, 9.5, 9.6_
  
  - [ ]* 13.2 Write property test for call stack display completeness
    - **Property 27: Call Stack Display Completeness**
    - **Validates: Requirements 9.1, 9.2, 9.3, 9.4, 9.6**

- [ ] 14. Implement display mode switching
  - [x] 14.1 Add display mode toggle to controls panel
    - Add "Display Mode" combo box using ImGui::Combo()
    - Options: "Overlay", "Split Screen"
    - Call set_display_mode() when selection changes
    - _Requirements: 10.1, 10.2_
  
  - [x] 14.2 Implement set_display_mode() method
    - Update display_mode_ member variable
    - In Overlay mode, render ImGui on top of full emulation viewport
    - In Split Screen mode, adjust emulation viewport size to make room for debugger
    - Modify SDLFrontend::render_frame() to adjust viewport based on mode
    - _Requirements: 10.2, 12.5, 12.6_
  
  - [ ]* 14.3 Write property test for display mode toggle
    - **Property 28: Display Mode Toggle**
    - **Validates: Requirements 10.2**

- [ ] 15. Implement panel visibility toggles
  - [x] 15.1 Add "View" menu to debugger UI
    - Create menu bar using ImGui::BeginMenuBar()
    - Add "View" menu using ImGui::BeginMenu()
    - Add checkbox menu items for each panel
    - Track visibility state for each panel
    - _Requirements: 10.4_
  
  - [x] 15.2 Conditionally render panels based on visibility
    - Check visibility flag before rendering each panel
    - Skip rendering if panel is hidden
    - _Requirements: 10.4_
  
  - [ ]* 15.3 Write property test for panel visibility toggle
    - **Property 29: Panel Visibility Toggle**
    - **Validates: Requirements 10.4**

- [ ] 16. Implement state persistence
  - [x] 16.1 Implement save_state() method
    - Create JSON object for custom state
    - Serialize breakpoints array (address, condition, enabled)
    - Serialize watch expressions array (type, expression, label)
    - Serialize display mode
    - Serialize panel visibility flags
    - Write JSON to file (e.g., "debugger_state.json")
    - ImGui automatically saves window positions/sizes to imgui.ini
    - _Requirements: 7.7, 10.5, 10.7, 12.7, 12.8_
  
  - [x] 16.2 Implement load_state() method
    - Read JSON from file
    - Deserialize breakpoints and add to debugger
    - Deserialize watch expressions and add to watch list
    - Deserialize display mode and apply
    - Deserialize panel visibility flags and apply
    - Handle missing file gracefully (use defaults)
    - Handle corrupted JSON gracefully (use defaults, log error)
    - ImGui automatically loads window positions/sizes from imgui.ini
    - _Requirements: 7.8, 10.6, 10.8_
  
  - [x] 16.3 Call save_state() in shutdown() and hide()
    - Save state when debugger UI is closed
    - Save state when application shuts down
    - _Requirements: 10.5, 10.7_
  
  - [x] 16.4 Call load_state() in initialize()
    - Load state when debugger UI is initialized
    - _Requirements: 10.6, 10.8_
  
  - [ ]* 16.5 Write property test for state persistence round-trip
    - **Property 30: State Persistence Round-Trip**
    - **Validates: Requirements 7.7, 7.8, 10.5, 10.6, 10.7, 10.8, 12.7**

- [ ] 17. Implement error handling
  - [x] 17.1 Add input validation for memory addresses
    - Validate address is in range 0x0000-0xFFFF
    - Display error message using ImGui::TextColored() if invalid
    - Do not navigate or modify memory if invalid
    - _Requirements: 3.3, 3.4, 3.5_
  
  - [x] 17.2 Add input validation for breakpoint conditions
    - Check condition syntax is valid
    - Display error message if malformed
    - Do not create breakpoint if invalid
    - _Requirements: 5.2, 5.3_
  
  - [x] 17.3 Add input validation for watch expressions
    - Check expression is valid (valid register name or address)
    - Display error message if invalid
    - Do not create watch if invalid
    - _Requirements: 7.1, 7.2_
  
  - [x] 17.4 Add limits for breakpoints and watch expressions
    - Limit breakpoints to 100
    - Limit watch expressions to 50
    - Display warning when limit reached
    - _Requirements: 5.1, 5.2, 5.3, 7.1, 7.2_
  
  - [x] 17.5 Add error handling for state persistence
    - Catch file I/O exceptions in save_state()
    - Display error dialog if save fails
    - Catch JSON parsing exceptions in load_state()
    - Display warning if load fails, use defaults
    - _Requirements: 10.5, 10.6, 10.7, 10.8_
  
  - [ ]* 17.6 Write unit tests for error handling
    - Test invalid memory address handling
    - Test invalid breakpoint condition handling
    - Test invalid watch expression handling
    - Test breakpoint limit enforcement
    - Test watch expression limit enforcement
    - Test state persistence error handling

- [x] 18. Checkpoint - Verify all features work
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 19. Add unit tests for ImGui integration
  - [ ]* 19.1 Write unit test for ImGui initialization
    - Test initialize() succeeds with valid SDL window/renderer
    - Test initialize() fails gracefully with null window/renderer
    - _Requirements: 12.1_
  
  - [ ]* 19.2 Write unit test for ImGui shutdown
    - Test shutdown() cleans up resources without leaking memory
    - _Requirements: 12.4_
  
  - [ ]* 19.3 Write unit test for F12 activation
    - Test F12 key press activates debugger and pauses emulator
    - _Requirements: 1.1_
  
  - [ ]* 19.4 Write unit test for menu activation
    - Test "Open Debugger" menu item activates debugger and pauses emulator
    - _Requirements: 1.2_
  
  - [ ]* 19.5 Write unit test for Escape/F12 close
    - Test Escape or F12 closes debugger and resumes emulation
    - _Requirements: 10.9_

- [ ] 20. Add property tests for debugger backend integration
  - [ ]* 20.1 Write property test for debugger activation preserves state
    - **Property 1: Debugger Activation Preserves Emulator State**
    - **Validates: Requirements 1.3**
  
  - [ ]* 20.2 Write property test for debugger prevents emulation
    - **Property 2: Debugger Prevents Emulation While Active**
    - **Validates: Requirements 1.4**
  
  - [ ]* 20.3 Write property test for debugger backend integration
    - **Property 32: Debugger Backend Integration**
    - **Validates: Requirements 11.1, 11.4, 11.5**

- [x] 21. Final checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

## Future Enhancements

- [ ] 22. Fix window positioning on fullscreen toggle
  - Detect window size changes (windowed ↔ fullscreen transitions)
  - Automatically reposition off-screen panels to visible area
  - Consider using relative positioning (percentages) instead of absolute pixels
  - Alternative: Clear imgui.ini on display mode changes to reset positions
  - _Issue: Panel positions saved in imgui.ini become invalid when window size changes_

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Checkpoints ensure incremental validation
- Property tests validate universal correctness properties
- Unit tests validate specific examples and edge cases
- The implementation builds incrementally: infrastructure → core panels → advanced features → persistence → error handling
