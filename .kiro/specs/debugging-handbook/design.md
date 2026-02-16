# Design Document: Videopac Debugging Handbook

## Overview

The Videopac Debugging Handbook is a comprehensive documentation project that consolidates debugging knowledge, workflows, and tools for the Videopac/Odyssey2 emulator. The handbook will be implemented as a structured Markdown document with multiple chapters, quick reference cards, case studies, and workflow templates. It serves as both a learning resource for new developers and a reference guide for experienced debuggers.

## Architecture

### Document Structure

```
.kiro/specs/debugging-handbook/
├── requirements.md          # This requirements document
├── design.md               # This design document
├── tasks.md                # Implementation tasks
├── handbook.md             # Main handbook document
├── case-studies/           # Detailed case study documents
│   ├── racing-game-color-bug.md
│   ├── collision-detection-analysis.md
│   └── [other-case-studies].md
├── quick-reference/        # Quick reference cards
│   ├── disassembly-commands.md
│   ├── trace-commands.md
│   ├── debugger-shortcuts.md
│   ├── vdc-registers.md
│   ├── cpu-registers.md
│   ├── memory-map.md
│   └── instruction-set.md
└── workflows/              # Workflow templates
    ├── graphics-debugging.md
    ├── audio-debugging.md
    ├── input-debugging.md
    ├── timing-debugging.md
    └── regression-testing.md
```

### Handbook Chapters

1. **Introduction and Overview**
   - Purpose and scope
   - How to use this handbook
   - Conventions and terminology

2. **Tools Overview**
   - Command-line emulator options
   - SDL vs Headless mode selection
   - In-game debugger features
   - External tools integration

3. **Disassembly Workflow**
   - Disassembling BIOS
   - Disassembling ROMs
   - Reading and annotating disassembly
   - Common code patterns

4. **Trace Analysis**
   - Capturing execution traces
   - Filtering and analyzing traces
   - VDC register traces
   - Memory access traces

5. **Graphics Debugging**
   - Sprite debugging
   - Character debugging
   - Grid debugging
   - Color palette debugging
   - Collision detection debugging

6. **Memory Analysis**
   - Memory map reference
   - Memory dumps and inspection
   - Watch expressions
   - Data structure identification

7. **Timing and Synchronization**
   - Videopac timing model
   - VBLANK analysis
   - Mid-frame updates
   - Performance profiling

8. **Audio Debugging**
   - Audio system overview
   - Sound register analysis
   - Waveform debugging

9. **Input Debugging**
   - Input system overview
   - Port analysis
   - Input mapping verification

10. **Comparative Analysis**
    - Comparing with real hardware
    - Visual comparison techniques
    - Audio comparison techniques

11. **Case Studies**
    - Racing game color bug
    - Collision detection analysis
    - [Additional case studies]

12. **Quick Reference**
    - Links to all quick reference cards

13. **Appendices**
    - Intel 8048 architecture
    - Intel 8245 VDC architecture
    - External resources

## Component Design

### 1. Main Handbook Document (handbook.md)

**Purpose:** Central document containing all handbook content with navigation and cross-references.

**Structure:**
- Table of contents with anchor links
- Chapter sections with subsections
- Code examples and command snippets
- Screenshots and diagrams
- Cross-references to case studies and quick references

**Format:**
```markdown
# Videopac Debugging Handbook

Version: 1.0.0
Last Updated: [Date]

## Table of Contents
- [Chapter 1: Introduction](#chapter-1-introduction)
- [Chapter 2: Tools Overview](#chapter-2-tools-overview)
...

## Chapter 1: Introduction
...

## Chapter 2: Tools Overview
...
```

### 2. Case Study Documents

**Purpose:** Detailed investigations of real bugs with step-by-step analysis.

**Structure for each case study:**
```markdown
# Case Study: [Bug Name]

## Summary
Brief description of the bug and its symptoms.

## Symptoms
- Visual symptoms
- Behavioral symptoms
- Affected games/ROMs

## Investigation Approach
1. Initial hypothesis
2. Data collection strategy
3. Analysis methodology

## Trace Analysis
- Relevant trace excerpts
- Key findings
- Register values

## Disassembly Analysis
- Relevant code sections
- Code flow analysis
- Key instructions

## Root Cause
Detailed explanation of the underlying issue.

## Solution
- Fix description
- Code changes
- Verification

## Lessons Learned
Key takeaways for future debugging.
```

### 3. Quick Reference Cards

**Purpose:** Single-page references for quick lookup during debugging.

