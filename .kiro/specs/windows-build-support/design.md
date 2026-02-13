# Design Document: Windows Build Support

## Overview

This design document describes the implementation approach for enabling native Windows builds of the Videopac emulator using both MSVC and MinGW-w64 toolchains. The design leverages modern CMake presets to provide a clean, reproducible build experience across different Windows development environments.

### Key Design Principles

1. **Preset-based configuration**: Use CMakePresets.json for project-wide settings and CMakeUserPresets.json for user-specific overrides
2. **Toolchain flexibility**: Support both MSVC and MinGW-w64 with appropriate compiler flags
3. **Dependency management**: Provide multiple options for SDL2 installation (vcpkg, manual)
4. **Documentation-first**: Comprehensive guides for Windows developers
5. **Minimal code changes**: Leverage existing portable C++ code

## Architecture

### CMake Preset Hierarchy

```
CMakePresets.json (project-level, committed to git)
├── cmake-pedantic (hidden base)
├── ci-std (hidden base, enforces C++17)
├── flags-windows (hidden base, MSVC flags)
├── flags-mingw (hidden base, MinGW flags)
├── ci-win64 (MSVC preset, inherits flags-windows + ci-std)
└── ci-mingw (MinGW preset, inherits flags-mingw + ci-std)

CMakeUserPresets.json (user-level, NOT committed)
├── dev-win64 (MSVC development, inherits ci-win64)
├── dev-mingw (MinGW development, inherits ci-mingw)
├── Build presets (dev-win64, dev-mingw)
└── Test presets (dev-win64, dev-mingw)
```

## Components

### 1. CMakePresets.json

The project-level preset file defines base configurations for Windows builds.

#### Hidden Base Presets

**cmake-pedantic**:
```json
{
  "name": "cmake-pedantic",
  "hidden": true,
  "warnings": {
    "dev": true,
    "deprecated": true,
    "uninitialized": true,
    "unusedCli": true,
    "systemVars": false
  },
  "errors": {
    "dev": true,
    "deprecated": true
  }
}
```

**ci-std** (C++17 enforcement):
```json
{
  "name": "ci-std",
  "hidden": true,
  "cacheVariables": {
    "CMAKE_CXX_EXTENSIONS": "OFF",
    "CMAKE_CXX_STANDARD": "17",
    "CMAKE_CXX_STANDARD_REQUIRED": "ON"
  }
}
```

**flags-windows** (MSVC compiler flags):
```json
{
  "name": "flags-windows",
  "hidden": true,
  "cacheVariables": {
    "CMAKE_CXX_FLAGS": "/sdl /guard:cf /utf-8 /W4 /permissive- /Zc:__cplusplus /EHsc",
    "CMAKE_EXE_LINKER_FLAGS": "/machine:x64 /guard:cf"
  }
}
```

Key MSVC flags:
- `/sdl`: Enable SDL security checks
- `/guard:cf`: Control Flow Guard for security
- `/utf-8`: Use UTF-8 encoding
- `/W4`: Warning level 4
- `/permissive-`: Conformance mode
- `/Zc:__cplusplus`: Correct `__cplusplus` macro value
- `/EHsc`: Exception handling model

**flags-mingw** (MinGW compiler flags):
```json
{
  "name": "flags-mingw",
  "hidden": true,
  "cacheVariables": {
    "CMAKE_CXX_FLAGS": "-D_FORTIFY_SOURCE=2 -O2 -fstack-protector-strong -Wall -Wextra -Wpedantic",
    "CMAKE_EXE_LINKER_FLAGS": "-Wl,--as-needed"
  }
}
```

Key MinGW flags:
- `-D_FORTIFY_SOURCE=2`: Buffer overflow detection
- `-fstack-protector-strong`: Stack protection
- `-Wall -Wextra -Wpedantic`: Comprehensive warnings

#### Public Presets

**ci-win64** (MSVC):
```json
{
  "name": "ci-win64",
  "inherits": ["flags-windows", "ci-std"],
  "generator": "Visual Studio 17 2022",
  "architecture": "x64",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "Release"
  }
}
```

**ci-mingw** (MinGW):
```json
{
  "name": "ci-mingw",
  "inherits": ["flags-mingw", "ci-std"],
  "generator": "MinGW Makefiles",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "Release"
  }
}
```

### 2. CMakeUserPresets.json.example

**IMPORTANT**: CMakeUserPresets.json is for user-specific configuration and should NOT be committed to the repository. Only CMakeUserPresets.json.example is committed as a template.

User-level presets for development workflows.

**dev-win64** (MSVC development):
```json
{
  "name": "dev-win64",
  "binaryDir": "${sourceDir}/build/dev-win64",
  "inherits": "ci-win64",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "Debug",
    "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
  }
}
```

**dev-mingw** (MinGW development):
```json
{
  "name": "dev-mingw",
  "binaryDir": "${sourceDir}/build/dev-mingw",
  "inherits": "ci-mingw",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "Debug",
    "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
  }
}
```

