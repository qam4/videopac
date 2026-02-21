# Implementation Plan: SDL Frontend Enhancements

## Overview

This implementation plan breaks down the SDL frontend enhancements into incremental, testable steps. The approach follows a bottom-up strategy: build foundational components first (configuration, file system utilities), then UI components (menu, file browser, dialogs), then integrate with the emulator, and finally add quality-of-life features. Each major component includes optional property-based tests to validate correctness properties from the design document.

The implementation is organized into logical phases that build upon each other, with checkpoints to ensure stability before proceeding. All tasks reference specific requirements from the requirements document for traceability.

## Tasks

- [x] 1. Set up project infrastructure and dependencies
  - Add miniz library for ZIP support (single-header library)
  - Add SDL_ttf for text rendering (or implement bitmap font fallback)
  - Add stb_image_write for screenshot/thumbnail saving (single-header library)
  - Update CMakeLists.txt with new dependencies
  - Create directory structure: `src/ui/`, `include/ui/`
  - _Requirements: 3.8, 17.2_

- [x] 2. Implement configuration management system
  - [x] 2.1 Create ConfigManager class with INI file parsing
    - Implement INI file parser (simple key=value format with sections)
    - Implement configuration getters and setters for all settings
    - Implement default values for all configuration options
    - Use SDL_GetPrefPath() for cross-platform config file location
    - _Requirements: 16.1, 16.2, 16.6_
  
  - [ ]* 2.2 Write property test for configuration round-trip
    - **Property 77: Configuration completeness**
    - **Property 78: Configuration loading on startup**
    - **Validates: Requirements 16.2, 16.3**
  
  - [ ]* 2.3 Write unit tests for ConfigManager
    - Test loading valid configuration file
    - Test loading corrupted configuration file (should use defaults)
    - Test saving configuration file
    - Test missing configuration file (should create new)
    - _Requirements: 16.3, 16.4, 16.5_


- [x] 3. Implement ZIP file handling
  - [x] 3.1 Create ZIPHandler class using miniz
    - Implement ZIP archive opening and enumeration
    - Implement file extraction to temporary directory
    - Implement ROM file filtering (.bin, .rom extensions)
    - Implement temporary file tracking for cleanup
    - _Requirements: 3.1, 3.2, 3.6, 3.8_
  
  - [ ]* 3.2 Write property test for ZIP extraction
    - **Property 11: ZIP files trigger extraction**
    - **Property 12: ZIP extraction filters ROM files**
    - **Validates: Requirements 3.1, 3.2**
  
  - [ ]* 3.3 Write property test for temporary file cleanup
    - **Property 14: Temporary file cleanup**
    - **Validates: Requirements 3.7**
  
  - [ ]* 3.4 Write unit tests for ZIPHandler
    - Test extracting ZIP with single ROM
    - Test extracting ZIP with multiple ROMs
    - Test extracting ZIP with no ROMs (should return error)
    - Test extracting corrupted ZIP (should return error)
    - Test temp file cleanup on destruction
    - _Requirements: 3.3, 3.4, 3.5, 3.7_

- [x] 4. Implement text rendering system
  - [x] 4.1 Create TextRenderer class
    - Implement SDL_ttf initialization and font loading
    - Implement text rendering with color and position
    - Implement text measurement (width/height calculation)
    - Implement fallback to SDL's built-in rendering if SDL_ttf fails
    - Support multiple font sizes (small, medium, large)
    - _Requirements: 1.5, 19.5_
  
  - [ ]* 4.2 Write unit tests for TextRenderer
    - Test font loading
    - Test text rendering
    - Test text measurement
    - Test fallback rendering
    - _Requirements: 1.5_

- [x] 5. Implement dialog system
  - [x] 5.1 Create MessageDialog class
    - Implement modal dialog rendering with title and message
    - Implement semi-transparent background overlay
    - Implement keyboard input handling (Enter/Escape to dismiss)
    - Implement text wrapping for long messages
    - _Requirements: 5.5, 6.1, 6.4_
  
  - [x] 5.2 Create ConfirmDialog class
    - Implement confirmation dialog with Yes/No options
    - Implement keyboard navigation (arrow keys, Enter, Escape)
    - Return true for confirmation, false for cancellation
    - _Requirements: 10.9_
  
  - [x] 5.3 Create ProgressDialog class
    - Implement progress message display
    - Implement show/hide methods
    - _Requirements: 5.1, 5.2, 5.3_
  
  - [ ]* 5.4 Write property test for dialog modal behavior
    - **Property 31: Error dialogs are modal**
    - **Validates: Requirements 6.4**
  
  - [ ]* 5.5 Write unit tests for dialog system
    - Test MessageDialog rendering and dismissal
    - Test ConfirmDialog navigation and selection
    - Test ProgressDialog show/hide
    - _Requirements: 5.5, 6.4, 10.9_

