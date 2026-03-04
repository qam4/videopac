# Requirements Document

## Introduction

This specification covers enhancements to the videopac libretro core (`src/libretro.cpp`). The three main areas are: a virtual on-screen keyboard overlay that gives users access to all 49 Odyssey 2 keyboard keys via gamepad or touch, a joystick port swap option for games that use the right controller as player 1, and updates to the core info file with additional BIOS entries and a version bump.

## Glossary

- **Core**: The videopac libretro core — the shared library loaded by a libretro frontend
- **Frontend**: A libretro frontend such as RetroArch that hosts the Core
- **Virtual_Keyboard**: A visual overlay rendered on top of the emulated framebuffer showing the Odyssey 2 keyboard layout
- **Keyboard_Cursor**: The currently highlighted key on the Virtual_Keyboard
- **Key_Layout_Table**: A data structure defining each key's label, position, size, VidKey scancode, and directional navigation links
- **VidKey**: The enum encoding Odyssey 2 keyboard matrix positions (row in high nibble, column in low nibble)
- **Video_Buffer**: The XRGB8888 framebuffer (160×240) submitted to the Frontend each frame
- **Core_Option**: A configurable setting exposed to the Frontend via `RETRO_ENVIRONMENT_SET_VARIABLES`
- **Input_Descriptor**: A mapping entry that tells the Frontend which joypad buttons the Core uses
- **Info_File**: The `videopac_libretro.info` metadata file read by the Frontend for display and BIOS information

## Requirements

### Requirement 1: Virtual Keyboard Toggle

**User Story:** As a player, I want to toggle an on-screen keyboard overlay so that I can access all Odyssey 2 keys without a physical keyboard.

#### Acceptance Criteria

1. WHEN the player presses the SELECT button on port 0, THE Core SHALL toggle the Virtual_Keyboard visibility between shown and hidden
2. WHEN the Virtual_Keyboard is shown, THE Core SHALL render the Virtual_Keyboard overlay onto the Video_Buffer after the emulated framebuffer conversion
3. WHEN the Virtual_Keyboard is hidden, THE Core SHALL pass D-pad and button inputs to the emulated joystick and keyboard mappings as normal
4. THE Core SHALL initialize the Virtual_Keyboard in the hidden state at startup

### Requirement 2: Virtual Keyboard Navigation

**User Story:** As a player, I want to navigate the on-screen keyboard with the D-pad so that I can select any key.

#### Acceptance Criteria

1. WHILE the Virtual_Keyboard is shown, WHEN the player presses a D-pad direction on port 0, THE Core SHALL move the Keyboard_Cursor to the adjacent key in that direction according to the Key_Layout_Table navigation links
2. WHILE the Virtual_Keyboard is shown, THE Core SHALL suppress D-pad input from reaching the emulated joystick
3. WHILE the Virtual_Keyboard is shown, THE Core SHALL visually highlight the Keyboard_Cursor position on the Virtual_Keyboard overlay
4. WHILE the Virtual_Keyboard is shown, WHEN the Keyboard_Cursor is at an edge of the keyboard layout, THE Core SHALL keep the Keyboard_Cursor at the current key for that direction

### Requirement 3: Virtual Keyboard Key Press

**User Story:** As a player, I want to press the highlighted key so that the emulated Odyssey 2 receives the corresponding keyboard input.

#### Acceptance Criteria

1. WHILE the Virtual_Keyboard is shown, WHEN the player presses the B button on port 0, THE Core SHALL set the corresponding VidKey matrix entry to pressed in the emulated InputState for that frame
2. WHILE the Virtual_Keyboard is shown, WHEN the player releases the B button on port 0, THE Core SHALL clear the corresponding VidKey matrix entry in the emulated InputState
3. WHILE the Virtual_Keyboard is shown, THE Core SHALL suppress the B button from reaching the emulated keyboard mapping for key 0

### Requirement 4: Virtual Keyboard Rendering

**User Story:** As a player, I want the on-screen keyboard to be rendered programmatically so that the Core remains self-contained without external image assets.

#### Acceptance Criteria

1. THE Core SHALL render the Virtual_Keyboard programmatically using filled rectangles for keys and a built-in bitmap font for key labels
2. THE Core SHALL render the Virtual_Keyboard in XRGB8888 pixel format matching the Video_Buffer format
3. THE Core SHALL alpha-blend the Virtual_Keyboard overlay onto the Video_Buffer using the configured transparency level
4. THE Core SHALL render each key in the Virtual_Keyboard as a distinct rectangular region with a visible border and label text

### Requirement 5: Virtual Keyboard Transparency

**User Story:** As a player, I want to configure the keyboard overlay transparency so that I can balance visibility of the keyboard against the game screen.

#### Acceptance Criteria

1. THE Core SHALL expose a Core_Option named "videopac_vkbd_transparency" with values "0%", "25%", "50%", "75%" and a default of "25%"
2. WHEN the transparency Core_Option value changes, THE Core SHALL apply the new transparency level to subsequent Virtual_Keyboard rendering
3. WHEN transparency is set to 0%, THE Core SHALL render the Virtual_Keyboard fully opaque
4. WHEN transparency is set to 75%, THE Core SHALL render the Virtual_Keyboard at 25% opacity

