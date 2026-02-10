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
