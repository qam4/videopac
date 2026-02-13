# Implementation Plan: Windows Build Support

## Overview

This implementation plan adds native Windows build support for the Videopac emulator using both MSVC and MinGW-w64 toolchains. The implementation uses modern CMake presets to provide a clean, reproducible build experience.

## Tasks

- [x] 1. Create CMake preset files
  - [x] 1.1 Create CMakePresets.json with Windows configurations
    - Add cmake-pedantic hidden base preset
    - Add ci-std hidden base preset (C++17 enforcement)
    - Add flags-windows hidden base preset with MSVC compiler flags
    - Add flags-mingw hidden base preset with MinGW compiler flags
    - Add ci-win64 public preset for MSVC builds
    - Add ci-mingw public preset for MinGW builds
    - Ensure presets inherit from appropriate base presets
    - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5, 1.6_

  - [x] 1.2 Create CMakeUserPresets.json.example template
    - Add dev-win64 preset for MSVC development builds
    - Add dev-mingw preset for MinGW development builds
    - Add build presets for dev-win64 and dev-mingw
    - Add test presets for dev-win64 and dev-mingw
    - Include example vcpkg toolchain file path
    - Include example SDL2_DIR path for manual installation
    - Set CMAKE_EXPORT_COMPILE_COMMANDS for IDE integration
    - _Requirements: 1.7, 1.8, 1.9, 9.1, 9.2_

  - [x] 1.3 Update .gitignore for Windows
    - Add CMakeUserPresets.json to .gitignore
    - Add build/dev-win64/ to .gitignore
    - Add build/dev-mingw/ to .gitignore
    - Add Windows-specific build artifacts (*.exe, *.dll, *.pdb)
    - Add Visual Studio specific files (.vs/, *.sln, *.vcxproj)
    - _Requirements: 1.10, 9.7_


- [ ] 2. Update CMakeLists.txt for Windows support
  - [x] 2.1 Improve SDL2 detection for Windows
    - Try find_package(SDL2 CONFIG) first (for vcpkg)
    - Fall back to find_package(SDL2 MODULE) for manual installation
    - Provide clear error messages when SDL2 is not found
    - Support both dynamic and static SDL2 linking
    - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5, 4.6_

  - [x] 2.2 Add Windows-specific linking
    - Link SDL2::SDL2main for WinMain wrapper on Windows
    - Handle both SDL2::SDL2 (vcpkg) and ${SDL2_LIBRARIES} (manual)
    - Add option to copy SDL2.dll to build directory
    - _Requirements: 4.7, 8.2, 8.3_

  - [x] 2.3 Verify existing MSVC flags in CMakeLists.txt
    - Ensure MSVC-specific flags are compatible with presets
    - Remove any conflicting flags
    - Verify /W4 warning level is appropriate
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9_

  - [x] 2.4 Test CMakeLists.txt changes on Linux
    - Verify Linux builds still work
    - Verify no regressions in existing functionality
    - Ensure changes are backward compatible
    - _Requirements: All (verification)_


- [x] 3. Create Windows build documentation
  - [x] 3.1 Create BUILDING.md (root level)
    - Document general build instructions for all platforms
    - Provide simple commands for release builds
    - Document multi-configuration generator usage (Visual Studio)
    - Document single-configuration generator usage (Unix Makefiles, MinGW)
    - Add MSVC-specific notes about conformance flags
    - Document preset-based builds (recommended for developers)
    - Document install instructions
    - Keep it simple and platform-agnostic where possible
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.7, 5.8_

  - [x] 3.2 Create HACKING.md (root level)
    - Document developer mode (videopac_DEVELOPER_MODE option)
    - Explain CMake presets system
    - Provide example CMakeUserPresets.json structure
    - Document how to configure, build, and test with presets
    - Document developer targets (if any: coverage, docs, format, etc.)
    - Document IDE integration (Visual Studio, VS Code, CLion)
    - Emphasize that CMakeUserPresets.json is NOT committed to repo
    - _Requirements: 5.5, 5.6, 9.1, 9.2, 9.3, 9.4, 9.5, 9.6_

  - [x] 3.3 Update main README.md
    - Add Windows to supported platforms list
    - Add quick start section
    - Link to BUILDING.md for build instructions
    - Link to HACKING.md for contributors
    - Update build status badges (if applicable)
    - _Requirements: 5.10_

  - [x] 3.4 Create troubleshooting section in BUILDING.md
    - SDL2 not found errors
    - Compiler not found errors
    - vcpkg toolchain not set errors
    - Linker errors with SDL2
    - Runtime DLL not found errors
    - Common MSVC compilation errors
    - Common MinGW compilation errors
    - _Requirements: 5.9_


