# Implementation Plan: Philips Videopac (Odyssey2) Emulator

## Overview

This implementation plan breaks down the Videopac emulator into discrete, testable components. The emulator will be implemented in C++ with a modular architecture separating the CPU, VDC, memory system, input handling, and frontend layers. We'll use CMake for the build system, Google Test for unit testing, and RapidCheck for property-based testing.

The implementation follows a bottom-up approach: core data structures and utilities first, then individual hardware components, followed by integration and frontend layers. Each component will be tested with both unit tests and property-based tests to ensure correctness.

## Tasks

- [x] 1. Project setup and build system
  - Create CMake project structure with src/, include/, tests/ directories
  - Configure CMake to build main emulator library, standalone executable, and libretro core
  - Set up Google Test framework for unit testing
  - Set up RapidCheck framework for property-based testing
  - Create initial header files for core types and interfaces
  - Configure C++17 or later standard
  - Set up compiler flags for warnings and optimizations
  - _Requirements: All (foundational)_

- [x] 2. Implement core data types and utilities
  - [x] 2.1 Create common types header (types.h)
    - Define uint8, uint16, uint32, uint64 type aliases
    - Define VideoStandard enum (PAL, NTSC)
    - Define Result/Error types for error handling
    - Define Color struct with RGB components
    - Define palette constants for 8 colors (bright and dim)
    - _Requirements: 3.1, 12.6, 12.7_

  - [x] 2.2 Implement utility functions
    - Create bit manipulation helpers (get_bit, set_bit, clear_bit)
    - Create byte packing/unpacking utilities
    - Create checksum calculation function for save states
    - _Requirements: 14.1, 14.5_


- [x] 3. Implement CPU component (Intel 8048)
  - [x] 3.1 Create CPU state structure (cpu.h)
    - Define CPUState struct with all registers (PC, A, PSW, R0-R7, R0'-R7')
    - Define internal RAM array (64 bytes)
    - Define stack array (8 levels) and stack pointer
    - Define port states (Port 1, Port 2)
    - Define timer/counter register and flags
    - Define interrupt enable flags
    - Define clock cycle counter
    - _Requirements: 1.3, 1.4, 1.5, 1.6, 1.10_

  - [x] 3.2 Implement CPU class interface (cpu.cpp)
    - Implement reset() method to initialize CPU state
    - Implement execute_instruction() method skeleton
    - Implement read_memory() and write_memory() methods
    - Implement read_port() and write_port() methods
    - Implement trigger_interrupt() method
    - Implement get_state() and set_state() for save states
    - _Requirements: 1.1, 1.2, 9.1, 9.2, 9.3, 9.4, 9.5, 9.6_

  - [x] 3.3 Implement instruction decoder
    - Create opcode lookup table mapping opcodes to instruction handlers
    - Implement instruction fetch logic
    - Implement operand fetch for 2-byte instructions
    - Return cycle count (1 or 2) for each instruction
    - _Requirements: 1.1, 1.8, 1.9_

  - [x] 3.4 Implement data transfer instructions
    - Implement MOV variants (register, immediate, indirect)
    - Implement MOVX (external memory access)
    - Implement MOVP (program memory read)
    - Implement XCH and XCHD (exchange operations)
    - _Requirements: 1.1_

  - [x] 3.5 Implement arithmetic instructions
    - Implement ADD and ADDC (add with carry)
    - Implement INC and DEC (increment/decrement)
    - Implement DA (decimal adjust)
    - Update carry and auxiliary carry flags correctly
    - _Requirements: 1.1, 1.2, 1.5_

  - [x] 3.6 Implement logical instructions
    - Implement ANL, ORL, XRL (AND, OR, XOR)
    - Implement CLR and CPL (clear, complement)
    - Update flags as appropriate
    - _Requirements: 1.1, 1.2, 1.5_

  - [x] 3.7 Implement branch instructions
    - Implement JMP (unconditional jump)
    - Implement conditional jumps (JC, JNC, JZ, JNZ, JT0, JNT0, JT1, JNT1, JF0, JF1, JTF, JNIBF, JOBF)
    - Implement DJNZ (decrement and jump if not zero)
    - Update program counter correctly for all branches
    - _Requirements: 1.1, 1.2_

  - [x] 3.8 Implement subroutine instructions
    - Implement CALL (push PC to stack, jump to address)
    - Implement RET (pop PC from stack)
    - Implement RETR (return from interrupt, restore PSW)
    - Handle stack overflow/underflow
    - _Requirements: 1.1, 1.10, 9.7, 9.8_

  - [x] 3.9 Implement I/O instructions
    - Implement IN and OUT (port I/O)
    - Implement INS and OUTL (bus I/O)
    - Implement ANL and ORL for port operations
    - _Requirements: 1.1, 1.6_

  - [x] 3.10 Implement control instructions
    - Implement NOP (no operation)
    - Implement EN I and DIS I (enable/disable interrupts)
    - Implement EN TCNTI and DIS TCNTI (timer interrupts)
    - Implement STRT CNT, STRT T, STOP TCNT (timer control)
    - Implement SEL RB0 and SEL RB1 (register bank selection)
    - _Requirements: 1.1, 1.4, 8.6, 8.7, 9.1, 9.3_

  - [x] 3.11 Implement timer/counter functionality
    - Implement timer increment logic
    - Implement timer overflow detection
    - Trigger timer interrupt on overflow
    - _Requirements: 8.6, 8.7, 9.3, 9.4_

  - [x]* 3.12 Write unit tests for CPU instructions
    - Test specific instruction examples (ADD, MOV, JMP, CALL, etc.)
    - Test edge cases (stack overflow, timer overflow)
    - Test flag updates for arithmetic operations
    - Test register bank switching
    - _Requirements: 1.1, 1.2, 1.4, 1.5, 1.10_

  - [ ]* 3.13 Write property test for instruction execution correctness
    - **Property 1: Instruction execution correctness**
    - **Validates: Requirements 1.1, 1.2**

  - [ ]* 3.14 Write property test for internal RAM persistence
    - **Property 2: Internal RAM persistence**
    - **Validates: Requirements 1.3**

  - [ ]* 3.15 Write property test for register bank switching
    - **Property 3: Register bank switching**
    - **Validates: Requirements 1.4**

  - [ ]* 3.16 Write property test for flag operations
    - **Property 4: Flag operations**
    - **Validates: Requirements 1.5**

  - [ ]* 3.17 Write property test for I/O port access
    - **Property 5: I/O port access**
    - **Validates: Requirements 1.6**

  - [ ]* 3.18 Write property test for instruction cycle counts
    - **Property 6: Instruction cycle counts**
    - **Validates: Requirements 1.9**

  - [ ]* 3.19 Write property test for stack operations
    - **Property 7: Stack operations**
    - **Validates: Requirements 1.10**

  - [ ]* 3.20 Write property test for interrupt stack preservation
    - **Property 8: Interrupt stack preservation**
    - **Validates: Requirements 9.7, 9.8**


