# Building with CMake

This document provides instructions for building the Videopac emulator on all supported platforms.

## Prerequisites

- CMake 3.15 or later
- C++17 compatible compiler
- SDL2 (optional, for graphics and audio support)

### Platform-Specific Requirements

**Windows:**
- Visual Studio 2019 or later (for MSVC builds), OR
- MinGW-w64 8.0 or later (for MinGW builds)
- vcpkg (recommended for SDL2 installation)

**Linux:**
- GCC 7+ or Clang 5+
- SDL2 development libraries: `sudo apt-get install libsdl2-dev` (Ubuntu/Debian)

**macOS:**
- Xcode Command Line Tools
- SDL2: `brew install sdl2`

## Quick Start

### Simple Build (Single-Configuration Generator)

For Unix Makefiles, MinGW Makefiles, or Ninja:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Multi-Configuration Build (Visual Studio)

For Visual Studio or Xcode generators:

```sh
cmake -S . -B build
cmake --build build --config Release
```

## Building with CMake Presets (Recommended for Developers)

CMake presets provide a convenient way to configure builds with predefined settings. See [HACKING.md](HACKING.md) for details on creating your `CMakeUserPresets.json` file.

### Windows (MSVC)

```sh
cmake --preset=ci-win64
cmake --build --preset=ci-win64 --config Release
```

### Windows (MinGW)

```sh
cmake --preset=ci-mingw
cmake --build build/ci-mingw
```

### Running Tests

```sh
ctest --preset=ci-win64 --config Release
# or
ctest --test-dir build/ci-mingw
```

## Building with MSVC

Note that MSVC by default is not standards compliant and you need to pass some flags to make it behave properly. The CMake presets handle this automatically via the `flags-windows` preset, which includes:

- `/permissive-` - Enable conformance mode
- `/Zc:__cplusplus` - Correct `__cplusplus` macro value
- `/utf-8` - Use UTF-8 encoding
- `/W4` - Warning level 4
- `/sdl` - Enable SDL security checks
- `/guard:cf` - Control Flow Guard for security
- `/EHsc` - Exception handling model

If you're not using presets, you'll need to set these flags manually via `CMAKE_CXX_FLAGS`.

## SDL2 Installation

SDL2 is required for graphics and audio support. Without it, only the headless version will be built.

### Windows (vcpkg - Recommended)

**Step 1: Install vcpkg (if not already installed)**

```powershell
# Clone vcpkg to C:\vcpkg (or your preferred location)
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
```

**Step 2: Install SDL2**

```powershell
# For MSVC builds
.\vcpkg install sdl2:x64-windows

# For MinGW builds (if using MinGW)
.\vcpkg install sdl2:x64-mingw-static
```

**Step 3: Configure CMake with vcpkg**

Option A - Command line:
```powershell
cmake --preset=ci-win64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Option B - Create CMakeUserPresets.json (recommended for development):
```powershell
# Copy the example file
copy CMakeUserPresets.json.example CMakeUserPresets.json
```

Then edit `CMakeUserPresets.json` and update the toolchain path:
```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "dev-win64",
      "inherits": "ci-win64",
      "binaryDir": "${sourceDir}/build/dev-win64",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
        "CMAKE_TOOLCHAIN_FILE": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
      }
    }
  ]
}
```

Then simply use:
```powershell
cmake --preset=dev-win64
cmake --build --preset=dev-win64
```

### Windows (Manual)

**Step 1: Download SDL2**

1. Go to https://github.com/libsdl-org/SDL/releases
2. Download `SDL2-devel-2.x.x-VC.zip` (for Visual Studio)
3. Extract to a location (e.g., `C:\SDL2`)

**Step 2: Configure CMake with SDL2_DIR**

```powershell
cmake --preset=ci-win64 -DSDL2_DIR=C:/SDL2/cmake
```

Or add to CMakeUserPresets.json:
```json
{
  "name": "dev-win64",
  "cacheVariables": {
    "SDL2_DIR": "C:/SDL2/cmake"
  }
}
```

### Linux

```sh
sudo apt-get install libsdl2-dev  # Ubuntu/Debian
sudo dnf install SDL2-devel       # Fedora
sudo pacman -S sdl2               # Arch Linux
```

### macOS

```sh
brew install sdl2
```

## Build Options

The following CMake options are available:

- `BUILD_TESTS` - Build unit tests (default: ON)
- `BUILD_LIBRETRO` - Build libretro core (default: ON)
- `BUILD_STANDALONE` - Build standalone emulator (default: ON)
- `ENABLE_SDL` - Enable SDL2 frontend (default: ON)
- `BUILD_TOOLS` - Build development tools (default: ON)

Example:

```sh
cmake -S . -B build -DBUILD_TESTS=OFF -DENABLE_SDL=OFF
```

## Install

After building, you can install the emulator:

```sh
cmake --install build --config Release
```

On Windows with Visual Studio, make sure to specify `--config Release` or `--config Debug`.

## Troubleshooting

### SDL2 not found (Windows)

**Error:** `SDL2 not found. Building headless-only version.`

**Solutions:**
- Install via vcpkg: `vcpkg install sdl2:x64-windows`
- Download from https://libsdl.org and set `SDL2_DIR` in CMakeUserPresets.json
- Set `CMAKE_TOOLCHAIN_FILE` to point to vcpkg toolchain file

### Compiler not found (Windows)

**Error:** `Could not find Visual Studio or MinGW.`

**Solutions:**
- Install Visual Studio 2019 or later (Community Edition is free)
- Install MinGW-w64 from https://www.mingw-w64.org/
- Ensure compiler is in your PATH

### vcpkg toolchain not set

**Warning:** `vcpkg toolchain file not found.`

**Solution:** Set `CMAKE_TOOLCHAIN_FILE` in your CMakeUserPresets.json:

```json
{
  "name": "dev-win64",
  "cacheVariables": {
    "CMAKE_TOOLCHAIN_FILE": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
  }
}
```

### Runtime DLL not found (Windows)

**Error:** `The code execution cannot proceed because SDL2.dll was not found.`

**Solutions:**
- The build system automatically copies SDL2.dll to the build directory when using vcpkg
- For manual SDL2 installation, copy SDL2.dll from the SDL2 lib directory to your executable directory
- Add SDL2 lib directory to your PATH environment variable

### MSVC compilation errors

**Error:** Various C++ standard compliance errors

**Solution:** Use CMake presets which include the correct MSVC flags, or manually add:
```sh
cmake -S . -B build -DCMAKE_CXX_FLAGS="/permissive- /Zc:__cplusplus /utf-8"
```

### MinGW compilation errors

**Error:** Linker errors or missing libraries

**Solution:** Ensure you're using MinGW-w64 (not the older MinGW) and that it's properly installed:
```sh
cmake --preset=ci-mingw
```

## Running the Emulator

After building, run the emulator with:

```sh
# Windows
build\videopac.exe --bios bios.bin game.bin

# Linux/macOS
./build/videopac --bios bios.bin game.bin
```

For more command-line options, run:

```sh
videopac --help
```

## Running Tests

```sh
# Using CTest
ctest --test-dir build --config Release

# Or run the test executable directly
./build/videopac_tests  # Linux/macOS
build\videopac_tests.exe  # Windows
```

## Additional Resources

- [HACKING.md](HACKING.md) - Developer guide with preset usage and IDE integration
- [README.md](README.md) - Project overview and quick start
- [.kiro/context.md](.kiro/context.md) - Project-specific development guidelines