- [x] 6. Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.


- [x] 7. Implement file browser component
  - [x] 7.1 Create FileBrowser class
    - Implement directory scanning using std::filesystem
    - Implement file filtering by extension (case-insensitive)
    - Implement file entry display with name and size
    - Implement keyboard navigation (arrow keys, Enter, Escape)
    - Implement scrolling for long file lists
    - Implement parent directory navigation (..)
    - _Requirements: 2.1, 2.4, 2.5, 2.6, 2.7, 2.8_
  
  - [x] 7.2 Integrate FileBrowser with ConfigManager for directory memory
    - Load last used directory from config on open
    - Save directory to config when file is selected
    - Fall back to current working directory if last directory doesn't exist
    - _Requirements: 2.2, 2.3, 9.1, 9.2, 9.3, 9.5_
  
  - [ ]* 7.3 Write property test for file filtering
    - **Property 5: File browser filters by extension**
    - **Validates: Requirements 2.1**
  
  - [ ]* 7.4 Write property test for directory navigation
    - **Property 8: Directory navigation round-trip**
    - **Validates: Requirements 2.5**
  
  - [ ]* 7.5 Write property test for directory memory
    - **Property 6: File browser remembers last directory**
    - **Validates: Requirements 2.2, 9.2**
  
  - [ ]* 7.6 Write unit tests for FileBrowser
    - Test file filtering with mixed file types
    - Test directory navigation (enter/back)
    - Test scrolling with long file lists
    - Test with empty directory
    - Test with special characters in filenames
    - Test cancellation (Escape key)
    - _Requirements: 2.1, 2.5, 2.7, 2.8_

- [x] 8. Implement menu system
  - [x] 8.1 Create MenuItem structure and MenuSystem class
    - Implement menu item structure with label, action, submenu, enabled state
    - Implement menu stack for submenu navigation
    - Implement keyboard navigation (arrow keys, Enter, Escape)
    - Implement menu rendering with highlighting
    - Implement submenu indicators (">")
    - _Requirements: 1.1, 1.6, 1.7_
  
  - [x] 8.2 Build main menu structure
    - Create main menu with all required options
    - Create submenus for Video Settings, Audio Settings, Input Settings, etc.
    - Implement menu action enumeration
    - _Requirements: 4.1_
  
  - [ ]* 8.3 Write property test for menu navigation
    - **Property 4: Menu accepts keyboard navigation**
    - **Validates: Requirements 1.6**
  
  - [ ]* 8.4 Write unit tests for MenuSystem
    - Test menu navigation (up/down)
    - Test menu selection (Enter)
    - Test submenu navigation (enter/back)
    - Test menu rendering
    - _Requirements: 1.6, 1.7_


- [x] 9. Implement OSD (On-Screen Display) renderer
  - [x] 9.1 Create OSDRenderer class
    - Implement FPS display rendering
    - Implement notification display with timeout
    - Implement status indicator rendering
    - Implement configurable positioning (top-left, top-right, bottom-left, bottom-right)
    - Implement configurable font size
    - Implement configurable opacity
    - _Requirements: 7.2, 7.4, 19.3, 19.5, 19.7_
  
  - [ ]* 9.2 Write property test for OSD positioning
    - **Property 91: OSD position changes apply immediately**
    - **Validates: Requirements 19.4**
  
  - [ ]* 9.3 Write unit tests for OSDRenderer
    - Test FPS display rendering
    - Test notification display and timeout
    - Test positioning changes
    - Test font size changes
    - Test opacity changes
    - _Requirements: 7.2, 7.4, 19.4, 19.6, 19.8_

