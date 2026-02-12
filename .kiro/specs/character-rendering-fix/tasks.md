# Implementation Plan: Character Rendering Fix

## Overview

This implementation plan addresses the character rendering bug by implementing a master clock synchronization architecture. The current time-slice approach (CPU executes for a scanline, then VDC renders) causes timing mismatches. The solution implements cycle-accurate emulation where CPU and VDC execute concurrently based on a unified master clock (VDC 3.54 MHz), with the VDC rendering continuously as the beam position advances.

The implementation follows this sequence:
1. Create the MasterClock class for cycle-accurate synchronization
2. Modify VDC to support continuous rendering instead of scanline-based rendering
3. Update EmulatorCore to use the master clock for coordinated execution
4. Implement property-based tests for all 21 correctness properties
5. Integration testing with Satellite Attack game
6. Update documentation

## Tasks

- [x] 1. Set up property-based testing framework
  - Install and configure RapidCheck library for C++ property-based testing
  - Add RapidCheck to CMakeLists.txt build configuration
  - Create test file structure for property-based tests
  - _Requirements: 13.5_

- [x] 2. Implement MasterClock class
  - [x] 2.1 Create MasterClock header and implementation files
    - Define MasterClock class in include/master_clock.h
    - Implement clock frequency constants (VDC: 3.54 MHz, CPU: ~358 kHz)
    - Implement cycle debt tracking (cpu_cycle_debt, vdc_cycle_debt)
    - Implement tick() method to determine which component executes next
    - Implement cpu_executed() and vdc_ticked() notification methods
    - Implement frame completion detection and reset
    - _Requirements: 1.1, 1.2, 1.3, 1.6_
  
  - [x]* 2.2 Write property test for master clock cycle ratio
    - **Property 1: Master clock cycle ratio**
    - **Validates: Requirements 1.2**
  
  - [x]* 2.3 Write property test for cycle debt execution order
    - **Property 2: Cycle debt determines execution order**
    - **Validates: Requirements 1.3**
  
  - [x]* 2.4 Write unit tests for MasterClock
    - Test specific cycle counts and debt calculations
    - Test frame boundary detection
    - Test reset behavior
    - _Requirements: 1.1, 1.2, 1.3, 1.6_

- [x] 3. Modify VDC for continuous rendering
  - [x] 3.1 Update VDCState structure
    - Replace scanline field with beam_y (vertical beam position)
    - Add beam_x field (horizontal beam position)
    - Add total_cycles field for cycle tracking
    - Remove cycle_counter field (replaced by beam_x calculation)
    - Update state serialization/deserialization
    - _Requirements: 2.4_
  
  - [x] 3.2 Implement tick_one_cycle() method
    - Advance total_cycles by 1
    - Calculate beam_x and beam_y from total_cycles
    - Handle HBLANK and VBLANK transitions
    - Call render_current_pixel() if beam is in visible area
    - _Requirements: 2.1, 2.3, 2.4_
  
  - [x] 3.3 Implement render_current_pixel() method
    - Check if beam position is in visible area
    - Render background pixel at current beam position
    - Render grid pixel if enabled
    - Render character pixel if enabled and visible
    - Render sprite pixel if visible
    - Handle collision detection at current pixel
    - _Requirements: 2.1, 2.3, 2.6_
  
  - [x] 3.4 Implement beam position helper methods
    - Implement is_beam_visible() to check if beam is in visible area
    - Implement get_beam_x() and get_beam_y() accessors
    - Implement is_hblank() and is_vblank() based on beam position
    - _Requirements: 2.4, 2.6_
  
  - [ ]* 3.5 Write property test for beam position advancement
    - **Property 4: Beam position advances with VDC cycles**
    - **Validates: Requirements 2.3, 2.4**
  
  - [ ]* 3.6 Write property test for beam position calculation
    - **Property 5: Beam position calculation from cycle count**
    - **Validates: Requirements 2.4**
  
  - [ ]* 3.7 Write property test for blanking period rendering
    - **Property 6: No rendering during blanking**
    - **Validates: Requirements 2.6**
  
  - [ ]* 3.8 Write unit tests for VDC continuous rendering
    - Test specific beam positions map to expected framebuffer coordinates
    - Test HBLANK and VBLANK detection
    - Test frame boundary transitions
    - _Requirements: 2.1, 2.3, 2.4, 2.5, 2.6_

