# Testing Video Settings Implementation

This document describes how to test the video settings implementation for task 22.2.

## Features Implemented

1. **Aspect Ratio Settings** - Applied immediately by adjusting viewport
   - Original (1:1 pixel, 320x240 with 2x horizontal scaling)
   - 4:3 (standard TV aspect ratio with letterboxing)
   - Stretch (fill entire window)

2. **CRT Effects** - Applied immediately using overlay rendering
   - None (no effect)
   - Light (subtle vignette effect)
   - Medium (moderate vignette effect)
   - Heavy (strong vignette effect)

3. **Scanlines** - Applied immediately using horizontal line overlay
   - Off (no scanlines)
   - 25% (subtle scanlines)
   - 50% (moderate scanlines)
   - 75% (strong scanlines)

4. **VSync Toggle** - Requires restart to take effect
   - On/Off toggle with notification

5. **Settings Persistence** - All settings are saved to config file immediately

## How to Test

### Prerequisites
- Build the emulator: `cmake --build build --target videopac`
- Have a BIOS file and ROM file ready

### Testing Steps

1. **Start the emulator:**
   ```
   .\build\videopac.exe --bios path\to\bios.bin path\to\rom.bin
   ```

2. **Open the menu:**
   - Press `F10` to open the menu

3. **Navigate to Video Settings:**
   - Use arrow keys to navigate to "Video Settings"
   - Press Enter to open the submenu

4. **Test Aspect Ratio:**
   - Navigate to "Aspect Ratio" and press Enter
   - Try each option (Original, 4:3, Stretch)
   - Press Escape to go back
   - Observe that the viewport changes immediately
   - The display should resize/reposition based on the selected mode

5. **Test CRT Effects:**
   - Navigate to "CRT Effects" and press Enter
   - Try each option (None, Light, Medium, Heavy)
   - Press Escape to go back
   - Observe the vignette effect at the edges of the display
   - Higher settings should show more pronounced darkening at edges

6. **Test Scanlines:**
   - Navigate to "Scanlines" and press Enter
   - Try each option (Off, 25%, 50%, 75%)
   - Press Escape to go back
   - Observe horizontal lines appearing across the display
   - Higher percentages should show darker scanlines

7. **Test VSync:**
   - Navigate to "VSync" and press Enter
   - Observe the notification indicating restart is required
   - The setting is saved but won't take effect until restart

8. **Test Settings Persistence:**
   - Change several video settings
   - Exit the emulator (press Escape to close menu, then Escape again to quit)
   - Restart the emulator with the same ROM
   - Open the menu and check Video Settings
   - All settings should be preserved from the previous session

### Expected Results

- ✅ Aspect ratio changes apply immediately without restart
- ✅ CRT effects apply immediately without restart
- ✅ Scanlines apply immediately without restart
- ✅ VSync toggle saves setting but shows "Restart Required" message
- ✅ All settings are persisted to config file
- ✅ Settings are restored on next launch
- ✅ Notifications appear for each setting change

### Configuration File

Settings are stored in:
- Windows: `%APPDATA%\videopac\videopac.cfg`
- Linux/macOS: `~/.config/videopac/videopac.cfg`

You can verify the settings are saved by checking the `[Video]` section:
```ini
[Video]
scaling_filter=nearest
aspect_ratio=4:3
vsync=true
crt_effect=medium
scanlines=50
```

## Implementation Notes

### Aspect Ratio
- **Original**: 160x240 framebuffer scaled 2x horizontally to 320x240, centered in window
- **4:3**: Calculates viewport to maintain 4:3 aspect ratio, adds letterboxing as needed
- **Stretch**: Fills entire window, may distort image

### CRT Effects
- Implemented as simple vignette overlay (darkening at edges)
- Uses semi-transparent rectangles at top/bottom/left/right edges
- Alpha values: Light=15, Medium=30, Heavy=50

### Scanlines
- Implemented as horizontal lines drawn every 2 pixels
- Alpha calculated from percentage: (percent * 255) / 100
- Creates authentic CRT scanline appearance

### VSync
- Setting is saved immediately
- Requires renderer recreation to take effect
- User is notified that restart is required
