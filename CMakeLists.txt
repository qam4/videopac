cmake_minimum_required(VERSION 3.15)
project(videopac_emulator VERSION 1.0.0 LANGUAGES C CXX)

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
option(O2EM_COMPAT "Enable O2EM compatibility mode (VBlank timing and timer quirks)" OFF)

# Apply O2EM_COMPAT compile definition if enabled
if(O2EM_COMPAT)
    add_compile_definitions(O2EM_COMPAT)
    message(STATUS "O2EM_COMPAT enabled: Using O2EM-compatible behavior")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party)

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
    src/vkeyboard.cpp
)

# UI source files
set(UI_SOURCES
    src/ui/config_manager.cpp
    src/ui/zip_handler.cpp
    src/ui/text_renderer.cpp
    src/ui/dialogs.cpp
    src/ui/file_browser.cpp
    src/ui/menu_system.cpp
    src/ui/osd_renderer.cpp
    src/ui/save_state_manager.cpp
    src/ui/input_mapper.cpp
    src/ui/recent_files_list.cpp
    src/ui/imgui_debugger_ui.cpp
)

# Core emulator library
add_library(videopac_core STATIC ${CORE_SOURCES})
target_include_directories(videopac_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
# Required for linking into shared libraries (libretro .so) on Linux
set_target_properties(videopac_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Find miniz for ZIP support
find_package(miniz CONFIG QUIET)
if(miniz_FOUND)
    target_link_libraries(videopac_core PUBLIC miniz::miniz)
    target_compile_definitions(videopac_core PUBLIC HAVE_MINIZ)
    message(STATUS "Building with miniz support for ZIP files")
else()
    message(WARNING "miniz not found. ZIP file support will be disabled.")
endif()

# Dear ImGui (for debugger UI) - must be before videopac executable
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
)
FetchContent_MakeAvailable(imgui)

# ImGui source files
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

# Disable strict-aliasing warnings for ImGui (it has some type-punning)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(${IMGUI_SOURCES} PROPERTIES
        COMPILE_FLAGS "-Wno-error=strict-aliasing"
    )
    # stb_image_write.h uses sprintf, deprecated on macOS Xcode 16.4+
    set_source_files_properties(src/ui/save_state_manager.cpp PROPERTIES
        COMPILE_FLAGS "-Wno-error=deprecated-declarations"
    )
endif()

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
            
            # Try to find SDL2_ttf (optional for text rendering)
            find_package(SDL2_ttf CONFIG QUIET)
        else()
            find_package(SDL2)
            find_package(SDL2_ttf QUIET)
        endif()
        
        if(SDL2_FOUND)
            add_executable(videopac
                src/main.cpp
                src/frontend_sdl.cpp
                ${UI_SOURCES}
            )
            
            # Add ImGui sources to videopac target
            target_sources(videopac PRIVATE ${IMGUI_SOURCES})
            target_include_directories(videopac PRIVATE 
                ${imgui_SOURCE_DIR} 
                ${imgui_SOURCE_DIR}/backends
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
                
                # Link SDL2_ttf if available
                if(TARGET SDL2_ttf::SDL2_ttf)
                    target_link_libraries(videopac SDL2_ttf::SDL2_ttf)
                    target_compile_definitions(videopac PRIVATE HAVE_SDL_TTF)
                    message(STATUS "Building with SDL2_ttf support")
                else()
                    message(STATUS "SDL2_ttf not found, using fallback text rendering")
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
        src/ui/zip_handler.cpp
    )
    target_link_libraries(disasm_tool videopac_core)
    if(miniz_FOUND)
        target_link_libraries(disasm_tool miniz::miniz)
        target_compile_definitions(disasm_tool PRIVATE HAVE_MINIZ)
    endif()

    add_executable(smoke_test_roms
        tools/smoke_test_roms.cpp
    )
    target_link_libraries(smoke_test_roms videopac_core)

    add_executable(videopac_benchmark
        tools/videopac_benchmark.cpp
    )
    target_link_libraries(videopac_benchmark videopac_core)

    add_executable(dump_audio
        tools/dump_audio.cpp
    )
    target_link_libraries(dump_audio videopac_core)

    add_executable(quick_profile
        tools/quick_profile.cpp
    )
    target_link_libraries(quick_profile videopac_core)