- [x] 10. Implement save state manager with thumbnails
  - [x] 10.1 Create SaveStateManager class
    - Implement save state file naming ({rom_name}.state{slot})
    - Implement save state directory management (saves/)
    - Implement thumbnail capture using current framebuffer
    - Implement thumbnail saving as PNG using stb_image_write
    - Implement save state listing with timestamps
    - Implement save state deletion with confirmation
    - _Requirements: 10.1, 10.4, 10.5, 10.7_
  
  - [ ]* 10.2 Write property test for save state round-trip
    - **Property 20: Save state round-trip**
    - **Validates: Requirements 4.10**
  
  - [ ]* 10.3 Write property test for save state file naming
    - **Property 19: Save state serialization**
    - **Validates: Requirements 4.8, 10.1**
  
  - [ ]* 10.4 Write property test for thumbnail capture
    - **Property 21: Save state creates thumbnail**
    - **Validates: Requirements 10.7**
  
  - [ ]* 10.5 Write unit tests for SaveStateManager
    - Test save state creation
    - Test save state loading
    - Test thumbnail capture
    - Test saves directory auto-creation
    - Test save state deletion
    - Test listing save states with timestamps
    - _Requirements: 4.8, 4.10, 10.1, 10.2, 10.3, 10.4, 10.5, 10.7, 10.9_

- [x] 11. Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.


- [x] 12. Implement input mapper for keyboard and joystick configuration
  - [x] 12.1 Create InputMapper class
    - Implement keyboard mapping storage and retrieval
    - Implement joystick mapping storage and retrieval
    - Implement joystick detection using SDL_NumJoysticks()
    - Implement joystick device assignment per player
    - Implement default keyboard mappings (Player 1: arrows+space, Player 2: WASD+shift)
    - Implement configuration persistence integration
    - _Requirements: 15.6, 15.7, 15.8, 15.9, 15.10, 15.11, 15.12_
  
  - [ ]* 12.2 Write property test for input remapping
    - **Property 69: Input remapping assigns input**
    - **Validates: Requirements 15.5**
  
  - [ ]* 12.3 Write property test for joystick detection
    - **Property 71: Joystick detection on startup**
    - **Validates: Requirements 15.7**
  
  - [x]* 12.4 Write unit tests for InputMapper
    - Test keyboard mapping
    - Test joystick mapping
    - Test joystick detection
    - Test device assignment
    - Test configuration persistence
    - _Requirements: 15.4, 15.5, 15.6, 15.7, 15.8, 15.11, 15.12_

- [x] 13. Implement recent files list management
  - [x] 13.1 Create RecentFilesList class
    - Implement list storage with maximum size of 10
    - Implement add operation with oldest entry eviction
    - Implement separate lists for ROMs and BIOS
    - Implement file existence checking
    - Implement configuration persistence integration
    - _Requirements: 11.1, 11.2, 11.3, 11.4, 11.5, 11.9, 11.10_
  
  - [ ]* 13.2 Write property test for recent list size limit
    - **Property 42: Recent lists limited to 10 entries**
    - **Validates: Requirements 11.4**
  
  - [ ]* 13.3 Write property test for recent list eviction
    - **Property 43: Recent list eviction policy**
    - **Validates: Requirements 11.5**
  
  - [ ]* 13.4 Write unit tests for RecentFilesList
    - Test adding files to list
    - Test list size limit
    - Test eviction of oldest entry
    - Test separate ROM and BIOS lists
    - Test file existence checking
    - Test configuration persistence
    - _Requirements: 11.1, 11.2, 11.3, 11.4, 11.5, 11.9, 11.10_


- [x] 14. Integrate menu system with SDLFrontend
  - [x] 14.1 Add menu system to SDLFrontend class
    - Add MenuSystem member to SDLFrontend
    - Implement F10 key handler to show/hide menu
    - Implement menu rendering in render_frame()
    - Implement emulator pause when menu is active
    - Implement menu input processing
    - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.6, 1.7_
  
  - [ ]* 14.2 Write property test for menu pause behavior
    - **Property 1: Menu activation pauses emulation**
    - **Property 2: Menu dismissal resumes emulation**
    - **Validates: Requirements 1.1, 1.2, 1.3, 1.7**
  
  - [ ]* 14.3 Write integration tests for menu system
    - Test F10 opens menu
    - Test Escape closes menu
    - Test emulator pauses when menu is active
    - Test emulator resumes when menu is closed
    - Test menu renders over game display
    - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.7_

