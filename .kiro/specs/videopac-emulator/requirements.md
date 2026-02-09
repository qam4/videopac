# Requirements Document: Philips Videopac (Odyssey2) Emulator

## Introduction

This document specifies the requirements for building a software emulator of the Philips Videopac (known as Magnavox Odyssey2 in North America), a home video game console released in 1978. The emulator will accurately reproduce the behavior of the original hardware, including the Intel 8048 microcontroller, the Intel 8245 Video Display Controller (VDC), memory systems, input devices, and audio output.

The emulator will enable users to load and play original ROM files, providing an authentic gaming experience while preserving this piece of computing history.

## Glossary

- **Emulator**: The complete software system that simulates the Videopac hardware
- **CPU**: The Intel 8048 microcontroller that executes game code
- **VDC**: Video Display Controller (Intel 8245) responsible for graphics and sound generation
- **ROM**: Read-Only Memory containing the system BIOS or game cartridge data
- **RAM**: Random Access Memory for temporary data storage
- **BIOS**: Basic Input/Output System stored in the CPU's internal 1KB ROM
- **PAL**: Phase Alternating Line video standard (50Hz, used in Europe)
- **NTSC**: National Television System Committee video standard (60Hz, used in North America)
- **Cartridge**: Game software stored in ROM format, typically 2KB to 8KB
- **Frame**: A complete video display update (262 scanlines at ~60Hz)
- **Scanline**: A single horizontal line of video output
- **VBLANK**: Vertical blanking period between frames when no video is drawn
- **HBLANK**: Horizontal blanking period between scanlines
- **Sprite**: A movable 8x8 pixel graphical object
- **Grid**: The background grid system with 8 rows and 9 columns
- **Character**: A text or graphical symbol from the built-in character set
- **Collision**: Detection of overlapping graphical objects
- **Machine_Cycle**: A single CPU instruction cycle (2.5 microseconds)
- **Host_System**: The modern computer running the emulator
- **Disassembler**: A tool that converts machine code to assembly language
- **libretro**: A standardized API for emulator cores used by RetroArch
- **RetroArch**: A cross-platform frontend for emulators and game engines

## Requirements

### Requirement 1: CPU Emulation

**User Story:** As a developer, I want accurate Intel 8048 CPU emulation, so that game code executes correctly.

#### Acceptance Criteria

1. THE CPU SHALL execute all 96 Intel 8048 instructions according to the official instruction set
2. WHEN the CPU executes an instruction, THE CPU SHALL update the program counter, accumulator, registers, and flags correctly
3. THE CPU SHALL maintain 64 bytes of internal RAM at addresses 0x00-0x3F
4. THE CPU SHALL provide 8 general-purpose registers (R0-R7) and 8 alternative registers (R0'-R7')
5. THE CPU SHALL implement the Program Status Word (PSW) with carry flag, auxiliary carry, and user flags F0/F1
6. THE CPU SHALL support two 8-bit I/O ports (Port 1 and Port 2) with configurable direction
7. THE CPU SHALL support configurable clock speeds for PAL (5.91 MHz) and NTSC (5.37 MHz) systems
8. WHEN the CPU clock runs, THE CPU SHALL divide the clock by 15 to produce the instruction cycle clock
9. THE CPU SHALL support single-cycle and dual-cycle instructions
10. THE CPU SHALL implement an 8-level stack for subroutine calls and interrupts
11. THE CPU SHALL load and execute the 1KB BIOS ROM from addresses 0x000-0x3FF

### Requirement 2: Memory System

**User Story:** As a developer, I want proper memory management, so that the emulator correctly handles ROM, RAM, and memory-mapped I/O.

#### Acceptance Criteria

1. THE Emulator SHALL load BIOS ROM into the CPU's internal 1KB program memory
2. THE Emulator SHALL support cartridge ROM sizes of 2KB, 4KB, and 8KB
3. WHEN a cartridge exceeds 2KB, THE Emulator SHALL implement bank switching via Port 1 pins P10 and P11
4. THE Emulator SHALL provide 128 bytes of external RAM at addresses 0x00-0x7F
5. WHEN the CPU accesses external RAM, THE Emulator SHALL enable it via Port 1 pin P14
6. WHEN the CPU accesses the VDC, THE Emulator SHALL enable it via Port 1 pin P13
7. THE Emulator SHALL implement the copy mode where P16=1, P13=0, and P14=0 for RAM-to-VDC transfers
8. THE Emulator SHALL map cartridge ROM starting at address 0x400, skipping address line A10

### Requirement 3: Video Display Controller (VDC)

**User Story:** As a developer, I want accurate VDC emulation, so that graphics render correctly.

#### Acceptance Criteria