- [x] 4. Implement memory system
  - [x] 4.1 Create memory system state structure (memory.h)
    - Define MemoryState struct with BIOS ROM array (1KB)
    - Define cartridge ROM array (up to 8KB)
    - Define external RAM array (128 bytes)
    - Define current bank selection state
    - Define ROM size and bank count
    - _Requirements: 2.1, 2.2, 2.4_

  - [x] 4.2 Implement memory system class (memory.cpp)
    - Implement load_bios() to load BIOS ROM from file
    - Implement load_cartridge() to load ROM and detect size
    - Implement read_program() for ROM access
    - Implement read_external() for RAM/VDC access
    - Implement write_external() for RAM/VDC writes
    - Implement set_bank() for bank switching
    - Implement get_state() and set_state() for save states
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.8, 11.1, 11.2_

  - [x] 4.3 Implement bank switching logic
    - Monitor Port 1 pins P10 and P11 for bank selection
    - Switch active ROM bank based on pin states
    - Support 2-bank (4KB) and 4-bank (8KB) cartridges
    - _Requirements: 2.3, 11.3, 11.4, 11.5_

  - [x] 4.4 Implement memory access control
    - Check Port 1 pin P13 for VDC enable
    - Check Port 1 pin P14 for external RAM enable
    - Check Port 1 pin P16 for copy mode
    - Route memory access to correct component
    - _Requirements: 2.5, 2.6, 2.7_

  - [x] 4.5 Implement ROM validation
    - Check file size is valid (2KB, 4KB, or 8KB)
    - Verify file can be read successfully
    - Return error for invalid or corrupted files
    - _Requirements: 11.6, 11.7_

  - [x]* 4.6 Write unit tests for memory system
    - Test BIOS and ROM loading
    - Test bank switching with different ROM sizes
    - Test memory access control via Port 1
    - Test error handling for invalid ROMs
    - _Requirements: 2.1, 2.2, 2.3, 11.2, 11.6_

  - [ ]* 4.7 Write property test for external RAM persistence
    - **Property 9: External RAM persistence**
    - **Validates: Requirements 2.4, 2.5**

  - [ ]* 4.8 Write property test for VDC register access
    - **Property 10: VDC register access**
    - **Validates: Requirements 2.6, 3.6**

  - [ ]* 4.9 Write property test for bank switching correctness
    - **Property 11: Bank switching correctness**
    - **Validates: Requirements 2.3, 11.4, 11.5**

  - [ ]* 4.10 Write property test for ROM size detection
    - **Property 12: ROM size detection**
    - **Validates: Requirements 11.2**

  - [ ]* 4.11 Write property test for ROM validation
    - **Property 13: ROM validation**
    - **Validates: Requirements 11.6**


