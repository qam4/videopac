# Requirements Document

## Introduction

This document specifies the requirements for enhancing the SDL frontend of the Videopac/Odyssey2 emulator with an in-game menu system, file browser, ZIP file support, and quality-of-life improvements. The enhancements will provide users with a modern, user-friendly interface for managing BIOS/ROM files, save states, and emulator settings without requiring command-line arguments or restarting the application.

## Glossary

- **SDL_Frontend**: The SDL2-based graphical frontend component responsible for video rendering, audio output, and user input handling
- **Menu_System**: An overlay UI that appears on top of the emulator display, allowing user interaction while pausing emulation
- **File_Browser**: A UI component that displays and allows selection of files from the filesystem
- **ROM**: Read-Only Memory file containing game cartridge data (.bin, .rom extensions)
- **BIOS**: Basic Input/Output System file required for emulator initialization
- **Save_State**: A snapshot of the complete emulator state that can be saved to and loaded from disk
- **Emulator_Core**: The core emulation engine that executes the Videopac/Odyssey2 system
- **ZIP_Archive**: A compressed file format (.zip extension) that may contain ROM or BIOS files

## Requirements

### Requirement 1: In-Game Menu System

**User Story:** As a user, I want to access an overlay menu during gameplay, so that I can manage emulator settings and files without exiting the application.

#### Acceptance Criteria

1. WHEN the user presses F10, THE Menu_System SHALL display an overlay menu on top of the emulator display
2. WHEN the Menu_System is active, THE Emulator_Core SHALL pause execution
3. WHEN the Menu_System is dismissed, THE Emulator_Core SHALL resume execution from the paused state
4. WHEN the Menu_System is displayed, THE SDL_Frontend SHALL continue rendering the paused game screen underneath
5. THE Menu_System SHALL use a simple, readable font and color scheme that contrasts with the game display
6. WHEN the Menu_System is active, THE SDL_Frontend SHALL accept keyboard input for menu navigation (arrow keys, Enter, Escape)
7. WHEN the user presses Escape in the Menu_System, THE Menu_System SHALL close and resume emulation

### Requirement 2: File Browser Component

**User Story:** As a user, I want to browse and select BIOS and ROM files from within the emulator, so that I can load different games without restarting the application.

#### Acceptance Criteria

1. WHEN the File_Browser is opened, THE File_Browser SHALL display files with .bin, .rom, and .zip extensions
2. WHEN the File_Browser is opened, THE File_Browser SHALL start in the last used directory if available
3. IF no last used directory exists, THEN THE File_Browser SHALL start in the current working directory
4. WHEN displaying files, THE File_Browser SHALL show the filename and file size for each entry
5. WHEN the user navigates directories, THE File_Browser SHALL allow moving up to parent directories
6. WHEN the user selects a file, THE File_Browser SHALL return the full path to the selected file
7. WHEN the File_Browser is active, THE SDL_Frontend SHALL accept keyboard input for navigation (arrow keys, Enter, Escape)
8. WHEN the user presses Escape in the File_Browser, THE File_Browser SHALL close without selecting a file

### Requirement 3: ZIP File Support

**User Story:** As a user, I want to load ROM files directly from ZIP archives, so that I can use compressed ROM collections without manual extraction.

#### Acceptance Criteria

1. WHEN a .zip file is selected for loading, THE SDL_Frontend SHALL extract the archive contents
2. WHEN extracting a ZIP_Archive, THE SDL_Frontend SHALL search for files with .bin or .rom extensions
3. IF multiple ROM files are found in the ZIP_Archive, THEN THE SDL_Frontend SHALL present a selection dialog
4. IF exactly one ROM file is found in the ZIP_Archive, THEN THE SDL_Frontend SHALL load it automatically
5. IF no ROM files are found in the ZIP_Archive, THEN THE SDL_Frontend SHALL display an error message
6. WHEN loading from a ZIP_Archive, THE SDL_Frontend SHALL extract the ROM to a temporary location
7. WHEN the emulator exits, THE SDL_Frontend SHALL clean up temporary extracted files
8. THE SDL_Frontend SHALL use the miniz library for ZIP decompression

### Requirement 4: Menu Options and Actions

**User Story:** As a user, I want to perform common emulator actions from the menu, so that I can manage my gaming session efficiently.

