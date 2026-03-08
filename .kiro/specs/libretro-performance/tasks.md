# Implementation Plan: Libretro Core Performance Profiling and Optimization

## Overview

Profile-first approach: build the benchmark harness and profiling infrastructure, run profiling to identify actual hot paths, then apply targeted optimizations based on real data. Tasks 4-10 contain speculated optimizations based on architectural knowledge — they MUST be validated against profiling results (Task 3.5) before execution. Optimizations targeting functions that are not hot paths should be deprioritized or removed. Each optimization preserves bit-identical framebuffer output. Property-based tests validate correctness alongside implementation.

## Tasks

- [x] 1. Create benchmark harness and profiling infrastructure
  - [x] 1.1 Create `tools/videopac_benchmark.cpp` with CLI parsing and headless frame loop
    - Implement `BenchmarkConfig` struct with defaults (6000 frames, 60 warmup)
    - Parse `--bios`, `--frames`, `--warmup`, `--check`, `--baseline-fps`, `--profile-output` arguments
    - Load BIOS + ROM via `EmulatorCore`, run warmup frames, then timed frames
    - Report wall-clock time, average FPS, average microseconds per frame
    - Report per-subsystem breakdown (CPU %, VDC %, overhead %)
    - Handle errors: missing files (exit 2), init failure (exit 3)
    - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5, 1.6_

  - [x] 1.2 Add `videopac_benchmark` CMake target in `CMakeLists.txt`
    - Add target under `BUILD_TOOLS` section, link against `videopac_core` only (no SDL)
    - _Requirements: 1.6_

  - [x] 1.3 Implement `--check` mode for regression prevention
    - Compare measured FPS against `--baseline-fps * 0.9` threshold
    - Exit code 0 on pass, non-zero + warning on fail
    - Validate `--check` requires `--baseline-fps`, `--baseline-fps > 0`
    - _Requirements: 9.1, 9.2_

  - [ ]* 1.4 Write property test for benchmark check mode threshold (Property 6)
    - **Property 6: Benchmark check mode threshold**
    - Generate random positive (measured_fps, baseline_fps) pairs
    - Verify pass/fail matches `measured_fps >= baseline_fps * 0.9`
    - Add to `tests/property_tests.cpp`
    - **Validates: Requirements 9.1, 9.2**

- [x] 2. Add profiling documentation and CMake presets
  - [x] 2.1 Create `doc/profiling.md` with gprof, Valgrind/Callgrind, and perf workflows
    - Document gprof: build with `profile-mingw`, run binary, `gprof videopac_benchmark.exe gmon.out > profile.txt`
    - Document Valgrind: `valgrind --tool=callgrind ./videopac_benchmark --bios <bios> <rom> --frames 600`
    - Document perf: `perf record ./videopac_benchmark --bios <bios> <rom> --frames 6000`, then `perf report`
    - Include platform availability notes (gprof: MinGW/GCC, Valgrind: Linux, perf: Linux)
    - _Requirements: 0.1, 0.2, 0.3, 0.4, 0.5_

  - [x] 2.2 Add `profile-mingw` preset to `CMakePresets.json`
    - Inherit from `flags-mingw`, `ci-std`, `cmake-pedantic`
    - Add `-pg` to `CMAKE_CXX_FLAGS` and `CMAKE_EXE_LINKER_FLAGS`
    - Build type `RelWithDebInfo`
    - _Requirements: 0.1_

  - [x] 2.3 Add `Release-LTO`, `pgo-generate`, and `pgo-use` presets to `CMakePresets.json`
    - `Release-LTO`: enable `-flto` for `videopac_core` and `videopac_libretro`
    - `pgo-generate`: add `-fprofile-generate` flags
    - `pgo-use`: add `-fprofile-use` flags with profile data path
    - Add PGO workflow documentation as comments in `CMakePresets.json`
    - _Requirements: 8.1, 8.2, 8.3_

- [x] 3. Checkpoint — Verify benchmark harness builds and runs
  - Ensure all tests pass, ask the user if questions arise.