### Requirement 6: Virtual Keyboard Position

**User Story:** As a player, I want to reposition the keyboard overlay so that it does not obscure the part of the screen I need to see.

#### Acceptance Criteria

1. WHILE the Virtual_Keyboard is shown, WHEN the player presses the Y button on port 0, THE Core SHALL toggle the Virtual_Keyboard position between top and bottom of the Video_Buffer
2. THE Core SHALL default the Virtual_Keyboard position to the bottom of the Video_Buffer
3. WHILE the Virtual_Keyboard is shown, THE Core SHALL suppress the Y button from reaching the emulated keyboard mapping for key 1

### Requirement 7: Virtual Keyboard Touch Support

**User Story:** As a player on a touch-enabled device, I want to tap keys directly on the on-screen keyboard so that I can use the keyboard without a gamepad.

#### Acceptance Criteria

1. WHILE the Virtual_Keyboard is shown, WHEN the player touches a coordinate within a key region on the Virtual_Keyboard, THE Core SHALL set the corresponding VidKey matrix entry to pressed in the emulated InputState
2. WHILE the Virtual_Keyboard is shown, WHEN the player lifts the touch from a key region, THE Core SHALL clear the corresponding VidKey matrix entry in the emulated InputState
3. THE Core SHALL read touch input using the libretro RETRO_DEVICE_POINTER device type
4. THE Core SHALL translate pointer coordinates from the Frontend's normalized coordinate space to Video_Buffer pixel coordinates for hit testing against the Key_Layout_Table


### Requirement 8: Key Layout Table

**User Story:** As a developer, I want a well-defined key layout data structure so that rendering, navigation, and hit testing all use a single source of truth.

#### Acceptance Criteria

1. THE Core SHALL define a Key_Layout_Table containing an entry for each of the 49 Odyssey 2 keys
2. THE Core SHALL store for each key entry: a display label, an x position, a y position, a width, a height, the corresponding VidKey value, and navigation links (up, down, left, right indices)
3. THE Core SHALL arrange the Key_Layout_Table entries to match the physical Odyssey 2 keyboard layout with rows for digits, QWERTY letters, and function keys
4. THE Core SHALL use the Key_Layout_Table as the single source of truth for Virtual_Keyboard rendering, cursor navigation, touch hit testing, and key-to-VidKey mapping

### Requirement 9: Joystick Port Swap

**User Story:** As a player, I want to swap joystick ports so that I can use my primary controller for games that expect player 1 on the right port.

#### Acceptance Criteria

1. THE Core SHALL expose a Core_Option named "videopac_swap_joysticks" with values "disabled" and "enabled" and a default of "disabled"
2. WHILE the joystick swap option is set to "enabled", THE Core SHALL route port 0 joypad input to emulated joystick 2 and port 1 joypad input to emulated joystick 1
3. WHILE the joystick swap option is set to "disabled", THE Core SHALL route port 0 joypad input to emulated joystick 1 and port 1 joypad input to emulated joystick 2
4. WHEN the joystick swap Core_Option value changes, THE Core SHALL apply the new routing on the next frame

### Requirement 10: Info File BIOS Entries

**User Story:** As a user setting up the core, I want the info file to list all supported BIOS files so that the Frontend can guide me to provide the correct files.

#### Acceptance Criteria

1. THE Info_File SHALL declare firmware_count as 4
2. THE Info_File SHALL declare firmware0 as "o2rom.bin" with description "Odyssey 2 BIOS" and opt set to "false"
3. THE Info_File SHALL declare firmware1 as "c52.bin" with description "Videopac C52 BIOS (French)" and opt set to "true"
4. THE Info_File SHALL declare firmware2 as "g7400.bin" with description "Videopac+ G7400 BIOS" and opt set to "true"
5. THE Info_File SHALL declare firmware3 as "jopac.bin" with description "Jopac BIOS (French VP+)" and opt set to "true"

### Requirement 11: Info File Version

**User Story:** As a user, I want the core version to reflect the new features so that I can verify I have the updated build.

#### Acceptance Criteria

1. THE Info_File SHALL set display_version to "0.3.0"

### Requirement 12: Input Descriptor Updates

**User Story:** As a player, I want the Frontend to show accurate button labels so that I know which buttons control the virtual keyboard.

#### Acceptance Criteria

1. THE Core SHALL register an Input_Descriptor for SELECT on port 0 with the label "Toggle Virtual Keyboard"
2. THE Core SHALL register an Input_Descriptor for Y on port 0 with the label "VKB Position / Key 1"
3. THE Core SHALL register an Input_Descriptor for B on port 0 with the label "VKB Press / Key 0"
4. THE Core SHALL register Input_Descriptors for all D-pad directions, A, X, L, and START on port 0 with their existing labels
5. THE Core SHALL register Input_Descriptors for D-pad directions and A on port 1 with "P2" prefixed labels