- [x] 15. Implement menu actions for file loading
  - [x] 15.1 Implement "Load BIOS" menu action
    - Open FileBrowser with BIOS file filter
    - Handle ZIP files (extract and select)
    - Load BIOS into EmulatorCore
    - Display loading status and success/error messages
    - Update recent BIOS list
    - _Requirements: 4.2, 4.3, 5.1, 5.4, 5.5, 11.2_
  
  - [x] 15.2 Implement "Load ROM" menu action
    - Open FileBrowser with ROM and ZIP file filter
    - Handle ZIP files (extract and select)
    - Load ROM into EmulatorCore
    - Display loading status and success/error messages
    - Update recent ROM list
    - Update last used directory
    - _Requirements: 4.4, 4.5, 5.2, 5.4, 5.5, 9.1, 11.1_
  
  - [ ]* 15.3 Write property test for file loading
    - **Property 15: BIOS loading updates emulator state**
    - **Property 16: ROM loading updates emulator state**
    - **Validates: Requirements 4.3, 4.5**
  
  - [ ]* 15.4 Write property test for recent list updates
    - **Property 39: Recent ROM list updates on load**
    - **Property 40: Recent BIOS list updates on load**
    - **Validates: Requirements 11.1, 11.2**
  
  - [ ]* 15.5 Write integration tests for file loading
    - Test loading BIOS file
    - Test loading ROM file
    - Test loading ROM from ZIP
    - Test loading with invalid file (error handling)
    - Test recent list updates
    - Test directory memory updates
    - _Requirements: 4.2, 4.3, 4.4, 4.5, 9.1, 11.1, 11.2_


- [x] 16. Implement menu actions for save states
  - [x] 16.1 Implement "Save State" menu action
    - Display slot selection submenu (slots 0-9)
    - Show existing save timestamps and thumbnails
    - Save emulator state to selected slot
    - Capture and save thumbnail
    - Display success/error messages
    - _Requirements: 4.7, 4.8, 10.2, 10.6, 10.7_
  
  - [x] 16.2 Implement "Load State" menu action
    - Display slot selection submenu (slots 0-9)
    - Show existing save timestamps and thumbnails
    - Indicate empty slots
    - Load emulator state from selected slot
    - Display success/error messages
    - _Requirements: 4.9, 4.10, 10.2, 10.3, 10.6_
  
  - [x] 16.3 Implement "Delete Save State" menu action
    - Display slot selection submenu
    - Show confirmation dialog before deletion
    - Delete save state and thumbnail files
    - Display success/error messages
    - _Requirements: 10.8, 10.9_
  
  - [ ]* 16.4 Write property test for save state operations
    - **Property 19: Save state serialization**
    - **Property 20: Save state round-trip**
    - **Validates: Requirements 4.8, 4.10, 10.1**
  
  - [ ]* 16.5 Write integration tests for save state menu actions
    - Test saving state to slot
    - Test loading state from slot
    - Test deleting state with confirmation
    - Test thumbnail display in menu
    - Test empty slot indication
    - _Requirements: 4.7, 4.8, 4.9, 4.10, 10.2, 10.3, 10.6, 10.8, 10.9_

- [x] 17. Implement remaining menu actions
  - [x] 17.1 Implement "Reset" menu action
    - Reset EmulatorCore to initial state
    - Preserve loaded BIOS and ROM
    - _Requirements: 4.6_
  
  - [x] 17.2 Implement "Display Info" menu action
    - Show dialog with current ROM name, BIOS name, emulator version
    - _Requirements: 4.11_
  
  - [x] 17.3 Implement "Exit" menu action
    - Save configuration before exit
    - Clean up temporary files
    - Terminate application
    - _Requirements: 4.12, 16.4_
  
  - [ ]* 17.4 Write property test for reset behavior
    - **Property 17: Reset preserves loaded files**
    - **Validates: Requirements 4.6**
  
  - [ ]* 17.5 Write integration tests for menu actions
    - Test reset preserves BIOS and ROM
    - Test display info shows correct information
    - Test exit saves configuration
    - _Requirements: 4.6, 4.11, 4.12, 16.4_

- [x] 18. Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.


