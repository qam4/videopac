# Design Document: SDL Frontend Enhancements

## Overview

This design document describes the architecture and implementation approach for enhancing the SDL frontend of the Videopac/Odyssey2 emulator with a comprehensive in-game menu system, file browser, ZIP file support, and quality-of-life improvements. The enhancements will transform the emulator from a command-line-driven application into a modern, user-friendly experience with full runtime configuration and file management capabilities.

The design follows a modular architecture with clear separation between UI components (menu system, file browser, dialogs), emulator integration (state management, file loading), and configuration persistence. All UI elements will be rendered using SDL2's rendering API with a simple, readable font system that overlays the emulated display.

### Key Design Principles

1. **Non-intrusive UI**: All UI elements overlay the emulated display without disrupting the underlying framebuffer
2. **Pause-on-menu**: Emulation pauses when any UI is active to prevent gameplay disruption
3. **Keyboard-driven**: All UI navigation uses keyboard input (arrow keys, Enter, Escape)
4. **Persistent configuration**: All user preferences are saved to a configuration file and restored on startup
5. **Graceful degradation**: If optional features fail (e.g., ZIP extraction), the emulator continues with error messages

## Architecture

### Component Overview

The enhanced SDL frontend consists of these major components:

```
SDLFrontend (existing)
├── MenuSystem (new)
│   ├── MenuItem
│   ├── MenuRenderer
│   └── MenuNavigator
├── FileBrowser (new)
│   ├── FileEntry
│   ├── DirectoryNavigator
│   └── FileFilter
├── DialogSystem (new)
│   ├── MessageDialog
│   ├── ConfirmDialog
│   └── ProgressDialog
├── ConfigManager (new)
│   ├── ConfigFile (INI format)
│   └── ConfigSerializer
├── ZIPHandler (new)
│   └── miniz integration
├── SaveStateManager (new)
│   ├── StateSerializer
│   └── ThumbnailCapture
├── OSDRenderer (new)
│   ├── FPSDisplay
│   ├── NotificationDisplay
│   └── StatusIndicators
└── InputMapper (new)
    ├── KeyboardMapper
    └── JoystickMapper
```

### Data Flow

1. **Menu Activation**: User presses F10 → SDLFrontend pauses emulation → MenuSystem renders overlay → User navigates menu
2. **File Loading**: User selects "Load ROM" → FileBrowser opens → User selects file → ZIPHandler extracts if needed → EmulatorCore loads ROM
3. **Configuration**: User changes settings → ConfigManager updates in-memory config → On exit, ConfigManager persists to disk
4. **Save States**: User presses F6 → SaveStateManager captures emulator state + thumbnail → Serializes to disk



## Components and Interfaces

### MenuSystem

The MenuSystem manages the in-game overlay menu, handling menu structure, navigation, and rendering.

**MenuSystem Class**:
```cpp
class MenuSystem {
public:
    MenuSystem(SDL_Renderer* renderer);
    
    void show();
    void hide();
    bool is_visible() const;
    
    MenuAction process_input(SDL_KeyCode key);
    void render();
    
    void set_menu_items(const std::vector<MenuItem>& items);
    void navigate_up();
    void navigate_down();
    void select_current();
    void go_back();
    
private:
    SDL_Renderer* renderer_;
    std::vector<MenuItem> items_;
    int selected_index_;
    std::stack<std::vector<MenuItem>> menu_stack_;  // For submenus
    bool visible_;
};
```

**MenuItem Structure**:
```cpp
struct MenuItem {
    std::string label;
    MenuAction action;
    std::vector<MenuItem> submenu;  // Empty if no submenu
    bool enabled;
    std::string value;  // For displaying current setting values
};

enum class MenuAction {
    None,
    LoadBIOS,
    LoadROM,
    Reset,
    SaveState,
    LoadState,
    DisplayInfo,
    Exit,
    ToggleFullscreen,
    VideoSettings,
    AudioSettings,
    InputSettings,
    ScreenshotSettings,
    SpeedControl,
    OSDSettings,
    ResetToDefaults
};
```

**MenuRenderer**:
- Renders menu background (semi-transparent overlay)
- Renders menu items with highlighting for selected item
- Renders submenu indicators (">") for items with submenus
- Renders current setting values for configuration items
- Uses SDL_ttf for text rendering or custom bitmap font

### FileBrowser

The FileBrowser provides a file selection interface with directory navigation and filtering.

**FileBrowser Class**:
```cpp
class FileBrowser {
public:
    FileBrowser(SDL_Renderer* renderer);
    
    std::optional<std::string> show(const std::string& start_dir, 
                                     const std::vector<std::string>& extensions);
    
    void process_input(SDL_KeyCode key);
    void render();
    
private:
    void scan_directory(const std::string& path);
    void navigate_up();
    void navigate_down();
    void enter_directory();
    void go_to_parent();
    std::string select_current();
    
    SDL_Renderer* renderer_;
    std::string current_dir_;
    std::vector<FileEntry> entries_;
    int selected_index_;
    int scroll_offset_;
    std::vector<std::string> extensions_;
};
```

**FileEntry Structure**:
```cpp
struct FileEntry {
    std::string name;
    std::string full_path;
    bool is_directory;
    size_t file_size;
};
```