1. THE VDC SHALL generate video output at 160x200 resolution
2. THE VDC SHALL support both PAL (50Hz, 312 scanlines) and NTSC (60Hz, 262 scanlines) video standards
3. WHEN operating in NTSC mode, THE VDC SHALL produce a vertical blank period of 22 scanlines
4. WHEN operating in PAL mode, THE VDC SHALL produce a vertical blank period of 28 scanlines
5. THE VDC SHALL implement a horizontal blank period of 12 microseconds per scanline
6. THE VDC SHALL provide 256 memory-mapped registers at addresses 0x00-0xFF
7. WHEN the VDC control register (0xA0) bit 5 is set, THE VDC SHALL enable foreground object rendering
8. WHEN the VDC control register bit 3 is set, THE VDC SHALL enable background grid rendering
9. THE VDC SHALL prevent modification of graphics objects while they are enabled
10. THE VDC SHALL update the status register (0xA1) to reflect VBLANK, HBLANK, and collision states
11. THE VDC SHALL provide X and Y beam position registers (0xA4, 0xA5) that track the electron beam

### Requirement 4: Graphics Objects

**User Story:** As a user, I want to see sprites, characters, and background graphics, so that games display correctly.

#### Acceptance Criteria

1. THE VDC SHALL render 4 independent sprites, each 8x8 pixels
2. WHEN a sprite control register is configured, THE VDC SHALL position the sprite at the specified X,Y coordinates
3. THE VDC SHALL support sprite colors from a palette of 8 colors
4. THE VDC SHALL support double-size sprites when bit 2 of sprite control byte 2 is set
5. THE VDC SHALL render up to 12 single characters from a 64-character built-in set
6. THE VDC SHALL render up to 4 quad character groups (16 characters total)
7. THE VDC SHALL render a background grid with 8 rows and 9 columns
8. WHEN grid fill mode is enabled (bit 7 of 0xA0), THE VDC SHALL fill boxes adjacent to vertical grid lines
9. THE VDC SHALL render a dot grid when bit 6 of control register 0xA0 is set
10. THE VDC SHALL apply colors from the color register (0xA3) to background and grid elements

### Requirement 5: Collision Detection

**User Story:** As a developer, I want collision detection between graphics objects, so that game logic can respond to object interactions.

#### Acceptance Criteria

1. THE VDC SHALL detect collisions between sprites, characters, and grid elements
2. WHEN the collision register (0xA2) is written with object enable bits, THE VDC SHALL monitor those objects for collisions
3. WHEN a collision occurs, THE VDC SHALL set the corresponding bits in the collision register
4. THE VDC SHALL update collision information only during VBLANK
5. THE VDC SHALL detect collisions between: sprites 0-3, vertical grid, horizontal grid, dots, and characters
6. WHEN character objects overlap, THE VDC SHALL set bit 7 of the status register (0xA1)

### Requirement 6: Audio System

**User Story:** As a user, I want to hear game audio, so that the gaming experience is complete.

#### Acceptance Criteria

1. THE VDC SHALL generate audio using a 24-bit shift register
2. WHEN sound registers (0xA7-0xA9) are loaded, THE VDC SHALL output the bit pattern as audio
3. THE VDC SHALL support two shift frequencies: 983Hz and 3933Hz
4. WHEN bit 5 of sound control register (0xAA) is set to 0, THE VDC SHALL use 983Hz frequency
5. WHEN bit 5 of sound control register is set to 1, THE VDC SHALL use 3933Hz frequency
6. THE VDC SHALL support noise generation when bit 4 of register 0xAA is set
7. WHEN bit 6 of register 0xAA is set, THE VDC SHALL loop the shift register continuously
8. WHEN bit 6 is cleared, THE VDC SHALL play the shift register once and stop
9. THE VDC SHALL control volume via bits 0-3 of register 0xAA
10. WHEN bit 7 of register 0xAA is set, THE VDC SHALL enable audio output

### Requirement 7: Input Handling

**User Story:** As a user, I want to use keyboard and joystick controls, so that I can play games.

#### Acceptance Criteria

1. THE Emulator SHALL emulate the keyboard matrix with 8 rows and 8 columns
2. WHEN a key is pressed, THE Emulator SHALL set the appropriate row and column bits
3. THE Emulator SHALL read keyboard input via Port 2 when P12 is set low
4. THE Emulator SHALL support two joystick controllers
5. WHEN Port 2 bits P20-P22 are set to 0, THE Emulator SHALL enable joystick 2 onto the data bus
6. WHEN Port 2 bits P20-P22 are set to 1, THE Emulator SHALL enable joystick 1 onto the data bus
7. THE Emulator SHALL read 5 bits per joystick: up, down, left, right, and fire button
8. THE Emulator SHALL map Host_System keyboard keys to Videopac keyboard keys
9. THE Emulator SHALL map Host_System input devices to joystick directions and buttons

