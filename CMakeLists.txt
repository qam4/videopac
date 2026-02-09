cmake_minimum_required(VERSION 3.15)
project(videopac_emulator VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX)
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

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files for core emulator library
set(CORE_SOURCES
    src/cpu.cpp
    src/vdc.cpp
    src/memory.cpp
    src/input.cpp
    src/emulator.cpp
    src/disassembler.cpp
    src/savestate.cpp
    src/debugger.cpp
)

# Core emulator library
add_library(videopac_core STATIC ${CORE_SOURCES})
target_include_directories(videopac_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Standalone executable
if(BUILD_STANDALONE)
    find_package(SDL2 REQUIRED)
    
    add_executable(videopac
        src/main.cpp
        src/frontend_sdl.cpp
    )
    
    target_include_directories(videopac PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(videopac
        videopac_core
        ${SDL2_LIBRARIES}
    )
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
        tests/test_integration.cpp
        tests/test_utils.cpp
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