**Directory Navigation**:
- Uses `std::filesystem` for cross-platform directory operations
- Filters files by extension (case-insensitive)
- Shows ".." entry for parent directory navigation
- Displays file sizes in human-readable format (KB, MB)
- Scrolls list if more entries than fit on screen

### DialogSystem

The DialogSystem provides modal dialogs for messages, confirmations, and progress indicators.

**MessageDialog**:
```cpp
class MessageDialog {
public:
    MessageDialog(SDL_Renderer* renderer);
    
    void show(const std::string& title, const std::string& message);
    bool wait_for_dismiss();  // Returns true when user presses Enter/Escape
    void render();
    
private:
    SDL_Renderer* renderer_;
    std::string title_;
    std::string message_;
    bool visible_;
};
```

**ConfirmDialog**:
```cpp
class ConfirmDialog {
public:
    ConfirmDialog(SDL_Renderer* renderer);
    
    bool show(const std::string& title, const std::string& message);
    // Returns true if user confirms (Enter), false if cancels (Escape)
    
private:
    SDL_Renderer* renderer_;
    std::string title_;
    std::string message_;
};
```

**ProgressDialog**:
```cpp
class ProgressDialog {
public:
    ProgressDialog(SDL_Renderer* renderer);
    
    void show(const std::string& message);
    void hide();
    void render();
    
private:
    SDL_Renderer* renderer_;
    std::string message_;
    bool visible_;
};
```

### ZIPHandler

The ZIPHandler manages ZIP file extraction using the miniz library.

**ZIPHandler Class**:
```cpp
class ZIPHandler {
public:
    ZIPHandler();
    ~ZIPHandler();
    
    Result<std::vector<std::string>> list_rom_files(const std::string& zip_path);
    Result<std::string> extract_file(const std::string& zip_path, 
                                      const std::string& filename);
    void cleanup_temp_files();
    
private:
    std::string temp_dir_;
    std::vector<std::string> temp_files_;
};
```

**ZIP Extraction Process**:
1. Open ZIP archive using miniz
2. Enumerate files, filter for .bin/.rom extensions
3. If multiple ROMs found, return list for user selection
4. If single ROM found, extract to temp directory
5. Return path to extracted file
6. Track temp files for cleanup on exit

**Temporary File Management**:
- Create temp directory in system temp location
- Use unique names to avoid conflicts
- Clean up on normal exit
- Use RAII to ensure cleanup even on exceptions

### ConfigManager

The ConfigManager handles loading, saving, and managing configuration settings.

**ConfigManager Class**:
```cpp
class ConfigManager {
public:
    ConfigManager();
    
    bool load(const std::string& config_path);
    bool save(const std::string& config_path);
    void reset_to_defaults();
    
    // Getters and setters for all configuration options
    std::string get_last_directory() const;
    void set_last_directory(const std::string& dir);
    
    std::vector<std::string> get_recent_roms() const;
    void add_recent_rom(const std::string& path);
    
    bool get_fullscreen() const;
    void set_fullscreen(bool enabled);
    
    // ... (similar for all settings)
    
private:
    std::map<std::string, std::string> config_;
    
    void parse_ini_file(std::istream& input);
    void write_ini_file(std::ostream& output);
};
```

**Configuration File Format (INI)**:
```ini
[General]
last_directory=/home/user/roms
fullscreen=false
show_fps=false

[Window]
width=960
height=720
position_x=100
position_y=100

[Video]
scaling_filter=nearest
aspect_ratio=4:3
vsync=true
crt_effect=none
scanlines=off

[Audio]
volume=100
muted=false
buffer_size=1024

[Input]
player1_device=keyboard
player1_up=Up
player1_down=Down
player1_left=Left
player1_right=Right
player1_button=Space

[RecentROMs]
rom1=/path/to/game1.bin
rom2=/path/to/game2.bin

[RecentBIOS]
bios1=/path/to/bios.bin

[OSD]
fps_position=top-right
notification_position=bottom-left
font_size=medium
opacity=75

[Screenshot]
format=png
```

**Configuration File Location**:
- Windows: `%APPDATA%/videopac/videopac.cfg`
- Linux/macOS: `~/.config/videopac/videopac.cfg`
- Use `SDL_GetPrefPath()` for cross-platform path resolution



### SaveStateManager

The SaveStateManager handles save state operations with thumbnail support.

**SaveStateManager Class**:
```cpp
class SaveStateManager {
public:
    SaveStateManager(EmulatorCore* emulator, SDL_Renderer* renderer);
    
    Result<void> save_state(int slot, const std::string& rom_name);
    Result<void> load_state(int slot, const std::string& rom_name);
    Result<void> delete_state(int slot, const std::string& rom_name);
    
    std::vector<SaveStateInfo> list_states(const std::string& rom_name);
    
private:
    std::string get_state_filename(const std::string& rom_name, int slot);
    std::string get_thumbnail_filename(const std::string& rom_name, int slot);
    
    Result<void> capture_thumbnail(const std::string& filename);
    
    EmulatorCore* emulator_;
    SDL_Renderer* renderer_;
    std::string saves_dir_;
};
```

**SaveStateInfo Structure**:
```cpp
struct SaveStateInfo {
    int slot;
    std::string filename;
    std::string thumbnail_path;
    std::time_t timestamp;
    bool exists;
};
```