### Requirement 8: Timing and Synchronization

**User Story:** As a developer, I want accurate timing, so that games run at the correct speed.

#### Acceptance Criteria

1. THE Emulator SHALL execute CPU instructions at the correct speed based on the selected video standard
2. WHEN operating in NTSC mode, THE Emulator SHALL generate video frames at approximately 60Hz
3. WHEN operating in PAL mode, THE Emulator SHALL generate video frames at approximately 50Hz
4. THE Emulator SHALL execute the appropriate number of machine cycles per frame based on video standard
5. THE Emulator SHALL execute the appropriate number of machine cycles per scanline
6. WHEN the timer/counter is started, THE Emulator SHALL increment it at the appropriate rate
7. THE Emulator SHALL generate timer interrupts when the counter overflows from 0xFF to 0x00
8. THE Emulator SHALL support horizontal line interrupts when enabled via bit 0 of register 0xA0
9. WHEN a line interrupt occurs, THE Emulator SHALL increment the internal counter and trigger an interrupt on overflow
10. THE Emulator SHALL synchronize emulation speed with the Host_System's real-time clock

### Requirement 9: Interrupt System

**User Story:** As a developer, I want proper interrupt handling, so that time-critical game code executes correctly.

#### Acceptance Criteria

1. THE CPU SHALL support external interrupts via the INT pin
2. WHEN an external interrupt occurs and interrupts are enabled, THE CPU SHALL jump to cartridge vector 0x402
3. THE CPU SHALL support timer/counter interrupts
4. WHEN a timer interrupt occurs and timer interrupts are enabled, THE CPU SHALL jump to cartridge vector 0x404
5. THE CPU SHALL support VBLANK interrupts via cartridge vector 0x406
6. THE CPU SHALL support horizontal line interrupts when enabled
7. WHEN an interrupt is serviced, THE CPU SHALL push the program counter and PSW onto the stack
8. WHEN a RETR instruction executes, THE CPU SHALL restore the program counter and PSW from the stack

### Requirement 10: BIOS Integration

**User Story:** As a developer, I want BIOS functionality, so that cartridges can use system services.

#### Acceptance Criteria

1. THE Emulator SHALL load the 1KB BIOS ROM at startup
2. WHEN the system resets, THE CPU SHALL jump to cartridge vector 0x400
3. THE BIOS SHALL provide routines for enabling the VDC (address 0xE7)
4. THE BIOS SHALL provide routines for enabling external RAM (address 0xEC)
5. THE BIOS SHALL provide routines for reading joysticks (address 0x38F)
6. THE BIOS SHALL provide routines for turning the display on (0x127) and off (0x11C)
7. THE BIOS SHALL implement the "Select Game" routine at address 0x2C3
8. THE Emulator SHALL allow cartridges to call BIOS routines via standard subroutine calls

### Requirement 11: ROM Loading and Cartridge Support

**User Story:** As a user, I want to load game ROMs, so that I can play different games.

#### Acceptance Criteria

1. THE Emulator SHALL load ROM files in binary format
2. THE Emulator SHALL detect ROM size automatically (2KB, 4KB, or 8KB)
3. WHEN a ROM is larger than 2KB, THE Emulator SHALL implement bank switching
4. THE Emulator SHALL support 2-bank cartridges using P10 for bank selection
5. THE Emulator SHALL support 4-bank cartridges using P10 and P11 for bank selection
6. THE Emulator SHALL validate that ROM files are not corrupted before loading
7. WHEN a ROM file cannot be loaded, THE Emulator SHALL display an error message
8. THE Emulator SHALL support loading ROMs via command-line argument or file dialog

### Requirement 12: Display Output

**User Story:** As a user, I want to see the game display, so that I can play games visually.

#### Acceptance Criteria

1. THE Emulator SHALL render the VDC output to a Host_System window
2. THE Emulator SHALL scale the 160x200 output to a reasonable window size
3. THE Emulator SHALL maintain the correct aspect ratio when scaling
4. WHEN operating in NTSC mode, THE Emulator SHALL render at 60 frames per second
5. WHEN operating in PAL mode, THE Emulator SHALL render at 50 frames per second
6. THE Emulator SHALL implement the 8-color palette accurately
7. THE Emulator SHALL support both bright and dim luminance levels for grid colors
8. WHEN the VDC is disabled, THE Emulator SHALL display a blank screen
9. THE Emulator SHALL render sprites, characters, grid, and background in the correct priority order

### Requirement 13: Audio Output

**User Story:** As a user, I want to hear game audio, so that the experience matches the original hardware.