**Format:**
```markdown
# Quick Reference: [Topic]

## [Section 1]
| Command | Description | Example |
|---------|-------------|---------|
| cmd1    | desc1       | ex1     |
| cmd2    | desc2       | ex2     |

## [Section 2]
...
```

### 4. Workflow Templates

**Purpose:** Step-by-step checklists for common debugging scenarios.

**Format:**
```markdown
# Workflow: [Scenario]

## Problem Identification
- [ ] Verify the issue is reproducible
- [ ] Document exact symptoms
- [ ] Identify affected games

## Data Collection
- [ ] Capture screenshots/video
- [ ] Generate disassembly
- [ ] Capture execution trace
- [ ] Capture memory dumps

## Analysis
- [ ] Analyze trace for anomalies
- [ ] Identify relevant code sections
- [ ] Check register values
- [ ] Compare with expected behavior

## Hypothesis Testing
- [ ] Formulate hypothesis
- [ ] Design test
- [ ] Execute test
- [ ] Evaluate results

## Solution Verification
- [ ] Implement fix
- [ ] Test fix
- [ ] Verify no regressions
- [ ] Document solution
```

## Implementation Strategy

### Phase 1: Core Structure and Tools Documentation
1. Create main handbook.md with table of contents
2. Write Introduction and Overview chapter
3. Write Tools Overview chapter
4. Document disassembly commands and workflow
5. Document trace capture commands and workflow

### Phase 2: Debugging Workflows
1. Write Graphics Debugging chapter
2. Write Memory Analysis chapter
3. Write Timing and Synchronization chapter
4. Write Audio Debugging chapter
5. Write Input Debugging chapter

### Phase 3: Reference Materials
1. Create all quick reference cards
2. Write Intel 8048 architecture appendix
3. Write Intel 8245 VDC architecture appendix
4. Create workflow templates

### Phase 4: Case Studies
1. Document racing game color bug case study
2. Document collision detection analysis case study
3. Add 3+ additional case studies from past investigations

### Phase 5: Polish and Integration
1. Add cross-references between sections
2. Add diagrams and screenshots
3. Review for clarity and completeness
4. Create changelog and version tracking

## Frontend Selection: SDL vs Headless

### When to Use Headless Mode

Headless mode (frontend_headless) is preferred for:

1. **Automated Trace Capture**
   - No GUI overhead, faster execution
   - Batch processing multiple ROMs
   - CI/CD integration for regression testing
   - Scripted debugging workflows

2. **Disassembly Generation**
   - No need for display output
   - Faster processing
   - Can be run on servers without display

3. **Long-Running Traces**
   - Capturing traces over many frames
   - No window management needed
   - Lower memory footprint

4. **Automated Testing**
   - Regression test suites
   - ROM validation
   - Performance benchmarking

### When to Use SDL Mode

SDL mode (frontend_sdl) is preferred for:

1. **Interactive Debugging**
   - Using in-game debugger (F12)
   - Visual inspection of graphics issues
   - Real-time register monitoring
   - Stepping through code with visual feedback

2. **Graphics Debugging**
   - Seeing sprite rendering in real-time
   - Color palette verification
   - Collision detection visualization
   - Frame-by-frame visual analysis

3. **Audio Debugging**
   - Hearing audio output
   - Verifying sound effects
   - Testing audio timing

4. **Input Debugging**
   - Testing keyboard/joystick input
   - Verifying input responsiveness
   - Testing input mapping

### Command-Line Frontend Selection

```bash
# Use headless mode for trace capture
videopac_headless --trace --trace-output trace.txt bios.bin game.bin

# Use SDL mode for interactive debugging
videopac --debugger bios.bin game.bin

# Use headless mode for disassembly
videopac_headless --disassemble-rom game.bin --output game_disasm.txt
```

## Disassembly Command Design

### Command-Line Interface

The emulator should support disassembly through command-line options:

```bash
# Disassemble BIOS
videopac --disassemble-bios <bios_file> --output <output_file>

# Disassemble ROM
videopac --disassemble-rom <rom_file> --output <output_file>

# Disassemble with address range
videopac --disassemble-rom <rom_file> --start 0x0000 --end 0x0FFF --output <output_file>
```

### Disassembly Output Format

```
Address  Opcode    Mnemonic  Operands    ; Comments
-------  --------  --------  ----------  -----------
0x0000   23        MOV       A, #0x23    ; Initialize accumulator
0x0001   F0        MOV       A, @R0      ; Load from memory
0x0002   96 0A     JNZ       0x0A        ; Jump if not zero
```