**Save State File Format**:
- Filename: `{rom_name}.state{slot}` (e.g., `satellite_attack.state1`)
- Thumbnail: `{rom_name}.state{slot}.png` (e.g., `satellite_attack.state1.png`)
- Location: `saves/` subdirectory in config directory
- Format: Binary serialization of emulator state (existing savestate.cpp functionality)

**Thumbnail Capture**:
- Capture current framebuffer from emulator
- Scale down to 160x120 (thumbnail size)
- Save as PNG using SDL_image or stb_image_write
- Store alongside save state file

### OSDRenderer

The OSDRenderer manages on-screen display elements like FPS counter, notifications, and status indicators.

**OSDRenderer Class**:
```cpp
class OSDRenderer {
public:
    OSDRenderer(SDL_Renderer* renderer);
    
    void render_fps(float fps, OSDPosition position);
    void render_notification(const std::string& message, OSDPosition position);
    void render_status_indicator(const std::string& icon, OSDPosition position);
    
    void set_font_size(FontSize size);
    void set_opacity(int opacity);  // 0-100
    
    void show_notification(const std::string& message, int duration_ms);
    void update(uint32_t current_time);  // For notification timeout
    
private:
    SDL_Renderer* renderer_;
    FontSize font_size_;
    int opacity_;
    
    std::string current_notification_;
    uint32_t notification_start_time_;
    int notification_duration_;
};
```

**OSD Positioning**:
```cpp
enum class OSDPosition {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

enum class FontSize {
    Small,
    Medium,
    Large
};
```

**Rendering Details**:
- Use SDL_ttf for text rendering or custom bitmap font
- Apply opacity using SDL_SetTextureAlphaMod
- Position text with padding from screen edges
- Use contrasting colors (white text with black outline/shadow)

### InputMapper

The InputMapper manages keyboard and joystick input configuration.

**InputMapper Class**:
```cpp
class InputMapper {
public:
    InputMapper();
    
    void set_keyboard_mapping(int player, Action action, SDL_Keycode key);
    void set_joystick_mapping(int player, Action action, JoystickInput input);
    
    SDL_Keycode get_keyboard_mapping(int player, Action action) const;
    JoystickInput get_joystick_mapping(int player, Action action) const;
    
    void set_player_device(int player, InputDevice device);
    InputDevice get_player_device(int player) const;
    
    void detect_joysticks();
    std::vector<JoystickInfo> get_connected_joysticks() const;
    
    void load_from_config(const ConfigManager& config);
    void save_to_config(ConfigManager& config);
    
private:
    std::map<std::pair<int, Action>, SDL_Keycode> keyboard_mappings_;
    std::map<std::pair<int, Action>, JoystickInput> joystick_mappings_;
    std::map<int, InputDevice> player_devices_;
    std::vector<SDL_Joystick*> joysticks_;
};
```

**Input Structures**:
```cpp
enum class Action {
    Up,
    Down,
    Left,
    Right,
    Button
};

enum class InputDevice {
    Keyboard,
    Joystick0,
    Joystick1,
    Joystick2,
    Joystick3
};

struct JoystickInput {
    int joystick_id;
    int button;  // -1 if axis
    int axis;    // -1 if button
    int axis_direction;  // -1 or 1
};

struct JoystickInfo {
    int id;
    std::string name;
};
```

**Default Keyboard Mappings**:
- Player 1: Arrow keys + Space
- Player 2: WASD + Left Shift

**Joystick Detection**:
- Detect joysticks on startup using SDL_NumJoysticks()
- Handle SDL_JOYDEVICEADDED and SDL_JOYDEVICEREMOVED events
- Store joystick names for display in menu



## Data Models

### Menu Structure

The menu system uses a hierarchical structure with the following main menu:

```
Main Menu
├── Load BIOS
│   ├── Browse...
│   └── Recent BIOS
│       ├── bios1.bin
│       └── bios2.bin
├── Load ROM
│   ├── Browse...
│   └── Recent ROMs
│       ├── game1.bin
│       ├── game2.bin
│       └── ...
├── Save State
│   ├── Slot 1 [Empty]
│   ├── Slot 2 [2024-01-15 14:30]
│   └── ...
├── Load State
│   ├── Slot 1 [Empty]
│   ├── Slot 2 [2024-01-15 14:30]
│   └── ...
├── Reset
├── Video Settings
│   ├── Scaling Filter: Nearest
│   ├── Aspect Ratio: 4:3
│   ├── VSync: On
│   ├── CRT Effects: None
│   └── Scanlines: Off
├── Audio Settings
│   ├── Volume: 100%
│   ├── Mute: Off
│   └── Buffer Size: Medium
├── Input Settings
│   ├── Configure Player 1
│   │   ├── Device: Keyboard
│   │   ├── Up: Arrow Up
│   │   ├── Down: Arrow Down
│   │   ├── Left: Arrow Left
│   │   ├── Right: Arrow Right
│   │   └── Button: Space
│   └── Configure Player 2
│       └── ...
├── Screenshot Settings
│   └── Format: PNG
├── Speed Control
│   ├── 25%
│   ├── 50%
│   ├── 100% [Current]
│   ├── 200%
│   ├── 400%
│   └── Unlimited
├── OSD Settings
│   ├── FPS Position: Top-Right
│   ├── Notification Position: Bottom-Left
│   ├── Font Size: Medium
│   └── Opacity: 75%
├── Toggle Fullscreen
├── Display Info
├── Reset to Defaults
└── Exit
```

