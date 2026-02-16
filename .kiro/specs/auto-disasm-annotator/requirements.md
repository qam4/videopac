# Auto Disassembly Annotator - Requirements

## 1. Overview

### 1.1 Purpose
Create a tool that automatically annotates raw 8048 disassembly output with meaningful labels, function names, and comments, similar to manually annotated files like `doc/french_bios_annotated.txt` and `doc/satellite-attack-disassembly.txt`.

### 1.2 Goals
- Reduce manual effort in reverse engineering ROM/BIOS files
- Provide consistent, high-quality annotations
- Enable pattern recognition and code structure analysis
- Make disassembly more readable and maintainable

### 1.3 Non-Goals
- Perfect annotation (manual review still expected)
- Semantic understanding of game logic
- Decompilation to high-level language
- Real-time annotation during emulation

## 2. User Stories

### 2.1 As a developer analyzing a new ROM
**I want** the tool to automatically identify and label functions  
**So that** I can quickly understand the code structure without manual analysis

**Acceptance Criteria:**
- Tool identifies function entry points from CALL instructions
- Tool generates meaningful function labels (e.g., `func_0400`, `subroutine_0x123`)
- Tool marks function boundaries with RET instructions

### 2.2 As a developer debugging VDC issues
**I want** the tool to recognize and annotate VDC register writes  
**So that** I can quickly find graphics-related code

**Acceptance Criteria:**
- Tool identifies OUTL instructions to VDC ports
- Tool adds comments indicating which VDC register is being written
- Tool recognizes common VDC patterns (sprite updates, color changes)

### 2.3 As a developer studying code patterns
**I want** the tool to identify and label common patterns (loops, initialization, tables)  
**So that** I can understand the code's purpose faster

**Acceptance Criteria:**
- Tool identifies loop structures (DJNZ, backward jumps)
- Tool identifies initialization sequences
- Tool identifies data tables vs executable code
- Tool adds descriptive comments for recognized patterns

### 2.4 As a developer comparing different ROMs
**I want** consistent annotation style across all analyzed files  
**So that** I can easily compare code between different games

**Acceptance Criteria:**
- Tool uses consistent naming conventions
- Tool uses consistent comment formatting
- Tool produces output compatible with existing annotated files

### 2.5 As a developer improving the tool
**I want** the tool to be extensible with new pattern recognition rules  
**So that** I can add domain-specific knowledge over time

**Acceptance Criteria:**
- Tool has pluggable pattern recognition system
- Tool can load custom pattern definitions
- Tool can be configured with different annotation styles

## 3. Functional Requirements

### 3.1 Input Processing
- **FR-1.1**: Tool shall accept raw disassembly files as input
- **FR-1.2**: Tool shall support both BIOS (offset 0x000) and ROM (offset 0x400) files
- **FR-1.3**: Tool shall parse disassembly format: `address: opcode bytes mnemonic operands`
- **FR-1.4**: Tool shall handle multi-pass analysis (build symbol table, then annotate)

### 3.2 Function Identification
- **FR-2.1**: Tool shall identify function entry points from CALL instruction targets
- **FR-2.2**: Tool shall identify function boundaries using RET/RETR instructions
- **FR-2.3**: Tool shall generate unique labels for each function
- **FR-2.4**: Tool shall detect nested functions and handle appropriately
- **FR-2.5**: Tool shall identify interrupt vectors (0x000, 0x003, 0x007, 0x009)

### 3.3 Jump Target Labeling
- **FR-3.1**: Tool shall identify all jump targets (JMP, JZ, JNZ, DJNZ, etc.)
- **FR-3.2**: Tool shall generate labels for jump targets (e.g., `loc_0x123`)
- **FR-3.3**: Tool shall distinguish between local jumps and function calls
- **FR-3.4**: Tool shall handle conditional vs unconditional jumps

### 3.4 Pattern Recognition
- **FR-4.1**: Tool shall recognize loop patterns (backward jumps, DJNZ)
- **FR-4.2**: Tool shall recognize initialization sequences (CLR A, MOV R0, #0x00)
- **FR-4.3**: Tool shall recognize VDC register writes (OUTL P1/P2 patterns)
- **FR-4.4**: Tool shall recognize data tables (sequences of invalid instructions)
- **FR-4.5**: Tool shall recognize common BIOS call patterns

### 3.5 Comment Generation
- **FR-5.1**: Tool shall add inline comments for VDC register operations
- **FR-5.2**: Tool shall add inline comments for recognized patterns
- **FR-5.3**: Tool shall add function header comments with purpose/parameters
- **FR-5.4**: Tool shall add comments for magic numbers and constants
- **FR-5.5**: Tool shall preserve existing comments if present

### 3.6 Output Formatting
- **FR-6.1**: Tool shall output annotated disassembly in compatible format
- **FR-6.2**: Tool shall use consistent indentation and spacing
- **FR-6.3**: Tool shall place labels on separate lines before instructions
- **FR-6.4**: Tool shall align comments consistently
- **FR-6.5**: Tool shall add section headers for major code regions

### 3.7 Configuration
- **FR-7.1**: Tool shall support configuration file for pattern definitions
- **FR-7.2**: Tool shall support command-line options for annotation style
- **FR-7.3**: Tool shall support custom label naming schemes
- **FR-7.4**: Tool shall support verbosity levels for comments

## 4. Non-Functional Requirements

### 4.1 Performance
- **NFR-1.1**: Tool shall process typical ROM file (2KB) in under 5 seconds
- **NFR-1.2**: Tool shall handle BIOS file (1KB) in under 2 seconds
- **NFR-1.3**: Tool shall use reasonable memory (< 100MB for typical files)

### 4.2 Usability
- **NFR-2.1**: Tool shall provide clear error messages for invalid input
- **NFR-2.2**: Tool shall provide progress indication for large files
- **NFR-2.3**: Tool shall have intuitive command-line interface
- **NFR-2.4**: Tool shall include help text and usage examples

### 4.3 Maintainability
- **NFR-3.1**: Tool shall be written in Python for consistency with other scripts
- **NFR-3.2**: Tool shall have modular architecture for pattern recognition
- **NFR-3.3**: Tool shall include unit tests for pattern recognition
- **NFR-3.4**: Tool shall have clear documentation for adding new patterns

### 4.4 Compatibility
- **NFR-4.1**: Tool shall work on Windows, Linux, and macOS
- **NFR-4.2**: Tool shall require only Python 3.8+ standard library
- **NFR-4.3**: Tool shall produce output compatible with existing tools

## 5. Constraints

### 5.1 Technical Constraints
- Must work with existing disassembly format
- Must not modify original disassembly tool
- Must handle incomplete/partial disassembly gracefully

### 5.2 Resource Constraints
- Development time: ~2-3 weeks for initial version
- No external dependencies beyond Python standard library

## 6. Assumptions

- Raw disassembly is syntactically correct
- Disassembly follows consistent format
- Code and data sections can be distinguished by analysis
- Most functions follow standard calling conventions

## 7. Dependencies

- Existing disassembly tool (`disasm_tool`)
- Python 3.8+
- Access to sample annotated files for reference

## 8. Success Metrics

- Tool correctly identifies 90%+ of functions in test ROMs
- Tool generates meaningful labels for 80%+ of jump targets
- Tool recognizes 70%+ of common patterns
- Manual annotation time reduced by 50%+
- Output quality comparable to manual annotations

## 9. Future Enhancements

- Machine learning for pattern recognition
- Integration with trace analysis for dynamic annotation
- Cross-reference generation between functions
- Call graph visualization
- Data flow analysis
- Integration with debugger for live annotation