- [x] 19. Implement keyboard shortcuts
  - [x] 19.1 Add keyboard shortcut handlers to SDLFrontend
    - F3: Toggle FPS display
    - F5: Reset emulator (when debugger not active)
    - F6: Quick-save to slot 0
    - F7: Quick-load from slot 0
    - F11/Alt+Enter: Toggle fullscreen
    - F12: Save screenshot
    - Escape: Exit application (when menu not active)
    - Tab: Turbo mode (hold for fast-forward)
    - F4: Toggle audio mute
    - _Requirements: 8.1, 8.2, 8.3, 8.4, 8.5, 8.6, 8.7, 14.5, 18.1_
  
  - [ ]* 19.2 Write property test for FPS toggle
    - **Property 32: FPS toggle is idempotent**
    - **Validates: Requirements 7.1, 8.2**
  
  - [ ]* 19.3 Write property test for quick save/load
    - **Property 37: Quick save/load round-trip**
    - **Validates: Requirements 8.4, 8.5**
  
  - [ ]* 19.4 Write property test for audio mute toggle
    - **Property 61: Audio mute toggle is idempotent**
    - **Validates: Requirements 14.5**
  
  - [ ]* 19.5 Write integration tests for keyboard shortcuts
    - Test F3 toggles FPS display
    - Test F5 resets emulator
    - Test F6/F7 quick save/load
    - Test F11 toggles fullscreen
    - Test F12 saves screenshot
    - Test Tab activates turbo mode
    - Test F4 toggles audio mute
    - _Requirements: 8.2, 8.3, 8.4, 8.5, 8.6, 14.5, 18.1_

- [x] 20. Implement FPS display
  - [x] 20.1 Add FPS display to OSDRenderer
    - Implement FPS calculation and display
    - Implement F3 toggle handler
    - Implement configurable position
    - Update FPS once per second
    - Hide FPS when disabled
    - _Requirements: 7.1, 7.2, 7.4, 7.5_
  
  - [ ]* 20.2 Write property test for FPS display
    - **Property 33: FPS display shows when enabled**
    - **Property 35: FPS display hidden when disabled**
    - **Validates: Requirements 7.2, 7.5**
  
  - [ ]* 20.3 Write unit tests for FPS display
    - Test FPS toggle
    - Test FPS rendering when enabled
    - Test FPS not rendered when disabled
    - Test FPS update frequency
    - _Requirements: 7.1, 7.2, 7.4, 7.5_


- [x] 21. Implement fullscreen support
  - [x] 21.1 Add fullscreen toggle to SDLFrontend
    - Implement F11/Alt+Enter handler
    - Implement fullscreen mode using SDL_SetWindowFullscreen()
    - Implement aspect ratio preservation with letterboxing
    - Store and restore window size and position
    - Persist fullscreen preference to config
    - Restore fullscreen state on startup
    - _Requirements: 12.1, 12.2, 12.3, 12.4, 12.5, 12.6, 12.7_
  
  - [x] 21.2 Add "Toggle Fullscreen" menu option
    - Add menu item to main menu
    - Implement menu action handler
    - _Requirements: 12.9_
  
  - [ ]* 21.3 Write property test for fullscreen toggle
    - **Property 47: Fullscreen toggle is idempotent**
    - **Property 50: Fullscreen round-trip preserves window state**
    - **Validates: Requirements 12.1, 12.5**
  
  - [ ]* 21.4 Write property test for fullscreen persistence
    - **Property 51: Fullscreen preference persisted**
    - **Property 52: Fullscreen state restored on startup**
    - **Validates: Requirements 12.6, 12.7**
  
  - [ ]* 21.5 Write integration tests for fullscreen
    - Test F11 toggles fullscreen
    - Test Alt+Enter toggles fullscreen
    - Test aspect ratio preservation
    - Test window state restoration
    - Test menu accessible in fullscreen
    - Test fullscreen persistence
    - _Requirements: 12.1, 12.2, 12.3, 12.4, 12.5, 12.6, 12.7, 12.8_

- [x] 22. Implement video settings menu
  - [x] 22.1 Create Video Settings submenu
    - Add submenu with options: Scaling Filter, Aspect Ratio, CRT Effects, Scanlines
    - Implement scaling filter options (Nearest, Linear)
    - Implement aspect ratio options (Original, 4:3, Stretch)
    - Implement CRT effect options (None, Light, Medium, Heavy)
    - Implement scanline options (Off, 25%, 50%, 75%)
    - _Requirements: 13.1, 13.2, 13.3, 13.5, 13.9, 13.11_
  
  - [x] 22.2 Implement video settings application
    - Apply scaling filter changes immediately using SDL_SetHint()
    - Apply aspect ratio changes immediately by adjusting viewport
    - Apply CRT effects immediately (shader or post-processing)
    - Apply scanlines immediately (overlay rendering)
    - Persist video settings to config
    - _Requirements: 13.4, 13.6, 13.10, 13.12, 13.13_
  
  - [ ]* 22.3 Write property tests for video settings
    - **Property 54: Scaling filter changes apply immediately**
    - **Property 55: Aspect ratio changes apply immediately**
    - **Validates: Requirements 13.4, 13.6**
  
  - [ ]* 22.4 Write integration tests for video settings
    - Test scaling filter changes
    - Test aspect ratio changes
    - Test CRT effects
    - Test scanlines
    - Test settings persistence
    - _Requirements: 13.4, 13.6, 13.10, 13.12, 13.13_