### Configuration Data Model

The configuration is stored as key-value pairs organized into sections:

**General Section**:
- `last_directory`: Last used directory for file browser
- `fullscreen`: Fullscreen mode enabled
- `show_fps`: FPS display enabled

**Window Section**:
- `width`: Window width in pixels
- `height`: Window height in pixels
- `position_x`: Window X position
- `position_y`: Window Y position

**Video Section**:
- `scaling_filter`: "nearest" or "linear"
- `aspect_ratio`: "original", "4:3", or "stretch"
- `vsync`: "true" or "false"
- `crt_effect`: "none", "light", "medium", or "heavy"
- `scanlines`: "off", "25", "50", or "75"

**Audio Section**:
- `volume`: 0-100
- `muted`: "true" or "false"
- `buffer_size`: 512, 1024, or 2048

**Input Section** (per player):
- `player{N}_device`: "keyboard" or "joystick{N}"
- `player{N}_up`: Key name or joystick input
- `player{N}_down`: Key name or joystick input
- `player{N}_left`: Key name or joystick input
- `player{N}_right`: Key name or joystick input
- `player{N}_button`: Key name or joystick input

**Recent Files Sections**:
- `RecentROMs`: List of ROM paths (rom1, rom2, ...)
- `RecentBIOS`: List of BIOS paths (bios1, bios2, ...)

**OSD Section**:
- `fps_position`: "top-left", "top-right", "bottom-left", "bottom-right"
- `notification_position`: Same as fps_position
- `font_size`: "small", "medium", "large"
- `opacity`: 0-100

**Screenshot Section**:
- `format`: "png", "bmp", or "tga"

**Speed Section**:
- `normal_speed`: 25, 50, 100, 200, 400, or 0 (unlimited)

### Save State Data Model

Save states are stored using the existing savestate serialization format with additional metadata:

**State File Structure**:
```
saves/
├── {rom_name}.state0       # Quick save slot
├── {rom_name}.state0.png   # Quick save thumbnail
├── {rom_name}.state1       # Manual save slot 1
├── {rom_name}.state1.png   # Manual save thumbnail 1
└── ...
```

**State File Contents** (existing format from savestate.cpp):
- Magic number and version
- CPU state (registers, flags)
- Memory state (RAM, VRAM)
- VDC state (registers, internal state)
- Audio state (sound generator state)
- Input state (joystick/keyboard state)

**Thumbnail Format**:
- PNG format, 160x120 pixels
- RGB24 color
- Scaled down from current framebuffer



## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property Reflection

After analyzing all acceptance criteria, I identified several areas of redundancy:

1. **Menu activation properties** (1.1, 8.1): Both test F10 opening the menu - can be combined
2. **FPS toggle properties** (7.1, 8.2): Both test F3 toggling FPS - can be combined
3. **Screenshot properties** (8.6, 17.1): Both test F12 screenshot - can be combined
4. **Directory memory properties** (2.2, 9.2): Both test last directory retrieval - can be combined
5. **Configuration persistence**: Many requirements test "persist to config file" - these can be combined into comprehensive properties
6. **Round-trip properties**: Several requirements test state preservation (menu show/hide, fullscreen toggle, save/load state) - these are natural round-trip properties

The following properties eliminate this redundancy while maintaining comprehensive coverage.

### Menu System Properties

Property 1: Menu activation pauses emulation
*For any* emulator state, when the menu is activated (F10 pressed), the emulator should pause execution and the menu should be visible.
**Validates: Requirements 1.1, 1.2, 8.1**

Property 2: Menu dismissal resumes emulation
*For any* emulator state, showing the menu then dismissing it (Escape pressed) should resume emulation from the same state.
**Validates: Requirements 1.3, 1.7**

Property 3: Menu renders over paused display
*For any* emulator state, when the menu is active, the underlying framebuffer should still be rendered beneath the menu overlay.
**Validates: Requirements 1.4**

Property 4: Menu accepts keyboard navigation
*For any* menu state, arrow keys should navigate menu items and Enter should select the current item.
**Validates: Requirements 1.6**

### File Browser Properties

Property 5: File browser filters by extension
*For any* directory containing mixed file types, the file browser should only display files matching the specified extensions (.bin, .rom, .zip).
**Validates: Requirements 2.1**

Property 6: File browser remembers last directory
*For any* valid directory path, if set as the last used directory, opening the file browser should start in that directory.
**Validates: Requirements 2.2, 9.2**

Property 7: File entries contain required information
*For any* file displayed in the browser, the entry should contain both the filename and file size.
**Validates: Requirements 2.4**

Property 8: Directory navigation round-trip
*For any* directory, navigating into a subdirectory then to parent should return to the original directory.
**Validates: Requirements 2.5**

Property 9: File selection returns full path
*For any* file selected in the browser, the returned path should be the complete absolute path to that file.
**Validates: Requirements 2.6**

Property 10: File browser accepts keyboard navigation
*For any* file browser state, arrow keys should navigate entries and Enter should select the current entry.
**Validates: Requirements 2.7**

### ZIP File Handling Properties

Property 11: ZIP files trigger extraction
*For any* valid ZIP file selected for loading, the system should extract the archive contents.
**Validates: Requirements 3.1**

