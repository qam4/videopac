# Videopac (Odyssey2) Emulator

A cycle-accurate emulator for the Philips Videopac / Magnavox Odyssey2 home video game console.

## Supported Platforms

- **Windows** (10 and later) - MSVC and MinGW-w64
- **Linux** - GCC 7+ and Clang 5+
- **macOS** - Xcode Command Line Tools

## Features

- Intel 8048 CPU emulation with all 96 instructions
- Intel 8245 VDC (Video Display Controller) emulation
- Accurate graphics rendering (sprites, characters, grid)
- Collision detection
- Audio generation
- Keyboard and joystick input
- Save state support
- Built-in disassembler
- Debugging tools
- SDL2-based standalone frontend
- libretro core for RetroArch integration

## Quick Start

### Building

**For detailed build instructions, see [BUILDING.md](BUILDING.md).**

Quick build (Linux/macOS):
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Quick build (Windows with Visual Studio):
```powershell
cmake -S . -B build
cmake --build build --config Release
```

Quick build (Windows with CMake presets):
```powershell
cmake --preset=ci-win64
cmake --build --preset=ci-win64 --config Release
```

### Requirements

- CMake 3.15 or later
- C++17 compatible compiler
  - Windows: Visual Studio 2019+ or MinGW-w64 8.0+
  - Linux: GCC 7+ or Clang 5+
  - macOS: Xcode Command Line Tools
- SDL2 (optional, for graphics and audio)
- Google Test (automatically downloaded)
- RapidCheck (automatically downloaded)

### Build Options

- `BUILD_TESTS=ON/OFF` - Build unit tests (default: ON)
- `BUILD_LIBRETRO=ON/OFF` - Build libretro core (default: ON)
- `BUILD_STANDALONE=ON/OFF` - Build standalone emulator (default: ON)
- `ENABLE_SDL=ON/OFF` - Enable SDL2 frontend (default: ON)
- `BUILD_TOOLS=ON/OFF` - Build development tools (default: ON)

## Running

### Standalone Emulator

Basic usage:
```bash
./videopac --bios <bios_file> <rom_file>
```

Example:
```bash
# USA Odyssey 2 (default) - red ship in Satellite Attack
./videopac --bios "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" "roms/Satellite Attack (1981)(Philips)(EU).bin"

# European Videopac G7000 - blue ship in Satellite Attack
./videopac --region europe --bios "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" "roms/Satellite Attack (1981)(Philips)(EU).bin"

# French C52 SECAM - red ship with PAL timing
./videopac --region france --bios "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" "roms/Satellite Attack (1981)(Philips)(EU).bin"
```

Command-line options:
- `--bios <file>` - Load BIOS from file (optional, auto-loads last used BIOS if available)
- `--region <name>` - Hardware region: `usa`, `europe`, `france` (default: `usa`)
  - `usa` - Magnavox Odyssey 2 (NTSC 60Hz, red ship in Satellite Attack)
  - `europe` - Philips Videopac G7000 (PAL 50Hz, blue ship in Satellite Attack)
  - `france` - Philips C52 SECAM (PAL 50Hz timing, NTSC colors - red ship)
- `--headless` - Run without display (for testing)
- `--frames <n>` - Run for N frames then exit (headless mode)
- `--screenshot <n>` - Save screenshot every N frames (headless mode)
- `--debug` - Enable debugger
- `--help` - Show help message

**Note on Hardware Regions:** The Videopac/Odyssey 2 had different hardware in different regions. The Intel 8244 (NTSC) and 8245 (PAL) chips have different color palettes - notably, color index 1 is red on NTSC systems but blue on PAL systems. The French C52 is special: it uses PAL timing but NTSC colors.

### Controls

- **ESC** - Quit emulator
- **F5** - Reset emulator
- **P** or **PAUSE** - Pause/unpause
- **F12** - Save screenshot
- **0-9, A-Z** - Videopac keyboard keys
- **Space, Enter, +, -, *, /, =, ?** - Special keys

### Running on Remote Systems (NICE DCV)

If running on a remote system via SSH (e.g., AWS EC2 with NICE DCV):

1. Ensure NICE DCV is installed and a session is running
2. Set the DISPLAY environment variable and use software rendering:
```bash
export DISPLAY=:0
export SDL_RENDER_DRIVER=software
./videopac --bios <bios_file> <rom_file>
```

Note: Audio may not work on systems without ALSA, but the emulator will continue without audio.

### Running Tests

```bash
./videopac_tests
```

## Documentation

- **[BUILDING.md](BUILDING.md)** - Comprehensive build instructions for all platforms
- **[HACKING.md](HACKING.md)** - Developer guide with CMake presets and IDE integration
- **[TESTING.md](TESTING.md)** - Testing guide including video settings
- **[doc/](doc/)** - Technical documentation:
  - **[doc/hardware/](doc/hardware/)** - Hardware reference (CPU, VDC, memory architecture)
  - **[doc/case-studies/](doc/case-studies/)** - Debugging case studies (Satellite Attack, etc.)
  - **[doc/reference/](doc/reference/)** - MCS-48 manuals, programming guides
  - **[doc/debugging.md](doc/debugging.md)** - Debugging guide
- **[.kiro/specs/](.kiro/specs/)** - Feature specifications and implementation plans

## Project Structure

```
.
├── include/          # Header files
├── src/              # Source files
├── tests/            # Unit and property-based tests
├── doc/              # Documentation
├── tools/            # Development tools (disassembler, etc.)
├── roms/             # ROM files (not included)
├── CMakeLists.txt    # Build configuration
├── CMakePresets.json # CMake presets for CI and base configurations
└── BUILDING.md       # Build instructions
```

## Contributing

Contributions are welcome! Please see [HACKING.md](HACKING.md) for developer guidelines.

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests and ensure they pass
5. Commit with descriptive messages (see [.kiro/context.md](.kiro/context.md))
6. Push to your fork
7. Create a pull request

## Implementation Status

This project follows the implementation plans in `.kiro/specs/`.

### Videopac Emulator Core
- [x] Task 1: Project setup and build system
- [x] Task 2: Core data types and utilities
- [x] Task 3: CPU emulation
- [x] Task 4: Memory system
- [x] Task 5: Checkpoint - Core components functional
- [x] Task 6: VDC emulation
- [x] Task 7: Input handling
- [x] Task 8: Emulator core orchestration
- [x] Task 10: Save states
- [x] Task 11: Disassembler
- [x] Task 12: Debugger
- [x] Task 13: Checkpoint - Debugging tools complete
- [x] Task 14: SDL2 frontend (partial)
- [ ] Task 15: libretro core
- [ ] Task 16: Final integration and testing

### Windows Build Support
- [x] Task 1: CMake preset files
- [x] Task 2: CMakeLists.txt updates (pending Linux testing)
- [x] Task 3: Documentation (BUILDING.md, HACKING.md)
- [ ] Task 4: Windows testing
- [ ] Task 5: Binary distribution (optional)
- [ ] Task 6: CI setup (optional)
- [ ] Task 7: Final verification

## License

TBD

## References

- Intel 8048 User Manual
- Intel 8245 Datasheet
- Videopac Programming Documentation