#### Acceptance Criteria

1. WHEN the Menu_System displays, THE Menu_System SHALL show options for "Load BIOS", "Load ROM", "Reset", "Save State", "Load State", "Display Info", and "Exit"
2. WHEN "Load BIOS" is selected, THE Menu_System SHALL open the File_Browser filtered for BIOS files
3. WHEN a BIOS file is selected, THE SDL_Frontend SHALL load the BIOS into the Emulator_Core
4. WHEN "Load ROM" is selected, THE Menu_System SHALL open the File_Browser filtered for ROM and ZIP files
5. WHEN a ROM file is selected, THE SDL_Frontend SHALL load the ROM into the Emulator_Core
6. WHEN "Reset" is selected, THE Emulator_Core SHALL reset to its initial state with the current BIOS and ROM
7. WHEN "Save State" is selected, THE Menu_System SHALL display a slot selection dialog (slots 1-9)
8. WHEN a save slot is selected, THE SDL_Frontend SHALL serialize the Emulator_Core state to disk
9. WHEN "Load State" is selected, THE Menu_System SHALL display available save state slots
10. WHEN a load slot is selected, THE SDL_Frontend SHALL deserialize the Emulator_Core state from disk
11. WHEN "Display Info" is selected, THE Menu_System SHALL show the current ROM name, BIOS name, and emulator version
12. WHEN "Exit" is selected, THE SDL_Frontend SHALL terminate the application

### Requirement 5: Loading Status and Progress

**User Story:** As a user, I want to see loading status and progress indicators, so that I know the emulator is responding to my actions.

#### Acceptance Criteria

1. WHEN loading a BIOS file, THE SDL_Frontend SHALL display a "Loading BIOS..." message
2. WHEN loading a ROM file, THE SDL_Frontend SHALL display a "Loading ROM..." message
3. WHEN extracting a ZIP_Archive, THE SDL_Frontend SHALL display a "Extracting..." message
4. WHEN a file operation completes successfully, THE SDL_Frontend SHALL display a success message for 2 seconds
5. WHEN a file operation fails, THE SDL_Frontend SHALL display an error message until dismissed by the user

### Requirement 6: Error Message Display

**User Story:** As a user, I want to see error messages in the UI rather than just the console, so that I can understand what went wrong without checking terminal output.

#### Acceptance Criteria

1. WHEN a file loading error occurs, THE SDL_Frontend SHALL display an error dialog with the error description
2. WHEN a ZIP extraction error occurs, THE SDL_Frontend SHALL display an error dialog with the error description
3. WHEN a save state operation fails, THE SDL_Frontend SHALL display an error dialog with the error description
4. WHEN an error dialog is displayed, THE SDL_Frontend SHALL wait for user acknowledgment (Enter or Escape key)
5. THE SDL_Frontend SHALL continue logging errors to the console in addition to displaying UI messages

### Requirement 7: FPS Display Toggle

**User Story:** As a user, I want to toggle an on-screen FPS counter, so that I can monitor emulator performance without checking console output.

#### Acceptance Criteria

1. WHEN the user presses F3, THE SDL_Frontend SHALL toggle the FPS display on or off
2. WHEN the FPS display is enabled, THE SDL_Frontend SHALL render the current FPS value in the top-right corner
3. WHEN rendering the FPS display, THE SDL_Frontend SHALL use a small, readable font that doesn't obstruct gameplay
4. THE SDL_Frontend SHALL update the FPS display once per second
5. WHEN the FPS display is disabled, THE SDL_Frontend SHALL not render any FPS information

### Requirement 8: Keyboard Shortcuts

**User Story:** As a user, I want convenient keyboard shortcuts for common actions, so that I can quickly access emulator features.

#### Acceptance Criteria

1. WHEN the user presses F10, THE SDL_Frontend SHALL open the Menu_System
2. WHEN the user presses F3, THE SDL_Frontend SHALL toggle the FPS display
3. WHEN the user presses F5 outside the debugger, THE SDL_Frontend SHALL reset the Emulator_Core
4. WHEN the user presses F6, THE SDL_Frontend SHALL quick-save to slot 0
5. WHEN the user presses F7, THE SDL_Frontend SHALL quick-load from slot 0
6. WHEN the user presses F12, THE SDL_Frontend SHALL save a screenshot
7. WHEN the user presses Escape outside the Menu_System, THE SDL_Frontend SHALL exit the application

