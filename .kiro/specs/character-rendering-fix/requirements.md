# Requirements Document

## Introduction

The Videopac emulator's character rendering system is not displaying foreground characters correctly in the Satellite Attack game. Investigation has revealed that satellites are rendered using foreground characters (VDC registers 0x10-0x3F), not sprites. While the game enables foreground display and writes character data, satellites are not appearing on screen.

Root cause analysis has identified a fundamental timing synchronization issue: the current emulator uses a "time-slice" approach where CPU executes for a scanline, then VDC renders that scanline. However, the CPU and VDC have independent clocks (CPU: ~358 kHz instruction cycle, VDC: 3.54 MHz) and should run concurrently, not sequentially.

The current architecture causes timing mismatches where `vdc_.tick()` advances the internal scanline counter during CPU execution, resulting in the wrong scanline being rendered. This manifests as character data written during frame 0 not appearing correctly in frame 1.

This specification addresses implementing a proper master clock synchronization architecture to accurately model the concurrent execution of CPU and VDC, ensuring correct character rendering and timing-sensitive operations.

## Glossary

- **VDC**: Video Display Controller (Intel 8245) - the graphics chip responsible for rendering video output
- **CPU**: Intel 8048 microcontroller running at ~358 kHz instruction cycle (1.79 MHz / 5)
- **Master_Clock**: A unified virtual clock that synchronizes CPU and VDC execution (using VDC 3.54 MHz as base)
- **VDC_Clock**: 3.54 MHz clock driving the VDC chip
- **CPU_Instruction_Cycle**: ~358 kHz (approximately 9.9 VDC clock cycles per CPU instruction)
- **Time_Slice**: A synchronization approach where one component runs, then the other "catches up"
- **Cycle_Accurate**: An approach where CPU and VDC advance in lockstep based on a master clock
- **Character**: A foreground display object defined by position, pattern pointer, and color attributes
- **Character_ROM**: Internal ROM containing 64 character patterns (8 bytes each, 512 bytes total)
- **Character_Pointer**: A 9-bit displacement value used to calculate the ROM address for character patterns
- **Single_Character**: One of 12 individual characters (registers 0x10-0x3F, 4 bytes each)
- **Quad_Character**: One of 4 character groups containing 4 characters each (registers 0x40-0x7F, 16 bytes each)
- **Display_Enable**: Control register bit 5 (0xA0) that enables/disables foreground display
- **Pattern_Byte**: An 8-bit value from Character_ROM where each bit represents a pixel (bit 7 = leftmost)
- **ROM_Address**: The calculated address in Character_ROM using the formula: char_ptr + (char_y / 2) + char_row
- **BEAM_X**: VDC register 0xA4 containing the horizontal beam position (0-226)
- **BEAM_Y**: VDC register 0xA5 containing the vertical beam position (0-261 NTSC, 0-311 PAL)
- **Emulator**: The Videopac/Odyssey2 emulator software being debugged

## Open Investigation

### VDC Y-Coordinate System Mystery

**Current Observation:** The Satellite Attack game positions quad characters (high score display) at Y=199 using the `set_location_of_quads` routine at address 0x730. With the current implementation where Y=0 is at the top and Y=199 is at the bottom of the 200-pixel framebuffer, only 1 scanline of the high score is visible at the very bottom of the screen.

**Evidence:**
1. Game code at 0x732: `MOV R3, C7` (0xC7 = 199 decimal) - writes to VDC register 0x40 (Y position of first quad)
2. Characters are 14 pixels tall (7 rows × 2 scanlines each)
3. Y=199 would render on scanlines 199-212, but only scanline 199 exists in the 200-pixel framebuffer
4. User confirms seeing "1 scan line with pixels at the bottom"

**Root Cause Analysis - RESOLVED:**

After thorough investigation and consultation with Intel 8244/8245 VDC specifications, the current implementation is **CORRECT**.

**Confirmed VDC Specifications:**
1. **Standard Resolution**: 160×200 pixels (this is the "standard visible area")
2. **Addressable Y Range**: 0 to 242 (objects can be positioned anywhere in this range)
3. **Visible Y Range**: Approximately 200 lines (with CRT overscan hiding edges)
4. **Off-screen Positioning**: Y=240+ (0xF0+) is used to hide objects in VBLANK
5. **Coordinate System**: Y=0 at top, Y=199 at bottom of visible area