- [x] 5. Checkpoint - Core components functional
  - Ensure all tests pass for CPU and memory system
  - Verify CPU can execute basic instruction sequences
  - Verify memory system correctly loads ROMs and handles banking
  - Ask the user if questions arise

- [ ] 6. Implement VDC component (Intel 8245)
  - [x] 6.1 Create VDC state structure (vdc.h)
    - Define VDCState struct with register array (256 bytes)
    - Define framebuffer array (160x200 pixels)
    - Define scanline and beam position counters
    - Define collision state register
    - Define video standard (PAL/NTSC)
    - Define audio shift register (24 bits)
    - Define audio state (frequency, volume, enable)
    - _Requirements: 3.1, 3.6, 3.10, 3.11, 6.1_

  - [x] 6.2 Implement VDC class interface (vdc.cpp)
    - Implement reset() method
    - Implement write_register() and read_register() methods
    - Implement tick() to advance VDC by cycle count
    - Implement render_scanline() skeleton
    - Implement is_vblank() and is_hblank() methods
    - Implement get_framebuffer() method
    - Implement get_audio_sample() method
    - Implement get_state() and set_state() for save states
    - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.10, 3.11, 6.1, 6.2_

  - [x] 6.3 Implement timing and synchronization
    - Calculate cycles per scanline for PAL and NTSC
    - Track current scanline (0-311 PAL, 0-261 NTSC)
    - Track beam position within scanline
    - Detect VBLANK period (22 scanlines NTSC, 28 scanlines PAL)
    - Detect HBLANK period (12 microseconds per scanline)
    - Update status register (0xA1) with VBLANK/HBLANK flags
    - Update beam position registers (0xA4, 0xA5)
    - _Requirements: 3.2, 3.3, 3.4, 3.5, 3.10, 3.11, 8.2, 8.3_

  - [x] 6.4 Implement sprite rendering
    - Parse sprite control registers for position, color, pattern
    - Render 4 sprites (8x8 pixels each)
    - Support double-size sprites (16x16 pixels)
    - Apply sprite colors from palette
    - Handle sprite enable/disable via control register bit 5
    - _Requirements: 4.1, 4.2, 4.3, 4.4_

  - [ ] 6.5 Implement character rendering
    - Load built-in 64-character set patterns
    - Parse character control registers for position, color, character code
    - Render up to 12 single characters
    - Render up to 4 quad character groups (16 characters total)
    - _Requirements: 4.5, 4.6_

  - [ ] 6.6 Implement grid rendering
    - Render background grid with 8 rows and 9 columns
    - Support grid fill mode (bit 7 of 0xA0)
    - Support dot grid mode (bit 6 of 0xA0)
    - Apply grid color from color register (0xA3)
    - Handle grid enable/disable via control register bit 3
    - _Requirements: 4.7, 4.8, 4.9, 4.10_

  - [ ] 6.7 Implement rendering pipeline
    - Clear scanline buffer to background color
    - Render grid elements if enabled
    - Render characters if enabled
    - Render sprites if enabled (on top)
    - Copy scanline to framebuffer
    - Enforce rendering priority order
    - _Requirements: 3.1, 4.1, 4.5, 4.7, 12.9_

  - [ ] 6.8 Implement collision detection
    - Track enabled objects from collision register (0xA2)
    - Detect pixel overlaps during rendering
    - Set collision bits for: sprites 0-3, vertical grid, horizontal grid, dots, characters
    - Update collision register during VBLANK
    - Set status register bit 7 for character-to-character collisions
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5, 5.6_

  - [ ] 6.9 Implement audio generation
    - Load 24-bit pattern from registers 0xA7-0xA9
    - Implement shift register with configurable frequency (983Hz or 3933Hz)
    - Implement noise mode with XOR feedback
    - Implement loop mode (continuous or one-shot)
    - Apply volume scaling (4-bit value from register 0xAA)
    - Enable/disable audio via bit 7 of register 0xAA
    - Generate audio samples synchronized with frame timing
    - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 6.8, 6.9, 6.10, 13.1, 13.2, 13.3, 13.4, 13.5, 13.6, 13.7_

  - [ ]* 6.10 Write unit tests for VDC
    - Test sprite rendering at various positions
    - Test character rendering with different character codes
    - Test grid rendering in different modes
    - Test collision detection between objects
    - Test audio generation with different patterns
    - Test VBLANK and HBLANK timing
    - _Requirements: 3.2, 3.3, 3.4, 4.1, 4.2, 4.5, 5.1, 6.1_

  - [ ]* 6.11 Write property test for framebuffer dimensions
    - **Property 14: Framebuffer dimensions**
    - **Validates: Requirements 3.1**

  - [ ]* 6.12 Write property test for status register updates
    - **Property 15: Status register updates**
    - **Validates: Requirements 3.10**

  - [ ]* 6.13 Write property test for beam position tracking
    - **Property 16: Beam position tracking**
    - **Validates: Requirements 3.11**

  - [ ]* 6.14 Write property test for sprite positioning
    - **Property 17: Sprite positioning**
    - **Validates: Requirements 4.1, 4.2**

  - [ ]* 6.15 Write property test for sprite color rendering
    - **Property 18: Sprite color rendering**
    - **Validates: Requirements 4.3**

  - [ ]* 6.16 Write property test for character rendering
    - **Property 19: Character rendering**
    - **Validates: Requirements 4.5, 4.6**

  - [ ]* 6.17 Write property test for grid color application
    - **Property 20: Grid color application**
    - **Validates: Requirements 4.10**

  - [ ]* 6.18 Write property test for rendering priority order
    - **Property 21: Rendering priority order**
    - **Validates: Requirements 12.9**

  - [ ]* 6.19 Write property test for collision detection accuracy
    - **Property 22: Collision detection accuracy**
    - **Validates: Requirements 5.1, 5.3, 5.5**

  - [ ]* 6.20 Write property test for collision enable filtering
    - **Property 23: Collision enable filtering**
    - **Validates: Requirements 5.2**

  - [ ]* 6.21 Write property test for sprite collision detection
    - **Property 24: Sprite collision detection**
    - **Validates: Requirements 5.5**

  - [ ]* 6.22 Write property test for audio pattern generation
    - **Property 25: Audio pattern generation**
    - **Validates: Requirements 6.1, 6.2**

  - [ ]* 6.23 Write property test for audio volume control
    - **Property 26: Audio volume control**
    - **Validates: Requirements 6.9**


