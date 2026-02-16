# Videopac Debugging Handbook

**Version:** 1.0.0  
**Last Updated:** 2025-01-27

---

## Changelog

### Version 1.0.0 (2025-01-27)
- Initial release of the Videopac Debugging Handbook
- Created core document structure with version tracking
- Established foundation for comprehensive debugging documentation

---

## Navigation Guide

This handbook uses anchor links for easy navigation. Click any section in the Table of Contents to jump directly to that section. Use the "↑ Back to Top" links at the end of each chapter to return to the Table of Contents.

---

## Table of Contents

1. [Introduction and Overview](#1-introduction-and-overview)
   - [1.1 Purpose and Scope](#11-purpose-and-scope)
   - [1.2 How to Use This Handbook](#12-how-to-use-this-handbook)
   - [1.3 Conventions and Terminology](#13-conventions-and-terminology)
   - [1.4 Overview of Debugging Workflow](#14-overview-of-debugging-workflow)

2. [Tools Overview](#2-tools-overview)
   - [2.1 Command-Line Emulator Options](#21-command-line-emulator-options)
   - [2.2 SDL vs Headless Mode Selection](#22-sdl-vs-headless-mode-selection)
   - [2.3 In-Game Debugger Features](#23-in-game-debugger-features)
   - [2.4 Debugger Panels](#24-debugger-panels)
   - [2.5 External Tools Integration](#25-external-tools-integration)
   - [2.6 Tool Selection Guidance](#26-tool-selection-guidance)

3. [Disassembly Workflow](#3-disassembly-workflow)
   - [3.1 Disassembly Command Syntax](#31-disassembly-command-syntax)
   - [3.2 Disassembly Output Format](#32-disassembly-output-format)
   - [3.3 Reading Intel 8048 Assembly](#33-reading-intel-8048-assembly)
   - [3.4 Common Code Patterns](#34-common-code-patterns)
   - [3.5 Function Identification Techniques](#35-function-identification-techniques)
   - [3.6 Annotation Best Practices](#36-annotation-best-practices)
   - [3.7 Examples](#37-examples)

4. [Trace Analysis](#4-trace-analysis)
   - [4.1 Trace Capture Command Syntax](#41-trace-capture-command-syntax)
   - [4.2 Trace Start Conditions](#42-trace-start-conditions)
   - [4.3 Trace Duration Options](#43-trace-duration-options)
   - [4.4 Trace Filtering Options](#44-trace-filtering-options)
   - [4.5 Trace Output Format](#45-trace-output-format)
   - [4.6 Reading Trace Output](#46-reading-trace-output)
   - [4.7 Performance Considerations](#47-performance-considerations)
   - [4.8 Examples](#48-examples)

5. [Graphics Debugging](#5-graphics-debugging)
   - [5.1 Sprite Debugging Workflow](#51-sprite-debugging-workflow)
   - [5.2 Character Debugging Workflow](#52-character-debugging-workflow)
   - [5.3 Grid Debugging Workflow](#53-grid-debugging-workflow)
   - [5.4 Color Palette Debugging Workflow](#54-color-palette-debugging-workflow)
   - [5.5 Collision Detection Debugging Workflow](#55-collision-detection-debugging-workflow)
   - [5.6 Using VDC Register Viewer](#56-using-vdc-register-viewer)
   - [5.7 Using Sprite Visualization](#57-using-sprite-visualization)
   - [5.8 Frame-by-Frame Analysis](#58-frame-by-frame-analysis)
   - [5.9 Examples](#59-examples)

6. [Memory Analysis](#6-memory-analysis)
   - [6.1 Videopac Memory Map](#61-videopac-memory-map)
   - [6.2 Memory Dump Capture](#62-memory-dump-capture)
   - [6.3 Using Memory Viewer](#63-using-memory-viewer)
   - [6.4 Memory Search Techniques](#64-memory-search-techniques)
   - [6.5 Tracking Memory Writes](#65-tracking-memory-writes)
   - [6.6 Data Structure Identification](#66-data-structure-identification)
   - [6.7 Watch Expression Usage](#67-watch-expression-usage)
   - [6.8 Examples](#68-examples)

7. [Timing and Synchronization](#7-timing-and-synchronization)
   - [7.1 Videopac Timing Model](#71-videopac-timing-model)
   - [7.2 Instruction Timing Analysis](#72-instruction-timing-analysis)
   - [7.3 VBLANK Identification](#73-vblank-identification)
   - [7.4 Mid-Frame Update Analysis](#74-mid-frame-update-analysis)
   - [7.5 Timing-Dependent Bug Identification](#75-timing-dependent-bug-identification)
   - [7.6 Using FPS Display and Metrics](#76-using-fps-display-and-metrics)
   - [7.7 Common Timing Issues](#77-common-timing-issues)
   - [7.8 Examples](#78-examples)

8. [Audio Debugging](#8-audio-debugging)
   - [8.1 Videopac Audio System Overview](#81-videopac-audio-system-overview)
   - [8.2 Audio VDC Registers](#82-audio-vdc-registers)
   - [8.3 Tracing Audio Register Writes](#83-tracing-audio-register-writes)
   - [8.4 Audio Waveform Analysis](#84-audio-waveform-analysis)
   - [8.5 Common Audio Issues](#85-common-audio-issues)
   - [8.6 Audio Comparison with Real Hardware](#86-audio-comparison-with-real-hardware)
   - [8.7 Examples](#87-examples)

9. [Input Debugging](#9-input-debugging)
   - [9.1 Videopac Input System Overview](#91-videopac-input-system-overview)
   - [9.2 Input I/O Ports](#92-input-io-ports)
   - [9.3 Tracing Input Port Reads](#93-tracing-input-port-reads)
   - [9.4 Input Mapping Verification](#94-input-mapping-verification)
   - [9.5 Input Responsiveness Testing](#95-input-responsiveness-testing)
   - [9.6 Common Input Issues](#96-common-input-issues)
   - [9.7 Examples](#97-examples)

10. [Comparative Analysis](#10-comparative-analysis)
    - [10.1 Capturing Reference Traces](#101-capturing-reference-traces)
    - [10.2 Comparing Emulator vs Reference Traces](#102-comparing-emulator-vs-reference-traces)
    - [10.3 Identifying Discrepancies](#103-identifying-discrepancies)
    - [10.4 Visual Comparison Techniques](#104-visual-comparison-techniques)
    - [10.5 Audio Comparison Techniques](#105-audio-comparison-techniques)
    - [10.6 Known Emulator vs Hardware Differences](#106-known-emulator-vs-hardware-differences)
    - [10.7 Examples](#107-examples)

11. [Case Studies](#11-case-studies)
    - [11.1 Racing Game Color Bug](#111-racing-game-color-bug)
    - [11.2 Collision Detection Analysis](#112-collision-detection-analysis)
    - [11.3 Additional Case Studies](#113-additional-case-studies)

12. [Quick Reference](#12-quick-reference)
    - [12.1 Disassembly Commands](#121-disassembly-commands)
    - [12.2 Trace Commands](#122-trace-commands)
    - [12.3 Debugger Shortcuts](#123-debugger-shortcuts)
    - [12.4 VDC Registers](#124-vdc-registers)
    - [12.5 CPU Registers](#125-cpu-registers)
    - [12.6 Memory Map](#126-memory-map)
    - [12.7 Intel 8048 Instruction Set](#127-intel-8048-instruction-set)

13. [Appendices](#13-appendices)
    - [13.1 Intel 8048 Architecture](#131-intel-8048-architecture)
    - [13.2 Intel 8245 VDC Architecture](#132-intel-8245-vdc-architecture)
    - [13.3 Workflow Templates](#133-workflow-templates)
    - [13.4 External Resources](#134-external-resources)

---

## 1. Introduction and Overview

### 1.1 Purpose and Scope

This handbook provides comprehensive guidance for debugging the Videopac/Odyssey2 emulator. It covers systematic approaches, tools, and workflows for investigating BIOS, ROM files, and emulator behavior across all games in the Videopac library.

**What This Handbook Covers:**
- Debugging tools and their usage
- Disassembly and trace analysis workflows
- Graphics, audio, and input debugging
- Timing and synchronization analysis
- Real-world case studies and examples

**What This Handbook Does Not Cover:**
- Game-specific strategies or walkthroughs
- Legal advice on ROM usage
- Implementing new emulator features

### 1.2 How to Use This Handbook

This handbook is organized to support both learning and reference use:

**For New Developers:**
1. Start with Chapter 1 (Introduction) and Chapter 2 (Tools Overview)
2. Read Chapter 3 (Disassembly) and Chapter 4 (Trace Analysis) to understand core workflows
3. Explore debugging chapters (5-10) relevant to your issue
4. Study case studies (Chapter 11) to see real-world examples

**For Experienced Developers:**
- Use the Table of Contents to jump to specific topics
- Refer to Quick Reference cards (Chapter 12) for command syntax
- Use workflow templates (Appendix 13.3) as checklists
- Consult case studies for similar issues

**Navigation Tips:**
- Click any section in the Table of Contents to jump directly there
- Use "↑ Back to Top" links to return to the Table of Contents
- Use your browser's search (Ctrl+F) to find specific terms

### 1.3 Conventions and Terminology

**Document Conventions:**

`Code and commands` - Inline code, commands, or file names  
```
Code blocks - Multi-line code examples or command output
```

**Important** - Key information to remember  
**Note** - Additional context or clarification  
**Warning** - Potential pitfalls or issues to avoid

**Terminology:**

- **ROM** - Read-Only Memory file containing game cartridge data
- **BIOS** - Basic Input/Output System file required for emulator initialization
- **Disassembly** - Human-readable Intel 8048 assembly code from binary files
- **Execution Trace** - Log of CPU instructions, registers, and I/O during emulation
- **VDC** - Video Display Controller (Intel 8245)
- **VDC Trace** - Filtered trace showing only VDC register accesses
- **Breakpoint** - Debugging marker that pauses execution at specific conditions
- **Watch Expression** - Monitor for memory addresses or registers

### 1.4 Overview of Debugging Workflow

A typical debugging workflow follows these steps:

1. **Problem Identification**
   - Reproduce the issue consistently
   - Document symptoms (visual, audio, behavioral)
   - Identify affected games or scenarios

2. **Tool Selection**
   - Choose SDL mode for interactive debugging
   - Choose Headless mode for automated trace capture
   - Select appropriate command-line options

3. **Data Collection**
   - Generate disassembly of relevant ROM/BIOS
   - Capture execution traces
   - Take screenshots or recordings
   - Capture memory dumps if needed

4. **Analysis**
   - Examine traces for anomalies
   - Identify relevant code sections in disassembly
   - Check register values and timing
   - Compare with expected behavior

5. **Hypothesis Formation**
   - Formulate theory about root cause
   - Identify specific behavior to test
   - Design verification approach

6. **Testing and Verification**
   - Implement fix or test
   - Verify fix resolves issue
   - Test for regressions in other games
   - Compare with real hardware if possible

7. **Documentation**
   - Document root cause and solution
   - Add case study to handbook
   - Update relevant sections

[↑ Back to Top](#table-of-contents)

---

## 2. Tools Overview

### 2.1 Command-Line Emulator Options

The Videopac emulator supports various command-line options for debugging:

**Basic Usage:**
```bash
videopac [options] <bios_file> <rom_file>
```

**Common Options:**
- `--debugger` - Enable in-game debugger (F12 to toggle)
- `--disassemble-bios <file>` - Disassemble BIOS file
- `--disassemble-rom <file>` - Disassemble ROM file
- `--trace` - Enable execution tracing
- `--trace-output <file>` - Specify trace output file
- `--output <file>` - Specify output file for disassembly

**Example:**
```bash
# Run with debugger enabled
videopac --debugger bios.bin game.bin

# Generate disassembly
videopac --disassemble-rom game.bin --output game_disasm.txt
```

### 2.2 SDL vs Headless Mode Selection

The emulator provides two frontend modes optimized for different debugging scenarios:

**SDL Mode (frontend_sdl):**
- Interactive debugging with visual feedback
- In-game debugger access (F12)
- Real-time graphics and audio
- Best for: Graphics debugging, audio debugging, interactive analysis

**Headless Mode (frontend_headless):**
- No GUI overhead, faster execution
- Automated trace capture
- Batch processing
- Best for: Trace capture, disassembly generation, automated testing

**Selection Guide:**

Use SDL Mode when:
- Using the in-game debugger
- Debugging graphics issues visually
- Testing audio output
- Verifying input responsiveness
- Stepping through code with visual feedback

Use Headless Mode when:
- Capturing execution traces
- Generating disassembly files
- Running automated tests
- Processing multiple ROMs
- Running on servers without display

### 2.3 In-Game Debugger Features

Press F12 during emulation to toggle the in-game debugger. The debugger provides:

**Execution Control:**
- Continue (F5) - Resume execution
- Step (F9) - Execute single instruction
- Step Over (F8) - Skip CALL instructions
- Step Out (Shift+F8) - Return from subroutine

**Breakpoints:**
- Address breakpoints - Pause at specific PC values
- Conditional breakpoints - Pause when conditions met
- Memory write breakpoints - Pause on memory changes
- VDC register breakpoints - Pause on VDC writes

**Watch Expressions:**
- Monitor memory addresses
- Monitor CPU registers
- Alert on value changes

### 2.4 Debugger Panels

The in-game debugger includes several panels:

**CPU State Panel:**
- Current register values (A, PSW, PC, SP, R0-R7)
- Program Status Word flags
- Current instruction at PC
- Call stack

**Memory Viewer:**
- Hex/ASCII memory browser
- Navigate with PgUp/PgDn
- Jump to address (Ctrl+G)
- Search for byte patterns

**Disassembly Panel:**
- Code disassembly around PC
- Breakpoint markers
- Current instruction highlight

**VDC Registers Panel:**
- All VDC register values (0xA0-0xAF)
- Sprite positions and colors
- Grid control settings
- Collision detection status

**Sprite Visualization:**
- Visual representation of all 4 sprites
- Pattern data display
- Position and color information

### 2.5 External Tools Integration

Enhance your debugging workflow with external tools:

**Text Editors:**
- Annotate disassembly files
- Add comments and labels
- Track code analysis notes

**Diff Tools:**
- Compare traces between emulator versions
- Compare emulator vs real hardware traces
- Identify behavioral changes

**Scripting Languages (Python, etc.):**
- Parse and analyze large trace files
- Extract specific patterns
- Generate statistics and reports

**Hex Editors:**
- Inspect ROM file structure
- Verify file integrity
- Examine binary patterns

**Image Comparison Tools:**
- Visual regression testing
- Compare screenshots
- Identify rendering differences

### 2.6 Tool Selection Guidance

Choose the right tool for your debugging task:

| Task | Recommended Tool | Mode |
|------|------------------|------|
| Visual graphics issue | In-game debugger | SDL |
| Audio issue | In-game debugger | SDL |
| Input issue | In-game debugger | SDL |
| Capture execution trace | Command-line trace | Headless |
| Generate disassembly | Command-line disassemble | Headless |
| Analyze VDC timing | Trace + disassembly | Headless |
| Step through code | In-game debugger | SDL |
| Batch test ROMs | Command-line | Headless |

[↑ Back to Top](#table-of-contents)

---

## 3. Disassembly Workflow

### 3.1 Disassembly Command Syntax

**Disassembling BIOS:**
```bash
videopac --disassemble-bios <bios_file> --output <output_file>
```

**Disassembling ROM:**
```bash
videopac --disassemble-rom <rom_file> --output <output_file>
```

**Disassembling Address Range:**
```bash
videopac --disassemble-rom <rom_file> --start 0x0000 --end 0x0FFF --output <output_file>
```

**Options:**
- `--start <addr>` - Start address in hexadecimal
- `--end <addr>` - End address in hexadecimal
- `--output <file>` - Output file path (defaults to stdout)

### 3.2 Disassembly Output Format

Disassembly output follows this format:

```
Address  Opcode    Mnemonic  Operands    ; Comments
-------  --------  --------  ----------  -----------
0x0000   23        MOV       A, #0x23    ; Initialize accumulator
0x0001   F0        MOV       A, @R0      ; Load from memory
0x0002   96 0A     JNZ       0x0A        ; Jump if not zero
0x0004   04 A3     JMP       0x04A3      ; Jump to address
```

**Columns:**
- **Address** - Memory address of instruction
- **Opcode** - Hexadecimal opcode bytes
- **Mnemonic** - Assembly instruction name
- **Operands** - Instruction operands
- **Comments** - Auto-generated or manual annotations

### 3.3 Reading Intel 8048 Assembly

**Common Instructions:**

**Data Movement:**
- `MOV A, #data` - Load immediate value into accumulator
- `MOV A, @Rr` - Load from memory (R0 or R1 as pointer)
- `MOV @Rr, A` - Store accumulator to memory
- `MOV Rr, A` - Move accumulator to register
- `MOV A, Rr` - Move register to accumulator

**Arithmetic:**
- `ADD A, #data` - Add immediate to accumulator
- `ADD A, Rr` - Add register to accumulator
- `ADDC A, #data` - Add with carry
- `INC A` - Increment accumulator
- `DEC A` - Decrement accumulator

**Logic:**
- `ANL A, #data` - AND immediate with accumulator
- `ORL A, #data` - OR immediate with accumulator
- `XRL A, #data` - XOR immediate with accumulator

**Control Flow:**
- `JMP addr` - Unconditional jump
- `DJNZ Rr, addr` - Decrement and jump if not zero
- `JZ addr` - Jump if accumulator is zero
- `JNZ addr` - Jump if accumulator is not zero
- `CALL addr` - Call subroutine
- `RET` - Return from subroutine

**I/O:**
- `OUTL Pp, A` - Output accumulator to port
- `INS A, BUS` - Input from bus to accumulator

### 3.4 Common Code Patterns

**Initialization Sequence:**
```assembly
0x0000   CLR A           ; Clear accumulator
0x0001   MOV R0, #0x00   ; Initialize pointer
0x0003   MOV R7, #0xFF   ; Initialize counter
```

**Main Loop:**
```assembly
0x0100   CALL 0x0200     ; Call update routine
0x0103   CALL 0x0300     ; Call render routine
0x0106   JMP 0x0100      ; Loop forever
```

**VDC Register Update:**
```assembly
0x0200   MOV A, #0x42    ; Load color value
0x0202   OUTL P1, A      ; Output to VDC
0x0203   MOV A, #0xA3    ; VDC register address
0x0205   OUTL P2, A      ; Select register
```

**Conditional Logic:**
```assembly
0x0300   MOV A, R5       ; Load game state
0x0301   JZ 0x0310       ; If zero, jump to handler
0x0303   CALL 0x0400     ; Otherwise, call routine
0x0306   JMP 0x0320      ; Continue
```

### 3.5 Function Identification Techniques

**Identifying Function Boundaries:**

1. **Look for CALL instructions** - Targets are likely function entry points
2. **Look for RET instructions** - Mark function exits
3. **Identify code blocks** - Sequences between CALL/RET pairs
4. **Check for stack operations** - Functions often save/restore registers

**Example:**
```assembly
; Function at 0x0400
0x0400   MOV R3, A       ; Save parameter
0x0401   MOV A, R0       ; Load data
0x0402   ADD A, R3       ; Process
0x0403   MOV R0, A       ; Store result
0x0404   RET             ; Return
```

**Identifying Data vs Code:**

- **Code** - Valid instruction sequences, referenced by jumps/calls
- **Data** - Invalid instructions, accessed by memory operations
- **Tables** - Sequences of similar values, accessed with indexed addressing

### 3.6 Annotation Best Practices

**Add Comments for:**
- Function purposes and parameters
- Register usage conventions
- VDC register writes
- Memory locations
- Magic numbers and constants

**Example Annotated Disassembly:**
```assembly
; UpdateSprite0 - Updates sprite 0 position and color
; Input: R0 = X position, R1 = Y position, R2 = color
0x0500   MOV A, R0       ; Load X position
0x0501   OUTL P1, A      ; Write to VDC
0x0502   MOV A, #0xA0    ; Sprite 0 X register
0x0504   OUTL P2, A      ; Select register
0x0505   MOV A, R1       ; Load Y position
0x0506   OUTL P1, A      ; Write to VDC
0x0507   MOV A, #0xA1    ; Sprite 0 Y register
0x0509   OUTL P2, A      ; Select register
0x050A   RET             ; Return
```

### 3.7 Examples

**Example 1: Disassembling BIOS**
```bash
videopac --disassemble-bios o2rom.bin --output bios_disasm.txt
```

**Example 2: Disassembling ROM**
```bash
videopac --disassemble-rom game.bin --output game_disasm.txt
```

**Example 3: Disassembling Specific Range**
```bash
# Disassemble first 256 bytes
videopac --disassemble-rom game.bin --start 0x0000 --end 0x00FF --output game_init.txt
```

[↑ Back to Top](#table-of-contents)

---

## 4. Trace Analysis

### 4.1 Trace Capture Command Syntax

**Basic Trace:**
```bash
videopac --trace --trace-output trace.txt <bios> <rom>
```

**Trace with Start Condition:**
```bash
# Start at specific frame
videopac --trace --trace-start-frame 100 --trace-output trace.txt <bios> <rom>

# Start on key press
videopac --trace --trace-start-key "1" --trace-output trace.txt <bios> <rom>

# Start at specific address
videopac --trace --trace-start-address 0x0400 --trace-output trace.txt <bios> <rom>
```

**Trace with Duration:**
```bash
# Trace for specific number of frames
videopac --trace --trace-frames 1 --trace-output trace.txt <bios> <rom>

# Trace for specific number of instructions
videopac --trace --trace-instructions 1000 --trace-output trace.txt <bios> <rom>
```

### 4.2 Trace Start Conditions

**Frame Number:**
- `--trace-start-frame <n>` - Start tracing at frame N
- Useful for capturing specific game states
- Frame 0 is the first frame after initialization

**Key Press:**
- `--trace-start-key <key>` - Start tracing when key is pressed
- Keys: "0"-"9", "A"-"Z", "SPACE", "ENTER"
- Useful for capturing player input response

**Address:**
- `--trace-start-address <addr>` - Start when PC reaches address
- Address in hexadecimal (e.g., 0x0400)
- Useful for tracing specific code sections

**Condition:**
- `--trace-start-condition <expr>` - Start when expression is true
- Example: "A == 0x42" or "R0 > 0x10"
- Useful for complex start conditions

### 4.3 Trace Duration Options

**Frame Count:**
- `--trace-frames <n>` - Trace for N frames
- Typical: 1 frame for single-frame analysis
- Use 10-60 for multi-frame sequences

**Instruction Count:**
- `--trace-instructions <n>` - Trace for N instructions
- Typical: 100-1000 for focused analysis
- Use 10000+ for comprehensive traces

**Time Duration:**
- `--trace-time <seconds>` - Trace for specified time
- Useful for real-time debugging scenarios

**Unlimited:**
- No duration option - Trace until program exit
- **Warning:** Can generate very large files

### 4.4 Trace Filtering Options

**Filter Types:**
```bash
# All events (default)
videopac --trace --trace-filter all --trace-output trace.txt <bios> <rom>

# CPU instructions only
videopac --trace --trace-filter cpu --trace-output trace.txt <bios> <rom>

# VDC register writes only
videopac --trace --trace-filter vdc --trace-output trace.txt <bios> <rom>

# Memory writes only
videopac --trace --trace-filter mem-write --trace-output trace.txt <bios> <rom>

# Memory reads only
videopac --trace --trace-filter mem-read --trace-output trace.txt <bios> <rom>

# I/O port accesses only
videopac --trace --trace-filter io --trace-output trace.txt <bios> <rom>
```

**Address Range Filtering:**
```bash
# Trace only specific address range
videopac --trace --trace-address-range 0x0400:0x0FFF --trace-output trace.txt <bios> <rom>
```

### 4.5 Trace Output Format

**Standard Format:**
```
Frame:0001 Scanline:010 Cycle:00123 PC:0x0234 A:0x42 PSW:0x08 | MOV A, #0x42
Frame:0001 Scanline:010 Cycle:00125 PC:0x0235 A:0x42 PSW:0x08 | OUTL P1, A
Frame:0001 Scanline:010 Cycle:00127 PC:0x0236 A:0x42 PSW:0x08 | VDC_WRITE[0xA3] = 0x42
```

**Columns:**
- **Frame** - Current video frame number
- **Scanline** - Current scanline (0-261)
- **Cycle** - CPU cycle within frame
- **PC** - Program Counter
- **A** - Accumulator value
- **PSW** - Program Status Word
- **Instruction** - Disassembled instruction or event

**VDC Write Format:**
```
Frame:0001 Scanline:010 Cycle:00127 | VDC_WRITE[0xA3] = 0x42 (Sprite 0 Color)
```

**Memory Access Format:**
```
Frame:0001 Scanline:010 Cycle:00130 | MEM_WRITE[0x1000] = 0x55
Frame:0001 Scanline:010 Cycle:00132 | MEM_READ[0x1000] = 0x55
```

### 4.6 Reading Trace Output

**Analyzing Execution Flow:**
1. Follow PC values to track code execution
2. Identify loops (PC returns to previous values)
3. Identify function calls (CALL instructions)
4. Identify returns (RET instructions)

**Analyzing Register Changes:**
1. Track accumulator (A) value changes
2. Identify data sources (immediate values, memory, registers)
3. Track PSW flag changes (carry, zero, etc.)

**Analyzing Timing:**
1. Check scanline values for VBLANK (scanline > 192)
2. Identify mid-frame updates (scanline 0-192)
3. Calculate instruction timing from cycle counts
4. Identify timing-critical sections

**Analyzing VDC Writes:**
1. Filter for VDC_WRITE events
2. Check register addresses (0xA0-0xAF)
3. Verify write timing (VBLANK vs mid-frame)
4. Track register value changes over time

### 4.7 Performance Considerations

**Trace File Size:**
- 1 frame ≈ 50,000-100,000 instructions
- 1 frame trace ≈ 5-10 MB uncompressed
- 60 frames ≈ 300-600 MB uncompressed

**Recommendations:**
- Use filtering to reduce file size
- Trace only necessary frames
- Use address range filtering for focused analysis
- Compress trace files after capture (gzip, zip)

**Memory Usage:**
- Traces are written to disk incrementally
- Memory usage is minimal during capture
- Large traces may take time to load in editors

### 4.8 Examples

**Example 1: Capture First Frame After Key Press**
```bash
videopac --trace --trace-start-key "1" --trace-frames 1 --trace-output first_frame.txt bios.bin game.bin
```

**Example 2: Capture VDC Writes Only**
```bash
videopac --trace --trace-filter vdc --trace-frames 10 --trace-output vdc_trace.txt bios.bin game.bin
```

**Example 3: Capture Specific Code Section**
```bash
videopac --trace --trace-start-address 0x0400 --trace-instructions 1000 --trace-output code_trace.txt bios.bin game.bin
```

**Example 4: Capture Memory Writes**
```bash
videopac --trace --trace-filter mem-write --trace-frames 1 --trace-output mem_trace.txt bios.bin game.bin
```

[↑ Back to Top](#table-of-contents)

---

## 5. Graphics Debugging

### 5.1 Sprite Debugging Workflow

**Problem:** Sprite appears incorrect (position, color, pattern, or invisible)

**Steps:**
1. **Verify in Debugger (F12)**
   - Open VDC Register Viewer
   - Check sprite registers (0xA0-0xAF for sprite 0-3)
   - Verify position, color, and pattern values

2. **Capture Trace**
   - Trace the frame where issue occurs
   - Filter for VDC writes: `--trace-filter vdc`
   - Look for sprite register writes

3. **Analyze Trace**
   - Check sprite position registers (X, Y)
   - Check sprite color register
   - Check sprite pattern register
   - Verify write timing (VBLANK vs mid-frame)

4. **Check Disassembly**
   - Locate code that writes sprite registers
   - Trace data sources for register values
   - Verify logic for sprite updates

5. **Common Issues**
   - Sprite off-screen (X or Y out of bounds)
   - Sprite color matches background
   - Sprite pattern is blank (all zeros)
   - Sprite disabled in control register

### 5.2 Character Debugging Workflow

**Problem:** Character display is incorrect

**Steps:**
1. **Verify Character Data**
   - Check character pattern memory
   - Verify character grid positions
   - Check character color settings

2. **Capture Trace**
   - Trace frame with character issue
   - Filter for VDC writes to character registers
   - Look for character memory writes

3. **Analyze Character Rendering**
   - Check character grid control register
   - Verify character pattern data
   - Check character color register

4. **Common Issues**
   - Character patterns not loaded
   - Character grid disabled
   - Character color matches background

### 5.3 Grid Debugging Workflow

**Problem:** Grid display is incorrect

**Steps:**
1. **Check Grid Control Register (0xAF)**
   - Verify grid is enabled
   - Check grid position settings
   - Verify grid color

2. **Capture Trace**
   - Filter for grid register writes
   - Check timing of grid updates

3. **Analyze Grid Rendering**
   - Verify grid pattern data
   - Check grid position registers
   - Verify grid color register

### 5.4 Color Palette Debugging Workflow

**Problem:** Colors appear incorrect

**Steps:**
1. **Verify Color Values**
   - Check color registers in VDC viewer
   - Verify RGBI color encoding
   - Compare with expected colors

2. **Capture Trace**
   - Filter for color register writes (0xA3, 0xAB-0xAE)
   - Check when colors are set
   - Look for dynamic color changes

3. **Analyze Color Logic**
   - Locate color-setting code in disassembly
   - Verify color calculation logic
   - Check for palette cycling or effects

4. **Common Issues**
   - Color register values incorrect
   - Color changes at wrong time
   - RGBI encoding issues

### 5.5 Collision Detection Debugging Workflow

**Problem:** Collision detection not working correctly

**Steps:**
1. **Check Collision Register (0xA2)**
   - Read collision register in debugger
   - Verify collision bits are set when sprites overlap
   - Check collision clear timing

2. **Capture Trace**
   - Trace frame with collision event
   - Filter for collision register reads
   - Check when game reads collision status

3. **Analyze Collision Logic**
   - Locate collision-checking code
   - Verify game reads collision register
   - Check collision response logic

4. **Common Issues**
   - Game uses software collision (doesn't read register)
   - Collision register not cleared properly
   - Collision detection timing issues

### 5.6 Using VDC Register Viewer

The VDC Register Viewer is a powerful real-time debugging tool that displays all Video Display Controller register values and their interpretations. It's essential for graphics debugging as it provides immediate visibility into the VDC state without requiring trace capture.

#### Accessing the VDC Register Viewer

1. **Launch emulator with debugger enabled:**
   ```bash
   videopac --debugger bios.bin game.bin
   ```

2. **Toggle debugger:** Press F12 during emulation

3. **Navigate to VDC Registers panel:** Use arrow keys or click on the VDC Registers tab

#### Information Displayed

The VDC Register Viewer shows all 16 VDC registers (0xA0-0xAF) with both raw hexadecimal values and human-readable interpretations:

**Control and Status Registers:**
- **0xA0 (Control)** - VDC control flags, enable/disable settings
- **0xA1 (Status)** - VDC status flags (read-only), VBLANK indicator
- **0xA2 (Collision)** - Collision detection bits for sprite-to-sprite and sprite-to-background collisions

**Color and Position Registers:**
- **0xA3 (Background Color)** - Background and grid color (RGBI format)
- **0xA4 (Grid Y Position)** - Vertical grid position
- **0xA5 (Grid X Position)** - Horizontal grid position

**Audio Registers:**
- **0xA7 (Sound Byte 0)** - Sound shift register byte 0
- **0xA8 (Sound Byte 1)** - Sound shift register byte 1
- **0xA9 (Sound Byte 2)** - Sound shift register byte 2
- **0xAA (Sound Control)** - Sound control and sprite 3 color

**Sprite Color Registers:**
- **0xAB (Sprite 0 Color)** - Color for sprite 0 (RGBI format)
- **0xAC (Sprite 1 Color)** - Color for sprite 1 (RGBI format)
- **0xAD (Sprite 2 Color)** - Color for sprite 2 (RGBI format)
- **0xAE (Sprite 3 Color)** - Color for sprite 3 (RGBI format)

**Grid Control:**
- **0xAF (Grid Control)** - Grid enable, pattern selection, and control flags

#### Interpreting Register Values

**RGBI Color Format:**
Colors are encoded in 4-bit RGBI format (Red, Green, Blue, Intensity):
- Bit 3: Intensity (0=dark, 1=bright)
- Bit 2: Red component
- Bit 1: Green component
- Bit 0: Blue component

Example: `0x7` = 0111 binary = Bright Cyan (I=0, R=1, G=1, B=1)

**Collision Register (0xA2):**
Each bit indicates a specific collision type:
- Bit 7: Sprite 3 collision
- Bit 6: Sprite 2 collision
- Bit 5: Sprite 1 collision
- Bit 4: Sprite 0 collision
- Bits 3-0: Background collision flags

**Grid Control Register (0xAF):**
- Bit 7: Grid enable (1=enabled, 0=disabled)
- Bits 6-0: Grid pattern and control settings

#### How to Use the VDC Register Viewer

**1. Real-Time Monitoring:**
- Watch register values update as the game runs
- Pause execution (F9) to freeze values for inspection
- Step through code (F9) to see register changes per instruction
- Identify which registers change during specific game events

**2. Verifying Expected Values:**
- Check sprite colors match your expectations
- Verify sprite positions are within screen bounds (0-255 for X, 0-191 for Y)
- Confirm background color is set correctly
- Validate collision detection flags are set when sprites overlap

**3. Identifying Issues:**
- **Sprite not visible?** Check if color matches background (0xAB-0xAE vs 0xA3)
- **Wrong colors?** Verify RGBI encoding in color registers
- **Collision not working?** Check if collision register (0xA2) bits are set
- **Grid not showing?** Check grid enable bit in 0xAF

**4. Comparing with Trace Data:**
- Capture a trace with VDC filter: `--trace-filter vdc`
- Compare register values in trace with values in viewer
- Verify timing of register writes (VBLANK vs mid-frame)
- Identify discrepancies between expected and actual values

**5. Debugging Workflows:**

**Sprite Color Issue:**
1. Open VDC Register Viewer
2. Locate sprite color register (0xAB-0xAE)
3. Note the current value (e.g., 0x1 = dark blue)
4. Check if it matches expected color
5. If wrong, capture trace to find where color is set
6. Analyze disassembly to understand color logic

**Sprite Position Issue:**
1. Check sprite position in Sprite Visualization panel
2. Note X/Y coordinates
3. Verify coordinates are within valid range
4. If off-screen, check position calculation logic
5. Use trace to track position register writes

**Collision Detection Issue:**
1. Monitor collision register (0xA2) in real-time
2. Move sprites to overlap
3. Check if collision bits are set
4. If not set, verify sprite overlap is sufficient
5. If set but game doesn't respond, check if game reads register

#### Practical Examples

**Example 1: Debugging Sprite Color**
```
Problem: Sprite 0 appears blue instead of red

Steps:
1. Press F12 to open debugger
2. Navigate to VDC Registers panel
3. Check register 0xAB (Sprite 0 Color)
4. Value shows: 0x1 (dark blue)
5. Expected: 0x2 (dark red)
6. Capture trace: videopac --trace --trace-filter vdc --trace-frames 1 --trace-output color.txt bios.bin game.bin
7. Search trace for writes to 0xAB
8. Analyze code that sets sprite 0 color
```

**Example 2: Verifying Collision Detection**
```
Problem: Collision detection not triggering

Steps:
1. Open VDC Register Viewer
2. Monitor register 0xA2 (Collision)
3. Move sprites to overlap
4. Check if collision bits are set (e.g., bit 4 for sprite 0)
5. If bits are set: Game may not be reading collision register
6. If bits not set: Sprites may not be overlapping sufficiently
7. Use Sprite Visualization to verify sprite positions
```

**Example 3: Grid Not Displaying**
```
Problem: Grid is not visible on screen

Steps:
1. Check register 0xAF (Grid Control)
2. Verify bit 7 is set (grid enabled)
3. If bit 7 is 0: Grid is disabled, find code that should enable it
4. If bit 7 is 1: Check grid color (0xA3) doesn't match background
5. Check grid position registers (0xA4, 0xA5)
6. Verify grid pattern data is loaded
```

#### Tips and Best Practices

**Tip 1: Use Pause and Step**
- Pause execution (F9) to freeze register values
- Step through code to see exactly when registers change
- This is more precise than watching values change in real-time

**Tip 2: Cross-Reference with Sprite Visualization**
- VDC Register Viewer shows raw values
- Sprite Visualization shows visual representation
- Use both together for complete picture

**Tip 3: Document Register States**
- Take screenshots of register values at key moments
- Note register values in your debugging notes
- Compare register states across different game states

**Tip 4: Watch for Unexpected Changes**
- If a register changes unexpectedly, set a breakpoint
- Use memory write breakpoint on VDC register address
- Trace back to find what code is modifying the register

**Tip 5: Understand VBLANK Timing**
- Most games update VDC registers during VBLANK
- Check status register (0xA1) for VBLANK indicator
- Mid-frame updates may cause visual artifacts

#### Common Pitfalls

**Pitfall 1: Misinterpreting RGBI Colors**
- Remember: Bit 3 is Intensity, not Alpha
- Dark colors have I=0, bright colors have I=1
- 0x0 is black, 0x8 is dark gray, 0xF is white

**Pitfall 2: Ignoring Collision Register Timing**
- Collision register is updated by hardware
- Game must read it at the right time
- Collision bits may be cleared after reading

**Pitfall 3: Assuming Static Values**
- Register values can change every frame
- Some games update registers mid-frame
- Always verify timing with traces

**Pitfall 4: Overlooking Register Interactions**
- Some registers affect others (e.g., 0xAA contains sprite 3 color)
- Grid control affects multiple rendering aspects
- Control register can disable entire VDC output

#### Integration with Other Tools

**With Trace Analysis:**
1. Use VDC Register Viewer to identify suspicious values
2. Capture trace with `--trace-filter vdc`
3. Search trace for writes to suspicious registers
4. Analyze timing and sequence of writes

**With Disassembly:**
1. Note register address with wrong value
2. Search disassembly for OUTL instructions to that address
3. Analyze code logic that sets the register
4. Verify data sources and calculations

**With Memory Viewer:**
1. Identify memory addresses that feed VDC registers
2. Use Memory Viewer to inspect those addresses
3. Verify data is correct before being written to VDC
4. Track data flow from memory to VDC

#### Summary

The VDC Register Viewer is your first stop for graphics debugging. It provides immediate visibility into the VDC state without requiring trace capture or code analysis. Use it to:
- Quickly verify register values
- Monitor real-time changes
- Identify incorrect settings
- Guide further investigation with traces and disassembly

Master the VDC Register Viewer, and you'll solve graphics issues faster and more efficiently.

### 5.7 Using Sprite Visualization

The Sprite Visualization feature provides a visual representation of all four sprites, making it easy to debug sprite-related issues without needing to analyze raw register values or pattern bytes.

#### Accessing Sprite Visualization

1. **Launch emulator with debugger enabled:**
   ```bash
   videopac --debugger bios.bin game.bin
   ```

2. **Toggle debugger:** Press F12 during emulation

3. **Navigate to VDC Registers panel:** The sprite visualization is part of the VDC Registers panel

4. **Expand Sprites section:** Click on "Sprites" to expand the sprite tree view

5. **View individual sprites:** Click on "Sprite 0", "Sprite 1", "Sprite 2", or "Sprite 3" to see detailed information

#### Information Displayed

For each of the 4 sprites (0-3), the visualization shows:

**Position Information:**
- **X Position** - Horizontal position (0-255)
- **Y Position** - Vertical position (0-255)

**Color Information:**
- **Color Index** - Color palette index (0-7 for sprites)
- **Visual Color** - The actual color displayed using the Videopac RGBI palette

**Size and Shift Information:**
- **Size** - Either "8x8" (normal) or "16x16" (double size)
- **Shift** - Horizontal shift mode: "None", "Even", or "Full"
  - None: No horizontal shift
  - Even: Shift even rows right by 1 pixel
  - Full: Shift all rows right by 1 pixel

**Pattern Visualization:**
- **8x8 Pixel Grid** - Visual representation of the sprite pattern
  - Each pixel is drawn as a colored square
  - Active pixels (bit=1) are shown in the sprite's color
  - Inactive pixels (bit=0) are shown in dark gray
  - Grid lines separate individual pixels for clarity

**Pattern Bytes:**
- **8 Hex Values** - Raw pattern data for each of the 8 rows
- Format: "Row 0: 0x00" through "Row 7: 0x00"

#### How to Interpret the Visual Representation

**Understanding the Pattern Grid:**

The 8x8 pixel grid shows exactly how the sprite will appear on screen:
- **Top row** = Pattern byte 0 (bits 7-0, left to right)
- **Bottom row** = Pattern byte 7 (bits 7-0, left to right)
- **Colored pixels** = Bits set to 1 in the pattern byte
- **Dark pixels** = Bits set to 0 in the pattern byte

**Example Pattern Interpretation:**
```
Pattern bytes:
  Row 0: 0x18  (00011000 binary) → Two pixels in center
  Row 1: 0x3C  (00111100 binary) → Four pixels in center
  Row 2: 0x7E  (01111110 binary) → Six pixels in center
  Row 3: 0xFF  (11111111 binary) → All eight pixels
  Row 4: 0xFF  (11111111 binary) → All eight pixels
  Row 5: 0x7E  (01111110 binary) → Six pixels in center
  Row 6: 0x3C  (00111100 binary) → Four pixels in center
  Row 7: 0x18  (00011000 binary) → Two pixels in center

This creates a diamond/circle shape.
```

**Understanding Sprite Colors:**

The Videopac uses a 16-color RGBI palette, but sprites use colors 0-7:
- **0** - Black
- **1** - Dark Blue
- **2** - Dark Green
- **3** - Light Blue
- **4** - Dark Red
- **5** - Violet
- **6** - Orange/Gold
- **7** - Grey

The visualization displays the sprite pattern in the actual color it will appear on screen.

**Understanding Size and Shift:**

**Size:**
- **8x8** - Normal sprite size (8 pixels wide, 8 pixels tall)
- **16x16** - Double size (each pixel is drawn 2x2, making sprite 16x16)

**Shift:**
- **None** - Sprite pattern is displayed as-is
- **Even** - Even-numbered rows (0, 2, 4, 6) are shifted right by 1 pixel
- **Full** - All rows are shifted right by 1 pixel

Note: The visualization shows the base 8x8 pattern. Size and shift effects are applied during rendering on screen.

#### How to Use Sprite Visualization for Debugging

**1. Verifying Sprite Patterns**

**Problem:** Sprite doesn't look right on screen

**Steps:**
1. Open sprite visualization for the affected sprite
2. Check the 8x8 pattern grid
3. Verify the pattern matches your expectations
4. Check pattern bytes to confirm bit patterns

**Common Issues:**
- Pattern is all zeros (sprite is blank)
- Pattern is inverted (bits are flipped)
- Pattern is corrupted (random bits set)
- Wrong pattern loaded (different sprite data)

**Example:**
```
Expected: Player ship sprite
Actual: All dark pixels (pattern bytes all 0x00)
Diagnosis: Pattern data not loaded or cleared
```

**2. Verifying Sprite Colors**

**Problem:** Sprite appears in wrong color

**Steps:**
1. Open sprite visualization
2. Check the "Color" value (0-7)
3. Verify the visual color matches expectations
4. Compare with other sprites if needed

**Common Issues:**
- Color index is wrong (e.g., 1 instead of 4)
- Color matches background (sprite invisible)
- Color is black (sprite appears as silhouette)

**Example:**
```
Expected: Red player ship (color 4)
Actual: Blue player ship (color 1)
Diagnosis: Color register set to wrong value
```

**3. Verifying Sprite Positions**

**Problem:** Sprite is not visible or in wrong location

**Steps:**
1. Check X and Y position values
2. Verify positions are within screen bounds:
   - X: 0-255 (visible area approximately 8-248)
   - Y: 0-255 (visible area approximately 24-216)
3. Check if sprite is off-screen

**Common Issues:**
- X or Y is 0 (sprite at edge or off-screen)
- X or Y is 255 (sprite at opposite edge or off-screen)
- Position values are swapped
- Position calculation is incorrect

**Example:**
```
Expected: Sprite at center of screen (X=128, Y=120)
Actual: X=0, Y=0
Diagnosis: Position registers not initialized
```

**4. Debugging Sprite Visibility Issues**

**Problem:** Sprite should be visible but isn't

**Checklist:**
- [ ] Pattern is not all zeros (check pattern grid)
- [ ] Color doesn't match background (check color value)
- [ ] Position is within visible area (check X/Y values)
- [ ] Display is enabled (check Control Register 0xA0)
- [ ] Sprite is not behind background elements

**5. Debugging Sprite Animation**

**Problem:** Sprite animation not working correctly

**Steps:**
1. Pause emulation (F9)
2. Check current sprite pattern
3. Step through frames (F5 to continue, F9 to pause)
4. Observe pattern changes in visualization
5. Verify pattern updates match expected animation sequence

**6. Comparing Multiple Sprites**

**Problem:** Need to verify sprite relationships (overlap, colors, patterns)

**Steps:**
1. Expand all four sprite nodes
2. Compare positions to identify overlaps
3. Compare colors to verify contrast
4. Compare patterns to verify uniqueness
5. Check for duplicate sprites or missing sprites

#### Practical Examples

**Example 1: Debugging Blank Sprite**

```
Problem: Player sprite is invisible

Steps:
1. Press F12, expand "Sprites" → "Sprite 0"
2. Check pattern visualization
3. Observation: All pixels are dark gray
4. Check pattern bytes: All 0x00
5. Diagnosis: Pattern data not loaded
6. Solution: Find code that loads sprite patterns, verify it runs
```

**Example 2: Debugging Wrong Sprite Color**

```
Problem: Enemy sprite appears blue instead of red

Steps:
1. Press F12, expand "Sprites" → "Sprite 2"
2. Check "Color" value: Shows "1" (Dark Blue)
3. Expected: "4" (Dark Red)
4. Capture trace: videopac --trace --trace-filter vdc --trace-frames 1 --trace-output color.txt bios.bin game.bin
5. Search trace for writes to sprite 2 color register (0xA8 + 2 = 0xAA)
6. Analyze code that sets sprite color
7. Solution: Fix color value in code
```

**Example 3: Debugging Sprite Position**

```
Problem: Sprite appears at wrong location

Steps:
1. Press F12, expand "Sprites" → "Sprite 1"
2. Check position: X=200, Y=50
3. Expected: X=100, Y=100
4. Observation: X is off by 100 pixels
5. Check disassembly for position calculation code
6. Diagnosis: X position calculation has offset error
7. Solution: Fix position calculation
```

**Example 4: Debugging Sprite Animation**

```
Problem: Sprite animation is stuck on first frame

Steps:
1. Press F12, expand "Sprites" → "Sprite 0"
2. Note current pattern (e.g., frame 1 of walk cycle)
3. Press F5 to continue, wait 1 second, press F9 to pause
4. Check pattern again: Still shows frame 1
5. Diagnosis: Pattern data not being updated
6. Capture trace to find pattern update code
7. Solution: Fix animation update logic
```

**Example 5: Verifying Sprite Overlap**

```
Problem: Collision detection not working

Steps:
1. Press F12, expand all sprites
2. Check Sprite 0: X=100, Y=100
3. Check Sprite 1: X=105, Y=102
4. Observation: Sprites are close but may not overlap
5. Check sprite sizes: Both 8x8
6. Calculate overlap: X overlap = 3 pixels, Y overlap = 6 pixels
7. Verify collision register (0xA2) shows collision bits
8. If collision bits not set: Sprites may not overlap enough
9. If collision bits set but game doesn't respond: Game not reading collision register
```

#### Integration with Other Debugging Tools

**With VDC Register Viewer:**
- Sprite Visualization shows visual representation
- VDC Register Viewer shows raw register values
- Use both together for complete picture

**With Trace Analysis:**
1. Use Sprite Visualization to identify issue (e.g., wrong color)
2. Note the sprite number and register address
3. Capture trace with VDC filter
4. Search trace for writes to that register
5. Analyze code that sets the register

**With Disassembly:**
1. Identify issue in Sprite Visualization
2. Determine which register is wrong
3. Search disassembly for OUTL instructions to that register
4. Analyze code logic and data sources

**With Memory Viewer:**
1. Note pattern base address (0x80 + sprite_num * 8)
2. Use Memory Viewer to inspect pattern memory
3. Verify pattern data before it's written to VDC
4. Track data flow from ROM/RAM to VDC

#### Tips and Best Practices

**Tip 1: Use Pause and Step for Animation**
- Pause execution to freeze sprite state
- Step through frames to see pattern changes
- This is essential for debugging animation issues

**Tip 2: Check Pattern Bytes for Bit Patterns**
- Visual grid shows overall shape
- Pattern bytes show exact bit values
- Use both to verify pattern data accuracy

**Tip 3: Verify Color Contrast**
- Check sprite color against background color
- Ensure sufficient contrast for visibility
- Remember: Color 0 (black) may be invisible on black background

**Tip 4: Watch for Off-Screen Sprites**
- X/Y values of 0 or 255 often indicate off-screen sprites
- Visible area is approximately X: 8-248, Y: 24-216
- Sprites partially off-screen may appear clipped

**Tip 5: Compare with Expected Patterns**
- Keep reference images of expected sprite patterns
- Compare visualization with reference
- Identify specific pixels that are wrong

**Tip 6: Use Shift and Size Information**
- Shift modes affect horizontal positioning
- Double size affects collision detection area
- Verify these settings match game requirements

#### Common Pitfalls

**Pitfall 1: Confusing Pattern Bytes with Screen Pixels**
- Pattern bytes are in VDC registers (0x80-0x9F)
- Screen pixels are rendered by VDC hardware
- Visualization shows pattern data, not screen output

**Pitfall 2: Ignoring Sprite Priority**
- Sprite 0 has highest priority (drawn on top)
- Sprite 3 has lowest priority (drawn on bottom)
- Overlapping sprites may hide each other

**Pitfall 3: Assuming Static Patterns**
- Patterns can change every frame for animation
- Always verify timing with traces
- Pause execution to freeze pattern state

**Pitfall 4: Overlooking Size and Shift**
- Size and shift affect final rendering
- Visualization shows base 8x8 pattern only
- Verify these settings match expectations

**Pitfall 5: Not Checking All Sprites**
- Issue may be with a different sprite than expected
- Always check all 4 sprites
- Verify sprite assignments match game logic

#### Summary

The Sprite Visualization feature is an essential tool for graphics debugging. It provides immediate visual feedback on sprite patterns, colors, positions, and properties without requiring trace analysis or register interpretation. Use it as your first step when debugging sprite-related issues, then follow up with traces and disassembly for deeper investigation.

**Key Benefits:**
- Visual representation of sprite patterns
- Real-time color and position information
- Easy identification of blank or corrupted patterns
- Quick verification of sprite properties
- Foundation for deeper debugging with traces

Master the Sprite Visualization tool, and you'll solve sprite issues faster and more efficiently.

### 5.8 Frame-by-Frame Analysis

Frame-by-frame analysis is a powerful technique for debugging graphics issues that change over time, such as animation problems, flickering, timing-dependent rendering bugs, and sprite movement issues. By capturing and comparing multiple consecutive frames, you can identify patterns, track state changes, and pinpoint the exact frame where a problem occurs.

#### 5.8.1 When to Use Frame-by-Frame Analysis

Frame-by-frame analysis is most effective for:

- **Animation Issues:** Sprites or characters not animating correctly, skipping frames, or animating at wrong speed
- **Flickering Problems:** Graphics elements appearing and disappearing unexpectedly
- **Movement Bugs:** Sprites moving incorrectly, teleporting, or having erratic motion
- **Timing-Dependent Rendering:** Issues that only appear after certain number of frames or at specific times
- **State Transition Problems:** Graphics not updating correctly when game state changes
- **Collision Detection Issues:** Collisions not detected or detected incorrectly during movement
- **Color Cycling Problems:** Palette changes not occurring at expected times

#### 5.8.2 Capturing Frames for Analysis

**Basic Frame Capture:**

```bash
# Capture 10 consecutive frames starting at frame 100
videopac --trace --trace-start-frame 100 --trace-frames 10 --trace-output frames_100-110.txt bios.bin game.bin

# Capture frames immediately after key press
videopac --trace --trace-start-key "1" --trace-frames 5 --trace-output frames_after_key.txt bios.bin game.bin

# Capture VDC writes only for faster analysis
videopac --trace --trace-start-frame 50 --trace-frames 20 --trace-filter vdc --trace-output vdc_frames.txt bios.bin game.bin
```

**Capturing Specific Frame Ranges:**

For animation analysis, capture enough frames to see a complete animation cycle:

```bash
# Capture 60 frames (approximately 1 second at 60 FPS)
videopac --trace --trace-start-frame 0 --trace-frames 60 --trace-filter vdc --trace-output animation_cycle.txt bios.bin game.bin
```

For flickering issues, capture the frames where flickering occurs:

```bash
# Capture frames around the problem area
videopac --trace --trace-start-frame 200 --trace-frames 30 --trace-filter vdc --trace-output flicker_frames.txt bios.bin game.bin
```

#### 5.8.3 Comparing Frames

**Manual Comparison:**

Split the trace file into individual frame sections and compare them:

```bash
# Extract frame 100
grep "Frame:0100" frames_100-110.txt > frame_100.txt

# Extract frame 101
grep "Frame:0101" frames_100-110.txt > frame_101.txt

# Compare the two frames
diff frame_100.txt frame_101.txt
```

**Automated Comparison Script:**

Create a simple script to extract and compare VDC register values across frames:

```python
# frame_compare.py
import sys

def extract_vdc_writes(trace_file, frame_num):
    """Extract VDC register writes for a specific frame."""
    vdc_writes = {}
    with open(trace_file, 'r') as f:
        for line in f:
            if f"Frame:{frame_num:04d}" in line and "VDC_WRITE" in line:
                # Parse: VDC_WRITE[0xA3] = 0x42
                parts = line.split("VDC_WRITE[")
                if len(parts) > 1:
                    reg_val = parts[1].split("]")
                    register = reg_val[0]
                    value = reg_val[1].split("=")[1].strip()
                    vdc_writes[register] = value
    return vdc_writes

def compare_frames(trace_file, frame1, frame2):
    """Compare VDC register writes between two frames."""
    writes1 = extract_vdc_writes(trace_file, frame1)
    writes2 = extract_vdc_writes(trace_file, frame2)
    
    print(f"Comparing Frame {frame1} vs Frame {frame2}")
    print("-" * 50)
    
    all_regs = set(writes1.keys()) | set(writes2.keys())
    for reg in sorted(all_regs):
        val1 = writes1.get(reg, "not written")
        val2 = writes2.get(reg, "not written")
        if val1 != val2:
            print(f"{reg}: {val1} -> {val2}")

if __name__ == "__main__":
    compare_frames(sys.argv[1], int(sys.argv[2]), int(sys.argv[3]))
```

Usage:
```bash
python frame_compare.py frames_100-110.txt 100 101
```

#### 5.8.4 Identifying Frame-Specific Issues

**Sprite Position Changes:**

Track sprite X/Y position registers (part of sprite data) across frames:

```bash
# Extract sprite position writes
grep "VDC_WRITE.*Sprite.*Position" frames.txt

# Look for unexpected jumps or missing updates
```

**Color Changes:**

Track color register writes (0xA3, 0xAB-0xAE) to identify palette issues:

```bash
# Extract color register writes
grep "VDC_WRITE\[0xA3\]\|VDC_WRITE\[0xAB\]\|VDC_WRITE\[0xAC\]\|VDC_WRITE\[0xAD\]\|VDC_WRITE\[0xAE\]" frames.txt
```

**Pattern Changes:**

Track sprite pattern updates to verify animation sequences:

```bash
# Extract pattern register writes
grep "VDC_WRITE.*Pattern" frames.txt
```

#### 5.8.5 Analyzing Animation Sequences

**Step 1: Identify Animation Cycle Length**

Count how many frames it takes for a sprite to return to its initial state:

```bash
# Extract sprite 0 pattern writes
grep "Sprite 0 Pattern" frames.txt | head -20

# Look for repeating pattern values
```

**Step 2: Verify Animation Timing**

Check if animation updates occur at expected intervals:

```bash
# Count frames between pattern changes
grep "Sprite 0 Pattern" frames.txt | awk '{print $1}' | uniq -c
```

**Step 3: Identify Missing or Duplicate Frames**

Look for frames where expected updates don't occur or occur multiple times:

```bash
# Check for frames with no VDC writes (suspicious)
for i in {100..110}; do
    count=$(grep -c "Frame:$(printf '%04d' $i)" frames.txt)
    echo "Frame $i: $count writes"
done
```

#### 5.8.6 Debugging Flickering Issues

Flickering often occurs when sprites or graphics elements are enabled/disabled rapidly or when register values alternate between states.

**Identify Flickering Pattern:**

```bash
# Track sprite enable/disable across frames
grep "VDC_WRITE\[0xA0\]" frames.txt  # Control register

# Look for alternating values
```

**Check for Mid-Frame Updates:**

Flickering can be caused by VDC register updates occurring at wrong times:

```bash
# Check scanline timing of VDC writes
grep "Scanline" frames.txt | grep "VDC_WRITE"

# Look for writes outside VBLANK period (scanlines 0-20 typically)
```

**Verify Sprite Visibility:**

Check if sprite positions are moving on/off screen:

```bash
# Extract sprite Y positions
grep "Sprite.*Y Position" frames.txt

# Look for values > 240 (off-screen) or rapid changes
```

#### 5.8.7 Interactive Frame-by-Frame Debugging

For visual inspection, use the in-game debugger with manual frame stepping:

**Method 1: Pause and Step**

1. Run emulator with debugger: `videopac --debugger bios.bin game.bin`
2. Press F12 to open debugger
3. Press F9 to step through instructions
4. Watch VDC Register Viewer for changes
5. Note the frame number when issue occurs

**Method 2: Breakpoint on Frame**

1. Set breakpoint at VBLANK start (if known address)
2. Continue execution (F5)
3. Inspect VDC registers each frame
4. Compare values across frames

**Method 3: Screenshot Comparison**

1. Capture screenshots of consecutive frames
2. Use image diff tool to identify pixel differences
3. Correlate visual changes with VDC register changes

#### 5.8.8 Common Frame-by-Frame Patterns

**Pattern 1: Sprite Animation**

Typical sprite animation updates pattern register every N frames:

```
Frame 100: Sprite 0 Pattern = 0x00
Frame 104: Sprite 0 Pattern = 0x01
Frame 108: Sprite 0 Pattern = 0x02
Frame 112: Sprite 0 Pattern = 0x03
Frame 116: Sprite 0 Pattern = 0x00  (cycle repeats)
```

**Pattern 2: Sprite Movement**

Typical sprite movement updates position registers every frame or every few frames:

```
Frame 100: Sprite 0 X = 50, Y = 100
Frame 101: Sprite 0 X = 52, Y = 100  (moving right)
Frame 102: Sprite 0 X = 54, Y = 100
Frame 103: Sprite 0 X = 56, Y = 100
```

**Pattern 3: Color Cycling**

Some games cycle colors for visual effects:

```
Frame 100: Color Register 0xA3 = 0x01 (blue)
Frame 110: Color Register 0xA3 = 0x02 (green)
Frame 120: Color Register 0xA3 = 0x04 (red)
Frame 130: Color Register 0xA3 = 0x01 (blue, cycle repeats)
```

#### 5.8.9 Performance Considerations

Frame-by-frame analysis can generate large trace files. Use these strategies to manage file size:

**Strategy 1: Filter to VDC Only**

```bash
# VDC writes are typically 10-100 per frame vs thousands of CPU instructions
videopac --trace --trace-filter vdc --trace-output vdc_only.txt bios.bin game.bin
```

**Strategy 2: Capture Fewer Frames**

```bash
# Capture just enough frames to see the issue
videopac --trace --trace-start-frame 100 --trace-frames 5 --trace-output short_trace.txt bios.bin game.bin
```

**Strategy 3: Use Headless Mode**

```bash
# Headless mode is faster for trace capture
videopac_headless --trace --trace-frames 100 --trace-output fast_trace.txt bios.bin game.bin
```

#### 5.8.10 Frame-by-Frame Analysis Workflow

**Step-by-Step Process:**

1. **Identify the Problem Frame Range**
   - Note when the issue occurs (frame number, time, or trigger)
   - Determine how many frames to capture (before and after issue)

2. **Capture Trace Data**
   - Use appropriate trace filters (VDC for graphics issues)
   - Capture enough frames to see pattern (typically 10-60 frames)

3. **Extract Frame Data**
   - Split trace into individual frames
   - Focus on relevant registers for the issue

4. **Compare Consecutive Frames**
   - Identify what changes between frames
   - Look for unexpected changes or missing updates

5. **Identify Patterns**
   - Look for repeating sequences (animation cycles)
   - Identify timing patterns (updates every N frames)

6. **Correlate with Code**
   - Use disassembly to find code that writes to registers
   - Trace code path to understand why values change

7. **Form Hypothesis**
   - Based on frame data and code analysis
   - Predict what should happen vs what does happen

8. **Test Hypothesis**
   - Capture additional frames if needed
   - Verify hypothesis with targeted traces

9. **Document Findings**
   - Record frame numbers where issue occurs
   - Document register values and changes
   - Note any patterns or anomalies

#### 5.8.11 Example: Debugging Sprite Animation

**Problem:** Sprite animation appears to skip frames or animate too fast.

**Step 1: Capture Animation Frames**

```bash
videopac --trace --trace-start-key "1" --trace-frames 30 --trace-filter vdc --trace-output sprite_anim.txt bios.bin game.bin
```

**Step 2: Extract Sprite Pattern Changes**

```bash
grep "Sprite 0 Pattern" sprite_anim.txt > sprite0_patterns.txt
cat sprite0_patterns.txt
```

Output:
```
Frame:0001 Scanline:010 VDC_WRITE[Sprite 0 Pattern] = 0x00
Frame:0002 Scanline:010 VDC_WRITE[Sprite 0 Pattern] = 0x01
Frame:0003 Scanline:010 VDC_WRITE[Sprite 0 Pattern] = 0x02
Frame:0004 Scanline:010 VDC_WRITE[Sprite 0 Pattern] = 0x00
```

**Step 3: Analyze Pattern**

- Pattern changes every frame (too fast for smooth animation)
- Expected: Pattern should change every 4-5 frames
- Conclusion: Animation timing logic is incorrect

**Step 4: Find Code**

Use disassembly to locate code that writes sprite pattern register, then analyze timing logic.

#### 5.8.12 Tips and Best Practices

- **Start with VDC-filtered traces** to reduce noise and file size
- **Capture more frames than you think you need** - it's easier than re-capturing
- **Use frame numbers as reference points** when discussing issues with others
- **Document your findings** as you analyze - it's easy to lose track
- **Compare with real hardware** if possible - capture reference frames
- **Use scripting** for repetitive analysis tasks - Python, bash, or your preferred language
- **Focus on changes** between frames rather than absolute values
- **Look for patterns** - most games have predictable update cycles
- **Check timing** - many issues are caused by updates at wrong scanlines
- **Verify VBLANK** - most VDC updates should occur during VBLANK period

### 5.9 Examples

This section provides comprehensive real-world examples demonstrating each graphics debugging workflow. Each example includes the problem scenario, step-by-step solution, command examples, and expected outputs.

---

#### Example 1: Debugging Sprite Color Issue (Sprite Debugging Workflow)

**Scenario:**
A racing game displays the player's car in blue instead of the expected red color. Other sprites (opponent cars) display correctly in white.

**Problem Identification:**
- Symptom: Player car (sprite 0) appears dark blue
- Expected: Player car should be dark red
- Affected: Only sprite 0, other sprites correct
- Reproducible: Occurs immediately when game starts

**Step-by-Step Solution:**

**Step 1: Verify the issue in the debugger**
```bash
# Launch with debugger enabled
videopac --debugger o2rom.bin racing_game.bin
```

Press F12 to open debugger, navigate to VDC Registers panel:
- Check register 0xAB (Sprite 0 Color)
- Observed value: 0x1 (dark blue)
- Expected value: 0x4 (dark red)

**Step 2: Capture VDC trace to find where color is set**
```bash
# Capture first frame after pressing '1' to start game
videopac --trace --trace-start-key "1" --trace-frames 1 --trace-filter vdc --trace-output sprite_color_trace.txt o2rom.bin racing_game.bin
```

**Step 3: Analyze trace for sprite 0 color writes**
```bash
# Search for writes to sprite 0 color register (0xAB)
grep "VDC_WRITE\[0xAB\]" sprite_color_trace.txt
```

**Expected Output:**
```
Frame:0001 Scanline:200 Cycle:12450 | VDC_WRITE[0xAB] = 0x1 (Sprite 0 Color)
```

**Step 4: Generate disassembly to find the code**
```bash
videopac --disassemble-rom racing_game.bin --output racing_game_disasm.txt
```

**Step 5: Search disassembly for VDC writes to 0xAB**
```bash
# Search for OUTL instructions that write to port containing 0xAB
grep -B 2 -A 2 "0xAB" racing_game_disasm.txt
```

**Expected Output:**
```assembly
0x0450   23 01     MOV       A, #0x01    ; Load color value (blue)
0x0452   D3        OUTL      P1, A       ; Write to VDC data
0x0453   23 AB     MOV       A, #0xAB    ; Sprite 0 color register
0x0455   D3        OUTL      P2, A       ; Select register
```

**Root Cause:**
The code at address 0x0450 loads immediate value 0x01 (blue) instead of 0x04 (red).

**Solution:**
Change the MOV instruction at 0x0450 from `MOV A, #0x01` to `MOV A, #0x04`.

**Verification:**
After fixing, run with debugger and verify register 0xAB shows 0x4 (dark red).

---

#### Example 2: Debugging Invisible Sprite (Sprite Debugging Workflow)

**Scenario:**
In a space shooter game, the player's ship is invisible at game start, but appears after moving.

**Problem Identification:**
- Symptom: Player ship (sprite 1) not visible initially
- Expected: Ship should be visible at center of screen
- Affected: Only at game start, appears after first movement
- Reproducible: Every game start

**Step-by-Step Solution:**

**Step 1: Check sprite in debugger at game start**
```bash
videopac --debugger o2rom.bin space_shooter.bin
```

Press '1' to start game, immediately press F12:
- Navigate to VDC Registers → Sprites → Sprite 1
- Check pattern visualization: All pixels are dark gray
- Check pattern bytes: All show 0x00
- Diagnosis: Pattern data not loaded

**Step 2: Capture trace of initialization**
```bash
# Capture first 5 frames after key press
videopac --trace --trace-start-key "1" --trace-frames 5 --trace-filter vdc --trace-output init_trace.txt o2rom.bin space_shooter.bin
```

**Step 3: Search for sprite 1 pattern writes**
```bash
# Sprite 1 pattern memory is at 0x88-0x8F (0x80 + 1*8)
grep "VDC_WRITE\[0x8[89ABCDEF]\]" init_trace.txt | head -20
```

**Expected Output:**
```
Frame:0003 Scanline:210 Cycle:15230 | VDC_WRITE[0x88] = 0x18
Frame:0003 Scanline:210 Cycle:15235 | VDC_WRITE[0x89] = 0x3C
Frame:0003 Scanline:210 Cycle:15240 | VDC_WRITE[0x8A] = 0x7E
...
```

**Observation:**
Pattern data is written in frame 3, not frame 1. This explains why sprite appears after movement.

**Step 4: Analyze disassembly for initialization order**
```bash
videopac --disassemble-rom space_shooter.bin --output space_shooter_disasm.txt
```

Search for initialization code and pattern loading routines.

**Root Cause:**
Pattern loading code is called after first input, not during initialization.

**Solution:**
Move pattern loading code to initialization routine that runs before first frame.

**Verification:**
After fixing, sprite should be visible immediately at game start.

---

#### Example 3: Debugging Character Display (Character Debugging Workflow)

**Scenario:**
A text-based game displays garbled characters instead of readable text in the score display.

**Problem Identification:**
- Symptom: Score area shows random patterns instead of numbers
- Expected: Should display "SCORE: 0000"
- Affected: All character-based text
- Reproducible: Always occurs

**Step-by-Step Solution:**

**Step 1: Verify character grid is enabled**
```bash
videopac --debugger o2rom.bin text_game.bin
```

Press F12, check VDC Registers:
- Register 0xAF (Grid Control): Check bit 7 (grid enable)
- Observed: 0x80 (bit 7 set, grid enabled)
- Grid is enabled, so issue is with character data

**Step 2: Capture VDC trace for character writes**
```bash
videopac --trace --trace-start-frame 10 --trace-frames 1 --trace-filter vdc --trace-output char_trace.txt o2rom.bin text_game.bin
```

**Step 3: Analyze character pattern memory writes**
```bash
# Character patterns are in VDC memory 0x00-0x7F
grep "VDC_WRITE\[0x[0-7][0-9A-F]\]" char_trace.txt | head -30
```

**Expected Output:**
```
Frame:0010 Scanline:205 Cycle:14200 | VDC_WRITE[0x00] = 0xFF
Frame:0010 Scanline:205 Cycle:14205 | VDC_WRITE[0x01] = 0x81
Frame:0010 Scanline:205 Cycle:14210 | VDC_WRITE[0x02] = 0x81
...
```

**Step 4: Verify character pattern data**

Compare the pattern bytes with expected character patterns. For digit '0':
```
Expected pattern for '0':
  0x3C (00111100) - Top
  0x66 (01100110)
  0x66 (01100110)
  0x66 (01100110)
  0x66 (01100110)
  0x66 (01100110)
  0x3C (00111100) - Bottom

Actual pattern:
  0xFF (11111111) - Wrong!
  0x81 (10000001)
  0x81 (10000001)
  ...
```

**Root Cause:**
Character pattern data is incorrect or corrupted. The game is loading wrong data into character memory.

**Step 5: Check ROM for character pattern data**
```bash
# Generate disassembly and look for character data tables
videopac --disassemble-rom text_game.bin --output text_game_disasm.txt

# Search for data tables (sequences of similar values)
grep -A 20 "DB\|DATA" text_game_disasm.txt
```

**Solution:**
Verify character pattern data in ROM matches expected font patterns. Fix data table or loading routine.

**Verification:**
After fixing, characters should display correctly as readable text.

---

#### Example 4: Debugging Grid Display (Grid Debugging Workflow)

**Scenario:**
A puzzle game's grid background is not visible, making it difficult to see the play area.

**Problem Identification:**
- Symptom: Grid background not displayed
- Expected: Should show grid pattern for puzzle pieces
- Affected: Entire grid
- Reproducible: Always occurs

**Step-by-Step Solution:**

**Step 1: Check grid control register**
```bash
videopac --debugger o2rom.bin puzzle_game.bin
```

Press F12, check VDC Registers:
- Register 0xAF (Grid Control): 0x00
- Bit 7 is 0, meaning grid is disabled

**Step 2: Capture trace to find grid control writes**
```bash
videopac --trace --trace-start-frame 0 --trace-frames 10 --trace-filter vdc --trace-output grid_trace.txt o2rom.bin puzzle_game.bin
```

**Step 3: Search for grid control register writes**
```bash
grep "VDC_WRITE\[0xAF\]" grid_trace.txt
```

**Expected Output:**
```
(No results found)
```

**Observation:**
Grid control register is never written to, meaning grid is never enabled.

**Step 4: Check disassembly for grid initialization**
```bash
videopac --disassemble-rom puzzle_game.bin --output puzzle_game_disasm.txt

# Search for writes to 0xAF
grep -B 3 -A 3 "0xAF" puzzle_game_disasm.txt
```

**Expected Output:**
```
(No results or commented-out code)
```

**Root Cause:**
Grid initialization code is missing or never called.

**Solution:**
Add grid initialization code:
```assembly
; Enable grid
MOV A, #0x80      ; Grid enable bit
OUTL P1, A        ; Write to VDC
MOV A, #0xAF      ; Grid control register
OUTL P2, A        ; Select register
```

**Verification:**
After adding initialization, grid should be visible.

---

#### Example 5: Debugging Color Palette (Color Palette Debugging Workflow)

**Scenario:**
A platformer game has incorrect colors - the sky is green instead of blue, and the ground is blue instead of green.

**Problem Identification:**
- Symptom: Colors are swapped (sky green, ground blue)
- Expected: Sky should be blue, ground should be green
- Affected: Background colors
- Reproducible: Always occurs

**Step-by-Step Solution:**

**Step 1: Verify color register values**
```bash
videopac --debugger o2rom.bin platformer.bin
```

Press F12, check VDC Registers:
- Register 0xA3 (Background Color): 0x2 (dark green)
- Expected for sky: 0x1 (dark blue)

**Step 2: Capture trace for color register writes**
```bash
videopac --trace --trace-start-frame 0 --trace-frames 1 --trace-filter vdc --trace-output color_trace.txt o2rom.bin platformer.bin
```

**Step 3: Search for background color writes**
```bash
grep "VDC_WRITE\[0xA3\]" color_trace.txt
```

**Expected Output:**
```
Frame:0000 Scanline:195 Cycle:13100 | VDC_WRITE[0xA3] = 0x2 (Background Color)
```

**Step 4: Analyze disassembly for color setting code**
```bash
videopac --disassemble-rom platformer.bin --output platformer_disasm.txt

# Search for writes to 0xA3
grep -B 3 -A 3 "0xA3" platformer_disasm.txt
```

**Expected Output:**
```assembly
0x0600   23 02     MOV       A, #0x02    ; Load green (should be 0x01 for blue)
0x0602   D3        OUTL      P1, A       ; Write to VDC
0x0603   23 A3     MOV       A, #0xA3    ; Background color register
0x0605   D3        OUTL      P2, A       ; Select register
```

**Root Cause:**
Color value is 0x02 (green) instead of 0x01 (blue). Colors are swapped in the code.

**Solution:**
Change MOV instruction at 0x0600 from `MOV A, #0x02` to `MOV A, #0x01`.
Also check ground color code and swap it from 0x01 to 0x02.

**Verification:**
After fixing, sky should be blue and ground should be green.

---

#### Example 6: Debugging Collision Detection (Collision Detection Debugging Workflow)

**Scenario:**
In a space invaders game, bullets pass through enemies without destroying them.

**Problem Identification:**
- Symptom: Collision detection not working
- Expected: Bullet should destroy enemy on contact
- Affected: Bullet-enemy collisions
- Reproducible: Always occurs

**Step-by-Step Solution:**

**Step 1: Verify collision register is being set**
```bash
videopac --debugger o2rom.bin space_invaders.bin
```

Fire bullet at enemy, press F12 when they overlap:
- Check register 0xA2 (Collision): 0x00
- Expected: Should have collision bits set
- Observation: Hardware collision detection not triggering

**Step 2: Verify sprite overlap**

In debugger, check Sprite Visualization:
- Bullet (sprite 2): X=100, Y=50
- Enemy (sprite 1): X=105, Y=52
- Sprites are close but may not overlap enough

**Step 3: Capture trace during collision**
```bash
# Capture frames during collision event
videopac --trace --trace-start-frame 100 --trace-frames 5 --trace-filter vdc --trace-output collision_trace.txt o2rom.bin space_invaders.bin
```

**Step 4: Check if game reads collision register**
```bash
# Search for reads from collision register
grep "READ\[0xA2\]" collision_trace.txt
```

**Expected Output:**
```
(No results found)
```

**Observation:**
Game never reads the collision register! This means the game uses software collision detection, not hardware collision.

**Step 5: Analyze disassembly for collision detection code**
```bash
videopac --disassemble-rom space_invaders.bin --output space_invaders_disasm.txt

# Search for collision detection logic
grep -B 5 -A 10 "collision\|overlap\|hit" space_invaders_disasm.txt
```

**Root Cause:**
Game implements software collision detection by comparing sprite positions. The collision detection logic has a bug in the overlap calculation.

**Solution:**
Fix the software collision detection algorithm to correctly detect overlaps:
```assembly
; Correct overlap detection
; Check if |X1 - X2| < 8 AND |Y1 - Y2| < 8
```

**Verification:**
After fixing, bullets should destroy enemies on contact.

---

#### Example 7: Using VDC Register Viewer (VDC Register Viewer Workflow)

**Scenario:**
A developer needs to quickly verify all VDC register values during a specific game state without capturing traces.

**Step-by-Step Solution:**

**Step 1: Launch with debugger**
```bash
videopac --debugger o2rom.bin test_game.bin
```

**Step 2: Navigate to desired game state**
- Press '1' to start game
- Play until reaching the state to debug (e.g., level 2)

**Step 3: Pause and inspect registers**
- Press F12 to open debugger
- Navigate to VDC Registers panel
- Review all register values:

**Example Register State:**
```
0xA0 (Control):     0x01  - VDC enabled
0xA1 (Status):      0x80  - VBLANK active
0xA2 (Collision):   0x00  - No collisions
0xA3 (BG Color):    0x01  - Dark blue background
0xA4 (Grid Y):      0x00  - Grid Y position
0xA5 (Grid X):      0x00  - Grid X position
0xA7 (Sound 0):     0x00  - Sound byte 0
0xA8 (Sound 1):     0x00  - Sound byte 1
0xA9 (Sound 2):     0x00  - Sound byte 2
0xAA (Sound Ctrl):  0x04  - Sound control
0xAB (Sprite 0):    0x04  - Dark red
0xAC (Sprite 1):    0x07  - Grey
0xAD (Sprite 2):    0x02  - Dark green
0xAE (Sprite 3):    0x01  - Dark blue
0xAF (Grid Ctrl):   0x80  - Grid enabled
```

**Step 4: Identify issues**
- All values look correct for this game state
- Sprite colors are distinct and visible
- Grid is enabled
- No unexpected collision flags

**Step 5: Step through code and watch changes**
- Press F9 to step one instruction
- Watch which registers change
- Identify when specific registers are updated

**Use Case:**
This workflow is ideal for quick verification without the overhead of trace capture and analysis.

---

#### Example 8: Using Sprite Visualization (Sprite Visualization Workflow)

**Scenario:**
A game's player sprite appears corrupted with random pixels instead of the expected ship pattern.

**Step-by-Step Solution:**

**Step 1: Launch with debugger**
```bash
videopac --debugger o2rom.bin ship_game.bin
```

**Step 2: Open sprite visualization**
- Press F12
- Navigate to VDC Registers → Sprites → Sprite 0

**Step 3: Inspect pattern visualization**

**Expected Pattern (ship):**
```
  . . # # # # . .
  . # # # # # # .
  # # # # # # # #
  # # . # # . # #
  # # # # # # # #
  . # # # # # # .
  . . # # # # . .
  . . . # # . . .
```

**Actual Pattern (corrupted):**
```
  # . # . # . # .
  . # . # . # . #
  # . # . # . # .
  . # . # . # . #
  # . # . # . # .
  . # . # . # . #
  # . # . # . # .
  . # . # . # . #
```

**Step 4: Check pattern bytes**
```
Row 0: 0xAA (10101010) - Alternating pattern
Row 1: 0x55 (01010101) - Alternating pattern
Row 2: 0xAA (10101010) - Alternating pattern
...
```

**Observation:**
Pattern is a checkerboard (0xAA, 0x55 alternating), not the ship pattern.

**Step 5: Capture trace to find pattern writes**
```bash
videopac --trace --trace-start-frame 0 --trace-frames 1 --trace-filter vdc --trace-output pattern_trace.txt o2rom.bin ship_game.bin
```

**Step 6: Search for sprite 0 pattern writes**
```bash
# Sprite 0 pattern is at 0x80-0x87
grep "VDC_WRITE\[0x8[0-7]\]" pattern_trace.txt
```

**Expected Output:**
```
Frame:0000 Scanline:200 Cycle:12000 | VDC_WRITE[0x80] = 0xAA
Frame:0000 Scanline:200 Cycle:12005 | VDC_WRITE[0x81] = 0x55
...
```

**Root Cause:**
Wrong pattern data is being loaded. The game is loading test pattern (checkerboard) instead of ship pattern.

**Solution:**
Fix pattern data source in ROM or pattern loading routine to load correct ship pattern.

**Verification:**
After fixing, sprite visualization should show correct ship pattern.

---

#### Example 9: Frame-by-Frame Analysis (Frame-by-Frame Analysis Workflow)

**Scenario:**
A game's sprite animation appears to skip frames, making movement look jerky.

**Step-by-Step Solution:**

**Step 1: Capture multiple frames**
```bash
# Capture 10 frames starting when animation begins
videopac --trace --trace-start-key "1" --trace-frames 10 --trace-filter vdc --trace-output animation_frames.txt o2rom.bin animation_game.bin
```

**Step 2: Extract sprite pattern writes per frame**
```bash
# Extract frame 1 sprite 0 pattern writes
grep "Frame:0001.*VDC_WRITE\[0x8[0-7]\]" animation_frames.txt > frame1_patterns.txt

# Extract frame 2 sprite 0 pattern writes
grep "Frame:0002.*VDC_WRITE\[0x8[0-7]\]" animation_frames.txt > frame2_patterns.txt

# Continue for all frames...
```

**Step 3: Compare pattern changes between frames**

**Frame 1 Pattern:**
```
0x80: 0x18
0x81: 0x3C
0x82: 0x7E
...
```

**Frame 2 Pattern:**
```
0x80: 0x18  (Same as frame 1)
0x81: 0x3C  (Same as frame 1)
0x82: 0x7E  (Same as frame 1)
...
```

**Frame 3 Pattern:**
```
0x80: 0x18  (Still same!)
0x81: 0x3C  (Still same!)
0x82: 0x7E  (Still same!)
...
```

**Frame 4 Pattern:**
```
0x80: 0x24  (Changed!)
0x81: 0x42  (Changed!)
0x82: 0x81  (Changed!)
...
```

**Observation:**
Pattern changes every 3 frames instead of every frame, causing jerky animation.

**Step 4: Analyze animation update logic**
```bash
videopac --disassemble-rom animation_game.bin --output animation_game_disasm.txt

# Search for animation counter or frame counter logic
grep -B 5 -A 10 "counter\|frame\|anim" animation_game_disasm.txt
```

**Root Cause:**
Animation update code has a counter that updates every 3 frames instead of every frame.

**Solution:**
Change animation update frequency from every 3 frames to every frame:
```assembly
; Change from:
DJNZ R5, skip_update  ; Decrement counter, skip if not zero (counts 3 frames)

; To:
JMP update_animation  ; Update every frame
```

**Verification:**
After fixing, capture another 10-frame trace and verify pattern changes every frame.

---

#### Example 10: Complete Debugging Session (All Workflows Combined)

**Scenario:**
A new game has multiple graphics issues: sprites flicker, colors are wrong, and collision detection doesn't work.

**Complete Debugging Process:**

**Phase 1: Initial Assessment**
```bash
# Launch with debugger to assess issues
videopac --debugger o2rom.bin problem_game.bin
```

Observations:
1. Sprites flicker (appear/disappear)
2. Sprite colors are incorrect
3. Collisions not detected

**Phase 2: Debug Flickering (Frame-by-Frame Analysis)**
```bash
# Capture 20 frames to analyze flickering
videopac --trace --trace-start-frame 50 --trace-frames 20 --trace-filter vdc --trace-output flicker_trace.txt o2rom.bin problem_game.bin

# Extract sprite position writes per frame
for i in {50..69}; do
  grep "Frame:00$i.*VDC_WRITE\[0xA[0-9A-F]\]" flicker_trace.txt > frame_$i.txt
done

# Compare frames
diff frame_50.txt frame_51.txt
```

Finding: Sprite positions are set to 0,0 on odd frames (off-screen).

Solution: Fix position update logic to maintain positions every frame.

**Phase 3: Debug Colors (Color Palette Workflow)**
```bash
# Check color registers in debugger
# Press F12, check VDC Registers
```

Finding: Sprite 0 color is 0x1 (blue), should be 0x4 (red).

```bash
# Capture trace for color writes
videopac --trace --trace-start-frame 0 --trace-frames 1 --trace-filter vdc --trace-output color_trace.txt o2rom.bin problem_game.bin

# Find color register writes
grep "VDC_WRITE\[0xAB\]" color_trace.txt
```

Solution: Change color value in initialization code from 0x1 to 0x4.

**Phase 4: Debug Collision (Collision Detection Workflow)**
```bash
# Check if collision register is read
grep "READ\[0xA2\]" flicker_trace.txt
```

Finding: Collision register never read - game uses software collision.

```bash
# Analyze disassembly for collision logic
videopac --disassemble-rom problem_game.bin --output problem_game_disasm.txt
grep -B 10 -A 10 "collision" problem_game_disasm.txt
```

Solution: Fix software collision detection algorithm.

**Phase 5: Verification**

After all fixes:
```bash
# Test with debugger
videopac --debugger o2rom.bin problem_game_fixed.bin
```

Verify:
- [ ] Sprites no longer flicker
- [ ] Colors are correct
- [ ] Collisions work properly

**Complete!**

---

**Summary of Examples:**

These examples demonstrate practical application of all graphics debugging workflows:
1. Sprite color issues → VDC register analysis + trace capture
2. Invisible sprites → Pattern data analysis + initialization debugging
3. Character display → Character memory analysis
4. Grid display → Grid control register debugging
5. Color palette → Color register analysis
6. Collision detection → Hardware vs software collision analysis
7. VDC Register Viewer → Real-time register inspection
8. Sprite Visualization → Pattern visualization and analysis
9. Frame-by-frame → Animation and timing analysis
10. Complete session → Combined workflow for complex issues

Each example includes real commands, expected outputs, and step-by-step solutions that can be applied to similar issues in any Videopac game.

[↑ Back to Top](#table-of-contents)

---

## 6. Memory Analysis

### 6.1 Videopac Memory Map

**Memory Layout:**

| Address Range | Size | Description |
|---------------|------|-------------|
| 0x0000-0x03FF | 1KB | Internal ROM (BIOS) |
| 0x0400-0x0FFF | 3KB | External ROM (Cartridge) |
| 0x1000-0x13FF | 1KB | Internal RAM |
| 0x1400-0x17FF | 1KB | External RAM (if present) |
| 0xA0-0xAF | 16 bytes | VDC registers |
| 0x20-0x27 | 8 bytes | Port 1 (input) |
| 0x90-0x97 | 8 bytes | Port 2 (input) |

### 6.2 Memory Dump Capture

Memory dumps provide snapshots of RAM contents at specific execution points, allowing you to inspect game state, data structures, and variable values. This is essential for debugging data-related issues, understanding game logic, and verifying memory initialization.

#### 6.2.1 When to Use Memory Dumps

Memory dumps are most effective for:

- **Data Structure Analysis:** Examining sprite tables, level data, score values, game state variables
- **Initialization Verification:** Confirming memory is properly initialized at startup
- **State Comparison:** Comparing memory before and after specific events
- **Corruption Detection:** Identifying unexpected memory modifications
- **Save State Analysis:** Understanding how games store persistent data
- **Variable Tracking:** Monitoring specific memory locations over time

#### 6.2.2 Capturing Memory Dumps via Command Line

**Basic Memory Dump:**

```bash
# Dump all RAM (0x1000-0x13FF) at specific frame
videopac --memory-dump --dump-frame 100 --dump-output mem_frame100.bin bios.bin game.bin

# Dump specific address range
videopac --memory-dump --dump-address 0x1000:0x10FF --dump-frame 50 --dump-output mem_range.bin bios.bin game.bin

# Dump on key press
videopac --memory-dump --dump-on-key "1" --dump-output mem_after_key.bin bios.bin game.bin
```

**Multiple Dumps for Comparison:**

```bash
# Dump at frame 0 (initialization)
videopac --memory-dump --dump-frame 0 --dump-output mem_init.bin bios.bin game.bin

# Dump at frame 100 (after gameplay)
videopac --memory-dump --dump-frame 100 --dump-output mem_gameplay.bin bios.bin game.bin

# Compare the two dumps
diff <(hexdump -C mem_init.bin) <(hexdump -C mem_gameplay.bin)
```

#### 6.2.3 Capturing Memory Dumps via In-Game Debugger

**Interactive Memory Dump:**

1. **Launch emulator with debugger:**
   ```bash
   videopac --debugger bios.bin game.bin
   ```

2. **Toggle debugger:** Press F12 during emulation

3. **Navigate to Memory Viewer panel**

4. **Set breakpoint at desired location:**
   - Press F2 to set breakpoint at current PC
   - Or set breakpoint at specific address (Ctrl+B, enter address)

5. **Continue execution:** Press F5 until breakpoint is hit

6. **Inspect memory in Memory Viewer:**
   - Use PgUp/PgDn to scroll through memory
   - Use Ctrl+G to jump to specific address
   - Note values of interest

7. **Export memory dump:**
   - Press Ctrl+E to export current memory view
   - Specify filename and address range
   - Save dump for later analysis

#### 6.2.4 Memory Dump File Formats

**Binary Format (.bin):**
- Raw binary data, one byte per memory location
- Smallest file size
- Requires hex editor or custom tools to view
- Best for programmatic analysis

**Hexdump Format (.txt):**
- Human-readable hex and ASCII representation
- Larger file size
- Easy to view in text editor
- Best for manual inspection

**Example Hexdump Output:**
```
00001000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00001010  42 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |B...............|
00001020  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
```

#### 6.2.5 Analyzing Memory Dumps

**Using Hex Editors:**

Popular hex editors for memory dump analysis:
- **HxD** (Windows) - Free, fast, supports large files
- **Hex Fiend** (macOS) - Free, native macOS app
- **xxd** (Linux/macOS) - Command-line hex dump tool
- **hexdump** (Linux/macOS/Windows) - Standard Unix tool

**Basic Hex Editor Workflow:**

1. Open memory dump in hex editor
2. Navigate to address of interest (subtract 0x1000 for RAM offset)
3. Inspect byte values in hex and ASCII
4. Look for patterns, strings, or recognizable data
5. Note addresses of interesting data structures

**Using Command-Line Tools:**

```bash
# View memory dump with hexdump
hexdump -C mem_dump.bin | less

# Search for specific byte pattern
hexdump -C mem_dump.bin | grep "42 00 00"

# Extract specific address range (bytes 0x00-0xFF)
dd if=mem_dump.bin of=extracted.bin bs=1 skip=0 count=256

# Compare two memory dumps
cmp -l mem_before.bin mem_after.bin | head -20
```

#### 6.2.6 Identifying Data in Memory Dumps

**Recognizing Common Patterns:**

**Sprite Tables:**
- Typically 4-8 bytes per sprite (X, Y, color, pattern, flags)
- Often located at fixed addresses
- Look for values in valid ranges (X: 0-255, Y: 0-255, color: 0-7)

**Example Sprite Table:**
```
Address  Data                          Interpretation
0x1000   64 80 04 12 00 00 00 00      Sprite 0: X=64, Y=80, Color=4, Pattern=0x12
0x1008   C0 60 01 34 00 00 00 00      Sprite 1: X=192, Y=96, Color=1, Pattern=0x34
```

**Score Values:**
- Often stored as BCD (Binary-Coded Decimal)
- Each nibble represents a decimal digit (0-9)
- Example: 0x1234 = 1234 points

**Game State Flags:**
- Bit flags packed into bytes
- Each bit represents a boolean state
- Example: 0x05 = 0000 0101 = flags 0 and 2 are set

**Level Data:**
- Arrays of tile indices or object types
- Often compressed or encoded
- Look for repeating patterns or sequences

**Text Strings:**
- ASCII characters (0x20-0x7E)
- May be null-terminated (0x00) or length-prefixed
- Look for readable text in ASCII column

#### 6.2.7 Comparing Memory Dumps

**Identifying Changes:**

```bash
# Generate hexdumps for comparison
hexdump -C mem_before.bin > mem_before.txt
hexdump -C mem_after.bin > mem_after.txt

# Compare with diff
diff mem_before.txt mem_after.txt

# Or use specialized diff tool
diff -u mem_before.txt mem_after.txt | less
```

**Interpreting Diff Output:**

```
< 00001020  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
---
> 00001020  42 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |B...............|
```

This shows that byte at address 0x1020 changed from 0x00 to 0x42.

**Automated Comparison Script:**

```python
# mem_diff.py
import sys

def compare_dumps(file1, file2):
    """Compare two binary memory dumps and report differences."""
    with open(file1, 'rb') as f1, open(file2, 'rb') as f2:
        data1 = f1.read()
        data2 = f2.read()
    
    if len(data1) != len(data2):
        print(f"Warning: File sizes differ ({len(data1)} vs {len(data2)})")
    
    min_len = min(len(data1), len(data2))
    differences = []
    
    for i in range(min_len):
        if data1[i] != data2[i]:
            differences.append((i, data1[i], data2[i]))
    
    print(f"Found {len(differences)} differences:")
    for addr, val1, val2 in differences[:20]:  # Show first 20
        print(f"  0x{addr+0x1000:04X}: 0x{val1:02X} -> 0x{val2:02X}")
    
    if len(differences) > 20:
        print(f"  ... and {len(differences)-20} more")

if __name__ == "__main__":
    compare_dumps(sys.argv[1], sys.argv[2])
```

Usage:
```bash
python mem_diff.py mem_before.bin mem_after.bin
```

#### 6.2.8 Practical Examples

**Example 1: Capturing Initialization State**

```bash
# Capture memory at frame 0 (after BIOS initialization)
videopac --memory-dump --dump-frame 0 --dump-output mem_init.bin bios.bin game.bin

# View the dump
hexdump -C mem_init.bin | less

# Look for non-zero values (initialized data)
hexdump -C mem_init.bin | grep -v "00 00 00 00 00 00 00 00"
```

**Example 2: Tracking Score Changes**

```bash
# Capture memory before scoring event
videopac --memory-dump --dump-frame 100 --dump-output mem_before_score.bin bios.bin game.bin

# Capture memory after scoring event
videopac --memory-dump --dump-frame 110 --dump-output mem_after_score.bin bios.bin game.bin

# Compare to find score variable location
python mem_diff.py mem_before_score.bin mem_after_score.bin
```

**Example 3: Analyzing Sprite Table**

```bash
# Capture memory during gameplay
videopac --memory-dump --dump-frame 50 --dump-output mem_sprites.bin bios.bin game.bin

# Extract suspected sprite table (e.g., 0x1000-0x101F = 32 bytes = 4 sprites * 8 bytes)
dd if=mem_sprites.bin of=sprite_table.bin bs=1 skip=0 count=32

# View sprite table
hexdump -C sprite_table.bin
```

**Example 4: Finding Text Strings**

```bash
# Capture memory dump
videopac --memory-dump --dump-output mem_full.bin bios.bin game.bin

# Search for ASCII strings
strings mem_full.bin

# Or use hexdump and grep
hexdump -C mem_full.bin | grep -i "score\|game\|over"
```

#### 6.2.9 Tips and Best Practices

**Tip 1: Capture at Consistent Points**
- Always capture dumps at the same game state for comparison
- Use frame numbers or key presses for reproducibility
- Document the game state when each dump was captured

**Tip 2: Focus on RAM Regions**
- ROM (0x0000-0x0FFF) doesn't change, focus on RAM (0x1000-0x17FF)
- VDC registers (0xA0-0xAF) are better analyzed with traces
- I/O ports (0x20-0x27, 0x90-0x97) are read-only

**Tip 3: Use Meaningful Filenames**
- Include frame number, game state, or event in filename
- Example: `mem_frame100_after_jump.bin`
- Makes it easier to organize and compare dumps

**Tip 4: Combine with Traces**
- Memory dumps show state at a point in time
- Traces show how that state was reached
- Use both together for complete understanding

**Tip 5: Look for Patterns**
- Data structures often have recognizable patterns
- Sprite tables have regular spacing
- Arrays have consistent element sizes
- Use patterns to identify data structure boundaries

#### 6.2.10 Common Pitfalls

**Pitfall 1: Confusing Addresses**
- Memory dump file offsets start at 0x00
- Actual RAM addresses start at 0x1000
- Add 0x1000 to file offset to get real address

**Pitfall 2: Timing Issues**
- Memory can change mid-frame
- Dumps captured at different frames may not be comparable
- Always capture at consistent timing points

**Pitfall 3: Incomplete Dumps**
- Some emulators only dump internal RAM (0x1000-0x13FF)
- External RAM (0x1400-0x17FF) may not be included
- Verify dump size matches expected memory range

**Pitfall 4: Endianness Confusion**
- Intel 8048 is big-endian
- Multi-byte values are stored high byte first
- Example: 0x1234 is stored as 0x12 0x34

**Pitfall 5: Assuming Static Data**
- Memory contents change constantly during execution
- Don't assume data structures are at fixed addresses
- Verify addresses with multiple dumps or traces

#### 6.2.11 Integration with Other Tools

**With Trace Analysis:**
1. Capture memory dump at specific frame
2. Capture trace for same frame
3. Use trace to see how memory was modified
4. Correlate memory writes in trace with dump values

**With Disassembly:**
1. Identify memory addresses in dump
2. Search disassembly for references to those addresses
3. Analyze code that reads/writes those addresses
4. Understand data structure usage

**With Memory Viewer:**
1. Capture dump for offline analysis
2. Use Memory Viewer for real-time inspection
3. Compare dump values with live values
4. Identify dynamic vs static data

**With Watch Expressions:**
1. Identify interesting addresses in dump
2. Set watch expressions for those addresses
3. Monitor changes in real-time
4. Capture new dumps when values change

#### 6.2.12 Summary

Memory dumps are essential for understanding game state and data structures. They provide snapshots of RAM contents that can be analyzed offline, compared across different game states, and correlated with traces and disassembly. Use memory dumps to:

- Verify initialization
- Track variable changes
- Identify data structures
- Debug data corruption
- Understand game logic

Combined with traces and disassembly, memory dumps provide a complete picture of how games store and manipulate data.

### 6.3 Using Memory Viewer

The in-game Memory Viewer provides real-time inspection of RAM contents during emulation. Unlike memory dumps which capture static snapshots, the Memory Viewer allows you to watch memory change as the game executes, making it ideal for interactive debugging and understanding dynamic behavior.

#### 6.3.1 Accessing the Memory Viewer

1. **Launch emulator with debugger enabled:**
   ```bash
   videopac --debugger bios.bin game.bin
   ```

2. **Toggle debugger:** Press F12 during emulation

3. **Navigate to Memory Viewer panel:** Use arrow keys or click on the Memory Viewer tab

4. **The Memory Viewer displays:**
   - Hexadecimal memory addresses (left column)
   - Hex byte values (center columns, 16 bytes per row)
   - ASCII representation (right column)
   - Current cursor position and selected byte

#### 6.3.2 Memory Viewer Interface

**Display Format:**

```
Address   00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  ASCII
--------  ----------------------  ----------------------  ----------------
00001000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  ................
00001010  42 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  B...............
00001020  64 80 04 12 00 00 00 00  C0 60 01 34 00 00 00 00  d........`.4....
00001030  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  ................
```

**Column Descriptions:**
- **Address:** Memory address of the first byte in the row (0x1000-0x17FF for RAM)
- **Hex Bytes:** 16 bytes displayed in hexadecimal (00-FF)
- **ASCII:** ASCII representation of bytes (printable characters or '.' for non-printable)

**Visual Indicators:**
- **Highlighted byte:** Current cursor position
- **Changed bytes:** Recently modified bytes may be highlighted in different color
- **Breakpoint markers:** Bytes with memory breakpoints may have special indicators

#### 6.3.3 Navigation Controls

**Keyboard Navigation:**

| Key | Action |
|-----|--------|
| Up/Down Arrow | Move cursor up/down one row (16 bytes) |
| Left/Right Arrow | Move cursor left/right one byte |
| PgUp | Scroll up one page (~256 bytes) |
| PgDn | Scroll down one page (~256 bytes) |
| Home | Jump to start of current row |
| End | Jump to end of current row |
| Ctrl+Home | Jump to start of memory (0x1000) |
| Ctrl+End | Jump to end of memory (0x17FF) |
| Ctrl+G | Go to specific address (opens dialog) |

**Mouse Navigation:**
- Click on any byte to move cursor to that position
- Scroll wheel to scroll up/down
- Click and drag to select multiple bytes

#### 6.3.4 Jumping to Specific Addresses

**Using Go To Address (Ctrl+G):**

1. Press Ctrl+G to open "Go To Address" dialog
2. Enter address in hexadecimal (e.g., 0x1020 or 1020)
3. Press Enter to jump to that address
4. Cursor will be positioned at the specified byte

**Common Addresses to Inspect:**

| Address Range | Description |
|---------------|-------------|
| 0x1000-0x13FF | Internal RAM (1KB) |
| 0x1400-0x17FF | External RAM (1KB, if present) |
| 0x1000-0x101F | Often used for sprite tables |
| 0x1020-0x103F | Often used for game state variables |
| 0x1040-0x10FF | Often used for level data or buffers |

#### 6.3.5 Inspecting Memory Values

**Reading Hex Values:**

Each byte is displayed as two hexadecimal digits (00-FF):
- 0x00 = 0 decimal
- 0x42 = 66 decimal
- 0xFF = 255 decimal

**Reading Multi-Byte Values:**

The Intel 8048 uses big-endian byte order (most significant byte first):

```
Address  Bytes        Interpretation
0x1000   12 34        16-bit value: 0x1234 = 4660 decimal
0x1002   00 FF        16-bit value: 0x00FF = 255 decimal
0x1004   42 00 00 00  32-bit value: 0x42000000 (if used)
```

**Reading ASCII Strings:**

Look at the ASCII column for readable text:

```
Address   Hex Bytes                                        ASCII
00001000  53 43 4F 52 45 3A 20 00  00 00 00 00 00 00 00 00  SCORE: .........
```

This shows the string "SCORE: " stored at address 0x1000.

#### 6.3.6 Monitoring Memory Changes

**Real-Time Monitoring:**

1. **Pause execution:** Press F9 to pause
2. **Note current values:** Record values of interest
3. **Step through code:** Press F9 to execute one instruction
4. **Watch for changes:** Changed bytes may be highlighted
5. **Repeat:** Continue stepping and watching

**Identifying Write Operations:**

When a byte changes:
1. Note the address and new value
2. Check CPU State panel for current PC
3. Look at Disassembly panel to see instruction that wrote the value
4. Verify the write was expected

**Example Workflow:**

```
1. Pause at address 0x0400 (game initialization)
2. Check memory at 0x1000: all zeros
3. Step through 10 instructions
4. Check memory at 0x1000: now contains 0x42
5. Conclusion: Initialization code wrote 0x42 to 0x1000
```

#### 6.3.7 Setting Memory Breakpoints

**Memory Write Breakpoints:**

1. Navigate to address of interest in Memory Viewer
2. Right-click on the byte (or press F2)
3. Select "Set Memory Write Breakpoint"
4. Continue execution (F5)
5. Emulator will pause when that address is written

**Memory Read Breakpoints:**

1. Navigate to address of interest
2. Right-click on the byte
3. Select "Set Memory Read Breakpoint"
4. Continue execution (F5)
5. Emulator will pause when that address is read

**Managing Breakpoints:**

- View all breakpoints in Breakpoints panel
- Disable/enable breakpoints without removing them
- Remove breakpoints by right-clicking and selecting "Remove Breakpoint"

#### 6.3.8 Searching Memory

**Search for Byte Pattern:**

1. Press Ctrl+F to open Find dialog
2. Enter hex bytes to search for (e.g., "42 00 00")
3. Press Enter to find first occurrence
4. Press F3 to find next occurrence
5. Press Shift+F3 to find previous occurrence

**Search for ASCII String:**

1. Press Ctrl+F to open Find dialog
2. Switch to "ASCII" mode
3. Enter text to search for (e.g., "SCORE")
4. Press Enter to find first occurrence

**Search Options:**
- **Case sensitive:** Match exact case for ASCII searches
- **Wrap around:** Continue search from beginning when reaching end
- **Search direction:** Forward or backward from current position

#### 6.3.9 Editing Memory Values

**Modifying Bytes (for testing):**

1. Navigate to byte to modify
2. Press Enter to enter edit mode
3. Type new hex value (00-FF)
4. Press Enter to confirm or Esc to cancel

**Use Cases for Memory Editing:**
- **Testing game behavior:** Change score, lives, or state variables
- **Bypassing initialization:** Set memory to known state
- **Reproducing bugs:** Force specific memory values
- **Verifying hypotheses:** Test if changing a value fixes an issue

**Warning:** Memory edits are temporary and will be overwritten if the game writes to those addresses. Use breakpoints to prevent overwrites if needed.

#### 6.3.10 Comparing Memory Regions

**Manual Comparison:**

1. Navigate to first address (e.g., 0x1000)
2. Note values of interest
3. Navigate to second address (e.g., 0x1100)
4. Compare values visually

**Using Dual Memory Viewers:**

Some debuggers support split-screen memory viewers:
1. Open second Memory Viewer panel
2. Navigate first viewer to address A
3. Navigate second viewer to address B
4. Compare side-by-side

#### 6.3.11 Practical Examples

**Example 1: Finding Sprite Table**

```
Problem: Need to locate sprite position data in memory

Steps:
1. Press F12 to open debugger
2. Open Memory Viewer
3. Jump to 0x1000 (start of RAM)
4. Look for patterns: X, Y, Color, Pattern (4 bytes per sprite)
5. Move sprite in game (use arrow keys)
6. Watch for changing values in memory
7. Identify addresses that change with sprite movement
8. Verify: X position should be 0-255, Y position should be 0-255
```

**Example 2: Tracking Score Variable**

```
Problem: Need to find where score is stored

Steps:
1. Note current score (e.g., 0 points)
2. Open Memory Viewer, jump to 0x1000
3. Search for 0x00 (initial score)
4. Score a point in game
5. Watch for memory location that changes from 0x00 to 0x01
6. Verify by scoring more points and watching value increase
7. Set memory write breakpoint on score address
8. Continue execution, breakpoint will trigger when score changes
```

**Example 3: Analyzing Level Data**

```
Problem: Need to understand level data format

Steps:
1. Start level 1
2. Open Memory Viewer
3. Scan through RAM for patterns (arrays of similar values)
4. Look for tile indices, object types, or coordinates
5. Note address range of suspected level data
6. Advance to level 2
7. Check same address range for different data
8. Compare level 1 and level 2 data to identify format
```

**Example 4: Debugging Memory Corruption**

```
Problem: Memory is being corrupted unexpectedly

Steps:
1. Identify corrupted address (e.g., 0x1050)
2. Set memory write breakpoint on 0x1050
3. Continue execution (F5)
4. Breakpoint triggers when corruption occurs
5. Check CPU State panel for PC (instruction that wrote)
6. Check Disassembly panel for instruction details
7. Analyze why that instruction is writing to wrong address
```

**Example 5: Verifying Initialization**

```
Problem: Need to verify memory is properly initialized

Steps:
1. Set breakpoint at game start (e.g., 0x0400)
2. Continue execution until breakpoint
3. Open Memory Viewer, jump to 0x1000
4. Verify all RAM is cleared (all 0x00)
5. Step through initialization code
6. Watch memory fill with initial values
7. Verify expected data structures are created
```

#### 6.3.12 Tips and Best Practices

**Tip 1: Use Pause and Step**
- Pause execution to freeze memory state
- Step through code to see exactly when memory changes
- This is more precise than watching in real-time

**Tip 2: Bookmark Important Addresses**
- Keep a list of important addresses (sprite table, score, state)
- Use Ctrl+G to quickly jump to bookmarked addresses
- Document what each address contains

**Tip 3: Look for Patterns**
- Data structures have recognizable patterns
- Arrays have regular spacing
- Tables have consistent element sizes
- Use patterns to identify data boundaries

**Tip 4: Cross-Reference with Disassembly**
- When you find interesting data, search disassembly for references
- Look for MOV instructions that read/write those addresses
- Understand how code uses the data

**Tip 5: Use Memory Breakpoints Strategically**
- Set breakpoints on critical variables (score, lives, state)
- Use breakpoints to catch unexpected writes
- Disable breakpoints when not needed to avoid slowdown

**Tip 6: Compare with Memory Dumps**
- Capture memory dump at specific point
- Compare live Memory Viewer with dump
- Identify dynamic vs static data

#### 6.3.13 Common Pitfalls

**Pitfall 1: Watching Wrong Address Range**
- RAM is at 0x1000-0x17FF, not 0x0000-0x0FFF (ROM)
- VDC registers are at 0xA0-0xAF (use VDC Register Viewer instead)
- I/O ports are at 0x20-0x27, 0x90-0x97 (read-only)

**Pitfall 2: Missing Rapid Changes**
- Memory can change multiple times per frame
- Pausing may miss intermediate values
- Use memory write breakpoints to catch all writes

**Pitfall 3: Confusing Hex and Decimal**
- Memory Viewer shows hex values (0x00-0xFF)
- Game may use decimal (0-255) or BCD (0x00-0x99)
- Always verify number format

**Pitfall 4: Assuming Static Addresses**
- Some games use dynamic memory allocation
- Data structures may move during execution
- Verify addresses are consistent across runs

**Pitfall 5: Editing Memory Without Understanding**
- Editing memory can cause crashes or unexpected behavior
- Always understand what a memory location contains before editing
- Save state before making edits for easy recovery

#### 6.3.14 Integration with Other Tools

**With VDC Register Viewer:**
- Memory Viewer shows RAM (0x1000-0x17FF)
- VDC Register Viewer shows VDC registers (0xA0-0xAF)
- Use both together for complete picture

**With Trace Analysis:**
1. Use Memory Viewer to identify interesting addresses
2. Capture trace with memory write filter
3. Search trace for writes to those addresses
4. Analyze timing and sequence of writes

**With Disassembly:**
1. Note address in Memory Viewer
2. Search disassembly for references to that address
3. Analyze code that reads/writes the address
4. Understand data flow

**With Watch Expressions:**
1. Identify important addresses in Memory Viewer
2. Create watch expressions for those addresses
3. Monitor changes without keeping Memory Viewer open
4. Get alerts when values change

#### 6.3.15 Summary

The Memory Viewer is an essential tool for interactive memory debugging. It provides real-time visibility into RAM contents, allowing you to:

- Inspect memory values at any time
- Watch memory change as code executes
- Set breakpoints on memory accesses
- Search for byte patterns and strings
- Edit memory for testing purposes

Combined with other debugging tools (traces, disassembly, VDC viewer), the Memory Viewer provides a complete picture of how games store and manipulate data in RAM. Master the Memory Viewer, and you'll debug data-related issues faster and more efficiently.

### 6.4 Memory Search Techniques

Memory search techniques help you locate specific data, patterns, or structures in RAM when you don't know their exact addresses. These techniques are essential for reverse engineering game data formats, finding variables, and understanding memory organization.

#### 6.4.1 Types of Memory Searches

**1. Exact Value Search**
- Search for specific byte value (e.g., 0x42)
- Useful when you know the exact value you're looking for
- Example: Finding score value, lives count, or state flags

**2. Pattern Search**
- Search for sequence of bytes (e.g., 0x42 0x00 0x00)
- Useful for finding data structures or signatures
- Example: Finding sprite tables, headers, or magic numbers

**3. ASCII String Search**
- Search for text strings (e.g., "SCORE", "GAME OVER")
- Useful for finding text data or debug strings
- Example: Finding level names, messages, or identifiers

**4. Range Search**
- Search for values within a range (e.g., 0x40-0x4F)
- Useful when exact value is unknown but range is known
- Example: Finding sprite positions (0-255), colors (0-7)

**5. Changed Value Search**
- Search for memory locations that changed between two states
- Useful for finding dynamic variables
- Example: Finding score, position, or state variables

**6. Unchanged Value Search**
- Search for memory locations that didn't change
- Useful for finding constants or static data
- Example: Finding level data, lookup tables, or configuration

#### 6.4.2 In-Game Debugger Search

**Basic Search (Ctrl+F):**

1. Open Memory Viewer (F12)
2. Press Ctrl+F to open Find dialog
3. Select search type:
   - **Hex:** Search for hexadecimal bytes
   - **ASCII:** Search for text strings
   - **Pattern:** Search for byte patterns
4. Enter search value
5. Press Enter to find first occurrence
6. Press F3 for next occurrence, Shift+F3 for previous

**Hex Search Example:**

```
Search for: 42
Finds: All bytes with value 0x42

Search for: 42 00 00
Finds: Sequence of bytes 0x42 0x00 0x00
```

**ASCII Search Example:**

```
Search for: SCORE
Finds: ASCII string "SCORE" (0x53 0x43 0x4F 0x52 0x45)

Search for: GAME OVER
Finds: ASCII string "GAME OVER"
```

**Search Options:**
- **Case Sensitive:** Match exact case for ASCII searches
- **Whole Words:** Match complete words only (ASCII)
- **Wrap Around:** Continue from beginning when reaching end
- **Search Range:** Limit search to specific address range

#### 6.4.3 Command-Line Memory Search

**Using grep with hexdump:**

```bash
# Capture memory dump
videopac --memory-dump --dump-output mem.bin bios.bin game.bin

# Search for specific byte value (0x42)
hexdump -C mem.bin | grep " 42 "

# Search for byte pattern (0x42 0x00 0x00)
hexdump -C mem.bin | grep "42 00 00"

# Search for ASCII string
hexdump -C mem.bin | grep "SCORE"
```

**Using strings command:**

```bash
# Extract all ASCII strings from memory dump
strings mem.bin

# Search for specific string
strings mem.bin | grep -i "score"

# Show strings with addresses
strings -t x mem.bin | grep -i "game"
```

**Using custom search script:**

```python
# mem_search.py
import sys

def search_bytes(filename, pattern):
    """Search for byte pattern in memory dump."""
    with open(filename, 'rb') as f:
        data = f.read()
    
    # Convert pattern string to bytes
    pattern_bytes = bytes.fromhex(pattern.replace(' ', ''))
    
    # Search for pattern
    matches = []
    offset = 0
    while True:
        offset = data.find(pattern_bytes, offset)
        if offset == -1:
            break
        matches.append(offset)
        offset += 1
    
    # Report matches
    print(f"Found {len(matches)} matches for pattern: {pattern}")
    for addr in matches:
        print(f"  0x{addr+0x1000:04X}: {pattern}")
        # Show context (8 bytes before and after)
        start = max(0, addr-8)
        end = min(len(data), addr+len(pattern_bytes)+8)
        context = data[start:end]
        print(f"    Context: {context.hex(' ')}")

if __name__ == "__main__":
    search_bytes(sys.argv[1], sys.argv[2])
```

Usage:
```bash
python mem_search.py mem.bin "42 00 00"
```

#### 6.4.4 Changed Value Search Technique

This technique helps find variables by comparing memory before and after a known change.

**Manual Process:**

1. **Capture initial state:**
   ```bash
   videopac --memory-dump --dump-frame 100 --dump-output mem_before.bin bios.bin game.bin
   ```

2. **Perform action that changes variable (e.g., score a point)**

3. **Capture new state:**
   ```bash
   videopac --memory-dump --dump-frame 110 --dump-output mem_after.bin bios.bin game.bin
   ```

4. **Compare dumps:**
   ```bash
   python mem_diff.py mem_before.bin mem_after.bin
   ```

5. **Analyze differences:**
   - Look for addresses where value increased (score, counter)
   - Look for addresses where value changed (state, flags)
   - Ignore addresses with random changes (temporary variables)

**Automated Changed Value Search:**

```python
# changed_value_search.py
import sys

def find_changed_values(file1, file2, min_change=1, max_change=100):
    """Find memory locations where value changed by specific amount."""
    with open(file1, 'rb') as f1, open(file2, 'rb') as f2:
        data1 = f1.read()
        data2 = f2.read()
    
    matches = []
    for i in range(min(len(data1), len(data2))):
        diff = data2[i] - data1[i]
        if min_change <= abs(diff) <= max_change:
            matches.append((i, data1[i], data2[i], diff))
    
    print(f"Found {len(matches)} changed values:")
    for addr, val1, val2, diff in matches[:20]:
        print(f"  0x{addr+0x1000:04X}: 0x{val1:02X} -> 0x{val2:02X} (diff: {diff:+d})")

if __name__ == "__main__":
    find_changed_values(sys.argv[1], sys.argv[2])
```

Usage:
```bash
# Find values that changed by 1-100
python changed_value_search.py mem_before.bin mem_after.bin
```

#### 6.4.5 Iterative Search Technique

For finding variables with unknown initial values, use iterative search:

**Process:**

1. **Capture initial dump:**
   ```bash
   videopac --memory-dump --dump-output mem_0.bin bios.bin game.bin
   ```

2. **Perform action (e.g., move sprite right)**

3. **Capture new dump:**
   ```bash
   videopac --memory-dump --dump-output mem_1.bin bios.bin game.bin
   ```

4. **Find increased values:**
   ```bash
   python changed_value_search.py mem_0.bin mem_1.bin
   ```

5. **Perform opposite action (e.g., move sprite left)**

6. **Capture another dump:**
   ```bash
   videopac --memory-dump --dump-output mem_2.bin bios.bin game.bin
   ```

7. **Find decreased values:**
   ```bash
   python changed_value_search.py mem_1.bin mem_2.bin
   ```

8. **Find addresses that appear in both searches:**
   - These are likely the variable you're looking for
   - Verify by repeating the process

#### 6.4.6 Pattern Recognition Techniques

**Identifying Sprite Tables:**

Sprite tables typically have this pattern:
```
X_pos (0-255), Y_pos (0-255), Color (0-7), Pattern (0-255), [flags]
```

Search strategy:
1. Look for sequences of 4-8 bytes
2. First two bytes should be in range 0-255 (positions)
3. Third byte should be in range 0-7 (color)
4. Pattern repeats every 4-8 bytes (for each sprite)

**Example Search:**

```python
# find_sprite_table.py
def find_sprite_tables(filename):
    """Find potential sprite tables in memory."""
    with open(filename, 'rb') as f:
        data = f.read()
    
    # Look for pattern: X (0-255), Y (0-255), Color (0-7), Pattern (0-255)
    for i in range(len(data) - 32):  # Check 32 bytes (4 sprites * 8 bytes)
        # Check if this looks like a sprite table
        valid = True
        for j in range(4):  # Check 4 sprites
            offset = i + j * 8
            x = data[offset]
            y = data[offset + 1]
            color = data[offset + 2]
            
            # Validate ranges
            if color > 7:  # Color must be 0-7
                valid = False
                break
        
        if valid:
            print(f"Potential sprite table at 0x{i+0x1000:04X}")
            # Show the data
            for j in range(4):
                offset = i + j * 8
                sprite_data = data[offset:offset+8]
                print(f"  Sprite {j}: {sprite_data.hex(' ')}")
```

**Identifying Arrays:**

Arrays have regular spacing and similar value ranges:

```python
# find_arrays.py
def find_arrays(filename, element_size=1, min_length=8):
    """Find potential arrays in memory."""
    with open(filename, 'rb') as f:
        data = f.read()
    
    # Look for sequences of similar values
    for i in range(len(data) - min_length * element_size):
        values = []
        for j in range(min_length):
            offset = i + j * element_size
            if element_size == 1:
                values.append(data[offset])
            elif element_size == 2:
                values.append(data[offset] | (data[offset+1] << 8))
        
        # Check if values are in similar range
        if len(set(values)) > 1:  # Not all same value
            min_val = min(values)
            max_val = max(values)
            if max_val - min_val < 64:  # Similar range
                print(f"Potential array at 0x{i+0x1000:04X}")
                print(f"  Values: {values[:8]}")
```

#### 6.4.7 Practical Search Examples

**Example 1: Finding Score Variable**

```
Problem: Need to find where score is stored

Steps:
1. Note current score: 0 points
2. Capture memory: videopac --memory-dump --dump-output score_0.bin bios.bin game.bin
3. Score 10 points in game
4. Capture memory: videopac --memory-dump --dump-output score_10.bin bios.bin game.bin
5. Search for changed values: python changed_value_search.py score_0.bin score_10.bin
6. Look for address where value increased by 10 (or 0x0A in hex)
7. Verify by scoring more points and checking if value increases
```

**Example 2: Finding Sprite Position**

```
Problem: Need to find sprite X position variable

Steps:
1. Note sprite X position: 100
2. Capture memory: videopac --memory-dump --dump-output pos_100.bin bios.bin game.bin
3. Move sprite right to X=110
4. Capture memory: videopac --memory-dump --dump-output pos_110.bin bios.bin game.bin
5. Search for value that increased by 10
6. Move sprite left to X=90
7. Capture memory: videopac --memory-dump --dump-output pos_90.bin bios.bin game.bin
8. Verify same address decreased by 20
```

**Example 3: Finding Text Strings**

```
Problem: Need to find where "GAME OVER" text is stored

Steps:
1. Capture memory: videopac --memory-dump --dump-output mem.bin bios.bin game.bin
2. Search for ASCII string: strings mem.bin | grep "GAME OVER"
3. Or use hexdump: hexdump -C mem.bin | grep "GAME OVER"
4. Note the address where string is found
5. Verify in Memory Viewer or disassembly
```

**Example 4: Finding Sprite Table**

```
Problem: Need to locate sprite data structure

Steps:
1. Open Memory Viewer in debugger
2. Jump to start of RAM (0x1000)
3. Look for pattern: X, Y, Color, Pattern (4 bytes per sprite)
4. Move sprite in game
5. Watch for changing X/Y values
6. Verify pattern repeats for each sprite (4 sprites = 16 bytes)
7. Set memory breakpoint on sprite table to see when it's updated
```

**Example 5: Finding Level Data**

```
Problem: Need to understand level data format

Steps:
1. Start level 1
2. Capture memory: videopac --memory-dump --dump-output level1.bin bios.bin game.bin
3. Start level 2
4. Capture memory: videopac --memory-dump --dump-output level2.bin bios.bin game.bin
5. Compare dumps: python mem_diff.py level1.bin level2.bin
6. Look for large blocks of changed data (level data)
7. Analyze patterns in the data (tile indices, object types, etc.)
```

#### 6.4.8 Advanced Search Techniques

**Wildcard Pattern Search:**

Search for patterns with unknown bytes:

```python
# wildcard_search.py
def wildcard_search(filename, pattern):
    """Search for pattern with wildcards (? = any byte)."""
    with open(filename, 'rb') as f:
        data = f.read()
    
    # Parse pattern: "42 ? ? 00" -> [0x42, None, None, 0x00]
    pattern_bytes = []
    for byte_str in pattern.split():
        if byte_str == '?':
            pattern_bytes.append(None)
        else:
            pattern_bytes.append(int(byte_str, 16))
    
    # Search
    matches = []
    for i in range(len(data) - len(pattern_bytes)):
        match = True
        for j, expected in enumerate(pattern_bytes):
            if expected is not None and data[i+j] != expected:
                match = False
                break
        if match:
            matches.append(i)
    
    print(f"Found {len(matches)} matches")
    for addr in matches[:10]:
        context = data[addr:addr+len(pattern_bytes)]
        print(f"  0x{addr+0x1000:04X}: {context.hex(' ')}")
```

Usage:
```bash
# Find pattern: 0x42 followed by any two bytes, then 0x00
python wildcard_search.py mem.bin "42 ? ? 00"
```

**Relative Value Search:**

Find values relative to each other:

```python
# relative_search.py
def find_relative_values(filename, offset, expected_diff):
    """Find pairs of values with specific difference."""
    with open(filename, 'rb') as f:
        data = f.read()
    
    matches = []
    for i in range(len(data) - offset):
        val1 = data[i]
        val2 = data[i + offset]
        if val2 - val1 == expected_diff:
            matches.append((i, val1, val2))
    
    print(f"Found {len(matches)} matches")
    for addr, val1, val2 in matches[:10]:
        print(f"  0x{addr+0x1000:04X}: {val1} and 0x{addr+offset+0x1000:04X}: {val2}")
```

Usage:
```bash
# Find pairs of values 8 bytes apart with difference of 10
python relative_search.py mem.bin 8 10
```

#### 6.4.9 Tips and Best Practices

**Tip 1: Start with Known Values**
- If you know the score is 100, search for 0x64 (100 in hex)
- If you know sprite X is 50, search for 0x32
- Known values are easier to find than unknown values

**Tip 2: Use Multiple Searches**
- One search may return many false positives
- Repeat search after changing value again
- Only addresses that appear in all searches are likely correct

**Tip 3: Verify with Memory Viewer**
- After finding address with search, verify in Memory Viewer
- Watch the address change in real-time
- Set breakpoint to see when it's written

**Tip 4: Look for Patterns**
- Data structures have recognizable patterns
- Sprite tables, arrays, and buffers have regular spacing
- Use pattern recognition to narrow search

**Tip 5: Document Findings**
- Keep a list of found addresses and their purposes
- Document data structure formats
- Create memory map for future reference

**Tip 6: Use Appropriate Search Type**
- Exact value: When you know the value
- Changed value: When you can trigger a change
- Pattern: When you know the structure
- ASCII: When looking for text

#### 6.4.10 Common Pitfalls

**Pitfall 1: Too Many Results**
- Searching for common values (0x00, 0xFF) returns many results
- Use more specific patterns or multiple searches
- Narrow search range to likely addresses

**Pitfall 2: Missing Results**
- Value may be stored in different format (BCD, packed, etc.)
- Value may be split across multiple bytes
- Value may be in external RAM (0x1400-0x17FF) if not searched

**Pitfall 3: False Positives**
- Temporary variables may match search criteria
- Verify found addresses are consistent across multiple searches
- Use memory breakpoints to confirm usage

**Pitfall 4: Timing Issues**
- Memory dumps capture specific moment in time
- Value may change between dumps
- Use consistent timing (same frame number, same game state)

**Pitfall 5: Endianness Confusion**
- Multi-byte values are big-endian (high byte first)
- 0x1234 is stored as 0x12 0x34, not 0x34 0x12
- Search for correct byte order

#### 6.4.11 Integration with Other Tools

**With Memory Viewer:**
1. Use search to find candidate addresses
2. Open Memory Viewer and jump to each address
3. Watch addresses in real-time to verify
4. Set breakpoints on confirmed addresses

**With Trace Analysis:**
1. Find address with search
2. Capture trace with memory write filter
3. Search trace for writes to that address
4. Analyze code that accesses the address

**With Disassembly:**
1. Find address with search
2. Search disassembly for references to that address
3. Analyze code that reads/writes the address
4. Understand data usage and format

**With Watch Expressions:**
1. Find addresses with search
2. Create watch expressions for those addresses
3. Monitor changes without manual inspection
4. Get alerts when values change

#### 6.4.12 Summary

Memory search techniques are essential for locating data in RAM when addresses are unknown. Key techniques include:

- **Exact value search:** Find specific byte values
- **Pattern search:** Find byte sequences
- **Changed value search:** Find variables that change
- **Iterative search:** Narrow down candidates through multiple searches
- **Pattern recognition:** Identify data structures by their format

Combined with Memory Viewer, traces, and disassembly, memory search provides a powerful way to reverse engineer game data formats and understand memory organization. Master these techniques, and you'll quickly locate any data you need for debugging.

### 6.5 Tracking Memory Writes

Tracking memory writes in execution traces allows you to see exactly when, where, and how memory is modified during program execution. This is essential for understanding data flow, debugging corruption issues, and analyzing game logic.

#### 6.5.1 Why Track Memory Writes

Memory write tracking is useful for:

- **Understanding Data Flow:** See how data moves from ROM to RAM to VDC
- **Debugging Corruption:** Identify unexpected writes to memory
- **Analyzing Initialization:** Verify memory is initialized correctly
- **Tracking State Changes:** See when game state variables are modified
- **Reverse Engineering:** Understand how games store and manipulate data
- **Performance Analysis:** Identify excessive memory writes

#### 6.5.2 Capturing Memory Write Traces

**Basic Memory Write Trace:**

```bash
# Capture all memory writes for one frame
videopac --trace --trace-filter mem-write --trace-frames 1 --trace-output mem_writes.txt bios.bin game.bin

# Capture memory writes starting at specific frame
videopac --trace --trace-filter mem-write --trace-start-frame 100 --trace-frames 1 --trace-output mem_writes_frame100.txt bios.bin game.bin

# Capture memory writes after key press
videopac --trace --trace-filter mem-write --trace-start-key "1" --trace-frames 1 --trace-output mem_writes_key.txt bios.bin game.bin
```

**Filtering by Address Range:**

```bash
# Capture writes to specific address range (e.g., 0x1000-0x10FF)
videopac --trace --trace-filter mem-write --trace-address-range 0x1000:0x10FF --trace-frames 1 --trace-output mem_writes_range.txt bios.bin game.bin

# Capture writes to sprite table (e.g., 0x1000-0x101F)
videopac --trace --trace-filter mem-write --trace-address-range 0x1000:0x101F --trace-frames 1 --trace-output sprite_writes.txt bios.bin game.bin
```

#### 6.5.3 Memory Write Trace Format

**Standard Format:**

```
Frame:0001 Scanline:010 Cycle:00123 PC:0x0234 | MEM_WRITE[0x1000] = 0x42 (A=0x42)
Frame:0001 Scanline:010 Cycle:00125 PC:0x0236 | MEM_WRITE[0x1001] = 0x00 (A=0x00)
Frame:0001 Scanline:010 Cycle:00127 PC:0x0238 | MEM_WRITE[0x1002] = 0xFF (A=0xFF)
```

**Columns:**
- **Frame:** Current video frame number
- **Scanline:** Current scanline (0-261)
- **Cycle:** CPU cycle within frame
- **PC:** Program Counter (address of instruction that wrote)
- **Address:** Memory address written to (0x1000-0x17FF for RAM)
- **Value:** Byte value written
- **Source:** Source of value (usually accumulator A)

#### 6.5.4 Analyzing Memory Write Traces

**Identifying Write Patterns:**

**Sequential Writes (Array Initialization):**
```
MEM_WRITE[0x1000] = 0x00
MEM_WRITE[0x1001] = 0x00
MEM_WRITE[0x1002] = 0x00
MEM_WRITE[0x1003] = 0x00
```
This pattern indicates array or buffer initialization (clearing memory).

**Repeated Writes (Loop):**
```
MEM_WRITE[0x1000] = 0x42  (Frame 1)
MEM_WRITE[0x1000] = 0x43  (Frame 2)
MEM_WRITE[0x1000] = 0x44  (Frame 3)
```
This pattern indicates a variable being updated each frame (counter, position, etc.).

**Sparse Writes (Structure Update):**
```
MEM_WRITE[0x1000] = 0x64  (X position)
MEM_WRITE[0x1001] = 0x80  (Y position)
MEM_WRITE[0x1002] = 0x04  (Color)
MEM_WRITE[0x1003] = 0x12  (Pattern)
```
This pattern indicates a data structure being updated (sprite data, game state, etc.).

#### 6.5.5 Correlating Writes with Code

**Finding the Writing Instruction:**

1. **Note the PC value** from the trace line:
   ```
   Frame:0001 Scanline:010 Cycle:00123 PC:0x0234 | MEM_WRITE[0x1000] = 0x42
   ```

2. **Look up PC in disassembly:**
   ```bash
   grep "0x0234" game_disasm.txt
   ```

3. **Analyze the instruction:**
   ```assembly
   0x0234   F0        MOV       @R0, A      ; Write A to memory at address in R0
   ```

4. **Understand the context:**
   - What value is in A? (shown in trace: A=0x42)
   - What address is in R0? (calculated: 0x1000)
   - Why is this write happening? (analyze surrounding code)

**Example Analysis:**

```
Trace line:
Frame:0001 Scanline:010 Cycle:00123 PC:0x0234 | MEM_WRITE[0x1000] = 0x42 (A=0x42)

Disassembly:
0x0230   23        MOV       A, #0x42    ; Load 0x42 into A
0x0232   90 00     MOV       R0, #0x00   ; Load 0x00 into R0 (low byte of address)
0x0234   F0        MOV       @R0, A      ; Write A to memory at R0 (0x1000 + 0x00 = 0x1000)

Conclusion:
Code is initializing memory at 0x1000 with value 0x42.
```

#### 6.5.6 Tracking Specific Memory Locations

**Filtering Trace for Specific Address:**

```bash
# Capture full trace
videopac --trace --trace-frames 10 --trace-output full_trace.txt bios.bin game.bin

# Filter for writes to specific address (0x1020)
grep "MEM_WRITE\[0x1020\]" full_trace.txt

# Output:
# Frame:0001 Scanline:010 PC:0x0234 | MEM_WRITE[0x1020] = 0x00
# Frame:0002 Scanline:010 PC:0x0234 | MEM_WRITE[0x1020] = 0x01
# Frame:0003 Scanline:010 PC:0x0234 | MEM_WRITE[0x1020] = 0x02
```

**Analyzing Write Frequency:**

```bash
# Count writes to each address
grep "MEM_WRITE" full_trace.txt | cut -d'[' -f2 | cut -d']' -f1 | sort | uniq -c | sort -rn | head -20

# Output:
#  100 0x1020
#   50 0x1021
#   25 0x1000
#   10 0x1050
```

This shows which addresses are written most frequently.

#### 6.5.7 Identifying Memory Corruption

**Detecting Unexpected Writes:**

1. **Identify protected memory region** (e.g., sprite table at 0x1000-0x101F)

2. **Capture trace with memory write filter:**
   ```bash
   videopac --trace --trace-filter mem-write --trace-address-range 0x1000:0x101F --trace-frames 10 --trace-output sprite_writes.txt bios.bin game.bin
   ```

3. **Analyze writes:**
   - Expected writes: Sprite update code during VBLANK
   - Unexpected writes: Random writes from buggy code

4. **Investigate unexpected writes:**
   - Note PC value of unexpected write
   - Look up instruction in disassembly
   - Trace back to find root cause

**Example:**

```
Expected write (during VBLANK):
Frame:0001 Scanline:005 PC:0x0400 | MEM_WRITE[0x1000] = 0x64  (sprite X update)

Unexpected write (mid-frame):
Frame:0001 Scanline:100 PC:0x0800 | MEM_WRITE[0x1000] = 0xFF  (corruption!)

Investigation:
1. Check disassembly at 0x0800
2. Find why this code is writing to sprite table
3. Verify address calculation is correct
4. Fix bug in address calculation or bounds checking
```

#### 6.5.8 Analyzing Initialization Sequences

**Tracking Memory Initialization:**

```bash
# Capture first frame (initialization)
videopac --trace --trace-filter mem-write --trace-start-frame 0 --trace-frames 1 --trace-output init_writes.txt bios.bin game.bin

# Analyze initialization pattern
cat init_writes.txt
```

**Common Initialization Patterns:**

**Pattern 1: Clear RAM**
```
MEM_WRITE[0x1000] = 0x00
MEM_WRITE[0x1001] = 0x00
MEM_WRITE[0x1002] = 0x00
...
MEM_WRITE[0x13FF] = 0x00
```
This clears all internal RAM (1KB).

**Pattern 2: Initialize Data Structures**
```
MEM_WRITE[0x1000] = 0x80  (sprite 0 X = 128)
MEM_WRITE[0x1001] = 0x60  (sprite 0 Y = 96)
MEM_WRITE[0x1002] = 0x04  (sprite 0 color = red)
MEM_WRITE[0x1003] = 0x00  (sprite 0 pattern = 0)
```
This initializes sprite data structure.

**Pattern 3: Copy Data from ROM**
```
MEM_WRITE[0x1100] = 0x42  (copy from ROM 0x0500)
MEM_WRITE[0x1101] = 0x43  (copy from ROM 0x0501)
MEM_WRITE[0x1102] = 0x44  (copy from ROM 0x0502)
```
This copies lookup table or level data from ROM to RAM.

#### 6.5.9 Tracking State Changes

**Monitoring Game State Variable:**

```bash
# Capture writes to suspected state variable (e.g., 0x1050)
videopac --trace --trace-filter mem-write --trace-address-range 0x1050:0x1050 --trace-frames 60 --trace-output state_writes.txt bios.bin game.bin

# Analyze state transitions
grep "MEM_WRITE\[0x1050\]" state_writes.txt
```

**Example Output:**

```
Frame:0001 PC:0x0400 | MEM_WRITE[0x1050] = 0x00  (state: INIT)
Frame:0010 PC:0x0420 | MEM_WRITE[0x1050] = 0x01  (state: MENU)
Frame:0050 PC:0x0440 | MEM_WRITE[0x1050] = 0x02  (state: PLAYING)
Frame:0100 PC:0x0460 | MEM_WRITE[0x1050] = 0x03  (state: GAME_OVER)
```

**Analyzing State Transitions:**

1. **Identify state values:** 0x00=INIT, 0x01=MENU, 0x02=PLAYING, 0x03=GAME_OVER
2. **Note transition frames:** State changes at frames 1, 10, 50, 100
3. **Find transition code:** Look up PC values in disassembly
4. **Understand triggers:** What causes each state transition?

#### 6.5.10 Practical Examples

**Example 1: Finding Score Update Code**

```bash
# Capture writes to score variable (found at 0x1020)
videopac --trace --trace-filter mem-write --trace-address-range 0x1020:0x1020 --trace-frames 10 --trace-output score_writes.txt bios.bin game.bin

# Analyze writes
cat score_writes.txt

# Output:
# Frame:0005 PC:0x0600 | MEM_WRITE[0x1020] = 0x0A  (score += 10)
# Frame:0008 PC:0x0600 | MEM_WRITE[0x1020] = 0x14  (score += 10)

# Look up PC 0x0600 in disassembly
grep "0x0600" game_disasm.txt

# Find score update function
```

**Example 2: Debugging Sprite Position Bug**

```bash
# Capture writes to sprite X position (0x1000)
videopac --trace --trace-filter mem-write --trace-address-range 0x1000:0x1000 --trace-frames 5 --trace-output sprite_x_writes.txt bios.bin game.bin

# Analyze writes
cat sprite_x_writes.txt

# Look for unexpected values or timing
# Example: Sprite X written mid-frame instead of during VBLANK
# Frame:0001 Scanline:100 PC:0x0800 | MEM_WRITE[0x1000] = 0xFF  (BUG!)

# Investigate PC 0x0800 to find buggy code
```

**Example 3: Tracking Memory Corruption**

```bash
# Capture all writes to protected region (0x1000-0x101F)
videopac --trace --trace-filter mem-write --trace-address-range 0x1000:0x101F --trace-frames 10 --trace-output protected_writes.txt bios.bin game.bin

# Look for writes from unexpected code
grep "MEM_WRITE" protected_writes.txt | grep -v "PC:0x0400\|PC:0x0420"  # Filter out known good writes

# Investigate any remaining writes
```

**Example 4: Analyzing Initialization**

```bash
# Capture first frame initialization
videopac --trace --trace-filter mem-write --trace-start-frame 0 --trace-frames 1 --trace-output init.txt bios.bin game.bin

# Count writes to each address
grep "MEM_WRITE" init.txt | cut -d'[' -f2 | cut -d']' -f1 | sort | uniq -c

# Identify which memory regions are initialized
# Verify all required data structures are set up
```

**Example 5: Comparing Write Patterns**

```bash
# Capture writes for level 1
videopac --trace --trace-filter mem-write --trace-start-frame 10 --trace-frames 1 --trace-output level1_writes.txt bios.bin game.bin

# Capture writes for level 2
videopac --trace --trace-filter mem-write --trace-start-frame 100 --trace-frames 1 --trace-output level2_writes.txt bios.bin game.bin

# Compare write patterns
diff level1_writes.txt level2_writes.txt

# Identify level-specific data
```

#### 6.5.11 Automated Analysis Scripts

**Script 1: Memory Write Summary**

```python
# mem_write_summary.py
import sys
from collections import defaultdict

def analyze_writes(trace_file):
    """Analyze memory write patterns in trace."""
    writes_by_addr = defaultdict(list)
    writes_by_pc = defaultdict(int)
    
    with open(trace_file, 'r') as f:
        for line in f:
            if 'MEM_WRITE' in line:
                # Parse: PC:0x0234 | MEM_WRITE[0x1000] = 0x42
                parts = line.split('PC:')
                if len(parts) > 1:
                    pc = parts[1].split()[0]
                    
                    mem_parts = line.split('MEM_WRITE[')
                    if len(mem_parts) > 1:
                        addr = mem_parts[1].split(']')[0]
                        value = mem_parts[1].split('=')[1].strip().split()[0]
                        
                        writes_by_addr[addr].append((pc, value))
                        writes_by_pc[pc] += 1
    
    # Report summary
    print("Memory Write Summary")
    print("=" * 50)
    print(f"\nTotal addresses written: {len(writes_by_addr)}")
    print(f"Total unique PCs: {len(writes_by_pc)}")
    
    print("\nTop 10 most written addresses:")
    sorted_addrs = sorted(writes_by_addr.items(), key=lambda x: len(x[1]), reverse=True)
    for addr, writes in sorted_addrs[:10]:
        print(f"  {addr}: {len(writes)} writes")
    
    print("\nTop 10 most active PCs:")
    sorted_pcs = sorted(writes_by_pc.items(), key=lambda x: x[1], reverse=True)
    for pc, count in sorted_pcs[:10]:
        print(f"  {pc}: {count} writes")

if __name__ == "__main__":
    analyze_writes(sys.argv[1])
```

Usage:
```bash
python mem_write_summary.py mem_writes.txt
```

**Script 2: Detect Unexpected Writes**

```python
# detect_unexpected_writes.py
import sys

def detect_unexpected(trace_file, expected_pcs):
    """Detect writes from unexpected code locations."""
    expected_set = set(expected_pcs)
    unexpected = []
    
    with open(trace_file, 'r') as f:
        for line in f:
            if 'MEM_WRITE' in line:
                parts = line.split('PC:')
                if len(parts) > 1:
                    pc = parts[1].split()[0]
                    if pc not in expected_set:
                        unexpected.append(line.strip())
    
    print(f"Found {len(unexpected)} unexpected writes:")
    for line in unexpected[:20]:
        print(f"  {line}")

if __name__ == "__main__":
    # Expected PCs for sprite update code
    expected = ['0x0400', '0x0420', '0x0440']
    detect_unexpected(sys.argv[1], expected)
```

Usage:
```bash
python detect_unexpected_writes.py sprite_writes.txt
```

#### 6.5.12 Tips and Best Practices

**Tip 1: Filter Aggressively**
- Memory write traces can be large (thousands of writes per frame)
- Use address range filtering to focus on specific regions
- Filter by frame to capture specific moments

**Tip 2: Correlate with Disassembly**
- Always look up PC values in disassembly
- Understand what instruction is writing
- Analyze surrounding code for context

**Tip 3: Look for Patterns**
- Sequential writes indicate loops or initialization
- Repeated writes indicate updates or counters
- Sparse writes indicate structure updates

**Tip 4: Track Timing**
- Note scanline values for timing analysis
- VBLANK writes (scanline 0-20) are normal
- Mid-frame writes (scanline 21-192) may cause issues

**Tip 5: Compare Across Frames**
- Capture multiple frames to see patterns
- Compare initialization vs steady-state
- Identify frame-specific vs persistent writes

**Tip 6: Use Automated Analysis**
- Write scripts to summarize large traces
- Automate detection of unexpected patterns
- Generate reports for review

#### 6.5.13 Common Pitfalls

**Pitfall 1: Too Much Data**
- Full memory write traces are very large
- Use filtering to reduce data volume
- Focus on specific addresses or time periods

**Pitfall 2: Missing Context**
- Memory write alone doesn't show why
- Always correlate with disassembly
- Understand the code that's writing

**Pitfall 3: Ignoring Timing**
- When a write happens matters
- VBLANK vs mid-frame makes a difference
- Check scanline values in trace

**Pitfall 4: Assuming Single Writer**
- Multiple code paths may write same address
- Check all PC values that write to address
- Understand all writers before making changes

**Pitfall 5: Not Verifying Addresses**
- Address calculation may be complex
- Verify calculated address matches trace
- Check for off-by-one errors

#### 6.5.14 Integration with Other Tools

**With Memory Viewer:**
1. Track writes in trace to find interesting addresses
2. Open Memory Viewer and jump to those addresses
3. Set memory breakpoints to catch writes in real-time
4. Verify trace findings with live debugging

**With Disassembly:**
1. Note PC values from memory write trace
2. Look up those PCs in disassembly
3. Analyze instructions and surrounding code
4. Understand data flow and logic

**With Memory Dumps:**
1. Capture memory dump before and after writes
2. Compare dumps to verify trace accuracy
3. Understand cumulative effect of writes
4. Identify final state after all writes

**With VDC Traces:**
1. Correlate memory writes with VDC writes
2. Understand data flow from RAM to VDC
3. Verify sprite data is written to RAM then VDC
4. Identify timing relationships

#### 6.5.15 Summary

Tracking memory writes in traces provides detailed insight into how programs modify RAM. Key benefits include:

- **Understanding data flow:** See how data moves through memory
- **Debugging corruption:** Identify unexpected writes
- **Analyzing initialization:** Verify proper setup
- **Tracking state changes:** Monitor game state variables
- **Performance analysis:** Identify excessive writes

Combined with disassembly, Memory Viewer, and memory dumps, memory write tracking provides a complete picture of memory usage and modification patterns. Master this technique, and you'll quickly identify and fix memory-related bugs.

### 6.6 Data Structure Identification

Identifying data structures in memory is essential for understanding how games organize and manipulate data. This section covers techniques for recognizing common patterns, reverse engineering data formats, and documenting data structures for future reference.

#### 6.6.1 Common Data Structures in Videopac Games

**1. Sprite Tables**
- **Purpose:** Store sprite position, color, and pattern data
- **Typical Size:** 4-8 bytes per sprite, 4 sprites total = 16-32 bytes
- **Common Location:** Start of RAM (0x1000-0x101F)
- **Format:** X, Y, Color, Pattern, [Flags, Size, etc.]

**2. Game State Variables**
- **Purpose:** Store current game state (menu, playing, paused, game over)
- **Typical Size:** 1-2 bytes
- **Common Location:** After sprite table (0x1020-0x1030)
- **Format:** Single byte enum or bit flags

**3. Score and Lives**
- **Purpose:** Store player score and remaining lives
- **Typical Size:** 1-4 bytes (BCD or binary)
- **Common Location:** Near game state (0x1030-0x1040)
- **Format:** BCD (0x00-0x99 per byte) or binary (0x00-0xFF)

**4. Level Data**
- **Purpose:** Store current level layout, tiles, or objects
- **Typical Size:** 16-256 bytes depending on complexity
- **Common Location:** Mid-RAM (0x1040-0x1100)
- **Format:** Array of tile indices, object types, or compressed data

**5. Input State**
- **Purpose:** Store current and previous input state
- **Typical Size:** 1-4 bytes
- **Common Location:** Near game state (0x1030-0x1040)
- **Format:** Bit flags for each button/direction

**6. Timers and Counters**
- **Purpose:** Store frame counters, animation timers, etc.
- **Typical Size:** 1-2 bytes per timer
- **Common Location:** Scattered throughout RAM
- **Format:** Binary counter (0x00-0xFF or 0x0000-0xFFFF)

#### 6.6.2 Identifying Sprite Tables

**Characteristics:**
- Regular spacing (4-8 bytes per sprite)
- X and Y values in range 0-255
- Color values in range 0-7
- Pattern values in range 0-255
- Typically 4 sprites (Videopac has 4 hardware sprites)

**Identification Process:**

1. **Capture memory dump during gameplay:**
   ```bash
   videopac --memory-dump --dump-frame 50 --dump-output mem_gameplay.bin bios.bin game.bin
   ```

2. **Search for sprite-like patterns:**
   ```bash
   hexdump -C mem_gameplay.bin | head -20
   ```

3. **Look for this pattern:**
   ```
   Address   Data                                        Interpretation
   00000000  64 80 04 12 00 00 00 00  C0 60 01 34 00 00 00 00  Sprite 0 & 1
   00000010  50 A0 02 56 00 00 00 00  78 70 03 78 00 00 00 00  Sprite 2 & 3
   ```

4. **Verify by checking values:**
   - Byte 0: X position (0x64 = 100) ✓
   - Byte 1: Y position (0x80 = 128) ✓
   - Byte 2: Color (0x04 = red, valid 0-7) ✓
   - Byte 3: Pattern (0x12 = 18) ✓

5. **Confirm with Memory Viewer:**
   - Open Memory Viewer, jump to 0x1000
   - Move sprite in game
   - Watch for X/Y values changing

**Example Sprite Table Format:**

```
Offset  Field       Size  Description
------  ----------  ----  -----------
0x00    Sprite 0 X  1     X position (0-255)
0x01    Sprite 0 Y  1     Y position (0-255)
0x02    Sprite 0 C  1     Color (0-7)
0x03    Sprite 0 P  1     Pattern index (0-255)
0x04    Sprite 0 F  1     Flags (visible, size, etc.)
0x05-07 Reserved    3     Padding or additional data
0x08    Sprite 1 X  1     X position
...
```

#### 6.6.3 Identifying Score Variables

**Characteristics:**
- Increases when player scores points
- May be BCD (Binary-Coded Decimal) or binary
- Typically 1-4 bytes
- Often near game state variables

**Identification Process:**

1. **Note current score:** 0 points

2. **Capture memory dump:**
   ```bash
   videopac --memory-dump --dump-output score_0.bin bios.bin game.bin
   ```

3. **Score points in game:** 10 points

4. **Capture new memory dump:**
   ```bash
   videopac --memory-dump --dump-output score_10.bin bios.bin game.bin
   ```

5. **Find changed values:**
   ```bash
   python mem_diff.py score_0.bin score_10.bin
   ```

6. **Look for value that increased by 10 (or 0x0A):**
   ```
   0x1020: 0x00 -> 0x0A  (binary format)
   or
   0x1020: 0x00 -> 0x10  (BCD format, 10 decimal)
   ```

7. **Verify by scoring more points**

**BCD vs Binary:**

**Binary Format:**
- Each byte represents 0-255
- Example: 0x64 = 100 decimal
- Arithmetic: Standard binary addition

**BCD Format:**
- Each nibble represents 0-9
- Example: 0x64 = 64 decimal (6 tens + 4 ones)
- Arithmetic: Special BCD addition (adjust for carries)

**Detecting BCD:**
- Values never exceed 0x99
- Each nibble is 0-9 (never A-F)
- Increases by 0x01, 0x10, 0x100 (not 0x01, 0x0A, 0x64)

#### 6.6.4 Identifying Game State Variables

**Characteristics:**
- Small number of distinct values (enum)
- Changes at specific game events (start, pause, game over)
- Typically 1 byte
- Often controls program flow

**Identification Process:**

1. **Observe game states:**
   - State 0: Title screen
   - State 1: Playing
   - State 2: Paused
   - State 3: Game over

2. **Capture memory at each state:**
   ```bash
   # Title screen
   videopac --memory-dump --dump-output state_title.bin bios.bin game.bin
   
   # Playing
   videopac --memory-dump --dump-output state_playing.bin bios.bin game.bin
   
   # Game over
   videopac --memory-dump --dump-output state_gameover.bin bios.bin game.bin
   ```

3. **Compare dumps:**
   ```bash
   python mem_diff.py state_title.bin state_playing.bin
   python mem_diff.py state_playing.bin state_gameover.bin
   ```

4. **Look for address that changes consistently:**
   ```
   Title -> Playing:    0x1050: 0x00 -> 0x01
   Playing -> GameOver: 0x1050: 0x01 -> 0x03
   ```

5. **Verify with trace:**
   ```bash
   videopac --trace --trace-filter mem-write --trace-address-range 0x1050:0x1050 --trace-frames 60 --trace-output state_writes.txt bios.bin game.bin
   ```

#### 6.6.5 Identifying Arrays and Buffers

**Characteristics:**
- Contiguous memory region
- Regular element size
- Similar value ranges
- Often initialized to same value

**Identification Process:**

1. **Look for repeating patterns in memory dump:**
   ```
   Address   Data
   00001040  00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F
   00001050  10 11 12 13 14 15 16 17  18 19 1A 1B 1C 1D 1E 1F
   ```
   This looks like an array of sequential values (level data, lookup table).

2. **Look for initialized buffers:**
   ```
   Address   Data
   00001100  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
   00001110  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
   ```
   This looks like a cleared buffer (ready for data).

3. **Track writes to identify usage:**
   ```bash
   videopac --trace --trace-filter mem-write --trace-address-range 0x1100:0x111F --trace-frames 5 --trace-output buffer_writes.txt bios.bin game.bin
   ```

4. **Analyze write patterns:**
   - Sequential writes: Array being filled
   - Random writes: Buffer being used
   - No writes: Unused or read-only data

#### 6.6.6 Identifying Bit Flags

**Characteristics:**
- Single byte with multiple boolean values
- Each bit represents a flag
- Changes by setting/clearing individual bits
- Often used for state flags, options, or capabilities

**Identification Process:**

1. **Look for byte with multiple bit changes:**
   ```
   Before: 0x05 = 0000 0101 (bits 0 and 2 set)
   After:  0x0D = 0000 1101 (bits 0, 2, and 3 set)
   ```

2. **Track bit changes over time:**
   ```bash
   videopac --trace --trace-filter mem-write --trace-address-range 0x1060:0x1060 --trace-frames 10 --trace-output flags_writes.txt bios.bin game.bin
   ```

3. **Analyze bit patterns:**
   ```
   Frame 1: 0x00 = 0000 0000 (all flags clear)
   Frame 2: 0x01 = 0000 0001 (flag 0 set)
   Frame 3: 0x05 = 0000 0101 (flags 0 and 2 set)
   Frame 4: 0x04 = 0000 0100 (flag 0 cleared, flag 2 still set)
   ```

4. **Identify flag meanings:**
   - Bit 0: Player has key
   - Bit 1: Door is open
   - Bit 2: Enemy is active
   - Bit 3: Power-up collected

#### 6.6.7 Documenting Data Structures

**Create Memory Map Document:**

```markdown
# Game Memory Map

## Sprite Table (0x1000-0x101F)

| Address | Field | Size | Description |
|---------|-------|------|-------------|
| 0x1000  | Sprite 0 X | 1 | X position (0-255) |
| 0x1001  | Sprite 0 Y | 1 | Y position (0-255) |
| 0x1002  | Sprite 0 Color | 1 | Color index (0-7) |
| 0x1003  | Sprite 0 Pattern | 1 | Pattern index (0-255) |
| 0x1004  | Sprite 0 Flags | 1 | Bit 0: Visible, Bit 1: Double size |
| 0x1005-0x1007 | Reserved | 3 | Padding |
| 0x1008-0x100F | Sprite 1 | 8 | Same format as Sprite 0 |
| 0x1010-0x1017 | Sprite 2 | 8 | Same format as Sprite 0 |
| 0x1018-0x101F | Sprite 3 | 8 | Same format as Sprite 0 |

## Game State (0x1020-0x102F)

| Address | Field | Size | Description |
|---------|-------|------|-------------|
| 0x1020  | Score | 2 | BCD format, 0x0000-0x9999 |
| 0x1022  | Lives | 1 | Binary, 0-9 |
| 0x1023  | Level | 1 | Binary, 1-99 |
| 0x1024  | State | 1 | 0=Title, 1=Playing, 2=Paused, 3=GameOver |
| 0x1025  | Flags | 1 | Bit 0: HasKey, Bit 1: DoorOpen, etc. |
| 0x1026-0x102F | Reserved | 10 | Future use |

## Level Data (0x1030-0x10FF)

| Address | Field | Size | Description |
|---------|-------|------|-------------|
| 0x1030-0x10FF | Tiles | 208 | 16x13 tile map, each byte is tile index |
```

**Create Structure Definitions (C-style):**

```c
// Sprite structure (8 bytes)
typedef struct {
    uint8_t x;          // 0x00: X position (0-255)
    uint8_t y;          // 0x01: Y position (0-255)
    uint8_t color;      // 0x02: Color index (0-7)
    uint8_t pattern;    // 0x03: Pattern index (0-255)
    uint8_t flags;      // 0x04: Bit 0: Visible, Bit 1: Double size
    uint8_t reserved[3]; // 0x05-0x07: Padding
} Sprite;

// Game state structure
typedef struct {
    uint16_t score;     // 0x1020: BCD format score
    uint8_t lives;      // 0x1022: Remaining lives
    uint8_t level;      // 0x1023: Current level
    uint8_t state;      // 0x1024: Game state enum
    uint8_t flags;      // 0x1025: State flags
} GameState;

// Memory layout
#define SPRITE_TABLE_ADDR  0x1000
#define GAME_STATE_ADDR    0x1020
#define LEVEL_DATA_ADDR    0x1030

Sprite sprites[4];      // 0x1000-0x101F
GameState gameState;    // 0x1020-0x1025
uint8_t levelData[208]; // 0x1030-0x10FF
```

#### 6.6.8 Practical Examples

**Example 1: Identifying Sprite Table**

```
Step 1: Capture memory dump
$ videopac --memory-dump --dump-output mem.bin bios.bin game.bin

Step 2: View first 32 bytes (potential sprite table)
$ hexdump -C mem.bin | head -3
00000000  64 80 04 12 00 00 00 00  C0 60 01 34 00 00 00 00
00000010  50 A0 02 56 00 00 00 00  78 70 03 78 00 00 00 00

Step 3: Analyze pattern
Sprite 0: X=0x64(100), Y=0x80(128), Color=0x04(red), Pattern=0x12
Sprite 1: X=0xC0(192), Y=0x60(96), Color=0x01(blue), Pattern=0x34
Sprite 2: X=0x50(80), Y=0xA0(160), Color=0x02(green), Pattern=0x56
Sprite 3: X=0x78(120), Y=0x70(112), Color=0x03(cyan), Pattern=0x78

Step 4: Verify in Memory Viewer
- Open debugger, jump to 0x1000
- Move sprite in game
- Confirm X/Y values change

Conclusion: Sprite table at 0x1000-0x101F, 8 bytes per sprite
```

**Example 2: Identifying Score Format**

```
Step 1: Score 0 points, capture dump
$ videopac --memory-dump --dump-output score_0.bin bios.bin game.bin

Step 2: Score 25 points, capture dump
$ videopac --memory-dump --dump-output score_25.bin bios.bin game.bin

Step 3: Compare dumps
$ python mem_diff.py score_0.bin score_25.bin
Found 1 difference:
  0x1020: 0x00 -> 0x25

Step 4: Analyze format
0x25 = 37 decimal (binary) or 25 decimal (BCD)
Since we scored 25 points, this is BCD format!

Step 5: Verify with more points
Score 99 points: 0x1020 = 0x99 ✓ (BCD)
Score 100 points: 0x1020 = 0x00, 0x1021 = 0x01 ✓ (BCD, 2 bytes)

Conclusion: Score at 0x1020-0x1021, 2-byte BCD format (0x0000-0x9999)
```

**Example 3: Identifying State Machine**

```
Step 1: Observe game states
- Title screen
- Playing
- Paused
- Game over

Step 2: Capture dumps at each state
$ videopac --memory-dump --dump-output state_title.bin bios.bin game.bin
$ videopac --memory-dump --dump-output state_playing.bin bios.bin game.bin
$ videopac --memory-dump --dump-output state_paused.bin bios.bin game.bin
$ videopac --memory-dump --dump-output state_gameover.bin bios.bin game.bin

Step 3: Compare dumps
$ python mem_diff.py state_title.bin state_playing.bin
  0x1024: 0x00 -> 0x01

$ python mem_diff.py state_playing.bin state_paused.bin
  0x1024: 0x01 -> 0x02

$ python mem_diff.py state_paused.bin state_gameover.bin
  0x1024: 0x02 -> 0x03

Conclusion: State variable at 0x1024
  0x00 = Title
  0x01 = Playing
  0x02 = Paused
  0x03 = Game Over
```

#### 6.6.9 Tips and Best Practices

**Tip 1: Start with Known Structures**
- Sprite tables are usually at start of RAM
- Game state is usually near sprites
- Level data is usually in larger blocks

**Tip 2: Use Multiple Techniques**
- Memory dumps show static state
- Traces show dynamic behavior
- Memory Viewer shows real-time changes
- Combine all three for complete picture

**Tip 3: Look for Patterns**
- Regular spacing indicates arrays or tables
- Similar value ranges indicate related data
- Bit patterns indicate flags or packed data

**Tip 4: Verify Your Findings**
- Test hypotheses by changing values
- Use Memory Viewer to watch changes
- Set breakpoints to see when data is accessed

**Tip 5: Document Everything**
- Create memory map document
- Define structures in C-style format
- Add comments explaining each field
- Keep documentation up-to-date

**Tip 6: Use Meaningful Names**
- Don't just call it "byte at 0x1020"
- Name it "score" or "playerLives"
- Makes debugging and analysis easier

#### 6.6.10 Common Pitfalls

**Pitfall 1: Assuming Fixed Addresses**
- Some games use dynamic allocation
- Verify addresses are consistent across runs
- Check if addresses change between levels

**Pitfall 2: Misidentifying Data Types**
- BCD vs binary can be confusing
- Verify number format with multiple values
- Check for nibble constraints (0-9 for BCD)

**Pitfall 3: Overlooking Padding**
- Structures may have padding bytes
- Don't assume all bytes are used
- Check for reserved or unused fields

**Pitfall 4: Confusing Temporary vs Persistent**
- Some memory is temporary (buffers, scratch)
- Some memory is persistent (game state, score)
- Verify data persists across frames

**Pitfall 5: Missing Indirect References**
- Some data is accessed via pointers
- Address in memory may point to other data
- Follow pointers to find actual data

#### 6.6.11 Integration with Other Tools

**With Memory Viewer:**
1. Use identification techniques to find structures
2. Open Memory Viewer and jump to addresses
3. Watch structures change in real-time
4. Verify field meanings by observing changes

**With Traces:**
1. Identify structure addresses
2. Capture traces with memory write filter
3. See when and how structures are modified
4. Understand data flow and timing

**With Disassembly:**
1. Find structure addresses
2. Search disassembly for references
3. Analyze code that accesses structures
4. Understand how data is used

**With Watch Expressions:**
1. Create watches for structure fields
2. Monitor changes without manual inspection
3. Get alerts when critical values change
4. Track relationships between fields

#### 6.6.12 Summary

Data structure identification is essential for understanding game memory organization. Key techniques include:

- **Pattern recognition:** Identify structures by their format
- **Comparative analysis:** Compare memory across different states
- **Dynamic observation:** Watch memory change in real-time
- **Verification:** Test hypotheses with multiple methods
- **Documentation:** Create comprehensive memory maps

Combined with other debugging tools, data structure identification provides deep insight into how games organize and manipulate data. Master these techniques, and you'll quickly reverse engineer any game's memory layout.

### 6.7 Watch Expression Usage

Watch expressions are powerful debugging tools that allow you to monitor specific memory addresses and CPU registers in real-time. The in-game debugger provides a Watch panel where you can create expressions that continuously display values and alert you when they change.

#### 6.7.1 Overview

Watch expressions enable you to:
- Monitor memory addresses without manually checking them
- Track CPU register values during execution
- Receive alerts when values change
- Compare multiple values simultaneously
- Identify when and how data changes during gameplay

The Watch panel updates automatically as you step through code or during continuous execution, making it easy to spot unexpected changes or verify that values are being updated correctly.

#### 6.7.2 Creating Watch Expressions for Memory Addresses

**Basic Memory Watch:**

To watch a memory address:
1. Open the debugger (F12)
2. Navigate to the Watch panel
3. Click "Add Watch" or press Insert
4. Enter the memory address in hexadecimal

**Syntax:**
```
[0x1000]        # Watch byte at address 0x1000
[0x1000:2]      # Watch 2 bytes starting at 0x1000
[0x1000:4]      # Watch 4 bytes starting at 0x1000
```

**Display Formats:**

You can specify how the value should be displayed:
```
[0x1000]        # Hexadecimal (default)
[0x1000]d       # Decimal
[0x1000]b       # Binary
[0x1000]c       # ASCII character
```

**Examples:**
```
[0x1000]        # Display: 0x42
[0x1000]d       # Display: 66
[0x1000]b       # Display: 01000010
[0x1000]c       # Display: 'B'
```

#### 6.7.3 Creating Watch Expressions for CPU Registers

**Register Watch Syntax:**

To watch CPU registers, use the register name directly:
```
A               # Accumulator
PSW             # Program Status Word
PC              # Program Counter
SP              # Stack Pointer
R0              # Register 0
R1              # Register 1
R2-R7           # Registers 2-7
```

**Flag Watches:**

You can watch individual PSW flags:
```
PSW.CY          # Carry flag
PSW.AC          # Auxiliary carry flag
PSW.F0          # User flag 0
PSW.BS          # Register bank select
```

**Examples:**
```
A               # Display: 0x42
A > 0x80        # Display: false (conditional expression)
R0              # Display: 0x10
PSW.CY          # Display: 1 (carry set)
```

#### 6.7.4 Watch Expression Alerts

**Change Detection:**

The Watch panel highlights expressions when their values change:
- **Green highlight** - Value increased
- **Red highlight** - Value decreased
- **Yellow highlight** - Value changed (non-numeric)
- **No highlight** - Value unchanged

Highlights persist for a configurable duration (default: 2 seconds) to make changes visible even during continuous execution.

**Alert Configuration:**

You can configure alert behavior:
1. Right-click on a watch expression
2. Select "Alert Settings"
3. Configure:
   - Highlight duration
   - Sound on change (optional)
   - Break on change (pause execution)
   - Log changes to file

**Break on Change:**

To pause execution when a value changes:
1. Right-click on watch expression
2. Select "Break on Change"
3. Execution will pause whenever the value changes
4. Useful for identifying when data is modified

#### 6.7.5 Advanced Watch Expressions

**Conditional Expressions:**

Watch expressions support conditional logic:
```
A == 0x42       # True when A equals 0x42
R0 > R1         # True when R0 is greater than R1
[0x1000] != 0   # True when memory is non-zero
```

**Arithmetic Expressions:**

Perform calculations in watch expressions:
```
A + R0          # Sum of A and R0
[0x1000] * 2    # Memory value times 2
PC - 0x0400     # Offset from base address
```

**Pointer Dereferencing:**

Watch memory pointed to by a register:
```
[@R0]           # Value at address in R0
[@R0:4]         # 4 bytes at address in R0
```

**Named Watches:**

Add descriptive names to watch expressions:
```
PlayerX: [0x1000]           # Player X position
PlayerY: [0x1001]           # Player Y position
Score: [0x1020:2]d          # Score (2 bytes, decimal)
Lives: [0x1022]d            # Lives remaining
```

#### 6.7.6 Common Debugging Scenarios

**Scenario 1: Tracking Sprite Position**

Watch sprite position in memory:
```
SpriteX: [0x1000]d          # X position (decimal)
SpriteY: [0x1001]d          # Y position (decimal)
SpriteColor: [0x1002]       # Color value
SpritePattern: [0x1003]     # Pattern index
```

Set "Break on Change" for SpriteX and SpriteY to identify when the game updates sprite positions.

**Scenario 2: Monitoring Game State**

Watch game state variables:
```
GameState: [0x1020]         # Current game state
Level: [0x1021]d            # Current level
Score: [0x1022:2]d          # Score (16-bit)
Lives: [0x1024]d            # Lives remaining
```

Monitor these values during gameplay to understand state transitions.

**Scenario 3: Debugging Collision Detection**

Watch collision-related values:
```
Sprite0X: [0x1000]d
Sprite1X: [0x1008]d
Distance: [0x1000] - [0x1008]   # Calculate distance
CollisionFlag: [0x1030]         # Collision detected flag
```

Set "Break on Change" for CollisionFlag to pause when collision is detected.

**Scenario 4: Tracking VDC Register Updates**

Watch memory that will be written to VDC:
```
NextVDCReg: [0x1040]        # Next VDC register to update
NextVDCValue: [0x1041]      # Value to write
UpdateFlag: [0x1042]        # Update pending flag
```

This helps identify when the game prepares VDC updates.

**Scenario 5: Monitoring CPU Registers During Loop**

Watch registers during a loop:
```
Counter: R7                 # Loop counter
Pointer: R0                 # Memory pointer
Accumulator: A              # Current value
CarryFlag: PSW.CY           # Carry status
```

Step through the loop (F9) and observe how values change.

#### 6.7.7 Watch Expression Best Practices

**Organization:**

- Group related watches together
- Use descriptive names for clarity
- Remove unused watches to reduce clutter
- Save watch configurations for different debugging sessions

**Performance:**

- Limit the number of active watches (recommended: < 20)
- Avoid complex expressions in performance-critical sections
- Use conditional breakpoints instead of "Break on Change" for rare events

**Verification:**

- Cross-reference watch values with Memory Viewer
- Verify watch expressions display expected values
- Test watch expressions with known values first

**Documentation:**

- Document watch expressions in debugging notes
- Share useful watch configurations with team
- Include watch expressions in bug reports

#### 6.7.8 Watch Panel Keyboard Shortcuts

| Key | Action |
|-----|--------|
| Insert | Add new watch expression |
| Delete | Remove selected watch |
| F2 | Edit selected watch |
| Ctrl+C | Copy watch expression |
| Ctrl+V | Paste watch expression |
| Ctrl+Up/Down | Reorder watches |
| Space | Toggle "Break on Change" |

#### 6.7.9 Troubleshooting Watch Expressions

**Watch Shows "???" or "Invalid":**
- Verify address is within valid memory range
- Check syntax of expression
- Ensure register names are correct
- Verify pointer dereference is valid

**Watch Not Updating:**
- Verify execution is running or stepping
- Check if address is being written to
- Ensure watch panel is visible and active
- Try removing and re-adding the watch

**False Change Alerts:**
- Some memory locations change frequently
- Adjust alert sensitivity settings
- Use conditional expressions to filter changes
- Consider using breakpoints instead

**Performance Issues:**
- Reduce number of active watches
- Simplify complex expressions
- Disable "Break on Change" for frequently changing values
- Close watch panel when not needed

#### 6.7.10 Example: Debugging Sprite Movement Bug

**Problem:** Player sprite moves erratically, sometimes jumping to wrong positions.

**Investigation with Watch Expressions:**

1. **Add watches for sprite data:**
```
PlayerX: [0x1000]d
PlayerY: [0x1001]d
PlayerVelX: [0x1002]d
PlayerVelY: [0x1003]d
InputState: [0x1010]
```

2. **Enable "Break on Change" for PlayerX and PlayerY**

3. **Run the game and press movement keys**

4. **Observe watch panel:**
   - PlayerX changes from 80 to 255 (unexpected!)
   - PlayerVelX shows 0xFF (-1 in signed)
   - This suggests velocity is being added as unsigned

5. **Locate the bug:**
   - Set breakpoint when PlayerX changes
   - Examine code that updates position
   - Find that velocity is treated as unsigned byte
   - Should be signed byte for negative movement

6. **Verify fix:**
   - After fixing velocity handling
   - Watch PlayerX change smoothly: 80, 79, 78, 77...
   - No more erratic jumps

This example demonstrates how watch expressions help identify the exact moment when incorrect values appear, leading directly to the root cause.

#### 6.7.11 Summary

Watch expressions are essential tools for real-time debugging:

- **Memory watches** - Monitor specific addresses continuously
- **Register watches** - Track CPU register values
- **Change alerts** - Visual and audible notifications
- **Break on change** - Pause execution when values change
- **Advanced expressions** - Conditionals, arithmetic, pointers
- **Named watches** - Descriptive labels for clarity

Master watch expressions to dramatically improve debugging efficiency. They provide continuous visibility into program state without manual inspection, helping you quickly identify when and how values change during execution.

### 6.8 Examples

This section provides practical examples demonstrating memory analysis techniques for common debugging scenarios in Videopac/Odyssey2 games.

#### Example 1: Analyzing Memory Dump for Sprite Data

**Scenario:** A racing game has sprite rendering issues. We need to identify where sprite data is stored in memory.

**Steps:**

1. **Capture Memory Dump**
```bash
videopac --memory-dump --dump-output racing_mem.bin bios.bin racing.bin
```

2. **Examine First 64 Bytes of RAM (0x1000-0x103F)**
```bash
hexdump -C racing_mem.bin -s 0x1000 -n 64
00001000  50 78 03 A4 00 00 00 00  64 80 01 B2 00 00 00 00  |Px......d.......|
00001010  C0 90 02 C8 00 00 00 00  A0 60 04 D5 00 00 00 00  |.........`......|
00001020  00 03 01 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00001030  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
```

3. **Analyze Pattern**
- Bytes 0x1000-0x1007: `50 78 03 A4 00 00 00 00`
  - 0x50 (80 decimal) - Possible X position
  - 0x78 (120 decimal) - Possible Y position
  - 0x03 - Possible color (cyan)
  - 0xA4 - Possible pattern index
  - 4 zero bytes - Padding or unused

- Bytes 0x1008-0x100F: `64 80 01 B2 00 00 00 00`
  - Similar pattern: X, Y, Color, Pattern, padding

4. **Verify in Memory Viewer**
- Open debugger (F12)
- Navigate to Memory Viewer
- Jump to address 0x1000 (Ctrl+G)
- Move player car in game
- Observe bytes at 0x1000-0x1001 change with car position

5. **Verify in VDC Register Viewer**
- Check VDC sprite registers
- Compare with memory values
- Confirm sprite 0 X/Y matches 0x1000-0x1001

**Conclusion:** Sprite table located at 0x1000-0x101F, 8 bytes per sprite (X, Y, Color, Pattern, 4 padding bytes).

#### Example 2: Searching Memory for Specific Patterns

**Scenario:** A puzzle game displays the number "42" on screen. We want to find where this value is stored.

**Steps:**

1. **Search for Decimal Value (0x2A)**
```bash
# In Memory Viewer (F12)
1. Press Ctrl+F for search
2. Enter: 2A
3. Search type: Hex
4. Press Enter
```

**Results:**
```
Found at: 0x1045
Found at: 0x10A3
Found at: 0x1200
```

2. **Search for BCD Value (0x42)**
```bash
# In Memory Viewer
1. Press Ctrl+F for search
2. Enter: 42
3. Search type: Hex
4. Press Enter
```

**Results:**
```
Found at: 0x1028
Found at: 0x1150
```

3. **Verify Which is Correct**
- Change game value to 43
- Check which memory location changed
- 0x1028 changed to 0x43 ✓
- Conclusion: Value stored in BCD format at 0x1028

4. **Search for ASCII String "SCORE"**
```bash
# In Memory Viewer
1. Press Ctrl+F for search
2. Enter: 53 43 4F 52 45
3. Search type: Hex (ASCII: S=0x53, C=0x43, O=0x4F, R=0x52, E=0x45)
4. Press Enter
```

**Result:** Found at 0x0850 (in ROM, not RAM)

**Conclusion:** Score value at 0x1028 (BCD), score label at 0x0850 (ROM string).

#### Example 3: Tracking Memory Writes in Execution Trace

**Scenario:** A platformer game has a bug where the player's lives counter doesn't decrement. We need to find where lives are stored and when they're updated.

**Steps:**

1. **Capture Trace with Memory Write Filter**
```bash
videopac --trace --trace-filter mem-write --trace-start-key "1" \
  --trace-frames 60 --trace-output lives_trace.txt bios.bin platform.bin
```

2. **Analyze Trace for RAM Writes**
```
Frame:0001 Scanline:010 Cycle:00234 | MEM_WRITE[0x1025] = 0x03
Frame:0001 Scanline:015 Cycle:00456 | MEM_WRITE[0x1026] = 0x00
Frame:0001 Scanline:020 Cycle:00678 | MEM_WRITE[0x1000] = 0x50
...
Frame:0120 Scanline:195 Cycle:12345 | MEM_WRITE[0x1025] = 0x02
```

3. **Identify Lives Counter**
- 0x1025 written with value 0x03 at game start
- 0x1025 written with value 0x02 after player death (frame 120)
- Hypothesis: 0x1025 is lives counter

4. **Verify with Memory Viewer**
- Jump to 0x1025
- Start game (should show 0x03)
- Die once (should show 0x02)
- Die again (should show 0x01)
- Confirmed: 0x1025 is lives counter

5. **Find Code That Updates Lives**
```bash
# Search trace for PC when 0x1025 is written
grep "MEM_WRITE\[0x1025\]" lives_trace.txt
Frame:0120 Scanline:195 Cycle:12345 PC:0x0456 | MEM_WRITE[0x1025] = 0x02
```

6. **Check Disassembly at 0x0456**
```assembly
0x0454   MOV A, @R0      ; Load current lives
0x0455   DEC A           ; Decrement lives
0x0456   MOV @R0, A      ; Store back to memory (0x1025)
0x0457   JZ 0x0500       ; If zero, game over
```

**Conclusion:** Lives counter at 0x1025, decremented by code at 0x0454-0x0457. Bug investigation can now focus on why this code isn't being called.

#### Example 4: Identifying Data Structures Through Comparative Analysis

**Scenario:** A space shooter has multiple enemy ships. We want to understand the enemy data structure.

**Steps:**

1. **Capture Memory with 0 Enemies**
```bash
videopac --memory-dump --dump-output enemies_0.bin bios.bin shooter.bin
```

2. **Capture Memory with 1 Enemy**
```bash
# Spawn one enemy, then capture
videopac --memory-dump --dump-output enemies_1.bin bios.bin shooter.bin
```

3. **Compare Dumps**
```bash
python3 << 'EOF'
with open('enemies_0.bin', 'rb') as f1, open('enemies_1.bin', 'rb') as f2:
    data1 = f1.read()
    data2 = f2.read()
    
    for i in range(min(len(data1), len(data2))):
        if data1[i] != data2[i]:
            print(f"0x{i:04X}: 0x{data1[i]:02X} -> 0x{data2[i]:02X}")
EOF
```

**Results:**
```
0x1040: 0x00 -> 0x01  (Enemy count?)
0x1041: 0x00 -> 0x78  (X position?)
0x1042: 0x00 -> 0x40  (Y position?)
0x1043: 0x00 -> 0x02  (Enemy type?)
0x1044: 0x00 -> 0x05  (Health?)
0x1045: 0x00 -> 0x00  (Padding?)
```

4. **Spawn Second Enemy and Compare**
```bash
videopac --memory-dump --dump-output enemies_2.bin bios.bin shooter.bin
```

**New Differences:**
```
0x1040: 0x01 -> 0x02  (Enemy count confirmed!)
0x1046: 0x00 -> 0xA0  (Enemy 2 X position)
0x1047: 0x00 -> 0x60  (Enemy 2 Y position)
0x1048: 0x00 -> 0x01  (Enemy 2 type)
0x1049: 0x00 -> 0x03  (Enemy 2 health)
0x104A: 0x00 -> 0x00  (Padding)
```

5. **Define Data Structure**
```c
// Enemy structure (5 bytes each)
typedef struct {
    uint8_t x;        // X position
    uint8_t y;        // Y position
    uint8_t type;     // Enemy type (0-3)
    uint8_t health;   // Health points
    uint8_t padding;  // Unused
} Enemy;

// Memory layout
#define MAX_ENEMIES 8
#define ENEMY_COUNT_ADDR 0x1040
#define ENEMY_ARRAY_ADDR 0x1041

uint8_t enemyCount;     // 0x1040
Enemy enemies[8];       // 0x1041-0x1068 (5 bytes × 8 enemies)
```

**Conclusion:** Enemy count at 0x1040, enemy array at 0x1041-0x1068, 5 bytes per enemy.

#### Example 5: Using Memory Viewer for Real-Time Analysis

**Scenario:** A maze game has a bug where walls sometimes disappear. We need to monitor the level data in real-time.

**Steps:**

1. **Identify Level Data Location**
- Hypothesis: Level data stored as tile map in RAM
- Expected: 16×12 grid = 192 bytes
- Likely location: After sprite/game state data

2. **Open Memory Viewer**
```
1. Press F12 to open debugger
2. Navigate to Memory Viewer panel
3. Press Ctrl+G to jump to address
4. Enter: 1100 (hypothesis: level data at 0x1100)
```

3. **Observe Memory Pattern**
```
0x1100: 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01  |................|
0x1110: 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01  |................|
0x1120: 01 00 01 01 01 00 01 01 01 01 01 00 01 01 00 01  |................|
0x1130: 01 00 01 00 00 00 00 00 00 00 00 00 00 01 00 01  |................|
```

**Pattern Analysis:**
- 0x01 = Wall tile
- 0x00 = Empty space
- Pattern shows maze structure
- 16 bytes per row = 16 tiles wide ✓

4. **Monitor for Changes**
- Play game and watch memory
- When wall disappears on screen, check memory
- If memory still shows 0x01, bug is in rendering
- If memory shows 0x00, bug is in level data update

5. **Set Watch Expression**
```
1. Open Watch panel
2. Add watch: [0x1120] (specific wall tile)
3. Play game
4. Watch alerts when value changes
```

**Observation:** Memory changes from 0x01 to 0x00 when wall disappears!

6. **Capture Trace at Moment of Change**
```bash
# Set breakpoint on memory write to 0x1120
videopac --debugger --breakpoint-mem-write 0x1120 bios.bin maze.bin
```

**Trace Shows:**
```
Frame:0234 Scanline:100 Cycle:05678 PC:0x0678 | MEM_WRITE[0x1120] = 0x00
```

7. **Check Disassembly at 0x0678**
```assembly
0x0676   MOV A, #0x00    ; Load empty tile
0x0678   MOV @R0, A      ; Write to level data (BUG: should check collision first!)
```

**Conclusion:** Bug found! Code at 0x0676-0x0678 writes empty tile without checking if it's a wall. Level data at 0x1100-0x11BF (192 bytes).

#### Example 6: Combining Multiple Techniques

**Scenario:** A sports game has incorrect score display. We need to find the score variable, understand its format, and verify the display logic.

**Steps:**

1. **Search Memory for Score Value**
- Current score: 150 points
- Search for 0x96 (150 decimal) - Not found
- Search for 0x0150 (hex) - Not found
- Search for 0x50 0x01 (BCD) - Found at 0x1030!

2. **Verify with Memory Viewer**
```
0x1030: 50 01 00 00  |P...|
```
- 0x1030 = 0x50 (50 in BCD)
- 0x1031 = 0x01 (1 in BCD)
- Combined: 150 points ✓

3. **Track Score Updates in Trace**
```bash
videopac --trace --trace-filter mem-write --trace-address-range 0x1030:0x1033 \
  --trace-frames 120 --trace-output score_trace.txt bios.bin sports.bin
```

**Trace Shows:**
```
Frame:0045 Scanline:200 Cycle:08901 PC:0x0890 | MEM_WRITE[0x1030] = 0x60
Frame:0045 Scanline:200 Cycle:08903 PC:0x0892 | MEM_WRITE[0x1031] = 0x01
```

4. **Analyze Score Update Code**
```assembly
0x0880   MOV A, @R0      ; Load score low byte (0x1030)
0x0881   ADD A, #0x10    ; Add 10 points (BCD)
0x0883   DA A            ; Decimal adjust
0x0884   MOV @R0, A      ; Store back
0x0885   INC R0          ; Point to high byte
0x0886   MOV A, @R0      ; Load score high byte (0x1031)
0x0887   ADDC A, #0x00   ; Add carry
0x0889   DA A            ; Decimal adjust
0x088A   MOV @R0, A      ; Store back
```

5. **Find Display Code**
```bash
# Search disassembly for reads from 0x1030
grep "0x1030" sports_disasm.txt
0x0A20   MOV R0, #0x30   ; Point to score
0x0A22   MOV A, @R0      ; Load score low byte
0x0A23   CALL 0x0B00     ; Display digit routine
```

6. **Verify Display Logic**
```assembly
0x0B00   MOV R1, A       ; Save value
0x0B01   ANL A, #0x0F    ; Get low nibble
0x0B03   CALL 0x0C00     ; Display digit
0x0B06   MOV A, R1       ; Restore value
0x0B07   SWAP A          ; Get high nibble
0x0B08   ANL A, #0x0F    ; Mask
0x0B0A   CALL 0x0C00     ; Display digit
```

**Bug Found:** Display code at 0x0B00 processes bytes in wrong order! It displays low byte first, then high byte, resulting in "5001" instead of "0150".

**Conclusion:** Score at 0x1030-0x1031 (2-byte BCD), updated correctly at 0x0880-0x088A, but displayed incorrectly at 0x0B00-0x0B0A due to byte order bug.

#### Summary

These examples demonstrate practical memory analysis workflows:

1. **Example 1:** Memory dump analysis for sprite data identification
2. **Example 2:** Memory search techniques for finding specific values
3. **Example 3:** Trace analysis for tracking memory writes
4. **Example 4:** Comparative analysis for data structure identification
5. **Example 5:** Real-time memory monitoring with Memory Viewer
6. **Example 6:** Combined techniques for comprehensive debugging

**Key Takeaways:**
- Use multiple techniques together for best results
- Verify hypotheses with different methods
- Document findings as you discover them
- Memory analysis is iterative - refine understanding over time
- Combine memory analysis with disassembly and trace analysis

[↑ Back to Top](#table-of-contents)

---

## 7. Timing and Synchronization

### 7.1 Videopac Timing Model

Understanding the Videopac/Odyssey2 timing model is essential for debugging timing-dependent issues, analyzing traces, and understanding the relationship between CPU execution and video rendering. The system consists of two primary timing domains: the CPU (Intel 8048) and the VDC (Intel 8245 Video Display Controller), which must synchronize to produce correct video output.

#### System Architecture Overview

The Videopac system uses two independent but synchronized clock domains:

1. **CPU Clock Domain** - Intel 8048 microcontroller executing game code
2. **VDC Clock Domain** - Intel 8245 video controller generating display output

These two components communicate through I/O ports and must coordinate their operations to produce correct video frames.

#### CPU Timing

**Clock Frequencies:**

The CPU clock frequency varies by video standard:

- **NTSC (North America, Japan):** 5.37 MHz base clock
- **PAL (Europe, Australia):** 5.91 MHz base clock

**Instruction Cycle Clock:**

The Intel 8048 divides the base clock by 15 to produce the instruction cycle clock:

- **NTSC:** 5.37 MHz ÷ 15 = 358 kHz (2.79 µs per cycle)
- **PAL:** 5.91 MHz ÷ 15 = 394 kHz (2.54 µs per cycle)

**Instruction Timing:**

Intel 8048 instructions execute in either 1 or 2 instruction cycles:

- **Single-cycle instructions:** 2.79 µs (NTSC) or 2.54 µs (PAL)
  - Examples: `MOV A, Rr`, `INC A`, `ADD A, #data`
- **Dual-cycle instructions:** 5.58 µs (NTSC) or 5.08 µs (PAL)
  - Examples: `JMP addr`, `CALL addr`, `MOVX A, @Rr`

**CPU Cycles Per Frame:**

The number of CPU instruction cycles available per video frame:

- **NTSC:** 358 kHz ÷ 60 Hz ≈ 5,967 cycles per frame
- **PAL:** 394 kHz ÷ 50 Hz ≈ 7,880 cycles per frame

This means games have approximately 6,000 (NTSC) or 8,000 (PAL) instruction cycles to update game logic, read input, and write VDC registers for each video frame.

#### VDC Timing

**VDC Clock Frequency:**

The Intel 8245 VDC operates at a fixed frequency regardless of video standard:

- **VDC Clock:** 3.54 MHz (282 ns per cycle)

This is the PAL color subcarrier frequency, chosen for compatibility with European television standards.

**Scanline Timing:**

The VDC generates video output one scanline at a time:

- **Scanline Duration:** 227 VDC clock cycles = 64.1 µs
- **Horizontal Frequency:** 15,625 Hz (15.625 kHz)

Each scanline consists of:
- **Active Display:** ~52 µs (visible pixels)
- **Horizontal Blanking (HBL):** ~12 µs (beam return)

**Frame Timing:**

A complete video frame consists of multiple scanlines:

**NTSC (60 Hz):**
- **Total Scanlines:** 262 scanlines per frame
- **Active Scanlines:** 192 scanlines (visible area)
- **Vertical Blanking (VBL):** 70 scanlines (beam return)
- **Frame Duration:** 262 × 64.1 µs = 16.79 ms
- **Frame Rate:** 59.56 Hz (approximately 60 Hz)

**PAL (50 Hz):**
- **Total Scanlines:** 312 or 313 scanlines per frame (alternating)
- **Active Scanlines:** 242 scanlines (visible area)
- **Vertical Blanking (VBL):** 70 scanlines (beam return)
- **Frame Duration:** 312.5 × 64.1 µs = 20.03 ms
- **Frame Rate:** 49.92 Hz (approximately 50 Hz)

#### CPU-VDC Synchronization

**Clock Ratio:**

The VDC runs approximately 9.9 times faster than the CPU:

- VDC Clock: 3.54 MHz
- CPU Instruction Cycle: 0.358 MHz (NTSC)
- Ratio: 3.54 ÷ 0.358 ≈ 9.9 VDC cycles per CPU instruction cycle

This means that for every CPU instruction executed, the VDC advances by approximately 10 clock cycles.

**Interleaved Execution:**

The emulator uses cycle-accurate interleaved execution:

1. Master clock tracks cycle debt for both CPU and VDC
2. When CPU debt ≥ 9.9 cycles, execute one CPU instruction
3. When VDC debt ≥ 1.0 cycles, execute one VDC clock cycle
4. This ensures proper synchronization between CPU and video output

**VDC Register Write Timing:**

Games typically write to VDC registers during specific periods:

**During VBLANK (Recommended):**
- VBLANK occurs for 70 scanlines at the end of each frame
- Duration: 70 × 64.1 µs = 4.49 ms
- CPU cycles available: ~1,600 cycles (NTSC) or ~1,770 cycles (PAL)
- Writing during VBLANK prevents visual artifacts (tearing, flickering)

**During Active Display (Mid-Frame):**
- Some games write VDC registers during active scanlines
- Used for effects like sprite multiplexing, color changes, or position updates
- Requires precise timing to avoid visual glitches
- Can cause tearing if not synchronized properly

**Horizontal Blanking (HBL):**
- Brief period during each scanline (~12 µs)
- CPU cycles available: ~4 cycles per HBL
- Some games use HBL for quick register updates

#### Timing Calculations

**Example 1: CPU Instructions Per Scanline**

NTSC:
- Scanline duration: 64.1 µs
- CPU instruction cycle: 2.79 µs
- Instructions per scanline: 64.1 ÷ 2.79 ≈ 23 instructions

PAL:
- Scanline duration: 64.1 µs
- CPU instruction cycle: 2.54 µs
- Instructions per scanline: 64.1 ÷ 2.54 ≈ 25 instructions

**Example 2: CPU Instructions During VBLANK**

NTSC:
- VBLANK duration: 70 scanlines × 64.1 µs = 4.49 ms
- CPU instruction cycle: 2.79 µs
- Instructions during VBLANK: 4,490 ÷ 2.79 ≈ 1,609 instructions

PAL:
- VBLANK duration: 70 scanlines × 64.1 µs = 4.49 ms
- CPU instruction cycle: 2.54 µs
- Instructions during VBLANK: 4,490 ÷ 2.54 ≈ 1,768 instructions

**Example 3: VDC Cycles Per CPU Instruction**

For a single-cycle CPU instruction (NTSC):
- CPU instruction: 2.79 µs
- VDC clock period: 0.282 µs
- VDC cycles: 2.79 ÷ 0.282 ≈ 9.9 cycles

#### Timing in Traces

When analyzing execution traces, timing information appears in the following format:

```
Frame:0001 Scanline:010 Cycle:00123 PC:0x0234 A:0x42 PSW:0x08 | MOV A, #0x42
```

**Frame:** Current video frame number (increments at start of each frame)

**Scanline:** Current scanline number (0-261 for NTSC, 0-312 for PAL)
- 0-191 (NTSC) or 0-241 (PAL): Active display
- 192+ (NTSC) or 242+ (PAL): Vertical blanking

**Cycle:** VDC clock cycle within the current scanline (0-226)
- 0-~185: Active display period
- ~186-226: Horizontal blanking period

**Identifying VBLANK in Traces:**

```
Frame:0001 Scanline:192 Cycle:00000 | VBLANK START (NTSC)
Frame:0001 Scanline:242 Cycle:00000 | VBLANK START (PAL)
```

**Identifying HBL in Traces:**

```
Frame:0001 Scanline:010 Cycle:00186 | HBL START
Frame:0001 Scanline:010 Cycle:00226 | HBL END
```

#### Timing Implications for Debugging

**1. Frame Budget:**

Games must complete all logic, input processing, and VDC updates within one frame:
- NTSC: 16.79 ms (5,967 CPU cycles)
- PAL: 20.03 ms (7,880 CPU cycles)

If game logic exceeds this budget, frame rate drops below 60 Hz (NTSC) or 50 Hz (PAL).

**2. VBLANK Window:**

Most VDC register updates should occur during VBLANK:
- Duration: 4.49 ms (~1,600-1,770 CPU cycles)
- If game needs more time, it may update registers mid-frame
- Mid-frame updates can cause visual artifacts

**3. Instruction Timing:**

When analyzing traces, remember:
- Each trace line represents one CPU instruction
- Single-cycle instructions advance ~10 VDC cycles
- Dual-cycle instructions advance ~20 VDC cycles
- VDC state changes continuously, even during CPU execution

**4. Synchronization Issues:**

Common timing bugs:
- Writing VDC registers too late (after VBLANK ends)
- Exceeding frame budget (causing slowdown)
- Mid-frame updates causing tearing
- Race conditions between CPU and VDC

#### Timing Summary Table

| Parameter | NTSC | PAL |
|-----------|------|-----|
| CPU Base Clock | 5.37 MHz | 5.91 MHz |
| CPU Instruction Clock | 358 kHz | 394 kHz |
| CPU Cycle Period | 2.79 µs | 2.54 µs |
| VDC Clock | 3.54 MHz | 3.54 MHz |
| VDC Cycle Period | 0.282 µs | 0.282 µs |
| VDC/CPU Ratio | 9.9:1 | 9.0:1 |
| Scanline Duration | 64.1 µs | 64.1 µs |
| Scanlines Per Frame | 262 | 312/313 |
| Active Scanlines | 192 | 242 |
| VBLANK Scanlines | 70 | 70 |
| Frame Rate | 59.56 Hz | 49.92 Hz |
| Frame Duration | 16.79 ms | 20.03 ms |
| CPU Cycles Per Frame | 5,967 | 7,880 |
| CPU Cycles Per Scanline | 23 | 25 |
| CPU Cycles During VBLANK | 1,609 | 1,768 |

#### Practical Examples

**Example 1: Calculating Sprite Update Time**

A game needs to update 4 sprites (position and color):
- 4 sprites × 3 registers each = 12 VDC writes
- Each write: ~2 CPU instructions (load value, output to port)
- Total: 24 CPU instructions × 2.79 µs = 67 µs
- This fits comfortably within VBLANK (4,490 µs)

**Example 2: Identifying Timing Bug**

Trace shows sprite color write at scanline 195 (NTSC):
```
Frame:0001 Scanline:195 Cycle:00050 | VDC_WRITE[0xAB] = 0x02
```
- Scanline 195 is during VBLANK (192-261)
- This is correct timing - no visual artifact expected

Trace shows sprite color write at scanline 100 (NTSC):
```
Frame:0001 Scanline:100 Cycle:00050 | VDC_WRITE[0xAB] = 0x02
```
- Scanline 100 is during active display (0-191)
- This is mid-frame update - may cause visual artifact
- Check if this is intentional (effect) or bug

**Example 3: Frame Rate Analysis**

Game takes 7,000 CPU cycles per frame (NTSC):
- Available: 5,967 cycles per frame
- Deficit: 7,000 - 5,967 = 1,033 cycles
- Frame time: 7,000 ÷ 358,000 = 19.55 ms
- Actual frame rate: 1,000 ÷ 19.55 = 51.2 Hz
- Result: Game runs at 51 Hz instead of 60 Hz (slowdown)

#### References

For more detailed timing information, see:
- [Intel 8048 Datasheet](#131-intel-8048-architecture) - CPU instruction timing
- [Intel 8245 Datasheet](#132-intel-8245-vdc-architecture) - VDC timing specifications
- [Trace Analysis Chapter](#4-trace-analysis) - Reading timing in traces
- [Common Timing Issues](#77-common-timing-issues) - Debugging timing problems

### 7.2 Instruction Timing Analysis

Analyzing instruction timing in execution traces is essential for understanding performance characteristics, identifying bottlenecks, and debugging timing-dependent issues. This section explains how to calculate execution time for code sequences, measure performance, and identify areas for optimization.

#### Understanding Instruction Cycles

The Intel 8048 CPU executes instructions in either 1 or 2 instruction cycles. Each instruction cycle consists of multiple machine cycles (states), but for timing analysis, we focus on instruction cycles.

**Instruction Cycle Duration:**

- **NTSC:** 2.79 µs per instruction cycle
- **PAL:** 2.54 µs per instruction cycle

**Instruction Types:**

**Single-Cycle Instructions (1 cycle):**
- Register operations: `MOV A, Rr`, `INC A`, `DEC A`, `CLR A`
- Immediate operations: `ADD A, #data`, `ANL A, #data`, `ORL A, #data`
- Register indirect: `MOV A, @Rr`, `MOV @Rr, A`
- Simple control: `NOP`, `RET`, `RETR`

**Dual-Cycle Instructions (2 cycles):**
- Jumps: `JMP addr`, `DJNZ Rr, addr`, `JZ addr`, `JNZ addr`
- Calls: `CALL addr`
- External memory: `MOVX A, @Rr`, `MOVX @Rr, A`
- I/O operations: `IN A, P`, `OUTL P, A`

#### Calculating Execution Time from Traces

When analyzing traces, you can calculate the execution time of code sequences by counting instructions and their cycle counts.

**Basic Calculation:**

```
Execution Time = (Single-Cycle Instructions × 1 + Dual-Cycle Instructions × 2) × Cycle Period
```

**Example Trace Sequence:**

```
Frame:0001 Scanline:010 Cycle:00100 PC:0x0200 | MOV A, #0x42      ; 1 cycle
Frame:0001 Scanline:010 Cycle:00110 PC:0x0201 | MOV R0, A         ; 1 cycle
Frame:0001 Scanline:010 Cycle:00120 PC:0x0202 | OUTL P1, A        ; 2 cycles
Frame:0001 Scanline:010 Cycle:00140 PC:0x0203 | INC A             ; 1 cycle
Frame:0001 Scanline:010 Cycle:00150 PC:0x0204 | JNZ 0x0200        ; 2 cycles
```

**Timing Calculation (NTSC):**

- Single-cycle instructions: 3 (MOV, MOV, INC)
- Dual-cycle instructions: 2 (OUTL, JNZ)
- Total cycles: (3 × 1) + (2 × 2) = 7 instruction cycles
- Execution time: 7 × 2.79 µs = 19.53 µs

#### Using VDC Cycle Counts

The trace includes VDC cycle counts, which provide precise timing information. The VDC cycle counter increments continuously and can be used to measure exact execution time.

**Method 1: VDC Cycle Difference**

```
Frame:0001 Scanline:010 Cycle:00100 PC:0x0200 | MOV A, #0x42
Frame:0001 Scanline:010 Cycle:00150 PC:0x0204 | JNZ 0x0200
```

**Calculation:**
- Start VDC cycle: 100
- End VDC cycle: 150
- VDC cycles elapsed: 150 - 100 = 50 cycles
- Execution time: 50 × 0.282 µs = 14.1 µs

**Method 2: Scanline Transitions**

When code spans multiple scanlines, account for scanline boundaries:

```
Frame:0001 Scanline:010 Cycle:00200 PC:0x0200 | MOV A, #0x42
Frame:0001 Scanline:011 Cycle:00050 PC:0x0250 | RET
```

**Calculation:**
- Scanline 10: 227 - 200 = 27 cycles remaining
- Scanline 11: 50 cycles elapsed
- Total VDC cycles: 27 + 50 = 77 cycles
- Execution time: 77 × 0.282 µs = 21.7 µs

#### Analyzing Code Sequences

**Example 1: Sprite Update Loop**

```
; Update sprite 0 position and color
0x0300: MOV A, #0x50        ; Load X position (1 cycle)
0x0301: OUTL P1, A          ; Write to VDC (2 cycles)
0x0302: MOV A, #0x60        ; Load Y position (1 cycle)
0x0303: OUTL P1, A          ; Write to VDC (2 cycles)
0x0304: MOV A, #0x02        ; Load color (1 cycle)
0x0305: OUTL P1, A          ; Write to VDC (2 cycles)
```

**Timing Analysis (NTSC):**
- Single-cycle: 3 instructions × 1 cycle = 3 cycles
- Dual-cycle: 3 instructions × 2 cycles = 6 cycles
- Total: 9 instruction cycles
- Execution time: 9 × 2.79 µs = 25.11 µs

**Performance Assessment:**
- This sequence takes 25 µs to update one sprite
- Updating 4 sprites: 4 × 25 µs = 100 µs
- VBLANK duration: 4,490 µs
- Sprite updates use: 100 ÷ 4,490 = 2.2% of VBLANK time
- Conclusion: Very efficient, plenty of time for other updates

**Example 2: Game Logic Loop**

```
; Main game loop iteration
0x0400: CALL ReadInput      ; Read joystick (2 cycles + subroutine)
0x0402: CALL UpdatePlayer   ; Update player state (2 cycles + subroutine)
0x0404: CALL UpdateEnemies  ; Update enemies (2 cycles + subroutine)
0x0406: CALL CheckCollision ; Check collisions (2 cycles + subroutine)
0x0408: CALL UpdateVDC      ; Write to VDC (2 cycles + subroutine)
0x040A: JMP 0x0400          ; Loop (2 cycles)
```

**Trace Analysis:**

```
Frame:0001 Scanline:000 Cycle:00000 PC:0x0400 | CALL ReadInput
Frame:0001 Scanline:002 Cycle:00100 PC:0x0402 | CALL UpdatePlayer
Frame:0001 Scanline:005 Cycle:00050 PC:0x0404 | CALL UpdateEnemies
Frame:0001 Scanline:010 Cycle:00200 PC:0x0406 | CALL CheckCollision
Frame:0001 Scanline:012 Cycle:00100 PC:0x0408 | CALL UpdateVDC
Frame:0001 Scanline:015 Cycle:00000 PC:0x040A | JMP 0x0400
```

**Timing Calculation:**
- Start: Scanline 0, Cycle 0
- End: Scanline 15, Cycle 0
- Scanlines elapsed: 15
- VDC cycles: 15 × 227 = 3,405 cycles
- Execution time: 3,405 × 0.282 µs = 960 µs
- CPU instruction cycles: 960 ÷ 2.79 ≈ 344 cycles

**Performance Assessment:**
- One game loop iteration: 960 µs
- Frame budget: 16,790 µs (NTSC)
- Maximum iterations per frame: 16,790 ÷ 960 ≈ 17 iterations
- If game needs 1 iteration per frame: 960 ÷ 16,790 = 5.7% of frame time
- Conclusion: Efficient, leaves 94% of frame time for other tasks

#### Identifying Performance Bottlenecks

**Method 1: Subroutine Timing**

Measure the time spent in each subroutine by finding CALL and RET pairs:

```
Frame:0001 Scanline:010 Cycle:00100 PC:0x0500 | CALL UpdateEnemies
...
Frame:0001 Scanline:015 Cycle:00200 PC:0x0550 | RET
```

**Calculation:**
- Start: Scanline 10, Cycle 100
- End: Scanline 15, Cycle 200
- Scanlines: 5 complete + partial
- VDC cycles: (5 × 227) + (200 - 100) = 1,235 cycles
- Execution time: 1,235 × 0.282 µs = 348 µs

**Method 2: Hotspot Identification**

Look for code sections that execute frequently or take significant time:

```
; Tight loop - executes 100 times
0x0600: MOV A, @R0          ; 1 cycle
0x0601: ADD A, #0x01        ; 1 cycle
0x0602: MOV @R0, A          ; 1 cycle
0x0603: INC R0              ; 1 cycle
0x0604: DJNZ R1, 0x0600     ; 2 cycles (if jump taken)
```

**Per-Iteration Timing:**
- Cycles per iteration: 1 + 1 + 1 + 1 + 2 = 6 cycles
- Time per iteration: 6 × 2.79 µs = 16.74 µs

**Total Loop Timing:**
- Iterations: 100
- Total time: 100 × 16.74 µs = 1,674 µs
- Percentage of frame: 1,674 ÷ 16,790 = 10% of frame time

**Optimization Opportunity:**
If this loop is a bottleneck, consider:
- Reducing iteration count
- Optimizing loop body (fewer instructions)
- Moving work outside the loop

**Method 3: Frame Budget Analysis**

Calculate total CPU time used per frame:

```
Frame:0001 Scanline:000 Cycle:00000 | Frame Start
Frame:0001 Scanline:261 Cycle:00226 | Frame End
```

**Calculation:**
- Total scanlines: 262 (NTSC)
- Total VDC cycles: 262 × 227 = 59,474 cycles
- Total time: 59,474 × 0.282 µs = 16,772 µs
- CPU cycles used: 16,772 ÷ 2.79 ≈ 6,013 cycles

**Budget Assessment:**
- Available CPU cycles: 5,967 (NTSC)
- Used CPU cycles: 6,013
- Deficit: 6,013 - 5,967 = 46 cycles
- Result: Frame overrun by 46 cycles (128 µs)
- Impact: Frame rate drops slightly below 60 Hz

#### Measuring Code Execution Time

**Technique 1: Trace Markers**

Insert distinctive instructions to mark code sections:

```
0x0700: MOV A, #0xFF        ; Marker: Start of section
0x0701: NOP                 ; Optional padding
; ... code to measure ...
0x0750: MOV A, #0x00        ; Marker: End of section
```

Search trace for markers to find execution time:

```
Frame:0001 Scanline:020 Cycle:00100 PC:0x0700 | MOV A, #0xFF
Frame:0001 Scanline:025 Cycle:00150 PC:0x0750 | MOV A, #0x00
```

**Technique 2: Breakpoint Timing**

Use debugger breakpoints to measure execution time:

1. Set breakpoint at start of code section
2. Note frame, scanline, and cycle
3. Set breakpoint at end of code section
4. Note frame, scanline, and cycle
5. Calculate elapsed time

**Technique 3: Statistical Profiling**

For frequently executed code, sample multiple executions:

```
Execution 1: 1,234 VDC cycles (348 µs)
Execution 2: 1,189 VDC cycles (335 µs)
Execution 3: 1,256 VDC cycles (354 µs)
Average: 1,226 VDC cycles (346 µs)
```

This accounts for variations due to conditional branches.

#### Performance Optimization Guidelines

**1. Minimize Dual-Cycle Instructions**

Dual-cycle instructions take twice as long as single-cycle instructions. Reduce their use in critical paths:

**Before:**
```
MOV A, #0x42        ; 1 cycle
OUTL P1, A          ; 2 cycles
MOV A, #0x50        ; 1 cycle
OUTL P1, A          ; 2 cycles
Total: 6 cycles
```

**After (if possible):**
```
MOV A, #0x42        ; 1 cycle
MOV R0, A           ; 1 cycle
MOV A, #0x50        ; 1 cycle
MOV R1, A           ; 1 cycle
; Batch output later
Total: 4 cycles (33% faster)
```

**2. Optimize Loops**

Loops execute many times, so small optimizations have large impact:

**Before:**
```
LOOP:
    MOV A, @R0      ; 1 cycle
    ADD A, #0x01    ; 1 cycle
    MOV @R0, A      ; 1 cycle
    INC R0          ; 1 cycle
    MOV A, R1       ; 1 cycle
    DEC A           ; 1 cycle
    MOV R1, A       ; 1 cycle
    JNZ LOOP        ; 2 cycles
Total: 9 cycles per iteration
```

**After:**
```
LOOP:
    MOV A, @R0      ; 1 cycle
    INC A           ; 1 cycle (combined ADD and INC)
    MOV @R0, A      ; 1 cycle
    INC R0          ; 1 cycle
    DJNZ R1, LOOP   ; 2 cycles (combined DEC and JNZ)
Total: 6 cycles per iteration (33% faster)
```

**3. Reduce VDC Writes**

VDC writes (OUTL instructions) are dual-cycle. Minimize unnecessary writes:

- Only write registers that changed
- Batch updates during VBLANK
- Avoid redundant writes of same value

**4. Use Register Bank Switching**

The 8048 has two register banks (R0-R7 and R0'-R7'). Switching banks is fast:

```
SEL RB1             ; Switch to bank 1 (1 cycle)
; Use R0'-R7' for temporary work
SEL RB0             ; Switch back to bank 0 (1 cycle)
```

This avoids saving/restoring registers (faster than stack operations).

#### Common Timing Patterns

**Pattern 1: VBLANK Update Routine**

```
; Wait for VBLANK
WAIT_VBLANK:
    IN A, P2            ; Read VDC status (2 cycles)
    JB7 WAIT_VBLANK     ; Loop if not in VBLANK (2 cycles)

; Update VDC registers during VBLANK
UPDATE_VDC:
    MOV A, sprite0_x    ; Load sprite 0 X (1 cycle)
    OUTL P1, A          ; Write to VDC (2 cycles)
    MOV A, sprite0_y    ; Load sprite 0 Y (1 cycle)
    OUTL P1, A          ; Write to VDC (2 cycles)
    ; ... more updates ...
```

**Timing Considerations:**
- VBLANK wait loop: 4 cycles per iteration
- Maximum wait: ~5,967 cycles (one full frame)
- Update routine should complete within VBLANK (1,609 cycles)

**Pattern 2: Delay Loop**

```
; Delay for N instruction cycles
DELAY:
    MOV R0, #N          ; 1 cycle
DELAY_LOOP:
    DJNZ R0, DELAY_LOOP ; 2 cycles per iteration
```

**Timing:**
- Setup: 1 cycle
- Loop: N × 2 cycles
- Total: 1 + (N × 2) cycles

**Pattern 3: Timed Sequence**

```
; Execute code at specific scanline
WAIT_SCANLINE_100:
    IN A, P2            ; Read VDC status (2 cycles)
    ; Check if scanline == 100
    ; ... comparison logic ...
    JNZ WAIT_SCANLINE_100

; Execute timed code
    MOV A, #0x05        ; Change color mid-frame
    OUTL P1, A
```

#### Timing Analysis Checklist

When analyzing instruction timing in traces:

- [ ] Identify start and end points of code sequence
- [ ] Count single-cycle and dual-cycle instructions
- [ ] Calculate total instruction cycles
- [ ] Convert to execution time (µs)
- [ ] Compare with frame budget (16.79 ms NTSC, 20.03 ms PAL)
- [ ] Identify bottlenecks (>10% of frame time)
- [ ] Check for timing-dependent code (VBLANK waits, delays)
- [ ] Verify VDC writes occur during appropriate periods
- [ ] Look for optimization opportunities (loops, redundant operations)
- [ ] Measure actual execution time using VDC cycle counts

#### Practical Example: Analyzing a Performance Issue

**Problem:** Game runs slower than expected (50 Hz instead of 60 Hz on NTSC).

**Step 1: Capture Full Frame Trace**

```bash
videopac --trace --trace-start-frame 100 --trace-frames 1 --trace-output frame_trace.txt bios.bin game.bin
```

**Step 2: Measure Frame Time**

```
Frame:0100 Scanline:000 Cycle:00000 | Frame Start
Frame:0100 Scanline:261 Cycle:00226 | Frame End
Frame:0101 Scanline:000 Cycle:00000 | Next Frame Start
```

**Step 3: Calculate CPU Cycles Used**

Count instructions in trace:
- Total instructions: 7,200
- Estimated dual-cycle: 1,200 (based on CALL, JMP, OUTL frequency)
- Estimated single-cycle: 6,000
- Total cycles: 6,000 + (1,200 × 2) = 8,400 cycles

**Step 4: Identify Bottleneck**

```
Frame:0100 Scanline:050 Cycle:00000 PC:0x0800 | CALL UpdateEnemies
Frame:0100 Scanline:120 Cycle:00100 PC:0x0850 | RET
```

- UpdateEnemies takes: 70 scanlines × 227 + 100 = 15,990 VDC cycles
- Execution time: 15,990 × 0.282 µs = 4,509 µs
- Percentage of frame: 4,509 ÷ 16,790 = 26.9% of frame time
- **Conclusion:** UpdateEnemies is the bottleneck

**Step 5: Analyze Bottleneck**

Examine UpdateEnemies disassembly and trace to find:
- Nested loops
- Redundant calculations
- Inefficient algorithms

**Step 6: Optimize**

Apply optimization techniques:
- Reduce loop iterations
- Cache repeated calculations
- Use faster instruction sequences

**Step 7: Verify**

Capture new trace and verify frame time is within budget.

#### References

- [Videopac Timing Model](#71-videopac-timing-model) - System timing fundamentals
- [Trace Analysis](#4-trace-analysis) - Capturing and reading traces
- [Intel 8048 Architecture](#131-intel-8048-architecture) - Instruction cycle details
- [Common Timing Issues](#77-common-timing-issues) - Debugging timing problems

### 7.3 VBLANK Identification

Identifying VBLANK (Vertical Blanking) periods in execution traces is crucial for debugging graphics issues, analyzing VDC register write timing, and understanding game synchronization behavior. VBLANK is the period when the video beam returns from the bottom of the screen to the top, and it's the safest time to update VDC registers without causing visual artifacts.

#### What is VBLANK?

**VBLANK (Vertical Blanking Interval)** is the period between video frames when the CRT electron beam returns from the bottom-right of the screen to the top-left to begin drawing the next frame. During this time:

- No visible pixels are being drawn
- VDC registers can be safely updated without causing tearing or flickering
- The CPU has a dedicated window for graphics updates
- Collision detection registers can be read and cleared

**Duration:**
- **VBLANK lasts 70 scanlines** for both NTSC and PAL
- **NTSC:** 70 × 64.1 µs = 4.49 ms (26.7% of frame time)
- **PAL:** 70 × 64.1 µs = 4.49 ms (22.4% of frame time)

**CPU Cycles Available:**
- **NTSC:** ~1,609 instruction cycles during VBLANK
- **PAL:** ~1,768 instruction cycles during VBLANK

#### VBLANK Scanline Numbers

The scanline numbers that indicate VBLANK differ between NTSC and PAL systems:

**NTSC (North America, Japan):**
- **Active Display:** Scanlines 0-191 (192 scanlines)
- **VBLANK Start:** Scanline 192
- **VBLANK End:** Scanline 261 (last scanline of frame)
- **Total Scanlines:** 262 per frame

**PAL (Europe, Australia):**
- **Active Display:** Scanlines 0-241 (242 scanlines)
- **VBLANK Start:** Scanline 242
- **VBLANK End:** Scanline 311 or 312 (alternates between frames)
- **Total Scanlines:** 312 or 313 per frame (alternating)

**Visual Representation:**

```
NTSC Frame Structure:
┌─────────────────────────────────┐
│ Scanline 0-191: Active Display │  192 scanlines (visible)
│ (Visible pixels being drawn)    │
├─────────────────────────────────┤
│ Scanline 192-261: VBLANK        │  70 scanlines (blanking)
│ (Beam return, safe for updates) │
└─────────────────────────────────┘

PAL Frame Structure:
┌─────────────────────────────────┐
│ Scanline 0-241: Active Display │  242 scanlines (visible)
│ (Visible pixels being drawn)    │
├─────────────────────────────────┤
│ Scanline 242-311: VBLANK        │  70 scanlines (blanking)
│ (Beam return, safe for updates) │
└─────────────────────────────────┘
```

#### Identifying VBLANK Start in Traces

**Method 1: Scanline Number**

The most straightforward way to identify VBLANK is by checking the scanline number in trace output:

**NTSC Example:**
```
Frame:0001 Scanline:190 Cycle:00200 PC:0x0400 | MOV A, #0x42
Frame:0001 Scanline:191 Cycle:00100 PC:0x0402 | OUTL P1, A
Frame:0001 Scanline:192 Cycle:00000 PC:0x0404 | CALL UpdateVDC    ← VBLANK STARTS
Frame:0001 Scanline:193 Cycle:00050 PC:0x0500 | MOV A, sprite0_x
Frame:0001 Scanline:194 Cycle:00100 PC:0x0502 | OUTL P1, A
```

**Identification:**
- Scanline 191 is the last active display scanline
- Scanline 192 marks the start of VBLANK
- Any VDC writes from scanline 192 onward are during VBLANK

**PAL Example:**
```
Frame:0001 Scanline:240 Cycle:00200 PC:0x0400 | MOV A, #0x42
Frame:0001 Scanline:241 Cycle:00100 PC:0x0402 | OUTL P1, A
Frame:0001 Scanline:242 Cycle:00000 PC:0x0404 | CALL UpdateVDC    ← VBLANK STARTS
Frame:0001 Scanline:243 Cycle:00050 PC:0x0500 | MOV A, sprite0_x
Frame:0001 Scanline:244 Cycle:00100 PC:0x0502 | OUTL P1, A
```

**Quick Reference:**
- **NTSC:** Scanline ≥ 192 = VBLANK
- **PAL:** Scanline ≥ 242 = VBLANK

#### Identifying VBLANK End in Traces

VBLANK ends when the frame transitions to the next frame (scanline returns to 0):

**NTSC Example:**
```
Frame:0001 Scanline:259 Cycle:00200 PC:0x0600 | MOV A, #0x10
Frame:0001 Scanline:260 Cycle:00100 PC:0x0602 | OUTL P1, A
Frame:0001 Scanline:261 Cycle:00200 PC:0x0604 | RET              ← Last scanline
Frame:0002 Scanline:000 Cycle:00000 PC:0x0606 | JMP MainLoop     ← VBLANK ENDS, new frame
Frame:0002 Scanline:001 Cycle:00050 PC:0x0400 | CALL GameLogic
```

**Identification:**
- Scanline 261 is the last VBLANK scanline (NTSC)
- Frame number increments from 0001 to 0002
- Scanline resets to 0, marking the start of active display
- VBLANK has ended; avoid VDC writes until next VBLANK

**PAL Example:**
```
Frame:0001 Scanline:310 Cycle:00200 PC:0x0600 | MOV A, #0x10
Frame:0001 Scanline:311 Cycle:00100 PC:0x0602 | OUTL P1, A
Frame:0002 Scanline:000 Cycle:00000 PC:0x0604 | JMP MainLoop     ← VBLANK ENDS, new frame
Frame:0002 Scanline:001 Cycle:00050 PC:0x0400 | CALL GameLogic
```

#### Finding VBLANK Periods in Traces

**Technique 1: Grep/Search for Scanline Transitions**

Use text search tools to find VBLANK boundaries:

**NTSC:**
```bash
# Find VBLANK start (scanline 192)
grep "Scanline:192 Cycle:00000" trace.txt

# Find VBLANK end (frame transition)
grep "Scanline:000 Cycle:00000" trace.txt
```

**PAL:**
```bash
# Find VBLANK start (scanline 242)
grep "Scanline:242 Cycle:00000" trace.txt

# Find VBLANK end (frame transition)
grep "Scanline:000 Cycle:00000" trace.txt
```

**Technique 2: Filter VDC Writes During VBLANK**

To see only VDC register writes that occur during VBLANK:

**NTSC:**
```bash
# Extract VDC writes during VBLANK (scanlines 192-261)
grep "VDC_WRITE" trace.txt | grep -E "Scanline:(19[2-9]|2[0-5][0-9]|26[0-1])"
```

**PAL:**
```bash
# Extract VDC writes during VBLANK (scanlines 242-311)
grep "VDC_WRITE" trace.txt | grep -E "Scanline:(24[2-9]|2[5-9][0-9]|3[0-1][0-9])"
```

**Technique 3: Visual Inspection**

When viewing traces in a text editor:

1. Search for frame transitions (Frame:XXXX Scanline:000)
2. Scroll backward to find the previous frame's VBLANK start
3. Note the scanline range for VBLANK
4. Look for VDC_WRITE events in that range

#### Verifying Code Executes During VBLANK

**Problem:** You want to verify that a specific VDC update routine executes during VBLANK.

**Step 1: Identify the Routine in Disassembly**

```assembly
; UpdateSprites routine at 0x0500
0x0500: MOV A, sprite0_x    ; Load sprite 0 X position
0x0502: OUTL P1, A          ; Write to VDC
0x0504: MOV A, sprite0_y    ; Load sprite 0 Y position
0x0506: OUTL P1, A          ; Write to VDC
0x0508: RET                 ; Return
```

**Step 2: Find Routine Calls in Trace**

```bash
# Search for calls to UpdateSprites
grep "PC:0x0500" trace.txt
```

**Step 3: Check Scanline Numbers**

```
Frame:0001 Scanline:195 Cycle:00100 PC:0x0500 | MOV A, sprite0_x    ← During VBLANK (195 ≥ 192)
Frame:0001 Scanline:195 Cycle:00110 PC:0x0502 | OUTL P1, A
Frame:0001 Scanline:195 Cycle:00130 PC:0x0504 | MOV A, sprite0_y
Frame:0001 Scanline:195 Cycle:00140 PC:0x0506 | OUTL P1, A
Frame:0001 Scanline:195 Cycle:00160 PC:0x0508 | RET
```

**Verification:**
- All instructions execute at scanline 195
- Scanline 195 ≥ 192 (NTSC VBLANK start)
- **Conclusion:** UpdateSprites executes during VBLANK ✓

**Counter-Example (Mid-Frame Update):**

```
Frame:0001 Scanline:100 Cycle:00100 PC:0x0500 | MOV A, sprite0_x    ← During active display (100 < 192)
Frame:0001 Scanline:100 Cycle:00110 PC:0x0502 | OUTL P1, A
Frame:0001 Scanline:100 Cycle:00130 PC:0x0504 | MOV A, sprite0_y
Frame:0001 Scanline:100 Cycle:00140 PC:0x0506 | OUTL P1, A
Frame:0001 Scanline:100 Cycle:00160 PC:0x0508 | RET
```

**Verification:**
- All instructions execute at scanline 100
- Scanline 100 < 192 (before VBLANK)
- **Conclusion:** UpdateSprites executes during active display (mid-frame) ⚠️
- **Impact:** May cause visual artifacts (tearing, flickering)

#### Common VBLANK Patterns in Game Code

**Pattern 1: VBLANK Wait Loop**

Many games wait for VBLANK before updating VDC registers:

```assembly
; Wait for VBLANK to start
WaitVBLANK:
    IN A, P2            ; Read VDC status register
    JB7 WaitVBLANK      ; Loop if bit 7 is not set (not in VBLANK)
    
; Now in VBLANK, safe to update VDC
UpdateVDC:
    MOV A, #0x42
    OUTL P1, A
    ; ... more updates ...
```

**Trace Pattern:**
```
Frame:0001 Scanline:189 Cycle:00100 PC:0x0400 | IN A, P2          ← Waiting
Frame:0001 Scanline:189 Cycle:00120 PC:0x0402 | JB7 0x0400        ← Loop back
Frame:0001 Scanline:190 Cycle:00100 PC:0x0400 | IN A, P2          ← Still waiting
Frame:0001 Scanline:190 Cycle:00120 PC:0x0402 | JB7 0x0400        ← Loop back
Frame:0001 Scanline:192 Cycle:00000 PC:0x0400 | IN A, P2          ← VBLANK starts
Frame:0001 Scanline:192 Cycle:00020 PC:0x0404 | MOV A, #0x42      ← Exit loop, start updates
Frame:0001 Scanline:192 Cycle:00030 PC:0x0406 | OUTL P1, A
```

**Identification:**
- Tight loop reading VDC status (IN A, P2)
- Loop exits when scanline transitions to VBLANK (192 or 242)
- VDC updates immediately follow

**Pattern 2: Frame Counter Synchronization**

Some games use a frame counter to synchronize with VBLANK:

```assembly
MainLoop:
    CALL GameLogic      ; Update game state
    CALL WaitVBLANK     ; Wait for VBLANK
    CALL UpdateVDC      ; Update graphics during VBLANK
    INC FrameCounter    ; Increment frame counter
    JMP MainLoop        ; Repeat
```

**Trace Pattern:**
```
Frame:0001 Scanline:150 Cycle:00000 PC:0x0300 | CALL GameLogic
Frame:0001 Scanline:180 Cycle:00100 PC:0x0302 | CALL WaitVBLANK
Frame:0001 Scanline:192 Cycle:00050 PC:0x0304 | CALL UpdateVDC    ← VBLANK
Frame:0001 Scanline:200 Cycle:00000 PC:0x0306 | INC FrameCounter
Frame:0001 Scanline:200 Cycle:00010 PC:0x0308 | JMP MainLoop
```

**Pattern 3: Immediate VBLANK Updates**

Some games update VDC registers immediately at VBLANK start:

```
Frame:0001 Scanline:191 Cycle:00226 PC:0x0500 | NOP               ← Last cycle of scanline 191
Frame:0001 Scanline:192 Cycle:00000 PC:0x0502 | MOV A, #0x42      ← First cycle of VBLANK
Frame:0001 Scanline:192 Cycle:00010 PC:0x0504 | OUTL P1, A        ← Immediate VDC write
```

**Identification:**
- VDC write occurs at scanline 192, cycle 0 or very early cycles
- No wait loop visible
- Game is precisely synchronized with frame timing

#### Debugging VBLANK Timing Issues

**Issue 1: VDC Writes Outside VBLANK**

**Symptom:** Visual artifacts (tearing, flickering, incorrect colors)

**Diagnosis:**
```
Frame:0001 Scanline:100 Cycle:00100 | VDC_WRITE[0xAB] = 0x02    ← Mid-frame write
```

**Analysis:**
- Scanline 100 < 192 (NTSC) - not in VBLANK
- VDC write during active display
- May cause visible artifact

**Solution:**
- Move VDC write to VBLANK period
- Add VBLANK wait loop before updates
- Verify timing in trace

**Issue 2: VBLANK Window Too Short**

**Symptom:** Some VDC updates don't complete before VBLANK ends

**Diagnosis:**
```
Frame:0001 Scanline:192 Cycle:00000 PC:0x0500 | CALL UpdateVDC    ← VBLANK starts
Frame:0001 Scanline:260 Cycle:00100 PC:0x0550 | OUTL P1, A        ← Still updating
Frame:0001 Scanline:261 Cycle:00200 PC:0x0552 | OUTL P1, A        ← Last scanline
Frame:0002 Scanline:000 Cycle:00000 PC:0x0554 | OUTL P1, A        ← VBLANK ended! ⚠️
```

**Analysis:**
- UpdateVDC routine takes 69 scanlines (192-260)
- Last write occurs after VBLANK ends (scanline 0 of next frame)
- VDC write during active display may cause artifact

**Solution:**
- Optimize UpdateVDC routine to complete faster
- Reduce number of VDC register writes
- Split updates across multiple frames if necessary

**Issue 3: Missing VBLANK Wait**

**Symptom:** Inconsistent graphics updates, timing-dependent bugs

**Diagnosis:**
```
Frame:0001 Scanline:050 Cycle:00000 PC:0x0400 | CALL UpdateVDC    ← No VBLANK wait
Frame:0001 Scanline:050 Cycle:00100 PC:0x0500 | MOV A, #0x42
Frame:0001 Scanline:050 Cycle:00110 PC:0x0502 | OUTL P1, A        ← Mid-frame write
```

**Analysis:**
- UpdateVDC called without waiting for VBLANK
- VDC writes occur during active display
- Timing depends on when game logic completes

**Solution:**
- Add VBLANK wait loop before UpdateVDC
- Ensure all VDC updates occur during VBLANK

#### VBLANK Identification Checklist

When analyzing traces for VBLANK timing:

- [ ] Identify video standard (NTSC or PAL) from emulator configuration
- [ ] Note VBLANK scanline range (192-261 NTSC, 242-311 PAL)
- [ ] Search trace for VBLANK start (scanline 192 or 242)
- [ ] Search trace for VBLANK end (frame transition to scanline 0)
- [ ] Filter VDC writes and check their scanline numbers
- [ ] Verify critical VDC writes occur during VBLANK
- [ ] Check for VBLANK wait loops in code
- [ ] Measure time spent in VDC update routines
- [ ] Verify updates complete before VBLANK ends
- [ ] Identify any mid-frame VDC writes and assess impact

#### Practical Examples

**Example 1: Finding VBLANK in a Trace**

```bash
# Capture one frame trace
videopac --trace --trace-start-frame 100 --trace-frames 1 --trace-output frame.txt bios.bin game.bin

# Find VBLANK start (NTSC)
grep "Scanline:192 Cycle:00000" frame.txt
```

**Output:**
```
Frame:0100 Scanline:192 Cycle:00000 PC:0x0450 | CALL UpdateSprites
```

**Conclusion:** UpdateSprites is called exactly at VBLANK start.

**Example 2: Verifying All VDC Writes Are During VBLANK**

```bash
# Extract all VDC writes with scanline numbers
grep "VDC_WRITE" frame.txt | grep -o "Scanline:[0-9]*" | sort -u
```

**Output:**
```
Scanline:195
Scanline:196
Scanline:197
Scanline:200
```

**Analysis:**
- All scanlines are ≥ 192 (NTSC VBLANK)
- All VDC writes occur during VBLANK ✓
- No mid-frame updates detected

**Example 3: Identifying Mid-Frame Updates**

```bash
# Find VDC writes during active display (NTSC: scanlines 0-191)
grep "VDC_WRITE" frame.txt | grep -E "Scanline:(0[0-9]{2}|1[0-8][0-9]|19[0-1])"
```

**Output:**
```
Frame:0100 Scanline:100 Cycle:00150 | VDC_WRITE[0xAB] = 0x05
Frame:0100 Scanline:150 Cycle:00200 | VDC_WRITE[0xAC] = 0x03
```

**Analysis:**
- Two VDC writes during active display (scanlines 100, 150)
- These are mid-frame updates
- May be intentional (effects) or bugs (artifacts)
- Investigate further to determine purpose

#### References

- [Videopac Timing Model](#71-videopac-timing-model) - Detailed timing specifications
- [Trace Analysis](#4-trace-analysis) - Capturing and filtering traces
- [Graphics Debugging](#5-graphics-debugging) - Debugging visual artifacts
- [Intel 8245 VDC Architecture](#132-intel-8245-vdc-architecture) - VDC timing details

### 7.4 Mid-Frame Update Analysis

Mid-frame updates occur when games write to VDC registers during the active display period (outside of VBLANK), while the screen is actively being drawn. Understanding mid-frame updates is essential for debugging visual artifacts, analyzing advanced graphics techniques, and distinguishing between intentional effects and timing bugs.

#### What Are Mid-Frame Updates?

**Mid-Frame Update** refers to any VDC register write that occurs during the active display period, when scanlines are being rendered to the screen.

**Active Display Periods:**
- **NTSC:** Scanlines 0-191 (192 scanlines)
- **PAL:** Scanlines 0-241 (242 scanlines)

**Characteristics:**
- VDC register changes take effect immediately
- Can cause visible artifacts if not carefully timed
- Used for advanced effects (sprite multiplexing, color changes, split-screen)
- May indicate timing bugs if unintentional

**Contrast with VBLANK Updates:**

| Aspect | VBLANK Updates | Mid-Frame Updates |
|--------|----------------|-------------------|
| Timing | Scanlines 192-261 (NTSC) or 242-311 (PAL) | Scanlines 0-191 (NTSC) or 0-241 (PAL) |
| Safety | Safe, no visual artifacts | Risky, can cause artifacts |
| Use Case | Standard graphics updates | Special effects or bugs |
| Visibility | Changes appear next frame | Changes appear immediately |
| Complexity | Simple, predictable | Complex, requires precise timing |

#### Why Games Use Mid-Frame Updates

**Intentional Use Cases:**

**1. Sprite Multiplexing**

Reusing sprite hardware to display more sprites than the VDC supports (4 sprites):

```
Scanline 50:  Sprite 0 shows enemy #1 at Y=50
Scanline 100: Sprite 0 repositioned to show enemy #2 at Y=100
Scanline 150: Sprite 0 repositioned to show enemy #3 at Y=150
```

This creates the illusion of 3 sprites using only 1 sprite register.

**2. Color Palette Changes**

Changing background or sprite colors mid-frame for visual effects:

```
Scanline 0-95:   Sky area (blue background)
Scanline 96:     Change to green background
Scanline 96-191: Ground area (green background)
```

Creates a horizon line effect with only one background color register.

**3. Horizontal Scrolling Effects**

Updating grid position registers mid-frame to create parallax scrolling:

```
Scanline 0-63:   Grid X offset = 0 (foreground)
Scanline 64-127: Grid X offset = 2 (middle ground)
Scanline 128-191: Grid X offset = 4 (background)
```

Creates depth illusion with different scroll speeds.

**4. Status Bar Separation**

Keeping a status bar stable while scrolling the playfield:

```
Scanline 0-31:   Status bar (grid offset = 0)
Scanline 32:     Update grid offset for scrolling
Scanline 32-191: Playfield (grid offset = scroll_position)
```

**5. Sprite Position Adjustments**

Fine-tuning sprite positions during rendering for smooth animation:

```
Scanline 80: Sprite Y position = 80
Scanline 81: Sprite Y position = 81 (smooth vertical movement)
```

#### Identifying Mid-Frame Updates in Traces

**Method 1: Scanline Number Check**

The most direct way to identify mid-frame updates is by checking the scanline number:

**NTSC Example:**
```
Frame:0001 Scanline:100 Cycle:00150 PC:0x0500 | VDC_WRITE[0xAB] = 0x02    ← Mid-frame
Frame:0001 Scanline:195 Cycle:00100 PC:0x0520 | VDC_WRITE[0xAC] = 0x03    ← VBLANK
```

**Analysis:**
- First write: Scanline 100 < 192 → Mid-frame update
- Second write: Scanline 195 ≥ 192 → VBLANK update

**PAL Example:**
```
Frame:0001 Scanline:150 Cycle:00200 PC:0x0500 | VDC_WRITE[0xA3] = 0x05    ← Mid-frame
Frame:0001 Scanline:250 Cycle:00100 PC:0x0520 | VDC_WRITE[0xA4] = 0x10    ← VBLANK
```

**Analysis:**
- First write: Scanline 150 < 242 → Mid-frame update
- Second write: Scanline 250 ≥ 242 → VBLANK update

**Method 2: Filtering VDC Writes by Scanline Range**

Use grep or text search to extract only mid-frame VDC writes:

**NTSC:**
```bash
# Extract VDC writes during active display (scanlines 0-191)
grep "VDC_WRITE" trace.txt | grep -E "Scanline:(0[0-9]{2}|1[0-8][0-9]|19[0-1])"
```

**PAL:**
```bash
# Extract VDC writes during active display (scanlines 0-241)
grep "VDC_WRITE" trace.txt | grep -E "Scanline:(0[0-9]{2}|1[0-9]{2}|2[0-3][0-9]|24[0-1])"
```

**Method 3: Visual Pattern Recognition**

When viewing traces, look for patterns:

**Pattern: Repeated Updates at Regular Intervals**
```
Frame:0001 Scanline:050 Cycle:00100 | VDC_WRITE[0xA0] = 0x10
Frame:0001 Scanline:100 Cycle:00100 | VDC_WRITE[0xA0] = 0x20
Frame:0001 Scanline:150 Cycle:00100 | VDC_WRITE[0xA0] = 0x30
```

**Interpretation:** Likely intentional effect (color cycling, sprite multiplexing)

**Pattern: Isolated Mid-Frame Write**
```
Frame:0001 Scanline:010 Cycle:00050 | VDC_WRITE[0xAB] = 0x02
Frame:0001 Scanline:195 Cycle:00100 | VDC_WRITE[0xAC] = 0x03
Frame:0001 Scanline:196 Cycle:00050 | VDC_WRITE[0xAD] = 0x04
```

**Interpretation:** Possibly unintentional (timing bug, early write)

#### Analyzing Mid-Frame Update Timing

**Technique 1: Scanline Correlation**

Correlate VDC writes with specific scanline positions to understand intent:

**Example: Sprite Multiplexing**
```
Frame:0001 Scanline:040 Cycle:00100 | VDC_WRITE[0x10] = 0x40    ; Sprite 0 Y = 64
Frame:0001 Scanline:040 Cycle:00120 | VDC_WRITE[0x14] = 0x20    ; Sprite 0 X = 32
Frame:0001 Scanline:090 Cycle:00100 | VDC_WRITE[0x10] = 0x90    ; Sprite 0 Y = 144
Frame:0001 Scanline:090 Cycle:00120 | VDC_WRITE[0x14] = 0x40    ; Sprite 0 X = 64
```

**Analysis:**
- First update at scanline 40: Sprite appears at Y=64 (scanline 64)
- Second update at scanline 90: Sprite repositioned to Y=144 (scanline 144)
- Updates occur ~24 scanlines before sprite appears
- **Conclusion:** Intentional sprite multiplexing with proper timing

**Technique 2: Register Pattern Analysis**

Identify which registers are being updated mid-frame:

**Sprite Registers (0x10-0x1F, 0xAB-0xAE):**
- Common for sprite multiplexing
- Position updates (X, Y) or color changes
- Usually intentional if timed with sprite Y position

**Color Registers (0xA3, 0xAB-0xAE):**
- Common for color cycling effects
- Background color changes for horizon effects
- Usually intentional if regular pattern

**Grid Registers (0xA4, 0xA5, 0xAF):**
- Common for scrolling effects
- Position updates for parallax
- Usually intentional if synchronized with scanlines

**Control Registers (0xA0, 0xA1):**
- Rare mid-frame
- May indicate timing bug if unexpected

**Technique 3: Cycle-Accurate Timing**

Calculate exact timing of mid-frame updates using VDC cycle counts:

**Example:**
```
Frame:0001 Scanline:100 Cycle:00150 | VDC_WRITE[0xA3] = 0x05
```

**Calculation:**
- Scanline 100, Cycle 150
- Total VDC cycles from frame start: (100 × 227) + 150 = 22,850 cycles
- Time from frame start: 22,850 × 0.282 µs = 6.44 ms
- Horizontal position: 150 ÷ 227 = 66% across scanline
- **Interpretation:** Update occurs 2/3 through scanline 100

**Technique 4: Frame-by-Frame Comparison**

Compare mid-frame update patterns across multiple frames:

**Frame 1:**
```
Frame:0001 Scanline:050 Cycle:00100 | VDC_WRITE[0xA3] = 0x02
Frame:0001 Scanline:100 Cycle:00100 | VDC_WRITE[0xA3] = 0x05
```

**Frame 2:**
```
Frame:0002 Scanline:050 Cycle:00100 | VDC_WRITE[0xA3] = 0x02
Frame:0002 Scanline:100 Cycle:00100 | VDC_WRITE[0xA3] = 0x05
```

**Analysis:**
- Identical pattern in both frames
- Same scanlines, same values
- **Conclusion:** Intentional, repeatable effect

#### Visual Effects and Artifacts

**Intentional Effects:**

**1. Sprite Multiplexing Appearance**

When done correctly:
- Multiple sprites appear on screen (more than 4)
- Sprites at different Y positions
- No flickering or tearing
- Smooth animation

**Trace Signature:**
```
Scanline:030 | VDC_WRITE[0x10] = 0x30    ; Sprite 0 at Y=48
Scanline:080 | VDC_WRITE[0x10] = 0x80    ; Sprite 0 at Y=128
Scanline:130 | VDC_WRITE[0x10] = 0xD0    ; Sprite 0 at Y=208
```

**2. Color Cycling Effect**

When done correctly:
- Smooth color transitions
- Synchronized with scanlines
- No color bleeding between areas

**Trace Signature:**
```
Scanline:000 | VDC_WRITE[0xA3] = 0x01    ; Blue
Scanline:050 | VDC_WRITE[0xA3] = 0x03    ; Cyan
Scanline:100 | VDC_WRITE[0xA3] = 0x05    ; Green
```

**3. Parallax Scrolling Effect**

When done correctly:
- Different scroll speeds for different screen areas
- Smooth scrolling
- No tearing at boundaries

**Trace Signature:**
```
Scanline:000 | VDC_WRITE[0xA5] = 0x00    ; Foreground offset
Scanline:064 | VDC_WRITE[0xA5] = 0x02    ; Midground offset
Scanline:128 | VDC_WRITE[0xA5] = 0x04    ; Background offset
```

**Unintentional Artifacts:**

**1. Tearing**

**Symptom:** Horizontal line where image appears split or misaligned

**Cause:** VDC register updated while that scanline is being drawn

**Trace Example:**
```
Frame:0001 Scanline:100 Cycle:00150 | VDC_WRITE[0xA5] = 0x10    ; Grid X position
```

**Analysis:**
- Update occurs mid-scanline (cycle 150 of 227)
- Scanline 100 partially drawn with old value, partially with new value
- Creates visible horizontal tear at scanline 100

**2. Flickering**

**Symptom:** Sprite or color rapidly changes, appears to flicker

**Cause:** Mid-frame update timing varies frame-to-frame

**Trace Example:**
```
Frame:0001 Scanline:100 Cycle:00100 | VDC_WRITE[0xAB] = 0x02
Frame:0002 Scanline:105 Cycle:00150 | VDC_WRITE[0xAB] = 0x02
Frame:0003 Scanline:095 Cycle:00200 | VDC_WRITE[0xAB] = 0x02
```

**Analysis:**
- Same register, same value, but different timing each frame
- Inconsistent timing causes sprite to appear at slightly different positions
- Creates flickering effect

**3. Color Bleeding**

**Symptom:** Wrong color appears in part of screen

**Cause:** Color register updated too late or too early

**Trace Example:**
```
Frame:0001 Scanline:096 Cycle:00200 | VDC_WRITE[0xA3] = 0x05    ; Change to green
```

**Analysis:**
- Update occurs late in scanline 96 (cycle 200 of 227)
- Scanline 96 mostly drawn with old color
- Color change takes effect partway through scanline
- Creates color bleeding artifact

**4. Sprite Duplication**

**Symptom:** Sprite appears in two places simultaneously

**Cause:** Sprite position updated too late (after sprite already drawn)

**Trace Example:**
```
Frame:0001 Scanline:080 Cycle:00200 | VDC_WRITE[0x10] = 0x80    ; Sprite Y = 128
```

**Analysis:**
- Sprite 0 was at Y=64, already drawn at scanline 64
- Position updated to Y=128 at scanline 80
- Sprite appears at both Y=64 (old position) and Y=128 (new position)
- Creates duplication artifact

#### Distinguishing Intentional vs Unintentional Updates

**Indicators of Intentional Mid-Frame Updates:**

1. **Regular Pattern:** Updates occur at consistent scanlines across frames
2. **Logical Timing:** Updates synchronized with sprite Y positions or screen areas
3. **Multiple Updates:** Series of updates creating a pattern (multiplexing, color cycling)
4. **Specific Registers:** Sprite position/color registers (common for effects)
5. **No Artifacts:** Visual output looks correct, no tearing or flickering

**Example: Intentional Sprite Multiplexing**
```
Frame:0001 Scanline:040 | VDC_WRITE[0x10] = 0x40    ; Sprite Y = 64
Frame:0001 Scanline:090 | VDC_WRITE[0x10] = 0x90    ; Sprite Y = 144
Frame:0002 Scanline:040 | VDC_WRITE[0x10] = 0x40    ; Sprite Y = 64
Frame:0002 Scanline:090 | VDC_WRITE[0x10] = 0x90    ; Sprite Y = 144
```

**Indicators of Unintentional Mid-Frame Updates:**

1. **Irregular Pattern:** Updates occur at different scanlines each frame
2. **Illogical Timing:** Updates not synchronized with any visible element
3. **Isolated Updates:** Single update without pattern or purpose
4. **Unexpected Registers:** Control or status registers (rare mid-frame)
5. **Visible Artifacts:** Tearing, flickering, color bleeding, duplication

**Example: Unintentional Early Update (Bug)**
```
Frame:0001 Scanline:010 | VDC_WRITE[0xAB] = 0x02    ; Too early!
Frame:0001 Scanline:195 | VDC_WRITE[0xAC] = 0x03    ; VBLANK updates
Frame:0001 Scanline:196 | VDC_WRITE[0xAD] = 0x04
```

**Analysis:**
- First write at scanline 10 (very early in frame)
- Remaining writes during VBLANK (195-196)
- Isolated early write suggests timing bug
- Likely intended to be in VBLANK with others

#### Debugging Mid-Frame Update Issues

**Issue 1: Unintended Mid-Frame Write**

**Symptom:** Visual artifact (tearing, flickering) appears

**Diagnosis Steps:**

1. **Capture trace of affected frame:**
```bash
videopac --trace --trace-start-frame 100 --trace-frames 1 --trace-output trace.txt bios.bin game.bin
```

2. **Filter for VDC writes during active display:**
```bash
# NTSC: scanlines 0-191
grep "VDC_WRITE" trace.txt | grep -E "Scanline:(0[0-9]{2}|1[0-8][0-9]|19[0-1])"
```

3. **Identify unexpected writes:**
```
Frame:0100 Scanline:015 Cycle:00100 | VDC_WRITE[0xAB] = 0x02    ← Unexpected
```

4. **Find code responsible:**
```bash
# Search for PC address in disassembly
grep "0x0500" game_disasm.txt
```

5. **Analyze code path:**
```assembly
0x0500: MOV A, sprite0_color
0x0502: OUTL P1, A              ; This is the mid-frame write
0x0504: CALL WaitVBLANK         ; VBLANK wait comes AFTER write (bug!)
0x0506: CALL UpdateOtherSprites
```

**Root Cause:** VBLANK wait occurs after VDC write instead of before

**Solution:** Move VBLANK wait before VDC writes
```assembly
0x0500: CALL WaitVBLANK         ; Wait FIRST
0x0502: MOV A, sprite0_color
0x0504: OUTL P1, A              ; Now writes during VBLANK
0x0506: CALL UpdateOtherSprites
```

**Issue 2: Sprite Multiplexing Timing**

**Symptom:** Sprite appears in wrong position or flickers

**Diagnosis Steps:**

1. **Identify sprite position updates:**
```
Frame:0001 Scanline:080 Cycle:00100 | VDC_WRITE[0x10] = 0x80    ; Sprite Y = 128
```

2. **Calculate when sprite will be drawn:**
- Sprite Y register = 128 (0x80)
- Sprite will be drawn at scanline 128

3. **Check update timing:**
- Update occurs at scanline 80
- Sprite drawn at scanline 128
- Gap: 128 - 80 = 48 scanlines
- **Assessment:** Good timing, update occurs 48 scanlines before sprite

**Problem Example:**
```
Frame:0001 Scanline:130 Cycle:00100 | VDC_WRITE[0x10] = 0x80    ; Sprite Y = 128
```

**Analysis:**
- Update occurs at scanline 130
- Sprite should be drawn at scanline 128
- Update is 2 scanlines LATE
- **Result:** Sprite appears at old position (duplication artifact)

**Solution:** Update sprite position earlier (before scanline 128)

**Issue 3: Color Change Timing**

**Symptom:** Color bleeding or incorrect color in part of screen

**Diagnosis Steps:**

1. **Identify color register writes:**
```
Frame:0001 Scanline:096 Cycle:00200 | VDC_WRITE[0xA3] = 0x05    ; Background color
```

2. **Check cycle position within scanline:**
- Scanline has 227 VDC cycles
- Update at cycle 200
- Position: 200 ÷ 227 = 88% through scanline
- **Assessment:** Very late in scanline, may cause bleeding

3. **Optimal timing:**
- Update at cycle 0 (start of scanline) for clean transition
- Update during HBL (cycles ~186-226) if possible

**Solution:** Update color at start of target scanline
```
Frame:0001 Scanline:096 Cycle:00000 | VDC_WRITE[0xA3] = 0x05    ; Clean transition
```

#### Advanced Analysis Techniques

**Technique 1: Scanline Histogram**

Count VDC writes per scanline to identify patterns:

```bash
# Extract scanline numbers from VDC writes
grep "VDC_WRITE" trace.txt | grep -oE "Scanline:[0-9]+" | sort | uniq -c

# Output:
#   1 Scanline:010
#   1 Scanline:050
#   1 Scanline:100
#  15 Scanline:195
#  12 Scanline:196
```

**Interpretation:**
- Scanlines 10, 50, 100: Isolated mid-frame writes (investigate)
- Scanlines 195-196: Bulk VBLANK updates (normal)

**Technique 2: Register Access Pattern**

Track which registers are written mid-frame:

```bash
# Extract register addresses from mid-frame writes
grep "VDC_WRITE" trace.txt | grep -E "Scanline:(0[0-9]{2}|1[0-8][0-9]|19[0-1])" | grep -oE "0x[A-F0-9]+"

# Output:
# 0xA3  (background color)
# 0xA3  (background color)
# 0xA3  (background color)
```

**Interpretation:**
- Only background color register (0xA3) written mid-frame
- Likely intentional color cycling effect

**Technique 3: Temporal Analysis**

Compare mid-frame update timing across multiple frames:

```bash
# Extract mid-frame writes from frames 1-10
grep "VDC_WRITE" trace.txt | grep -E "Frame:000[1-9]" | grep -E "Scanline:(0[0-9]{2}|1[0-8][0-9]|19[0-1])"
```

**Consistent Pattern (Intentional):**
```
Frame:0001 Scanline:050 | VDC_WRITE[0xA3] = 0x02
Frame:0002 Scanline:050 | VDC_WRITE[0xA3] = 0x02
Frame:0003 Scanline:050 | VDC_WRITE[0xA3] = 0x02
```

**Inconsistent Pattern (Bug):**
```
Frame:0001 Scanline:050 | VDC_WRITE[0xA3] = 0x02
Frame:0002 Scanline:055 | VDC_WRITE[0xA3] = 0x02
Frame:0003 Scanline:048 | VDC_WRITE[0xA3] = 0x02
```

#### Practical Examples

**Example 1: Analyzing Sprite Multiplexing**

**Scenario:** Game shows 8 enemies on screen, but VDC only supports 4 sprites.

**Investigation:**

1. **Capture trace:**
```bash
videopac --trace --trace-frames 1 --trace-filter vdc --trace-output vdc_trace.txt bios.bin game.bin
```

2. **Search for sprite 0 position updates:**
```bash
grep "VDC_WRITE\[0x10\]" vdc_trace.txt
```

3. **Results:**
```
Frame:0001 Scanline:030 Cycle:00050 | VDC_WRITE[0x10] = 0x30    ; Y = 48
Frame:0001 Scanline:060 Cycle:00050 | VDC_WRITE[0x10] = 0x60    ; Y = 96
Frame:0001 Scanline:090 Cycle:00050 | VDC_WRITE[0x10] = 0x90    ; Y = 144
Frame:0001 Scanline:120 Cycle:00050 | VDC_WRITE[0x10] = 0xC0    ; Y = 192
```

4. **Analysis:**
- Sprite 0 repositioned 4 times per frame
- Updates at scanlines 30, 60, 90, 120
- Sprites appear at Y positions 48, 96, 144, 192
- Each update occurs ~18 scanlines before sprite appears
- **Conclusion:** Intentional sprite multiplexing, properly timed

**Example 2: Debugging Color Artifact**

**Scenario:** Sky area shows incorrect color (green instead of blue).

**Investigation:**

1. **Capture trace:**
```bash
videopac --trace --trace-frames 1 --trace-filter vdc --trace-output vdc_trace.txt bios.bin game.bin
```

2. **Search for background color writes:**
```bash
grep "VDC_WRITE\[0xA3\]" vdc_trace.txt
```

3. **Results:**
```
Frame:0001 Scanline:005 Cycle:00100 | VDC_WRITE[0xA3] = 0x05    ; Green (unexpected!)
Frame:0001 Scanline:096 Cycle:00000 | VDC_WRITE[0xA3] = 0x01    ; Blue
Frame:0001 Scanline:195 Cycle:00050 | VDC_WRITE[0xA3] = 0x00    ; Black
```

4. **Analysis:**
- First write at scanline 5: Sets green (wrong color for sky)
- Second write at scanline 96: Sets blue (correct color)
- Sky area is scanlines 0-95, but green is set at scanline 5
- **Root Cause:** Initial color set to green instead of blue

5. **Solution:** Change initial color write to blue (0x01) instead of green (0x05)

**Example 3: Identifying Unintentional Mid-Frame Write**

**Scenario:** Occasional screen tearing appears during gameplay.

**Investigation:**

1. **Capture trace when tearing occurs:**
```bash
videopac --trace --trace-start-key "1" --trace-frames 1 --trace-output trace.txt bios.bin game.bin
```

2. **Filter mid-frame VDC writes:**
```bash
grep "VDC_WRITE" trace.txt | grep -E "Scanline:(0[0-9]{2}|1[0-8][0-9]|19[0-1])"
```

3. **Results:**
```
Frame:0001 Scanline:015 Cycle:00100 | VDC_WRITE[0xAB] = 0x02    ; Sprite 0 color
Frame:0001 Scanline:195 Cycle:00050 | VDC_WRITE[0xAC] = 0x03    ; Sprite 1 color
Frame:0001 Scanline:196 Cycle:00100 | VDC_WRITE[0xAD] = 0x04    ; Sprite 2 color
Frame:0001 Scanline:197 Cycle:00050 | VDC_WRITE[0xAE] = 0x05    ; Sprite 3 color
```

4. **Analysis:**
- Sprite 0 color written at scanline 15 (mid-frame)
- Other sprite colors written during VBLANK (195-197)
- Isolated early write suggests timing bug
- **Root Cause:** Sprite 0 color updated before VBLANK wait

5. **Find responsible code:**
```bash
grep "PC:0x0500" trace.txt | head -1
```

6. **Disassembly:**
```assembly
0x0500: MOV A, sprite0_color
0x0502: OUTL P1, A              ; Mid-frame write (bug!)
0x0504: CALL WaitVBLANK
0x0506: MOV A, sprite1_color
0x0508: OUTL P1, A              ; VBLANK write (correct)
```

7. **Solution:** Move sprite 0 color write after VBLANK wait

#### Mid-Frame Update Checklist

When analyzing mid-frame updates:

- [ ] Identify all VDC writes during active display (scanlines 0-191 NTSC, 0-241 PAL)
- [ ] Note scanline and cycle position of each write
- [ ] Identify which registers are being written
- [ ] Check for regular patterns (same scanlines across frames)
- [ ] Correlate sprite position updates with sprite Y coordinates
- [ ] Check timing: updates should occur before target scanline
- [ ] Look for visual artifacts (tearing, flickering, color bleeding)
- [ ] Distinguish intentional effects from timing bugs
- [ ] Verify code path in disassembly
- [ ] Test fix by moving writes to VBLANK or adjusting timing

#### Summary

Mid-frame updates are a powerful technique for advanced graphics effects but require precise timing to avoid artifacts. When debugging:

1. **Identify** mid-frame writes by scanline number (< 192 NTSC, < 242 PAL)
2. **Analyze** timing, register patterns, and frame-to-frame consistency
3. **Distinguish** intentional effects (regular patterns) from bugs (isolated writes)
4. **Verify** visual output matches expectations
5. **Fix** timing bugs by moving writes to VBLANK or adjusting timing

Understanding mid-frame updates is essential for debugging visual artifacts and appreciating the sophisticated techniques used by Videopac game developers.

#### References

For related information, see:
- [Videopac Timing Model](#71-videopac-timing-model) - CPU and VDC timing fundamentals
- [VBLANK Identification](#73-vblank-identification) - Identifying safe update periods
- [VDC Register Reference](#82-vdc-registers) - Register functions and addresses
- [Graphics Debugging](#5-graphics-debugging) - Debugging visual issues

### 7.5 Timing-Dependent Bug Identification

Timing-dependent bugs are among the most challenging issues to debug in emulation because they depend on the precise synchronization between CPU execution, VDC rendering, and game logic. These bugs may appear intermittently, vary between NTSC and PAL systems, or only manifest under specific conditions. This section explains what timing-dependent bugs are, how to identify them in traces, and strategies for reproducing and isolating them.

#### What Are Timing-Dependent Bugs?

**Timing-dependent bugs** are issues that arise from incorrect synchronization between different components of the system or from assumptions about execution timing that don't hold in all cases. Unlike logic bugs (which produce wrong results regardless of timing) or graphics bugs (which affect visual output), timing-dependent bugs only manifest when specific timing conditions are met.

**Characteristics of Timing-Dependent Bugs:**

1. **Intermittent:** May not occur every time the code executes
2. **Condition-Dependent:** Triggered by specific timing windows or race conditions
3. **System-Dependent:** May behave differently on NTSC vs PAL, or emulator vs real hardware
4. **Hard to Reproduce:** Difficult to trigger consistently
5. **Trace-Sensitive:** May disappear when tracing is enabled (Heisenbug effect)
6. **Frame-Dependent:** May only occur on specific frames or after certain durations

**Common Types:**

- **Race Conditions:** Two operations compete for the same resource
- **Synchronization Issues:** CPU and VDC operations not properly coordinated
- **Frame Budget Overruns:** Code takes longer than one frame to execute
- **Polling Failures:** Waiting for a condition that's missed due to timing
- **Delayed Updates:** Register updates that arrive too late to take effect
- **Timing Assumptions:** Code assumes specific execution speeds

#### Identifying Race Conditions

Race conditions occur when two operations access the same resource (register, memory location) and the outcome depends on which operation executes first.

**Example 1: VDC Register Read/Write Race**

**Symptom:** Collision detection sometimes fails to detect valid collisions

**Trace Evidence:**
```
Frame:0001 Scanline:100 Cycle:00100 PC:0x0400 | IN A, P2          ; Read collision register
Frame:0001 Scanline:100 Cycle:00120 PC:0x0402 | MOV R0, A         ; Store collision value
Frame:0001 Scanline:100 Cycle:00130 PC:0x0404 | MOV A, #0x00      ; Clear collision
Frame:0001 Scanline:100 Cycle:00140 PC:0x0406 | OUTL P2, A        ; Write to VDC
Frame:0001 Scanline:100 Cycle:00160 PC:0x0408 | JZ NoCollision    ; Check if zero
```

**Analysis:**
- Collision register read at cycle 100
- Collision register cleared at cycle 140
- Gap of 40 VDC cycles (11.3 µs)
- If new collision occurs between cycles 100-140, it's lost
- **Race Condition:** New collision can be overwritten by clear operation

**Identification Markers:**
- Read followed by write to same register
- Time gap between read and write
- Conditional logic based on read value
- Intermittent failures (when collision occurs in gap)

**Example 2: Memory Read-Modify-Write Race**

**Symptom:** Score occasionally increments by wrong amount

**Trace Evidence:**
```
; Thread 1: Add 10 points
Frame:0001 Scanline:050 Cycle:00100 PC:0x0500 | MOV A, @R0        ; Read score (100)
Frame:0001 Scanline:050 Cycle:00110 PC:0x0502 | ADD A, #0x0A      ; Add 10 (110)
Frame:0001 Scanline:050 Cycle:00120 PC:0x0504 | MOV @R0, A        ; Write score (110)

; Thread 2: Add 5 points (interrupts between read and write)
Frame:0001 Scanline:050 Cycle:00105 PC:0x0600 | MOV A, @R0        ; Read score (100)
Frame:0001 Scanline:050 Cycle:00115 PC:0x0602 | ADD A, #0x05      ; Add 5 (105)
Frame:0001 Scanline:050 Cycle:00125 PC:0x0604 | MOV @R0, A        ; Write score (105)
```

**Analysis:**
- Both operations read score = 100
- First adds 10, writes 110
- Second adds 5, writes 105 (overwrites 110!)
- Expected result: 100 + 10 + 5 = 115
- Actual result: 105
- **Race Condition:** Lost update due to interleaved execution

**Identification Markers:**
- Multiple read-modify-write sequences to same memory
- Interleaved execution (interrupts, subroutine calls)
- Final value doesn't match expected calculation
- Intermittent incorrect results

#### Identifying Synchronization Issues

Synchronization issues occur when CPU operations and VDC rendering are not properly coordinated, leading to visual artifacts or incorrect behavior.

**Example 1: Late VBLANK Update**

**Symptom:** Sprite position update sometimes doesn't take effect until next frame

**Trace Evidence:**
```
Frame:0001 Scanline:260 Cycle:00200 PC:0x0700 | MOV A, #0x50      ; Load new position
Frame:0001 Scanline:261 Cycle:00100 PC:0x0702 | OUTL P1, A        ; Write position
Frame:0002 Scanline:000 Cycle:00000 PC:0x0704 | JMP MainLoop      ; Frame transition
```

**Analysis:**
- Position write occurs at scanline 261 (last VBLANK scanline)
- Very late in VBLANK period
- Frame transition occurs shortly after (scanline 0)
- **Synchronization Issue:** Update arrives too late, may miss frame deadline

**Identification Markers:**
- VDC writes very late in VBLANK (scanlines 260-261 NTSC)
- Frame transition occurs shortly after write
- Intermittent "skipped frame" behavior
- Update takes effect one frame later than expected

**Example 2: Missed VBLANK Window**

**Symptom:** Screen tearing appears intermittently

**Trace Evidence:**
```
Frame:0001 Scanline:192 Cycle:00000 | VBLANK START
Frame:0001 Scanline:195 Cycle:00100 PC:0x0800 | CALL UpdateSprites
Frame:0001 Scanline:260 Cycle:00200 PC:0x0850 | RET               ; Return from UpdateSprites
Frame:0002 Scanline:005 Cycle:00100 PC:0x0802 | OUTL P1, A        ; Write sprite color (too late!)
```

**Analysis:**
- UpdateSprites called during VBLANK (scanline 195)
- Subroutine takes 65 scanlines to complete
- Returns at scanline 260 (still in VBLANK)
- But next instruction (VDC write) executes at scanline 5 (next frame, active display!)
- **Synchronization Issue:** Subroutine takes too long, misses VBLANK window

**Identification Markers:**
- Long-running subroutine called during VBLANK
- Subroutine execution spans VBLANK boundary
- VDC writes occur after frame transition
- Tearing or flickering artifacts

**Example 3: Polling Timeout**

**Symptom:** Game occasionally freezes or becomes unresponsive

**Trace Evidence:**
```
; Wait for VBLANK
Frame:0001 Scanline:191 Cycle:00200 PC:0x0900 | IN A, P2          ; Read VDC status
Frame:0001 Scanline:191 Cycle:00220 PC:0x0902 | JB7 VBLANKReady   ; Check VBLANK bit
Frame:0001 Scanline:191 Cycle:00240 PC:0x0904 | JMP WaitVBLANK    ; Loop
Frame:0001 Scanline:192 Cycle:00000 | VBLANK START
Frame:0001 Scanline:192 Cycle:00010 PC:0x0900 | IN A, P2          ; Read VDC status
Frame:0001 Scanline:192 Cycle:00030 PC:0x0902 | JB7 VBLANKReady   ; VBLANK bit set
Frame:0001 Scanline:192 Cycle:00040 PC:0x0906 | CALL UpdateVDC    ; Proceed
```

**Normal Case:** VBLANK detected, code proceeds

**Problem Case:**
```
; Wait for VBLANK (but VBLANK bit already set from previous frame)
Frame:0001 Scanline:010 Cycle:00100 PC:0x0900 | IN A, P2          ; Read VDC status
Frame:0001 Scanline:010 Cycle:00120 PC:0x0902 | JB7 VBLANKReady   ; VBLANK bit still set!
Frame:0001 Scanline:010 Cycle:00130 PC:0x0906 | CALL UpdateVDC    ; Update mid-frame (wrong!)
```

**Analysis:**
- Code polls for VBLANK bit
- But VBLANK bit may persist from previous frame
- Code doesn't wait for bit to clear first
- **Synchronization Issue:** Polling logic doesn't account for bit persistence

**Identification Markers:**
- Polling loop without timeout
- Condition check without state transition detection
- Intermittent early execution
- Mid-frame updates when VBLANK expected

#### Identifying Frame Budget Overruns

Frame budget overruns occur when game logic takes longer than one frame period to execute, causing frame rate drops or timing issues.

**Example 1: Consistent Slowdown**

**Symptom:** Game runs at 50 Hz instead of 60 Hz (NTSC)

**Trace Evidence:**
```
Frame:0001 Scanline:000 Cycle:00000 | Frame Start
Frame:0001 Scanline:261 Cycle:00226 | Frame End
Frame:0002 Scanline:000 Cycle:00000 | Frame Start (should be here)
```

**Timing Calculation:**
- Expected frame duration: 16.79 ms (NTSC 60 Hz)
- Count instructions in trace: 7,200 instructions
- Estimate cycles: ~8,400 instruction cycles (accounting for dual-cycle instructions)
- Actual frame duration: 8,400 ÷ 358,000 Hz = 23.46 ms
- Actual frame rate: 1,000 ÷ 23.46 = 42.6 Hz
- **Frame Budget Overrun:** Game exceeds available CPU cycles by 40%

**Identification Markers:**
- Frame duration exceeds 16.79 ms (NTSC) or 20.03 ms (PAL)
- Instruction count exceeds ~5,967 cycles (NTSC) or ~7,880 cycles (PAL)
- Consistent slowdown across all frames
- No visual artifacts, just slower gameplay

**Example 2: Intermittent Frame Drops**

**Symptom:** Game occasionally stutters or drops frames

**Trace Evidence:**
```
Frame:0001 Duration: 16.5 ms (normal)
Frame:0002 Duration: 16.8 ms (normal)
Frame:0003 Duration: 25.2 ms (overrun!)
Frame:0004 Duration: 16.6 ms (normal)
Frame:0005 Duration: 16.7 ms (normal)
Frame:0006 Duration: 24.8 ms (overrun!)
```

**Analysis:**
- Most frames complete within budget
- Every 3rd frame takes ~50% longer
- Pattern suggests periodic heavy computation
- **Frame Budget Overrun:** Intermittent, triggered by specific game state

**Identification Markers:**
- Periodic frame duration spikes
- Pattern correlates with game events (enemy spawns, explosions, etc.)
- Stuttering or hitching during gameplay
- Some frames within budget, others exceed it

**Example 3: Cumulative Drift**

**Symptom:** Game timing drifts over time, becomes progressively slower

**Trace Evidence:**
```
Frame:0001 Duration: 16.9 ms (slight overrun)
Frame:0002 Duration: 17.1 ms (slight overrun)
Frame:0003 Duration: 17.3 ms (slight overrun)
...
Frame:0100 Duration: 22.5 ms (significant overrun)
```

**Analysis:**
- Each frame slightly exceeds budget
- Overrun accumulates over time
- No single bottleneck, but cumulative effect
- **Frame Budget Overrun:** Gradual accumulation of small overruns

**Identification Markers:**
- Frame duration increases gradually over time
- No sudden spikes, just steady increase
- Game starts at correct speed, slows down over time
- May be caused by memory leaks, growing data structures, or inefficient algorithms

#### Identifying Delayed Updates

Delayed updates occur when register writes or state changes arrive too late to take effect in the current frame, causing visual glitches or incorrect behavior.

**Example 1: Sprite Position Lag**

**Symptom:** Sprite appears one frame behind player input

**Trace Evidence:**
```
Frame:0001 Scanline:010 Cycle:00100 | IN A, P1          ; Read joystick (right pressed)
Frame:0001 Scanline:015 Cycle:00200 | CALL MovePlayer   ; Process input
Frame:0001 Scanline:020 Cycle:00100 | MOV sprite_x, A   ; Update sprite X in memory
Frame:0002 Scanline:195 Cycle:00050 | MOV A, sprite_x   ; Load sprite X (next frame!)
Frame:0002 Scanline:195 Cycle:00070 | OUTL P1, A        ; Write to VDC
```

**Analysis:**
- Input read and processed in frame 1
- Sprite position updated in memory in frame 1
- But VDC register not written until frame 2
- **Delayed Update:** One-frame lag between input and visual update

**Identification Markers:**
- Input processing and VDC update in different frames
- Memory updated in one frame, VDC updated in next
- Visible lag between input and response
- Sprite position always one frame behind

**Example 2: Color Change Delay**

**Symptom:** Background color changes one scanline late

**Trace Evidence:**
```
; Intended: Change color at scanline 100
Frame:0001 Scanline:099 Cycle:00200 PC:0x0A00 | MOV A, #0x05      ; Load new color
Frame:0001 Scanline:100 Cycle:00010 PC:0x0A02 | OUTL P1, A        ; Write color
```

**Analysis:**
- Color loaded at scanline 99, cycle 200
- Color written at scanline 100, cycle 10
- Scanline 100 already started rendering (cycle 10 of 227)
- **Delayed Update:** Color change takes effect partway through scanline

**Identification Markers:**
- VDC write occurs after target scanline starts
- Cycle number > 0 when write occurs
- Color bleeding or partial scanline artifacts
- Update intended for scanline boundary but arrives late

#### Identifying Timing Assumptions

Timing assumption bugs occur when code assumes specific execution speeds or timing relationships that don't hold in all cases.

**Example 1: Fixed Delay Loop**

**Symptom:** Animation speed differs between NTSC and PAL

**Code:**
```assembly
; Delay loop (assumes NTSC timing)
DELAY:
    MOV R0, #100        ; Loop 100 times
DELAY_LOOP:
    DJNZ R0, DELAY_LOOP ; 2 cycles per iteration
    RET
```

**Analysis:**
- Delay: 100 × 2 = 200 instruction cycles
- NTSC: 200 × 2.79 µs = 558 µs
- PAL: 200 × 2.54 µs = 508 µs
- **Timing Assumption:** Fixed loop count assumes NTSC timing
- Result: Animation 10% faster on PAL

**Identification Markers:**
- Fixed delay loops (constant iteration counts)
- Animation or timing code without system detection
- Different behavior on NTSC vs PAL
- Code assumes specific CPU clock frequency

**Example 2: Scanline Counting**

**Symptom:** Mid-frame effect appears at wrong position on PAL

**Code:**
```assembly
; Wait for scanline 100 (assumes NTSC)
WAIT_SCANLINE:
    IN A, P2            ; Read VDC status
    ; ... check scanline number ...
    JNZ WAIT_SCANLINE
    
; Change color at scanline 100
    MOV A, #0x05
    OUTL P1, A
```

**Analysis:**
- Code waits for scanline 100
- NTSC: Scanline 100 is at 52% of active display (100 ÷ 192)
- PAL: Scanline 100 is at 41% of active display (100 ÷ 242)
- **Timing Assumption:** Scanline 100 represents same screen position on both systems
- Result: Effect appears at different vertical position on PAL

**Identification Markers:**
- Hard-coded scanline numbers
- Mid-frame effects without system detection
- Different visual appearance on NTSC vs PAL
- Code assumes specific scanline count

#### Reproducing Timing-Dependent Bugs

Timing-dependent bugs are often difficult to reproduce consistently. Use these techniques to increase reproducibility:

**Technique 1: Frame-Specific Capture**

Capture traces at specific frames where bug is known to occur:

```bash
# Capture frame 100 (where bug appears)
videopac --trace --trace-start-frame 100 --trace-frames 1 --trace-output bug_frame.txt bios.bin game.bin
```

**Technique 2: Event-Triggered Capture**

Capture traces when specific events occur (key press, collision, etc.):

```bash
# Capture first frame after pressing '1' key
videopac --trace --trace-start-key "1" --trace-frames 1 --trace-output event_trace.txt bios.bin game.bin
```

**Technique 3: Multi-Frame Comparison**

Capture multiple frames to identify patterns:

```bash
# Capture 10 frames starting at frame 100
videopac --trace --trace-start-frame 100 --trace-frames 10 --trace-output multi_frame.txt bios.bin game.bin
```

Compare frames to find:
- Frames where bug occurs vs doesn't occur
- Timing differences between frames
- Patterns in register access or execution flow

**Technique 4: Stress Testing**

Create conditions that increase likelihood of bug:

- Spawn maximum enemies (stress CPU)
- Trigger multiple events simultaneously (stress synchronization)
- Play for extended duration (expose cumulative issues)
- Rapidly press inputs (expose race conditions)

**Technique 5: System Variation**

Test on different system configurations:

- NTSC vs PAL mode
- Different emulator versions
- Real hardware (if available)
- Different CPU speeds (if emulator supports)

#### Isolating Timing-Dependent Bugs

Once reproduced, isolate the bug to specific code sections:

**Step 1: Identify Suspicious Code Regions**

Look for:
- Polling loops without timeouts
- Read-modify-write sequences
- VDC register access patterns
- Delay loops or timing-sensitive code
- Interrupt handlers or asynchronous operations

**Step 2: Narrow Down Timing Window**

Use binary search approach:

1. Capture trace of full frame
2. Identify midpoint of frame
3. Determine if bug occurs in first or second half
4. Repeat with smaller windows until isolated

**Step 3: Analyze Instruction Sequence**

Once isolated to small window:

1. Examine instruction sequence in trace
2. Check register values and timing
3. Look for race conditions or synchronization issues
4. Verify against disassembly

**Step 4: Verify Root Cause**

Test hypothesis:

1. Identify suspected instruction or timing issue
2. Predict what would happen if timing changed
3. Capture trace with different timing (different frame, system, etc.)
4. Verify prediction matches actual behavior

#### Debugging Strategies for Timing Bugs

**Strategy 1: Add Timing Markers**

Insert distinctive instructions to mark timing points:

```assembly
; Before critical section
MOV A, #0xFF        ; Marker: Start
NOP

; Critical section
; ...

; After critical section
MOV A, #0x00        ; Marker: End
NOP
```

Search trace for markers to measure timing:

```bash
grep "MOV A, #0xFF" trace.txt  # Find start
grep "MOV A, #0x00" trace.txt  # Find end
```

**Strategy 2: Breakpoint Analysis**

Use debugger breakpoints to pause at critical points:

1. Set breakpoint before suspected race condition
2. Examine register and memory state
3. Single-step through critical section
4. Verify timing and synchronization

**Strategy 3: Comparative Tracing**

Compare traces of working vs broken cases:

```bash
# Capture working case
videopac --trace --trace-start-frame 50 --trace-frames 1 --trace-output working.txt bios.bin game.bin

# Capture broken case
videopac --trace --trace-start-frame 100 --trace-frames 1 --trace-output broken.txt bios.bin game.bin

# Compare
diff working.txt broken.txt
```

Look for:
- Different instruction sequences
- Different timing (scanline/cycle positions)
- Different register values
- Different VDC access patterns

**Strategy 4: Timing Instrumentation**

Measure execution time of suspected code sections:

1. Note start scanline/cycle from trace
2. Note end scanline/cycle from trace
3. Calculate elapsed time
4. Compare with expected or budgeted time
5. Identify if timing exceeds expectations

**Strategy 5: Simplification**

Reduce complexity to isolate bug:

1. Disable non-essential features
2. Reduce number of sprites/enemies
3. Simplify game logic
4. Remove animations or effects

If bug disappears, re-enable features one at a time to identify trigger.

#### Common Timing Bug Patterns

**Pattern 1: Check-Then-Act Race**

```assembly
; Check condition
IN A, P2            ; Read status
JB7 Ready           ; Check if ready

; Act on condition (but condition may have changed!)
MOV A, #0x42
OUTL P1, A
```

**Fix:** Use atomic operations or disable interrupts during critical section

**Pattern 2: Busy-Wait Without Timeout**

```assembly
; Wait forever for condition
WAIT:
    IN A, P2
    JB7 WAIT        ; Loop until bit 7 set
```

**Fix:** Add timeout counter or alternative exit condition

**Pattern 3: Delayed State Update**

```assembly
; Update state in memory
MOV sprite_x, A

; ... many instructions later ...

; Write to VDC (may be next frame!)
MOV A, sprite_x
OUTL P1, A
```

**Fix:** Write to VDC immediately after state update

**Pattern 4: Assumption of Execution Order**

```assembly
; Assume this executes before VDC reads register
OUTL P1, A          ; Write sprite position

; But VDC may have already read old value!
```

**Fix:** Ensure writes occur during VBLANK or before VDC needs value

#### Timing Bug Checklist

When investigating timing-dependent bugs:

- [ ] Reproduce bug consistently (specific frame, event, or condition)
- [ ] Capture execution trace of bug occurrence
- [ ] Identify timing-sensitive code regions (polling, VDC access, delays)
- [ ] Check for race conditions (read-modify-write, concurrent access)
- [ ] Verify synchronization (VBLANK timing, frame boundaries)
- [ ] Measure frame budget (total CPU cycles per frame)
- [ ] Check for delayed updates (memory vs VDC register timing)
- [ ] Identify timing assumptions (fixed delays, scanline numbers)
- [ ] Compare NTSC vs PAL behavior
- [ ] Test with different timing conditions (frames, events, stress)
- [ ] Isolate to specific instruction sequence
- [ ] Verify root cause hypothesis
- [ ] Implement and test fix
- [ ] Verify fix doesn't introduce new timing issues

#### Practical Example: Debugging a Race Condition

**Problem:** Collision detection occasionally fails to detect valid collisions between player and enemy sprites.

**Step 1: Reproduce**

Play game until collision fails to detect. Note frame number from FPS display.

**Step 2: Capture Trace**

```bash
videopac --trace --trace-start-frame 1250 --trace-frames 1 --trace-output collision_bug.txt bios.bin game.bin
```

**Step 3: Find Collision Detection Code**

Search disassembly for collision register reads:

```bash
grep "IN A, P2" game_disasm.txt | grep "0xA2"  # Collision register
```

**Step 4: Analyze Trace**

```
Frame:1250 Scanline:100 Cycle:00100 PC:0x0800 | IN A, P2          ; Read collision (0x00)
Frame:1250 Scanline:100 Cycle:00120 PC:0x0802 | MOV R0, A         ; Store collision
Frame:1250 Scanline:100 Cycle:00130 PC:0x0804 | MOV A, #0x00      ; Clear collision
Frame:1250 Scanline:100 Cycle:00140 PC:0x0806 | OUTL P2, A        ; Write clear
Frame:1250 Scanline:100 Cycle:00160 PC:0x0808 | MOV A, R0         ; Load collision
Frame:1250 Scanline:100 Cycle:00170 PC:0x080A | JZ NoCollision    ; Jump if zero
```

**Step 5: Identify Race Condition**

- Collision register read at cycle 100 (value = 0x00, no collision)
- Collision register cleared at cycle 140
- Gap of 40 VDC cycles between read and clear
- **Hypothesis:** Collision occurs between cycles 100-140, but is immediately cleared

**Step 6: Verify Hypothesis**

Check sprite positions at scanline 100:

```bash
grep "Scanline:100" collision_bug.txt | grep "VDC_WRITE\[0x1[0-3]\]"  # Sprite positions
```

```
Frame:1250 Scanline:095 Cycle:00200 | VDC_WRITE[0x10] = 0x64    ; Player Y = 100
Frame:1250 Scanline:095 Cycle:00220 | VDC_WRITE[0x11] = 0x50    ; Player X = 80
Frame:1250 Scanline:096 Cycle:00100 | VDC_WRITE[0x12] = 0x64    ; Enemy Y = 100
Frame:1250 Scanline:096 Cycle:00120 | VDC_WRITE[0x13] = 0x50    ; Enemy X = 80
```

**Analysis:**
- Player and enemy at same position (X=80, Y=100)
- Collision should be detected
- But collision register read before VDC detects collision
- **Root Cause:** Collision detection reads register too early, before VDC updates it

**Step 7: Fix**

Move collision detection to later in frame, after VDC has time to detect collisions:

```assembly
; Original (too early)
0x0800: IN A, P2            ; Read collision at scanline 100

; Fixed (wait for VDC)
0x0800: CALL WaitScanline120  ; Wait until scanline 120
0x0802: IN A, P2            ; Read collision (VDC has detected it)
```

**Step 8: Verify Fix**

Capture new trace and verify collision is detected:

```
Frame:1250 Scanline:120 Cycle:00100 PC:0x0802 | IN A, P2          ; Read collision (0x01)
Frame:1250 Scanline:120 Cycle:00120 PC:0x0804 | MOV R0, A         ; Store collision
Frame:1250 Scanline:120 Cycle:00130 PC:0x0806 | MOV A, #0x00      ; Clear collision
Frame:1250 Scanline:120 Cycle:00140 PC:0x0808 | OUTL P2, A        ; Write clear
Frame:1250 Scanline:120 Cycle:00160 PC:0x080A | MOV A, R0         ; Load collision
Frame:1250 Scanline:120 Cycle:00170 PC:0x080C | JNZ Collision     ; Jump (collision detected!)
```

**Success:** Collision now detected correctly.

#### Summary

Timing-dependent bugs are challenging but can be systematically debugged:

1. **Understand** the types: race conditions, synchronization issues, frame overruns, delayed updates, timing assumptions
2. **Identify** markers in traces: interleaved operations, late updates, budget overruns, timing gaps
3. **Reproduce** consistently using frame-specific captures, event triggers, or stress testing
4. **Isolate** to specific code regions using binary search and timing analysis
5. **Verify** root cause through comparative tracing and hypothesis testing
6. **Fix** by adjusting timing, adding synchronization, or removing timing assumptions
7. **Test** thoroughly across different frames, systems (NTSC/PAL), and conditions

Understanding timing-dependent bugs is essential for accurate emulation and debugging complex synchronization issues in Videopac games.

#### References

For related information, see:
- [Videopac Timing Model](#71-videopac-timing-model) - System timing fundamentals
- [Instruction Timing Analysis](#72-instruction-timing-analysis) - Measuring execution time
- [VBLANK Identification](#73-vblank-identification) - Synchronization windows
- [Mid-Frame Update Analysis](#74-mid-frame-update-analysis) - VDC register timing
- [Common Timing Issues](#77-common-timing-issues) - Typical timing problems

### 7.6 Using FPS Display and Metrics

The emulator provides real-time performance metrics through an FPS (Frames Per Second) display and debugger statistics. These metrics help identify performance issues, timing problems, and emulation accuracy concerns.

#### Enabling the FPS Display

**In SDL Mode:**

The FPS display is enabled by default and shows in the bottom-right corner of the screen.

**Toggle FPS Display:**
- Press **F3** to toggle the FPS display on/off
- The display shows: `FPS: 60.0` (or current frame rate)
- An on-screen notification confirms the state change

**Configuration:**
The FPS display state persists across sessions through the configuration file.

**Display Position:**
The FPS display appears in a unified status bar at the bottom of the screen, along with other indicators:
- `FPS: 60.0` - Current frame rate
- `MUTE` - Audio is muted (if applicable)
- `TURBO` - Turbo mode is active (if applicable)
- `PAUSED` - Emulation is paused (if applicable)

#### Understanding FPS Metrics

**Target Frame Rates:**

The Videopac emulator targets different frame rates based on video standard:
- **NTSC**: 60 FPS (60Hz refresh rate)
- **PAL**: 50 FPS (50Hz refresh rate)

**FPS Calculation:**

The FPS counter updates once per second and displays the average frame rate over that period:

```cpp
// FPS is calculated every 1000ms
if (current_time - last_fps_time >= 1000) {
    current_fps = fps_counter * 1000.0 / (current_time - last_fps_time);
    fps_counter = 0;
    last_fps_time = current_time;
}
```

**Interpreting FPS Values:**

| FPS Range | Interpretation | Likely Cause |
|-----------|----------------|--------------|
| 59-61 (NTSC) | Perfect | Emulation running at correct speed |
| 49-51 (PAL) | Perfect | Emulation running at correct speed |
| < 50 (PAL) or < 55 (NTSC) | Slow | CPU bottleneck, heavy debugging, or system load |
| > 65 (NTSC) or > 55 (PAL) | Fast | Turbo mode active or timing issue |
| Fluctuating | Unstable | Inconsistent frame timing or system interruptions |

#### Debugger Performance Statistics

When the debugger is enabled, additional performance metrics are available through the `FrameStats` structure:

**Available Metrics:**

1. **Total Cycles** - Cumulative VDC cycles executed since emulator start
2. **Frame Count** - Total number of frames rendered
3. **FPS** - Current frames per second (updated by frontend)
4. **Average Cycles Per Frame** - Mean VDC cycles per frame

**Accessing Statistics:**

**In Code:**
```cpp
FrameStats stats = debugger_->get_frame_stats();
std::cout << "Total Cycles: " << stats.total_cycles << std::endl;
std::cout << "Frame Count: " << stats.frame_count << std::endl;
std::cout << "FPS: " << stats.fps << std::endl;
std::cout << "Avg Cycles/Frame: " << stats.average_cycles_per_frame << std::endl;
```

**In ImGui Debugger:**
The Performance panel displays real-time statistics:
- Total VDC cycles executed
- Current frame count
- Cycles per frame average

**Resetting Statistics:**
```cpp
debugger_->reset_frame_stats();
```

#### Using Metrics for Performance Debugging

**Scenario 1: Identifying Slow Emulation**

**Symptoms:**
- FPS consistently below target (< 55 for NTSC, < 45 for PAL)
- Game appears to run in slow motion
- Audio may stutter or lag

**Debugging Steps:**

1. **Check FPS Display:**
   - Press F3 to enable FPS display if not visible
   - Observe the FPS value over several seconds
   - Note if FPS is consistently low or fluctuating

2. **Identify Bottleneck:**
   - **Trace Logging**: Disable trace logging if enabled (massive performance impact)
   - **Debugger Overhead**: Check if debugger is paused or stepping
   - **System Load**: Close other applications consuming CPU
   - **VSync**: Try disabling VSync if enabled

3. **Measure Baseline:**
   - Run emulator without debugger: `videopac bios.bin game.bin`
   - Compare FPS with debugger enabled
   - Typical overhead: Debugger adds 5-10% CPU usage

**Scenario 2: Detecting Frame Timing Issues**

**Symptoms:**
- FPS fluctuates wildly (e.g., 45-75 FPS)
- Game speed varies noticeably
- Audio pitch changes

**Debugging Steps:**

1. **Monitor FPS Stability:**
   - Watch FPS display for 10-20 seconds
   - Note minimum, maximum, and average values
   - Check if fluctuations correlate with game events

2. **Check Frame Budget:**
   - Access debugger frame statistics
   - Compare `average_cycles_per_frame` with expected values:
     - **NTSC**: ~59,659 cycles per frame (227 cycles/scanline × 262 scanlines)
     - **PAL**: ~70,681 cycles per frame (227 cycles/scanline × 312 scanlines)

3. **Identify Timing Anomalies:**
   - Frames significantly over budget indicate performance issues
   - Frames significantly under budget may indicate timing bugs
   - Consistent frame times suggest stable emulation

**Example Analysis:**
```
Frame 1: 59,650 cycles (normal)
Frame 2: 59,655 cycles (normal)
Frame 3: 75,000 cycles (ANOMALY - 25% over budget)
Frame 4: 59,660 cycles (normal)
```

The anomaly in Frame 3 suggests:
- Possible infinite loop or hang
- Excessive instruction execution
- Timing synchronization issue

**Scenario 3: Verifying Emulation Accuracy**

**Symptoms:**
- Game runs too fast or too slow compared to real hardware
- Audio pitch is incorrect
- Timing-sensitive gameplay is off

**Debugging Steps:**

1. **Verify Target FPS:**
   - Check video standard setting (NTSC vs PAL)
   - Confirm FPS matches target (60 for NTSC, 50 for PAL)
   - Compare with real hardware recordings if available

2. **Check Cycle Accuracy:**
   - Monitor `average_cycles_per_frame` over 100+ frames
   - Compare with theoretical values:
     - **NTSC**: 59,659 cycles/frame
     - **PAL**: 70,681 cycles/frame
   - Deviation > 5% indicates timing inaccuracy

3. **Test with Known Games:**
   - Run games with precise timing requirements
   - Compare behavior with real hardware
   - Use audio pitch as timing reference

#### Performance Profiling Workflow

**Step 1: Establish Baseline**
```bash
# Run without debugger
videopac bios.bin game.bin

# Observe FPS for 30 seconds
# Record: Min FPS, Max FPS, Average FPS
```

**Step 2: Enable Debugger Metrics**
```bash
# Run with debugger enabled
videopac --debugger bios.bin game.bin

# Press F12 to open debugger
# Navigate to Performance panel
# Record: Total cycles, Frame count, Avg cycles/frame
```

**Step 3: Identify Performance Hotspots**
- Enable minimal trace logging: `--trace-level minimal`
- Run for 10 frames
- Analyze trace for repeated patterns or long sequences
- Identify CPU-intensive code sections

**Step 4: Optimize and Verify**
- Make performance improvements
- Re-run baseline test
- Compare FPS before and after
- Verify no regressions in emulation accuracy

#### Common Performance Issues

**Issue 1: Trace Logging Overhead**

**Symptoms:**
- FPS drops to 5-15 when trace is enabled
- Massive trace.log files (100+ MB)

**Solution:**
- Use `--trace-level minimal` instead of `full`
- Limit trace duration: `--trace-frames 1`
- Filter trace output: `--trace-filter vdc`
- Disable trace when not needed

**Issue 2: Debugger Stepping Overhead**

**Symptoms:**
- FPS shows 0.0 or very low value
- Game appears frozen

**Cause:**
- Debugger is paused or stepping through code
- This is expected behavior

**Solution:**
- Press F5 to continue execution
- Use breakpoints instead of continuous stepping

**Issue 3: VSync Limiting**

**Symptoms:**
- FPS locked at monitor refresh rate (e.g., 60 FPS on 60Hz monitor)
- Cannot exceed monitor refresh rate even in turbo mode

**Cause:**
- VSync synchronizes rendering with monitor refresh

**Solution:**
- Disable VSync in configuration if higher FPS needed
- Note: VSync provides smooth rendering but limits maximum FPS

#### Best Practices

**For General Debugging:**
1. Keep FPS display enabled to monitor performance impact
2. Disable trace logging when not actively analyzing
3. Use minimal trace level for routine debugging
4. Reset frame statistics when starting new test runs

**For Performance Analysis:**
1. Establish baseline FPS without debugger
2. Measure debugger overhead separately
3. Profile with minimal trace first, then full trace if needed
4. Compare cycle counts with theoretical values

**For Timing Verification:**
1. Monitor FPS over extended periods (1+ minutes)
2. Check for frame rate stability
3. Verify average cycles per frame matches video standard
4. Test with timing-sensitive games

#### Limitations

**FPS Display Limitations:**
- Updates only once per second (1Hz refresh)
- Shows average over 1-second window, not instantaneous FPS
- May not reflect brief frame drops or spikes

**Debugger Statistics Limitations:**
- Cycle counts are cumulative, not per-frame snapshots
- FPS value in FrameStats is set by frontend, not calculated by debugger
- Statistics reset on emulator restart

**Measurement Accuracy:**
- System load affects FPS measurements
- Background processes can cause fluctuations
- VSync can mask timing issues by forcing frame rate

#### References

For related information, see:
- [Videopac Timing Model](#71-videopac-timing-model) - Understanding frame timing
- [Instruction Timing Analysis](#72-instruction-timing-analysis) - Cycle-accurate timing
- [VBLANK Identification](#73-vblank-identification) - Frame synchronization
- [Timing-Dependent Bug Identification](#75-timing-dependent-bug-identification) - Timing issues
- [Common Timing Issues](#77-common-timing-issues) - Typical problems

### 7.7 Common Timing Issues

This section documents common timing issues encountered in Videopac emulation, providing quick reference for identifying, diagnosing, and resolving timing-related problems. Each issue includes symptoms, root causes, diagnostic techniques, and solutions based on real debugging scenarios.

#### Issue 1: Frame Rate Below Target (Slowdown)

**Symptoms:**
- Game runs slower than expected (< 55 FPS on NTSC, < 45 FPS on PAL)
- Gameplay appears in slow motion
- Audio pitch is lower than normal
- Consistent slowdown across all frames

**Root Causes:**

1. **CPU Cycle Budget Exceeded**
   - Game logic takes more than available CPU cycles per frame
   - NTSC: > 5,967 instruction cycles per frame
   - PAL: > 7,880 instruction cycles per frame

2. **Inefficient Game Code**
   - Nested loops with high iteration counts
   - Redundant calculations
   - Excessive VDC register writes

3. **Emulator Performance Issues**
   - Trace logging enabled (massive overhead)
   - Debugger overhead
   - System resource contention

**Diagnostic Techniques:**

**Check FPS Display:**
```bash
# Run with FPS display enabled
videopac bios.bin game.bin
# Press F3 to toggle FPS display
# Observe FPS value over 10-20 seconds
```

**Capture Frame Trace:**
```bash
# Capture one complete frame
videopac --trace --trace-start-frame 100 --trace-frames 1 --trace-output frame.txt bios.bin game.bin
```

**Count Instructions:**
```bash
# Count total instructions in frame
wc -l frame.txt

# Estimate dual-cycle instructions (CALL, JMP, OUTL, etc.)
grep -E "CALL|JMP|OUTL|MOVX|IN " frame.txt | wc -l
```

**Calculate Frame Budget:**
```
Total Instructions: 7,200
Dual-Cycle Instructions: 1,200
Single-Cycle Instructions: 6,000
Total Cycles: 6,000 + (1,200 × 2) = 8,400 cycles

NTSC Budget: 5,967 cycles
Deficit: 8,400 - 5,967 = 2,433 cycles (41% overrun)
Expected FPS: 5,967 ÷ 8,400 × 60 = 42.6 FPS
```

**Identify Bottlenecks:**
```bash
# Find long-running subroutines
grep "CALL" frame.txt > calls.txt
grep "RET" frame.txt > returns.txt
# Match CALL/RET pairs and calculate duration
```

**Solutions:**

**For Game Code Issues:**
1. **Optimize Loops** - Reduce iteration counts or optimize loop bodies
2. **Cache Calculations** - Store repeated calculations in variables
3. **Batch VDC Updates** - Group register writes together
4. **Simplify Logic** - Remove unnecessary computations

**For Emulator Issues:**
1. **Disable Trace Logging** - Remove `--trace` flag
2. **Close Debugger** - Run without debugger for baseline performance
3. **Reduce System Load** - Close other applications
4. **Optimize Emulator** - Profile and optimize hot paths

**Example Fix:**

**Before (Slow):**
```assembly
; Update 4 sprites individually
UpdateSprites:
    CALL UpdateSprite0    ; 500 cycles
    CALL UpdateSprite1    ; 500 cycles
    CALL UpdateSprite2    ; 500 cycles
    CALL UpdateSprite3    ; 500 cycles
    RET                   ; Total: 2,000 cycles
```

**After (Fast):**
```assembly
; Update sprites in tight loop
UpdateSprites:
    MOV R0, #4            ; Sprite count
    MOV R1, #sprite_data  ; Data pointer
UpdateLoop:
    MOV A, @R1            ; Load X position (1 cycle)
    OUTL P1, A            ; Write to VDC (2 cycles)
    INC R1                ; Next byte (1 cycle)
    MOV A, @R1            ; Load Y position (1 cycle)
    OUTL P1, A            ; Write to VDC (2 cycles)
    INC R1                ; Next byte (1 cycle)
    DJNZ R0, UpdateLoop   ; Loop (2 cycles)
    RET                   ; Total: 4 × 10 = 40 cycles (50× faster!)
```

#### Issue 2: Screen Tearing or Flickering

**Symptoms:**
- Horizontal line appears across screen where image splits
- Sprites or graphics flicker on/off
- Visual artifacts during gameplay
- Inconsistent appearance frame-to-frame

**Root Causes:**

1. **Mid-Frame VDC Updates**
   - VDC registers written during active display (scanlines 0-191 NTSC, 0-241 PAL)
   - Updates occur while screen is being drawn
   - Causes visible split between old and new values

2. **Late VBLANK Updates**
   - Updates start during VBLANK but extend into active display
   - Subroutine takes longer than VBLANK duration
   - Some registers updated after frame transition

3. **Missing VBLANK Synchronization**
   - Code doesn't wait for VBLANK before updating
   - Updates occur at arbitrary times
   - No synchronization with video timing

**Diagnostic Techniques:**

**Filter VDC Writes by Scanline:**
```bash
# Find VDC writes during active display (NTSC)
grep "VDC_WRITE" trace.txt | grep -E "Scanline:0[0-9]{2}|Scanline:1[0-8][0-9]|Scanline:19[0-1]"

# Find VDC writes during VBLANK (NTSC)
grep "VDC_WRITE" trace.txt | grep -E "Scanline:19[2-9]|Scanline:2[0-5][0-9]|Scanline:26[0-1]"
```

**Identify Problematic Writes:**
```
Frame:0001 Scanline:100 Cycle:00150 | VDC_WRITE[0xAB] = 0x02    ← Mid-frame (tearing!)
Frame:0001 Scanline:195 Cycle:00100 | VDC_WRITE[0xAC] = 0x05    ← VBLANK (safe)
Frame:0002 Scanline:005 Cycle:00050 | VDC_WRITE[0xAD] = 0x03    ← After VBLANK (tearing!)
```

**Check VBLANK Timing:**
```bash
# Find VBLANK start
grep "Scanline:192 Cycle:00000" trace.txt

# Find frame transition (VBLANK end)
grep "Scanline:000 Cycle:00000" trace.txt

# Check if VDC updates occur between these points
```

**Solutions:**

**1. Move Updates to VBLANK:**

**Before (Tearing):**
```assembly
; Update sprite color mid-frame
MainLoop:
    CALL GameLogic        ; Scanline 50
    MOV A, sprite_color   ; Scanline 100
    OUTL P1, A            ; Write mid-frame (tearing!)
    JMP MainLoop
```

**After (No Tearing):**
```assembly
; Wait for VBLANK before updating
MainLoop:
    CALL GameLogic        ; Scanline 50
    CALL WaitVBLANK       ; Wait until scanline 192
    MOV A, sprite_color   ; Scanline 195
    OUTL P1, A            ; Write during VBLANK (safe)
    JMP MainLoop

WaitVBLANK:
    IN A, P2              ; Read VDC status
    JB7 WaitVBLANK        ; Loop until VBLANK bit set
    RET
```

**2. Batch Updates:**

**Before (Multiple Mid-Frame Writes):**
```assembly
; Scattered updates throughout frame
Scanline:050: OUTL P1, A    ; Update 1 (mid-frame)
Scanline:100: OUTL P1, A    ; Update 2 (mid-frame)
Scanline:150: OUTL P1, A    ; Update 3 (mid-frame)
```

**After (Batched VBLANK Updates):**
```assembly
; Store values in memory during frame
Scanline:050: MOV sprite0_color, A    ; Store in memory
Scanline:100: MOV sprite1_color, A    ; Store in memory
Scanline:150: MOV sprite2_color, A    ; Store in memory

; Write all values during VBLANK
Scanline:195: MOV A, sprite0_color    ; Load from memory
Scanline:195: OUTL P1, A              ; Write to VDC
Scanline:196: MOV A, sprite1_color    ; Load from memory
Scanline:196: OUTL P1, A              ; Write to VDC
Scanline:197: MOV A, sprite2_color    ; Load from memory
Scanline:197: OUTL P1, A              ; Write to VDC
```

**3. Optimize Long Subroutines:**

If update routine exceeds VBLANK duration:

```assembly
; Original: Takes 2,000 cycles (exceeds VBLANK budget of 1,609)
UpdateAll:
    CALL UpdateSprites    ; 800 cycles
    CALL UpdateGrid       ; 600 cycles
    CALL UpdateColors     ; 600 cycles
    RET                   ; Total: 2,000 cycles

; Optimized: Split across frames
Frame1:
    CALL WaitVBLANK
    CALL UpdateSprites    ; 800 cycles (within budget)
    
Frame2:
    CALL WaitVBLANK
    CALL UpdateGrid       ; 600 cycles (within budget)
    
Frame3:
    CALL WaitVBLANK
    CALL UpdateColors     ; 600 cycles (within budget)
```

#### Issue 3: Sprite Position Lag (Input Delay)

**Symptoms:**
- Sprite appears one frame behind player input
- Noticeable delay between button press and sprite movement
- Sprite "catches up" when input stops
- Affects gameplay responsiveness

**Root Causes:**

1. **Delayed VDC Update**
   - Input processed in frame N
   - Sprite position updated in memory in frame N
   - VDC register written in frame N+1
   - One-frame lag between input and display

2. **Update Order Issue**
   - VDC updated at start of frame
   - Input processed later in frame
   - New input doesn't affect current frame's display

3. **Double Buffering**
   - Game uses double buffering for sprite data
   - Display buffer lags behind input buffer by one frame

**Diagnostic Techniques:**

**Trace Input and VDC Updates:**
```bash
# Find input reads
grep "IN A, P1" trace.txt > input.txt

# Find sprite position writes
grep "VDC_WRITE\[0x1[0-3]\]" trace.txt > sprite_pos.txt

# Compare frame numbers
```

**Example Trace:**
```
Frame:0100 Scanline:010 Cycle:00100 | IN A, P1          ; Read input (right pressed)
Frame:0100 Scanline:015 Cycle:00200 | MOV sprite_x, A   ; Update memory
Frame:0101 Scanline:195 Cycle:00050 | MOV A, sprite_x   ; Load position (next frame!)
Frame:0101 Scanline:195 Cycle:00070 | OUTL P1, A        ; Write to VDC
```

**Analysis:**
- Input read in frame 100
- Memory updated in frame 100
- VDC updated in frame 101
- **One-frame lag**

**Solutions:**

**1. Update VDC Immediately:**

**Before (Lag):**
```assembly
; Frame N: Read input and update memory
ReadInput:
    IN A, P1              ; Read joystick
    MOV sprite_x, A       ; Store in memory
    RET

; Frame N+1: Update VDC
UpdateVDC:
    MOV A, sprite_x       ; Load from memory
    OUTL P1, A            ; Write to VDC (one frame late!)
    RET
```

**After (No Lag):**
```assembly
; Frame N: Read input and update VDC immediately
ReadInput:
    IN A, P1              ; Read joystick
    MOV sprite_x, A       ; Store in memory
    OUTL P1, A            ; Write to VDC immediately (same frame!)
    RET
```

**2. Reorder Update Sequence:**

**Before (Lag):**
```assembly
MainLoop:
    CALL UpdateVDC        ; Update display first (uses old data)
    CALL ReadInput        ; Read input second (too late for this frame)
    CALL GameLogic        ; Process game logic
    JMP MainLoop
```

**After (No Lag):**
```assembly
MainLoop:
    CALL ReadInput        ; Read input first
    CALL GameLogic        ; Process game logic
    CALL UpdateVDC        ; Update display last (uses current data)
    JMP MainLoop
```

**3. Predictive Input:**

For games where lag is unavoidable, predict next position:

```assembly
; Predict sprite position based on velocity
ReadInput:
    IN A, P1              ; Read joystick
    MOV velocity, A       ; Store velocity
    
    MOV A, sprite_x       ; Load current position
    ADD A, velocity       ; Add velocity
    ADD A, velocity       ; Add again (predict next frame)
    MOV sprite_x, A       ; Store predicted position
    OUTL P1, A            ; Update VDC with prediction
    RET
```

#### Issue 4: Collision Detection Failures

**Symptoms:**
- Valid collisions not detected
- Intermittent collision detection (works sometimes, fails other times)
- Collision detected one frame late
- Collision register always reads zero

**Root Causes:**

1. **Early Collision Read**
   - Collision register read before VDC detects collision
   - VDC needs time to compare sprite positions
   - Read occurs too early in frame

2. **Collision Register Race**
   - Read collision register
   - Clear collision register
   - New collision occurs between read and clear
   - New collision lost

3. **Collision Cleared Too Early**
   - Collision register cleared before game logic reads it
   - Multiple systems reading collision register
   - First reader clears it, second reader sees zero

**Diagnostic Techniques:**

**Trace Collision Register Access:**
```bash
# Find collision register reads (0xA2)
grep "IN A, P2" trace.txt | grep "0xA2"

# Find collision register writes (clear)
grep "OUTL P2" trace.txt | grep "0xA2"

# Find sprite position updates
grep "VDC_WRITE\[0x1[0-3]\]" trace.txt
```

**Example Trace (Early Read):**
```
Frame:0100 Scanline:050 Cycle:00100 | VDC_WRITE[0x10] = 0x64    ; Player Y = 100
Frame:0100 Scanline:050 Cycle:00120 | VDC_WRITE[0x11] = 0x50    ; Player X = 80
Frame:0100 Scanline:051 Cycle:00100 | VDC_WRITE[0x12] = 0x64    ; Enemy Y = 100
Frame:0100 Scanline:051 Cycle:00120 | VDC_WRITE[0x13] = 0x50    ; Enemy X = 80
Frame:0100 Scanline:052 Cycle:00000 | IN A, P2                  ; Read collision (too early!)
Frame:0100 Scanline:052 Cycle:00020 | MOV R0, A                 ; Store collision (0x00)
```

**Analysis:**
- Sprites at same position (collision should occur)
- Collision read at scanline 52 (immediately after position update)
- VDC hasn't had time to detect collision yet
- **Solution:** Read collision later in frame

**Example Trace (Race Condition):**
```
Frame:0100 Scanline:100 Cycle:00100 | IN A, P2          ; Read collision (0x00)
Frame:0100 Scanline:100 Cycle:00120 | MOV R0, A         ; Store collision
Frame:0100 Scanline:100 Cycle:00130 | MOV A, #0x00      ; Clear collision
Frame:0100 Scanline:100 Cycle:00140 | OUTL P2, A        ; Write clear
; [Collision occurs here between cycles 100-140]
Frame:0100 Scanline:100 Cycle:00160 | MOV A, R0         ; Load collision (0x00)
Frame:0100 Scanline:100 Cycle:00170 | JZ NoCollision    ; Jump (collision missed!)
```

**Analysis:**
- 40 VDC cycles between read and clear
- Collision can occur in this window
- New collision immediately cleared
- **Solution:** Minimize gap between read and clear

**Solutions:**

**1. Read Collision Later:**

**Before (Too Early):**
```assembly
; Read collision immediately after sprite update
UpdateSprites:
    MOV A, sprite_x       ; Scanline 50
    OUTL P1, A            ; Write position
    IN A, P2              ; Read collision (too early!)
    RET
```

**After (Proper Timing):**
```assembly
; Wait for VDC to detect collision
UpdateSprites:
    MOV A, sprite_x       ; Scanline 50
    OUTL P1, A            ; Write position
    ; ... other game logic ...
    CALL WaitScanline100  ; Wait until scanline 100
    IN A, P2              ; Read collision (VDC has detected it)
    RET
```

**2. Minimize Read-Clear Gap:**

**Before (Race Condition):**
```assembly
; Large gap between read and clear
CheckCollision:
    IN A, P2              ; Read collision
    MOV R0, A             ; Store in register
    ; ... many instructions ...
    MOV A, #0x00          ; Clear collision (40 cycles later)
    OUTL P2, A
    MOV A, R0             ; Load collision
    JZ NoCollision
```

**After (Atomic Read-Clear):**
```assembly
; Minimize gap
CheckCollision:
    IN A, P2              ; Read collision
    MOV R0, A             ; Store in register (1 cycle)
    MOV A, #0x00          ; Clear collision (1 cycle)
    OUTL P2, A            ; Write clear (2 cycles) - only 4 cycle gap!
    MOV A, R0             ; Load collision
    JZ NoCollision
```

**3. Read Collision During VBLANK:**

Most reliable approach:

```assembly
; Read collision at end of frame during VBLANK
MainLoop:
    CALL GameLogic        ; Update sprites during frame
    CALL WaitVBLANK       ; Wait for VBLANK
    IN A, P2              ; Read collision (all frame collisions detected)
    MOV collision_flags, A
    MOV A, #0x00
    OUTL P2, A            ; Clear for next frame
    CALL ProcessCollision ; Handle collision
    JMP MainLoop
```

#### Issue 5: NTSC/PAL Timing Differences

**Symptoms:**
- Game runs at different speeds on NTSC vs PAL
- Animation timing differs between systems
- Mid-frame effects appear at different screen positions
- Sound pitch differs between systems

**Root Causes:**

1. **Fixed Delay Loops**
   - Code uses fixed iteration counts for delays
   - Doesn't account for different CPU clock speeds
   - NTSC: 2.79 µs per cycle, PAL: 2.54 µs per cycle

2. **Hard-Coded Scanline Numbers**
   - Code waits for specific scanline numbers
   - NTSC: 192 active scanlines, PAL: 242 active scanlines
   - Same scanline number represents different screen positions

3. **Frame-Based Timing**
   - Code assumes 60 Hz frame rate
   - PAL runs at 50 Hz (16.7% slower)
   - Timing calculations incorrect on PAL

**Diagnostic Techniques:**

**Compare NTSC and PAL Traces:**
```bash
# Capture NTSC trace
videopac --system ntsc --trace --trace-frames 1 --trace-output ntsc.txt bios.bin game.bin

# Capture PAL trace
videopac --system pal --trace --trace-frames 1 --trace-output pal.txt bios.bin game.bin

# Compare timing
diff ntsc.txt pal.txt
```

**Identify Fixed Delays:**
```bash
# Find delay loops
grep -A 5 "DJNZ" disasm.txt | grep -B 5 "MOV R[0-7], #"
```

**Identify Hard-Coded Scanlines:**
```bash
# Find scanline comparisons
grep -E "MOV A, #(100|150|192)" disasm.txt
```

**Solutions:**

**1. System-Adaptive Delays:**

**Before (Fixed):**
```assembly
; Fixed delay (assumes NTSC)
DELAY:
    MOV R0, #100          ; 100 iterations
DELAY_LOOP:
    DJNZ R0, DELAY_LOOP   ; 2 cycles per iteration
    RET                   ; Total: 200 cycles
    ; NTSC: 558 µs, PAL: 508 µs (10% faster on PAL)
```

**After (Adaptive):**
```assembly
; System-adaptive delay
DELAY:
    MOV A, system_type    ; Load system type (0=NTSC, 1=PAL)
    JZ DELAY_NTSC
DELAY_PAL:
    MOV R0, #110          ; 110 iterations for PAL
    JMP DELAY_LOOP
DELAY_NTSC:
    MOV R0, #100          ; 100 iterations for NTSC
DELAY_LOOP:
    DJNZ R0, DELAY_LOOP
    RET
```

**2. Proportional Scanline Positioning:**

**Before (Hard-Coded):**
```assembly
; Wait for scanline 100 (assumes NTSC)
WAIT_SCANLINE:
    IN A, P2              ; Read VDC status
    ; ... check if scanline == 100 ...
    JNZ WAIT_SCANLINE
    ; NTSC: 52% down screen (100/192)
    ; PAL: 41% down screen (100/242) - wrong position!
```

**After (Proportional):**
```assembly
; Wait for 52% down screen (proportional)
WAIT_SCANLINE:
    MOV A, system_type
    JZ WAIT_NTSC
WAIT_PAL:
    MOV R0, #126          ; 52% of 242 = 126
    JMP WAIT_LOOP
WAIT_NTSC:
    MOV R0, #100          ; 52% of 192 = 100
WAIT_LOOP:
    IN A, P2
    ; ... check if scanline == R0 ...
    JNZ WAIT_LOOP
```

**3. Frame-Rate-Independent Timing:**

**Before (Frame-Based):**
```assembly
; Move sprite 1 pixel per frame
; NTSC: 60 pixels/second, PAL: 50 pixels/second (16.7% slower)
MoveSprite:
    MOV A, sprite_x
    INC A                 ; +1 pixel per frame
    MOV sprite_x, A
    RET
```

**After (Time-Based):**
```assembly
; Move sprite 60 pixels per second (both systems)
MoveSprite:
    MOV A, system_type
    JZ MOVE_NTSC
MOVE_PAL:
    ; PAL: Move 1.2 pixels per frame (60/50)
    MOV A, sprite_x
    INC A                 ; +1 pixel
    MOV sprite_x, A
    INC frame_counter
    MOV A, frame_counter
    ANL A, #0x04          ; Every 5th frame
    JZ MOVE_EXTRA
    RET
MOVE_EXTRA:
    MOV A, sprite_x
    INC A                 ; +0.2 pixel (averaged)
    MOV sprite_x, A
    RET
MOVE_NTSC:
    ; NTSC: Move 1 pixel per frame
    MOV A, sprite_x
    INC A
    MOV sprite_x, A
    RET
```

#### Issue 6: Emulator vs Real Hardware Timing Differences

**Symptoms:**
- Game works correctly in emulator but not on real hardware (or vice versa)
- Timing-sensitive effects behave differently
- Collision detection works differently
- Audio timing differs

**Root Causes:**

1. **Emulator Timing Inaccuracies**
   - Cycle-accurate emulation not perfect
   - Rounding errors in timing calculations
   - Instruction timing approximations

2. **Hardware Timing Variations**
   - Real hardware has component tolerances
   - Clock frequencies vary slightly
   - Propagation delays not modeled in emulator

3. **Synchronization Differences**
   - Emulator uses interleaved execution
   - Real hardware has true parallel execution
   - Subtle timing differences in edge cases

**Diagnostic Techniques:**

**Compare Traces:**
```bash
# Capture emulator trace
videopac --trace --trace-frames 1 --trace-output emulator.txt bios.bin game.bin

# Capture real hardware trace (if available)
# Compare specific events
diff emulator.txt hardware.txt
```

**Identify Timing-Sensitive Code:**
```bash
# Find tight timing loops
grep -A 10 "IN A, P2" disasm.txt | grep -B 5 "JB7"

# Find mid-frame VDC updates
grep "VDC_WRITE" trace.txt | grep -v "Scanline:19[2-9]"
```

**Solutions:**

**1. Add Timing Margins:**

**Before (Tight Timing):**
```assembly
; Update sprite at exact scanline
WAIT_SCANLINE_100:
    IN A, P2
    ; ... check scanline == 100 ...
    JNZ WAIT_SCANLINE_100
    OUTL P1, A            ; Update immediately (risky!)
```

**After (Timing Margin):**
```assembly
; Update sprite with margin
WAIT_SCANLINE_95:
    IN A, P2
    ; ... check scanline == 95 ...
    JNZ WAIT_SCANLINE_95
    ; Wait a bit more
    MOV R0, #10
DELAY:
    DJNZ R0, DELAY
    OUTL P1, A            ; Update with margin (safer!)
```

**2. Use VBLANK for Critical Updates:**

Avoid mid-frame updates that depend on precise timing:

```assembly
; Instead of mid-frame color change
; Do all updates during VBLANK
MainLoop:
    CALL WaitVBLANK
    CALL UpdateAllVDC     ; All updates in one place
    CALL GameLogic
    JMP MainLoop
```

**3. Test on Both Systems:**

Always test timing-sensitive code on both emulator and real hardware:

1. Develop and debug in emulator (faster iteration)
2. Test on real hardware (verify timing)
3. Adjust timing margins if needed
4. Re-test in emulator to ensure fix works everywhere

#### Quick Reference: Timing Issue Checklist

When encountering timing issues, check:

**Performance Issues:**
- [ ] FPS below target (< 55 NTSC, < 45 PAL)
- [ ] Frame budget exceeded (> 5,967 cycles NTSC, > 7,880 PAL)
- [ ] Trace logging enabled (disable for performance)
- [ ] Long-running subroutines identified
- [ ] Optimization opportunities found

**Visual Artifacts:**
- [ ] VDC writes during active display (scanlines 0-191 NTSC, 0-241 PAL)
- [ ] VDC writes after VBLANK ends (scanline 0 of next frame)
- [ ] Long update routines exceeding VBLANK duration
- [ ] Missing VBLANK synchronization
- [ ] Updates batched during VBLANK

**Input Lag:**
- [ ] Input read and VDC update in same frame
- [ ] Update order: Input → Logic → VDC
- [ ] No unnecessary delays between input and display
- [ ] Predictive input if lag unavoidable

**Collision Detection:**
- [ ] Collision read after sprite position updates
- [ ] Sufficient time for VDC to detect collision
- [ ] Minimal gap between collision read and clear
- [ ] Collision read during VBLANK (most reliable)

**System Compatibility:**
- [ ] No fixed delay loops (use system-adaptive delays)
- [ ] No hard-coded scanline numbers (use proportional positioning)
- [ ] Frame-rate-independent timing
- [ ] Tested on both NTSC and PAL

**Hardware Compatibility:**
- [ ] Timing margins added for critical updates
- [ ] VBLANK used for important updates
- [ ] Tested on both emulator and real hardware
- [ ] Timing assumptions documented

#### Summary

Common timing issues in Videopac emulation:

1. **Frame Rate Below Target** - CPU budget exceeded, optimize game code or emulator
2. **Screen Tearing/Flickering** - Mid-frame VDC updates, move to VBLANK
3. **Sprite Position Lag** - Delayed VDC updates, update immediately after input
4. **Collision Detection Failures** - Early reads or race conditions, read during VBLANK
5. **NTSC/PAL Differences** - Fixed timing assumptions, use adaptive timing
6. **Emulator vs Hardware** - Timing inaccuracies, add margins and test on both

Understanding these common issues helps quickly identify and resolve timing problems during debugging.

#### References

For related information, see:
- [Videopac Timing Model](#71-videopac-timing-model) - System timing fundamentals
- [Instruction Timing Analysis](#72-instruction-timing-analysis) - Measuring execution time
- [VBLANK Identification](#73-vblank-identification) - Synchronization windows
- [Mid-Frame Update Analysis](#74-mid-frame-update-analysis) - VDC register timing
- [Timing-Dependent Bug Identification](#75-timing-dependent-bug-identification) - Debugging timing bugs
- [Using FPS Display and Metrics](#76-using-fps-display-and-metrics) - Performance monitoring

### 7.8 Examples

This section provides practical examples of timing analysis workflows, demonstrating how to apply the concepts from previous sections to real debugging scenarios. Each example includes trace excerpts, analysis steps, and conclusions.

#### Example 1: Analyzing Instruction Timing in a Sprite Update Routine

**Scenario:** You want to measure how long it takes to update all four sprites during VBLANK.

**Step 1: Capture Trace During VBLANK**

```bash
# Capture trace starting at frame 100 for 1 frame
videopac_headless --trace --trace-start-frame 100 --trace-frames 1 --trace-output sprite_update.txt bios.bin game.bin
```

**Step 2: Locate Sprite Update Routine in Trace**

Search for the UpdateSprites routine (assume it starts at address 0x0500):

```bash
grep "PC:0x0500" sprite_update.txt
```

**Trace Output:**

```
Frame:0100 Scanline:195 Cycle:00100 PC:0x0500 | MOV A, #0x50      ; Sprite 0 X
Frame:0100 Scanline:195 Cycle:00110 PC:0x0502 | OUTL P1, A        ; Write to VDC
Frame:0100 Scanline:195 Cycle:00130 PC:0x0504 | MOV A, #0x60      ; Sprite 0 Y
Frame:0100 Scanline:195 Cycle:00140 PC:0x0506 | OUTL P1, A        ; Write to VDC
Frame:0100 Scanline:195 Cycle:00160 PC:0x0508 | MOV A, #0x02      ; Sprite 0 Color
Frame:0100 Scanline:195 Cycle:00170 PC:0x050A | OUTL P1, A        ; Write to VDC
Frame:0100 Scanline:195 Cycle:00190 PC:0x050C | MOV A, #0x70      ; Sprite 1 X
Frame:0100 Scanline:195 Cycle:00200 PC:0x050E | OUTL P1, A        ; Write to VDC
Frame:0100 Scanline:195 Cycle:00220 PC:0x0510 | MOV A, #0x80      ; Sprite 1 Y
Frame:0100 Scanline:196 Cycle:00003 PC:0x0512 | OUTL P1, A        ; Write to VDC
Frame:0100 Scanline:196 Cycle:00023 PC:0x0514 | MOV A, #0x03      ; Sprite 1 Color
Frame:0100 Scanline:196 Cycle:00033 PC:0x0516 | OUTL P1, A        ; Write to VDC
Frame:0100 Scanline:196 Cycle:00053 PC:0x0518 | RET               ; Return
```

**Step 3: Calculate Execution Time Using VDC Cycles**

- **Start:** Scanline 195, Cycle 100
- **End:** Scanline 196, Cycle 53
- **VDC Cycles Calculation:**
  - Remaining cycles in scanline 195: 227 - 100 = 127 cycles
  - Cycles in scanline 196: 53 cycles
  - Total VDC cycles: 127 + 53 = 180 cycles
- **Execution Time:** 180 × 0.282 µs = 50.76 µs

**Step 4: Count Instructions**

- **Single-cycle instructions:** 6 (MOV A, #data)
- **Dual-cycle instructions:** 7 (6 × OUTL + 1 × RET)
- **Total instruction cycles:** 6 + (7 × 2) = 20 cycles
- **Verification:** 20 × 2.79 µs (NTSC) = 55.8 µs (close to 50.76 µs from VDC cycles)

**Step 5: Performance Assessment**

- **Time per sprite:** 50.76 µs ÷ 2 sprites shown = ~25 µs per sprite
- **Time for all 4 sprites:** 25 µs × 4 = 100 µs
- **VBLANK duration:** 4,490 µs
- **Percentage used:** 100 ÷ 4,490 = 2.2% of VBLANK
- **Conclusion:** Sprite updates are very efficient, leaving 97.8% of VBLANK for other tasks

**Key Takeaways:**
- VDC cycle counts provide precise timing measurements
- Account for scanline boundaries when calculating elapsed cycles
- Compare measured time against VBLANK budget to assess efficiency

---

#### Example 2: Identifying VBLANK Periods in a Game Trace

**Scenario:** You want to verify that a game properly synchronizes VDC updates with VBLANK.

**Step 1: Capture Full Frame Trace**

```bash
videopac_headless --trace --trace-start-frame 50 --trace-frames 2 --trace-output vblank_check.txt bios.bin game.bin
```

**Step 2: Find VBLANK Boundaries**

For NTSC, VBLANK starts at scanline 192:

```bash
# Find VBLANK start
grep "Scanline:192 Cycle:00000" vblank_check.txt

# Find frame transitions (VBLANK end)
grep "Scanline:000 Cycle:00000" vblank_check.txt
```

**Trace Output:**

```
Frame:0050 Scanline:191 Cycle:00220 PC:0x0400 | CALL GameLogic
Frame:0050 Scanline:192 Cycle:00000 PC:0x0600 | CALL WaitVBLANK    ← VBLANK STARTS
Frame:0050 Scanline:192 Cycle:00010 PC:0x0700 | IN A, P2
Frame:0050 Scanline:192 Cycle:00030 PC:0x0702 | JB7 0x0700
Frame:0050 Scanline:193 Cycle:00000 PC:0x0704 | CALL UpdateVDC
Frame:0050 Scanline:195 Cycle:00100 PC:0x0800 | MOV A, sprite0_x
Frame:0050 Scanline:195 Cycle:00110 PC:0x0802 | OUTL P1, A         ; VDC write during VBLANK ✓
...
Frame:0050 Scanline:260 Cycle:00200 PC:0x0900 | RET
Frame:0050 Scanline:261 Cycle:00100 PC:0x0402 | JMP MainLoop
Frame:0051 Scanline:000 Cycle:00000 PC:0x0400 | CALL GameLogic     ← VBLANK ENDS, new frame
```

**Step 3: Verify VDC Writes Occur During VBLANK**

Filter for VDC writes:

```bash
grep "VDC_WRITE\|OUTL P1" vblank_check.txt | head -20
```

**Analysis:**

```
Frame:0050 Scanline:195 Cycle:00110 | VDC_WRITE[0xA3] = 0x50    ; Scanline 195 ≥ 192 ✓
Frame:0050 Scanline:195 Cycle:00140 | VDC_WRITE[0xA4] = 0x60    ; Scanline 195 ≥ 192 ✓
Frame:0050 Scanline:196 Cycle:00023 | VDC_WRITE[0xAB] = 0x02    ; Scanline 196 ≥ 192 ✓
Frame:0050 Scanline:197 Cycle:00100 | VDC_WRITE[0xA5] = 0x70    ; Scanline 197 ≥ 192 ✓
```

**Conclusion:**
- All VDC writes occur at scanlines ≥ 192 (VBLANK for NTSC)
- Game properly synchronizes updates with VBLANK
- No mid-frame updates detected
- **Result:** Correct VBLANK synchronization ✓

**Key Takeaways:**
- VBLANK starts at scanline 192 (NTSC) or 242 (PAL)
- Frame transitions mark VBLANK end (scanline returns to 0)
- All VDC writes should occur during VBLANK to avoid visual artifacts

---

#### Example 3: Analyzing Mid-Frame VDC Register Updates

**Scenario:** A game has visual tearing, and you suspect mid-frame VDC updates are the cause.

**Step 1: Capture Trace and Filter for VDC Writes**

```bash
videopac_headless --trace --trace-filter vdc --trace-start-frame 100 --trace-frames 1 --trace-output vdc_writes.txt bios.bin game.bin
```

**Step 2: Examine VDC Write Timing**

```
Frame:0100 Scanline:050 Cycle:00100 | VDC_WRITE[0xA3] = 0x50    ← Mid-frame! (50 < 192)
Frame:0100 Scanline:050 Cycle:00120 | VDC_WRITE[0xA4] = 0x60    ← Mid-frame! (50 < 192)
Frame:0100 Scanline:100 Cycle:00200 | VDC_WRITE[0xAB] = 0x02    ← Mid-frame! (100 < 192)
Frame:0100 Scanline:150 Cycle:00150 | VDC_WRITE[0xA5] = 0x70    ← Mid-frame! (150 < 192)
Frame:0100 Scanline:195 Cycle:00100 | VDC_WRITE[0xA6] = 0x80    ← During VBLANK ✓
Frame:0100 Scanline:196 Cycle:00050 | VDC_WRITE[0xAC] = 0x03    ← During VBLANK ✓
```

**Step 3: Identify Problem Pattern**

- **Scanlines 50, 100, 150:** Active display period (< 192)
- **Registers affected:** 0xA3 (sprite X), 0xA4 (sprite Y), 0xAB (sprite color)
- **Pattern:** Sprite position and color updates during active display

**Step 4: Locate Code Responsible**

Find the code that writes to these registers:

```bash
grep "VDC_WRITE\[0xA3\]" vdc_writes.txt
```

```
Frame:0100 Scanline:050 Cycle:00100 PC:0x0450 | VDC_WRITE[0xA3] = 0x50
```

Check disassembly at address 0x0450:

```assembly
0x0450: MOV A, sprite_x_pos    ; Load sprite X position
0x0452: OUTL P1, A             ; Write to VDC (mid-frame!)
```

**Step 5: Root Cause Analysis**

The game updates sprite positions immediately after processing input, without waiting for VBLANK. This causes:
- **Visual tearing:** Sprite position changes mid-frame
- **Flickering:** Sprite appears in two positions briefly
- **Inconsistent rendering:** Some frames show old position, some show new

**Step 6: Recommended Fix**

Move VDC updates to VBLANK:

```assembly
; Store new position in RAM
0x0450: MOV A, new_sprite_x
0x0452: MOV sprite_x_pos, A    ; Save to RAM (don't write to VDC yet)

; Later, during VBLANK:
0x0600: MOV A, sprite_x_pos    ; Load from RAM
0x0602: OUTL P1, A             ; Write to VDC during VBLANK
```

**Key Takeaways:**
- Mid-frame VDC writes (scanline < 192 NTSC or < 242 PAL) cause visual artifacts
- Filter traces for VDC writes and check scanline numbers
- Move VDC updates to VBLANK by buffering values in RAM

---

#### Example 4: Identifying a Timing-Dependent Bug (Race Condition)

**Scenario:** Collision detection works inconsistently - sometimes collisions are detected, sometimes they're missed.

**Step 1: Capture Trace with Collision Detection**

```bash
videopac_headless --trace --trace-start-key "1" --trace-frames 5 --trace-output collision_trace.txt bios.bin game.bin
```

**Step 2: Find Collision Register Reads**

The collision register is at 0xA2. Search for reads:

```bash
grep "VDC_READ\[0xA2\]\|IN A, P2" collision_trace.txt | head -20
```

**Trace Output (Frame with Missed Collision):**

```
Frame:0100 Scanline:010 Cycle:00100 PC:0x0500 | MOV A, sprite0_x   ; Update sprite 0 position
Frame:0100 Scanline:010 Cycle:00110 PC:0x0502 | OUTL P1, A
Frame:0100 Scanline:010 Cycle:00130 PC:0x0504 | MOV A, sprite1_x   ; Update sprite 1 position
Frame:0100 Scanline:010 Cycle:00140 PC:0x0506 | OUTL P1, A
Frame:0100 Scanline:010 Cycle:00160 PC:0x0508 | IN A, P2           ; Read collision register
Frame:0100 Scanline:010 Cycle:00180 | VDC_READ[0xA2] = 0x00        ; No collision detected!
```

**Trace Output (Frame with Detected Collision):**

```
Frame:0105 Scanline:195 Cycle:00100 PC:0x0500 | MOV A, sprite0_x   ; Update sprite 0 position
Frame:0105 Scanline:195 Cycle:00110 PC:0x0502 | OUTL P1, A
Frame:0105 Scanline:195 Cycle:00130 PC:0x0504 | MOV A, sprite1_x   ; Update sprite 1 position
Frame:0105 Scanline:195 Cycle:00140 PC:0x0506 | OUTL P1, A
Frame:0105 Scanline:196 Cycle:00100 PC:0x0508 | IN A, P2           ; Read collision register
Frame:0105 Scanline:196 Cycle:00120 | VDC_READ[0xA2] = 0x03        ; Collision detected! ✓
```

**Step 3: Analyze Timing Difference**

**Missed Collision (Frame 100):**
- Sprite updates at scanline 10 (active display)
- Collision read at scanline 10, cycle 180
- Time between update and read: 180 - 140 = 40 VDC cycles = 11.3 µs

**Detected Collision (Frame 105):**
- Sprite updates at scanline 195 (VBLANK)
- Collision read at scanline 196, cycle 100
- Time between update and read: (227 - 140) + 100 = 187 VDC cycles = 52.7 µs

**Step 4: Root Cause**

The VDC needs time to detect collisions after sprite positions are updated. Reading the collision register too quickly (11.3 µs) doesn't give the VDC enough time. Reading after a longer delay (52.7 µs) allows collision detection to complete.

**Step 5: Recommended Fix**

Add a delay between sprite updates and collision reads:

```assembly
; Update sprite positions
0x0500: MOV A, sprite0_x
0x0502: OUTL P1, A
0x0504: MOV A, sprite1_x
0x0506: OUTL P1, A

; Add delay (at least 50 µs / 2.79 µs = 18 instruction cycles)
0x0508: MOV R0, #9              ; 1 cycle
0x050A: DJNZ R0, 0x050A         ; 9 × 2 = 18 cycles
                                ; Total: 19 cycles = 53 µs

; Now read collision register
0x050C: IN A, P2                ; Read collision
```

**Key Takeaways:**
- Race conditions occur when timing assumptions are violated
- VDC operations (collision detection) need time to complete
- Compare successful and failed cases to identify timing differences
- Add delays or move operations to VBLANK for reliable timing

---

#### Example 5: Using FPS Display to Identify Performance Issues

**Scenario:** A game feels sluggish, and you want to measure the actual frame rate.

**Step 1: Enable FPS Display**

Run the emulator with FPS display enabled:

```bash
videopac --fps-display bios.bin game.bin
```

**Step 2: Observe FPS During Gameplay**

**Normal Gameplay:**
```
FPS: 59.8 (NTSC target: 60.0)
Frame Time: 16.7 ms (Target: 16.7 ms)
CPU Usage: 85%
```

**During Intense Action (Many Enemies):**
```
FPS: 45.2 (NTSC target: 60.0)    ← Below target!
Frame Time: 22.1 ms (Target: 16.7 ms)
CPU Usage: 135%                   ← Exceeding budget!
```

**Step 3: Capture Trace During Slowdown**

```bash
videopac_headless --trace --trace-start-frame 200 --trace-frames 1 --trace-output slowdown_trace.txt bios.bin game.bin
```

**Step 4: Measure Frame Time in Trace**

```
Frame:0200 Scanline:000 Cycle:00000 | Frame Start
...
Frame:0201 Scanline:000 Cycle:00000 | Frame End (next frame start)
```

Count total instructions in the frame:

```bash
wc -l slowdown_trace.txt
```

Output: `8500 lines` (approximately 8,500 instructions)

**Step 5: Calculate CPU Cycles Used**

Estimate instruction mix:
- Assume 20% dual-cycle instructions: 8,500 × 0.20 = 1,700
- Assume 80% single-cycle instructions: 8,500 × 0.80 = 6,800
- Total cycles: 6,800 + (1,700 × 2) = 10,200 cycles

**Step 6: Compare with Budget**

- **Available cycles (NTSC):** 5,967 cycles per frame
- **Used cycles:** 10,200 cycles
- **Deficit:** 10,200 - 5,967 = 4,233 cycles (71% over budget!)
- **Actual frame time:** 10,200 ÷ 358,000 = 28.5 ms
- **Actual frame rate:** 1,000 ÷ 28.5 = 35.1 Hz

**Step 7: Identify Bottleneck**

Search for long-running subroutines:

```bash
grep "CALL" slowdown_trace.txt | sort | uniq -c | sort -rn | head -5
```

Output:
```
1250 CALL UpdateEnemies    ← Called frequently!
 500 CALL CheckCollisions
 300 CALL UpdatePlayer
 200 CALL UpdateVDC
 150 CALL ReadInput
```

Measure UpdateEnemies execution time:

```
Frame:0200 Scanline:050 Cycle:00000 PC:0x0800 | CALL UpdateEnemies
Frame:0200 Scanline:150 Cycle:00100 PC:0x0850 | RET
```

- **Scanlines:** 100 scanlines
- **VDC cycles:** 100 × 227 + 100 = 22,800 cycles
- **Time:** 22,800 × 0.282 µs = 6,430 µs (38% of frame time!)

**Step 8: Optimization Strategy**

UpdateEnemies is the bottleneck. Optimization options:
1. **Reduce enemy count** during intense action
2. **Optimize enemy update algorithm** (fewer instructions per enemy)
3. **Update enemies every other frame** (30 Hz update rate)
4. **Use simpler collision detection** for distant enemies

**Key Takeaways:**
- FPS display provides real-time performance feedback
- Frame time exceeding budget causes slowdown
- Trace analysis identifies specific bottlenecks
- Optimize hot paths (frequently executed code) for maximum impact

---

#### Example 6: Comparing NTSC vs PAL Timing

**Scenario:** A game runs correctly on NTSC but has timing issues on PAL.

**Step 1: Capture Traces on Both Systems**

```bash
# NTSC trace
videopac_headless --system ntsc --trace --trace-frames 1 --trace-output ntsc_trace.txt bios.bin game.bin

# PAL trace
videopac_headless --system pal --trace --trace-frames 1 --trace-output pal_trace.txt bios.bin game.bin
```

**Step 2: Compare VBLANK Timing**

**NTSC:**
```
Frame:0001 Scanline:192 Cycle:00000 | VBLANK START
Frame:0001 Scanline:261 Cycle:00226 | VBLANK END
```
- VBLANK: Scanlines 192-261 (70 scanlines)

**PAL:**
```
Frame:0001 Scanline:242 Cycle:00000 | VBLANK START
Frame:0001 Scanline:311 Cycle:00226 | VBLANK END
```
- VBLANK: Scanlines 242-311 (70 scanlines)

**Step 3: Identify Timing Assumption**

Search for hard-coded scanline checks:

```assembly
; NTSC-specific code (BUG!)
0x0600: IN A, P2            ; Read VDC status
0x0602: MOV R0, A
0x0604: ANL A, #0x7F        ; Mask scanline bits
0x0606: ADD A, #-192        ; Compare with 192 (NTSC VBLANK start)
0x0608: JC NotVBLANK        ; Jump if scanline < 192
```

**Problem:** This code assumes VBLANK starts at scanline 192, which is only true for NTSC. On PAL, VBLANK starts at scanline 242, so this check fails.

**Step 4: Verify Issue in PAL Trace**

```
Frame:0001 Scanline:242 Cycle:00000 PC:0x0600 | IN A, P2
Frame:0001 Scanline:242 Cycle:00020 PC:0x0602 | MOV R0, A
Frame:0001 Scanline:242 Cycle:00030 PC:0x0604 | ANL A, #0x7F
Frame:0001 Scanline:242 Cycle:00040 PC:0x0606 | ADD A, #-192
Frame:0001 Scanline:242 Cycle:00050 PC:0x0608 | JC NotVBLANK    ← Wrong! 242 ≥ 192, but not VBLANK check
```

The code thinks scanline 242 is past VBLANK (since 242 > 192), but it's actually the start of VBLANK on PAL.

**Step 5: Recommended Fix**

Use VDC status register instead of hard-coded scanline numbers:

```assembly
; System-agnostic code (CORRECT)
0x0600: IN A, P2            ; Read VDC status register
0x0602: JB7 InVBLANK        ; Bit 7 = VBLANK flag (set during VBLANK)
0x0604: JMP NotVBLANK
InVBLANK:
    ; VDC update code here
```

**Key Takeaways:**
- Avoid hard-coded scanline numbers (NTSC: 192, PAL: 242)
- Use VDC status register for system-agnostic VBLANK detection
- Test games on both NTSC and PAL systems
- PAL has more CPU cycles per frame (7,880 vs 5,967) but same VBLANK duration

---

#### Summary of Examples

These examples demonstrate practical timing analysis techniques:

1. **Example 1:** Measuring instruction timing using VDC cycle counts
2. **Example 2:** Identifying VBLANK periods and verifying synchronization
3. **Example 3:** Detecting mid-frame VDC updates causing visual artifacts
4. **Example 4:** Diagnosing race conditions in collision detection
5. **Example 5:** Using FPS display to identify performance bottlenecks
6. **Example 6:** Comparing NTSC vs PAL timing and fixing compatibility issues

**Key Debugging Techniques:**
- Use VDC cycle counts for precise timing measurements
- Check scanline numbers to identify VBLANK vs active display
- Compare successful and failed cases to find timing differences
- Measure frame time and compare with budget
- Identify bottlenecks using trace analysis
- Test on both NTSC and PAL systems

**Common Patterns:**
- VDC updates should occur during VBLANK (scanline ≥ 192 NTSC, ≥ 242 PAL)
- Collision detection needs time after sprite updates (~50 µs minimum)
- Frame budget: 5,967 cycles (NTSC) or 7,880 cycles (PAL)
- Use VDC status register for system-agnostic timing

For more information, see:
- [Videopac Timing Model](#71-videopac-timing-model) - System timing fundamentals
- [Instruction Timing Analysis](#72-instruction-timing-analysis) - Measuring execution time
- [VBLANK Identification](#73-vblank-identification) - Finding VBLANK periods
- [Mid-Frame Update Analysis](#74-mid-frame-update-analysis) - Analyzing VDC write timing
- [Timing-Dependent Bug Identification](#75-timing-dependent-bug-identification) - Debugging timing bugs
- [Using FPS Display and Metrics](#76-using-fps-display-and-metrics) - Performance monitoring
- [Common Timing Issues](#77-common-timing-issues) - Typical timing problems

[↑ Back to Top](#table-of-contents)

---

## 8. Audio Debugging

### 8.1 Videopac Audio System Overview

The Videopac audio system is built around a 24-bit shift register that generates audio waveforms. Understanding this system is essential for debugging sound-related issues in games.

**Core Components:**

1. **24-Bit Sound Shift Register**
   - Three 8-bit registers (0xA7, 0xA8, 0xA9) form a 24-bit shift register
   - Register 0xA7 contains bits 0-7 (least significant byte)
   - Register 0xA8 contains bits 8-15 (middle byte)
   - Register 0xA9 contains bits 16-23 (most significant byte)
   - The shift register shifts right by 1 bit at a controlled frequency
   - Bit 0 (output bit) determines the audio output level

2. **Sound Control Register (0xAA)**
   - Bit 7: Enable sound (1 = enabled, 0 = disabled)
   - Bit 6: Loop mode (1 = recirculate bit 0 to bit 23)
   - Bit 5: Shift frequency (0 = 983Hz, 1 = 3933Hz)
   - Bit 4: Noise mode (1 = XOR feedback for noise generation)
   - Bits 0-3: Volume (0-15, where 15 is maximum volume)

**How Audio Generation Works:**

1. **Pattern Loading**
   - Games write a 24-bit pattern to registers 0xA7-0xA9
   - This pattern represents the waveform to be played
   - Different patterns create different tones and effects

2. **Shift Operation**
   - The shift register shifts right at the selected frequency (983Hz or 3933Hz)
   - Bit 0 is output as the audio sample (0 or 1)
   - The output bit is scaled by the volume setting (0-15)

3. **Loop Mode**
   - When enabled, bit 0 is recirculated to bit 23 after shifting
   - This creates a repeating pattern for sustained tones
   - Without loop mode, the register eventually becomes all zeros (silence)

4. **Noise Mode**
   - When enabled, uses XOR feedback to generate pseudo-random patterns
   - Creates noise effects for explosions, crashes, etc.
   - Feedback bit is XORed from bits 0 and 1, then placed at bit 23

**Frequency Control:**

The shift frequency determines the pitch of the sound:
- **Low frequency (983Hz)**: Slower shifting, lower pitch tones
- **High frequency (3933Hz)**: Faster shifting, higher pitch tones

The actual audio frequency depends on both the shift frequency and the pattern in the shift register. For example, a pattern with 12 bits set to 1 followed by 12 bits set to 0 will produce a tone at shift_frequency / 24.

**Audio Output:**

- The output bit (bit 0) is either 0 or 1
- This is scaled by the volume (0-15) to produce the final audio sample
- Volume 0 = silence, Volume 15 = maximum amplitude
- The audio system produces a square wave output

**Common Audio Patterns:**

Games typically use these approaches:
- **Simple tones**: Alternating patterns (e.g., 0xFFFFFF, 0x000000 for square wave)
- **Complex tones**: Custom patterns for specific sound effects
- **Noise**: Enable noise mode for explosion/crash sounds
- **Silence**: Write 0x000000 or disable sound (bit 7 of 0xAA = 0)

**Debugging Implications:**

When debugging audio issues, check:
1. Is sound enabled in register 0xAA (bit 7)?
2. What pattern is loaded in registers 0xA7-0xA9?
3. What is the shift frequency setting (bit 5 of 0xAA)?
4. What is the volume setting (bits 0-3 of 0xAA)?
5. Is loop mode or noise mode enabled?
6. When are the audio registers being written (timing)?

### 8.2 Audio VDC Registers

The Videopac audio system uses four VDC registers to control sound generation. Understanding these registers is crucial for debugging audio issues.

**Register Summary:**

| Register | Address | Name | Description |
|----------|---------|------|-------------|
| 0xA7 | Sound Byte 0 | Sound shift register bits 0-7 (LSB) |
| 0xA8 | Sound Byte 1 | Sound shift register bits 8-15 |
| 0xA9 | Sound Byte 2 | Sound shift register bits 16-23 (MSB) |
| 0xAA | Sound Control | Sound control and volume |

#### Register 0xA7 - Sound Shift Register Byte 0 (LSB)

**Address:** 0xA7  
**Access:** Write-only  
**Function:** Least significant byte of the 24-bit sound shift register

```
Bit:  7   6   5   4   3   2   1   0
      |   |   |   |   |   |   |   |
      +---+---+---+---+---+---+---+--- Sound pattern bits 0-7
```

This register holds bits 0-7 of the 24-bit sound pattern. Bit 0 is the output bit that determines the current audio sample value.

**Usage:**
- Write the least significant byte of your sound pattern to this register
- This byte is shifted right during audio generation
- Bit 0 is output as the audio sample (0 or 1)

**Example:**
```
MOV A, #0xFF    ; Load pattern 0xFF (all bits set)
OUTL P2, A      ; Write to port 2
MOV A, #0xA7    ; Select register 0xA7
OUTL P1, A      ; Write register address
```

#### Register 0xA8 - Sound Shift Register Byte 1

**Address:** 0xA8  
**Access:** Write-only  
**Function:** Middle byte of the 24-bit sound shift register

```
Bit:  7   6   5   4   3   2   1   0
      |   |   |   |   |   |   |   |
      +---+---+---+---+---+---+---+--- Sound pattern bits 8-15
```

This register holds bits 8-15 of the 24-bit sound pattern.

**Usage:**
- Write the middle byte of your sound pattern to this register
- This byte is part of the overall 24-bit pattern that gets shifted

**Example:**
```
MOV A, #0x00    ; Load pattern 0x00 (all bits clear)
OUTL P2, A      ; Write to port 2
MOV A, #0xA8    ; Select register 0xA8
OUTL P1, A      ; Write register address
```

#### Register 0xA9 - Sound Shift Register Byte 2 (MSB)

**Address:** 0xA9  
**Access:** Write-only  
**Function:** Most significant byte of the 24-bit sound shift register

```
Bit:  7   6   5   4   3   2   1   0
      |   |   |   |   |   |   |   |
      +---+---+---+---+---+---+---+--- Sound pattern bits 16-23
```

This register holds bits 16-23 of the 24-bit sound pattern.

**Usage:**
- Write the most significant byte of your sound pattern to this register
- In loop mode, bit 0 of register 0xA7 is recirculated to bit 23 (this register)

**Example:**
```
MOV A, #0xFF    ; Load pattern 0xFF (all bits set)
OUTL P2, A      ; Write to port 2
MOV A, #0xA9    ; Select register 0xA9
OUTL P1, A      ; Write register address
```

#### Register 0xAA - Sound Control

**Address:** 0xAA  
**Access:** Write-only  
**Function:** Sound control, frequency, mode, and volume

```
Bit:  7   6   5   4   3   2   1   0
      |   |   |   |   |   |   |   |
      |   |   |   |   +---+---+---+--- Volume (0-15)
      |   |   |   +------------------- Noise mode (1 = enabled)
      |   |   +----------------------- Shift frequency (0 = 983Hz, 1 = 3933Hz)
      |   +--------------------------- Loop mode (1 = recirculate bit 0)
      +------------------------------- Sound enable (1 = enabled, 0 = disabled)
```

**Bit Definitions:**

- **Bit 7 (Sound Enable):**
  - 0 = Sound disabled (silence)
  - 1 = Sound enabled (shift register active)

- **Bit 6 (Loop Mode):**
  - 0 = One-shot mode (register shifts to all zeros)
  - 1 = Loop mode (bit 0 recirculates to bit 23)

- **Bit 5 (Shift Frequency):**
  - 0 = Low frequency (983Hz shift rate)
  - 1 = High frequency (3933Hz shift rate)

- **Bit 4 (Noise Mode):**
  - 0 = Normal mode (pattern shifts as-is)
  - 1 = Noise mode (XOR feedback generates pseudo-random pattern)

- **Bits 0-3 (Volume):**
  - 0 = Silence (minimum volume)
  - 15 = Maximum volume
  - Linear scaling of output amplitude

**Usage:**
```
MOV A, #0xCF    ; Enable sound (bit 7), loop mode (bit 6),
                ; high frequency (bit 5), max volume (bits 0-3 = 15)
OUTL P2, A      ; Write to port 2
MOV A, #0xAA    ; Select register 0xAA
OUTL P1, A      ; Write register address
```

**Common Control Values:**

| Value | Binary | Description |
|-------|--------|-------------|
| 0x00 | 00000000 | Sound disabled, silence |
| 0x8F | 10001111 | Sound enabled, one-shot, low freq, max volume |
| 0xCF | 11001111 | Sound enabled, loop, high freq, max volume |
| 0xDF | 11011111 | Sound enabled, loop, high freq, noise, max volume |
| 0x87 | 10000111 | Sound enabled, one-shot, low freq, volume 7 |

**Debugging Tips:**

1. **Check Sound Enable (Bit 7):**
   - If bit 7 is 0, no sound will be produced regardless of other settings
   - This is the most common cause of "no sound" issues

2. **Check Volume (Bits 0-3):**
   - Volume 0 produces silence even if sound is enabled
   - Verify volume is set to a reasonable value (8-15 for audible sound)

3. **Check Loop Mode (Bit 6):**
   - Without loop mode, sound will fade to silence after 24 shifts
   - Sustained tones require loop mode enabled

4. **Check Shift Frequency (Bit 5):**
   - Low frequency (983Hz) produces lower pitch tones
   - High frequency (3933Hz) produces higher pitch tones
   - Incorrect frequency can make sound too low or too high to hear properly

5. **Check Noise Mode (Bit 4):**
   - Noise mode should only be enabled for noise effects
   - If enabled for tones, it will produce unpredictable sounds

### 8.3 Tracing Audio Register Writes

Tracing audio register writes is essential for understanding when and how games generate sound. This section covers techniques for capturing and analyzing audio-related VDC register writes.

#### Basic Audio Trace Capture

To capture audio register writes, use the trace filtering feature to focus on VDC registers 0xA7-0xAA:

```bash
# Capture all VDC writes (includes audio registers)
videopac_headless --trace --trace-filter vdc --trace-output audio_trace.txt bios.bin game.bin

# Capture trace starting at a specific frame
videopac_headless --trace --trace-start-frame 100 --trace-frames 10 \
  --trace-filter vdc --trace-output audio_trace.txt bios.bin game.bin

# Capture trace when a key is pressed (e.g., when sound should start)
videopac_headless --trace --trace-start-key "1" --trace-frames 5 \
  --trace-filter vdc --trace-output audio_trace.txt bios.bin game.bin
```

#### Filtering Audio Registers from Trace

Once you have a VDC trace, you can filter for audio-specific registers using grep or similar tools:

```bash
# Extract only audio register writes (0xA7-0xAA)
grep "VDC_WRITE\[0xA[7-9A]\]" audio_trace.txt > audio_only.txt

# Extract sound control register writes only
grep "VDC_WRITE\[0xAA\]" audio_trace.txt > sound_control.txt

# Extract sound pattern writes only
grep -E "VDC_WRITE\[0xA[789]\]" audio_trace.txt > sound_pattern.txt
```

#### Reading Audio Trace Output

Audio trace output follows the standard trace format:

```
Frame:0001 Scanline:010 Cycle:00123 PC:0x0234 A:0xFF PSW:0x08 | VDC_WRITE[0xA7] = 0xFF
Frame:0001 Scanline:010 Cycle:00125 PC:0x0236 A:0x00 PSW:0x08 | VDC_WRITE[0xA8] = 0x00
Frame:0001 Scanline:010 Cycle:00127 PC:0x0238 A:0xFF PSW:0x08 | VDC_WRITE[0xA9] = 0xFF
Frame:0001 Scanline:010 Cycle:00129 PC:0x023A A:0xCF PSW:0x08 | VDC_WRITE[0xAA] = 0xCF
```

**Key Information:**

- **Frame:** Which video frame the write occurred in
- **Scanline:** Which scanline within the frame (0-261)
- **Cycle:** CPU cycle count within the frame
- **PC:** Program counter (address of the instruction)
- **A:** Accumulator value (the value being written)
- **Register:** Which audio register is being written (0xA7-0xAA)
- **Value:** The value being written to the register

#### Analyzing Audio Register Write Patterns

**1. Identify Sound Initialization:**

Look for the sequence of writes that set up a sound:

```
VDC_WRITE[0xA7] = 0xFF    ; Load pattern byte 0
VDC_WRITE[0xA8] = 0x00    ; Load pattern byte 1
VDC_WRITE[0xA9] = 0xFF    ; Load pattern byte 2
VDC_WRITE[0xAA] = 0xCF    ; Enable sound with loop, high freq, max volume
```

This pattern indicates the game is setting up a 24-bit sound pattern (0xFF00FF) and enabling sound.

**2. Identify Sound Updates:**

Games may update audio registers every frame or only when sound changes:

```
Frame:0001 ... VDC_WRITE[0xAA] = 0xCF    ; Sound enabled
Frame:0002 ... VDC_WRITE[0xAA] = 0xCF    ; Sound still enabled (same value)
Frame:0003 ... VDC_WRITE[0xAA] = 0xCF    ; Sound still enabled
Frame:0004 ... VDC_WRITE[0xAA] = 0x00    ; Sound disabled
```

**3. Identify Sound Effects:**

Short-duration sounds (explosions, beeps) typically:
- Write a pattern to 0xA7-0xA9
- Enable sound with one-shot mode (bit 6 = 0)
- Disable sound after a few frames

```
Frame:0010 ... VDC_WRITE[0xA7] = 0xAA
Frame:0010 ... VDC_WRITE[0xA8] = 0x55
Frame:0010 ... VDC_WRITE[0xA9] = 0xAA
Frame:0010 ... VDC_WRITE[0xAA] = 0x8F    ; One-shot mode (bit 6 = 0)
Frame:0015 ... VDC_WRITE[0xAA] = 0x00    ; Disable sound
```

**4. Identify Background Music:**

Continuous sounds (music, engine noise) typically:
- Write a pattern to 0xA7-0xA9
- Enable sound with loop mode (bit 6 = 1)
- Keep sound enabled across many frames
- May update pattern periodically for melody changes

```
Frame:0001 ... VDC_WRITE[0xA7] = 0xFF
Frame:0001 ... VDC_WRITE[0xA8] = 0x00
Frame:0001 ... VDC_WRITE[0xA9] = 0xFF
Frame:0001 ... VDC_WRITE[0xAA] = 0xCF    ; Loop mode enabled
... (many frames with no audio writes)
Frame:0100 ... VDC_WRITE[0xA7] = 0xF0    ; Pattern change (melody)
Frame:0100 ... VDC_WRITE[0xA8] = 0x0F
Frame:0100 ... VDC_WRITE[0xA9] = 0xF0
```

#### Timing Analysis

**VBLANK vs Mid-Frame Writes:**

Audio register writes can occur during VBLANK or mid-frame:

```
Frame:0001 Scanline:241 ... VDC_WRITE[0xAA] = 0xCF    ; VBLANK write (typical)
Frame:0002 Scanline:100 ... VDC_WRITE[0xAA] = 0x00    ; Mid-frame write (less common)
```

- **VBLANK writes (scanlines 241-261):** Most common, synchronized with video
- **Mid-frame writes:** Can occur but may cause audio glitches if not handled properly

**Write Frequency:**

Count how often audio registers are updated:

```bash
# Count audio writes per frame
grep "VDC_WRITE\[0xA[7-9A]\]" audio_trace.txt | cut -d: -f2 | cut -d' ' -f1 | sort | uniq -c
```

This shows how many audio writes occur in each frame, helping identify:
- Frames with sound initialization (multiple writes)
- Frames with sound updates (single writes)
- Frames with no audio activity (no writes)

#### Common Trace Patterns

**Pattern 1: Sound Enable/Disable Toggle**

```
Frame:0001 ... VDC_WRITE[0xAA] = 0xCF    ; Enable sound
Frame:0010 ... VDC_WRITE[0xAA] = 0x00    ; Disable sound
Frame:0020 ... VDC_WRITE[0xAA] = 0xCF    ; Enable sound again
```

**Pattern 2: Volume Ramping**

```
Frame:0001 ... VDC_WRITE[0xAA] = 0xCF    ; Volume 15 (max)
Frame:0002 ... VDC_WRITE[0xAA] = 0xCE    ; Volume 14
Frame:0003 ... VDC_WRITE[0xAA] = 0xCD    ; Volume 13
Frame:0004 ... VDC_WRITE[0xAA] = 0xCC    ; Volume 12
```

**Pattern 3: Frequency Switching**

```
Frame:0001 ... VDC_WRITE[0xAA] = 0xCF    ; High frequency (bit 5 = 1)
Frame:0010 ... VDC_WRITE[0xAA] = 0xDF    ; Still high frequency
Frame:0020 ... VDC_WRITE[0xAA] = 0x9F    ; Low frequency (bit 5 = 0)
```

**Pattern 4: Noise Effect**

```
Frame:0001 ... VDC_WRITE[0xAA] = 0xDF    ; Noise mode enabled (bit 4 = 1)
Frame:0005 ... VDC_WRITE[0xAA] = 0x00    ; Noise disabled
```

#### Debugging Workflow

1. **Capture trace when sound should occur:**
   ```bash
   videopac_headless --trace --trace-start-key "1" --trace-frames 10 \
     --trace-filter vdc --trace-output audio_trace.txt bios.bin game.bin
   ```

2. **Filter for audio registers:**
   ```bash
   grep "VDC_WRITE\[0xA[7-9A]\]" audio_trace.txt > audio_only.txt
   ```

3. **Analyze the pattern:**
   - Check if 0xAA is written with bit 7 = 1 (sound enabled)
   - Check if 0xA7-0xA9 are written with a pattern
   - Check the timing of writes (which frame, which scanline)
   - Check the volume setting (bits 0-3 of 0xAA)

4. **Compare with expected behavior:**
   - Does the game write audio registers when sound should play?
   - Are the register values correct for the desired sound?
   - Is the timing appropriate (VBLANK vs mid-frame)?

5. **Identify discrepancies:**
   - Missing writes (game doesn't write audio registers)
   - Incorrect values (wrong pattern, volume, or control bits)
   - Incorrect timing (writes at wrong time)

#### Example: Debugging Silent Game

**Problem:** Game should play sound when '1' is pressed, but no sound is heard.

**Step 1:** Capture trace when '1' is pressed:
```bash
videopac_headless --trace --trace-start-key "1" --trace-frames 5 \
  --trace-filter vdc --trace-output audio_trace.txt bios.bin game.bin
```

**Step 2:** Filter for audio registers:
```bash
grep "VDC_WRITE\[0xA[7-9A]\]" audio_trace.txt > audio_only.txt
```

**Step 3:** Examine audio_only.txt:
```
Frame:0001 Scanline:245 Cycle:12345 PC:0x0234 A:0xFF PSW:0x08 | VDC_WRITE[0xA7] = 0xFF
Frame:0001 Scanline:245 Cycle:12347 PC:0x0236 A:0x00 PSW:0x08 | VDC_WRITE[0xA8] = 0x00
Frame:0001 Scanline:245 Cycle:12349 PC:0x0238 A:0xFF PSW:0x08 | VDC_WRITE[0xA9] = 0xFF
Frame:0001 Scanline:245 Cycle:12351 PC:0x023A A:0x4F PSW:0x08 | VDC_WRITE[0xAA] = 0x4F
```

**Step 4:** Analyze register 0xAA value (0x4F = 01001111):
- Bit 7 = 0: Sound is DISABLED!
- Bit 6 = 1: Loop mode enabled
- Bit 5 = 0: Low frequency
- Bit 4 = 0: Normal mode
- Bits 0-3 = 15: Max volume

**Conclusion:** The game writes a sound pattern but doesn't enable sound (bit 7 = 0). This is a bug in the game code or emulator's audio register handling.

### 8.4 Audio Waveform Analysis

Understanding the relationship between register values and the resulting audio waveform is crucial for debugging audio issues. This section explains how to analyze and predict audio output based on register settings.

#### The 24-Bit Shift Register

The audio system uses a 24-bit shift register composed of three 8-bit registers:

```
Register 0xA9 (MSB)    Register 0xA8 (Middle)    Register 0xA7 (LSB)
[23 22 21 20 19 18 17 16] [15 14 13 12 11 10 9 8] [7 6 5 4 3 2 1 0]
                                                                   ^
                                                                   |
                                                              Output bit
```

**Shift Operation:**
1. The entire 24-bit register shifts right by 1 bit
2. Bit 0 (rightmost bit of 0xA7) is output as the audio sample
3. In loop mode, bit 0 is recirculated to bit 23 (leftmost bit of 0xA9)
4. In one-shot mode, bit 23 becomes 0 after each shift

#### Waveform Generation

The output bit (bit 0) determines the audio sample value:
- **Bit 0 = 0:** Low audio level (negative amplitude)
- **Bit 0 = 1:** High audio level (positive amplitude)

The volume setting (bits 0-3 of register 0xAA) scales the amplitude:
- Volume 0: No output (silence)
- Volume 15: Maximum amplitude
- Volumes 1-14: Linear scaling between minimum and maximum

#### Common Waveform Patterns

**1. Square Wave (50% Duty Cycle)**

Pattern: 12 bits high, 12 bits low

```
Register values:
0xA7 = 0xFF (bits 0-7: all high)
0xA8 = 0x0F (bits 8-11: high, bits 12-15: low)
0xA9 = 0x00 (bits 16-23: all low)

Binary representation:
000000000000111111111111
```

This produces a square wave with 50% duty cycle. The frequency depends on the shift rate:
- Low frequency (983Hz): 983Hz / 24 = 40.96Hz tone
- High frequency (3933Hz): 3933Hz / 24 = 163.88Hz tone

**2. Square Wave (25% Duty Cycle)**

Pattern: 6 bits high, 18 bits low

```
Register values:
0xA7 = 0x3F (bits 0-5: high, bits 6-7: low)
0xA8 = 0x00 (bits 8-15: all low)
0xA9 = 0x00 (bits 16-23: all low)

Binary representation:
000000000000000000111111
```

This produces a square wave with 25% duty cycle, creating a different timbre.

**3. Pulse Wave (Short Pulse)**

Pattern: 1 bit high, 23 bits low

```
Register values:
0xA7 = 0x01 (bit 0: high, bits 1-7: low)
0xA8 = 0x00 (bits 8-15: all low)
0xA9 = 0x00 (bits 16-23: all low)

Binary representation:
000000000000000000000001
```

This produces a very short pulse, useful for percussive sounds.

**4. Alternating Pattern**

Pattern: Alternating bits

```
Register values:
0xA7 = 0xAA (10101010)
0xA8 = 0xAA (10101010)
0xA9 = 0xAA (10101010)

Binary representation:
101010101010101010101010
```

This produces the highest possible frequency (shift rate / 2).

#### Frequency Calculation

The audio frequency is determined by:
1. The shift rate (983Hz or 3933Hz)
2. The number of bits in the repeating pattern

**Formula:**
```
Audio Frequency = Shift Rate / Pattern Length
```

**Examples:**

| Pattern | Shift Rate | Pattern Length | Audio Frequency |
|---------|------------|----------------|-----------------|
| 0xFFFFFF / 0x000000 | 983Hz | 24 bits | 40.96Hz |
| 0xFFFFFF / 0x000000 | 3933Hz | 24 bits | 163.88Hz |
| 0x0FFF / 0x000 | 983Hz | 12 bits | 81.92Hz |
| 0x0FFF / 0x000 | 3933Hz | 12 bits | 327.75Hz |
| 0x3F / 0x00 | 983Hz | 6 bits | 163.83Hz |
| 0x3F / 0x00 | 3933Hz | 6 bits | 655.5Hz |
| 0xAAAAAA | 983Hz | 2 bits | 491.5Hz |
| 0xAAAAAA | 3933Hz | 2 bits | 1966.5Hz |

**Note:** In loop mode, the pattern repeats indefinitely. In one-shot mode, the pattern shifts to all zeros after 24 shifts.

#### Analyzing Waveforms from Trace Data

Given a trace with audio register writes, you can reconstruct the waveform:

**Example Trace:**
```
VDC_WRITE[0xA7] = 0xFF
VDC_WRITE[0xA8] = 0x00
VDC_WRITE[0xA9] = 0xFF
VDC_WRITE[0xAA] = 0xCF  (11001111: enabled, loop, high freq, max volume)
```

**Step 1: Reconstruct the 24-bit pattern**
```
0xA9 = 0xFF = 11111111
0xA8 = 0x00 = 00000000
0xA7 = 0xFF = 11111111

24-bit pattern: 111111110000000011111111
```

**Step 2: Identify the repeating unit**

In loop mode, bit 0 recirculates to bit 23, so the pattern repeats every 24 bits.

Looking at the pattern: 111111110000000011111111
- 8 bits high (0xFF from 0xA9)
- 8 bits low (0x00 from 0xA8)
- 8 bits high (0xFF from 0xA7)

This is an 8-8-8 pattern with a 24-bit period.

**Step 3: Calculate frequency**

Shift rate: 3933Hz (high frequency, bit 5 = 1)
Pattern length: 24 bits
Audio frequency: 3933Hz / 24 = 163.88Hz

**Step 4: Determine waveform shape**

The pattern has 16 bits high and 8 bits low, creating a 66.7% duty cycle square wave.

#### Noise Mode Analysis

When noise mode is enabled (bit 4 of 0xAA = 1), the shift register uses XOR feedback to generate pseudo-random patterns.

**Feedback Mechanism:**
1. XOR bits 0 and 1 of the shift register
2. Place the result at bit 23 after shifting
3. This creates a pseudo-random sequence

**Characteristics:**
- Pattern appears random but is deterministic
- Useful for explosion, crash, and static sounds
- The initial pattern (0xA7-0xA9) seeds the random sequence
- Different seeds produce different noise patterns

**Example:**

Initial pattern: 0xFFFFFF (all bits set)

```
Shift 1: 011111111111111111111111 (XOR of bits 0,1 = 1^1 = 0)
Shift 2: 001111111111111111111111 (XOR of bits 0,1 = 1^1 = 0)
Shift 3: 100111111111111111111111 (XOR of bits 0,1 = 1^1 = 0, then 1^0 = 1)
...
```

The pattern becomes increasingly random-looking as it shifts.

#### Volume and Amplitude

The volume setting (bits 0-3 of 0xAA) scales the output amplitude:

```
Volume 0:  Output = 0 (silence)
Volume 1:  Output = 1/15 * max_amplitude
Volume 2:  Output = 2/15 * max_amplitude
...
Volume 15: Output = 15/15 * max_amplitude (full volume)
```

**Waveform with Different Volumes:**

Same pattern (0xFFFFFF / 0x000000) with different volumes:

```
Volume 15: ████████████        ████████████        (full amplitude)
Volume 10: ████████            ████████            (67% amplitude)
Volume 5:  ████                ████                (33% amplitude)
Volume 1:  █                   █                   (7% amplitude)
Volume 0:  ________________________                (silence)
```

#### Practical Waveform Analysis

**Example 1: Debugging Incorrect Pitch**

**Problem:** Game sound is too high-pitched.

**Trace shows:**
```
VDC_WRITE[0xA7] = 0xFF
VDC_WRITE[0xA8] = 0xFF
VDC_WRITE[0xA9] = 0xFF
VDC_WRITE[0xAA] = 0xEF  (11101111: enabled, loop, high freq, noise, max volume)
```

**Analysis:**
- Pattern: 0xFFFFFF (all bits set)
- Shift rate: 3933Hz (high frequency)
- In loop mode with all bits set, bit 0 is always 1
- This produces a constant high output (DC offset), not a tone!

**Conclusion:** The pattern is incorrect. The game should use a pattern with both 0s and 1s to create a waveform.

**Example 2: Debugging Weak Sound**

**Problem:** Game sound is barely audible.

**Trace shows:**
```
VDC_WRITE[0xA7] = 0xFF
VDC_WRITE[0xA8] = 0x00
VDC_WRITE[0xA9] = 0xFF
VDC_WRITE[0xAA] = 0xC1  (11000001: enabled, loop, high freq, volume 1)
```

**Analysis:**
- Pattern: 111111110000000011111111 (good pattern)
- Shift rate: 3933Hz (high frequency)
- Volume: 1 (very low!)

**Conclusion:** The volume is set too low. The game should use volume 8-15 for audible sound.

**Example 3: Debugging Sound Cutoff**

**Problem:** Game sound starts but quickly fades to silence.

**Trace shows:**
```
VDC_WRITE[0xA7] = 0xFF
VDC_WRITE[0xA8] = 0x00
VDC_WRITE[0xA9] = 0xFF
VDC_WRITE[0xAA] = 0x8F  (10001111: enabled, one-shot, low freq, max volume)
```

**Analysis:**
- Pattern: 111111110000000011111111 (good pattern)
- Shift rate: 983Hz (low frequency)
- Loop mode: DISABLED (bit 6 = 0)
- Volume: 15 (max)

**Conclusion:** One-shot mode is enabled, so the pattern shifts to all zeros after 24 shifts (24ms at 983Hz). The game should enable loop mode (bit 6 = 1) for sustained sound.

#### Visualizing Waveforms

To visualize a waveform from register values:

1. **Convert registers to binary:**
   ```
   0xA7 = 0xFF = 11111111
   0xA8 = 0x00 = 00000000
   0xA9 = 0xFF = 11111111
   ```

2. **Concatenate into 24-bit pattern:**
   ```
   111111110000000011111111
   ```

3. **Draw the waveform (1 = high, 0 = low):**
   ```
   ████████        ████████
   ```

4. **Calculate period and frequency:**
   ```
   Pattern length: 24 bits
   Shift rate: 3933Hz
   Frequency: 3933 / 24 = 163.88Hz
   Period: 1 / 163.88 = 6.1ms
   ```

This visualization helps understand what sound the game is trying to produce and whether it matches expectations.

### 8.5 Common Audio Issues

This section covers common audio problems encountered in Videopac emulation and how to diagnose and fix them.

#### Issue 1: No Sound at All

**Symptoms:**
- Game should produce sound but emulator is silent
- No audio output from any game

**Possible Causes:**

**1. Sound Not Enabled in Register 0xAA**

Check if bit 7 of register 0xAA is set to 1:

```bash
# Capture trace and filter for 0xAA writes
videopac_headless --trace --trace-start-key "1" --trace-frames 5 \
  --trace-filter vdc --trace-output audio_trace.txt bios.bin game.bin
grep "VDC_WRITE\[0xAA\]" audio_trace.txt
```

Look for writes to 0xAA:
```
VDC_WRITE[0xAA] = 0x4F  ; Bit 7 = 0, sound DISABLED
VDC_WRITE[0xAA] = 0xCF  ; Bit 7 = 1, sound ENABLED
```

**Solution:** If bit 7 is 0, the game is not enabling sound. This could be a game bug or an emulator bug in input handling (game doesn't detect key press).

**2. Volume Set to Zero**

Check if bits 0-3 of register 0xAA are set to 0:

```
VDC_WRITE[0xAA] = 0xC0  ; Volume = 0 (bits 0-3 = 0000)
```

**Solution:** Volume 0 produces silence. The game should set volume to 1-15.

**3. No Audio Register Writes**

Check if the game writes to audio registers at all:

```bash
grep "VDC_WRITE\[0xA[7-9A]\]" audio_trace.txt
```

If no results, the game is not writing to audio registers.

**Solution:** This could indicate:
- Game doesn't produce sound in this scenario
- Emulator bug preventing VDC writes
- Input not being detected (game doesn't know to play sound)

**4. Emulator Audio System Not Initialized**

Check emulator logs for audio initialization errors:
- SDL audio device not opened
- Audio callback not registered
- Sample rate mismatch

**Solution:** Fix emulator audio initialization code.

#### Issue 2: Sound Too Quiet

**Symptoms:**
- Sound is present but very faint
- Need to turn up system volume significantly

**Possible Causes:**

**1. Low Volume Setting**

Check bits 0-3 of register 0xAA:

```
VDC_WRITE[0xAA] = 0xC1  ; Volume = 1 (very quiet)
VDC_WRITE[0xAA] = 0xC7  ; Volume = 7 (moderate)
VDC_WRITE[0xAA] = 0xCF  ; Volume = 15 (maximum)
```

**Solution:** If volume is 1-5, the game may be using low volume intentionally, or there may be a bug. Compare with real hardware.

**2. Emulator Volume Scaling**

The emulator may be scaling audio output incorrectly:
- Output amplitude too low
- Sample format incorrect (8-bit vs 16-bit)
- Volume multiplier too small

**Solution:** Check emulator audio output code and adjust scaling factor.

#### Issue 3: Sound Too Loud / Distorted

**Symptoms:**
- Sound is distorted or clipping
- Audio has crackling or popping

**Possible Causes:**

**1. Emulator Volume Scaling Too High**

The emulator may be amplifying audio too much:
- Output amplitude too high
- Sample values exceeding range (clipping)

**Solution:** Reduce emulator volume scaling factor.

**2. Audio Buffer Underrun**

The emulator may not be generating audio samples fast enough:
- Audio callback requests more samples than available
- Gaps in audio stream cause popping

**Solution:** Increase audio buffer size or improve audio generation performance.

**3. Incorrect Sample Rate**

The emulator may be using the wrong sample rate:
- Emulator generates at 44100Hz but SDL expects 48000Hz
- Sample rate mismatch causes pitch and quality issues

**Solution:** Ensure emulator and SDL use the same sample rate.

#### Issue 4: Incorrect Pitch

**Symptoms:**
- Sound plays but pitch is wrong (too high or too low)
- Music sounds off-key

**Possible Causes:**

**1. Wrong Shift Frequency**

Check bit 5 of register 0xAA:

```
VDC_WRITE[0xAA] = 0x9F  ; Low frequency (983Hz), bit 5 = 0
VDC_WRITE[0xAA] = 0xDF  ; High frequency (3933Hz), bit 5 = 1
```

**Solution:** If the game uses the wrong frequency setting, the pitch will be off by a factor of 4. Compare with real hardware to determine correct setting.

**2. Incorrect Shift Rate Implementation**

The emulator may be using the wrong shift rates:
- Should be 983Hz for low frequency (bit 5 = 0)
- Should be 3933Hz for high frequency (bit 5 = 1)

**Solution:** Verify emulator shift rate implementation matches hardware specifications.

**3. Incorrect Pattern**

The game may be loading the wrong pattern:

```
; Expected pattern for 440Hz tone at 3933Hz shift rate:
; 3933 / 440 = 8.94 bits per cycle, use 9-bit pattern
0xA7 = 0xFF  ; 8 bits high
0xA8 = 0x01  ; 1 bit high, 7 bits low
0xA9 = 0x00  ; 8 bits low

; Actual pattern (wrong):
0xA7 = 0xFF  ; 8 bits high
0xA8 = 0xFF  ; 8 bits high
0xA9 = 0xFF  ; 8 bits high (all bits high = DC offset, not a tone!)
```

**Solution:** Check if the game is loading the correct pattern for the desired pitch.

#### Issue 5: Sound Cuts Off Too Soon

**Symptoms:**
- Sound starts but stops after a brief moment
- Sustained tones fade to silence

**Possible Causes:**

**1. Loop Mode Not Enabled**

Check bit 6 of register 0xAA:

```
VDC_WRITE[0xAA] = 0x8F  ; One-shot mode (bit 6 = 0)
VDC_WRITE[0xAA] = 0xCF  ; Loop mode (bit 6 = 1)
```

In one-shot mode, the pattern shifts to all zeros after 24 shifts (24ms at 983Hz, 6ms at 3933Hz).

**Solution:** For sustained tones, loop mode must be enabled (bit 6 = 1).

**2. Game Disables Sound Too Early**

Check if the game writes 0x00 to register 0xAA shortly after enabling sound:

```
Frame:0001 ... VDC_WRITE[0xAA] = 0xCF  ; Enable sound
Frame:0002 ... VDC_WRITE[0xAA] = 0x00  ; Disable sound (too soon!)
```

**Solution:** This is a game bug. The game should keep sound enabled longer.

#### Issue 6: Wrong Sound Effect

**Symptoms:**
- Sound plays but doesn't match expected effect
- Explosion sounds like a tone, or vice versa

**Possible Causes:**

**1. Noise Mode Incorrectly Set**

Check bit 4 of register 0xAA:

```
VDC_WRITE[0xAA] = 0xCF  ; Normal mode (bit 4 = 0), for tones
VDC_WRITE[0xAA] = 0xDF  ; Noise mode (bit 4 = 1), for explosions
```

**Solution:** Noise mode should be enabled for noise effects (explosions, crashes) and disabled for tones (music, beeps).

**2. Wrong Pattern Loaded**

The game may be loading a pattern intended for a different sound effect:

```
; Pattern for tone (alternating bits):
0xA7 = 0xAA, 0xA8 = 0xAA, 0xA9 = 0xAA

; Pattern for noise (random-looking):
0xA7 = 0x5C, 0xA8 = 0x3A, 0xA9 = 0x91
```

**Solution:** Check if the game is loading the correct pattern for the desired effect.

#### Issue 7: Audio Timing Issues

**Symptoms:**
- Sound plays at wrong time (too early or too late)
- Sound doesn't sync with visual events

**Possible Causes:**

**1. Audio Register Writes at Wrong Time**

Check when audio registers are written in the trace:

```
Frame:0001 Scanline:100 ... VDC_WRITE[0xAA] = 0xCF  ; Mid-frame write
Frame:0002 Scanline:245 ... VDC_WRITE[0xAA] = 0xCF  ; VBLANK write
```

**Solution:** Audio writes should typically occur during VBLANK (scanlines 241-261) for proper synchronization.

**2. Emulator Audio Latency**

The emulator may have too much audio buffering:
- Large audio buffer causes delay between register write and sound output
- Sound lags behind visual events

**Solution:** Reduce audio buffer size (but not too small, or underruns will occur).

#### Issue 8: Crackling or Popping

**Symptoms:**
- Audio has crackling, popping, or clicking sounds
- Sound quality is poor

**Possible Causes:**

**1. Audio Buffer Underrun**

The emulator is not generating audio samples fast enough:
- Audio callback requests samples but buffer is empty
- Gaps in audio stream cause popping

**Solution:** Increase audio buffer size or improve audio generation performance.

**2. Incorrect Sample Generation**

The emulator may be generating samples incorrectly:
- Abrupt transitions between samples (not smoothed)
- Sample values jumping unexpectedly

**Solution:** Check audio sample generation code for bugs.

**3. Volume Changes Too Abrupt**

The game may be changing volume too quickly:

```
Frame:0001 ... VDC_WRITE[0xAA] = 0xCF  ; Volume 15
Frame:0002 ... VDC_WRITE[0xAA] = 0xC0  ; Volume 0 (abrupt change!)
```

**Solution:** This is expected behavior if the game changes volume abruptly. The emulator should handle this correctly.

#### Debugging Workflow for Audio Issues

1. **Identify the symptom:**
   - No sound, quiet sound, loud sound, wrong pitch, wrong effect, etc.

2. **Capture audio trace:**
   ```bash
   videopac_headless --trace --trace-start-key "1" --trace-frames 10 \
     --trace-filter vdc --trace-output audio_trace.txt bios.bin game.bin
   ```

3. **Filter for audio registers:**
   ```bash
   grep "VDC_WRITE\[0xA[7-9A]\]" audio_trace.txt > audio_only.txt
   ```

4. **Analyze register values:**
   - Check 0xAA for sound enable (bit 7), loop mode (bit 6), frequency (bit 5), noise mode (bit 4), volume (bits 0-3)
   - Check 0xA7-0xA9 for pattern
   - Check timing of writes (frame, scanline)

5. **Compare with expected behavior:**
   - Should sound be enabled?
   - Is the pattern correct for the desired sound?
   - Is the volume appropriate?
   - Is the timing correct?

6. **Test hypothesis:**
   - Modify emulator code to test specific behavior
   - Compare with real hardware if available
   - Check emulator audio system implementation

7. **Fix and verify:**
   - Implement fix
   - Test with multiple games
   - Verify no regressions

### 8.6 Audio Comparison with Real Hardware

Comparing emulator audio output with real Videopac hardware is essential for verifying accuracy. This section covers techniques for capturing and comparing audio.

#### Why Compare with Real Hardware?

Audio emulation can be subtle and complex. Comparing with real hardware helps:
- Verify shift rates are correct (983Hz vs 3933Hz)
- Verify volume scaling is accurate
- Verify noise mode implementation
- Verify timing and synchronization
- Identify emulation inaccuracies

#### Capturing Real Hardware Audio

**Equipment Needed:**
- Real Videopac/Odyssey2 console
- Game cartridge
- Audio cable (RCA or 3.5mm)
- Computer with audio input
- Audio recording software (Audacity, etc.)

**Recording Process:**

1. **Connect hardware to computer:**
   - Use RCA-to-3.5mm cable from console audio out to computer line-in
   - Or use USB audio interface for better quality

2. **Configure recording software:**
   - Sample rate: 44100Hz or 48000Hz
   - Bit depth: 16-bit
   - Mono or stereo (Videopac is mono)

3. **Record gameplay:**
   - Start recording
   - Play the game and trigger the sound you want to analyze
   - Stop recording after capturing the sound

4. **Save recording:**
   - Export as WAV file for lossless quality
   - Name file descriptively (e.g., "racing_game_engine_sound_real.wav")

#### Capturing Emulator Audio

**Method 1: Record Emulator Output**

Use the same recording software to capture emulator audio:

1. **Configure recording software:**
   - Set input to "Stereo Mix" or "What U Hear" (captures system audio)
   - Sample rate: 44100Hz or 48000Hz
   - Bit depth: 16-bit

2. **Record emulator:**
   - Start recording
   - Run emulator and trigger the same sound
   - Stop recording

3. **Save recording:**
   - Export as WAV file
   - Name file descriptively (e.g., "racing_game_engine_sound_emu.wav")

**Method 2: Emulator Built-in Recording**

If the emulator supports audio recording:

```bash
# Run emulator with audio recording enabled
videopac --record-audio output.wav bios.bin game.bin
```

This captures audio directly from the emulator's audio generation code, avoiding any system audio processing.

#### Visual Comparison with Waveform Editor

**Using Audacity:**

1. **Open both recordings:**
   - File → Open → Select real hardware recording
   - File → Open → Select emulator recording

2. **Align waveforms:**
   - Use Time Shift Tool to align the start of the sounds
   - Zoom in to see individual waveforms

3. **Visual inspection:**
   - Compare waveform shapes
   - Check if frequencies match
   - Check if amplitudes match
   - Look for differences in timing

**What to Look For:**

- **Frequency differences:** Waveforms have different periods
- **Amplitude differences:** One waveform is louder than the other
- **Waveform shape differences:** Square wave vs distorted wave
- **Timing differences:** Sound starts at different times

**Example Comparison:**

```
Real Hardware:
████████        ████████        ████████        (clean square wave)

Emulator:
████████        ████████        ████████        (matches!)

Emulator (bug):
████████████████████████████████████████████    (constant high, no waveform!)
```

#### Frequency Analysis with Spectrum View

**Using Audacity Spectrum View:**

1. **Select audio region:**
   - Highlight a portion of the sound (1-2 seconds)

2. **Switch to Spectrum view:**
   - Click waveform dropdown → Spectrogram
   - Or: Analyze → Plot Spectrum

3. **Compare frequencies:**
   - Real hardware shows peak at specific frequency (e.g., 440Hz)
   - Emulator should show peak at same frequency
   - Differences indicate pitch issues

**Example:**

```
Real Hardware Spectrum:
Frequency (Hz)  Amplitude
440             ████████████ (strong peak at 440Hz)
880             ████         (harmonic at 880Hz)
1320            ██           (harmonic at 1320Hz)

Emulator Spectrum:
Frequency (Hz)  Amplitude
440             ████████████ (matches!)
880             ████         (matches!)
1320            ██           (matches!)

Emulator (bug):
Frequency (Hz)  Amplitude
550             ████████████ (wrong frequency! 25% too high)
```

#### Numerical Comparison

**Measuring Frequency:**

1. **Zoom in to see individual cycles:**
   - Zoom in until you can see the waveform clearly
   - Measure the time between two peaks (period)

2. **Calculate frequency:**
   ```
   Frequency = 1 / Period
   ```

   Example:
   - Period = 6.1ms (0.0061 seconds)
   - Frequency = 1 / 0.0061 = 163.93Hz

3. **Compare measurements:**
   - Real hardware: 163.93Hz
   - Emulator: 163.88Hz
   - Difference: 0.05Hz (negligible, within tolerance)

**Measuring Amplitude:**

1. **Select audio region:**
   - Highlight a portion of the sound

2. **Check amplitude:**
   - Analyze → Contrast → Analyze
   - Or: View → Show Clipping

3. **Compare measurements:**
   - Real hardware: Peak amplitude 0.8 (-2dB)
   - Emulator: Peak amplitude 0.4 (-8dB)
   - Difference: Emulator is 6dB quieter (half amplitude)

#### Comparing Noise Effects

Noise effects are harder to compare because they're pseudo-random:

1. **Visual comparison:**
   - Both should look random/noisy
   - Check if density is similar

2. **Spectrum comparison:**
   - Both should have broad spectrum (white noise)
   - Check if frequency distribution is similar

3. **Subjective comparison:**
   - Listen to both recordings
   - Do they sound similar?
   - Is one more "harsh" or "soft"?

**Example:**

```
Real Hardware Noise:
█ ██  █ ███ █  ██ █ ███  █ ██  (random pattern)

Emulator Noise:
█ ██  █ ███ █  ██ █ ███  █ ██  (similar random pattern)

Emulator (bug):
████████████████████████████    (constant high, not random!)
```

#### Comparing Volume Levels

**Test Procedure:**

1. **Record same sound at different volumes:**
   - Real hardware: Volume 15, 10, 5, 1
   - Emulator: Volume 15, 10, 5, 1

2. **Measure peak amplitudes:**
   - Use Audacity or similar tool
   - Record peak amplitude for each volume level

3. **Compare scaling:**
   - Real hardware: Linear scaling (volume 10 = 67% of volume 15)
   - Emulator: Should match real hardware scaling

**Example:**

| Volume | Real Hardware | Emulator | Match? |
|--------|---------------|----------|--------|
| 15 | 0.8 | 0.8 | ✓ |
| 10 | 0.53 | 0.53 | ✓ |
| 5 | 0.27 | 0.27 | ✓ |
| 1 | 0.05 | 0.05 | ✓ |

#### Comparing Shift Rates

**Test Procedure:**

1. **Create test ROM:**
   - Load pattern 0xFFFFFF / 0x000000 (12 bits high, 12 bits low)
   - Test with low frequency (bit 5 = 0)
   - Test with high frequency (bit 5 = 1)

2. **Record both:**
   - Real hardware at low frequency
   - Emulator at low frequency
   - Real hardware at high frequency
   - Emulator at high frequency

3. **Measure frequencies:**
   - Low frequency should be ~41Hz (983Hz / 24)
   - High frequency should be ~164Hz (3933Hz / 24)

4. **Compare:**
   - Real hardware low: 40.96Hz
   - Emulator low: 40.96Hz ✓
   - Real hardware high: 163.88Hz
   - Emulator high: 163.88Hz ✓

#### Known Differences

Some differences between emulator and real hardware are expected:

**1. Audio Quality:**
- Real hardware may have analog noise, hum, or distortion
- Emulator produces clean digital audio
- This is acceptable and often preferred

**2. Volume Levels:**
- Real hardware volume depends on console condition, cables, etc.
- Emulator volume is consistent
- Relative volumes (volume 15 vs volume 5) should match, not absolute levels

**3. Frequency Precision:**
- Real hardware may have slight frequency drift due to analog components
- Emulator has perfect digital frequency
- Small differences (<1%) are acceptable

**4. Latency:**
- Real hardware has near-zero latency
- Emulator has some latency due to audio buffering
- This affects timing but not the sound itself

#### Documenting Comparisons

When comparing audio, document your findings:

```markdown
## Audio Comparison: Racing Game Engine Sound

### Setup
- Real Hardware: Videopac G7000, recorded via RCA-to-USB
- Emulator: Version 1.2.3, SDL audio backend
- Recording: Audacity 3.0, 44100Hz, 16-bit

### Findings
- Frequency: Real 163.93Hz, Emulator 163.88Hz (0.03% difference) ✓
- Amplitude: Real 0.8, Emulator 0.8 (matches) ✓
- Waveform: Both square waves with 50% duty cycle ✓
- Timing: Emulator has 50ms latency (acceptable)

### Conclusion
Emulator audio matches real hardware within acceptable tolerances.
```

This documentation helps track emulation accuracy and identify areas for improvement.

### 8.7 Examples

This section provides complete examples of audio debugging workflows for common scenarios.

#### Example 1: Debugging Silent Game

**Scenario:** A racing game should play engine sound when '1' is pressed to start, but no sound is heard.

**Step 1: Capture Trace**

```bash
# Capture trace starting when '1' is pressed
videopac_headless --trace --trace-start-key "1" --trace-frames 5 \
  --trace-filter vdc --trace-output racing_audio_trace.txt bios.bin racing.bin
```

**Step 2: Filter for Audio Registers**

```bash
# Extract audio register writes
grep "VDC_WRITE\[0xA[7-9A]\]" racing_audio_trace.txt > audio_only.txt
```

**Step 3: Examine Trace**

```
Frame:0001 Scanline:245 Cycle:12345 PC:0x0234 A:0xFF PSW:0x08 | VDC_WRITE[0xA7] = 0xFF
Frame:0001 Scanline:245 Cycle:12347 PC:0x0236 A:0x00 PSW:0x08 | VDC_WRITE[0xA8] = 0x00
Frame:0001 Scanline:245 Cycle:12349 PC:0x0238 A:0xFF PSW:0x08 | VDC_WRITE[0xA9] = 0xFF
Frame:0001 Scanline:245 Cycle:12351 PC:0x023A A:0x4F PSW:0x08 | VDC_WRITE[0xAA] = 0x4F
```

**Step 4: Analyze Register 0xAA**

Value: 0x4F = 01001111 binary

```
Bit 7 (Sound Enable): 0 = DISABLED ← Problem found!
Bit 6 (Loop Mode): 1 = Enabled
Bit 5 (Frequency): 0 = Low (983Hz)
Bit 4 (Noise): 0 = Normal mode
Bits 0-3 (Volume): 15 = Maximum
```

**Step 5: Root Cause**

The game writes a sound pattern (0xFF00FF) and sets volume to maximum, but doesn't enable sound (bit 7 = 0). This is a bug in the game code.

**Step 6: Disassemble to Find Bug**

```bash
# Disassemble ROM to find the audio code
videopac_headless --disassemble-rom racing.bin --output racing_disasm.txt

# Search for the code that writes to 0xAA
grep -A 5 -B 5 "0xAA" racing_disasm.txt
```

**Disassembly excerpt:**

```
0x0234   23 FF     MOV    A, #0xFF      ; Load pattern byte 0
0x0236   90 A7     OUTL   P2, A         ; Write to VDC
0x0238   ...       ...                  ; (more code)
0x023A   23 4F     MOV    A, #0x4F      ; Load control value (BUG: should be 0xCF)
0x023C   90 AA     OUTL   P2, A         ; Write to 0xAA
```

**Step 7: Fix**

The game should load 0xCF (11001111) instead of 0x4F (01001111) to enable sound. This is a game ROM bug that needs to be patched, or an emulator bug if the game works on real hardware.

**Step 8: Verify Fix**

After patching the ROM or fixing the emulator:

```bash
# Capture trace again
videopac_headless --trace --trace-start-key "1" --trace-frames 5 \
  --trace-filter vdc --trace-output racing_audio_trace_fixed.txt bios.bin racing_fixed.bin

# Check register 0xAA value
grep "VDC_WRITE\[0xAA\]" racing_audio_trace_fixed.txt
```

Expected output:
```
Frame:0001 Scanline:245 Cycle:12351 PC:0x023A A:0xCF PSW:0x08 | VDC_WRITE[0xAA] = 0xCF
```

Now bit 7 = 1, sound is enabled! ✓

---

#### Example 2: Debugging Incorrect Pitch

**Scenario:** A game plays a beep sound, but the pitch is too high compared to real hardware.

**Step 1: Capture Trace**

```bash
videopac_headless --trace --trace-start-key "1" --trace-frames 3 \
  --trace-filter vdc --trace-output beep_trace.txt bios.bin game.bin
grep "VDC_WRITE\[0xA[7-9A]\]" beep_trace.txt > beep_audio.txt
```

**Step 2: Examine Trace**

```
Frame:0001 Scanline:245 Cycle:10000 PC:0x0100 A:0xFF PSW:0x08 | VDC_WRITE[0xA7] = 0xFF
Frame:0001 Scanline:245 Cycle:10002 PC:0x0102 A:0x00 PSW:0x08 | VDC_WRITE[0xA8] = 0x00
Frame:0001 Scanline:245 Cycle:10004 PC:0x0104 A:0xFF PSW:0x08 | VDC_WRITE[0xA9] = 0xFF
Frame:0001 Scanline:245 Cycle:10006 PC:0x0106 A:0xEF PSW:0x08 | VDC_WRITE[0xAA] = 0xEF
```

**Step 3: Analyze Registers**

**Pattern (0xA7-0xA9):**
```
0xA9 = 0xFF = 11111111
0xA8 = 0x00 = 00000000
0xA7 = 0xFF = 11111111

24-bit pattern: 111111110000000011111111
Pattern length: 24 bits (16 bits high, 8 bits low)
```

**Control (0xAA):**
```
0xEF = 11101111 binary

Bit 7 (Sound Enable): 1 = Enabled ✓
Bit 6 (Loop Mode): 1 = Enabled ✓
Bit 5 (Frequency): 1 = High (3933Hz) ← Check this
Bit 4 (Noise): 0 = Normal mode ✓
Bits 0-3 (Volume): 15 = Maximum ✓
```

**Step 4: Calculate Frequency**

```
Shift rate: 3933Hz (high frequency)
Pattern length: 24 bits
Audio frequency: 3933 / 24 = 163.88Hz
```

**Step 5: Compare with Real Hardware**

Record real hardware audio and measure frequency:
- Real hardware: 40.96Hz
- Emulator: 163.88Hz
- Difference: 4x too high!

**Step 6: Hypothesis**

The game should be using low frequency (983Hz), not high frequency (3933Hz):
```
Expected shift rate: 983Hz
Expected frequency: 983 / 24 = 40.96Hz ✓
```

**Step 7: Root Cause**

The game is setting bit 5 = 1 (high frequency) when it should be 0 (low frequency). This could be:
1. Game bug (wrong value in ROM)
2. Emulator bug (bit 5 inverted or misinterpreted)

**Step 8: Test Hypothesis**

Check if other games use high frequency correctly:

```bash
# Test another game that uses high frequency
videopac_headless --trace --trace-start-key "1" --trace-frames 3 \
  --trace-filter vdc --trace-output other_game_trace.txt bios.bin other_game.bin
grep "VDC_WRITE\[0xAA\]" other_game_trace.txt
```

If other games work correctly with high frequency, this is a game bug. If all games have pitch issues, it's an emulator bug.

**Step 9: Fix**

If it's an emulator bug, check the shift rate implementation:

```cpp
// Emulator code (example)
int shift_rate = (control_reg & 0x20) ? 3933 : 983;  // Bit 5 selects frequency
```

Verify this matches hardware specifications.

If it's a game bug, patch the ROM to use 0x9F instead of 0xEF for register 0xAA.

---

#### Example 3: Debugging Noise Effect

**Scenario:** A game should play an explosion sound (noise), but it sounds like a tone instead.

**Step 1: Capture Trace**

```bash
videopac_headless --trace --trace-start-key "SPACE" --trace-frames 5 \
  --trace-filter vdc --trace-output explosion_trace.txt bios.bin game.bin
grep "VDC_WRITE\[0xA[7-9A]\]" explosion_trace.txt > explosion_audio.txt
```

**Step 2: Examine Trace**

```
Frame:0001 Scanline:245 Cycle:15000 PC:0x0200 A:0x5C PSW:0x08 | VDC_WRITE[0xA7] = 0x5C
Frame:0001 Scanline:245 Cycle:15002 PC:0x0202 A:0x3A PSW:0x08 | VDC_WRITE[0xA8] = 0x3A
Frame:0001 Scanline:245 Cycle:15004 PC:0x0204 A:0x91 PSW:0x08 | VDC_WRITE[0xA9] = 0x91
Frame:0001 Scanline:245 Cycle:15006 PC:0x0206 A:0xCF PSW:0x08 | VDC_WRITE[0xAA] = 0xCF
```

**Step 3: Analyze Register 0xAA**

```
0xCF = 11001111 binary

Bit 7 (Sound Enable): 1 = Enabled ✓
Bit 6 (Loop Mode): 1 = Enabled ✓
Bit 5 (Frequency): 1 = High (3933Hz) ✓
Bit 4 (Noise): 0 = Normal mode ← Problem found!
Bits 0-3 (Volume): 15 = Maximum ✓
```

**Step 4: Root Cause**

The game loads a random-looking pattern (0x5C3A91) but doesn't enable noise mode (bit 4 = 0). Without noise mode, this pattern just produces a complex tone, not noise.

**Step 5: Expected Value**

For noise effect, register 0xAA should be 0xDF (11011111):
```
Bit 4 (Noise): 1 = Noise mode enabled
```

**Step 6: Disassemble to Find Bug**

```bash
videopac_headless --disassemble-rom game.bin --output game_disasm.txt
grep -A 5 -B 5 "0x0206" game_disasm.txt
```

**Disassembly excerpt:**

```
0x0200   23 5C     MOV    A, #0x5C      ; Load pattern byte 0
0x0202   90 A7     OUTL   P2, A         ; Write to 0xA7
0x0204   23 3A     MOV    A, #0x3A      ; Load pattern byte 1
0x0206   90 A8     OUTL   P2, A         ; Write to 0xA8
0x0208   23 91     MOV    A, #0x91      ; Load pattern byte 2
0x020A   90 A9     OUTL   P2, A         ; Write to 0xA9
0x020C   23 CF     MOV    A, #0xCF      ; Load control (BUG: should be 0xDF)
0x020E   90 AA     OUTL   P2, A         ; Write to 0xAA
```

**Step 7: Fix**

Change 0xCF to 0xDF at address 0x020C in the ROM, or fix the emulator if this works on real hardware.

**Step 8: Verify Fix**

```bash
videopac_headless --trace --trace-start-key "SPACE" --trace-frames 5 \
  --trace-filter vdc --trace-output explosion_trace_fixed.txt bios.bin game_fixed.bin
grep "VDC_WRITE\[0xAA\]" explosion_trace_fixed.txt
```

Expected output:
```
Frame:0001 Scanline:245 Cycle:15006 PC:0x0206 A:0xDF PSW:0x08 | VDC_WRITE[0xAA] = 0xDF
```

Now bit 4 = 1, noise mode is enabled! ✓

---

#### Example 4: Debugging Volume Issue

**Scenario:** A game's sound is barely audible, even with system volume at maximum.

**Step 1: Capture Trace**

```bash
videopac_headless --trace --trace-start-key "1" --trace-frames 3 \
  --trace-filter vdc --trace-output quiet_trace.txt bios.bin game.bin
grep "VDC_WRITE\[0xAA\]" quiet_trace.txt
```

**Step 2: Examine Trace**

```
Frame:0001 Scanline:245 Cycle:10000 PC:0x0100 A:0xC1 PSW:0x08 | VDC_WRITE[0xAA] = 0xC1
```

**Step 3: Analyze Register 0xAA**

```
0xC1 = 11000001 binary

Bit 7 (Sound Enable): 1 = Enabled ✓
Bit 6 (Loop Mode): 1 = Enabled ✓
Bit 5 (Frequency): 0 = Low (983Hz) ✓
Bit 4 (Noise): 0 = Normal mode ✓
Bits 0-3 (Volume): 1 = Very low! ← Problem found!
```

**Step 4: Root Cause**

Volume is set to 1 (minimum audible volume). The game should use volume 8-15 for normal sounds.

**Step 5: Compare with Real Hardware**

Test on real hardware:
- Real hardware: Sound is also very quiet
- Emulator: Matches real hardware

**Conclusion:** This is not a bug! The game intentionally uses low volume for this sound effect. The emulator is correct.

**Alternative:** If real hardware is louder, check emulator volume scaling:

```cpp
// Emulator code (example)
float volume_scale = volume / 15.0f;  // Linear scaling
int16_t sample = (output_bit ? 1 : -1) * volume_scale * 32767;
```

Verify this matches real hardware behavior.

---

#### Example 5: Debugging Sound Cutoff

**Scenario:** A game plays a sustained tone, but it cuts off after a brief moment.

**Step 1: Capture Trace**

```bash
videopac_headless --trace --trace-start-key "1" --trace-frames 10 \
  --trace-filter vdc --trace-output cutoff_trace.txt bios.bin game.bin
grep "VDC_WRITE\[0xA[7-9A]\]" cutoff_trace.txt > cutoff_audio.txt
```

**Step 2: Examine Trace**

```
Frame:0001 Scanline:245 Cycle:10000 PC:0x0100 A:0xFF PSW:0x08 | VDC_WRITE[0xA7] = 0xFF
Frame:0001 Scanline:245 Cycle:10002 PC:0x0102 A:0x00 PSW:0x08 | VDC_WRITE[0xA8] = 0x00
Frame:0001 Scanline:245 Cycle:10004 PC:0x0104 A:0xFF PSW:0x08 | VDC_WRITE[0xA9] = 0xFF
Frame:0001 Scanline:245 Cycle:10006 PC:0x0106 A:0x8F PSW:0x08 | VDC_WRITE[0xAA] = 0x8F
... (no more audio writes in subsequent frames)
```

**Step 3: Analyze Register 0xAA**

```
0x8F = 10001111 binary

Bit 7 (Sound Enable): 1 = Enabled ✓
Bit 6 (Loop Mode): 0 = One-shot mode ← Problem found!
Bit 5 (Frequency): 0 = Low (983Hz) ✓
Bit 4 (Noise): 0 = Normal mode ✓
Bits 0-3 (Volume): 15 = Maximum ✓
```

**Step 4: Root Cause**

Loop mode is disabled (bit 6 = 0), so the pattern shifts to all zeros after 24 shifts:
```
Time to silence: 24 bits / 983Hz = 24.4ms
```

The sound cuts off after ~24ms because the shift register becomes all zeros.

**Step 5: Expected Value**

For sustained tone, register 0xAA should be 0xCF (11001111):
```
Bit 6 (Loop Mode): 1 = Enabled (recirculate bit 0 to bit 23)
```

**Step 6: Fix**

Change 0x8F to 0xCF in the game code or ROM.

**Step 7: Verify Fix**

```bash
videopac_headless --trace --trace-start-key "1" --trace-frames 10 \
  --trace-filter vdc --trace-output cutoff_trace_fixed.txt bios.bin game_fixed.bin
grep "VDC_WRITE\[0xAA\]" cutoff_trace_fixed.txt
```

Expected output:
```
Frame:0001 Scanline:245 Cycle:10006 PC:0x0106 A:0xCF PSW:0x08 | VDC_WRITE[0xAA] = 0xCF
```

Now bit 6 = 1, loop mode is enabled, and the sound sustains! ✓

---

These examples demonstrate complete audio debugging workflows from symptom identification to root cause analysis and verification.

[↑ Back to Top](#table-of-contents)

---

## 9. Input Debugging

### 9.1 Videopac Input System Overview

The Videopac/Odyssey2 features two input systems:

**1. Keyboard Matrix (8x8)**
- 48 keys organized in an 8x8 matrix
- Includes alphanumeric keys (0-9, A-Z)
- Includes special keys (+, -, *, /, =, ?, .)
- Includes control keys (YES, NO, CLEAR, ENTER, SPACE)
- Read via Port 2 with row selection

**2. Joystick Ports (2 joysticks)**
- Two analog joystick ports
- Each joystick has 4 directions (Up, Down, Left, Right)
- Each joystick has 1 fire button
- Read via Port 2 with joystick selection

**Input Architecture:**

The input system uses the Intel 8048's Port 2 for both keyboard and joystick input:

```
Port 2 (P2) - Bidirectional I/O Port
├── Output Mode: Row/Joystick Selection
│   ├── P20-P22 (bits 0-2): Select keyboard row (0-7) or joystick (0-1)
│   └── P12 (Port 1 bit 2): Enable keyboard (0) or joystick (1)
│
└── Input Mode: Read Key/Joystick State
    ├── P20-P27 (bits 0-7): Keyboard column data (active low)
    └── P20-P27 (bits 0-7): Joystick direction/button data (active low)
```

**Reading Input:**

1. **Select Input Source** - Write to Port 1 bit 2 (P12):
   - P12 = 0: Enable keyboard reading
   - P12 = 1: Enable joystick reading

2. **Select Row/Joystick** - Write to Port 2 bits 0-2 (P20-P22):
   - For keyboard: 0-7 selects keyboard row
   - For joystick: 0 = joystick 1, 1 = joystick 2

3. **Read Input Data** - Read from Port 2:
   - Returns 8-bit value with input state (active low)
   - Bit = 0: Key/button pressed
   - Bit = 1: Key/button not pressed

### 9.2 Input I/O Ports

**Port 1 (P1) - 8-bit Output Port**

| Bit | Name | Direction | Description |
|-----|------|-----------|-------------|
| P10 | VDC Data 0 | Output | VDC data bus bit 0 |
| P11 | VDC Data 1 | Output | VDC data bus bit 1 |
| P12 | Input Select | Output | 0 = Keyboard, 1 = Joystick |
| P13 | VDC Data 3 | Output | VDC data bus bit 3 |
| P14 | VDC Data 4 | Output | VDC data bus bit 4 |
| P15 | VDC Data 5 | Output | VDC data bus bit 5 |
| P16 | VDC Data 6 | Output | VDC data bus bit 6 |
| P17 | VDC Data 7 | Output | VDC data bus bit 7 |

**Note:** Port 1 is primarily used for VDC communication, but bit P12 controls input source selection.

**Port 2 (P2) - 8-bit Bidirectional Port**

| Bit | Output Mode | Input Mode (Keyboard) | Input Mode (Joystick) |
|-----|-------------|----------------------|----------------------|
| P20 | Row/Joy Select 0 | Column 0 data | Up (Joy 1) / Down (Joy 2) |
| P21 | Row/Joy Select 1 | Column 1 data | Down (Joy 1) / Left (Joy 2) |
| P22 | Row/Joy Select 2 | Column 2 data | Left (Joy 1) / Right (Joy 2) |
| P23 | - | Column 3 data | Right (Joy 1) / Up (Joy 2) |
| P24 | - | Column 4 data | Fire (Joy 1) / Fire (Joy 2) |
| P25 | - | Column 5 data | - |
| P26 | - | Column 6 data | - |
| P27 | - | Column 7 data | - |

**Keyboard Matrix Layout:**

| Row | Col 0 | Col 1 | Col 2 | Col 3 | Col 4 | Col 5 | Col 6 | Col 7 |
|-----|-------|-------|-------|-------|-------|-------|-------|-------|
| 0 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
| 1 | 8 | 9 | - | + | * | / | = | YES |
| 2 | Q | W | E | R | T | Y | U | I |
| 3 | A | S | D | F | G | H | J | K |
| 4 | Z | X | C | V | B | N | M | . |
| 5 | SPACE | ? | L | P | O | CLEAR | ENTER | NO |
| 6-7 | (Unused or special keys) |

**Joystick Bit Mapping:**

**Joystick 1 (P20-P22 = 0b111):**
- P20 (bit 0): Up
- P21 (bit 1): Down
- P22 (bit 2): Left
- P23 (bit 3): Right
- P24 (bit 4): Fire

**Joystick 2 (P20-P22 = 0b000):**
- P20 (bit 0): Down
- P21 (bit 1): Left
- P22 (bit 2): Right
- P23 (bit 3): Up
- P24 (bit 4): Fire

**Note:** All input bits are active low (0 = pressed, 1 = not pressed).

### 9.3 Tracing Input Port Reads

**Capturing Input Traces:**

Use the I/O filter to capture only input port reads:

```bash
# Trace all I/O operations
videopac --trace --trace-filter io --trace-output io_trace.txt bios.bin game.bin

# Trace specific frame with input
videopac --trace --trace-start-key "1" --trace-frames 1 --trace-filter io --trace-output input_trace.txt bios.bin game.bin
```

**Trace Output Format:**

```
Frame:0001 Scanline:010 Cycle:00123 PC:0x0234 | IN A,P2 -> 0xFF (no input)
Frame:0001 Scanline:015 Cycle:00145 PC:0x0240 | IN A,P2 -> 0xFE (bit 0 pressed)
Frame:0001 Scanline:020 Cycle:00167 PC:0x0250 | IN A,P2 -> 0xEF (bit 4 pressed)
```

**Analyzing Input Traces:**

1. **Identify Input Polling Code:**
   - Look for repeated `IN A,P2` instructions
   - Typically in main game loop
   - May be in interrupt handler

2. **Check Row/Joystick Selection:**
   - Look for `OUTL P2,A` before `IN A,P2`
   - Verify correct row/joystick is selected
   - Check P12 bit in Port 1 for keyboard vs joystick

3. **Verify Input Values:**
   - Check returned values from `IN A,P2`
   - 0xFF = no input
   - Bit 0 = key/button pressed (active low)
   - Multiple bits clear = multiple keys pressed

4. **Check Input Processing:**
   - Follow code after `IN A,P2`
   - Look for bit testing (ANL, JB, JNB)
   - Verify game responds to input

**Example Trace Analysis:**

```
; Game polls keyboard row 0 (number keys)
Frame:0001 Scanline:010 PC:0x0200 | MOV A,#0x00      ; Select row 0
Frame:0001 Scanline:010 PC:0x0202 | OUTL P2,A        ; Write to Port 2
Frame:0001 Scanline:010 PC:0x0203 | IN A,P2          ; Read keyboard -> 0xFE
Frame:0001 Scanline:010 PC:0x0204 | ANL A,#0x01      ; Test bit 0 (key '0')
Frame:0001 Scanline:010 PC:0x0206 | JZ 0x0300        ; Jump if pressed

; Analysis: Game is checking if '0' key is pressed
; Bit 0 clear (0xFE) means key '0' is pressed
```

### 9.4 Input Mapping Verification

**Verifying Keyboard Mapping:**

1. **Check Emulator Key Mapping:**
   - Review `frontend_sdl.cpp` for SDL key to Videopac key mapping
   - Verify host keys map to correct Videopac keys
   - Check for missing or incorrect mappings

2. **Test Key Mapping:**
   - Press each key and verify in debugger
   - Open VDC Register Viewer (F12)
   - Check Port 2 input values change correctly

3. **Common Mapping Issues:**
   - Host keyboard layout differences (QWERTY vs AZERTY)
   - Special keys not mapped (YES, NO, CLEAR)
   - Number keys vs numpad keys

**Verifying Joystick Mapping:**

1. **Check Joystick Selection:**
   - Verify P20-P22 bits select correct joystick
   - Joystick 1: P20-P22 = 0b111 (7)
   - Joystick 2: P20-P22 = 0b000 (0)

2. **Check Direction Mapping:**
   - Verify arrow keys map to correct directions
   - Check bit positions match joystick specification
   - Test diagonal inputs (multiple directions)

3. **Check Button Mapping:**
   - Verify fire button (typically Space or Numpad 0)
   - Check bit 4 (P24) for fire button state

**Emulator Key Mapping (Default):**

**Joystick 1 (Numpad):**
- Numpad 8: Up
- Numpad 5: Down
- Numpad 4: Left
- Numpad 6: Right
- Numpad 0: Fire

**Joystick 2 (Arrow Keys):**
- Up Arrow: Up
- Down Arrow: Down
- Left Arrow: Left
- Right Arrow: Right
- Space: Fire

**Keyboard Keys:**
- Number keys 0-9: Videopac keys 0-9
- Letter keys A-Z: Videopac keys A-Z
- Other keys: Mapped to special Videopac keys

### 9.5 Input Responsiveness Testing

**Testing Input Latency:**

1. **Visual Feedback Test:**
   - Press key and observe on-screen response
   - Measure frames between input and visual change
   - Typical: 1-2 frames latency

2. **Trace-Based Test:**
   - Capture trace starting on key press
   - Find first `IN A,P2` that reads the key
   - Calculate cycles/frames from key press to read

3. **Polling Frequency Test:**
   - Count `IN A,P2` instructions per frame
   - Typical: 1-2 polls per frame in main loop
   - More frequent polling = better responsiveness

**Testing Input Reliability:**

1. **Rapid Input Test:**
   - Press keys rapidly in sequence
   - Verify all inputs are registered
   - Check for missed inputs in trace

2. **Simultaneous Input Test:**
   - Press multiple keys simultaneously
   - Verify all keys are read correctly
   - Check keyboard matrix scanning logic

3. **Held Input Test:**
   - Hold key for multiple frames
   - Verify input remains active
   - Check for auto-repeat behavior

**Performance Considerations:**

- **Polling vs Interrupt:** Most games use polling (check input in main loop)
- **Scan Rate:** Games typically scan keyboard once per frame
- **Debouncing:** Some games implement software debouncing
- **Input Buffer:** Some games buffer inputs for processing

### 9.6 Common Input Issues

**Issue 1: Input Not Detected**

**Symptoms:**
- Key press has no effect
- Joystick movement ignored
- Game doesn't respond to any input

**Possible Causes:**
1. **Incorrect Port Selection:**
   - P12 bit not set correctly (keyboard vs joystick)
   - Wrong row/joystick selected in P20-P22

2. **Emulator Mapping Issue:**
   - Host key not mapped to Videopac key
   - Joystick not configured correctly

3. **Game Not Polling Input:**
   - Game stuck in loop not reading input
   - Input polling code not reached

**Debugging Steps:**
1. Capture I/O trace with `--trace-filter io`
2. Verify `IN A,P2` instructions are executed
3. Check Port 2 values returned
4. Verify game processes input values

**Issue 2: Wrong Key Detected**

**Symptoms:**
- Pressing one key triggers different action
- Joystick directions reversed or swapped

**Possible Causes:**
1. **Incorrect Row Selection:**
   - Game selects wrong keyboard row
   - Row bits (P20-P22) incorrect

2. **Bit Position Error:**
   - Game checks wrong bit for key
   - Column mapping incorrect

3. **Emulator Mapping Error:**
   - Host key mapped to wrong Videopac key
   - Joystick direction bits swapped

**Debugging Steps:**
1. Identify which key is detected (trace bit testing)
2. Check keyboard matrix row/column
3. Verify emulator key mapping
4. Compare with expected behavior

**Issue 3: Delayed or Missed Input**

**Symptoms:**
- Input registered after delay
- Some key presses missed
- Inconsistent input response

**Possible Causes:**
1. **Low Polling Frequency:**
   - Game polls input infrequently
   - Input checked only in specific game states

2. **Timing Issue:**
   - Input read at wrong time in frame
   - Race condition between input and processing

3. **Emulator Timing:**
   - Input events not synchronized with emulation
   - Frame timing issues

**Debugging Steps:**
1. Count input polls per frame in trace
2. Check when input is polled (scanline, cycle)
3. Verify input state persists between polls
4. Test with different frame rates

**Issue 4: Stuck Input**

**Symptoms:**
- Input remains active after key release
- Character continues moving after releasing direction
- Key appears "stuck" down

**Possible Causes:**
1. **Input State Not Cleared:**
   - Emulator doesn't clear input on key release
   - Input handler state management issue

2. **Game Logic Issue:**
   - Game doesn't check for key release
   - Input buffer not cleared

3. **Emulator Bug:**
   - Key release event not processed
   - Input state corruption

**Debugging Steps:**
1. Verify key release events in emulator
2. Check input state after key release
3. Trace input reads after release
4. Verify Port 2 returns 0xFF (no input)

**Issue 5: Multiple Keys Conflict**

**Symptoms:**
- Pressing two keys simultaneously causes issues
- Some key combinations don't work
- Unexpected behavior with multiple inputs

**Possible Causes:**
1. **Keyboard Matrix Ghosting:**
   - Hardware limitation of matrix scanning
   - Certain key combinations create false readings

2. **Game Limitation:**
   - Game doesn't support multiple simultaneous keys
   - Input processing logic assumes single key

3. **Emulator Issue:**
   - Multiple key state not handled correctly
   - Matrix scanning simulation incorrect

**Debugging Steps:**
1. Test specific key combinations
2. Check Port 2 values with multiple keys
3. Verify matrix scanning logic
4. Compare with real hardware behavior

### 9.7 Examples

**Example 1: Tracing Keyboard Input**

```bash
# Capture input when pressing '1' key
videopac --trace --trace-start-key "1" --trace-frames 1 --trace-filter io --trace-output key1_trace.txt bios.bin game.bin
```

**Expected Trace Output:**
```
Frame:0001 Scanline:010 PC:0x0200 | MOV A,#0x00      ; Select row 0
Frame:0001 Scanline:010 PC:0x0202 | OUTL P2,A        ; Write to Port 2
Frame:0001 Scanline:010 PC:0x0203 | IN A,P2          ; Read -> 0xFD (bit 1 clear)
Frame:0001 Scanline:010 PC:0x0204 | ANL A,#0x02      ; Test bit 1 (key '1')
Frame:0001 Scanline:010 PC:0x0206 | JZ 0x0300        ; Jump if pressed
```

**Analysis:**
- Game selects keyboard row 0 (number keys)
- Reads Port 2 and gets 0xFD (bit 1 clear = key '1' pressed)
- Tests bit 1 and jumps to handler at 0x0300

**Example 2: Tracing Joystick Input**

```bash
# Capture input when pressing arrow keys
videopac --trace --trace-start-key "UP" --trace-frames 1 --trace-filter io --trace-output joy_trace.txt bios.bin game.bin
```

**Expected Trace Output:**
```
Frame:0001 Scanline:015 PC:0x0210 | MOV A,#0x00      ; Select joystick 2
Frame:0001 Scanline:015 PC:0x0212 | OUTL P2,A        ; Write to Port 2
Frame:0001 Scanline:015 PC:0x0213 | IN A,P2          ; Read -> 0xF7 (bit 3 clear)
Frame:0001 Scanline:015 PC:0x0214 | ANL A,#0x08      ; Test bit 3 (Up for Joy 2)
Frame:0001 Scanline:015 PC:0x0216 | JZ 0x0400        ; Jump if pressed
```

**Analysis:**
- Game selects joystick 2 (P20-P22 = 0b000)
- Reads Port 2 and gets 0xF7 (bit 3 clear = Up pressed)
- Tests bit 3 and jumps to movement handler

**Example 3: Debugging Missing Input**

**Problem:** Game doesn't respond to '5' key press

**Steps:**

1. **Capture trace when pressing '5':**
```bash
videopac --trace --trace-start-key "5" --trace-frames 1 --trace-filter io --trace-output key5_trace.txt bios.bin game.bin
```

2. **Analyze trace:**
```
Frame:0001 Scanline:010 PC:0x0200 | MOV A,#0x00      ; Select row 0
Frame:0001 Scanline:010 PC:0x0202 | OUTL P2,A        ; Write to Port 2
Frame:0001 Scanline:010 PC:0x0203 | IN A,P2          ; Read -> 0xDF (bit 5 clear)
```

3. **Check game logic:**
```assembly
0x0200   MOV A,#0x00      ; Select row 0
0x0202   OUTL P2,A        ; Write to Port 2
0x0203   IN A,P2          ; Read keyboard
0x0204   ANL A,#0x1F      ; Mask bits 0-4 only (keys 0-4)
0x0206   JZ 0x0300        ; Jump if any pressed
```

4. **Root Cause:**
   - Game only checks bits 0-4 (keys 0-4)
   - Key '5' is bit 5, which is masked out
   - Game doesn't support key '5'

**Solution:** This is game behavior, not an emulator bug. Document that this game only supports keys 0-4.

**Example 4: Verifying Input Mapping**

**Test all number keys:**

```bash
# Create test script
for key in 0 1 2 3 4 5 6 7 8 9; do
    echo "Testing key $key"
    videopac --trace --trace-start-key "$key" --trace-frames 1 --trace-filter io --trace-output "key${key}_trace.txt" bios.bin game.bin
done
```

**Verify each trace:**
- Key '0': Bit 0 clear in row 0 (0xFE)
- Key '1': Bit 1 clear in row 0 (0xFD)
- Key '2': Bit 2 clear in row 0 (0xFB)
- Key '3': Bit 3 clear in row 0 (0xF7)
- Key '4': Bit 4 clear in row 0 (0xEF)
- Key '5': Bit 5 clear in row 0 (0xDF)
- Key '6': Bit 6 clear in row 0 (0xBF)
- Key '7': Bit 7 clear in row 0 (0x7F)
- Key '8': Bit 0 clear in row 1 (0xFE)
- Key '9': Bit 1 clear in row 1 (0xFD)

**Example 5: Testing Input Responsiveness**

**Measure input latency:**

```bash
# Capture trace starting on key press
videopac --trace --trace-start-key "1" --trace-frames 2 --trace-output latency_trace.txt bios.bin game.bin
```

**Analyze trace:**
```
Frame:0001 Scanline:000 Cycle:00000 | [Key '1' pressed by user]
Frame:0001 Scanline:010 Cycle:00500 | IN A,P2 -> 0xFD (key detected)
Frame:0001 Scanline:015 Cycle:00600 | CALL 0x0300 (input handler)
Frame:0001 Scanline:192 Cycle:10000 | [Frame ends]
Frame:0002 Scanline:010 Cycle:00500 | [Visual change appears]
```

**Latency Calculation:**
- Input detected: 500 cycles into frame 1
- Visual change: Frame 2
- Total latency: ~1.5 frames (25ms at 60 FPS)

This is typical and acceptable latency for Videopac games.

[↑ Back to Top](#table-of-contents)

---

## 10. Comparative Analysis

Comparative analysis is the process of comparing emulator behavior with real Videopac/Odyssey2 hardware to identify emulation inaccuracies. This chapter covers techniques for capturing reference data from hardware, comparing it with emulator output, and identifying discrepancies.

### 10.1 Capturing Reference Traces

When real hardware is available, capturing reference traces provides ground truth for emulator validation.

**Hardware Requirements:**
- Videopac/Odyssey2 console
- ROM cartridge
- Capture equipment (logic analyzer, oscilloscope, or custom hardware)
- Video/audio recording equipment

**Trace Capture Methods:**

**1. Logic Analyzer Capture:**
```
Equipment: Logic analyzer with 16+ channels
Connections:
- CPU address bus (12 bits)
- CPU data bus (8 bits)
- CPU control signals (RD, WR, ALE)
- VDC chip select
- Clock signal

Capture Settings:
- Sample rate: 10-20 MHz (2-4x CPU clock)
- Trigger: Frame sync or specific address
- Duration: 1-10 frames
```

**2. Custom Hardware Capture:**
- FPGA-based capture board
- Arduino/Raspberry Pi with fast GPIO
- Intercept bus signals without disrupting operation
- Log to SD card or stream to PC

**3. Emulator-Based Reference:**
- If hardware unavailable, use known-accurate emulator
- MAME/MESS Videopac driver
- Other validated emulators
- Document emulator version and settings

**Trace Data to Capture:**
- CPU instruction execution (PC, opcode, operands)
- Register values (A, PSW, R0-R7)
- Memory accesses (address, data, read/write)
- VDC register writes (address, data, timing)
- I/O port accesses
- Frame timing (scanline, cycle count)

**Trace Format:**
Save traces in same format as emulator traces for easy comparison:
```
Frame:0001 Scanline:010 Cycle:00123 PC:0x0234 A:0x42 PSW:0x08 | MOV A, #0x42
Frame:0001 Scanline:010 Cycle:00125 PC:0x0235 A:0x42 PSW:0x08 | OUTL P1, A
Frame:0001 Scanline:010 Cycle:00127 PC:0x0236 A:0x42 PSW:0x08 | VDC_WRITE[0xA3] = 0x42
```

**Practical Considerations:**
- Hardware capture is complex and expensive
- Focus on specific scenarios (first frame, key press response)
- Capture multiple runs to verify consistency
- Document hardware revision and modifications
- Store reference traces for regression testing

### 10.2 Comparing Emulator vs Reference Traces

Once reference traces are captured, systematic comparison reveals emulation inaccuracies.

**Comparison Workflow:**

**1. Align Traces:**
- Ensure both traces start at same point (frame 0, key press, etc.)
- Verify frame numbering matches
- Check scanline and cycle alignment
- Account for timing differences

**2. Compare Instruction Execution:**
```bash
# Use diff tool to compare traces
diff -u reference_trace.txt emulator_trace.txt > differences.txt

# Or use specialized comparison script
python compare_traces.py reference_trace.txt emulator_trace.txt
```

**3. Identify Divergence Point:**
- Find first instruction where traces differ
- Check PC values - do they diverge?
- Check register values - when do they differ?
- Check timing - are cycle counts different?

**4. Analyze Divergence:**
- Is it a timing difference (same instructions, different timing)?
- Is it a logic difference (different instructions executed)?
- Is it a register difference (same instructions, different results)?
- Is it a VDC difference (different register writes)?

**Comparison Tools:**

**Diff Tools:**
```bash
# Line-by-line comparison
diff -u reference.txt emulator.txt

# Side-by-side comparison
diff -y reference.txt emulator.txt

# Ignore whitespace differences
diff -w reference.txt emulator.txt
```

**Custom Scripts:**
```python
# Example Python script for trace comparison
def compare_traces(ref_file, emu_file):
    with open(ref_file) as ref, open(emu_file) as emu:
        for line_num, (ref_line, emu_line) in enumerate(zip(ref, emu), 1):
            if ref_line != emu_line:
                print(f"Line {line_num} differs:")
                print(f"  REF: {ref_line.strip()}")
                print(f"  EMU: {emu_line.strip()}")
                # Analyze specific differences
                analyze_difference(ref_line, emu_line)
```

**Statistical Comparison:**
- Count total instructions executed
- Compare instruction mix (MOV, ADD, JMP percentages)
- Compare VDC write frequency
- Compare memory access patterns
- Identify systematic vs random differences

**Focused Comparison:**
Instead of comparing entire traces, focus on specific aspects:
- VDC register writes only
- Memory writes only
- Specific address ranges
- Specific time windows (VBLANK, first frame, etc.)

### 10.3 Identifying Discrepancies

Discrepancies between emulator and hardware fall into several categories.

**Types of Discrepancies:**

**1. Timing Discrepancies:**
- Instruction timing incorrect (wrong cycle counts)
- VDC timing incorrect (scanline/frame timing)
- Interrupt timing incorrect
- I/O timing incorrect

**Symptoms:**
- Same instructions, different cycle counts
- VDC writes at different scanlines
- Frame rate differences
- Audio pitch differences

**Example:**
```
REF: Frame:0001 Scanline:010 Cycle:00123 PC:0x0234 | MOV A, #0x42
EMU: Frame:0001 Scanline:010 Cycle:00125 PC:0x0234 | MOV A, #0x42
                                    ^^^^^ 2 cycles off
```

**2. Logic Discrepancies:**
- Instruction behavior incorrect
- Flag calculation incorrect
- Addressing mode incorrect
- Conditional branch incorrect

**Symptoms:**
- Different instructions executed
- Different code paths taken
- Different register values
- Different memory contents

**Example:**
```
REF: PC:0x0234 A:0x42 PSW:0x08 | ADD A, #0x10  -> A:0x52 PSW:0x00
EMU: PC:0x0234 A:0x42 PSW:0x08 | ADD A, #0x10  -> A:0x52 PSW:0x80
                                                              ^^^^ Carry flag wrong
```

**3. VDC Discrepancies:**
- Register behavior incorrect
- Sprite rendering incorrect
- Collision detection incorrect
- Color palette incorrect

**Symptoms:**
- Different VDC register values
- Different rendering output
- Different collision results
- Different visual appearance

**Example:**
```
REF: VDC_WRITE[0xA3] = 0x42 (Sprite 0 Color = Red)
EMU: VDC_WRITE[0xA3] = 0x42 (Sprite 0 Color = Blue)
                                                ^^^^ Color mapping wrong
```

**4. Memory Discrepancies:**
- RAM behavior incorrect
- ROM mapping incorrect
- Memory mirroring incorrect
- Uninitialized memory different

**Symptoms:**
- Different memory contents
- Different data read from memory
- Crashes or unexpected behavior
- Different game state

**Discrepancy Analysis Workflow:**

**Step 1: Categorize the Discrepancy**
- Is it timing, logic, VDC, or memory?
- Is it systematic or random?
- Is it consistent or intermittent?

**Step 2: Isolate the Root Cause**
- Find the earliest point where traces diverge
- Identify the specific instruction or operation
- Check emulator implementation of that operation
- Verify against hardware documentation

**Step 3: Verify the Discrepancy**
- Test with multiple ROMs
- Test with different scenarios
- Verify reference trace is correct
- Check for hardware variations

**Step 4: Assess Impact**
- Does it affect gameplay?
- Does it affect multiple games?
- Is it a critical or cosmetic issue?
- What is the priority for fixing?

**Common False Positives:**

**Uninitialized Memory:**
- RAM contents at startup are random
- Different values are normal
- Focus on initialized data only

**Timing Variations:**
- Real hardware has timing variations
- Small differences (1-2 cycles) may be acceptable
- Focus on large systematic differences

**Hardware Revisions:**
- Different Videopac revisions exist
- BIOS versions differ
- VDC chip revisions differ
- Document which revision is reference

### 10.4 Visual Comparison Techniques

Visual comparison is often the most practical method for validating emulator accuracy.

**Screenshot Comparison:**

**Capturing Screenshots:**
```bash
# Emulator screenshot
videopac --screenshot frame_100.png --screenshot-frame 100 bios.bin game.bin

# Hardware screenshot
# Use camera or capture card to photograph/record TV output
```

**Comparison Methods:**

**1. Side-by-Side Visual Inspection:**
- Display screenshots side by side
- Look for obvious differences
- Check sprite positions, colors, patterns
- Check character rendering
- Check grid rendering

**2. Difference Image:**
```bash
# Using ImageMagick
compare reference.png emulator.png difference.png

# Using Python PIL
from PIL import Image, ImageChops
ref = Image.open('reference.png')
emu = Image.open('emulator.png')
diff = ImageChops.difference(ref, emu)
diff.save('difference.png')
```

**3. Pixel-by-Pixel Comparison:**
```python
# Count different pixels
def compare_images(ref_path, emu_path):
    ref = Image.open(ref_path)
    emu = Image.open(emu_path)
    
    if ref.size != emu.size:
        print("Image sizes differ!")
        return
    
    diff_pixels = 0
    total_pixels = ref.size[0] * ref.size[1]
    
    for x in range(ref.size[0]):
        for y in range(ref.size[1]):
            if ref.getpixel((x, y)) != emu.getpixel((x, y)):
                diff_pixels += 1
    
    accuracy = 100 * (1 - diff_pixels / total_pixels)
    print(f"Accuracy: {accuracy:.2f}%")
    print(f"Different pixels: {diff_pixels}/{total_pixels}")
```

**Video Comparison:**

**Capturing Video:**
```bash
# Emulator video recording
videopac --record video.mp4 --record-frames 600 bios.bin game.bin

# Hardware video recording
# Use capture card or camera to record TV output
```

**Comparison Methods:**

**1. Frame-by-Frame Comparison:**
- Extract frames from both videos
- Compare each frame as screenshot
- Identify frames with differences
- Analyze timing of differences

**2. Motion Analysis:**
- Compare sprite movement
- Check animation timing
- Verify scrolling behavior
- Check for stuttering or tearing

**3. Visual Regression Testing:**
- Capture reference video once
- Compare emulator output against reference
- Automate comparison for regression testing
- Flag frames with significant differences

**Practical Considerations:**

**Color Accuracy:**
- CRT displays have different color characteristics
- Capture cards may alter colors
- Emulator color output may differ from hardware
- Use color calibration if possible

**Resolution and Scaling:**
- Videopac output: 320x240 (approximately)
- Modern displays scale differently
- Use integer scaling for accuracy
- Avoid filtering/smoothing for comparison

**Timing Synchronization:**
- Ensure both start at same point
- Account for frame rate differences
- Use frame numbers for alignment
- Consider NTSC vs PAL differences

**Screenshot Locations:**
- First frame after boot
- First frame after key press
- Specific game states (menu, gameplay, game over)
- Known problematic scenes

### 10.5 Audio Comparison Techniques

Audio comparison validates sound generation accuracy.

**Audio Capture:**

**Emulator Audio:**
```bash
# Record audio to WAV file
videopac --record-audio audio.wav --record-time 10 bios.bin game.bin
```

**Hardware Audio:**
- Connect audio output to recording device
- Use audio interface or capture card
- Record to WAV format (uncompressed)
- Use same sample rate as emulator (44.1kHz or 48kHz)

**Comparison Methods:**

**1. Waveform Comparison:**
```bash
# Using Audacity or similar audio editor
# Load both files
# Visual comparison of waveforms
# Check amplitude, frequency, timing
```

**2. Spectral Analysis:**
```python
# Using Python with scipy
import numpy as np
from scipy.io import wavfile
from scipy.fft import fft
import matplotlib.pyplot as plt

def compare_audio_spectrum(ref_file, emu_file):
    # Load audio files
    ref_rate, ref_data = wavfile.read(ref_file)
    emu_rate, emu_data = wavfile.read(emu_file)
    
    # Compute FFT
    ref_fft = np.abs(fft(ref_data))
    emu_fft = np.abs(fft(emu_data))
    
    # Plot comparison
    plt.figure(figsize=(12, 6))
    plt.subplot(2, 1, 1)
    plt.plot(ref_fft[:len(ref_fft)//2])
    plt.title('Reference Hardware Spectrum')
    plt.subplot(2, 1, 2)
    plt.plot(emu_fft[:len(emu_fft)//2])
    plt.title('Emulator Spectrum')
    plt.show()
```

**3. Frequency Analysis:**
- Identify dominant frequencies
- Compare pitch accuracy
- Check for harmonics
- Verify frequency ratios

**4. Timing Analysis:**
- Compare sound effect timing
- Check note duration
- Verify silence periods
- Check for audio glitches

**Audio Discrepancy Types:**

**Pitch Differences:**
- Frequency too high or too low
- Indicates timing or clock rate issue
- Check CPU clock rate
- Check audio sample rate

**Waveform Differences:**
- Different sound character
- Indicates waveform generation issue
- Check sound shift register implementation
- Verify audio register behavior

**Timing Differences:**
- Sound starts/stops at wrong time
- Indicates synchronization issue
- Check VDC audio register timing
- Verify frame timing

**Volume Differences:**
- Too loud or too quiet
- Indicates amplitude scaling issue
- Check audio output level
- Verify mixer settings

**Practical Considerations:**

**Recording Quality:**
- Use high-quality recording equipment
- Minimize background noise
- Use consistent recording levels
- Avoid clipping or distortion

**Synchronization:**
- Start recording at same point
- Use visual cue for alignment
- Account for recording latency
- Trim silence from beginning/end

**Hardware Variations:**
- Different consoles sound slightly different
- Component aging affects audio
- Modifications affect output
- Document hardware condition

### 10.6 Known Emulator vs Hardware Differences

Some differences between emulator and hardware are known and documented.

**Timing Differences:**

**CPU Clock Rate:**
- Hardware: ~1.79 MHz (NTSC) or ~1.75 MHz (PAL)
- Emulator: May use slightly different rate
- Impact: Audio pitch, game speed
- Mitigation: Configurable clock rate

**Frame Rate:**
- Hardware: 60 Hz (NTSC) or 50 Hz (PAL)
- Emulator: May sync to monitor refresh rate
- Impact: Game speed, audio timing
- Mitigation: Frame rate limiting

**VDC Timing:**
- Hardware: Precise scanline timing
- Emulator: May approximate timing
- Impact: Mid-frame effects, collision detection
- Mitigation: Cycle-accurate VDC emulation

**Graphics Differences:**

**Color Palette:**
- Hardware: CRT color characteristics
- Emulator: RGB color output
- Impact: Colors appear different
- Mitigation: Color calibration, CRT shader

**Sprite Rendering:**
- Hardware: Analog sprite generation
- Emulator: Digital sprite rendering
- Impact: Sprite edges, transparency
- Mitigation: Accurate sprite implementation

**Collision Detection:**
- Hardware: Hardware collision detection
- Emulator: Software collision detection
- Impact: Collision timing, accuracy
- Mitigation: Cycle-accurate collision

**Audio Differences:**

**Sound Generation:**
- Hardware: Analog sound generation
- Emulator: Digital sound synthesis
- Impact: Sound character, harmonics
- Mitigation: Accurate waveform generation

**Audio Filtering:**
- Hardware: TV speaker filtering
- Emulator: Direct digital output
- Impact: Sound brightness, bass response
- Mitigation: Audio filtering, EQ

**Input Differences:**

**Input Latency:**
- Hardware: Direct controller connection
- Emulator: OS input latency, USB polling
- Impact: Input responsiveness
- Mitigation: Low-latency input handling

**Controller Mapping:**
- Hardware: Original controllers
- Emulator: Keyboard/gamepad mapping
- Impact: Control feel, accuracy
- Mitigation: Configurable input mapping

**Acceptable Differences:**

Some differences are acceptable and don't affect gameplay:
- Minor timing variations (< 1%)
- Color differences due to display technology
- Audio filtering differences
- Input latency within reasonable bounds (< 16ms)

**Critical Differences:**

Some differences significantly affect gameplay:
- Incorrect instruction behavior
- Wrong VDC register behavior
- Collision detection errors
- Timing errors causing game logic issues

**Documentation:**

Maintain a list of known differences:
- Document each difference
- Classify as acceptable or critical
- Track fixes and improvements
- Update as emulator evolves

### 10.7 Examples

**Example 1: Comparing First Frame Traces**

**Scenario:** Verify emulator executes same instructions as hardware for first frame.

**Steps:**
1. Capture reference trace from hardware (first frame)
2. Capture emulator trace (first frame)
3. Compare traces line by line

**Reference Trace (first 10 lines):**
```
Frame:0000 Scanline:000 Cycle:00000 PC:0x0000 A:0x00 PSW:0x00 | JMP 0x0010
Frame:0000 Scanline:000 Cycle:00002 PC:0x0010 A:0x00 PSW:0x00 | CLR A
Frame:0000 Scanline:000 Cycle:00003 PC:0x0011 A:0x00 PSW:0x00 | MOV R0, #0x00
Frame:0000 Scanline:000 Cycle:00005 PC:0x0013 A:0x00 PSW:0x00 | MOV R7, #0xFF
Frame:0000 Scanline:000 Cycle:00007 PC:0x0015 A:0x00 PSW:0x00 | CALL 0x0100
```

**Emulator Trace (first 10 lines):**
```
Frame:0000 Scanline:000 Cycle:00000 PC:0x0000 A:0x00 PSW:0x00 | JMP 0x0010
Frame:0000 Scanline:000 Cycle:00002 PC:0x0010 A:0x00 PSW:0x00 | CLR A
Frame:0000 Scanline:000 Cycle:00003 PC:0x0011 A:0x00 PSW:0x00 | MOV R0, #0x00
Frame:0000 Scanline:000 Cycle:00005 PC:0x0013 A:0x00 PSW:0x00 | MOV R7, #0xFF
Frame:0000 Scanline:000 Cycle:00007 PC:0x0015 A:0x00 PSW:0x00 | CALL 0x0100
```

**Result:** Traces match perfectly - emulator is accurate for initialization.

**Example 2: Identifying VDC Timing Discrepancy**

**Scenario:** Sprite appears at wrong position - suspect VDC timing issue.

**Reference Trace (VDC writes):**
```
Frame:0001 Scanline:200 Cycle:15234 | VDC_WRITE[0xA0] = 0x50  (Sprite 0 X)
Frame:0001 Scanline:200 Cycle:15240 | VDC_WRITE[0xA1] = 0x30  (Sprite 0 Y)
```

**Emulator Trace (VDC writes):**
```
Frame:0001 Scanline:195 Cycle:14890 | VDC_WRITE[0xA0] = 0x50  (Sprite 0 X)
Frame:0001 Scanline:195 Cycle:14896 | VDC_WRITE[0xA1] = 0x30  (Sprite 0 Y)
```

**Analysis:**
- VDC writes occur at different scanlines (200 vs 195)
- Cycle counts differ by ~344 cycles
- Indicates timing issue in emulator
- Emulator is running ~5 scanlines ahead

**Solution:** Adjust emulator timing to match hardware scanline timing.

**Example 3: Visual Comparison of Sprite Rendering**

**Scenario:** Sprite colors appear incorrect in emulator.

**Reference Screenshot:**
- Sprite 0: Red (color 0x04)
- Sprite 1: White (color 0x07)
- Sprite 2: Green (color 0x02)
- Sprite 3: Blue (color 0x01)

**Emulator Screenshot:**
- Sprite 0: Red (color 0x04) ✓
- Sprite 1: White (color 0x07) ✓
- Sprite 2: Green (color 0x02) ✓
- Sprite 3: Red (color 0x04) ✗

**Analysis:**
- Sprite 3 shows wrong color
- Check VDC register 0xAE (Sprite 3 color)
- Trace shows: VDC_WRITE[0xAE] = 0x01 (Blue)
- But sprite renders as Red (0x04)

**Root Cause:** Emulator incorrectly reads sprite 3 color from wrong register.

**Solution:** Fix sprite 3 color register mapping in VDC implementation.

**Example 4: Audio Frequency Comparison**

**Scenario:** Game sound effects have wrong pitch.

**Reference Audio:**
- Dominant frequency: 440 Hz (A4 note)
- Duration: 0.5 seconds

**Emulator Audio:**
- Dominant frequency: 450 Hz
- Duration: 0.5 seconds

**Analysis:**
- Frequency is 2.3% too high
- Indicates CPU clock rate issue
- Hardware: 1.79 MHz
- Emulator: ~1.83 MHz (estimated)

**Solution:** Adjust emulator CPU clock rate to match hardware.

**Example 5: Regression Testing with Screenshots**

**Scenario:** Verify emulator update doesn't break existing games.

**Process:**
1. Capture reference screenshots from 20 games (before update)
2. Apply emulator update
3. Capture new screenshots from same 20 games
4. Compare each pair of screenshots
5. Flag any differences for investigation

**Results:**
- 18 games: Identical screenshots ✓
- 1 game: Minor color difference (acceptable)
- 1 game: Sprite position changed (investigate)

**Action:** Investigate sprite position change, verify it's not a regression.

[↑ Back to Top](#table-of-contents)

---

## 11. Case Studies

### 11.1 Racing Game Color Bug

*Content to be added in future tasks*

### 11.2 Collision Detection Analysis

*Content to be added in future tasks*

### 11.3 Additional Case Studies

*Content to be added in future tasks*

[↑ Back to Top](#table-of-contents)

---

## 12. Quick Reference

### 12.1 Disassembly Commands

**Basic Commands:**

| Command | Description |
|---------|-------------|
| `--disassemble-bios <file>` | Disassemble BIOS file |
| `--disassemble-rom <file>` | Disassemble ROM file |
| `--output <file>` | Specify output file |
| `--start <addr>` | Start address (hex) |
| `--end <addr>` | End address (hex) |

**Examples:**
```bash
# Disassemble entire BIOS
videopac --disassemble-bios bios.bin --output bios_disasm.txt

# Disassemble ROM
videopac --disassemble-rom game.bin --output game_disasm.txt

# Disassemble address range
videopac --disassemble-rom game.bin --start 0x0000 --end 0x0FFF --output game_partial.txt
```

### 12.2 Trace Commands

**Basic Commands:**

| Command | Description |
|---------|-------------|
| `--trace` | Enable execution tracing |
| `--trace-output <file>` | Specify trace output file |
| `--trace-start-frame <n>` | Start tracing at frame N |
| `--trace-frames <n>` | Trace for N frames |
| `--trace-start-key <key>` | Start tracing on key press |
| `--trace-filter <type>` | Filter trace output |

**Filter Types:**

| Filter | Description |
|--------|-------------|
| `all` | All events (default) |
| `cpu` | CPU instructions only |
| `vdc` | VDC register writes only |
| `mem-write` | Memory writes only |
| `mem-read` | Memory reads only |
| `io` | I/O port accesses only |

**Examples:**
```bash
# Trace first frame after key press
videopac --trace --trace-start-key "1" --trace-frames 1 --trace-output trace.txt bios.bin game.bin

# Trace VDC writes only
videopac --trace --trace-filter vdc --trace-frames 10 --trace-output vdc_trace.txt bios.bin game.bin

# Trace specific frame range
videopac --trace --trace-start-frame 100 --trace-frames 10 --trace-output trace.txt bios.bin game.bin
```

### 12.3 Debugger Shortcuts

**Debugger Control:**

| Key | Action |
|-----|--------|
| F12 | Toggle debugger |
| F10 | Open menu |
| Esc | Close debugger/menu |

**Execution Control:**

| Key | Action |
|-----|--------|
| F5 | Continue execution |
| F9 | Step (single instruction) |
| F8 | Step over (skip CALL) |
| Shift+F8 | Step out (return from subroutine) |
| F2 | Toggle breakpoint at cursor |

**Navigation:**

| Key | Action |
|-----|--------|
| Up/Down | Navigate lists |
| PgUp/PgDn | Scroll memory/disassembly |
| Home/End | Jump to start/end |
| Ctrl+G | Go to address |

### 12.4 VDC Registers

| Address | Name | Description |
|---------|------|-------------|
| 0xA0 | Control | VDC control register |
| 0xA1 | Status | VDC status register (read-only) |
| 0xA2 | Collision | Collision detection register |
| 0xA3 | Color | Background/grid color |
| 0xA4 | Y Position | Vertical grid position |
| 0xA5 | X Position | Horizontal grid position |
| 0xA7 | Sound | Sound shift register byte 0 |
| 0xA8 | Sound | Sound shift register byte 1 |
| 0xA9 | Sound | Sound shift register byte 2 |
| 0xAA | Sound Control | Sound control and sprite 3 color |
| 0xAB | Sprite 0 Color | Sprite 0 color |
| 0xAC | Sprite 1 Color | Sprite 1 color |
| 0xAD | Sprite 2 Color | Sprite 2 color |
| 0xAE | Sprite 3 Color | Sprite 3 color |
| 0xAF | Grid Control | Grid control register |

### 12.5 CPU Registers

| Register | Size | Description |
|----------|------|-------------|
| A | 8-bit | Accumulator |
| PSW | 8-bit | Program Status Word |
| PC | 12-bit | Program Counter |
| SP | 3-bit | Stack Pointer (0-7) |
| R0-R7 | 8-bit | Working registers (bank 0) |
| R0'-R7' | 8-bit | Working registers (bank 1) |

**PSW Flags:**

| Bit | Name | Description |
|-----|------|-------------|
| 7 | CY | Carry flag |
| 6 | AC | Auxiliary carry flag |
| 5 | F0 | User flag 0 |
| 4 | BS | Register bank select (0 or 1) |

### 12.6 Memory Map

| Address Range | Size | Description |
|---------------|------|-------------|
| 0x0000-0x03FF | 1KB | Internal ROM (BIOS) |
| 0x0400-0x0FFF | 3KB | External ROM (Cartridge) |
| 0x1000-0x13FF | 1KB | Internal RAM |
| 0x1400-0x17FF | 1KB | External RAM (if present) |
| 0xA0-0xAF | 16 bytes | VDC registers |

### 12.7 Intel 8048 Instruction Set

**Data Movement Instructions:**

| Opcode | Mnemonic | Operands | Cycles | Description |
|--------|----------|----------|--------|-------------|
| 23-2F | MOV | A, #data | 1 | Move immediate to accumulator |
| F0-F1 | MOV | A, @Rr | 1 | Move indirect RAM to accumulator |
| A0-A7 | MOV | @Rr, A | 1 | Move accumulator to indirect RAM |
| A8-AF | MOV | Rr, A | 1 | Move accumulator to register |
| F8-FF | MOV | A, Rr | 1 | Move register to accumulator |
| B8-BF | MOV | Rr, #data | 1 | Move immediate to register |
| 17 | INC | A | 1 | Increment accumulator |
| 18-1F | INC | Rr | 1 | Increment register |
| 07 | DEC | A | 1 | Decrement accumulator |
| C8-CF | DEC | Rr | 1 | Decrement register |
| 27 | CLR | A | 1 | Clear accumulator |
| 97 | CLR | C | 1 | Clear carry flag |
| 37 | CPL | A | 1 | Complement accumulator |
| A7 | CPL | C | 1 | Complement carry flag |

**Arithmetic Instructions:**

| Opcode | Mnemonic | Operands | Cycles | Description |
|--------|----------|----------|--------|-------------|
| 03 | ADD | A, #data | 1 | Add immediate to accumulator |
| 60-67 | ADD | A, @Rr | 1 | Add indirect RAM to accumulator |
| 68-6F | ADD | A, Rr | 1 | Add register to accumulator |
| 13 | ADDC | A, #data | 1 | Add immediate with carry |
| 70-77 | ADDC | A, @Rr | 1 | Add indirect RAM with carry |
| 78-7F | ADDC | A, Rr | 1 | Add register with carry |

**Logical Instructions:**

| Opcode | Mnemonic | Operands | Cycles | Description |
|--------|----------|----------|--------|-------------|
| 53 | ANL | A, #data | 1 | AND immediate with accumulator |
| 58-5F | ANL | A, Rr | 1 | AND register with accumulator |
| 43 | ORL | A, #data | 1 | OR immediate with accumulator |
| 48-4F | ORL | A, Rr | 1 | OR register with accumulator |
| D3 | XRL | A, #data | 1 | XOR immediate with accumulator |
| D8-DF | XRL | A, Rr | 1 | XOR register with accumulator |

**Control Transfer Instructions:**

| Opcode | Mnemonic | Operands | Cycles | Description |
|--------|----------|----------|--------|-------------|
| 04-E4 | JMP | addr | 2 | Jump to address (page select) |
| 14-F4 | CALL | addr | 2 | Call subroutine (page select) |
| 83 | RET | - | 2 | Return from subroutine |
| 93 | RETR | - | 2 | Return from interrupt |
| B6 | JF0 | addr | 2 | Jump if flag 0 set |
| 76 | JF1 | addr | 2 | Jump if flag 1 set |
| C6 | JZ | addr | 2 | Jump if accumulator zero |
| 96 | JNZ | addr | 2 | Jump if accumulator not zero |
| F6 | JC | addr | 2 | Jump if carry set |
| E6 | JNC | addr | 2 | Jump if carry not set |
| B3 | JMPP | @A | 2 | Jump indirect (PC = page + A) |
| E8-EF | DJNZ | Rr, addr | 2 | Decrement register and jump if not zero |

**I/O Instructions:**

| Opcode | Mnemonic | Operands | Cycles | Description |
|--------|----------|----------|--------|-------------|
| 08-09 | INS | A, BUS | 2 | Input from bus to accumulator |
| 38-3F | OUTL | Pp, A | 2 | Output accumulator to port |
| 80-81 | MOVX | A, @Rr | 2 | Move external RAM to accumulator |
| 90-91 | MOVX | @Rr, A | 2 | Move accumulator to external RAM |

**Stack Instructions:**

| Opcode | Mnemonic | Operands | Cycles | Description |
|--------|----------|----------|--------|-------------|
| 14-F4 | CALL | addr | 2 | Call subroutine (pushes PC) |
| 83 | RET | - | 2 | Return from subroutine (pops PC) |

**Miscellaneous Instructions:**

| Opcode | Mnemonic | Operands | Cycles | Description |
|--------|----------|----------|--------|-------------|
| 00 | NOP | - | 1 | No operation |
| 65 | STOP | TCNT | 1 | Stop timer/counter |
| 45 | STRT | CNT | 1 | Start counter |
| 55 | STRT | T | 1 | Start timer |
| 75 | EN | TCNTI | 1 | Enable timer/counter interrupt |
| 25 | EN | I | 1 | Enable external interrupt |
| 05 | EN | DMA | 1 | Enable DMA |
| 15 | DIS | TCNTI | 1 | Disable timer/counter interrupt |
| 35 | DIS | I | 1 | Disable external interrupt |

**Addressing Modes:**

- **Immediate**: `MOV A, #0x42` - Value specified in instruction
- **Register**: `MOV A, R0` - Value in register R0-R7
- **Indirect**: `MOV A, @R0` - Value at address in R0 or R1
- **Direct**: `JMP 0x0400` - Absolute address
- **Relative**: `DJNZ R0, -5` - Offset from current PC

**Common Instruction Patterns:**

```assembly
; Load immediate value
MOV A, #0x42        ; A = 0x42

; Load from memory
MOV R0, #0x10       ; R0 = 0x10 (address)
MOV A, @R0          ; A = memory[0x10]

; Store to memory
MOV A, #0x55        ; A = 0x55
MOV R0, #0x20       ; R0 = 0x20 (address)
MOV @R0, A          ; memory[0x20] = 0x55

; Loop with counter
MOV R7, #10         ; R7 = 10 (counter)
LOOP:
  ; ... loop body ...
  DJNZ R7, LOOP     ; Decrement R7, jump if not zero

; Conditional jump
MOV A, R5           ; Load value
JZ ZERO_HANDLER     ; Jump if A == 0
; ... non-zero code ...
JMP CONTINUE
ZERO_HANDLER:
; ... zero code ...
CONTINUE:

; Subroutine call
CALL SUBROUTINE     ; Call function
; ... continues here after return ...

SUBROUTINE:
; ... function code ...
RET                 ; Return to caller

; I/O operations
MOV A, #0x42        ; Load value
OUTL P1, A          ; Output to port 1
INS A, BUS          ; Input from bus
```

[↑ Back to Top](#table-of-contents)

---

## 13. Appendices

### 13.1 Intel 8048 Architecture

The Intel 8048 is an 8-bit microcontroller from the MCS-48 family, used as the CPU in the Videopac/Odyssey2 console. Understanding its architecture is essential for debugging emulator behavior and analyzing game code.

#### Architecture Overview

The Intel 8048 is a Harvard architecture microcontroller with separate program and data memory spaces. Key characteristics:

- **8-bit data bus** - All data operations are 8-bit
- **12-bit program counter** - Addresses up to 4KB of program memory
- **64 bytes internal RAM** - Fast on-chip memory for variables and stack
- **1KB internal ROM** - On-chip program memory (BIOS in Videopac)
- **External memory support** - Can address external ROM and RAM
- **Two 8-bit I/O ports** - For interfacing with VDC and input devices
- **8-level hardware stack** - For subroutine calls and interrupts
- **Single interrupt** - External interrupt with vector to 0x0003
- **Timer/Counter** - 8-bit timer with prescaler

**Clock Speed:**
- Videopac uses 5.91 MHz crystal (NTSC) or 5.37 MHz (PAL)
- Internal clock divider: CPU runs at crystal frequency / 15
- NTSC: ~394 kHz instruction cycle
- PAL: ~358 kHz instruction cycle

**Memory Organization:**
- **0x0000-0x03FF** - Internal ROM (1KB) - BIOS
- **0x0400-0x0FFF** - External ROM (3KB) - Game cartridge
- **0x00-0x3F** - Internal RAM (64 bytes)
- **0x40-0xFF** - External RAM (if present)

#### CPU Registers

The Intel 8048 has a minimal register set optimized for embedded control:

**Accumulator (A):**
- 8-bit general-purpose register
- Primary register for arithmetic and logic operations
- Source/destination for most data movement
- Used for I/O operations

**Program Status Word (PSW):**
- 8-bit register containing CPU flags
- See detailed PSW flags section below

**Program Counter (PC):**
- 12-bit register (0x000-0xFFF)
- Points to next instruction to execute
- Automatically incremented after instruction fetch
- Modified by jumps, calls, and returns

**Stack Pointer (SP):**
- 3-bit internal pointer (0-7)
- Points to current stack level
- Automatically managed by CALL/RET instructions
- Stack overflow wraps around (no error detection)

**Working Registers (R0-R7):**
- Eight 8-bit general-purpose registers
- Two banks of 8 registers each (bank 0 and bank 1)
- Bank selected by BS bit in PSW
- R0 and R1 can be used as indirect address pointers
- Located in internal RAM:
  - Bank 0: RAM addresses 0x00-0x07
  - Bank 1: RAM addresses 0x18-0x1F

**Register Usage Conventions:**
- **A** - Primary data register, I/O operations
- **R0, R1** - Indirect addressing pointers
- **R2-R7** - General purpose, loop counters, temporary storage
- **PSW** - Flags for conditional branching

#### Program Status Word (PSW) Flags

The PSW is an 8-bit register at address 0xD0 (special function register):

```
Bit 7   6   5   4   3   2   1   0
    CY  AC  F0  BS  -   -   -   -
```

**Bit 7 - CY (Carry Flag):**
- Set by arithmetic operations that generate carry/borrow
- Used for multi-byte arithmetic
- Tested by JC, JNC instructions
- Set/cleared by CPL C, CLR C instructions
- Affected by: ADD, ADDC, SUBB, RLC, RRC, DA

**Bit 6 - AC (Auxiliary Carry Flag):**
- Set when carry occurs from bit 3 to bit 4
- Used for BCD (Binary Coded Decimal) arithmetic
- Used by DA (Decimal Adjust) instruction
- Not directly testable by conditional jumps

**Bit 5 - F0 (User Flag 0):**
- General-purpose flag bit
- Can be set, cleared, and tested by software
- Tested by JF0, JTF instructions
- Set/cleared by CPL F0, CLR F0 instructions
- Not affected by arithmetic operations

**Bit 4 - BS (Bank Select):**
- Selects working register bank
- 0 = Bank 0 (R0-R7 at RAM 0x00-0x07)
- 1 = Bank 1 (R0-R7 at RAM 0x18-0x1F)
- Set/cleared by SEL RB0, SEL RB1 instructions
- Allows quick context switching

**Bits 3-0 - Unused:**
- Always read as 0
- Writes have no effect

**Flag Usage Examples:**

```assembly
; Carry flag for multi-byte addition
ADD  A, R0      ; Add R0 to A, set CY if overflow
ADDC A, R1      ; Add R1 + carry to A

; User flag for state tracking
CLR  F0         ; Clear user flag
; ... game logic ...
CPL  F0         ; Toggle user flag
JF0  handler    ; Jump if F0 is set

; Bank switching for context save
SEL  RB1        ; Switch to bank 1
MOV  R0, A      ; Save A in bank 1 R0
SEL  RB0        ; Switch back to bank 0
```

#### Addressing Modes

The Intel 8048 supports several addressing modes:

**1. Immediate Addressing:**
- Operand is part of instruction
- Syntax: `MOV A, #0x42`
- Example: `MOV A, #0xFF` - Load 0xFF into A
- 2-byte instruction (opcode + immediate value)

**2. Register Addressing:**
- Operand is in a working register (R0-R7)
- Syntax: `MOV A, Rr` or `ADD A, Rr`
- Example: `MOV A, R3` - Move R3 to A
- 1-byte instruction

**3. Register Indirect Addressing:**
- Address is in R0 or R1
- Syntax: `MOV A, @Rr` (r = 0 or 1)
- Example: `MOV A, @R0` - Load from address in R0
- Used for array/table access
- 1-byte instruction

**4. Direct Addressing:**
- Address is part of instruction (for internal RAM only)
- Limited use in 8048 (mainly for special registers)
- Example: `MOV A, PSW`

**5. Absolute Addressing:**
- Full address specified for jumps/calls
- Syntax: `JMP addr` or `CALL addr`
- 11-bit address (0x000-0x7FF) within current 2KB page
- Example: `JMP 0x0400`
- 2-byte instruction

**6. Relative Addressing:**
- Offset from current PC for short jumps
- Syntax: `DJNZ Rr, offset`
- 8-bit signed offset (-128 to +127)
- Example: `DJNZ R7, loop` - Decrement R7 and jump if not zero

**Addressing Mode Examples:**

```assembly
; Immediate - load constant
MOV  A, #0x42       ; A = 0x42

; Register - use working register
MOV  R5, A          ; R5 = A
ADD  A, R5          ; A = A + R5

; Register indirect - array access
MOV  R0, #0x20      ; R0 = pointer to array
MOV  A, @R0         ; A = memory[0x20]
INC  R0             ; R0++
MOV  A, @R0         ; A = memory[0x21]

; Absolute - jump to address
JMP  0x0400         ; PC = 0x0400
CALL 0x0200         ; Call subroutine at 0x0200
```

#### Instruction Set with Opcodes

The Intel 8048 instruction set includes 96 instructions organized by function:

**Data Movement Instructions:**

| Opcode | Mnemonic | Operands | Description | Cycles |
|--------|----------|----------|-------------|--------|
| 0x23-0x2F | MOV | A, #data | Move immediate to A | 1 |
| 0xF8-0xFF | MOV | A, Rr | Move register to A | 1 |
| 0xA8-0xAF | MOV | Rr, A | Move A to register | 1 |
| 0xB8-0xBF | MOV | Rr, #data | Move immediate to register | 2 |
| 0xF0-0xF1 | MOV | A, @Rr | Move indirect to A | 1 |
| 0xA0-0xA1 | MOV | @Rr, A | Move A to indirect | 1 |
| 0xB0-0xB1 | MOV | @Rr, #data | Move immediate to indirect | 2 |
| 0xC7 | MOV | A, PSW | Move PSW to A | 1 |
| 0xD7 | MOV | PSW, A | Move A to PSW | 1 |

**Arithmetic Instructions:**

| Opcode | Mnemonic | Operands | Description | Cycles |
|--------|----------|----------|-------------|--------|
| 0x03 | ADD | A, #data | Add immediate to A | 1 |
| 0x68-0x6F | ADD | A, Rr | Add register to A | 1 |
| 0x60-0x61 | ADD | A, @Rr | Add indirect to A | 1 |
| 0x13 | ADDC | A, #data | Add immediate with carry | 1 |
| 0x78-0x7F | ADDC | A, Rr | Add register with carry | 1 |
| 0x70-0x71 | ADDC | A, @Rr | Add indirect with carry | 1 |
| 0x17 | INC | A | Increment A | 1 |
| 0x18-0x1F | INC | Rr | Increment register | 1 |
| 0x10-0x11 | INC | @Rr | Increment indirect | 1 |
| 0x07 | DEC | A | Decrement A | 1 |
| 0xC8-0xCF | DEC | Rr | Decrement register | 1 |
| 0x57 | DA | A | Decimal adjust A | 1 |

**Logic Instructions:**

| Opcode | Mnemonic | Operands | Description | Cycles |
|--------|----------|----------|-------------|--------|
| 0x53 | ANL | A, #data | AND immediate with A | 1 |
| 0x58-0x5F | ANL | A, Rr | AND register with A | 1 |
| 0x50-0x51 | ANL | A, @Rr | AND indirect with A | 1 |
| 0x43 | ORL | A, #data | OR immediate with A | 1 |
| 0x48-0x4F | ORL | A, Rr | OR register with A | 1 |
| 0x40-0x41 | ORL | A, @Rr | OR indirect with A | 1 |
| 0xD3 | XRL | A, #data | XOR immediate with A | 1 |
| 0xD8-0xDF | XRL | A, Rr | XOR register with A | 1 |
| 0xD0-0xD1 | XRL | A, @Rr | XOR indirect with A | 1 |
| 0x27 | CLR | A | Clear A (A = 0) | 1 |
| 0x37 | CPL | A | Complement A (A = ~A) | 1 |

**Rotate and Shift Instructions:**

| Opcode | Mnemonic | Operands | Description | Cycles |
|--------|----------|----------|-------------|--------|
| 0xE7 | RL | A | Rotate A left | 1 |
| 0xF7 | RLC | A | Rotate A left through carry | 1 |
| 0x77 | RR | A | Rotate A right | 1 |
| 0x67 | RRC | A | Rotate A right through carry | 1 |
| 0x47 | SWAP | A | Swap nibbles of A | 1 |

**Control Transfer Instructions:**

| Opcode | Mnemonic | Operands | Description | Cycles |
|--------|----------|----------|-------------|--------|
| 0x04-0xE4 | JMP | addr | Jump to address (11-bit) | 2 |
| 0x14-0xF4 | CALL | addr | Call subroutine | 2 |
| 0x83 | RET | - | Return from subroutine | 2 |
| 0x93 | RETR | - | Return from interrupt | 2 |
| 0x12 | JB0 | addr | Jump if bit 0 of A | 2 |
| 0x32 | JB1 | addr | Jump if bit 1 of A | 2 |
| 0x52 | JB2 | addr | Jump if bit 2 of A | 2 |
| 0x72 | JB3 | addr | Jump if bit 3 of A | 2 |
| 0x92 | JB4 | addr | Jump if bit 4 of A | 2 |
| 0xB2 | JB5 | addr | Jump if bit 5 of A | 2 |
| 0xD2 | JB6 | addr | Jump if bit 6 of A | 2 |
| 0xF2 | JB7 | addr | Jump if bit 7 of A | 2 |
| 0xC6 | JZ | addr | Jump if A is zero | 2 |
| 0x96 | JNZ | addr | Jump if A not zero | 2 |
| 0xF6 | JC | addr | Jump if carry set | 2 |
| 0xE6 | JNC | addr | Jump if carry clear | 2 |
| 0xB6 | JF0 | addr | Jump if F0 flag set | 2 |
| 0x76 | JF1 | addr | Jump if F1 flag set | 2 |
| 0x16 | JTF | addr | Jump if timer flag set | 2 |
| 0x36 | JT0 | addr | Jump if T0 pin high | 2 |
| 0x26 | JT1 | addr | Jump if T1 pin high | 2 |
| 0xE8-0xEF | DJNZ | Rr, addr | Decrement and jump if not zero | 2 |

**I/O Instructions:**

| Opcode | Mnemonic | Operands | Description | Cycles |
|--------|----------|----------|-------------|--------|
| 0x08 | INS | A, BUS | Input from bus to A | 2 |
| 0x09-0x0A | IN | A, Pp | Input from port to A | 2 |
| 0x02 | OUTL | BUS, A | Output A to bus | 2 |
| 0x38-0x3A | OUTL | Pp, A | Output A to port | 2 |
| 0x90-0x91 | MOVD | A, Pp | Move data port to A | 2 |
| 0xB0-0xB1 | MOVD | Pp, A | Move A to data port | 2 |

**Flag and Control Instructions:**

| Opcode | Mnemonic | Operands | Description | Cycles |
|--------|----------|----------|-------------|--------|
| 0x97 | CLR | C | Clear carry flag | 1 |
| 0xA7 | CPL | C | Complement carry flag | 1 |
| 0x85 | CLR | F0 | Clear F0 flag | 1 |
| 0x95 | CPL | F0 | Complement F0 flag | 1 |
| 0xC5 | SEL | RB0 | Select register bank 0 | 1 |
| 0xD5 | SEL | RB1 | Select register bank 1 | 1 |
| 0x05 | EN | I | Enable interrupts | 1 |
| 0x15 | DIS | I | Disable interrupts | 1 |
| 0x55 | STRT | CNT | Start counter | 1 |
| 0x45 | STRT | T | Start timer | 1 |
| 0x65 | STOP | TCNT | Stop timer/counter | 1 |
| 0x00 | NOP | - | No operation | 1 |

#### Instruction Timing

All Intel 8048 instructions execute in 1 or 2 machine cycles:

**1-Cycle Instructions (most instructions):**
- Data movement (register, immediate)
- Arithmetic (ADD, INC, DEC)
- Logic (AND, OR, XOR)
- Rotate/shift
- Flag operations
- Execution time: 15 clock cycles (2.5 µs at 5.91 MHz)

**2-Cycle Instructions:**
- Control transfer (JMP, CALL, RET)
- Conditional jumps (JZ, JNZ, JC, etc.)
- I/O operations (IN, OUT, INS, OUTL)
- DJNZ (decrement and jump)
- Execution time: 30 clock cycles (5.0 µs at 5.91 MHz)

**Timing Calculation:**
- Machine cycle = 15 clock cycles
- At 5.91 MHz: 1 cycle = 2.54 µs
- At 5.37 MHz: 1 cycle = 2.79 µs

**Example Timing Analysis:**
```assembly
MOV  A, #0x42    ; 1 cycle = 2.5 µs
ADD  A, R0       ; 1 cycle = 2.5 µs
JNZ  0x0100      ; 2 cycles = 5.0 µs
; Total: 4 cycles = 10.0 µs
```

**Performance Considerations:**
- Videopac frame time: 16.67 ms (NTSC) or 20 ms (PAL)
- Instructions per frame: ~6,560 (NTSC) or ~7,168 (PAL)
- Critical timing for VDC synchronization
- VBLANK period: ~3,000 cycles available

#### Stack and Subroutine Mechanism

The Intel 8048 has an 8-level hardware stack for subroutine calls and interrupts:

**Stack Characteristics:**
- 8 levels deep (can nest 8 CALL instructions)
- Each level stores 12-bit return address
- Hardware-managed (no stack pointer in RAM)
- 3-bit stack pointer (SP) tracks current level
- Stack overflow wraps around (level 8 → level 0)
- No stack underflow detection

**CALL Instruction:**
```assembly
CALL 0x0400     ; Push PC+2 to stack, jump to 0x0400
```
Operation:
1. Increment SP (SP = SP + 1)
2. Push PC + 2 to stack[SP]
3. Set PC = target address
4. Takes 2 machine cycles

**RET Instruction:**
```assembly
RET             ; Pop return address from stack
```
Operation:
1. Pop stack[SP] to PC
2. Decrement SP (SP = SP - 1)
3. Takes 2 machine cycles

**Nested Calls Example:**
```assembly
; Main program
0x0100   CALL 0x0200    ; SP=1, stack[1]=0x0103
0x0103   ; ... continues here after return

; Subroutine 1
0x0200   CALL 0x0300    ; SP=2, stack[2]=0x0203
0x0203   RET            ; SP=1, PC=0x0103

; Subroutine 2
0x0300   ; ... do work
0x0310   RET            ; SP=1, PC=0x0203
```

**Stack Overflow:**
- If 9th CALL is executed, SP wraps to 0
- Overwrites first return address
- No error indication
- Causes unpredictable behavior
- **Debugging tip:** Track call depth in traces

**Stack Usage Guidelines:**
- Maximum nesting: 7 levels (reserve 1 for interrupts)
- Use DJNZ for loops instead of recursive calls
- Avoid deep call chains in interrupt handlers
- Monitor SP in debugger during development

#### Interrupt Handling

The Intel 8048 supports a single external interrupt:

**Interrupt Characteristics:**
- Single external interrupt pin (INT)
- Edge-triggered (falling edge)
- Vector address: 0x0003
- Can be enabled/disabled by software
- Interrupt flag (IF) indicates pending interrupt
- Uses one stack level

**Interrupt Enable/Disable:**
```assembly
EN  I           ; Enable interrupts
DIS I           ; Disable interrupts
```

**Interrupt Sequence:**
1. External device asserts INT pin (falling edge)
2. If interrupts enabled, CPU finishes current instruction
3. Push PC to stack (SP = SP + 1)
4. Set PC = 0x0003 (interrupt vector)
5. Disable interrupts automatically (prevent nesting)
6. Execute interrupt handler

**Interrupt Handler:**
```assembly
; Interrupt vector at 0x0003
0x0003   JMP  isr_handler    ; Jump to actual handler

; Interrupt service routine
isr_handler:
    ; Save context if needed
    MOV  R0, A              ; Save A
    
    ; Handle interrupt
    ; ... interrupt processing ...
    
    ; Restore context
    MOV  A, R0              ; Restore A
    
    ; Return from interrupt
    RETR                    ; Re-enable interrupts and return
```

**RETR Instruction:**
```assembly
RETR            ; Return from interrupt
```
Operation:
1. Pop stack[SP] to PC
2. Decrement SP (SP = SP - 1)
3. Re-enable interrupts
4. Takes 2 machine cycles

**Interrupt Latency:**
- Maximum: 2 machine cycles (5 µs)
- Minimum: 1 machine cycle (2.5 µs)
- Plus time to save context

**Videopac Interrupt Usage:**
- Videopac does not typically use interrupts
- Most games use polling for input and timing
- BIOS may use interrupts for specific functions
- Interrupt vector at 0x0003 in BIOS ROM

**Debugging Interrupts:**
- Check if interrupts are enabled (EN I instruction)
- Set breakpoint at 0x0003 to catch interrupts
- Verify RETR is used (not RET) to return
- Check stack depth (interrupts use one level)

#### External Resources

**Official Intel Documentation:**
- [Intel 8048 Family User's Manual](https://archive.org/details/bitsavers_intelMCS48_) - Complete architecture and instruction set reference
- [Intel MCS-48 Microcomputer User's Manual](http://www.bitsavers.org/components/intel/MCS48/) - Detailed programming guide
- Intel 8048/8049/8050 Data Sheet - Electrical specifications and timing

**Third-Party Resources:**
- [8048 Instruction Set Summary](http://www.pastraiser.com/cpu/i8048/i8048_opcodes.html) - Quick opcode reference
- [MCS-48 Family Architecture](http://www.cpu-world.com/CPUs/8048/) - Architecture overview
- Videopac/Odyssey2 Technical Documentation - System-specific implementation details

**Emulator-Specific Resources:**
- See emulator source code for implementation details
- Check `cpu.cpp` for instruction implementation
- Review `types.h` for register definitions

[↑ Back to Top](#table-of-contents)

### 13.2 Intel 8245 VDC Architecture

*Content to be added in future tasks*

### 13.3 Workflow Templates

*Content to be added in future tasks*

### 13.4 External Resources

**Intel 8048 Documentation:**
- Intel 8048 Family User's Manual
- Intel MCS-48 Microcomputer User's Manual

**Intel 8245 Documentation:**
- Intel 8245 Video Display Controller Datasheet

**Videopac/Odyssey2 Resources:**
- Videopac/Odyssey2 Technical Documentation
- Game ROM archives and documentation

[↑ Back to Top](#table-of-contents)

---

## End of Handbook

This handbook is a living document and will be updated as new debugging techniques are discovered and new features are added to the emulator. Contributions and feedback are welcome.

For the latest version, check the version number and last updated date at the top of this document.
