# Requirements Document: In-Game Debugger UI

## Introduction

This document specifies the requirements for an in-game graphical debugger UI for the Videopac emulator. The debugger UI provides real-time visualization and control of the emulator's CPU, memory, VDC registers, and execution flow. It builds upon the existing command-line debugger infrastructure (Debugger and DebuggerUI classes) to provide an interactive, overlay-based debugging experience accessible during emulation.

## Glossary

- **Debugger_UI**: The graphical in-game debugger interface that overlays the emulation display, implemented using Dear ImGui
- **Dear_ImGui**: An immediate-mode graphical user interface library for C++ used to render the debugger panels
- **CPU_State_Viewer**: A panel displaying all CPU registers, flags, and the current instruction
- **Memory_Viewer**: A panel for browsing and editing RAM/ROM with hex and ASCII display
- **VDC_Register_Viewer**: A panel showing all VDC (Video Display Controller) registers in real-time
- **Breakpoint_Manager**: A panel for managing execution breakpoints with address and condition support
- **Watch_Expression**: A user-defined monitor for specific memory addresses or CPU registers
- **Disassembly_Viewer**: A panel showing disassembled machine code around the current program counter
- **Call_Stack_Viewer**: A panel displaying the CPU's call stack with return addresses
- **Execution_Controls**: UI controls for stepping through code, continuing execution, and managing breakpoints
- **Overlay_Mode**: A display mode where the debugger UI is rendered on top of the emulation display
- **Split_Screen_Mode**: A display mode where the debugger UI and emulation display share screen space
- **Emulator_Core**: The main emulation system containing CPU, VDC, and memory components
- **Debugger**: The existing backend debugger class that manages breakpoints and execution control
- **VDC**: Video Display Controller - the Intel 8245 graphics chip
- **PC**: Program Counter - the CPU register pointing to the current instruction
- **PSW**: Program Status Word - the CPU flags register
- **F10_Menu**: The existing in-game menu system accessed via F10 key

## Requirements

### Requirement 1: Debugger UI Activation

**User Story:** As a developer, I want to activate the debugger UI during emulation, so that I can inspect and debug the running program without restarting.

#### Acceptance Criteria

1. WHEN the user presses F12 during emulation, THE Debugger_UI SHALL activate and pause the Emulator_Core
2. WHEN the user selects "Open Debugger" from the F10_Menu, THE Debugger_UI SHALL activate and pause the Emulator_Core
3. WHEN the Debugger_UI activates, THE Emulator_Core SHALL preserve its current state including CPU registers, memory, and VDC state
4. WHEN the Debugger_UI is active, THE Emulator_Core SHALL remain paused until the user resumes execution

### Requirement 2: CPU State Viewer

**User Story:** As a developer, I want to view all CPU registers and flags in real-time, so that I can understand the current execution state.

#### Acceptance Criteria

1. WHEN the Debugger_UI is active, THE CPU_State_Viewer SHALL display the program counter (PC) in hexadecimal format
2. WHEN the Debugger_UI is active, THE CPU_State_Viewer SHALL display the accumulator (A) in hexadecimal format
3. WHEN the Debugger_UI is active, THE CPU_State_Viewer SHALL display the program status word (PSW) in hexadecimal and binary formats
4. WHEN the Debugger_UI is active, THE CPU_State_Viewer SHALL display the stack pointer (SP) value
5. WHEN the Debugger_UI is active, THE CPU_State_Viewer SHALL display all working registers (R0-R7) in hexadecimal format
6. WHEN the Debugger_UI is active, THE CPU_State_Viewer SHALL display the current register bank indicator (0 or 1)
7. WHEN the Debugger_UI is active, THE CPU_State_Viewer SHALL display individual PSW flags (Carry, Auxiliary Carry, F0, Register Bank Select)
8. WHEN the Debugger_UI is active, THE CPU_State_Viewer SHALL display the disassembled current instruction at PC
9. WHEN the CPU state changes during stepping, THE CPU_State_Viewer SHALL update all displayed values immediately