### Requirement 9: Directory Memory

**User Story:** As a user, I want the file browser to remember my last used directory, so that I don't have to navigate to my ROM folder every time.

#### Acceptance Criteria

1. WHEN a file is successfully loaded, THE SDL_Frontend SHALL store the directory path
2. WHEN the File_Browser is opened, THE SDL_Frontend SHALL retrieve the last used directory path
3. THE SDL_Frontend SHALL persist the last used directory path to a configuration file
4. WHEN the emulator starts, THE SDL_Frontend SHALL load the last used directory from the configuration file
5. IF the stored directory no longer exists, THEN THE File_Browser SHALL fall back to the current working directory

### Requirement 10: Save State File Management

**User Story:** As a user, I want save states to be organized and named clearly, so that I can identify and manage my saved games.

#### Acceptance Criteria

1. WHEN saving a state, THE SDL_Frontend SHALL create a file named "{rom_name}.state{slot_number}"
2. WHEN displaying save state slots, THE SDL_Frontend SHALL show the timestamp of each save file if it exists
3. WHEN displaying save state slots, THE SDL_Frontend SHALL indicate empty slots
4. THE SDL_Frontend SHALL store save state files in a "saves" subdirectory
5. WHEN the "saves" directory does not exist, THE SDL_Frontend SHALL create it automatically
6. WHEN displaying save state slots, THE Menu_System SHALL show a preview thumbnail if available
7. WHEN saving a state, THE SDL_Frontend SHALL capture a screenshot thumbnail of the current display
8. THE Menu_System SHALL include options to "Delete Save State" for managing save files
9. WHEN deleting a save state, THE SDL_Frontend SHALL prompt for confirmation before deletion


### Requirement 11: Recent Files List

**User Story:** As a user, I want to see a list of recently loaded ROMs and BIOS files, so that I can quickly reload my favorite games without browsing.

#### Acceptance Criteria

1. WHEN a ROM file is successfully loaded, THE SDL_Frontend SHALL add it to the recent files list
2. WHEN a BIOS file is successfully loaded, THE SDL_Frontend SHALL add it to the recent BIOS list
3. THE SDL_Frontend SHALL maintain separate lists for recent ROMs and recent BIOS files
4. THE SDL_Frontend SHALL store up to 10 recent files in each list
5. WHEN the recent files list exceeds 10 entries, THE SDL_Frontend SHALL remove the oldest entry
6. WHEN the Menu_System displays "Load ROM", THE Menu_System SHALL show a "Recent ROMs" submenu
7. WHEN the Menu_System displays "Load BIOS", THE Menu_System SHALL show a "Recent BIOS" submenu
8. WHEN a recent file is selected, THE SDL_Frontend SHALL load the file directly without opening the File_Browser
9. THE SDL_Frontend SHALL persist the recent files lists to a configuration file
10. WHEN a recent file no longer exists, THE SDL_Frontend SHALL display an error and remove it from the list

### Requirement 12: Fullscreen Support

**User Story:** As a user, I want to toggle fullscreen mode, so that I can enjoy games on the full screen without distractions.

#### Acceptance Criteria

1. WHEN the user presses F11 or Alt+Enter, THE SDL_Frontend SHALL toggle between windowed and fullscreen mode
2. WHEN entering fullscreen mode, THE SDL_Frontend SHALL use the current desktop resolution
3. WHEN in fullscreen mode, THE SDL_Frontend SHALL maintain the correct aspect ratio of the Videopac display
4. WHEN maintaining aspect ratio, THE SDL_Frontend SHALL add letterboxing (black bars) as needed
5. WHEN exiting fullscreen mode, THE SDL_Frontend SHALL restore the previous window size and position
6. THE SDL_Frontend SHALL persist the fullscreen preference to a configuration file
7. WHEN the emulator starts, THE SDL_Frontend SHALL restore the last used fullscreen state
8. WHEN in fullscreen mode, THE Menu_System SHALL still be accessible and visible
9. THE Menu_System SHALL include a "Toggle Fullscreen" option

### Requirement 13: Video Settings Configuration

**User Story:** As a user, I want to configure video settings like scaling filters and aspect ratio, so that I can customize the visual appearance to my preference.