- [x] 23. Implement audio settings menu
  - [x] 23.1 Create Audio Settings submenu
    - Add submenu with options: Volume, Mute, Audio Buffer Size
    - Implement volume slider (0-100% in 10% increments)
    - Implement mute toggle
    - Implement buffer size options (Small/512, Medium/1024, Large/2048)
    - _Requirements: 14.1, 14.2, 14.3, 14.8_
  
  - [x] 23.2 Implement audio settings application
    - Apply volume changes immediately
    - Apply mute toggle immediately (stop output, continue processing)
    - Display mute indicator when muted
    - Display warning for buffer size changes (requires restart/reload)
    - Persist audio settings to config
    - Restore audio settings on startup
    - _Requirements: 14.4, 14.5, 14.6, 14.7, 14.9, 14.10, 14.11, 14.12_
  
  - [ ]* 23.3 Write property tests for audio settings
    - **Property 60: Volume changes apply immediately**
    - **Property 63: Mute stops audio output**
    - **Validates: Requirements 14.4, 14.7**
  
  - [ ]* 23.4 Write property test for audio persistence
    - **Property 66: Audio settings persisted**
    - **Property 67: Audio state restored on startup**
    - **Validates: Requirements 14.11, 14.12**
  
  - [ ]* 23.5 Write integration tests for audio settings
    - Test volume changes
    - Test mute toggle
    - Test mute indicator display
    - Test buffer size warning
    - Test settings persistence
    - Test settings restoration on startup
    - _Requirements: 14.4, 14.5, 14.6, 14.7, 14.9, 14.10, 14.11, 14.12_

- [ ] 24. Implement input settings menu
  - [ ] 24.1 Create Input Settings submenu
    - Add submenu with options: Configure Player 1, Configure Player 2
    - Display all mappable actions (Up, Down, Left, Right, Button)
    - Display current input device for each player
    - Display current key/button mappings
    - _Requirements: 15.1, 15.2, 15.3, 15.9_
  
  - [ ] 24.2 Implement input remapping
    - Implement input capture mode (wait for key/button press)
    - Assign captured input to selected action
    - Support keyboard and joystick input
    - Display joystick selection for each player
    - Persist input mappings to config
    - Restore input mappings on startup
    - _Requirements: 15.4, 15.5, 15.6, 15.8, 15.11, 15.12_
  
  - [ ]* 24.3 Write property tests for input configuration
    - **Property 68: Input remapping captures input**
    - **Property 69: Input remapping assigns input**
    - **Validates: Requirements 15.4, 15.5**
  
  - [ ]* 24.4 Write property test for input persistence
    - **Property 74: Input mappings persisted**
    - **Property 75: Input mappings restored on startup**
    - **Validates: Requirements 15.11, 15.12**
  
  - [ ]* 24.5 Write integration tests for input settings
    - Test input remapping
    - Test keyboard input support
    - Test joystick input support
    - Test joystick selection
    - Test settings persistence
    - Test settings restoration on startup
    - _Requirements: 15.4, 15.5, 15.6, 15.8, 15.11, 15.12_

- [ ] 25. Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.


- [ ] 26. Implement screenshot capture
  - [ ] 26.1 Add screenshot capture to SDLFrontend
    - Implement F12 handler to capture screenshot
    - Implement screenshot saving with timestamp filename format
    - Support PNG, BMP, and TGA formats using stb_image_write
    - Store screenshots in screenshots/ subdirectory
    - Auto-create screenshots directory if missing
    - Display success notification after capture
    - _Requirements: 8.6, 17.1, 17.4, 17.5, 17.6, 17.7_
  
  - [ ] 26.2 Create Screenshot Settings submenu
    - Add submenu with format selection (PNG, BMP, TGA)
    - Persist screenshot format preference to config
    - _Requirements: 17.2, 17.3, 17.8_
  
  - [ ]* 26.3 Write property tests for screenshot capture
    - **Property 38: F12 captures screenshot**
    - **Property 80: Screenshot filename format**
    - **Validates: Requirements 8.6, 17.1, 17.4**
  
  - [ ]* 26.4 Write property test for screenshot directory
    - **Property 81: Screenshots stored in screenshots directory**
    - **Property 82: Screenshots directory auto-creation**
    - **Validates: Requirements 17.5, 17.6**
  
  - [ ]* 26.5 Write integration tests for screenshot capture
    - Test F12 captures screenshot
    - Test filename format
    - Test PNG format
    - Test BMP format
    - Test TGA format
    - Test directory auto-creation
    - Test success notification
    - Test format preference persistence
    - _Requirements: 8.6, 17.1, 17.2, 17.4, 17.5, 17.6, 17.7, 17.8_