- [x] 4. Implement character rendering with immediate register effects
  - [x] 4.1 Modify write_register() for immediate effect
    - Update display_enabled flag immediately when register 0xA0 bit 5 changes
    - Update grid_enabled flag immediately when register 0xA0 bit 3 changes
    - Ensure character control register writes take effect at current beam position
    - _Requirements: 1.7, 3.3_
  
  - [x] 4.2 Implement character visibility and bounds checking
    - Check if character Y position is within visible range [0, 191]
    - Check if character X position is within visible range [0, 159]
    - Skip rendering for characters outside visible area
    - Implement per-pixel bounds checking before framebuffer writes
    - _Requirements: 4.1, 4.2, 4.3, 11.4_
  
  - [x] 4.3 Implement character position extraction
    - Extract single character positions from registers 0x10-0x3F
    - Extract quad character base positions from registers 0x40-0x7F
    - Calculate quad sub-character positions with 8-pixel spacing
    - _Requirements: 4.4, 4.5, 9.4, 9.5_
  
  - [ ]* 4.4 Write property test for immediate register effect
    - **Property 3: Immediate register effect**
    - **Validates: Requirements 1.7**
  
  - [ ]* 4.5 Write property test for display enable control
    - **Property 7: Display enable controls character rendering**
    - **Validates: Requirements 3.1, 3.2, 3.3**
  
  - [ ]* 4.6 Write property test for character position bounds checking
    - **Property 8: Character position bounds checking**
    - **Validates: Requirements 4.1, 4.2, 4.3, 11.1, 11.2**
  
  - [ ]* 4.7 Write property test for single character position extraction
    - **Property 9: Single character position extraction**
    - **Validates: Requirements 4.4**
  
  - [ ]* 4.8 Write property test for quad character position calculation
    - **Property 10: Quad character position calculation**
    - **Validates: Requirements 4.5, 9.4, 9.5**
  
  - [ ]* 4.9 Write unit tests for character position handling
    - Test specific character positions at boundaries
    - Test edge cases (position 0, position 159/191)
    - Test out-of-bounds positions
    - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5_

- [x] 5. Checkpoint - Ensure basic rendering infrastructure works
  - Ensure all tests pass, ask the user if questions arise.

- [x] 6. Implement character ROM address calculation and pattern rendering
  - [x] 6.1 Implement ROM address calculation
    - Calculate char_row as (scanline - char_y) / 2
    - Calculate ROM address as (char_ptr + (char_y / 2) + char_row) & 0x1FF
    - Validate ROM address is < 512, skip character if invalid
    - Extract 9-bit character pointer from bytes 2 and 3
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5_
  
  - [x] 6.2 Implement pattern byte rendering
    - Fetch pattern byte from character_rom_ at calculated ROM address
    - Extract pixel bits from pattern byte (bit 7 = leftmost, bit 0 = rightmost)
    - Draw pixel if pattern bit is 1, skip if 0 (transparent)
    - Render 8 pixels horizontally per scanline
    - Render each character row across 2 scanlines (double-height)
    - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5, 6.6_
  
  - [x] 6.3 Implement color attribute extraction
    - Extract color from bits 1-3 of character attribute byte (byte 3)
    - Shift right by 1 to get 3-bit color index (0-7)
    - Write color value to framebuffer at pixel position
    - _Requirements: 7.1, 7.2, 7.3_
  
  - [ ]* 6.4 Write property test for ROM address calculation
    - **Property 11: ROM address calculation**
    - **Validates: Requirements 5.1, 5.4, 5.5**
  
  - [ ]* 6.5 Write property test for ROM address bounds validation
    - **Property 12: ROM address bounds validation**
    - **Validates: Requirements 5.2, 11.3**
  
  - [ ]* 6.6 Write property test for character pointer extraction
    - **Property 13: Character pointer extraction**
    - **Validates: Requirements 5.3**
  
  - [ ]* 6.7 Write property test for pattern byte pixel rendering
    - **Property 14: Pattern byte pixel rendering**
    - **Validates: Requirements 6.2, 6.3, 6.4**
  
  - [ ]* 6.8 Write property test for character width and height
    - **Property 15: Character width and height**
    - **Validates: Requirements 6.5, 6.6**
  
  - [ ]* 6.9 Write property test for color attribute extraction
    - **Property 16: Color attribute extraction**
    - **Validates: Requirements 7.1, 7.2**
  
  - [ ]* 6.10 Write unit tests for ROM address calculation
    - Test specific char_ptr, char_y, char_row combinations
    - Test ROM address wrapping at 512 bytes
    - Test invalid ROM addresses (>= 512)
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5_
  
  - [ ]* 6.11 Write unit tests for pattern rendering
    - Test specific pattern bytes render correct pixels
    - Test transparent pixels (bit 0) are not drawn
    - Test opaque pixels (bit 1) are drawn with correct color
    - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5, 6.6_