**Build presets**:
```json
{
  "name": "dev-win64",
  "configurePreset": "dev-win64",
  "configuration": "Debug",
  "jobs": 8
}
```

**Test presets**:
```json
{
  "name": "dev-win64",
  "configurePreset": "dev-win64",
  "configuration": "Debug",
  "output": {
    "outputOnFailure": true
  }
}
```

### 3. CMakeLists.txt Updates

Minimal changes to support Windows builds.

#### SDL2 Detection

```cmake
# Find SDL2
if(WIN32)
    # Try vcpkg first
    find_package(SDL2 CONFIG QUIET)
    if(NOT SDL2_FOUND)
        # Try FindSDL2.cmake module
        find_package(SDL2 MODULE)
    endif()
else()
    find_package(SDL2)
endif()

if(SDL2_FOUND)
    message(STATUS "SDL2 found: ${SDL2_LIBRARIES}")
else()
    message(WARNING "SDL2 not found, building headless-only version")
endif()
```

#### Windows-specific Linking

```cmake
if(WIN32 AND SDL2_FOUND)
    target_link_libraries(videopac
        videopac_core
        SDL2::SDL2
        SDL2::SDL2main  # WinMain wrapper
    )
endif()
```

#### Runtime DLL Copying (Optional)

```cmake
if(WIN32 AND SDL2_FOUND)
    # Copy SDL2.dll to build directory for convenience
    add_custom_command(TARGET videopac POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:videopac>
    )
endif()
```

### 4. SDL2 Installation Options

#### Option A: vcpkg (Recommended)

vcpkg is Microsoft's C++ package manager, providing easy SDL2 installation.

**Installation**:
```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install SDL2
.\vcpkg install sdl2:x64-windows
```

**CMake Integration**:
```powershell
cmake --preset dev-win64 -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Or set in CMakeUserPresets.json:
```json
{
  "name": "dev-win64",
  "cacheVariables": {
    "CMAKE_TOOLCHAIN_FILE": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
  }
}
```

#### Option B: Manual Installation

Download SDL2 development libraries from libsdl.org.

**CMake Integration**:
```json
{
  "name": "dev-win64",
  "cacheVariables": {
    "SDL2_DIR": "C:/SDL2-2.28.0/cmake"
  }
}
```

### 5. Build Documentation Structure

The project will follow the documentation structure from the reference project:

**BUILDING.md** (root level):
- General build instructions for all platforms
- Simple commands for release builds
- Multi-configuration generator instructions
- MSVC-specific notes
- Install instructions

**HACKING.md** (root level):
- Developer mode explanation
- Preset usage guide
- How to create CMakeUserPresets.json
- Developer targets (coverage, docs, format, etc.)
- IDE integration notes

**README.md** (updated):
- Quick start section
- Link to BUILDING.md for detailed instructions
- Link to HACKING.md for contributors

Example BUILDING.md structure:
```markdown
# Building with CMake

## Build

Here are the steps for building in release mode with a single-configuration
generator, like the Unix Makefiles one:

```sh
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

Here are the steps for building in release mode with a multi-configuration
generator, like the Visual Studio ones:

```sh
cmake -S . -B build
cmake --build build --config Release
```

### Building with MSVC

Note that MSVC by default is not standards compliant and you need to pass some
flags to make it behave properly. See the `flags-windows` preset in the
[CMakePresets.json](CMakePresets.json) file for the flags and with what
variable to provide them to CMake during configuration.

### Building with Presets (Recommended)

For developers, using CMake presets is recommended. See [HACKING.md](HACKING.md)
for details on creating your CMakeUserPresets.json file.

```sh
cmake --preset=dev-win64  # or dev-mingw
cmake --build --preset=dev-win64
ctest --preset=dev-win64
```

## Install

```sh
cmake --install build --config Release
```
```

Example HACKING.md structure:
```markdown
# Hacking

## Developer mode

Developer mode is enabled via the `videopac_DEVELOPER_MODE` option.

## Presets

Create a `CMakeUserPresets.json` file at the root:

```json
{
  "version": 2,
  "configurePresets": [
    {
      "name": "dev",
      "binaryDir": "${sourceDir}/build/dev",
      "inherits": ["ci-win64"],  // or ci-mingw, ci-linux
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_TOOLCHAIN_FILE": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
      }
    }
  ]
}
```

Replace `ci-win64` with `ci-linux` or `ci-darwin` for your OS.

## Configure, build and test

```sh
cmake --preset=dev
cmake --build --preset=dev
ctest --preset=dev
```
```

### 6. Path Handling

The existing code uses `std::ifstream` and `std::ofstream`, which handle Windows paths correctly. No changes needed.

**Verification**:
- `std::ifstream` accepts both `/` and `\` on Windows
- Absolute paths like `C:\path\to\file` work correctly
- Relative paths work correctly

**Command-line arguments**:
Windows command prompt and PowerShell pass arguments correctly to `main(int argc, char* argv[])`. No special handling needed.

### 7. Binary Distribution

#### Static Linking (Recommended)

Link SDL2 statically to create a standalone executable:

```cmake
if(WIN32)
    set(SDL2_STATIC ON CACHE BOOL "Link SDL2 statically")