- [ ] 27. Implement speed control and turbo mode
  - [ ] 27.1 Add speed control to SDLFrontend
    - Implement Tab key handler for turbo mode (hold for fast-forward)
    - Implement speed adjustment (25%, 50%, 100%, 200%, 400%, Unlimited)
    - Display speed indicator when running at non-100% speed
    - Turbo mode always runs at maximum speed
    - Return to normal speed when Tab is released
    - Persist normal speed setting to config
    - _Requirements: 18.1, 18.3, 18.4, 18.7, 18.8, 18.9_
  
  - [ ] 27.2 Create Speed Control submenu
    - Add submenu with speed options
    - Display current speed setting
    - _Requirements: 18.2_
  
  - [ ]* 27.3 Write property tests for speed control
    - **Property 85: Turbo mode activates on Tab hold**
    - **Property 88: Turbo mode is temporary**
    - **Property 90: Turbo mode ignores normal speed**
    - **Validates: Requirements 18.1, 18.7, 18.9**
  
  - [ ]* 27.4 Write integration tests for speed control
    - Test Tab activates turbo mode
    - Test Tab release returns to normal speed
    - Test speed changes apply immediately
    - Test speed indicator display
    - Test turbo mode at maximum speed
    - Test speed setting persistence
    - _Requirements: 18.1, 18.3, 18.4, 18.7, 18.8, 18.9_


- [ ] 28. Implement OSD customization menu
  - [ ] 28.1 Create OSD Settings submenu
    - Add submenu with options: FPS Position, Notification Position, Font Size, OSD Opacity
    - Implement position options (Top-Left, Top-Right, Bottom-Left, Bottom-Right)
    - Implement font size options (Small, Medium, Large)
    - Implement opacity options (25%, 50%, 75%, 100%)
    - _Requirements: 19.1, 19.2, 19.3, 19.5, 19.7_
  
  - [ ] 28.2 Implement OSD settings application
    - Apply position changes immediately to all OSD elements
    - Apply font size changes immediately to all OSD text
    - Apply opacity changes immediately to all OSD elements
    - Persist OSD settings to config
    - _Requirements: 19.4, 19.6, 19.8, 19.9_
  
  - [ ]* 28.3 Write property tests for OSD customization
    - **Property 91: OSD position changes apply immediately**
    - **Property 92: OSD font size changes apply immediately**
    - **Property 93: OSD opacity changes apply immediately**
    - **Validates: Requirements 19.4, 19.6, 19.8**
  
  - [ ]* 28.4 Write integration tests for OSD customization
    - Test position changes
    - Test font size changes
    - Test opacity changes
    - Test settings persistence
    - _Requirements: 19.4, 19.6, 19.8, 19.9_

- [ ] 29. Implement recent files menu integration
  - [ ] 29.1 Add recent files submenus
    - Add "Recent ROMs" submenu under "Load ROM"
    - Add "Recent BIOS" submenu under "Load BIOS"
    - Display up to 10 recent files in each submenu
    - Display file paths in menu
    - _Requirements: 11.6, 11.7_
  
  - [ ] 29.2 Implement recent file loading
    - Load file directly when selected from recent list (bypass file browser)
    - Handle missing files (display error and remove from list)
    - Update recent list on successful load
    - _Requirements: 11.8, 11.10_
  
  - [ ]* 29.3 Write property tests for recent files
    - **Property 44: Recent file loading bypasses browser**
    - **Property 46: Missing recent files removed**
    - **Validates: Requirements 11.8, 11.10**
  
  - [ ]* 29.4 Write integration tests for recent files menu
    - Test recent ROM submenu display
    - Test recent BIOS submenu display
    - Test loading from recent list
    - Test missing file handling
    - Test list updates
    - _Requirements: 11.6, 11.7, 11.8, 11.10_