Property 12: ZIP extraction filters ROM files
*For any* ZIP archive, extraction should identify all files with .bin or .rom extensions.
**Validates: Requirements 3.2**

Property 13: ZIP extraction creates temporary files
*For any* ROM extracted from a ZIP, the file should be placed in a temporary location.
**Validates: Requirements 3.6**

Property 14: Temporary file cleanup
*For any* temporary files created during ZIP extraction, exiting the emulator should remove all temporary files.
**Validates: Requirements 3.7**

### File Loading Properties

Property 15: BIOS loading updates emulator state
*For any* valid BIOS file, loading it should update the emulator core's BIOS state.
**Validates: Requirements 4.3**

Property 16: ROM loading updates emulator state
*For any* valid ROM file, loading it should update the emulator core's ROM state.
**Validates: Requirements 4.5**

Property 17: Reset preserves loaded files
*For any* emulator state with loaded BIOS and ROM, resetting should preserve the same BIOS and ROM.
**Validates: Requirements 4.6**

Property 18: File loading updates last directory
*For any* file successfully loaded, the directory containing that file should become the last used directory.
**Validates: Requirements 9.1**

### Save State Properties

Property 19: Save state serialization
*For any* emulator state, saving to a slot should create a file with the correct naming format "{rom_name}.state{slot}".
**Validates: Requirements 4.8, 10.1**

Property 20: Save state round-trip
*For any* emulator state, saving to a slot then loading from that slot should restore an equivalent emulator state.
**Validates: Requirements 4.10**

Property 21: Save state creates thumbnail
*For any* save state operation, a thumbnail image should be created alongside the state file.
**Validates: Requirements 10.7**

Property 22: Save states stored in saves directory
*For any* save state file, it should be located in the "saves" subdirectory.
**Validates: Requirements 10.4**

Property 23: Saves directory auto-creation
*For any* save state operation, if the "saves" directory doesn't exist, it should be created automatically.
**Validates: Requirements 10.5**

Property 24: Save state display shows timestamps
*For any* existing save state, the menu display should show the file's timestamp.
**Validates: Requirements 10.2**

Property 25: Empty save slots indicated
*For any* save slot without a file, the menu display should indicate it as empty.
**Validates: Requirements 10.3**

Property 26: Save state deletion requires confirmation
*For any* save state deletion request, a confirmation dialog should be displayed before deletion.
**Validates: Requirements 10.9**

### Status and Error Display Properties

Property 27: Loading operations show status messages
*For any* file loading operation (BIOS, ROM, ZIP), a status message should be displayed during the operation.
**Validates: Requirements 5.1, 5.2, 5.3**

Property 28: Success messages have timeout
*For any* successful file operation, the success message should be displayed for approximately 2 seconds.
**Validates: Requirements 5.4**

Property 29: Error messages require dismissal
*For any* failed file operation, the error message should remain visible until the user dismisses it.
**Validates: Requirements 5.5**

Property 30: Errors displayed in UI and console
*For any* error condition, the error should be both displayed in the UI and logged to the console.
**Validates: Requirements 6.5**

Property 31: Error dialogs are modal
*For any* error dialog displayed, the system should wait for user acknowledgment before proceeding.
**Validates: Requirements 6.4**

### FPS Display Properties

Property 32: FPS toggle is idempotent
*For any* FPS display state, toggling it twice (F3 pressed twice) should return to the original state.
**Validates: Requirements 7.1, 8.2**

Property 33: FPS display shows when enabled
*For any* emulator state with FPS display enabled, the current FPS value should be rendered on screen.
**Validates: Requirements 7.2**

Property 34: FPS display updates periodically
*For any* emulator state with FPS display enabled, the FPS value should update approximately once per second.
**Validates: Requirements 7.4**

Property 35: FPS display hidden when disabled
*For any* emulator state with FPS display disabled, no FPS information should be rendered.
**Validates: Requirements 7.5**

### Keyboard Shortcut Properties

Property 36: F5 resets emulator
*For any* emulator state (when debugger is not active), pressing F5 should reset the emulator core.
**Validates: Requirements 8.3**

Property 37: Quick save/load round-trip
*For any* emulator state, pressing F6 (quick-save) then F7 (quick-load) should restore an equivalent state.
**Validates: Requirements 8.4, 8.5**

Property 38: F12 captures screenshot
*For any* emulator state, pressing F12 should create a screenshot file.
**Validates: Requirements 8.6, 17.1**

### Recent Files Properties

Property 39: Recent ROM list updates on load
*For any* ROM file successfully loaded, it should be added to the recent ROMs list.
**Validates: Requirements 11.1**

Property 40: Recent BIOS list updates on load
*For any* BIOS file successfully loaded, it should be added to the recent BIOS list.
**Validates: Requirements 11.2**

Property 41: Recent lists are separate
*For any* file loaded, ROM files should only appear in the recent ROMs list and BIOS files should only appear in the recent BIOS list.
**Validates: Requirements 11.3**

Property 42: Recent lists limited to 10 entries
*For any* recent files list, it should contain at most 10 entries.
**Validates: Requirements 11.4**

Property 43: Recent list eviction policy
*For any* recent files list with 10 entries, adding a new entry should remove the oldest entry.
**Validates: Requirements 11.5**

