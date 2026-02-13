# Requirements Document: Windows Build Support

## Introduction

This document specifies the requirements for enabling native Windows builds of the Videopac emulator using both MSVC (Visual Studio) and MinGW-w64 toolchains. The goal is to provide Windows developers with a straightforward build process using modern CMake presets, allowing them to compile and run the emulator with full audio support on Windows platforms.

## Glossary

- **MSVC**: Microsoft Visual C++ compiler, part of Visual Studio
- **MinGW-w64**: Minimalist GNU for Windows, provides GCC toolchain for Windows
- **CMakePresets.json**: Project-level CMake configuration presets
- **CMakeUserPresets.json**: User-level CMake configuration presets (not committed to git)
- **vcpkg**: Microsoft's C++ package manager for Windows
- **SDL2**: Simple DirectMedia Layer 2, cross-platform multimedia library
- **Visual Studio**: Microsoft's IDE with MSVC compiler
- **Generator**: CMake's backend build system (Visual Studio, MinGW Makefiles, Ninja, etc.)

## Requirements

### Requirement 1: CMake Presets Configuration

**User Story:** As a Windows developer, I want to use CMake presets to configure builds, so that I can easily switch between MSVC and MinGW toolchains.

#### Acceptance Criteria

1. THE project SHALL provide a CMakePresets.json file with Windows build configurations
2. THE CMakePresets.json SHALL include a hidden base preset "flags-windows" with MSVC compiler flags
3. THE CMakePresets.json SHALL include a hidden base preset "flags-mingw" with MinGW compiler flags
4. THE CMakePresets.json SHALL include a "ci-win64" preset for MSVC builds using Visual Studio generator
5. THE CMakePresets.json SHALL include a "ci-mingw" preset for MinGW builds using MinGW Makefiles generator
6. THE CMakePresets.json SHALL enforce C++17 standard for all Windows presets
7. THE project SHALL provide a CMakeUserPresets.json.example template for user-specific configurations
8. THE CMakeUserPresets.json.example SHALL include "dev-win64" preset for MSVC development builds
9. THE CMakeUserPresets.json.example SHALL include "dev-mingw" preset for MinGW development builds
10. THE .gitignore SHALL exclude CMakeUserPresets.json from version control

### Requirement 2: MSVC Compiler Support

**User Story:** As a Windows developer using Visual Studio, I want to build the emulator with MSVC, so that I can use native Windows debugging tools.

#### Acceptance Criteria

1. THE MSVC preset SHALL use Visual Studio generator
2. THE MSVC preset SHALL target x64 architecture
3. THE MSVC preset SHALL enable SDL security checks (/sdl)
4. THE MSVC preset SHALL enable code analysis (/analyze)
5. THE MSVC preset SHALL enable Control Flow Guard (/guard:cf)
6. THE MSVC preset SHALL use UTF-8 encoding (/utf-8)
7. THE MSVC preset SHALL enable conformance mode (/permissive-)
8. THE MSVC preset SHALL enable appropriate warning levels (/W4)
9. THE MSVC preset SHALL enable C++17 standard conformance flags
10. THE build SHALL succeed without errors on Windows with Visual Studio 2019 or later

### Requirement 3: MinGW-w64 Compiler Support

**User Story:** As a Windows developer preferring GCC, I want to build the emulator with MinGW-w64, so that I can use familiar GCC tooling.

#### Acceptance Criteria

1. THE MinGW preset SHALL use MinGW Makefiles generator
2. THE MinGW preset SHALL enable stack protection (-fstack-protector-strong)
3. THE MinGW preset SHALL enable comprehensive warnings (-Wall -Wextra -Wpedantic)
4. THE MinGW preset SHALL enable security hardening flags (-D_FORTIFY_SOURCE=2)
5. THE MinGW preset SHALL enable appropriate linker flags
6. THE MinGW preset SHALL support both 32-bit and 64-bit builds
7. THE build SHALL succeed without errors on Windows with MinGW-w64 8.0 or later

### Requirement 4: SDL2 Dependency Management

**User Story:** As a Windows developer, I want clear instructions for installing SDL2, so that I can build the emulator with graphics and audio support.

#### Acceptance Criteria

1. THE documentation SHALL provide instructions for installing SDL2 via vcpkg
2. THE documentation SHALL provide instructions for manual SDL2 installation
3. THE CMakeLists.txt SHALL use find_package(SDL2) to locate SDL2
4. THE CMakeLists.txt SHALL support SDL2 installed via vcpkg
5. THE CMakeLists.txt SHALL support SDL2 installed manually
6. THE CMakeLists.txt SHALL provide clear error messages when SDL2 is not found
7. THE CMakeUserPresets.json.example SHALL include example SDL2 path configuration
8. WHEN SDL2 is not found, THE build SHALL fall back to headless-only mode

### Requirement 5: Build Documentation