### Requirement 3: Memory Viewer and Editor

**User Story:** As a developer, I want to browse and modify memory contents, so that I can inspect data structures and test behavior with modified values.

#### Acceptance Criteria

1. WHEN the Memory_Viewer is displayed, THE Memory_Viewer SHALL show memory contents in hexadecimal format with 16 bytes per row
2. WHEN the Memory_Viewer is displayed, THE Memory_Viewer SHALL show ASCII representation of memory contents alongside hexadecimal values
3. WHEN the user scrolls in the Memory_Viewer, THE Memory_Viewer SHALL allow navigation through the entire address space (0x0000-0xFFFF)
4. WHEN the user clicks on a memory address, THE Memory_Viewer SHALL allow editing the byte value at that address
5. WHEN the user modifies a memory value, THE Emulator_Core SHALL update the corresponding memory location immediately
6. WHEN the user enters a search query, THE Memory_Viewer SHALL locate and highlight matching byte sequences in memory
7. WHEN the user enters a goto address, THE Memory_Viewer SHALL scroll to display that memory address
8. WHEN memory contents change during execution, THE Memory_Viewer SHALL highlight modified bytes in a distinct color

### Requirement 4: VDC Register Viewer

**User Story:** As a developer, I want to view all VDC registers in real-time, so that I can debug graphics and sprite rendering issues.

#### Acceptance Criteria

1. WHEN the VDC_Register_Viewer is displayed, THE VDC_Register_Viewer SHALL show all sprite control registers (position, color, pattern) for sprites 0-3
2. WHEN the VDC_Register_Viewer is displayed, THE VDC_Register_Viewer SHALL show the VDC control register (0xA0) with individual bit flags decoded
3. WHEN the VDC_Register_Viewer is displayed, THE VDC_Register_Viewer SHALL show the VDC status register (0xA1) with individual bit flags decoded
4. WHEN the VDC_Register_Viewer is displayed, THE VDC_Register_Viewer SHALL show the collision register (0xA2) with collision bits decoded
5. WHEN the VDC_Register_Viewer is displayed, THE VDC_Register_Viewer SHALL show the color register (0xA3) value
6. WHEN the VDC_Register_Viewer is displayed, THE VDC_Register_Viewer SHALL show beam position registers (X and Y)
7. WHEN the VDC_Register_Viewer is displayed, THE VDC_Register_Viewer SHALL show audio registers (sound shift register bytes and control)
8. WHEN the VDC_Register_Viewer is displayed, THE VDC_Register_Viewer SHALL show grid control registers
9. WHEN VDC register values change during execution, THE VDC_Register_Viewer SHALL update displayed values immediately

### Requirement 5: Breakpoint Manager

**User Story:** As a developer, I want to manage execution breakpoints with conditions, so that I can pause execution at specific points of interest.

#### Acceptance Criteria

1. WHEN the user adds a breakpoint with an address, THE Breakpoint_Manager SHALL create a breakpoint at that address
2. WHEN the user adds a breakpoint with an address and condition, THE Breakpoint_Manager SHALL create a conditional breakpoint
3. WHEN the user adds a breakpoint with only a condition, THE Breakpoint_Manager SHALL create a condition-only breakpoint
4. WHEN the Breakpoint_Manager displays breakpoints, THE Breakpoint_Manager SHALL show the address, condition, and enabled status for each breakpoint
5. WHEN the user toggles a breakpoint's enabled status, THE Breakpoint_Manager SHALL update the breakpoint state in the Debugger
6. WHEN the user deletes a breakpoint, THE Breakpoint_Manager SHALL remove the breakpoint from the Debugger
7. WHEN execution reaches a breakpoint address, THE Emulator_Core SHALL pause and highlight the breakpoint in the Breakpoint_Manager
8. WHEN a conditional breakpoint's condition evaluates to true, THE Emulator_Core SHALL pause execution
9. WHEN the user presses F2 in the Disassembly_Viewer, THE Breakpoint_Manager SHALL toggle a breakpoint at the current cursor address