Property 44: Recent file loading bypasses browser
*For any* file in the recent files list, selecting it should load the file directly without opening the file browser.
**Validates: Requirements 11.8**

Property 45: Recent lists persisted
*For any* recent files list, it should be saved to the configuration file.
**Validates: Requirements 11.9**

Property 46: Missing recent files removed
*For any* recent file that no longer exists, attempting to load it should display an error and remove it from the list.
**Validates: Requirements 11.10**

### Fullscreen Properties

Property 47: Fullscreen toggle is idempotent
*For any* window state, toggling fullscreen twice (F11 or Alt+Enter pressed twice) should return to the original window state.
**Validates: Requirements 12.1**

Property 48: Fullscreen uses desktop resolution
*For any* system, entering fullscreen mode should use the current desktop resolution.
**Validates: Requirements 12.2**

Property 49: Fullscreen maintains aspect ratio
*For any* fullscreen mode, the emulator display should maintain the correct Videopac aspect ratio with letterboxing if needed.
**Validates: Requirements 12.3, 12.4**

Property 50: Fullscreen round-trip preserves window state
*For any* window state (size and position), entering fullscreen then exiting should restore the original window state.
**Validates: Requirements 12.5**

Property 51: Fullscreen preference persisted
*For any* fullscreen state, it should be saved to the configuration file.
**Validates: Requirements 12.6**

Property 52: Fullscreen state restored on startup
*For any* saved fullscreen preference, starting the emulator should restore that fullscreen state.
**Validates: Requirements 12.7**

Property 53: Menu accessible in fullscreen
*For any* fullscreen state, the menu system should be accessible and visible.
**Validates: Requirements 12.8**

### Video Settings Properties

Property 54: Scaling filter changes apply immediately
*For any* scaling filter selection, changing it should immediately update the display rendering.
**Validates: Requirements 13.4**

Property 55: Aspect ratio changes apply immediately
*For any* aspect ratio mode selection, changing it should immediately adjust the display viewport.
**Validates: Requirements 13.6**

Property 56: VSync toggle applies immediately
*For any* VSync state, toggling it should immediately enable or disable vertical synchronization.
**Validates: Requirements 13.8**

Property 57: CRT effects apply immediately
*For any* CRT effect level, changing it should immediately apply the corresponding shader or post-processing effect.
**Validates: Requirements 13.10**

Property 58: Scanlines apply immediately
*For any* scanline level, changing it should immediately render the corresponding scanline overlay.
**Validates: Requirements 13.12**

Property 59: Video settings persisted
*For any* video setting change, it should be saved to the configuration file.
**Validates: Requirements 13.13**

### Audio Settings Properties

Property 60: Volume changes apply immediately
*For any* volume level selection, changing it should immediately adjust the audio output volume.
**Validates: Requirements 14.4**

Property 61: Audio mute toggle is idempotent
*For any* audio mute state, toggling it twice (F4 pressed twice) should return to the original state.
**Validates: Requirements 14.5**

Property 62: Mute indicator displayed when muted
*For any* audio state with mute enabled, a mute indicator should be displayed on screen.
**Validates: Requirements 14.6**

Property 63: Mute stops audio output
*For any* audio state with mute enabled, no audio should be output (but audio processing continues).
**Validates: Requirements 14.7**

Property 64: Buffer size change reinitializes audio
*For any* audio buffer size selection, changing it should reinitialize the audio subsystem with the new buffer size.
**Validates: Requirements 14.9**

Property 65: Buffer size change shows warning
*For any* audio buffer size change, a warning message should be displayed about requiring restart or ROM reload.
**Validates: Requirements 14.10**

Property 66: Audio settings persisted
*For any* audio setting change, it should be saved to the configuration file.
**Validates: Requirements 14.11**

Property 67: Audio state restored on startup
*For any* saved audio settings (volume and mute state), starting the emulator should restore those settings.
**Validates: Requirements 14.12**

### Input Configuration Properties

Property 68: Input remapping captures input
*For any* action selected for remapping, the system should wait for and capture the next keyboard or joystick input.
**Validates: Requirements 15.4**

Property 69: Input remapping assigns input
*For any* input received during remapping, it should be assigned to the selected action.
**Validates: Requirements 15.5**

Property 70: Input supports keyboard and joystick
*For any* player configuration, both keyboard keys and joystick buttons/axes should be supported for input.
**Validates: Requirements 15.6**

Property 71: Joystick detection on startup
*For any* connected joysticks, they should be detected when the emulator starts.
**Validates: Requirements 15.7**

Property 72: Joystick assignment per player
*For any* player with multiple joysticks connected, the user should be able to select which joystick is assigned to that player.
**Validates: Requirements 15.8**

Property 73: Input device display
*For any* player, the current input device (keyboard or joystick) should be displayed in the configuration menu.
**Validates: Requirements 15.9**

Property 74: Input mappings persisted
*For any* input mapping change, it should be saved to the configuration file.
**Validates: Requirements 15.11**

Property 75: Input mappings restored on startup
*For any* saved input mappings, starting the emulator should restore those mappings.
**Validates: Requirements 15.12**

### Configuration Persistence Properties

Property 76: Configuration file naming
*For any* configuration save operation, the file should be named "videopac.cfg" in the appropriate user directory.
**Validates: Requirements 16.1**

