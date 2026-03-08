# Requirements Document

## Introduction

The videopac emulator is used as a libretro core in the retro-ai reinforcement learning framework, where it runs headless at maximum speed. Training throughput is bottlenecked by emulation performance. This feature covers profiling the emulator to identify hot paths, optimizing the critical inner loops (VDC `tick_one_cycle()`, CPU instruction dispatch, collision detection, framebuffer writes), and adding a benchmark harness to measure and prevent performance regressions.

The emulator's architecture is cycle-accurate: a master clock drives interleaved CPU and VDC execution. For NTSC, each frame requires ~59,605 VDC cycles (264 scanlines × ~227 VDC ticks). The VDC's `tick_one_cycle()` is called once per VDC tick and performs per-pixel rendering, collision detection, audio sampling, and status register updates — making it the dominant hot path.

## Glossary

- **Emulator_Core**: The `EmulatorCore` class (`src/emulator.cpp`) that orchestrates per-frame execution of CPU, VDC, and master clock.
- **VDC**: The Video Display Controller (`src/vdc.cpp`), Intel 8245 emulation. Renders pixels, detects collisions, and generates audio.
- **CPU**: The MCS-48 processor emulation (`src/cpu.cpp`). Executes instructions interleaved with VDC ticks.
- **Master_Clock**: The timing coordinator (`src/master_clock.cpp`) that determines whether CPU, VDC, or both execute on each master tick.
- **Benchmark_Harness**: A standalone test program that loads a ROM and runs N frames headless, measuring wall-clock time and reporting frames per second.
- **Hot_Path**: A code path executed millions of times per frame (VDC tick, CPU dispatch, collision detection).
- **Headless_Mode**: Emulation without display output, used during AI training for maximum throughput.
- **Libretro_Core**: The shared library (`src/libretro.cpp`) exposing the libretro API for use with RetroArch or retro-ai.
- **Framebuffer_Conversion**: The `convert_framebuffer()` function in `src/libretro.cpp` that converts palette-indexed pixels to XRGB8888.
- **Profiling_Mode**: The existing `enable_profile` configuration flag that enables per-frame timing instrumentation in `run_frame()`.

## Requirements

### Requirement 0: Profiling Toolchain

**User Story:** As a developer, I want a documented profiling workflow using portable tools, so that I can identify performance bottlenecks on any supported platform.

#### Acceptance Criteria

1. THE CMake build system SHALL provide a `profile-mingw` preset that adds `-pg` to compiler and linker flags, enabling gprof instrumentation for the MinGW/GCC toolchain.
2. THE profiling workflow SHALL document how to run gprof on the benchmark harness: build with `profile-mingw`, run the binary to generate `gmon.out`, then `gprof videopac_benchmark.exe gmon.out > profile.txt`.
3. THE profiling workflow SHALL document how to use Valgrind/Callgrind on Linux: `valgrind --tool=callgrind ./videopac_benchmark --bios <bios> <rom> --frames 600`, then `callgrind_annotate callgrind.out.<pid>` or visualize with KCachegrind.
4. THE profiling workflow SHALL document how to use `perf` on Linux for sampling-based profiling: `perf record ./videopac_benchmark --bios <bios> <rom> --frames 6000`, then `perf report`.
5. THE profiling documentation SHALL be added to a `doc/profiling.md` file covering all three tools (gprof, Valgrind/Callgrind, perf) with platform availability notes.
6. *(Optional — future)* THE profiling documentation SHALL include a section on Tracy Profiler (github.com/wolfpld/tracy) as a real-time frame profiler option: adding `ZoneScoped` macros to hot functions, compiling with `TRACY_ENABLE`, and connecting the Tracy GUI for visual timeline analysis. Tracy is cross-platform (Windows/Linux/macOS) with zero overhead when compiled out.
7. *(Optional — future)* THE profiling documentation SHALL include a section on Google Benchmark (github.com/google/benchmark) as a micro-benchmarking option for A/B testing specific functions: integrating via CMake `FetchContent`, writing benchmark fixtures for isolated subsystems (e.g., scanline rendering, CPU dispatch), and interpreting statistical output.

### Requirement 1: Benchmark Harness

**User Story:** As a developer, I want a standalone benchmark tool that measures headless emulation throughput, so that I can quantify performance before and after optimizations.

#### Acceptance Criteria

1. THE Benchmark_Harness SHALL load a BIOS file and a ROM file from command-line arguments and run a configurable number of frames in Headless_Mode.
2. WHEN the benchmark run completes, THE Benchmark_Harness SHALL report total wall-clock time, average frames per second, and average microseconds per frame to standard output.
3. WHEN the benchmark run completes, THE Benchmark_Harness SHALL report per-subsystem breakdown (CPU time percentage, VDC time percentage, overhead time percentage) to standard output.
4. THE Benchmark_Harness SHALL default to 6000 frames (100 seconds of NTSC emulation) when no frame count is specified.
5. THE Benchmark_Harness SHALL accept a `--warmup` argument specifying a number of frames to run before measurement begins, defaulting to 60 frames.
6. THE Benchmark_Harness SHALL be buildable as a CMake target named `videopac_benchmark` without requiring SDL or any display library.

### Requirement 2: VDC Hot Path Optimization — Render Pipeline

**User Story:** As a developer, I want the VDC per-pixel rendering path optimized, so that headless emulation throughput increases.

#### Acceptance Criteria