**The Current Situation:**
- The emulator's 160×200 framebuffer is **correct** for the standard visible area
- The game positions the high score at Y=199 (0xC7)
- Y=199 is at the **very bottom edge** of the 200-line visible area
- Characters are 14 pixels tall, so Y=199 would render scanlines 199-212
- Only scanline 199 exists in the framebuffer, hence only 1 pixel is visible

**Possible Explanations:**
1. **Game Bug**: The high score positioning might be incorrect in the game code
2. **Intentional Off-Screen**: The high score might be intentionally hidden (e.g., not implemented yet in this game version)
3. **Dynamic Repositioning**: The game might reposition the high score later (not found in current analysis)
4. **Different Game Mode**: The high score might only be visible in certain game modes or after certain events

**Conclusion:**
The emulator is working correctly. The issue is either:
- A bug in the game code (unlikely, as the game was commercially released)
- The high score is intentionally positioned off-screen for this particular game state
- We need to investigate further to understand when/how the high score becomes visible

**No Changes Required** to the framebuffer size or coordinate system. The 160×200 framebuffer correctly represents the Intel 8244/8245 VDC's standard visible area.

## Requirements

### Requirement 1: Master Clock Synchronization

**User Story:** As an emulator developer, I want CPU and VDC to execute concurrently based on a unified master clock, so that timing-sensitive operations (like mid-scanline register writes) work correctly as on real hardware.

#### Acceptance Criteria

1. THE Emulator SHALL maintain a master clock counter using the VDC clock frequency (3.54 MHz) as the base unit
2. THE Emulator SHALL calculate CPU instruction cycles as approximately 9.9 VDC clock cycles (3.54 MHz / 358 kHz)
3. WHEN the master clock advances, THE Emulator SHALL determine which component (CPU or VDC) should execute next based on accumulated cycle debt
4. THE CPU SHALL execute one instruction when its cycle debt reaches the instruction cycle threshold
5. THE VDC SHALL advance by one clock cycle for each master clock tick
6. THE Emulator SHALL maintain separate cycle accumulators for CPU and VDC to track execution timing
7. WHEN a CPU instruction writes to VDC registers, THE VDC SHALL reflect the change at the current beam position (not deferred to end of scanline)

### Requirement 2: VDC Continuous Rendering

**User Story:** As an emulator developer, I want the VDC to render continuously as the beam progresses, so that the framebuffer accurately reflects the display state at any point in time.

#### Acceptance Criteria

1. THE VDC SHALL render pixels continuously as the internal beam position advances
2. THE VDC SHALL NOT render in discrete "scanline" chunks
3. WHEN the VDC clock advances, THE VDC SHALL update the beam position and render any visible pixels at that position
4. THE VDC SHALL maintain accurate horizontal (beam_x) and vertical (beam_y) beam positions based on cycle count
5. WHEN the beam position advances to a new scanline, THE VDC SHALL automatically begin rendering that scanline
6. THE VDC SHALL handle HBLANK and VBLANK periods correctly without rendering pixels during blanking

### Requirement 3: Beam Position Register Mapping

**User Story:** As a game developer, I want to read the correct beam position from VDC registers, so that timing-sensitive operations (like wait_until_scanline) work correctly.

#### Acceptance Criteria

1. THE VDC SHALL map BEAM_X (horizontal beam position) to register address 0xA4
2. THE VDC SHALL map BEAM_Y (vertical beam position) to register address 0xA5
3. WHEN the CPU reads register 0xA4, THE VDC SHALL return the current horizontal beam position (0-226)
4. WHEN the CPU reads register 0xA5, THE VDC SHALL return the current vertical beam position (0-261 NTSC, 0-311 PAL)
5. THE VDC SHALL update BEAM_X and BEAM_Y registers continuously as the beam advances during rendering

#### Background

Investigation revealed that the emulator had BEAM_X and BEAM_Y register definitions swapped in include/vdc.h:
- Incorrect mapping: BEAM_Y=0xA4, BEAM_X=0xA5
- Correct mapping: BEAM_X=0xA4, BEAM_Y=0xA5 (confirmed by Intel 8245 documentation)

This bug caused the Satellite Attack game's wait_until_scanline function to take 243 iterations (33,502 cycles) instead of 8 iterations (969 cycles), a 97% performance degradation. The function was reading the Y position when it expected the X position, causing it to loop unnecessarily.

### Requirement 4: Character Display Enable

**User Story:** As a game developer, I want characters to display when the foreground enable bit is set, so that my game graphics appear correctly.

#### Acceptance Criteria