## Trace Capture Command Design

### Command-Line Interface

```bash
# Basic trace
videopac --trace --trace-output trace.txt <bios> <rom>

# Trace with start condition (frame number)
videopac --trace --trace-start-frame 100 --trace-frames 1 --trace-output trace.txt <bios> <rom>

# Trace with start condition (key press)
videopac --trace --trace-start-key "1" --trace-frames 1 --trace-output trace.txt <bios> <rom>

# Trace with filtering (VDC only)
videopac --trace --trace-filter vdc --trace-output vdc_trace.txt <bios> <rom>

# Trace with filtering (memory writes)
videopac --trace --trace-filter mem-write --trace-output mem_trace.txt <bios> <rom>
```

### Trace Output Format

```
Frame:0001 Scanline:010 Cycle:00123 PC:0x0234 A:0x42 PSW:0x08 | MOV A, #0x42
Frame:0001 Scanline:010 Cycle:00125 PC:0x0235 A:0x42 PSW:0x08 | OUTL P1, A
Frame:0001 Scanline:010 Cycle:00127 PC:0x0236 A:0x42 PSW:0x08 | VDC_WRITE[0xA3] = 0x42
```

## VDC Register Reference Design

### Register Table Format

| Address | Name | Bits | Description |
|---------|------|------|-------------|
| 0xA0 | Control | 7:0 | VDC control register |
| 0xA1 | Status | 7:0 | VDC status register (read-only) |
| 0xA2 | Collision | 7:0 | Collision detection register |
| 0xA3 | Color | 3:0 | Background/grid color |
| 0xA4 | Y Position | 7:0 | Vertical grid position |
| 0xA5 | X Position | 7:0 | Horizontal grid position |
| 0xA6 | - | - | Unused |
| 0xA7 | Sound | 7:0 | Sound shift register byte 0 |
| 0xA8 | Sound | 7:0 | Sound shift register byte 1 |
| 0xA9 | Sound | 7:0 | Sound shift register byte 2 |
| 0xAA | Sound Control | 7:0 | Sound control and sprite 3 color |
| 0xAB | Sprite 0 Color | 3:0 | Sprite 0 color |
| 0xAC | Sprite 1 Color | 3:0 | Sprite 1 color |
| 0xAD | Sprite 2 Color | 3:0 | Sprite 2 color |
| 0xAE | Sprite 3 Color | 3:0 | Sprite 3 color (also in 0xAA) |
| 0xAF | Grid Control | 7:0 | Grid control register |

### Bit-Level Details

Each register should have detailed bit descriptions:

```markdown
### 0xA0 - Control Register

| Bit | Name | Description |
|-----|------|-------------|
| 7   | -    | Unused |
| 6   | -    | Unused |
| 5   | -    | Unused |
| 4   | -    | Unused |
| 3   | -    | Unused |
| 2   | -    | Unused |
| 1   | -    | Unused |
| 0   | EN   | Enable VDC output |
```

## CPU Register Reference Design

### Register Table

| Register | Size | Description |
|----------|------|-------------|
| A | 8-bit | Accumulator |
| PSW | 8-bit | Program Status Word |
| PC | 12-bit | Program Counter |
| SP | 3-bit | Stack Pointer (0-7) |
| R0-R7 | 8-bit | Working registers (bank 0) |
| R0'-R7' | 8-bit | Working registers (bank 1) |

### PSW Flags

| Bit | Name | Description |
|-----|------|-------------|
| 7 | CY | Carry flag |
| 6 | AC | Auxiliary carry flag |
| 5 | F0 | User flag 0 |
| 4 | BS | Register bank select (0 or 1) |
| 3-0 | - | Unused |

## Memory Map Reference Design

```markdown
### Videopac Memory Map

| Address Range | Size | Description |
|---------------|------|-------------|
| 0x0000-0x03FF | 1KB | Internal ROM (BIOS) |
| 0x0400-0x0FFF | 3KB | External ROM (Cartridge) |
| 0x1000-0x13FF | 1KB | Internal RAM |
| 0x1400-0x17FF | 1KB | External RAM (if present) |
| 0xA0-0xAF | 16 bytes | VDC registers |
| 0x20-0x27 | 8 bytes | Port 1 (input) |
| 0x90-0x97 | 8 bytes | Port 2 (input) |
```

## Case Study Template Design

### Racing Game Color Bug Case Study