#### Acceptance Criteria

1. THE Menu_System SHALL include a "Video Settings" submenu
2. WHEN "Video Settings" is opened, THE Menu_System SHALL display options for "Scaling Filter", "Aspect Ratio", "VSync", "CRT Effects", and "Scanlines"
3. THE SDL_Frontend SHALL support scaling filters: "Nearest" (sharp pixels) and "Linear" (smooth scaling)
4. WHEN a scaling filter is changed, THE SDL_Frontend SHALL apply it immediately to the display
5. THE SDL_Frontend SHALL support aspect ratio modes: "Original" (1:1 pixel), "4:3" (standard TV), and "Stretch" (fill window)
6. WHEN an aspect ratio mode is changed, THE SDL_Frontend SHALL adjust the display viewport accordingly
7. THE SDL_Frontend SHALL support VSync toggle (on/off)
8. WHEN VSync is toggled, THE SDL_Frontend SHALL enable or disable vertical synchronization
9. THE SDL_Frontend SHALL support CRT filter effects: "None", "Light", "Medium", "Heavy"
10. WHEN CRT effects are enabled, THE SDL_Frontend SHALL apply a shader or post-processing effect simulating CRT display characteristics
11. THE SDL_Frontend SHALL support scanline overlay: "Off", "25%", "50%", "75%"
12. WHEN scanlines are enabled, THE SDL_Frontend SHALL render horizontal lines to simulate CRT scanlines
13. THE SDL_Frontend SHALL persist video settings to the configuration file

### Requirement 14: Audio Settings Configuration

**User Story:** As a user, I want to control audio settings like volume and mute, so that I can adjust sound to my environment.

#### Acceptance Criteria

1. THE Menu_System SHALL include an "Audio Settings" submenu
2. WHEN "Audio Settings" is opened, THE Menu_System SHALL display options for "Volume", "Mute", and "Audio Buffer Size"
3. THE SDL_Frontend SHALL support volume levels from 0% to 100% in 10% increments
4. WHEN volume is adjusted, THE SDL_Frontend SHALL apply the new volume level immediately
5. WHEN the user presses F4, THE SDL_Frontend SHALL toggle audio mute on or off
6. WHEN audio is muted, THE SDL_Frontend SHALL display a mute indicator on screen
7. WHEN audio is muted, THE SDL_Frontend SHALL stop audio output but continue audio processing
8. THE SDL_Frontend SHALL support audio buffer sizes: "Small" (512 samples), "Medium" (1024 samples), "Large" (2048 samples)
9. WHEN audio buffer size is changed, THE SDL_Frontend SHALL reinitialize the audio subsystem with the new buffer size
10. THE SDL_Frontend SHALL display a warning that changing buffer size requires restart or ROM reload
11. THE SDL_Frontend SHALL persist audio settings to the configuration file
12. WHEN the emulator starts, THE SDL_Frontend SHALL restore the last used volume and mute state

### Requirement 15: Input Configuration and Key Mapping

**User Story:** As a user, I want to configure keyboard and joystick mappings, so that I can use my preferred input devices and customize controls.

#### Acceptance Criteria

1. THE Menu_System SHALL include an "Input Settings" submenu
2. WHEN "Input Settings" is opened, THE Menu_System SHALL display options for "Configure Player 1" and "Configure Player 2"
3. WHEN configuring a player, THE Menu_System SHALL display all mappable actions: Up, Down, Left, Right, Action Button
4. WHEN a user selects an action to remap, THE SDL_Frontend SHALL wait for keyboard or joystick input
5. WHEN input is received, THE SDL_Frontend SHALL assign that input to the selected action
6. THE SDL_Frontend SHALL support both keyboard keys and joystick buttons/axes for input
7. THE SDL_Frontend SHALL detect connected joysticks on startup and when devices are connected
8. WHEN multiple joysticks are connected, THE Menu_System SHALL allow selecting which joystick is assigned to each player
9. THE SDL_Frontend SHALL display the current input device for each player (Keyboard, Joystick 1, Joystick 2, etc.)
10. THE SDL_Frontend SHALL provide default keyboard mappings: Arrow keys + Space for Player 1, WASD + Left Shift for Player 2
11. THE SDL_Frontend SHALL persist input mappings to the configuration file
12. WHEN the emulator starts, THE SDL_Frontend SHALL restore saved input mappings