#### Acceptance Criteria

1. THE Emulator SHALL output audio to the Host_System's audio device
2. THE Emulator SHALL generate audio waveforms based on the VDC's 24-bit shift register
3. THE Emulator SHALL implement both 983Hz and 3933Hz shift frequencies
4. THE Emulator SHALL implement noise generation when enabled
5. THE Emulator SHALL control audio volume based on the volume bits in register 0xAA
6. THE Emulator SHALL synchronize audio output with video frame timing
7. WHEN audio is disabled via bit 7 of register 0xAA, THE Emulator SHALL mute output

### Requirement 14: State Management

**User Story:** As a user, I want to save and load emulator state, so that I can pause and resume games.

#### Acceptance Criteria

1. THE Emulator SHALL capture the complete system state including CPU, VDC, and memory
2. WHEN the user requests a save state, THE Emulator SHALL serialize the state to a file
3. WHEN the user loads a save state, THE Emulator SHALL restore the exact system state
4. THE Emulator SHALL support multiple save state slots
5. THE Emulator SHALL validate save state files before loading
6. WHEN a save state is incompatible, THE Emulator SHALL display an error message

### Requirement 15: Debugging and Development Tools

**User Story:** As a developer, I want debugging tools, so that I can understand emulator behavior and troubleshoot issues.

#### Acceptance Criteria

1. THE Emulator SHALL provide a CPU instruction trace mode
2. WHEN trace mode is enabled, THE Emulator SHALL log each instruction executed with register states
3. THE Emulator SHALL provide memory inspection capabilities
4. THE Emulator SHALL provide VDC register inspection capabilities
5. THE Emulator SHALL support breakpoints at specific program counter addresses
6. WHEN a breakpoint is hit, THE Emulator SHALL pause execution and display system state
7. THE Emulator SHALL support single-step execution
8. THE Emulator SHALL display frame timing statistics (FPS, cycle count)

### Requirement 16: Disassembler

**User Story:** As a developer, I want to disassemble Intel 8048 machine code, so that I can analyze ROM contents and understand game logic.

#### Acceptance Criteria

1. THE Disassembler SHALL convert Intel 8048 machine code to human-readable assembly language
2. WHEN given a ROM file, THE Disassembler SHALL produce a complete disassembly listing
3. THE Disassembler SHALL display instruction mnemonics according to the official Intel 8048 instruction set
4. THE Disassembler SHALL show operands in hexadecimal format with appropriate prefixes
5. THE Disassembler SHALL display memory addresses for each instruction
6. THE Disassembler SHALL calculate and display branch target addresses for jump and call instructions
7. THE Disassembler SHALL identify and label BIOS routine calls
8. THE Disassembler SHALL support disassembly of specific address ranges
9. THE Disassembler SHALL output disassembly in a format suitable for reassembly
10. THE Disassembler SHALL be accessible both as a standalone tool and integrated into the emulator debugger

### Requirement 17: RetroArch Integration

**User Story:** As a user, I want to use the emulator as a RetroArch core, so that I can play games on multiple platforms including Android and iOS.

#### Acceptance Criteria

1. THE Emulator SHALL implement the libretro API specification
2. THE Emulator SHALL compile as a shared library compatible with RetroArch
3. THE Emulator SHALL expose video output through the libretro video callback
4. THE Emulator SHALL expose audio output through the libretro audio callback
5. THE Emulator SHALL handle input through the libretro input polling interface
6. THE Emulator SHALL support RetroArch's save state functionality via libretro API
7. THE Emulator SHALL provide core options for PAL/NTSC selection through libretro
8. THE Emulator SHALL provide core options for display and audio settings through libretro
9. THE Emulator SHALL support loading ROMs through RetroArch's content loading system
10. THE Emulator SHALL compile for Android (ARM/ARM64) and iOS (ARM64) architectures
11. THE Emulator SHALL handle platform-specific requirements for mobile devices
12. THE Emulator SHALL provide appropriate core information metadata for RetroArch

### Requirement 18: Configuration and User Interface

**User Story:** As a user, I want to configure the emulator, so that I can customize the experience.

#### Acceptance Criteria

1. THE Emulator SHALL provide a configuration file or settings interface
2. THE Emulator SHALL allow users to select between PAL and NTSC video standards
3. THE Emulator SHALL allow users to configure keyboard mappings
4. THE Emulator SHALL allow users to configure display scaling and filtering options
5. THE Emulator SHALL allow users to configure audio volume and sample rate
6. THE Emulator SHALL save configuration settings between sessions
7. THE Emulator SHALL provide a menu or interface for loading ROMs
8. THE Emulator SHALL provide controls for reset, pause, and resume
9. THE Emulator SHALL display the current ROM name and emulation status