```markdown
# Case Study: Racing Game Color Bug

## Summary
The player's car in "Course de Voitures" appears blue instead of red in the emulator, despite correct palette implementation.

## Symptoms
- Player car (sprite 3) renders as blue (color 1)
- Vertical road lines render as blue
- On real hardware, both appear red
- Other sprites (1, 2) render correctly as white

## Investigation Approach
1. Verify palette implementation is correct (RGBI 16-color)
2. Check sprite 3 color register value in debugger
3. Hypothesis: Game may dynamically change color during gameplay
4. Strategy: Capture trace of first frame after pressing '1'

## Data Collection
- Disassemble racing game ROM
- Capture execution trace for first frame
- Filter trace for VDC register writes
- Focus on sprite 3 color register (0xAA or 0xAE)

## Trace Analysis
[Trace excerpts showing VDC writes]

## Disassembly Analysis
[Code sections showing color register manipulation]

## Root Cause
[To be determined during investigation]

## Solution
[To be determined after root cause analysis]

## Lessons Learned
- Always verify game behavior on real hardware
- Dynamic register updates are common in Videopac games
- Trace analysis is essential for timing-dependent issues
```

## Workflow Template Design

### Graphics Debugging Workflow

```markdown
# Workflow: Graphics Debugging

## Problem Identification
- [ ] Verify the issue is reproducible
- [ ] Capture screenshot showing the issue
- [ ] Identify which graphics elements are affected (sprites, characters, grids)
- [ ] Note any patterns (specific colors, positions, timing)

## Initial Inspection
- [ ] Open in-game debugger (F12)
- [ ] Check VDC Register Viewer for affected elements
- [ ] Verify register values match expectations
- [ ] Check sprite visualization for pattern data

## Data Collection
- [ ] Generate ROM disassembly
- [ ] Capture execution trace for affected frame
- [ ] Filter trace for VDC register writes
- [ ] Capture memory dump if needed

## VDC Register Analysis
- [ ] Identify relevant VDC registers for the issue
- [ ] Check register write timing (VBLANK vs mid-frame)
- [ ] Verify register values in trace
- [ ] Compare with expected values

## Code Analysis
- [ ] Locate VDC write instructions in disassembly
- [ ] Trace code path leading to writes
- [ ] Identify data sources for register values
- [ ] Check for conditional logic affecting writes

## Hypothesis Formation
- [ ] Formulate hypothesis based on findings
- [ ] Identify specific emulator behavior to test
- [ ] Design test to verify hypothesis

## Testing
- [ ] Implement test or temporary fix
- [ ] Verify fix resolves the issue
- [ ] Test with multiple ROMs for regressions
- [ ] Compare with real hardware if possible

## Documentation
- [ ] Document root cause
- [ ] Document solution
- [ ] Add case study to handbook
- [ ] Update relevant handbook sections
```

## Quick Reference Card Design

### Disassembly Commands Quick Reference

```markdown
# Quick Reference: Disassembly Commands

## Basic Commands

| Command | Description |
|---------|-------------|
| `--disassemble-bios <file>` | Disassemble BIOS file |
| `--disassemble-rom <file>` | Disassemble ROM file |
| `--output <file>` | Specify output file |
| `--start <addr>` | Start address (hex) |
| `--end <addr>` | End address (hex) |

## Examples

```bash
# Disassemble entire BIOS
videopac --disassemble-bios bios.bin --output bios_disasm.txt

# Disassemble ROM
videopac --disassemble-rom game.bin --output game_disasm.txt

# Disassemble address range
videopac --disassemble-rom game.bin --start 0x0000 --end 0x0FFF --output game_partial.txt
```

## Output Format

```
Address  Opcode    Mnemonic  Operands
0x0000   23        MOV       A, #0x23
0x0001   F0        MOV       A, @R0
```
```

### Trace Commands Quick Reference

```markdown
# Quick Reference: Trace Commands

## Basic Commands

| Command | Description |
|---------|-------------|
| `--trace` | Enable execution tracing |
| `--trace-output <file>` | Specify trace output file |
| `--trace-start-frame <n>` | Start tracing at frame N |
| `--trace-frames <n>` | Trace for N frames |
| `--trace-start-key <key>` | Start tracing on key press |
| `--trace-filter <type>` | Filter trace output |

## Filter Types

| Filter | Description |
|--------|-------------|
| `all` | All events (default) |
| `cpu` | CPU instructions only |
| `vdc` | VDC register writes only |
| `mem-write` | Memory writes only |
| `mem-read` | Memory reads only |
| `io` | I/O port accesses only |

## Examples

```bash
# Trace first frame after key press
videopac --trace --trace-start-key "1" --trace-frames 1 --trace-output trace.txt bios.bin game.bin