**User Story:** As a Windows developer, I want comprehensive build instructions, so that I can set up my development environment quickly.

#### Acceptance Criteria

1. THE project SHALL provide a Windows-specific build guide (doc/BUILD-Windows.md)
2. THE build guide SHALL document prerequisites (CMake, Visual Studio or MinGW, vcpkg)
3. THE build guide SHALL provide step-by-step MSVC build instructions
4. THE build guide SHALL provide step-by-step MinGW build instructions
5. THE build guide SHALL document how to install SDL2 via vcpkg
6. THE build guide SHALL document how to install SDL2 manually
7. THE build guide SHALL document how to run the emulator on Windows
8. THE build guide SHALL document how to run tests on Windows
9. THE build guide SHALL include troubleshooting section for common issues
10. THE main README.md SHALL reference the Windows build guide

### Requirement 6: Testing on Windows

**User Story:** As a Windows developer, I want to run the test suite, so that I can verify my build is correct.

#### Acceptance Criteria

1. THE test suite SHALL compile and run on Windows with MSVC
2. THE test suite SHALL compile and run on Windows with MinGW
3. THE CMakeUserPresets.json.example SHALL include test presets for Windows
4. THE documentation SHALL explain how to run tests using CMake presets
5. THE documentation SHALL explain how to run tests using CTest
6. ALL existing tests SHALL pass on Windows builds

### Requirement 7: Path Handling

**User Story:** As a Windows developer, I want the emulator to handle Windows file paths correctly, so that I can load ROMs and BIOS files.

#### Acceptance Criteria

1. THE emulator SHALL accept both forward slashes (/) and backslashes (\) in file paths
2. THE emulator SHALL handle Windows absolute paths (C:\path\to\file)
3. THE emulator SHALL handle Windows relative paths correctly
4. THE emulator SHALL handle paths with spaces correctly
5. THE file I/O code SHALL use std::filesystem or portable path handling
6. THE command-line argument parsing SHALL handle Windows path conventions

### Requirement 8: Binary Distribution

**User Story:** As a Windows user, I want to download a pre-built executable, so that I can run the emulator without building from source.

#### Acceptance Criteria

1. THE project SHALL provide instructions for creating Windows release builds
2. THE release build SHALL be statically linked or include required DLLs
3. THE release build SHALL include SDL2.dll if dynamically linked
4. THE documentation SHALL explain how to package the emulator for distribution
5. THE documentation SHALL list required runtime dependencies
6. OPTIONAL: THE project MAY provide a Windows installer or portable ZIP package

### Requirement 9: Development Workflow

**User Story:** As a Windows developer, I want a smooth development workflow, so that I can contribute to the project efficiently.

#### Acceptance Criteria

1. THE CMakeUserPresets.json.example SHALL include Debug and Release configurations
2. THE CMakeUserPresets.json.example SHALL enable compile_commands.json export for IDE integration
3. THE documentation SHALL explain how to use presets with Visual Studio
4. THE documentation SHALL explain how to use presets with Visual Studio Code
5. THE documentation SHALL explain how to use presets with CLion
6. THE documentation SHALL explain how to debug the emulator on Windows
7. THE .gitignore SHALL exclude Windows-specific build artifacts

### Requirement 10: Continuous Integration (Optional)

**User Story:** As a project maintainer, I want automated Windows builds, so that I can catch Windows-specific issues early.

#### Acceptance Criteria

1. OPTIONAL: THE project MAY include GitHub Actions workflow for Windows builds
2. OPTIONAL: THE CI workflow SHALL test both MSVC and MinGW builds
3. OPTIONAL: THE CI workflow SHALL run the test suite on Windows
4. OPTIONAL: THE CI workflow SHALL upload Windows build artifacts
5. OPTIONAL: THE CI workflow SHALL test with multiple Visual Studio versions

## Non-Functional Requirements

### Performance

1. THE Windows build SHALL run at the same speed as Linux builds (within 5%)
2. THE Windows build SHALL maintain 60 FPS for NTSC games and 50 FPS for PAL games

### Compatibility

1. THE emulator SHALL run on Windows 10 and later
2. THE emulator SHALL support both 64-bit Windows (required) and 32-bit Windows (optional)
3. THE emulator SHALL work with Visual Studio 2019, 2022, and later
4. THE emulator SHALL work with MinGW-w64 8.0 and later

### Usability

1. THE build process SHALL take less than 5 minutes on a modern Windows PC
2. THE documentation SHALL be clear enough for developers with basic CMake knowledge
3. THE error messages SHALL be helpful and actionable

## Success Criteria

The Windows build support will be considered successful when:

1. A Windows developer can clone the repository and build the emulator in under 30 minutes
2. Both MSVC and MinGW builds compile without errors or warnings
3. All tests pass on Windows
4. The emulator runs Satellite Attack ROM with graphics and audio on Windows
5. The documentation is clear and complete
6. At least one Windows developer successfully builds and runs the emulator