- [ ] 7. Implement input handler
  - [ ] 7.1 Create input handler state structure (input.h)
    - Define keyboard matrix state (8x8 array)
    - Define joystick state structures (2 joysticks, 5 bits each)
    - Define key mapping table (host keys to Videopac keys)
    - _Requirements: 7.1, 7.4, 7.8_

  - [ ] 7.2 Implement input handler class (input.cpp)
    - Implement set_key_state() for keyboard input
    - Implement set_joystick_state() for joystick directions
    - Implement set_joystick_button() for fire buttons
    - Implement read_keyboard() to read matrix columns for selected row
    - Implement read_joystick() to read joystick state based on Port 2 selection
    - Implement map_host_key() to convert host keys to Videopac keys
    - _Requirements: 7.1, 7.2, 7.3, 7.4, 7.5, 7.6, 7.7, 7.8, 7.9_

  - [ ] 7.3 Implement keyboard matrix logic
    - Support row selection via Port 1 (P10-P17)
    - Return column states via Port 2 (P20-P27) when P12=0
    - Map all Videopac keys to correct row/column positions
    - _Requirements: 7.1, 7.2, 7.3_

  - [ ] 7.4 Implement joystick reading logic
    - Support joystick selection via Port 2 (P20-P22)
    - Return 5-bit joystick state (up, down, left, right, fire)
    - Support two independent joysticks
    - _Requirements: 7.4, 7.5, 7.6, 7.7_

  - [ ]* 7.5 Write unit tests for input handler
    - Test keyboard matrix reading with various key combinations
    - Test joystick reading for both joysticks
    - Test host key mapping
    - _Requirements: 7.1, 7.2, 7.4, 7.7, 7.8_

  - [ ]* 7.6 Write property test for keyboard matrix correctness
    - **Property 27: Keyboard matrix correctness**
    - **Validates: Requirements 7.2**

  - [ ]* 7.7 Write property test for joystick input reading
    - **Property 28: Joystick input reading**
    - **Validates: Requirements 7.7**

  - [ ]* 7.8 Write property test for host input mapping
    - **Property 29: Host input mapping**
    - **Validates: Requirements 7.8, 7.9**


