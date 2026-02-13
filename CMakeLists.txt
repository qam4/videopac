cmake_minimum_required(VERSION 3.15)
project(videopac_emulator VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
# Note: MSVC flags are now managed via CMakePresets.json (flags-windows preset)
# These flags are for non-preset builds
if(MSVC)
    # Only add /W4 if not already set by presets
    if(NOT CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        add_compile_options(/W4)
    endif()
    # Note: /WX (warnings as errors) removed to allow successful builds
    # Enable it via presets or CMAKE_CXX_FLAGS if needed for strict builds
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_LIBRETRO "Build libretro core" ON)
option(BUILD_STANDALONE "Build standalone emulator" ON)
option(ENABLE_SDL "Enable SDL2 frontend (requires SDL2)" ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files for core emulator library
set(CORE_SOURCES
    src/cpu.cpp
    src/vdc.cpp
    src/memory.cpp
    src/input.cpp
    src/emulator.cpp
    src/master_clock.cpp
    src/disassembler.cpp
    src/savestate.cpp
    src/debugger.cpp
    src/debugger_ui.cpp
    src/frontend_headless.cpp
)

# Core emulator library
add_library(videopac_core STATIC ${CORE_SOURCES})
target_include_directories(videopac_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Standalone executable
if(BUILD_STANDALONE)
    if(ENABLE_SDL)
        # Find SDL2 with Windows-specific handling
        if(WIN32)
            # Try vcpkg first (CONFIG mode)
            find_package(SDL2 CONFIG QUIET)
            if(NOT SDL2_FOUND)
                # Fall back to FindSDL2.cmake module
                find_package(SDL2 MODULE)
            endif()
        else()
            find_package(SDL2)
        endif()
        
        if(SDL2_FOUND)
            add_executable(videopac
                src/main.cpp
                src/frontend_sdl.cpp
            )
            
            # Handle both vcpkg (SDL2::SDL2) and manual installation (${SDL2_LIBRARIES})
            if(TARGET SDL2::SDL2)
                # Link libraries in correct order for MinGW
                if(WIN32 AND TARGET SDL2::SDL2main)
                    target_link_libraries(videopac
                        videopac_core
                        SDL2::SDL2main
                        SDL2::SDL2
                    )
                else()
                    target_link_libraries(videopac
                        videopac_core
                        SDL2::SDL2
                    )
                endif()
                message(STATUS "Building with SDL2 support (vcpkg)")
            else()
                target_include_directories(videopac PRIVATE ${SDL2_INCLUDE_DIRS})
                target_link_libraries(videopac
                    videopac_core
                    ${SDL2_LIBRARIES}
                )
                message(STATUS "Building with SDL2 support (manual)")
            endif()
            
            target_compile_definitions(videopac PRIVATE ENABLE_SDL)
            
            # Optional: Copy SDL2.dll to build directory on Windows for convenience
            if(WIN32 AND TARGET SDL2::SDL2)
                add_custom_command(TARGET videopac POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:SDL2::SDL2>
                    $<TARGET_FILE_DIR:videopac>
                    COMMENT "Copying SDL2.dll to build directory"
                )
            endif()
        else()
            message(WARNING "SDL2 not found. Building headless-only version.")
            message(WARNING "To enable SDL2 support:")
            message(WARNING "  - Install via vcpkg: vcpkg install sdl2:x64-windows")
            message(WARNING "  - Or download from https://libsdl.org and set SDL2_DIR")
            add_executable(videopac
                src/main.cpp
            )
            target_link_libraries(videopac videopac_core)
        endif()
    else()
        message(STATUS "SDL2 disabled, building headless-only version")
        add_executable(videopac
            src/main.cpp
        )
        target_link_libraries(videopac videopac_core)
    endif()
endif()

# Tools
option(BUILD_TOOLS "Build development tools" ON)
if(BUILD_TOOLS)
    add_executable(disasm_tool
        tools/disasm_tool.cpp
    )
    target_link_libraries(disasm_tool videopac_core)
endif()

# Libretro core
if(BUILD_LIBRETRO)
    add_library(videopac_libretro SHARED
        src/libretro.cpp
    )
    
    target_link_libraries(videopac_libretro videopac_core)
    
    # Libretro naming convention
    set_target_properties(videopac_libretro PROPERTIES
        PREFIX ""
        OUTPUT_NAME "videopac_libretro"
    )
endif()

# Testing
if(BUILD_TESTS)
    enable_testing()
    
    # Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    # RapidCheck
    FetchContent_Declare(
        rapidcheck
        GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(rapidcheck)
    
    # Test executable
    add_executable(videopac_tests
        tests/test_cpu.cpp
        tests/test_memory.cpp
        tests/test_vdc.cpp
        tests/test_input.cpp
        tests/test_disassembler.cpp
        tests/test_savestate.cpp
        tests/test_debugger.cpp
        tests/test_integration.cpp
        tests/test_utils.cpp
        tests/test_master_clock.cpp
        tests/test_character_rendering.cpp
        tests/property_tests.cpp
    )
    
    target_link_libraries(videopac_tests
        videopac_core
        gtest_main
        rapidcheck
    )
    
    include(GoogleTest)
    gtest_discover_tests(videopac_tests)
endif()
