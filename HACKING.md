# Hacking on Videopac Emulator

This guide is for developers who want to contribute to the Videopac emulator project.

## Developer Mode

Developer mode enables additional build targets and features useful for development. It's enabled via the `videopac_DEVELOPER_MODE` CMake option (currently not implemented, but reserved for future use).

## CMake Presets

The project uses CMake presets to provide consistent, reproducible build configurations. Presets are defined in two files:

- **CMakePresets.json** (committed to git) - Project-wide presets for CI and base configurations
- **CMakeUserPresets.json** (NOT committed) - User-specific presets for local development

### Creating Your CMakeUserPresets.json

Copy the example file and customize it for your environment:

```sh
cp CMakeUserPresets.json.example CMakeUserPresets.json
```

Then edit `CMakeUserPresets.json` to match your setup:

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "dev",
      "binaryDir": "${sourceDir}/build/dev",
      "inherits": "ci-win64",  // or ci-mingw, ci-linux
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
        "CMAKE_TOOLCHAIN_FILE": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "dev",
      "configurePreset": "dev",
      "jobs": 8
    }
  ],
  "testPresets": [
    {
      "name": "dev",
      "configurePreset": "dev",
      "output": {
        "outputOnFailure": true
      }
    }
  ]
}
```

**Important:** `CMakeUserPresets.json` is in `.gitignore` and should NEVER be committed to the repository. It contains user-specific paths and preferences.

### Available Base Presets

The project provides these base presets in `CMakePresets.json`:

- **ci-win64** - MSVC build with Visual Studio 2022 generator
- **ci-mingw** - MinGW build with MinGW Makefiles generator
- **ci-linux** - Linux build (if you add it)
- **ci-darwin** - macOS build (if you add it)

Each preset inherits from hidden base presets that configure compiler flags and C++ standard:

- **cmake-pedantic** - Strict CMake warnings and errors
- **ci-std** - C++17 standard enforcement
- **flags-windows** - MSVC-specific compiler flags
- **flags-mingw** - MinGW-specific compiler flags

### Using Presets

#### Configure

```sh
cmake --preset=dev
```

#### Build

```sh
cmake --build --preset=dev
```

For multi-configuration generators (Visual Studio), specify the configuration:

```sh
cmake --build --preset=dev --config Debug
```

#### Test

```sh
ctest --preset=dev
```

For multi-configuration generators:

```sh
ctest --preset=dev --config Debug
```

### Preset Inheritance

Presets can inherit from other presets, allowing you to build on existing configurations:

```json
{
  "name": "dev-win64-release",
  "inherits": "ci-win64",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "Release",
    "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
  }
}
```

## IDE Integration

### Visual Studio

1. Open the project folder in Visual Studio (File > Open > Folder)
2. Visual Studio automatically detects `CMakePresets.json`
3. Select your preset from the configuration dropdown
4. Build and debug normally using Visual Studio's CMake integration

### Visual Studio Code

1. Install the CMake Tools extension
2. Open the project folder in VS Code
3. Press `Ctrl+Shift+P` and select "CMake: Select Configure Preset"
4. Choose your preset (e.g., "dev-win64")
5. Use the CMake sidebar or commands to build and test

### CLion

1. Open the project in CLion
2. CLion automatically detects `CMakePresets.json`
3. Go to Settings > Build, Execution, Deployment > CMake
4. Select your preset from the list
5. Build and debug normally

## Development Workflow

### 1. Make Changes

Edit source files in `src/` or `include/`.

### 2. Build

```sh
cmake --build --preset=dev
```

### 3. Run Tests

```sh
ctest --preset=dev
```

**CRITICAL:** All tests MUST pass before committing. Never commit with failing tests.

### 4. Run the Emulator

```sh
./build/dev/videopac --bios bios.bin game.bin
```

### 5. Debug

Use your IDE's debugger, or run with a debugger:

```sh
# Visual Studio
devenv build/dev/videopac.exe

# GDB (Linux/MinGW)
gdb --args ./build/dev/videopac --bios bios.bin game.bin

# LLDB (macOS)
lldb -- ./build/dev/videopac --bios bios.bin game.bin
```

## Compiler Flags

### MSVC (flags-windows preset)

- `/sdl` - Enable SDL security checks
- `/guard:cf` - Control Flow Guard for security
- `/utf-8` - Use UTF-8 encoding
- `/W4` - Warning level 4
- `/permissive-` - Conformance mode
- `/Zc:__cplusplus` - Correct `__cplusplus` macro value
- `/EHsc` - Exception handling model

### MinGW (flags-mingw preset)

- `-D_FORTIFY_SOURCE=2` - Buffer overflow detection
- `-O2` - Optimization level 2
- `-fstack-protector-strong` - Stack protection
- `-Wall -Wextra -Wpedantic` - Comprehensive warnings

## Build Targets

The project provides several build targets:

- **videopac** - Standalone emulator executable
- **videopac_core** - Core emulator library (static)
- **videopac_libretro** - LibRetro core (shared library)
- **videopac_tests** - Test suite
- **disasm_tool** - Disassembler tool

Build a specific target:

```sh
cmake --build --preset=dev --target videopac_tests
```

## Testing

### Running All Tests

```sh
ctest --preset=dev
```

### Running Specific Tests

```sh
# Run tests matching a pattern
ctest --preset=dev -R cpu

# Run a specific test
./build/dev/videopac_tests --gtest_filter=CPUTest.ADD
```

### Property-Based Tests

The project uses RapidCheck for property-based testing. These tests run 100+ iterations with random inputs to verify correctness properties.

To run only property tests:

```sh
./build/dev/videopac_tests --gtest_filter=*Property*
```

## Code Style

Follow the existing code style in the project:

- C++17 standard
- 4-space indentation
- Opening braces on same line for functions and classes
- Use `snake_case` for functions and variables
- Use `PascalCase` for classes and structs
- Use `UPPER_CASE` for constants and macros

## Commit Guidelines

See [.kiro/context.md](.kiro/context.md) for detailed commit guidelines. Quick summary:

1. **Test first** - All tests must pass
2. **Stage relevant files** - Don't use `git add .`
3. **Review changes** - Use `git diff --staged`
4. **Write good commit messages** - Use conventional commit format

Example:

```
feat: add Windows build support with CMake presets

Add CMakePresets.json with MSVC and MinGW configurations.
Update CMakeLists.txt to handle SDL2 detection on Windows.
Create BUILDING.md and HACKING.md documentation.

Implements tasks 1-3 from windows-build-support spec.
```

## Debugging Tips

### Enable Trace Logging

```sh
./videopac --debug --bios bios.bin game.bin
```

This creates `trace.log` with detailed execution traces.

### Conditional Breakpoints

```sh
./videopac --break 0x400 --condition "cpu.A == 0xFF" --bios bios.bin game.bin
```

### Memory Inspection

Use the debugger interface to inspect CPU, VDC, and memory state.

## SDL2 Development

When working on SDL2 frontend code:

1. Ensure SDL2 is installed (see BUILDING.md)
2. Build with `ENABLE_SDL=ON` (default)
3. Test with actual ROMs to verify graphics and audio

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests and ensure they pass
5. Commit with descriptive messages
6. Push to your fork
7. Create a pull request

## Resources

- [BUILDING.md](BUILDING.md) - Build instructions
- [README.md](README.md) - Project overview
- [.kiro/context.md](.kiro/context.md) - Project-specific guidelines
- [.kiro/specs/](. kiro/specs/) - Feature specifications

## Getting Help

- Check existing documentation
- Review the spec files in `.kiro/specs/`
- Look at similar code in the project
- Ask questions in pull requests or issues