### Requirement 6: Execution Controls

**User Story:** As a developer, I want to control program execution with keyboard shortcuts, so that I can efficiently step through code and resume execution.

#### Acceptance Criteria

1. WHEN the user presses F9, THE Emulator_Core SHALL execute a single instruction and update all viewer panels
2. WHEN the user presses F5, THE Emulator_Core SHALL resume continuous execution until a breakpoint is hit
3. WHEN the user presses F8, THE Emulator_Core SHALL execute until the next instruction after a CALL instruction (step over)
4. WHEN the user presses Shift+F8, THE Emulator_Core SHALL execute until a RET instruction completes (step out)
5. WHEN the user selects "Run to Cursor" in the Disassembly_Viewer, THE Emulator_Core SHALL execute until the cursor address is reached
6. WHEN the user presses F2, THE Breakpoint_Manager SHALL toggle a breakpoint at the current PC or cursor position
7. WHEN execution is paused, THE Execution_Controls SHALL display "Paused" status
8. WHEN execution is running, THE Execution_Controls SHALL display "Running" status

### Requirement 7: Watch Expressions

**User Story:** As a developer, I want to monitor specific memory addresses and registers with custom labels, so that I can track important values during execution.

#### Acceptance Criteria

1. WHEN the user adds a watch expression with a memory address, THE Debugger_UI SHALL display the current value at that address
2. WHEN the user adds a watch expression with a register name, THE Debugger_UI SHALL display the current value of that register
3. WHEN the user adds a custom label to a watch expression, THE Debugger_UI SHALL display the label alongside the value
4. WHEN a watched value changes during execution, THE Debugger_UI SHALL highlight the changed value in a distinct color
5. WHEN the user deletes a watch expression, THE Debugger_UI SHALL remove it from the watch list
6. WHEN the user edits a watch expression, THE Debugger_UI SHALL update the expression and display the new value
7. WHEN the Debugger_UI saves state, THE Debugger_UI SHALL persist all watch expressions to disk
8. WHEN the Debugger_UI loads state, THE Debugger_UI SHALL restore all watch expressions from disk

### Requirement 8: Disassembly Viewer

**User Story:** As a developer, I want to view disassembled code around the current PC, so that I can understand the program flow and set breakpoints.

#### Acceptance Criteria

1. WHEN the Disassembly_Viewer is displayed, THE Disassembly_Viewer SHALL show disassembled instructions starting 10 lines before the current PC
2. WHEN the Disassembly_Viewer is displayed, THE Disassembly_Viewer SHALL show disassembled instructions ending 10 lines after the current PC
3. WHEN the Disassembly_Viewer is displayed, THE Disassembly_Viewer SHALL highlight the current PC instruction in a distinct color
4. WHEN the Disassembly_Viewer is displayed, THE Disassembly_Viewer SHALL show instruction addresses in hexadecimal format
5. WHEN the Disassembly_Viewer is displayed, THE Disassembly_Viewer SHALL show instruction opcodes in hexadecimal format
6. WHEN the Disassembly_Viewer is displayed, THE Disassembly_Viewer SHALL show instruction mnemonics and operands
7. WHEN a breakpoint exists at an address, THE Disassembly_Viewer SHALL display a breakpoint indicator at that address
8. WHEN the user scrolls in the Disassembly_Viewer, THE Disassembly_Viewer SHALL allow navigation through the entire address space
9. WHEN the user clicks on an instruction, THE Disassembly_Viewer SHALL allow setting the cursor position for "Run to Cursor" functionality

### Requirement 9: Call Stack Viewer

**User Story:** As a developer, I want to view the call stack with return addresses, so that I can understand the program's execution context.

#### Acceptance Criteria