- [x] 3.5. **PROFILING GATE — Run profiling, analyze results, establish baseline**
  - [x] 3.5.1 Run benchmark harness to establish baseline FPS
    - Baseline: Satellite Attack 156.8 FPS, Killer Bees 70.5 FPS (ci-mingw, no instrumentation)
  - [x] 3.5.2 Run gprof profiling to identify actual hot paths
    - gprof required `-no-pie -static` flags on MinGW to produce data
    - Results documented in `doc/profiling-results.md`
  - [x] 3.5.3 Analyze profiling results and revise optimization plan
    - **Key finding**: `CPU::update_counter()` is #1 hot path (~48%), not VDC rendering (~14%)
    - Optimization tasks below reordered by measured impact
    - See `doc/profiling-results.md` for full analysis

---

**Optimization tasks below are ordered by profiling impact (highest first).**
**Baseline: Satellite Attack 156.8 FPS, Killer Bees 70.5 FPS.**

- [x] 4. Optimize VDC render pipeline (profiling: ~14% combined — already done pre-profiling)
  - [x] 4.1 Add sprite cache structures to `include/vdc.h` and rebuild per frame
    - _Requirements: 2.4, 2.5_
  - [x] 4.2 Add bounds-check skip to `render_current_pixel()` in `src/vdc.cpp`
    - _Requirements: 2.1, 2.2, 2.3, 2.5_
  - [x] 4.3 Update `is_sprite_pixel_at()` to use cached sprite data
    - _Requirements: 2.4_
  - [ ]* 4.4 Write property test for bit-identical framebuffer after VDC optimizations (Property 1)
    - **Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5, 9.4**

- [ ] 5. Inline CPU update_counter (profiling: 48 percent of execution time)
  - [ ] 5.1 Move update_counter to cpu.h as inline method
    - The function body is trivial (edge detection + conditional increment) but called 418M times per 3000 frames
    - Function call overhead dominates — inlining eliminates it entirely
    - Move implementation from `src/cpu.cpp` to `include/cpu.h`
    - _Requirements: 5.1 (new), 9.4_

- [ ] 6. [HIGH IMPACT] Inline MasterClock accessors and optimize tick path (profiling: ~28% combined)
  - [ ] 6.1 Move MasterClock::is_hblank() and MasterClock::get_beam_x() to include/master_clock.h
    - Currently in `src/master_clock.cpp`, preventing inlining
    - `is_hblank()` called 366M times (~3%), `get_beam_x()` called 183M times (~1.4%)
    - `get_t1_state()` already inline but calls `is_hblank()` — inlining `is_hblank()` fixes both
    - _Requirements: 6.2_
  - [ ] 6.2 Move `VDC::is_frame_complete()`, `VDC::is_hblank()`, `VDC::get_t1_state()`, `VDC::get_beam_x()` to header if not already inline
    - VDC accessor wrappers called 183-366M times (~5% combined)
    - _Requirements: 6.2_
  - [ ] 6.3 Verify `MasterClock::tick()` uses integer-only comparisons
    - Verify no division or floating-point in `tick()` — use modular arithmetic only
    - `tick()` itself is ~20% of execution time (366M calls)
    - _Requirements: 6.1_

- [ ] 7. Checkpoint — Verify high-impact optimizations, re-profile
  - Build and run all tests
  - Re-run benchmark to measure FPS improvement from Tasks 5-6
  - Compare against baseline (Satellite Attack 156.8 FPS, Killer Bees 70.5 FPS)
  - If improvement is significant, continue to lower-impact tasks
  - Ask the user if questions arise