- [ ] 8. Implement emulator core orchestration
  - [ ] 8.1 Create emulator core state structure (emulator.h)
    - Define EmulatorCore class containing CPU, VDC, Memory, and Input instances
    - Define configuration structure (video standard, timing parameters)
    - Define running/paused state flags
    - Define frame counter and cycle counters
    - _Requirements: 8.1, 8.2, 8.3, 8.4, 8.5_

  - [ ] 8.2 Implement emulator core class (emulator.cpp)
    - Implement constructor to initialize all components
    - Implement load_bios() to load BIOS into memory system
    - Implement load_rom() to load cartridge into memory system
    - Implement reset() to reset all components
    - Implement run_frame() skeleton
    - Implement step() for single-instruction debugging
    - Implement get_framebuffer() to retrieve VDC output
    - Implement get_audio_buffer() to retrieve audio samples
    - _Requirements: 8.1, 8.10, 10.1, 10.2, 11.1, 11.8, 15.7_

  - [ ] 8.3 Implement frame execution loop
    - Calculate cycles per frame based on video standard (PAL/NTSC)
    - Calculate cycles per scanline
    - Execute CPU instructions for appropriate cycle count per scanline
    - Call VDC tick() to advance video generation
    - Render scanlines when not in VBLANK
    - Check for HBLANK interrupts
    - Check for VBLANK interrupts at end of frame
    - Update collision detection during VBLANK
    - Generate audio samples for the frame
    - _Requirements: 8.1, 8.2, 8.3, 8.4, 8.5, 8.8, 8.9, 9.5, 9.6_

  - [ ] 8.4 Implement interrupt handling
    - Trigger external interrupts via CPU interrupt pin
    - Trigger timer interrupts on counter overflow
    - Trigger VBLANK interrupts at end of frame
    - Trigger horizontal line interrupts when enabled
    - Route interrupts to correct cartridge vectors (0x402, 0x404, 0x406)
    - _Requirements: 8.8, 9.1, 9.2, 9.3, 9.4, 9.5, 9.6_

  - [ ] 8.5 Implement timing synchronization
    - Track real-time clock for frame pacing
    - Sleep or busy-wait to maintain correct frame rate (50Hz or 60Hz)
    - Handle frame skipping if emulation falls behind
    - _Requirements: 8.1, 8.2, 8.3, 8.10_

  - [ ] 8.6 Implement BIOS integration
    - Load 1KB BIOS ROM at startup
    - Jump to cartridge vector 0x400 on reset
    - Support BIOS routine calls from cartridges
    - Provide access to BIOS routines (VDC enable, RAM enable, joystick read, display on/off, select game)
    - _Requirements: 10.1, 10.2, 10.3, 10.4, 10.5, 10.6, 10.7, 10.8_

  - [ ]* 8.7 Write integration tests for emulator core
    - Test complete frame execution with all components
    - Test interrupt handling across components
    - Test BIOS integration and routine calls
    - Test timing and synchronization
    - _Requirements: 8.1, 8.2, 8.3, 9.1, 9.5, 10.1_


- [ ] 9. Checkpoint - Core emulation complete
  - Ensure all tests pass for CPU, VDC, memory, input, and core
  - Verify emulator can execute a simple test ROM
  - Verify frame timing is correct for PAL and NTSC
  - Verify audio generation produces correct waveforms
  - Ask the user if questions arise

- [ ] 10. Implement state management (save states)
  - [ ] 10.1 Create save state structure (savestate.h)
    - Define SaveState struct with version number
    - Include CPUState, VDCState, MemoryState, InputState
    - Include frame counter
    - Include checksum field
    - _Requirements: 14.1_

  - [ ] 10.2 Implement save state serialization (savestate.cpp)
    - Implement serialize() to convert state to binary format
    - Implement deserialize() to restore state from binary
    - Calculate and verify checksum
    - Validate version compatibility
    - Handle file I/O errors
    - _Requirements: 14.2, 14.3, 14.5, 14.6_

  - [ ] 10.3 Integrate save states into emulator core
    - Implement save_state() method in EmulatorCore
    - Implement load_state() method in EmulatorCore
    - Support multiple save state slots
    - _Requirements: 14.2, 14.3, 14.4_

  - [ ]* 10.4 Write unit tests for save states
    - Test serialization and deserialization
    - Test checksum validation
    - Test version compatibility checking
    - Test error handling for corrupted files
    - _Requirements: 14.2, 14.5, 14.6_

  - [ ]* 10.5 Write property test for save state completeness
    - **Property 30: Save state completeness**
    - **Validates: Requirements 14.1**

  - [ ]* 10.6 Write property test for save state round-trip
    - **Property 31: Save state serialization round-trip**
    - **Validates: Requirements 14.2, 14.3**

  - [ ]* 10.7 Write property test for save state validation
    - **Property 32: Save state validation**
    - **Validates: Requirements 14.5**