1. WHEN the Display_Enable bit (bit 5 of register 0xA0) is set to 1, THE VDC SHALL render foreground characters
2. WHEN the Display_Enable bit is set to 0, THE VDC SHALL NOT render foreground characters
3. WHEN the display_enabled flag is updated, THE VDC SHALL reflect the current state of bit 5 in register 0xA0

### Requirement 5: Character Position Validation

**User Story:** As a game developer, I want characters to appear at their specified screen positions, so that game objects are positioned correctly.

#### Acceptance Criteria

1. WHEN a character's Y position is within the visible scanline range (0-191), THE VDC SHALL render the character at that Y position
2. WHEN a character's X position is within the framebuffer width (0-159), THE VDC SHALL render the character at that X position
3. WHEN a character's position is outside the visible area, THE VDC SHALL skip rendering that character
4. FOR ALL Single_Characters, THE VDC SHALL use the X position from byte 1 and Y position from byte 0 of the character control block
5. FOR ALL Quad_Characters, THE VDC SHALL calculate X positions relative to the quad's base X position (byte 13 of the quad control block)

### Requirement 6: Character Pattern ROM Address Calculation

**User Story:** As an emulator developer, I want the ROM address calculation to match hardware behavior, so that correct character patterns are displayed.

#### Acceptance Criteria

1. WHEN calculating ROM_Address for a character, THE VDC SHALL use the formula: (char_ptr + (char_y / 2) + char_row) & 0x1FF
2. WHEN the calculated ROM_Address is greater than or equal to 512, THE VDC SHALL skip rendering that character
3. WHEN the Character_Pointer is a 9-bit value, THE VDC SHALL extract it from bits 0-7 of byte 2 and bit 0 of byte 3
4. WHEN char_row is calculated, THE VDC SHALL use the formula: (scanline - char_y) / 2
5. FOR ALL character rendering operations, THE VDC SHALL wrap ROM_Address at 512 bytes using the 9-bit mask 0x1FF

### Requirement 7: Character Pattern Rendering

**User Story:** As a game developer, I want character patterns to render with correct pixel data, so that recognizable shapes appear on screen.

#### Acceptance Criteria

1. WHEN rendering a character scanline, THE VDC SHALL fetch the Pattern_Byte from Character_ROM at the calculated ROM_Address
2. WHEN a Pattern_Byte bit is 1, THE VDC SHALL draw a pixel at the corresponding screen position with the character's color
3. WHEN a Pattern_Byte bit is 0, THE VDC SHALL NOT draw a pixel (transparent)
4. WHEN extracting pixel bits from Pattern_Byte, THE VDC SHALL use bit 7 as the leftmost pixel and bit 0 as the rightmost pixel
5. FOR ALL characters, THE VDC SHALL render 8 pixels horizontally per scanline
6. FOR ALL characters, THE VDC SHALL render each character row across 2 scanlines (double-height)

### Requirement 8: Character Color Attributes

**User Story:** As a game developer, I want characters to display in their specified colors, so that game graphics have correct appearance.

#### Acceptance Criteria

1. WHEN rendering a character pixel, THE VDC SHALL use the color value from bits 1-3 of the character attribute byte (byte 3)
2. WHEN the color value is extracted, THE VDC SHALL mask bits 1-3 and shift right by 1 to get a 3-bit color index (0-7)
3. FOR ALL visible character pixels, THE VDC SHALL write the color value to the framebuffer at the pixel's screen position

### Requirement 9: Single Character Rendering

**User Story:** As a game developer, I want all 12 single characters to render independently, so that I can display multiple separate objects.

#### Acceptance Criteria

1. THE VDC SHALL support rendering up to 12 Single_Characters simultaneously
2. FOR ALL Single_Characters, THE VDC SHALL read control data from registers 0x10-0x3F (4 bytes per character)
3. WHEN rendering Single_Characters, THE VDC SHALL process characters 0-11 in order
4. FOR ALL Single_Characters, THE VDC SHALL use independent position, pattern pointer, and color attributes

### Requirement 10: Quad Character Rendering

**User Story:** As a game developer, I want quad characters to render as groups of 4, so that I can efficiently display multi-character objects.

#### Acceptance Criteria

1. THE VDC SHALL support rendering up to 4 Quad_Character groups simultaneously
2. FOR ALL Quad_Characters, THE VDC SHALL read control data from registers 0x40-0x7F (16 bytes per quad)
3. WHEN rendering a Quad_Character group, THE VDC SHALL render all 4 sub-characters with 8-pixel horizontal spacing
4. WHEN calculating sub-character X positions, THE VDC SHALL use the formula: quad_x + (sub_char_index * 8)
5. FOR ALL Quad_Characters, THE VDC SHALL use the Y and X position from bytes 0-1 (the FIRST character) as the base position for the entire quad