- [x] 7. Implement single and quad character support
  - [x] 7.1 Implement single character rendering
    - Support rendering up to 12 single characters simultaneously
    - Read control data from registers 0x10-0x3F (4 bytes per character)
    - Process characters 0-11 in order
    - Use independent position, pattern pointer, and color for each
    - _Requirements: 8.1, 8.2, 8.3, 8.4_
  
  - [x] 7.2 Implement quad character rendering
    - Support rendering up to 4 quad character groups simultaneously
    - Read control data from registers 0x40-0x7F (16 bytes per quad)
    - Render all 4 sub-characters with 8-pixel horizontal spacing
    - Calculate sub-character X positions as quad_x + (sub_char_index * 8)
    - Use byte 13 as base X position for entire quad
    - _Requirements: 9.1, 9.2, 9.3, 9.4, 9.5_
  
  - [ ]* 7.3 Write property test for single character capacity
    - **Property 17: Single character capacity**
    - **Validates: Requirements 8.1, 8.2**
  
  - [ ]* 7.4 Write property test for quad character capacity
    - **Property 18: Quad character capacity**
    - **Validates: Requirements 9.1, 9.2**
  
  - [ ]* 7.5 Write property test for quad character spacing
    - **Property 19: Quad character spacing**
    - **Validates: Requirements 9.3**
  
  - [ ]* 7.6 Write unit tests for single character rendering
    - Test rendering 1, 6, and 12 single characters
    - Test character independence (changing one doesn't affect others)
    - _Requirements: 8.1, 8.2, 8.3, 8.4_
  
  - [ ]* 7.7 Write unit tests for quad character rendering
    - Test rendering 1, 2, and 4 quad groups
    - Test 8-pixel spacing between sub-characters
    - Test base X position calculation
    - _Requirements: 9.1, 9.2, 9.3, 9.4, 9.5_

- [x] 8. Implement character ROM and rendering priority
  - [x] 8.1 Verify character ROM data integrity
    - Ensure character_rom_ contains 64 character patterns
    - Verify 8 bytes per character (512 bytes total)
    - Validate Intel 8245 character pattern data is loaded
    - Add bounds checking for ROM address access
    - _Requirements: 10.1, 10.2, 10.3, 10.4_
  
  - [x] 8.2 Implement rendering layer priority
    - Render characters after grid layer
    - Render characters before sprite layer
    - Ensure character pixels are visible over background/grid
    - Ensure sprite pixels are visible over characters
    - _Requirements: 12.1, 12.2, 12.3, 12.4_
  
  - [ ]* 8.3 Write property test for character ROM structure
    - **Property 20: Character ROM structure**
    - **Validates: Requirements 10.2, 10.4**
  
  - [ ]* 8.4 Write property test for rendering layer priority
    - **Property 21: Rendering layer priority**
    - **Validates: Requirements 12.1, 12.2, 12.3, 12.4**
  
  - [ ]* 8.5 Write unit tests for character ROM
    - Test ROM contains valid data at all addresses 0-511
    - Test ROM address bounds checking
    - Test specific character patterns match expected shapes
    - _Requirements: 10.1, 10.2, 10.3, 10.4_
  
  - [ ]* 8.6 Write unit tests for rendering priority
    - Test character pixels override grid pixels
    - Test sprite pixels override character pixels
    - Test layering with all layers enabled
    - _Requirements: 12.1, 12.2, 12.3, 12.4_

- [x] 9. Checkpoint - Ensure character rendering is complete
  - Ensure all tests pass, ask the user if questions arise.

- [x] 10. Update EmulatorCore to use master clock
  - [x] 10.1 Add MasterClock member to EmulatorCore
    - Add master_clock_ member variable
    - Initialize master clock in constructor with video standard
    - Update reset() to reset master clock
    - _Requirements: 1.1, 1.2, 1.3_
  
  - [x] 10.2 Rewrite run_frame() with cycle-accurate loop
    - Replace scanline-based loop with master clock loop
    - Use master_clock_.tick() to determine CPU or VDC execution
    - Execute one CPU instruction when CPU debt threshold reached
    - Advance VDC by one cycle when VDC should execute
    - Notify master clock after CPU instruction execution
    - Continue until frame is complete
    - _Requirements: 1.3, 1.4, 1.5, 1.6_
  
  - [x] 10.3 Update interrupt handling for new timing
    - Check for VBLANK interrupt based on VDC beam position
    - Trigger CPU interrupt when VBLANK starts
    - Update vblank_interrupt_triggered_ flag handling
    - _Requirements: 1.7_
  
  - [ ]* 10.4 Write integration tests for EmulatorCore
    - Test complete frame execution with master clock
    - Test CPU and VDC execute in correct order
    - Test frame completion detection
    - Test interrupt timing
    - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7_

- [ ] 11. Implement diagnostic logging
  - [ ] 11.1 Add logging for character rendering state
    - Log character control register values when diagnostic mode enabled
    - Log calculated ROM addresses for each character
    - Log character visibility decisions
    - Log pattern byte values fetched from ROM
    - Add compile-time or runtime flag to enable/disable logging
    - _Requirements: 12.1, 12.2, 12.3, 12.4, 12.5_
  
  - [ ]* 11.2 Write unit tests for diagnostic logging
    - Test logging can be enabled/disabled
    - Test log output contains expected information
    - Test logging doesn't affect emulation behavior
    - _Requirements: 12.1, 12.2, 12.3, 12.4, 12.5_

- [x] 12. Integration testing with Satellite Attack
  - [ ]* 12.1 Create Satellite Attack integration test
    - Load Satellite Attack ROM
    - Run emulator for several frames
    - Verify character data is written to registers 0x10-0x3F
    - Verify display enable bit (0xA0 bit 5) is set
    - Verify satellites (foreground characters) appear in framebuffer
    - Compare framebuffer output with expected satellite positions
    - _Requirements: 3.1, 4.1, 4.2, 8.1, 8.2_
  
  - [ ]* 12.2 Create mid-scanline register write test
    - Write character position during mid-scanline
    - Verify character appears at correct position
    - Verify register write takes effect at current beam position
    - _Requirements: 1.7_
  
  - [ ]* 12.3 Create timing accuracy test
    - Compare CPU and VDC cycle counts over multiple frames
    - Verify cycle ratio maintains ~9.9 VDC cycles per CPU instruction
    - Verify frame timing matches expected duration
    - _Requirements: 1.1, 1.2, 1.3_

- [ ] 13. Update state management for backward compatibility
  - [ ] 13.1 Implement state migration for VDCState
    - Add version field to VDCState serialization
    - Implement migration from old VDCState (scanline, cycle_counter) to new (beam_x, beam_y, total_cycles)
    - Handle loading old save states with graceful conversion
    - _Requirements: 2.4_
  
  - [ ] 13.2 Update debugger display for new VDC state
    - Update debugger to show beam_x and beam_y instead of scanline
    - Update debugger to show total_cycles
    - Ensure debugger can inspect character control registers
    - _Requirements: 2.4, 12.1_
  
  - [ ]* 13.3 Write unit tests for state migration
    - Test loading old save state format
    - Test saving and loading new save state format
    - Test state conversion preserves emulation state
    - _Requirements: 2.4_

- [ ] 14. Performance optimization
  - [ ] 14.1 Optimize hot paths
    - Inline tick_one_cycle() for performance
    - Optimize beam position calculation (use fast modulo)
    - Cache visibility checks per scanline where possible
    - Profile rendering performance and optimize bottlenecks
    - _Requirements: 2.1, 2.3, 2.4_
  
  - [ ]* 14.2 Create performance benchmark tests
    - Measure frame rendering time
    - Compare performance with old scanline-based approach
    - Ensure performance is acceptable (target: 60fps on modern hardware)
    - _Requirements: 2.1, 2.3_

- [x] 15. Final checkpoint - Complete system integration
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 16. Documentation updates
  - [ ] 16.1 Update code documentation
    - Add detailed comments to MasterClock class
    - Add detailed comments to VDC continuous rendering methods
    - Update EmulatorCore run_frame() documentation
    - Document state migration process
    - _Requirements: All_
  
  - [ ] 16.2 Update architecture documentation
    - Document master clock synchronization architecture
    - Document cycle-accurate execution model
    - Document beam position calculation
    - Add diagrams showing CPU-VDC coordination
    - _Requirements: 1.1, 1.2, 1.3, 2.1, 2.3, 2.4_
  
  - [ ] 16.3 Update testing documentation
    - Document property-based testing approach
    - Document how to run property tests
    - Document test coverage for character rendering
    - Add examples of property test usage
    - _Requirements: 13.1, 13.2, 13.3, 13.4, 13.5, 13.6_

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Checkpoints ensure incremental validation at key milestones
- Property tests validate universal correctness properties (minimum 100 iterations each)
- Unit tests validate specific examples, edge cases, and error conditions
- Integration tests verify complete system behavior with real game ROMs
- The implementation uses C++ with Google Test for unit tests and RapidCheck for property-based tests
- All property tests must be tagged with: **Feature: character-rendering-fix, Property N: [property text]**
- State migration ensures backward compatibility with existing save states
- Performance optimization ensures the cycle-accurate approach maintains acceptable frame rates