Property 77: Configuration completeness
*For any* configuration save operation, all settings (directory, recent files, fullscreen, window, video, audio, input, OSD, screenshot) should be stored.
**Validates: Requirements 16.2**

Property 78: Configuration loading on startup
*For any* existing configuration file, starting the emulator should load all settings from that file.
**Validates: Requirements 16.3**

Property 79: Configuration saving on exit
*For any* normal emulator exit, the current configuration should be saved to the file.
**Validates: Requirements 16.4**

### Screenshot Properties

Property 80: Screenshot filename format
*For any* screenshot captured, the filename should follow the format "videopac_YYYYMMDD_HHMMSS.{ext}".
**Validates: Requirements 17.4**

Property 81: Screenshots stored in screenshots directory
*For any* screenshot file, it should be located in the "screenshots" subdirectory.
**Validates: Requirements 17.5**

Property 82: Screenshots directory auto-creation
*For any* screenshot operation, if the "screenshots" directory doesn't exist, it should be created automatically.
**Validates: Requirements 17.6**

Property 83: Screenshot success notification
*For any* successful screenshot operation, a brief notification message should be displayed.
**Validates: Requirements 17.7**

Property 84: Screenshot format persisted
*For any* screenshot format selection, it should be saved to the configuration file.
**Validates: Requirements 17.8**

### Speed Control Properties

Property 85: Turbo mode activates on Tab hold
*For any* emulator state, holding Tab should run emulation at turbo speed (2x-4x normal).
**Validates: Requirements 18.1**

Property 86: Speed changes apply immediately
*For any* speed selection, changing it should immediately adjust the emulation speed.
**Validates: Requirements 18.3**

Property 87: Speed indicator displayed for non-100% speeds
*For any* emulator state running at non-100% speed, the current speed percentage should be displayed on screen.
**Validates: Requirements 18.4**

Property 88: Turbo mode is temporary
*For any* emulator state with Tab held, releasing Tab should return to the configured normal speed.
**Validates: Requirements 18.7**

Property 89: Speed setting persisted
*For any* normal speed setting, it should be saved to the configuration file.
**Validates: Requirements 18.8**

Property 90: Turbo mode ignores normal speed
*For any* configured normal speed, holding Tab should always run at maximum speed regardless of the normal speed setting.
**Validates: Requirements 18.9**

### OSD Customization Properties

Property 91: OSD position changes apply immediately
*For any* OSD position selection, changing it should immediately move all on-screen displays to the new position.
**Validates: Requirements 19.4**

Property 92: OSD font size changes apply immediately
*For any* font size selection, changing it should immediately resize all on-screen text.
**Validates: Requirements 19.6**

Property 93: OSD opacity changes apply immediately
*For any* opacity level selection, changing it should immediately apply transparency to all on-screen displays.
**Validates: Requirements 19.8**

Property 94: OSD settings persisted
*For any* OSD setting change, it should be saved to the configuration file.
**Validates: Requirements 19.9**



## Error Handling

### Error Categories

The SDL frontend enhancements must handle several categories of errors gracefully:

1. **File System Errors**
   - File not found
   - Permission denied
   - Disk full
   - Invalid path

2. **ZIP Extraction Errors**
   - Corrupted ZIP file
   - Unsupported compression method
   - No ROM files in archive
   - Extraction failure

3. **Configuration Errors**
   - Corrupted configuration file
   - Invalid configuration values
   - Missing configuration directory

4. **Save State Errors**
   - Save state file corrupted
   - Incompatible save state version
   - Disk write failure
   - Missing saves directory

5. **Input Errors**
   - Joystick disconnected during gameplay
   - Invalid key mapping
   - Joystick enumeration failure

6. **Resource Errors**
   - Font loading failure
   - Texture creation failure
   - SDL subsystem initialization failure

### Error Handling Strategy

**Display Errors in UI**:
- All errors should be displayed in modal dialogs with clear descriptions
- Error dialogs should require user acknowledgment (Enter or Escape)
- Error messages should be user-friendly, not technical stack traces

**Log Errors to Console**:
- All errors should also be logged to console for debugging
- Console logs should include technical details (error codes, stack traces)
- Use consistent log format: `[ERROR] Component: Description`

**Graceful Degradation**:
- If optional features fail (e.g., thumbnail capture), continue without them
- If configuration loading fails, use default values
- If joystick detection fails, fall back to keyboard input
- If font loading fails, use SDL's built-in rendering

**Recovery Actions**:
- For corrupted configuration: Reset to defaults and create new config file
- For missing directories: Automatically create required directories
- For invalid recent files: Remove from list and continue
- For save state errors: Display error but don't crash

**Error Message Format**:
```
Title: Operation Failed
Message: Could not load ROM file
Details: File not found: /path/to/game.bin
Action: Press Enter to continue
```

### Specific Error Scenarios

**ZIP File Errors**:
- Empty ZIP: "No ROM files found in archive"
- Corrupted ZIP: "Failed to extract ZIP file: {error}"
- Multiple ROMs: Present selection dialog (not an error)

**Configuration Errors**:
- Corrupted file: "Configuration file corrupted, using defaults"
- Invalid values: Ignore invalid values, use defaults for those settings
- Missing file: Silently create new file with defaults

