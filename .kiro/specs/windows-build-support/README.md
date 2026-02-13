# Windows Build Support Spec

## Overview

This spec adds native Windows build support for the Videopac emulator using both MSVC (Visual Studio) and MinGW-w64 toolchains. The implementation uses modern CMake presets for clean, reproducible builds.

## Goals

1. Enable Windows developers to build the emulator with MSVC or MinGW-w64
2. Provide clear documentation for Windows build process
3. Support SDL2 installation via vcpkg or manual download
4. Maintain compatibility with existing Linux builds
5. Enable full graphics and audio support on Windows

## Key Features

- **CMake Presets**: Modern preset-based configuration
- **Dual Toolchain Support**: Both MSVC and MinGW-w64
- **Flexible SDL2 Setup**: vcpkg or manual installation
- **Comprehensive Documentation**: Step-by-step Windows build guide
- **IDE Integration**: Works with Visual Studio, VS Code, and CLion
- **Optional CI**: GitHub Actions workflow for automated Windows builds

## Files in This Spec

- **requirements.md**: Detailed requirements with acceptance criteria
- **design.md**: Architecture and implementation approach
- **tasks.md**: Step-by-step implementation tasks
- **README.md**: This file

## Quick Start (for implementers)

1. **Read the requirements** (requirements.md) to understand what needs to be built
2. **Review the design** (design.md) to understand the architecture
3. **Follow the tasks** (tasks.md) in order to implement

## Implementation Phases

### Phase 1: Configuration Files (Can do on Linux)
- Create CMakePresets.json
- Create CMakeUserPresets.json.example
- Update .gitignore

### Phase 2: CMake Updates (Test on Linux first)
- Update CMakeLists.txt for Windows SDL2 detection
- Add Windows-specific linking
- Verify backward compatibility

### Phase 3: Documentation
- Create doc/BUILD-Windows.md
- Update README.md
- Document IDE integration

### Phase 4: Windows Testing (Requires Windows system)
- Test MSVC build
- Test MinGW build
- Test emulator functionality
- Performance testing

### Phase 5: Distribution (Optional)
- Create release build script
- Create portable package
- Document packaging process

### Phase 6: CI (Optional)
- Create GitHub Actions workflow
- Test automated builds
- Add status badges

## Key Design Decisions

1. **CMake Presets over manual configuration**: Provides reproducible builds
2. **vcpkg recommended for SDL2**: Easiest for Windows developers
3. **Support both MSVC and MinGW**: Flexibility for different workflows
4. **Minimal code changes**: Leverage existing portable C++ code
5. **Documentation-first**: Clear guides reduce support burden

## Dependencies

- CMake 3.15 or later
- Visual Studio 2019/2022 OR MinGW-w64 8.0+
- SDL2 (via vcpkg or manual installation)
- Google Test (fetched by CMake)
- RapidCheck (fetched by CMake)

## Testing Requirements

All tests must pass on Windows:
- Unit tests (Google Test)
- Property-based tests (RapidCheck)
- Integration tests
- Manual testing with Satellite Attack ROM

## Success Criteria

✅ MSVC build compiles without errors/warnings
✅ MinGW build compiles without errors/warnings
✅ All tests pass on Windows
✅ Emulator runs with graphics and audio on Windows
✅ Documentation is clear and complete
✅ Setup time under 30 minutes for new Windows developer

## Related Specs

This spec builds on the main Videopac emulator spec:
- `.kiro/specs/videopac-emulator/` - Main emulator implementation

## Questions or Issues?

If you encounter issues during implementation:
1. Check the troubleshooting section in BUILD-Windows.md
2. Review the design document for architecture details
3. Consult the requirements for acceptance criteria
4. Ask for clarification if requirements are unclear

## Next Steps

1. Review all three spec documents (requirements, design, tasks)
2. Set up a Windows development environment (or VM)
3. Start with Phase 1 tasks (can be done on Linux)
4. Test on Windows as early as possible
5. Update documentation based on real-world testing

Good luck with the implementation! 🚀
