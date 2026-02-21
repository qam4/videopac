# Testing the Videopac Emulator

## SDL Frontend Testing

### Local Testing (with display)

```bash
build/videopac --bios "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" "roms/Satellite Attack (1981)(Philips)(EU).bin"
```

### Remote Testing (NICE DCV on AWS)

When running on a remote EC2 instance via SSH with NICE DCV:

```bash
# Set environment variables for DCV display and software rendering
export DISPLAY=:0
export SDL_RENDER_DRIVER=software

# Run the emulator
build/videopac --bios "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" "roms/Satellite Attack (1981)(Philips)(EU).bin"
```

**Note:** Audio initialization may fail with ALSA errors on systems without sound hardware. The emulator will continue without audio.

### Headless Testing (no display required)

```bash
# Run for 60 frames and save a screenshot
build/videopac --headless --frames 60 --screenshot 60 \
  --bios "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" \
  "roms/Satellite Attack (1981)(Philips)(EU).bin"

# Check the screenshot
ls -lh screenshots/frame_000060.ppm
```

## Controls

### Special Keys
- **ESC** - Quit emulator
- **F5** - Reset emulator  
- **P** or **PAUSE** - Pause/unpause
- **F12** - Save screenshot to `screenshot.ppm`

### Videopac Keyboard
- **0-9** - Number keys
- **A-Z** - Letter keys
- **Space** - Space key
- **Enter** - Enter key
- **+, -, *, /, =** - Math operators
- **.** - Period
- **?** - Question mark

## Expected Behavior

### French BIOS
The French BIOS displays "QUEL JEU?" (Which game?) on startup.

### Display
- Resolution: 160x200 pixels
- Colors: 8 colors (black, blue, green, cyan, red, magenta, yellow, white)
- Default window size: 160x200 (can be scaled)

## Troubleshooting

### "Failed to create renderer" error
Set SDL to use software rendering:
```bash
export SDL_RENDER_DRIVER=software
```

### "ALSA: Couldn't open audio device" warning
This is normal on systems without audio hardware. The emulator continues without audio.

### Black screen
- Verify BIOS and ROM files are valid
- Check that the emulator is running (not frozen)
- Try pressing F5 to reset

### No DISPLAY variable
On remote systems, set:
```bash
export DISPLAY=:0
```

## Unit Tests

Run all unit tests:
```bash
build/videopac_tests
```

Run specific test suite:
```bash
build/videopac_tests --gtest_filter="CPUTest.*"
build/videopac_tests --gtest_filter="VDCTest.*"
build/videopac_tests --gtest_filter="MemoryTest.*"
```

## Performance

The emulator should run at full speed (60 FPS for NTSC, 50 FPS for PAL) on modern hardware.

To see FPS counter, the frontend can be modified to enable `show_fps` in the config.


---

## Video Settings Testing

### Features

The emulator includes several video settings that can be adjusted through the in-game menu (F10):

1. **Aspect Ratio** - Applied immediately
   - Original (1:1 pixel, 320x240 with 2x horizontal scaling)
   - 4:3 (standard TV aspect ratio with letterboxing)
   - Stretch (fill entire window)

2. **CRT Effects** - Applied immediately
   - None, Light, Medium, Heavy (vignette effect at edges)

3. **Scanlines** - Applied immediately
   - Off, 25%, 50%, 75% (horizontal line overlay)

### Testing Video Settings

1. **Start the emulator and press F10** to open the menu

2. **Navigate to Video Settings** using arrow keys and press Enter

3. **Test each setting:**
   - Aspect Ratio: Changes viewport immediately
   - CRT Effects: Adds edge darkening effect
   - Scanlines: Adds horizontal lines across display

4. **Verify persistence:**
   - Change settings, exit emulator
   - Restart and verify settings are preserved

### Configuration File

Settings are stored in:
- **Windows**: `%APPDATA%\videopac\videopac.cfg`
- **Linux/macOS**: `~/.config/videopac/videopac.cfg`

Example `[Video]` section:
```ini
[Video]
scaling_filter=nearest
aspect_ratio=4:3
crt_effect=medium
scanlines=50
```