1. WHEN the Call_Stack_Viewer is displayed, THE Call_Stack_Viewer SHALL show all return addresses from the CPU stack
2. WHEN the Call_Stack_Viewer is displayed, THE Call_Stack_Viewer SHALL show stack depth (0-7 for Intel 8048)
3. WHEN the Call_Stack_Viewer is displayed, THE Call_Stack_Viewer SHALL show each return address in hexadecimal format
4. WHEN the Call_Stack_Viewer is displayed, THE Call_Stack_Viewer SHALL show the disassembled instruction at each return address
5. WHEN the user clicks on a stack frame, THE Disassembly_Viewer SHALL navigate to that return address
6. WHEN the stack changes during execution, THE Call_Stack_Viewer SHALL update the displayed stack frames immediately

### Requirement 10: UI Layout and Persistence

**User Story:** As a developer, I want to customize the debugger layout and have my preferences persist, so that I can optimize my debugging workflow.

#### Acceptance Criteria

1. WHEN the Debugger_UI activates, THE Debugger_UI SHALL display in Overlay_Mode by default
2. WHEN the user toggles display mode, THE Debugger_UI SHALL switch between Overlay_Mode and Split_Screen_Mode
3. WHEN the user resizes a panel, THE Debugger_UI SHALL update the panel dimensions and reflow other panels
4. WHEN the user toggles a panel's visibility, THE Debugger_UI SHALL hide or show that panel
5. WHEN the user closes the Debugger_UI, THE Debugger_UI SHALL save the current layout configuration to disk
6. WHEN the Debugger_UI activates, THE Debugger_UI SHALL restore the previous layout configuration from disk
7. WHEN the Debugger_UI saves state, THE Debugger_UI SHALL persist all breakpoints to disk
8. WHEN the Debugger_UI loads state, THE Debugger_UI SHALL restore all breakpoints from disk
9. WHEN the user presses Escape or F12, THE Debugger_UI SHALL close and resume emulation

### Requirement 11: Integration with Existing Debugger

**User Story:** As a developer, I want the graphical debugger to work seamlessly with the existing command-line debugger, so that I can use both interfaces interchangeably.

#### Acceptance Criteria

1. WHEN the Debugger_UI activates, THE Debugger_UI SHALL use the existing Debugger class for breakpoint management
2. WHEN the Debugger_UI adds a breakpoint, THE Debugger SHALL store the breakpoint and check it during execution
3. WHEN the Debugger_UI modifies memory, THE Emulator_Core SHALL update the MemorySystem state
4. WHEN the Debugger_UI steps execution, THE Debugger_UI SHALL call the existing Debugger step method
5. WHEN the command-line debugger adds a breakpoint, THE Debugger_UI SHALL display the breakpoint if active
6. WHEN the Debugger_UI queries CPU state, THE Debugger_UI SHALL use the Emulator_Core get_cpu_state method
7. WHEN the Debugger_UI queries VDC state, THE Debugger_UI SHALL use the Emulator_Core get_vdc_state method
8. WHEN the Debugger_UI queries memory state, THE Debugger_UI SHALL use the Emulator_Core get_memory_state method

### Requirement 12: Dear ImGui Integration

**User Story:** As a developer, I want the debugger UI to use Dear ImGui for rendering, so that I have a responsive and feature-rich debugging interface.

#### Acceptance Criteria

1. WHEN the Debugger_UI initializes, THE Debugger_UI SHALL initialize Dear_ImGui with SDL2 backend
2. WHEN the Debugger_UI renders, THE Debugger_UI SHALL use Dear_ImGui windows for all panels
3. WHEN the user interacts with Dear_ImGui widgets, THE Dear_ImGui SHALL handle input events from SDL2
4. WHEN the Debugger_UI closes, THE Debugger_UI SHALL properly shutdown Dear_ImGui and release resources
5. WHEN the Debugger_UI renders in Overlay_Mode, THE Dear_ImGui SHALL render on top of the emulation framebuffer
6. WHEN the Debugger_UI renders in Split_Screen_Mode, THE Dear_ImGui SHALL render alongside the emulation framebuffer
7. WHEN the user resizes Dear_ImGui windows, THE Dear_ImGui SHALL persist window positions and sizes
8. WHEN the Debugger_UI saves state, THE Debugger_UI SHALL save Dear_ImGui layout configuration using ImGui INI file format