### Requirement 16: Configuration Persistence

**User Story:** As a user, I want my preferences and settings to be saved, so that I don't have to reconfigure the emulator every time I start it.

#### Acceptance Criteria

1. THE SDL_Frontend SHALL store configuration in a file named "videopac.cfg" in the user's home directory or application data folder
2. THE configuration file SHALL store: last used directory, recent files lists, fullscreen state, FPS display state, window size, window position, video settings, audio settings, and input mappings
3. WHEN the emulator starts, THE SDL_Frontend SHALL load configuration from the file if it exists
4. WHEN the emulator exits normally, THE SDL_Frontend SHALL save the current configuration to the file
5. IF the configuration file is corrupted or invalid, THEN THE SDL_Frontend SHALL use default values and create a new configuration file
6. THE configuration file SHALL use a simple text format (INI or JSON) for easy manual editing
7. THE Menu_System SHALL include a "Reset to Defaults" option that restores all settings to their default values

### Requirement 17: Screenshot Capture and Format Options

**User Story:** As a user, I want to capture screenshots in different formats, so that I can save and share my gameplay moments.

#### Acceptance Criteria

1. WHEN the user presses F12, THE SDL_Frontend SHALL capture a screenshot of the current display
2. THE SDL_Frontend SHALL support screenshot formats: PNG, BMP, and TGA
3. THE Menu_System SHALL include a "Screenshot Settings" submenu with format selection
4. WHEN a screenshot is captured, THE SDL_Frontend SHALL save it with filename "videopac_YYYYMMDD_HHMMSS.{ext}"
5. THE SDL_Frontend SHALL store screenshots in a "screenshots" subdirectory
6. WHEN the "screenshots" directory does not exist, THE SDL_Frontend SHALL create it automatically
7. WHEN a screenshot is saved successfully, THE SDL_Frontend SHALL display a brief notification message
8. THE SDL_Frontend SHALL persist the screenshot format preference to the configuration file

### Requirement 18: Turbo Mode and Speed Control

**User Story:** As a user, I want to speed up or slow down emulation, so that I can skip through slow parts or practice difficult sections.

#### Acceptance Criteria

1. WHEN the user holds Tab, THE SDL_Frontend SHALL run emulation at turbo speed (2x-4x normal)
2. THE Menu_System SHALL include a "Speed Control" submenu with options: "25%", "50%", "100%", "200%", "400%", "Unlimited"
3. WHEN speed is changed, THE SDL_Frontend SHALL adjust the emulation speed accordingly
4. WHEN running at non-100% speed, THE SDL_Frontend SHALL display the current speed percentage on screen
5. THE SDL_Frontend SHALL maintain audio pitch at all speeds (no chipmunk effect)
6. WHEN running at speeds above 100%, THE SDL_Frontend MAY disable audio to maintain performance
7. WHEN the user releases Tab, THE SDL_Frontend SHALL return to the configured normal speed
8. THE SDL_Frontend SHALL persist the normal speed setting to the configuration file
9. THE turbo key (Tab) SHALL always run at maximum speed regardless of the configured normal speed

### Requirement 19: On-Screen Display Customization

**User Story:** As a user, I want to customize the position and appearance of on-screen information, so that it doesn't interfere with my gameplay.

#### Acceptance Criteria

1. THE Menu_System SHALL include an "OSD Settings" submenu
2. WHEN "OSD Settings" is opened, THE Menu_System SHALL display options for "FPS Position", "Notification Position", "Font Size", and "OSD Opacity"
3. THE SDL_Frontend SHALL support OSD positions: "Top-Left", "Top-Right", "Bottom-Left", "Bottom-Right"
4. WHEN OSD position is changed, THE SDL_Frontend SHALL move all on-screen displays to the new position
5. THE SDL_Frontend SHALL support font sizes: "Small", "Medium", "Large"
6. WHEN font size is changed, THE SDL_Frontend SHALL resize all on-screen text accordingly
7. THE SDL_Frontend SHALL support OSD opacity levels: "25%", "50%", "75%", "100%"
8. WHEN OSD opacity is changed, THE SDL_Frontend SHALL apply transparency to all on-screen displays
9. THE SDL_Frontend SHALL persist OSD settings to the configuration file