- [ ] 11. Implement disassembler component
  - [ ] 11.1 Create disassembler structures (disassembler.h)
    - Define Instruction struct (address, opcode, operand, mnemonic, operand_text, cycles, size)
    - Define opcode to mnemonic mapping table
    - Define BIOS routine address to name mapping
    - _Requirements: 16.1, 16.3, 16.7_

  - [ ] 11.2 Implement disassembler class (disassembler.cpp)
    - Implement disassemble_instruction() for single instruction
    - Implement disassemble_range() for address range
    - Implement disassemble_rom() for entire ROM
    - Implement format_instruction() to produce assembly text
    - Implement identify_bios_call() to label known BIOS routines
    - _Requirements: 16.1, 16.2, 16.3, 16.7, 16.8_

  - [ ] 11.3 Implement instruction decoding
    - Decode all 96 Intel 8048 instructions
    - Handle 1-byte and 2-byte instructions
    - Extract operands (immediate values, register numbers, addresses)
    - _Requirements: 16.1, 16.3_

  - [ ] 11.4 Implement operand formatting
    - Format immediate values in hexadecimal with 0x prefix
    - Format register operands (R0-R7, A, etc.)
    - Format memory addresses in hexadecimal
    - Calculate and display branch target addresses
    - _Requirements: 16.4, 16.5, 16.6_

  - [ ] 11.5 Implement disassembly output formatting
    - Display address for each instruction
    - Display mnemonic and operands
    - Align output for readability
    - Support output format suitable for reassembly
    - _Requirements: 16.5, 16.9_

  - [ ] 11.6 Create standalone disassembler tool
    - Create command-line tool that loads ROM file
    - Support address range specification
    - Output disassembly to stdout or file
    - _Requirements: 16.2, 16.8, 16.10_

  - [ ] 11.7 Integrate disassembler into debugger
    - Add disassembly view to debugger interface
    - Show disassembly around current PC
    - Highlight current instruction
    - _Requirements: 16.10_

  - [ ]* 11.8 Write unit tests for disassembler
    - Test disassembly of specific instructions
    - Test operand formatting
    - Test branch target calculation
    - Test BIOS routine identification
    - _Requirements: 16.3, 16.4, 16.6, 16.7_

  - [ ]* 11.9 Write property test for disassembly correctness
    - **Property 33: Disassembly correctness**
    - **Validates: Requirements 16.1, 16.3**

  - [ ]* 11.10 Write property test for complete ROM disassembly
    - **Property 34: Complete ROM disassembly**
    - **Validates: Requirements 16.2**

  - [ ]* 11.11 Write property test for operand formatting
    - **Property 35: Operand formatting**
    - **Validates: Requirements 16.4**

  - [ ]* 11.12 Write property test for address display
    - **Property 36: Address display**
    - **Validates: Requirements 16.5**

  - [ ]* 11.13 Write property test for branch target calculation
    - **Property 37: Branch target calculation**
    - **Validates: Requirements 16.6**

  - [ ]* 11.14 Write property test for BIOS routine identification
    - **Property 38: BIOS routine identification**
    - **Validates: Requirements 16.7**

  - [ ]* 11.15 Write property test for disassembly round-trip
    - **Property 39: Disassembly round-trip**
    - **Validates: Requirements 16.9**


- [ ] 12. Implement debugging and development tools
  - [ ] 12.1 Create debugger interface (debugger.h)
    - Define breakpoint structure (address, enabled)
    - Define debugger state (paused, step mode, trace mode)
    - Define debugger commands (continue, step, break, inspect)
    - _Requirements: 15.1, 15.5, 15.6, 15.7_

  - [ ] 12.2 Implement debugger class (debugger.cpp)
    - Implement breakpoint management (add, remove, check)
    - Implement single-step execution
    - Implement instruction trace logging
    - Implement memory inspection
    - Implement VDC register inspection
    - Implement frame timing statistics (FPS, cycle count)
    - _Requirements: 15.1, 15.2, 15.3, 15.4, 15.5, 15.6, 15.7, 15.8_

  - [ ] 12.3 Integrate debugger into emulator core
    - Check breakpoints before each instruction
    - Pause execution when breakpoint is hit
    - Log instruction trace when trace mode is enabled
    - Display system state when paused
    - _Requirements: 15.5, 15.6, 15.7_

  - [ ] 12.4 Create debugger UI or command interface
    - Implement text-based debugger commands
    - Display CPU registers, memory, and VDC state
    - Show disassembly around current PC
    - Show frame timing statistics
    - _Requirements: 15.2, 15.3, 15.4, 15.8_

  - [ ]* 12.5 Write unit tests for debugger
    - Test breakpoint functionality
    - Test single-step execution
    - Test instruction trace logging
    - Test memory and register inspection
    - _Requirements: 15.1, 15.2, 15.3, 15.4, 15.5, 15.7_


- [ ] 13. Checkpoint - Debugging tools complete
  - Ensure debugger can set breakpoints and single-step
  - Verify instruction trace produces correct output
  - Verify memory and register inspection works
  - Test disassembler with real ROM files
  - Ask the user if questions arise