endif()

# Libretro core
if(BUILD_LIBRETRO)
    add_library(videopac_libretro SHARED
        src/libretro.cpp
        src/vkeyboard.cpp
    )
    
    target_link_libraries(videopac_libretro videopac_core)
    target_include_directories(videopac_libretro PRIVATE ${CMAKE_SOURCE_DIR}/include)
    
    # Libretro naming convention
    set_target_properties(videopac_libretro PROPERTIES
        PREFIX ""
        OUTPUT_NAME "videopac_libretro"
    )
    
    # Hide all symbols except retro_* on non-Windows
    if(NOT WIN32)
        target_compile_options(videopac_libretro PRIVATE -fvisibility=hidden)
    endif()
endif()

# Install rules
if(BUILD_STANDALONE AND TARGET videopac)
    install(TARGETS videopac RUNTIME DESTINATION bin)
    # Bundle SDL2.dll on Windows
    if(WIN32 AND TARGET SDL2::SDL2)
        install(FILES $<TARGET_FILE:SDL2::SDL2> DESTINATION bin)
    endif()
endif()
if(BUILD_LIBRETRO AND TARGET videopac_libretro)
    install(TARGETS videopac_libretro LIBRARY DESTINATION lib RUNTIME DESTINATION lib)
    install(FILES ${CMAKE_SOURCE_DIR}/videopac_libretro.info DESTINATION lib)
endif()

# Testing
if(BUILD_TESTS)
    enable_testing()
    
    # Google Test (EXCLUDE_FROM_ALL to suppress install rules from release artifacts)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_GetProperties(googletest)
    if(NOT googletest_POPULATED)
        FetchContent_Populate(googletest)
        add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
    
    # RapidCheck (use add_subdirectory with EXCLUDE_FROM_ALL to suppress install rules)
    FetchContent_Declare(
        rapidcheck
        GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
        GIT_TAG master
    )
    FetchContent_GetProperties(rapidcheck)
    if(NOT rapidcheck_POPULATED)
        FetchContent_Populate(rapidcheck)
        add_subdirectory(${rapidcheck_SOURCE_DIR} ${rapidcheck_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
    
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
        tests/test_file_browser.cpp
        tests/test_osd_renderer.cpp
        tests/test_save_state_manager_ui.cpp
        tests/test_input_mapper.cpp
        tests/test_recent_files_list.cpp
        tests/test_config_manager.cpp
        tests/test_reset_functionality.cpp
        tests/test_watch_expression.cpp
        tests/test_imgui_debugger_ui.cpp
        tests/property_tests.cpp
        tests/test_vkeyboard.cpp
        tests/property_tests_vkeyboard.cpp
        tests/test_audio.cpp
        tests/property_tests_audio.cpp
        # Include UI sources for testing
        ${UI_SOURCES}
    )
    
    # Add ImGui sources and includes to test target
    target_sources(videopac_tests PRIVATE ${IMGUI_SOURCES})
    target_include_directories(videopac_tests PRIVATE 
        ${imgui_SOURCE_DIR} 
        ${imgui_SOURCE_DIR}/backends
    )
    
    target_link_libraries(videopac_tests
        videopac_core
        gtest_main
        rapidcheck
    )
    
    # Link SDL2 for UI tests
    if(TARGET SDL2::SDL2)
        target_link_libraries(videopac_tests SDL2::SDL2)
        if(TARGET SDL2_ttf::SDL2_ttf)
            target_link_libraries(videopac_tests SDL2_ttf::SDL2_ttf)
        endif()
    elseif(SDL2_FOUND)
        target_include_directories(videopac_tests PRIVATE ${SDL2_INCLUDE_DIRS})
        target_link_libraries(videopac_tests ${SDL2_LIBRARIES})
        if(SDL2_ttf_FOUND)
            target_link_libraries(videopac_tests ${SDL2_TTF_LIBRARIES})
        endif()
    endif()
    
    include(GoogleTest)
    gtest_discover_tests(videopac_tests)
endif()