**Save State Errors**:
- Incompatible version: "Save state is from an incompatible emulator version"
- Corrupted file: "Save state file is corrupted or invalid"
- Disk full: "Failed to save state: Disk full"

**File Browser Errors**:
- Permission denied: "Cannot access directory: Permission denied"
- Invalid path: Fall back to current working directory
- Empty directory: Display "No files found" message

**Input Errors**:
- Joystick disconnected: Display notification, fall back to keyboard
- Invalid mapping: Use default mapping for that action
- No joysticks found: Silently use keyboard input



## Testing Strategy

### Dual Testing Approach

This feature requires both unit tests and property-based tests for comprehensive coverage:

**Unit Tests**: Focus on specific examples, edge cases, and integration points
- Menu navigation sequences
- File browser directory traversal
- ZIP extraction with specific archive structures
- Configuration file parsing with known inputs
- Error handling with specific error conditions
- UI rendering with specific states

**Property-Based Tests**: Focus on universal properties across all inputs
- File filtering works for any directory structure
- Configuration round-trip preserves all settings
- Save state round-trip preserves emulator state
- Menu navigation maintains valid state
- Recent files list maintains size limit
- Input remapping works for any key/button

### Property-Based Testing Configuration

**Testing Library**: Use Catch2 with Catch2's built-in generators for C++ property-based testing

**Test Configuration**:
- Minimum 100 iterations per property test
- Each property test references its design document property
- Tag format: `[Feature: sdl-frontend-enhancements][Property N: {property_text}]`

**Example Property Test**:
```cpp
TEST_CASE("Property 42: Recent lists limited to 10 entries", 
          "[Feature: sdl-frontend-enhancements][Property 42]") {
    auto recent_list = GENERATE(take(100, 
        chunk(15, random(0, 1000))));  // Generate lists of 15 random items
    
    RecentFilesList list;
    for (int item : recent_list) {
        list.add(std::to_string(item) + ".bin");
    }
    
    REQUIRE(list.size() <= 10);
}
```

### Test Organization

**Unit Test Files**:
- `tests/test_menu_system.cpp`: Menu navigation, rendering, actions
- `tests/test_file_browser.cpp`: File browsing, filtering, navigation
- `tests/test_zip_handler.cpp`: ZIP extraction, temp file management
- `tests/test_config_manager.cpp`: Configuration loading, saving, parsing
- `tests/test_save_state_manager.cpp`: Save state operations, thumbnails
- `tests/test_osd_renderer.cpp`: OSD rendering, positioning, opacity
- `tests/test_input_mapper.cpp`: Input mapping, joystick detection

**Property Test File**:
- `tests/property_tests_sdl_frontend.cpp`: All property-based tests for SDL frontend enhancements

### Key Testing Scenarios

**Menu System Testing**:
- Navigate through all menu levels
- Select each menu action
- Test menu state transitions (show/hide/navigate)
- Test submenu navigation (enter/back)
- Test menu rendering with different screen sizes

**File Browser Testing**:
- Browse directories with various file types
- Test filtering with different extensions
- Test navigation (up/down/enter/back)
- Test with empty directories
- Test with very long file lists (scrolling)
- Test with special characters in filenames

**ZIP Handler Testing**:
- Extract ZIPs with single ROM
- Extract ZIPs with multiple ROMs
- Extract ZIPs with no ROMs
- Extract corrupted ZIPs
- Test temp file cleanup
- Test with nested directories in ZIP

**Configuration Testing**:
- Save and load all configuration sections
- Test with missing configuration file
- Test with corrupted configuration file
- Test with partial configuration (missing sections)
- Test configuration migration (future versions)
- Test with invalid values

**Save State Testing**:
- Save and load emulator state
- Test with different ROM states
- Test thumbnail capture
- Test with missing saves directory
- Test save state deletion
- Test with corrupted save files

**Input Mapping Testing**:
- Remap all actions for both players
- Test with keyboard input
- Test with joystick input
- Test joystick detection
- Test joystick disconnection
- Test with multiple joysticks

**Integration Testing**:
- Complete workflow: Start emulator → Open menu → Load ROM → Play → Save state → Load state
- Test fullscreen toggle during gameplay
- Test menu access during fullscreen
- Test configuration persistence across restarts
- Test recent files list updates
- Test all keyboard shortcuts

### Mock Objects

For testing UI components without full SDL initialization:

**MockRenderer**: Simulates SDL_Renderer for testing rendering calls
**MockFileSystem**: Simulates file system for testing file operations
**MockEmulatorCore**: Simulates emulator for testing frontend integration
**MockConfigFile**: Simulates configuration file for testing config manager

### Test Data

**Test ROMs**: Use small test ROM files (not copyrighted games)
**Test ZIPs**: Create test ZIP archives with known contents
**Test Configs**: Create test configuration files with various scenarios
**Test Save States**: Create test save state files with known data

### Coverage Goals

- **Line Coverage**: Aim for >90% coverage of new code
- **Branch Coverage**: Aim for >85% coverage of conditional branches
- **Property Coverage**: 100% of correctness properties must have tests
- **Error Path Coverage**: All error handling paths must be tested

### Continuous Testing

- Run unit tests on every build
- Run property tests on every commit
- Run integration tests before releases
- Test on multiple platforms (Windows, Linux, macOS)
- Test with different SDL versions