- [ ] 14. Implement frontend layer (standalone application)
  - [ ] 14.1 Create frontend interface (frontend.h)
    - Define Frontend abstract class with virtual methods
    - Define Configuration struct (video standard, display scale, audio settings, key mappings)
    - Define InputState struct for host input
    - Define MenuAction enum for user actions
    - _Requirements: 12.1, 12.2, 12.3, 12.4, 12.5, 13.1, 18.1, 18.2, 18.3, 18.4, 18.5_

  - [ ] 14.2 Implement SDL2-based frontend (frontend_sdl.cpp)
    - Initialize SDL2 for video, audio, and input
    - Create window with appropriate size and scaling
    - Set up OpenGL or SDL_Renderer for rendering
    - Set up SDL audio callback for audio output
    - _Requirements: 12.1, 12.2, 13.1_

  - [ ] 14.3 Implement video rendering
    - Convert VDC framebuffer (palette indices) to RGB
    - Scale 160x200 output to window size
    - Maintain correct aspect ratio
    - Support configurable display scaling
    - Render at correct frame rate (50Hz or 60Hz)
    - _Requirements: 12.1, 12.2, 12.3, 12.4, 12.5, 12.6, 12.7, 12.8, 12.9_

  - [ ] 14.4 Implement audio output
    - Set up audio callback to request samples from emulator
    - Convert VDC audio samples to host audio format
    - Support configurable sample rate (44100Hz or 48000Hz)
    - Apply master volume control
    - Synchronize audio with video frame timing
    - _Requirements: 13.1, 13.2, 13.3, 13.4, 13.5, 13.6_

  - [ ] 14.5 Implement input handling
    - Poll SDL keyboard and joystick events
    - Map SDL keys to Videopac keyboard matrix
    - Map SDL joystick/gamepad to Videopac joysticks
    - Support configurable key mappings
    - Pass input state to emulator core
    - _Requirements: 7.8, 7.9, 18.3_

  - [ ] 14.6 Implement configuration system
    - Load configuration from file (JSON or INI format)
    - Save configuration between sessions
    - Support PAL/NTSC selection
    - Support display scaling and filtering options
    - Support audio volume and sample rate
    - Support key mapping customization
    - _Requirements: 18.1, 18.2, 18.3, 18.4, 18.5, 18.6_

  - [ ] 14.7 Implement menu and UI
    - Create simple menu for ROM loading
    - Add controls for reset, pause, resume
    - Add save state and load state menu options
    - Display current ROM name and emulation status
    - Show FPS and performance statistics
    - _Requirements: 18.7, 18.8, 18.9_

  - [ ] 14.8 Implement ROM loading
    - Support command-line argument for ROM path
    - Support file dialog for ROM selection
    - Display error messages for invalid ROMs
    - _Requirements: 11.7, 11.8_

  - [ ]* 14.9 Write integration tests for frontend
    - Test video rendering with test patterns
    - Test audio output with test waveforms
    - Test input mapping
    - Test configuration loading and saving
    - _Requirements: 12.1, 13.1, 18.1, 18.6_


- [ ] 15. Implement libretro core for RetroArch integration
  - [ ] 15.1 Create libretro interface wrapper (libretro.cpp)
    - Implement retro_init() and retro_deinit()
    - Implement retro_api_version()
    - Implement retro_get_system_info() with core metadata
    - Implement retro_set_environment() callback
    - Implement retro_set_video_refresh() callback
    - Implement retro_set_audio_sample() callback
    - Implement retro_set_input_poll() callback
    - _Requirements: 17.1, 17.12_

  - [ ] 15.2 Implement libretro game loading
    - Implement retro_load_game() to load ROM
    - Support loading BIOS from RetroArch system directory
    - Validate ROM and BIOS files
    - _Requirements: 17.9_

  - [ ] 15.3 Implement libretro frame execution
    - Implement retro_run() to execute one frame
    - Call video refresh callback with framebuffer
    - Call audio sample callback with audio samples
    - Poll input via input callback
    - _Requirements: 17.1, 17.3, 17.4, 17.5_

  - [ ] 15.4 Implement libretro save states
    - Implement retro_serialize_size() to return state size
    - Implement retro_serialize() to save state to buffer
    - Implement retro_unserialize() to load state from buffer
    - _Requirements: 17.6_

  - [ ] 15.5 Implement libretro core options
    - Define core option for PAL/NTSC selection
    - Define core options for display settings
    - Define core options for audio settings
    - Expose options through retro_set_environment()
    - _Requirements: 17.7, 17.8_

  - [ ] 15.6 Configure libretro build
    - Create Makefile or CMake configuration for shared library
    - Support building for multiple platforms (Linux, Windows, macOS)
    - Support cross-compilation for Android (ARM/ARM64)
    - Support cross-compilation for iOS (ARM64)
    - Handle platform-specific requirements
    - _Requirements: 17.2, 17.10, 17.11_

  - [ ] 15.7 Create libretro core info file
    - Create videopac_libretro.info with core metadata
    - Specify supported file extensions (.bin, .rom)
    - Specify core name, version, and author
    - Specify supported features (save states, core options)
    - _Requirements: 17.12_

  - [ ]* 15.8 Write integration tests for libretro core
    - Test core initialization and deinitialization
    - Test ROM loading through libretro API
    - Test frame execution and callbacks
    - Test save state serialization
    - Test core options
    - _Requirements: 17.1, 17.3, 17.4, 17.5, 17.6, 17.7, 17.9_


