# Videopac (Odyssey2) Emulator

A cycle-accurate emulator for the Philips Videopac / Magnavox Odyssey2 home video game console.

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

## Building

### Requirements

- CMake 3.15 or later
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- SDL2 (for standalone build)
- Google Test (automatically downloaded)
- RapidCheck (automatically downloaded)

### Build Instructions

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Build Options

- `BUILD_TESTS=ON/OFF` - Build unit tests (default: ON)
- `BUILD_LIBRETRO=ON/OFF` - Build libretro core (default: ON)
- `BUILD_STANDALONE=ON/OFF` - Build standalone emulator (default: ON)

Example:
```bash
cmake -DBUILD_TESTS=OFF ..
```

## Running

### Standalone Emulator

```bash
./videopac <rom_file>
```

### Running Tests

```bash
./videopac_tests
```

## Project Structure

```
.
├── include/          # Header files
├── src/              # Source files
├── tests/            # Unit and property-based tests
├── doc/              # Documentation
├── roms/             # ROM files (not included)
└── CMakeLists.txt    # Build configuration
```

## Implementation Status

This project follows the implementation plan in `.kiro/specs/videopac-emulator/tasks.md`.

- [x] Task 1: Project setup and build system
- [ ] Task 2: Core data types and utilities
- [ ] Task 3: CPU emulation
- [ ] Task 4: Memory system
- [ ] Task 5: VDC emulation
- [ ] Task 6: Input handling
- [ ] Task 7: Emulator core
- [ ] Task 8: Save states
- [ ] Task 9: Disassembler
- [ ] Task 10: Debugger
- [ ] Task 11: SDL2 frontend
- [ ] Task 12: libretro core

## License

TBD

## References

- Intel 8048 User Manual
- Intel 8245 Datasheet
- Videopac Programming Documentation