- [ ] 30. Implement "Reset to Defaults" menu option
  - [ ] 30.1 Add "Reset to Defaults" menu option
    - Add menu item to main menu
    - Display confirmation dialog before reset
    - Reset all settings to default values
    - Save default configuration to file
    - Display success message
    - _Requirements: 16.7_
  
  - [ ]* 30.2 Write integration test for reset to defaults
    - Test confirmation dialog appears
    - Test all settings reset to defaults
    - Test configuration file updated
    - _Requirements: 16.7_

- [ ] 31. Implement configuration persistence on startup and exit
  - [ ] 31.1 Add configuration loading on startup
    - Load configuration file on SDLFrontend initialization
    - Apply all loaded settings (window, video, audio, input, OSD)
    - Use defaults if configuration file doesn't exist or is corrupted
    - _Requirements: 16.3, 16.5_
  
  - [ ] 31.2 Add configuration saving on exit
    - Save configuration file on normal SDLFrontend shutdown
    - Ensure all current settings are persisted
    - _Requirements: 16.4_
  
  - [ ]* 31.3 Write property test for configuration persistence
    - **Property 78: Configuration loading on startup**
    - **Property 79: Configuration saving on exit**
    - **Validates: Requirements 16.3, 16.4**
  
  - [ ]* 31.4 Write integration tests for configuration persistence
    - Test configuration loading on startup
    - Test configuration saving on exit
    - Test default values when file missing
    - Test default values when file corrupted
    - _Requirements: 16.3, 16.4, 16.5_

- [ ] 32. Implement error handling and user feedback
  - [ ] 32.1 Add error handling for file operations
    - Display error dialogs for file loading failures
    - Display error dialogs for ZIP extraction failures
    - Display error dialogs for save state failures
    - Log all errors to console
    - _Requirements: 6.1, 6.2, 6.3, 6.5_
  
  - [ ] 32.2 Add status messages for operations
    - Display "Loading BIOS..." during BIOS load
    - Display "Loading ROM..." during ROM load
    - Display "Extracting..." during ZIP extraction
    - Display success messages for 2 seconds
    - Display error messages until dismissed
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5_
  
  - [ ]* 32.3 Write property tests for error handling
    - **Property 29: Error messages require dismissal**
    - **Property 30: Errors displayed in UI and console**
    - **Validates: Requirements 5.5, 6.5**
  
  - [ ]* 32.4 Write integration tests for error handling
    - Test file loading error display
    - Test ZIP extraction error display
    - Test save state error display
    - Test error logging to console
    - Test status message display
    - Test success message timeout
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5, 6.1, 6.2, 6.3, 6.5_

- [ ] 33. Final checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.


- [ ] 34. Polish and refinement
  - [ ] 34.1 Improve UI aesthetics
    - Refine menu colors and contrast
    - Refine dialog appearance
    - Refine OSD appearance
    - Ensure consistent styling across all UI elements
    - _Requirements: 1.5_
  
  - [ ] 34.2 Optimize performance
    - Profile menu rendering performance
    - Optimize file browser for large directories
    - Optimize OSD rendering
    - Ensure smooth 60fps during gameplay with OSD active
  
  - [ ] 34.3 Add keyboard navigation hints
    - Display keyboard hints in menu footer (e.g., "↑↓: Navigate, Enter: Select, Esc: Back")
    - Display keyboard hints in file browser
    - Display keyboard hints in dialogs
  
  - [ ] 34.4 Test on multiple platforms
    - Test on Windows
    - Test on Linux
    - Test on macOS
    - Verify configuration file paths work correctly on all platforms
    - Verify file browser works correctly on all platforms

- [ ] 35. Documentation and cleanup
  - [ ] 35.1 Update user documentation
    - Document all keyboard shortcuts
    - Document menu system usage
    - Document configuration file format
    - Document save state management
  
  - [ ] 35.2 Update developer documentation
    - Document new classes and interfaces
    - Document configuration system
    - Document UI component architecture
    - Add code comments for complex logic
  
  - [ ] 35.3 Clean up code
    - Remove debug logging
    - Remove commented-out code
    - Ensure consistent code style
    - Run static analysis tools

## Notes

- Tasks marked with `*` are optional property-based tests and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Checkpoints ensure incremental validation at major milestones
- Property tests validate universal correctness properties from the design document
- Unit tests validate specific examples, edge cases, and integration points
- The implementation follows a bottom-up approach: foundational components first, then UI, then integration
- Configuration persistence is integrated throughout to ensure all settings are saved and restored
- Error handling is comprehensive to provide good user experience even when things go wrong