- [ ] 4. Test Windows builds
  - [x] 4.1 Test MSVC build on Windows
    - Install Visual Studio 2022
    - Install vcpkg and SDL2
    - Configure with dev-win64 preset
    - Build all targets (videopac, videopac_tests, disasm_tool)
    - Verify no compilation errors
    - Verify no compilation warnings
    - Run test suite
    - Verify all tests pass
    - _Requirements: 2.10, 6.1, 6.2_

  - [x] 4.2 Test MinGW build on Windows
    - Install MinGW-w64
    - Install vcpkg and SDL2 for MinGW
    - Configure with dev-mingw preset
    - Build all targets
    - Verify no compilation errors
    - Verify no compilation warnings
    - Run test suite
    - Verify all tests pass
    - _Requirements: 3.7, 6.1, 6.2_

  - [x] 4.3 Test emulator functionality on Windows
    - Run videopac.exe with Satellite Attack ROM
    - Verify window opens and displays correctly
    - Verify graphics render correctly
    - Verify audio plays correctly
    - Verify keyboard input works
    - Test save state functionality
    - Test loading different ROM sizes (2KB, 4KB, 8KB)
    - _Requirements: 7.1, 7.2, 7.3, 7.4, 7.5, 7.6_

  - [x] 4.4 Test with different SDL2 installation methods
    - Test with vcpkg SDL2
    - Test with manually installed SDL2
    - Test with static SDL2 linking
    - Test with dynamic SDL2 linking
    - Verify all methods work correctly
    - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5_

  - [x] 4.5 Test path handling on Windows
    - Test loading ROM with forward slashes (roms/game.bin)
    - Test loading ROM with backslashes (roms\game.bin)
    - Test loading ROM with absolute path (C:\path\to\rom.bin)
    - Test loading ROM with spaces in path
    - Test save state paths
    - Verify all path formats work correctly
    - _Requirements: 7.1, 7.2, 7.3, 7.4, 7.5_

  - [ ] 4.6 Performance testing on Windows
    - Measure frame rate with MSVC build
    - Measure frame rate with MinGW build
    - Compare with Linux build performance
    - Verify performance is within 5% of Linux
    - _Requirements: Performance requirements_


- [ ] 5. Create binary distribution package (Optional)
  - [ ] 5.1 Create release build script
    - Script to build Release configuration
    - Script to copy required DLLs
    - Script to create distribution folder structure
    - _Requirements: 8.1, 8.2_

  - [ ] 5.2 Document packaging process
    - Explain how to create release builds
    - List required runtime dependencies
    - Explain static vs dynamic linking trade-offs
    - Provide example distribution structure
    - _Requirements: 8.3, 8.4, 8.5_

  - [ ] 5.3 Create portable ZIP package (Optional)
    - Create videopac-windows-x64.zip
    - Include videopac.exe
    - Include SDL2.dll (if dynamic)
    - Include README.txt with usage instructions
    - Include example ROMs (if licensing allows)
    - _Requirements: 8.6_


- [ ] 6. Set up continuous integration for Windows (Optional)
  - [ ] 6.1 Create GitHub Actions workflow for Windows
    - Add workflow file .github/workflows/windows.yml
    - Configure MSVC build job
    - Configure MinGW build job
    - Install vcpkg and SDL2 in CI
    - Run test suite in CI
    - Upload build artifacts
    - _Requirements: 10.1, 10.2, 10.3, 10.4_

  - [ ] 6.2 Test CI workflow
    - Push changes and verify workflow runs
    - Verify MSVC build succeeds
    - Verify MinGW build succeeds
    - Verify tests pass in CI
    - Verify artifacts are uploaded
    - _Requirements: 10.1, 10.2, 10.3, 10.4_

  - [ ] 6.3 Add build status badges to README
    - Add Windows build status badge
    - Add link to CI workflow
    - _Requirements: 10.5_


- [ ] 7. Final verification and documentation review
  - [ ] 7.1 Review all documentation
    - Verify BUILD-Windows.md is complete and accurate
    - Verify README.md Windows section is clear
    - Verify CMakeUserPresets.json.example has helpful comments
    - Check for typos and formatting issues
    - _Requirements: 5.1-5.10_

  - [ ] 7.2 Verify all requirements are met
    - Go through requirements document
    - Verify each acceptance criterion is satisfied
    - Document any deviations or limitations
    - _Requirements: All_

  - [ ] 7.3 Create Windows build checklist
    - Create quick reference checklist for Windows developers
    - Include common commands
    - Include troubleshooting quick tips
    - _Requirements: 5.1-5.10_

  - [ ] 7.4 Test with fresh Windows installation
    - Use clean Windows VM or PC
    - Follow documentation from scratch
    - Time the setup process (should be under 30 minutes)
    - Note any unclear steps or issues
    - Update documentation based on findings
    - _Requirements: Success criteria_


## Notes

### Task Execution Guidelines

- Tasks marked with `*` are optional and can be skipped for MVP
- Each task references specific requirements for traceability
- Test on actual Windows system before marking tasks complete
- Update documentation immediately when issues are discovered

### Prerequisites for Testing

- Access to Windows 10 or later (VM or physical machine)
- Visual Studio 2019 or 2022 (Community Edition is fine)
- MinGW-w64 8.0 or later
- CMake 3.15 or later
- Git for Windows

### Recommended Testing Order

1. Create preset files first (can be done on Linux)
2. Update CMakeLists.txt (test on Linux first)
3. Create documentation
4. Test on Windows with MSVC
5. Test on Windows with MinGW
6. Create distribution package
7. Set up CI (optional)

### Common Issues to Watch For

- SDL2 linking differences between vcpkg and manual installation
- Path separator handling (should work automatically with std::ifstream)
- MSVC warning level differences from GCC
- MinGW runtime DLL dependencies
- Visual Studio generator version compatibility

### Success Criteria

The Windows build support will be considered complete when:

1. ✅ CMakePresets.json and CMakeUserPresets.json.example are created
2. ✅ Documentation is complete and accurate
3. ✅ MSVC build compiles without errors or warnings
4. ✅ MinGW build compiles without errors or warnings
5. ✅ All tests pass on Windows
6. ✅ Emulator runs Satellite Attack with graphics and audio on Windows
7. ✅ A Windows developer can build from scratch in under 30 minutes
8. ✅ At least one successful Windows build by a different developer

### Future Enhancements

After initial Windows support is complete, consider:

- Windows installer (MSI or NSIS)
- DirectX rendering backend for better performance
- Windows Store distribution
- ARM64 Windows support
- Automated release builds with GitHub Actions