# Trace VDC writes only
videopac --trace --trace-filter vdc --trace-output vdc_trace.txt bios.bin game.bin

# Trace specific frame range
videopac --trace --trace-start-frame 100 --trace-frames 10 --trace-output trace.txt bios.bin game.bin
```
```

### Debugger Shortcuts Quick Reference

```markdown
# Quick Reference: In-Game Debugger Shortcuts

## Debugger Control

| Key | Action |
|-----|--------|
| F12 | Toggle debugger |
| F10 | Open menu |
| Esc | Close debugger/menu |

## Execution Control

| Key | Action |
|-----|--------|
| F5 | Continue execution |
| F9 | Step (single instruction) |
| F8 | Step over (skip CALL) |
| Shift+F8 | Step out (return from subroutine) |
| F2 | Toggle breakpoint at cursor |

## Navigation

| Key | Action |
|-----|--------|
| Up/Down | Navigate lists |
| PgUp/PgDn | Scroll memory/disassembly |
| Home/End | Jump to start/end |
| Ctrl+G | Go to address |

## Panels

| Panel | Description |
|-------|-------------|
| CPU State | Registers, flags, current instruction |
| Memory Viewer | Hex/ASCII memory browser |
| Disassembly | Code disassembly around PC |
| VDC Registers | Video controller state |
| Breakpoints | Breakpoint manager |
| Watch | Watch expressions |
| Call Stack | Subroutine call stack |
```

## Correctness Properties

### Property 1: Handbook Completeness
**Description:** All required chapters and sections are present and complete.

**Validation:**
- Verify all 13 chapters are present in handbook.md
- Verify all quick reference cards are created
- Verify at least 5 case studies are documented
- Verify all workflow templates are created

### Property 2: Cross-Reference Integrity
**Description:** All internal links and cross-references are valid.

**Validation:**
- Check all anchor links in table of contents
- Check all cross-references to case studies
- Check all cross-references to quick references
- Verify no broken links

### Property 3: Command Accuracy
**Description:** All documented commands and options are accurate and functional.

**Validation:**
- Test all disassembly commands
- Test all trace commands
- Test all debugger shortcuts
- Verify command output matches documentation

### Property 4: Example Validity
**Description:** All code examples and command examples are valid and produce expected results.

**Validation:**
- Run all example commands
- Verify example outputs match documentation
- Test example workflows with real ROMs

### Property 5: Technical Accuracy
**Description:** All technical information (registers, memory map, instruction set) is accurate.

**Validation:**
- Cross-reference with Intel 8048 datasheet
- Cross-reference with Intel 8245 datasheet
- Verify register addresses and bit definitions
- Verify memory map against emulator implementation

## Testing Strategy

### Manual Testing
1. Follow each workflow template with a real ROM issue
2. Verify all commands produce expected output
3. Check all links and cross-references
4. Review for clarity and completeness

### Peer Review
1. Have another developer review the handbook
2. Collect feedback on clarity and usefulness
3. Identify missing information or unclear sections
4. Iterate based on feedback

### Real-World Validation
1. Use handbook to debug at least 3 different ROM issues
2. Document any gaps or missing information discovered
3. Update handbook based on real-world usage
4. Collect metrics on debugging time reduction

## Maintenance Plan

### Version Control
- Use semantic versioning (MAJOR.MINOR.PATCH)
- Increment PATCH for typo fixes and clarifications
- Increment MINOR for new sections or case studies
- Increment MAJOR for structural changes

### Update Triggers
- New debugging features added to emulator
- New bugs investigated and resolved
- Improved workflows discovered
- User feedback received

### Review Schedule
- Review handbook after each major emulator release
- Update case studies as new bugs are investigated
- Update quick references when commands change
- Update architecture references if emulator behavior changes

## Dependencies

### Emulator Features Required
- Command-line disassembly support
- Command-line trace capture support
- In-game debugger with all documented features
- VDC register viewer with sprite visualization

### External Tools
- Text editor for viewing/editing Markdown
- Diff tool for comparing traces
- Image viewer for screenshots
- Optional: Python for trace analysis scripts

## Future Enhancements

### Potential Additions
1. Interactive HTML version with search
2. Video tutorials for common workflows
3. Automated trace analysis scripts
4. ROM test suite for regression testing
5. Integration with external debugging tools
6. Graphical trace visualization tools

### Community Contributions
- Accept case study contributions from other developers
- Accept workflow improvements
- Accept corrections and clarifications
- Maintain contribution guidelines