endif()
```

With vcpkg:
```powershell
.\vcpkg install sdl2:x64-windows-static
```

#### Dynamic Linking

If using dynamic SDL2, include SDL2.dll with the executable:

```
videopac-1.0-windows/
├── videopac.exe
├── SDL2.dll
├── README.txt
└── roms/
    └── (example ROMs)
```

### 8. Development Workflow

#### Visual Studio

1. Open folder in Visual Studio
2. Visual Studio detects CMakePresets.json
3. Select "dev-win64" preset from dropdown
4. Build and debug normally

#### Visual Studio Code

1. Install CMake Tools extension
2. Open folder in VS Code
3. Select "dev-win64" or "dev-mingw" preset
4. Use CMake: Build command

#### CLion

1. Open project in CLion
2. CLion detects CMakePresets.json
3. Select preset from Settings > Build, Execution, Deployment > CMake
4. Build and debug normally

### 9. Testing Strategy

#### Unit Tests

All existing tests should pass on Windows without modification:
- Google Test is cross-platform
- RapidCheck is cross-platform
- Test code uses portable C++

#### Integration Tests

Test Windows-specific scenarios:
- Loading ROMs with Windows paths
- Saving/loading save states with Windows paths
- SDL2 initialization and rendering
- Audio output

#### Manual Testing

1. Build with MSVC and MinGW
2. Run emulator with Satellite Attack ROM
3. Verify graphics render correctly
4. Verify audio plays correctly
5. Verify keyboard input works
6. Verify save states work

### 10. Continuous Integration (Optional)

GitHub Actions workflow for Windows builds:

```yaml
name: Windows

on: [push, pull_request]

jobs:
  build-msvc:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - name: Setup vcpkg
        run: |
          git clone https://github.com/Microsoft/vcpkg.git
          .\vcpkg\bootstrap-vcpkg.bat
          .\vcpkg\vcpkg install sdl2:x64-windows
      - name: Configure
        run: cmake --preset ci-windows -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
      - name: Build
        run: cmake --build build --config Release
      - name: Test
        run: ctest --test-dir build --config Release

  build-mingw:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - name: Setup MinGW
        uses: egor-tensin/setup-mingw@v2
      - name: Setup vcpkg
        run: |
          git clone https://github.com/Microsoft/vcpkg.git
          .\vcpkg\bootstrap-vcpkg.bat
          .\vcpkg\vcpkg install sdl2:x64-mingw-static
      - name: Configure
        run: cmake --preset ci-mingw -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
      - name: Build
        run: cmake --build build
      - name: Test
        run: ctest --test-dir build
```

## Error Handling

### SDL2 Not Found

```
CMake Warning: SDL2 not found. Building headless-only version.
To enable SDL2 support:
  - Install via vcpkg: vcpkg install sdl2:x64-windows
  - Or download from https://libsdl.org and set SDL2_DIR
```

### Compiler Not Found

```
CMake Error: Could not find Visual Studio or MinGW.
Please install:
  - Visual Studio 2019 or later (for MSVC builds)
  - MinGW-w64 8.0 or later (for MinGW builds)
```

### vcpkg Toolchain Not Set

```
CMake Warning: vcpkg toolchain file not found.
Set CMAKE_TOOLCHAIN_FILE in CMakeUserPresets.json:
  "CMAKE_TOOLCHAIN_FILE": "C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

## File Structure

```
videopac/
├── CMakeLists.txt (updated)
├── CMakePresets.json (new)
├── CMakeUserPresets.json.example (new)
├── .gitignore (updated)
├── doc/
│   └── BUILD-Windows.md (new)
├── README.md (updated with Windows section)
└── (existing source files)
```

## Migration Path

For developers currently building on Linux:

1. No changes to existing Linux workflow
2. CMakePresets.json includes Linux presets (ci-linux, dev-linux)
3. CMakeUserPresets.json is optional on Linux
4. Existing build commands continue to work

## Performance Considerations

- MSVC and MinGW builds should perform similarly to Linux builds
- SDL2 on Windows has comparable performance to Linux
- No Windows-specific performance optimizations needed initially
- Future: Consider Windows-specific optimizations (DirectX backend, etc.)

## Security Considerations

- MSVC flags enable Control Flow Guard (/guard:cf)
- MinGW flags enable stack protection (-fstack-protector-strong)
- Both toolchains enable buffer overflow detection
- SDL2 provides secure input handling

## Future Enhancements

1. **Windows Installer**: Create MSI or NSIS installer
2. **Portable Package**: ZIP file with all dependencies
3. **DirectX Backend**: Optional DirectX renderer for better performance
4. **Windows Store**: Publish to Microsoft Store
5. **ARM64 Support**: Build for Windows on ARM