#### Background

**Important Note on Documentation Error**: The o2doc.md states that "the X position and Y position of the LAST character sets the position of the whole set", but this is incorrect. Actual hardware behavior, confirmed by game code analysis and technical research, shows that the FIRST character's position (bytes 0-1 of the quad) controls the entire group. The hardware ignores the Y/X values in bytes 4-5, 8-9, and 12-13 of each quad.

### Requirement 11: Character ROM Data Integrity

**User Story:** As an emulator developer, I want the Character_ROM to contain accurate pattern data, so that characters display with correct shapes.

#### Acceptance Criteria

1. THE Character_ROM SHALL contain 64 character patterns (characters 0-63)
2. FOR ALL character patterns, THE Character_ROM SHALL store 8 bytes per character (512 bytes total)
3. WHEN the Emulator initializes, THE Character_ROM SHALL be populated with Intel 8245 character pattern data
4. FOR ALL Character_ROM addresses 0-511, THE Character_ROM SHALL return valid pattern data

### Requirement 12: Character Visibility Bounds Checking

**User Story:** As an emulator developer, I want proper bounds checking to prevent rendering artifacts, so that the display remains stable.

#### Acceptance Criteria

1. WHEN a character pixel's X coordinate is negative or >= framebuffer width, THE VDC SHALL skip rendering that pixel
2. WHEN a character's Y position places it entirely outside the framebuffer height, THE VDC SHALL skip rendering that character
3. WHEN a calculated ROM_Address is >= 512, THE VDC SHALL skip rendering that character
4. FOR ALL character rendering operations, THE VDC SHALL validate coordinates before writing to the framebuffer

### Requirement 13: Character Rendering Priority

**User Story:** As a game developer, I want characters to render in the correct layer order, so that visual priority is correct.

#### Acceptance Criteria

1. WHEN rendering a frame, THE VDC SHALL render characters after the grid layer
2. WHEN rendering a frame, THE VDC SHALL render characters before the sprite layer
3. WHEN a character pixel overlaps a background or grid pixel, THE character pixel SHALL be visible
4. WHEN a sprite pixel overlaps a character pixel, THE sprite pixel SHALL be visible

### Requirement 15: Diagnostic Logging

**User Story:** As an emulator developer, I want detailed logging of character rendering state, so that I can debug rendering issues.

#### Acceptance Criteria

1. WHEN diagnostic logging is enabled, THE Emulator SHALL log character control register values
2. WHEN diagnostic logging is enabled, THE Emulator SHALL log calculated ROM addresses for each character
3. WHEN diagnostic logging is enabled, THE Emulator SHALL log character visibility decisions
4. WHEN diagnostic logging is enabled, THE Emulator SHALL log Pattern_Byte values fetched from Character_ROM
5. THE Emulator SHALL provide a mechanism to enable/disable diagnostic logging without recompilation

### Requirement 14: Diagnostic Logging

**User Story:** As an emulator developer, I want detailed logging of character rendering state, so that I can debug rendering issues.

#### Acceptance Criteria

1. WHEN diagnostic logging is enabled, THE Emulator SHALL log character control register values
2. WHEN diagnostic logging is enabled, THE Emulator SHALL log calculated ROM addresses for each character
3. WHEN diagnostic logging is enabled, THE Emulator SHALL log character visibility decisions
4. WHEN diagnostic logging is enabled, THE Emulator SHALL log Pattern_Byte values fetched from Character_ROM
5. THE Emulator SHALL provide a mechanism to enable/disable diagnostic logging without recompilation

### Requirement 15: Character Rendering Test Cases

**User Story:** As an emulator developer, I want comprehensive test coverage for character rendering, so that bugs are caught early.

#### Acceptance Criteria

1. THE Emulator SHALL include unit tests for ROM address calculation with various char_ptr, char_y, and char_row values
2. THE Emulator SHALL include unit tests for character visibility bounds checking
3. THE Emulator SHALL include unit tests for pattern byte pixel extraction
4. THE Emulator SHALL include unit tests for color attribute extraction
5. THE Emulator SHALL include property-based tests for character rendering with random valid inputs
6. THE Emulator SHALL include integration tests that verify complete character rendering for known patterns
7. THE Emulator SHALL include unit tests for beam position register mapping (0xA4=BEAM_X, 0xA5=BEAM_Y)
