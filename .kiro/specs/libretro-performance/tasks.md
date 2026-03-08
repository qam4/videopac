# Implementation Plan: Libretro Core Performance Profiling and Optimization

## Overview

Profile-first approach: build the benchmark harness and profiling infrastructure, run profiling to identify actual hot paths, then apply targeted optimizations based on real data. Each optimization preserves bit-identical framebuffer output. Property-based tests validate correctness alongside implementation.

## Measurement Methodology

**Primary metric: `perf stat` CPU cycle counts.** Cycle counts measure intrinsic code
speed independent of system load — if another process is hogging the CPU, your process
gets fewer time slices but uses the same number of cycles. This makes A/B comparisons
reliable on shared machines.

**Secondary metric: wall-clock FPS.** Useful for real-world throughput but sensitive to
system load. On shared instances, FPS varied 30-50% between runs (270-420 FPS for the
same binary). Use FPS for user-facing reporting; use cycles for optimization decisions.

**Baseline (perf stat, Satellite Attack, 600 frames, ci-linux -O2):**
- **7.91B cycles, 32.25B instructions, 4.08 IPC**

**Current (after tasks 4-6):**
- **5.82B cycles (-26.4%), 22.43B instructions (-30.4%), 3.85 IPC**

## Tasks

- [x] 1. Create benchmark harness and profiling infrastructure
  - [x] 1.1 Create `tools/videopac_benchmark.cpp` with CLI parsing and headless frame loop
    - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5, 1.6_
  - [x] 1.2 Add `videopac_benchmark` CMake target in `CMakeLists.txt`
    - _Requirements: 1.6_
  - [x] 1.3 Implement `--check` mode for regression prevention
    - _Requirements: 9.1, 9.2_
  - [ ]* 1.4 Write property test for benchmark check mode threshold (Property 6)
    - **Validates: Requirements 9.1, 9.2**

- [x] 2. Add profiling documentation and CMake presets
  - [x] 2.1 Create `doc/profiling.md` with gprof, Valgrind/Callgrind, and perf workflows
    - _Requirements: 0.1, 0.2, 0.3, 0.4, 0.5_
  - [x] 2.2 Add `profile-mingw` preset to `CMakePresets.json`
    - _Requirements: 0.1_
  - [x] 2.3 Add `Release-LTO`, `pgo-generate`, and `pgo-use` presets to `CMakePresets.json`
    - _Requirements: 8.1, 8.2, 8.3_

- [x] 3. Checkpoint — Verify benchmark harness builds and runs

- [x] 3.5. **PROFILING GATE — Run profiling, analyze results, establish baseline**
  - [x] 3.5.1 Run benchmark harness to establish baseline FPS
    - Windows baseline (unreliable): Satellite Attack 156.8 FPS (ci-mingw)
  - [x] 3.5.2 Run gprof profiling on Windows (initial)
    - Results documented in `doc/profiling-results.md`
  - [x] 3.5.3 Run perf profiling on Linux (authoritative)
    - **Linux baseline: Satellite Attack 419 FPS** (ci-linux, GCC 7.3.1, -O2, stable ±1%)
    - perf record with 37K samples, see `doc/profiling-results.md`
  - [x] 3.5.4 Analyze results and revise optimization plan
    - Previous optimizations (sprite cache, inline update_counter, inline MasterClock accessors)
      were reverted after Linux perf showed they had zero measurable impact — GCC -O2 already
      inlines small functions, and sprite/collision code is <1% of execution time.
    - **Actual hot paths (Linux perf):**
      - `VDC::is_character_pixel_at()` — 35.5%
      - `MasterClock::tick()` — 21.3%
      - `VDC::render_current_pixel()` — 12.2%
      - `VDC::is_grid_pixel_at()` — 7.0%
      - `VDC::tick_one_cycle()` — 6.9%
      - VDC accessor wrappers (get_t1_state, is_frame_complete, is_hblank) — 7.9%
      - `EmulatorCore::run_frame()` — 3.7%
      - `VDC::capture_audio_sample()` — 1.7%

---

**Optimization tasks below are ordered by Linux perf impact (highest first).**
**Baseline: 7.91B cycles / 32.25B instructions (Satellite Attack, 600 frames, ci-linux -O2).**

- [x] 4. [HIGHEST IMPACT] Optimize VDC::is_character_pixel_at (perf: 35.5%)
  - [x] 4.1 Profile `is_character_pixel_at()` to identify inner bottleneck
    - perf annotate showed Y bounds comparisons and height calculation math dominate
  - [x] 4.2 Add quick Y-range reject using max character height (16 scanlines)
    - Check `y < char_y || y >= char_y + 16` before computing exact height
    - For quads: check quad Y once and skip all 4 sub-characters
    - Reorder X check before height calculation
    - _Requirements: 2.2, 9.4_
  - Result: 10B fewer instructions (-32%), 1.5B fewer cycles (-19% cumulative)

- [x] 5. [HIGH IMPACT] Optimize MasterClock::tick (perf: 21.3%)
  - [x] 5.1 Replace modulo operations with countdown counters
    - `% vdc_tick_divisor_` and `% cpu_tick_divisor_` replaced with decrementing counters
    - Result: 480M fewer cycles (-6%), modulo→dec+jnz
    - _Requirements: 6.1_
  - [x] 5.2 Verify tick() uses integer-only comparisons throughout
    - Confirmed: no division or floating-point in hot path
    - _Requirements: 6.1_