1. WHILE the display is enabled, THE VDC SHALL render pixels using `render_current_pixel()` without redundant coordinate conversion when the extended framebuffer mode is disabled.
2. THE VDC SHALL avoid calling `is_grid_pixel_at()`, `is_character_pixel_at()`, and `is_sprite_pixel_at()` for beam positions outside the framebuffer bounds.
3. WHILE the grid is disabled, THE VDC SHALL skip all grid pixel checks in `render_current_pixel()`.
4. THE VDC SHALL cache sprite bounding box data per frame to avoid recomputing sprite Y range, double-size flag, and shift attributes on every pixel.
5. WHEN no sprites are visible on the current scanline, THE VDC SHALL skip all sprite pixel checks for that scanline.

### Requirement 3: VDC Hot Path Optimization — Collision Detection

**User Story:** As a developer, I want collision detection optimized, so that per-pixel collision checks do not dominate frame time.

#### Acceptance Criteria

1. WHEN the collision enable register is zero, THE VDC SHALL skip all collision detection logic in `tick_one_cycle()`.
2. THE VDC SHALL use cached sprite bounding box data (from Requirement 2) during collision detection to avoid redundant attribute decoding.
3. WHEN checking sprite presence at a pixel during collision detection, THE VDC SHALL exit the sprite loop early upon finding the first matching sprite bit, rather than iterating all pixel columns.

### Requirement 4: VDC Hot Path Optimization — Audio Path

**User Story:** As a developer, I want the audio sampling path in `tick_one_cycle()` optimized, so that audio processing does not add unnecessary overhead per VDC cycle.

#### Acceptance Criteria

1. WHEN audio is disabled (sound control enable bit is 0), THE VDC SHALL skip both `update_audio()` and `capture_audio_sample()` calls in `tick_one_cycle()`.
2. THE VDC SHALL avoid the fixed-point accumulator comparison in `capture_audio_sample()` when the audio sample buffer is full.

### Requirement 5: CPU Instruction Dispatch Optimization

**User Story:** As a developer, I want CPU instruction execution optimized, so that the CPU does not become a bottleneck relative to the VDC.

#### Acceptance Criteria

1. THE CPU SHALL execute instructions via a flat switch-case dispatch (current implementation) or a function pointer table, avoiding any dynamic dispatch overhead.
2. WHEN the debugger is not attached, THE Emulator_Core SHALL skip all debugger-related checks (breakpoint check, trace logging, VDC trace check) in the main execution loop.
3. WHEN profiling mode is disabled, THE Emulator_Core SHALL skip all `std::chrono` timing calls in the main execution loop, incurring zero timing overhead.

### Requirement 6: Master Clock Tick Optimization

**User Story:** As a developer, I want the master clock tick path optimized, so that the per-tick scheduling overhead is minimal.

#### Acceptance Criteria

1. THE Master_Clock `tick()` method SHALL determine the next execution target (CPU, VDC, BOTH, NONE) using only integer comparisons and modular arithmetic, without division or floating-point operations.
2. THE Master_Clock SHALL use inline-friendly code for `get_beam_x()`, `is_hblank()`, and `get_t1_state()` to allow the compiler to inline these into `tick_one_cycle()`.

### Requirement 7: Framebuffer Conversion Optimization

**User Story:** As a developer, I want the palette-to-XRGB8888 framebuffer conversion in the libretro core optimized, so that it does not add significant per-frame overhead.

#### Acceptance Criteria

1. THE Libretro_Core SHALL convert the palette-indexed framebuffer to XRGB8888 using a precomputed 16-entry lookup table, performing one table lookup per pixel.
2. THE Libretro_Core SHALL convert the audio buffer from mono to stereo without per-sample branching.

### Requirement 8: Compile-Time Optimization Support

**User Story:** As a developer, I want the build system to support profile-guided optimization and link-time optimization, so that the compiler can optimize across translation units.

#### Acceptance Criteria

1. THE CMake build system SHALL provide a `Release-LTO` preset or option that enables link-time optimization (LTO) for the `videopac_core` library and `videopac_libretro` target.
2. THE CMake build system SHALL support a two-pass profile-guided optimization (PGO) workflow: a `pgo-generate` build that instruments the binary, and a `pgo-use` build that applies the collected profile data.
3. THE CMake build system SHALL document the PGO workflow in a comment or README section within CMakeLists.txt or CMakePresets.json.

### Requirement 9: Performance Regression Prevention

**User Story:** As a developer, I want performance regressions detected automatically, so that future changes do not silently degrade headless throughput.

#### Acceptance Criteria

1. THE Benchmark_Harness SHALL support a `--check` mode that compares measured FPS against a baseline value provided via `--baseline-fps` argument.
2. WHEN the measured FPS is more than 10% below the baseline, THE Benchmark_Harness SHALL exit with a non-zero exit code and print a warning message.
3. THE existing test suite SHALL continue to pass after all optimizations are applied, verifying that emulation correctness is preserved.
4. IF an optimization changes the output of `get_framebuffer()` for any frame, THEN THE Emulator_Core SHALL produce bit-identical framebuffer output compared to the pre-optimization implementation.

### Requirement 10: Profiling Mode Improvements

**User Story:** As a developer, I want the existing profiling mode enhanced with structured output, so that I can track performance over time and across commits.

#### Acceptance Criteria

1. WHEN profiling mode is enabled, THE Emulator_Core SHALL support outputting per-frame timing data in CSV format (frame_number, cpu_us, vdc_us, overhead_us, total_us) to a file specified by a `--profile-output` argument.
2. WHEN profiling mode is enabled and no output file is specified, THE Emulator_Core SHALL print summary statistics (min, max, mean, p95 frame time) at the end of execution instead of per-frame console output every 60 frames.
3. THE Profiling_Mode SHALL add zero overhead to the execution loop when disabled (no conditional branches in the hot path for profiling checks).