- [ ] 8. Optimize emulator execution loop (profiling: ~4% run_frame + debugger overhead)
  - [ ] 8.1 Skip debugger checks when no debugger attached in `src/emulator.cpp`
    - Guard `check_debugger_breakpoint()`, `log_instruction()`, `is_vdc_trace_enabled()` calls with `if (debugger_)` checks
    - `check_debugger_breakpoint()` called 9.2M times even with no debugger
    - _Requirements: 5.2_
  - [ ] 8.2 Eliminate profiling overhead when disabled in `run_frame()`
    - Restructure `run_frame()` to avoid `std::chrono` calls entirely when `config_.enable_profile` is false
    - Use compile-time or branch-free pattern: separate fast path (no timing) from profiled path
    - _Requirements: 5.3, 10.3_

- [ ] 9. Optimize VDC collision detection (profiling: included in VDC ~14%)
  - [ ] 9.1 Add collision-enable-zero skip in `tick_one_cycle()`
    - When `registers[0xA2] == 0`, skip all collision detection logic entirely
    - _Requirements: 3.1_
  - [ ] 9.2 Update `detect_collision_at_pixel()` to use cached sprite data and early exit
    - Use `sprite_cache_` for bounding box checks instead of re-decoding registers
    - Exit sprite loop early upon finding first matching sprite bit
    - _Requirements: 3.2, 3.3_
  - [ ]* 9.3 Write property test for zero collision state when disabled (Property 2)
    - **Validates: Requirements 3.1**

- [ ] 10. Optimize VDC audio path (profiling: ~2% capture_audio_sample)
  - [ ] 10.1 Add audio-disabled skip in `tick_one_cycle()`
    - When sound control enable bit is 0, skip both `update_audio()` and `capture_audio_sample()`
    - Add buffer-full early return in `capture_audio_sample()`
    - _Requirements: 4.1, 4.2_
  - [ ]* 10.2 Write property test for silent audio when disabled (Property 3)
    - **Validates: Requirements 4.1**

- [ ] 11. Optimize framebuffer conversion in libretro core (profiling: not visible per-tick, per-frame only)
  - [ ] 11.1 Add precomputed palette LUT to `src/libretro.cpp`
    - _Requirements: 7.1_
  - [ ]* 11.2 Write property test for palette LUT equivalence (Property 4)
    - **Validates: Requirements 7.1**
  - [ ] 11.3 Implement branchless mono-to-stereo conversion in `retro_run()`
    - _Requirements: 7.2_
  - [ ]* 11.4 Write property test for mono-to-stereo duplication (Property 5)
    - **Validates: Requirements 7.2**

- [ ] 12. Checkpoint — Verify all optimizations preserve correctness
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 13. Enhance profiling mode with CSV output and summary statistics
  - [ ] 13.1 Add `profile_output_path` to `Configuration` in `include/emulator.h`
    - Add `std::string profile_output_path` field
    - Add `FrameTiming` struct (frame_number, cpu_us, vdc_us, overhead_us, total_us)
    - Add `ProfilingSummary` struct (min, max, mean, p95 frame time)
    - _Requirements: 10.1, 10.2_
  - [ ] 13.2 Implement CSV output and summary statistics in `src/emulator.cpp`
    - _Requirements: 10.1, 10.2_
  - [ ] 13.3 Wire `--profile-output` argument from benchmark harness to `Configuration`
    - _Requirements: 10.1_
  - [ ]* 13.4 Write property test for profiling summary statistics (Property 7)
    - **Validates: Requirements 10.2**

- [ ] 14. Final checkpoint — Full regression verification
  - Ensure all tests pass, ask the user if questions arise.
  - Re-run benchmark and compare against baseline
  - Run gprof again to verify hot paths have shifted
  - Verify existing test suite passes after all optimizations (Requirement 9.3)

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Property tests use RapidCheck (already integrated via CMake FetchContent)
- All optimizations must preserve bit-identical framebuffer output (Requirement 9.4)
- Checkpoints ensure incremental validation after each optimization group
- The benchmark harness (Task 1) should be used to measure before/after each optimization group
- **CRITICAL DEPENDENCY**: Tasks 4-10 are speculated optimizations. Task 3.5 (profiling gate) MUST be completed and reviewed before executing any optimization task. Profiling results may reprioritize, modify, or eliminate optimization tasks.