- [ ] 16. Final integration and testing
  - [ ] 16.1 Create test ROM suite
    - Gather or create test ROMs for instruction validation
    - Create test ROMs for graphics rendering
    - Create test ROMs for audio generation
    - Create test ROMs for input handling
    - Document expected behavior for each test ROM
    - _Requirements: All (validation)_

  - [ ] 16.2 Implement regression testing
    - Load each test ROM and execute for fixed number of frames
    - Compare framebuffer output against reference images
    - Compare audio output against reference waveforms
    - Verify memory and register states
    - _Requirements: All (validation)_

  - [ ] 16.3 Test with commercial ROMs
    - Test with known Videopac/Odyssey2 game ROMs
    - Verify games load and run correctly
    - Verify graphics render correctly
    - Verify audio plays correctly
    - Verify input controls work correctly
    - _Requirements: All (validation)_

  - [ ] 16.4 Performance optimization
    - Profile emulator to identify bottlenecks
    - Optimize CPU instruction execution
    - Optimize VDC rendering pipeline
    - Optimize memory access patterns
    - Ensure emulator runs at full speed on target platforms
    - _Requirements: 8.10_

  - [ ] 16.5 Documentation
    - Write README with build instructions
    - Document configuration options
    - Document key mappings
    - Document debugger commands
    - Document libretro core usage
    - Create user guide for standalone application
    - _Requirements: All (usability)_

  - [ ] 16.6 Platform testing
    - Test standalone application on Linux, Windows, macOS
    - Test libretro core with RetroArch on desktop platforms
    - Test libretro core on Android devices
    - Test libretro core on iOS devices (if possible)
    - Fix platform-specific issues
    - _Requirements: 17.10, 17.11_

  - [ ]* 16.7 Write end-to-end integration tests
    - Test complete emulation workflow (load BIOS, load ROM, run, save state, load state)
    - Test all components working together
    - Test error handling across components
    - _Requirements: All (integration)_


- [ ] 17. Final checkpoint - Complete emulator
  - Ensure all unit tests pass
  - Ensure all property-based tests pass (minimum 100 iterations each)
  - Verify emulator runs commercial ROMs correctly
  - Verify standalone application works on all target platforms
  - Verify libretro core works with RetroArch
  - Verify save states work correctly
  - Verify debugger and disassembler work correctly
  - Ask the user if questions arise

## Notes

### Testing Framework Configuration

- **Unit Tests**: Google Test (gtest) framework
  - Install: `sudo apt-get install libgtest-dev` (Linux) or build from source
  - Link against: `gtest`, `gtest_main`, `pthread`
  
- **Property-Based Tests**: RapidCheck framework
  - Install: Clone from https://github.com/emil-e/rapidcheck and build
  - Link against: `rapidcheck`, `rapidcheck_gtest`
  - Configure each property test to run minimum 100 iterations
  - Tag format: `// Feature: videopac-emulator, Property N: [property text]`

### Build System

- **CMake** configuration with the following targets:
  - `videopac_core`: Static library with all emulator components
  - `videopac`: Standalone executable with SDL2 frontend
  - `videopac_libretro`: Shared library for RetroArch
  - `videopac_tests`: Test executable with all unit and property tests
  - `videopac_disasm`: Standalone disassembler tool

### Dependencies

- **Core**: C++17 or later, STL
- **Standalone**: SDL2 (video, audio, input)
- **LibRetro**: libretro.h header (included in project)
- **Testing**: Google Test, RapidCheck
- **Optional**: OpenGL for hardware-accelerated rendering

### Task Execution Guidelines

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Checkpoints ensure incremental validation
- Property tests validate universal correctness properties (39 total)
- Unit tests validate specific examples and edge cases
- Integration tests validate components working together

### Property-Based Test Implementation

Each of the 39 correctness properties should be implemented as a separate property-based test:
- Use RapidCheck generators to create random valid inputs
- Run minimum 100 iterations per test
- Include comment tag referencing the design property
- Example:
  ```cpp
  // Feature: videopac-emulator, Property 2: Internal RAM persistence
  TEST(CPUPropertyTests, InternalRAMPersistence) {
      rc::check([](uint8_t address, uint8_t value) {
          RC_PRE(address <= 0x3F);  // Constrain to valid range
          CPU cpu;
          cpu.write_internal_ram(address, value);
          uint8_t read_value = cpu.read_internal_ram(address);
          RC_ASSERT(read_value == value);
      });
  }
  ```

### Implementation Order Rationale

The task order follows a bottom-up approach:
1. **Foundation** (Tasks 1-2): Build system and core types
2. **Core Components** (Tasks 3-7): CPU, Memory, VDC, Input (independently testable)
3. **Integration** (Task 8): Emulator core orchestration
4. **State Management** (Task 10): Save states
5. **Tools** (Tasks 11-12): Disassembler and debugger
6. **Frontend** (Tasks 14-15): Standalone app and libretro core
7. **Validation** (Task 16): Testing and optimization

This order ensures each component can be tested independently before integration, catching errors early and enabling incremental progress.