- [x] 6. [HIGH IMPACT] Inline VDC accessor wrappers (perf: 7.9% combined)
  - [x] 6.1 Move VDC::is_frame_complete(), VDC::is_hblank(), VDC::get_t1_state(), VDC::get_beam_x() to header
    - Also moved: is_vblank(), get_scanline(), get_beam_y(), get_frame_number(), clear_frame_complete()
    - Moved MasterClock::is_hblank() and MasterClock::get_beam_x() to header (required for cross-TU inlining)
    - Replaced forward declaration of MasterClock in vdc.h with #include "master_clock.h"
    - Result: 560M fewer cycles (-8.8% from task 5 baseline), IPC improved 3.45→3.85
    - _Requirements: 6.2_

- [x] 7. Checkpoint — Verify high-impact optimizations, re-profile
  - Build and run all tests
  - Re-run `perf stat` to measure cycle reduction from Tasks 4-6
  - Compare against baseline (7.91B cycles / 600 frames)
  - Re-run `perf record` + `perf report` to verify hot paths have shifted
  - Report FPS as secondary metric
  - Post-task-6 profile showed accessor wrappers eliminated from top;
    tick_one_cycle (20%), is_character_pixel_at (18%), render_current_pixel (13%),
    run_frame (12%), is_sprite_pixel_at (9%), is_grid_pixel_at (8%) now dominate.

- [ ] 8. Optimize VDC::render_current_pixel dispatch (perf: 12.2%)
  - [ ] 8.1 Reduce function call overhead in render_current_pixel
    - Currently calls is_grid_pixel_at (7%), is_character_pixel_at (35.5%), is_sprite_pixel_at
    - Consider inlining the grid check (simple coordinate math)
    - _Requirements: 2.1, 2.3_

- [x] 9. Optimize emulator execution loop (perf: 12% run_frame after inlining)
  - [x] 9.1 Skip debugger checks when no debugger attached in `src/emulator.cpp`
    - Added fast path in run_frame() that bypasses all debugger/profiling code
    - _Requirements: 5.2_
  - [x] 9.2 Eliminate profiling overhead when disabled in `run_frame()`
    - Fast path has zero chrono calls, zero debugger null-checks, zero profiling branches
    - Result: 160M fewer cycles (-2.7%), 387M fewer instructions, 132M fewer branches
    - _Requirements: 5.3, 10.3_

- [x] 10. Optimize VDC audio path (perf: 3.3% capture_audio_sample)
  - [x] 10.1 Add audio-disabled skip in `tick_one_cycle()`
    - When sound control enable bit is 0, skip both `update_audio()` and `capture_audio_sample()`
    - Safe: get_audio_buffer() returns silence when sample count is 0
    - Result: 241M fewer cycles (-4.2%), 886M fewer instructions
    - _Requirements: 4.1, 4.2_

- [x] 11. Optimize framebuffer conversion in libretro core (perf: not visible per-tick)
  - [x] 11.1 Add precomputed palette LUT to `src/libretro.cpp`
    - 16-entry XRGB8888 lookup table, built once, one table lookup per pixel
    - _Requirements: 7.1_
  - [x] 11.2 Mono-to-stereo conversion already branchless — no change needed
    - Existing loop is a simple copy with no per-sample branching
    - _Requirements: 7.2_

- [ ] 12. Checkpoint — Verify all optimizations preserve correctness
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 13. Enhance profiling mode with CSV output and summary statistics
  - [ ] 13.1 Add `profile_output_path` to `Configuration` in `include/emulator.h`
    - _Requirements: 10.1, 10.2_
  - [ ] 13.2 Implement CSV output and summary statistics in `src/emulator.cpp`
    - _Requirements: 10.1, 10.2_
  - [ ] 13.3 Wire `--profile-output` argument from benchmark harness to `Configuration`
    - _Requirements: 10.1_

- [ ] 14. Final checkpoint — Full regression verification
  - Re-run `perf stat` and compare against baseline (7.91B cycles / 600 frames)
  - Report FPS as secondary metric
  - Re-run `perf record` to verify hot paths have shifted
  - Verify existing test suite passes after all optimizations (Requirement 9.3)

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Property tests use RapidCheck (already integrated via CMake FetchContent)
- All optimizations must preserve bit-identical framebuffer output (Requirement 9.4)
- Checkpoints ensure incremental validation after each optimization group
- **Primary metric: `perf stat` CPU cycle counts** — load-independent, deterministic,
  reliable for A/B comparisons on shared machines. FPS is secondary (useful for
  user-facing reporting but sensitive to system load).
- **Linux perf is the authoritative profiler** — Windows gprof was misleading due to
  instrumentation overhead distorting relative percentages and unstable timing.
- Previous optimizations (sprite cache, inline update_counter, inline MasterClock accessors,
  collision-enable skip) were reverted after Linux perf confirmed zero measurable impact.
  GCC -O2 already inlines small functions; sprite/collision code is <1% of execution time.
