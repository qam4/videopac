# Implementation Plan: Cycle-Accurate Audio

## Overview

Implement cycle-accurate audio improvements across three layers: VDC audio engine (flush, DC offset, low-pass filter, 48kHz), SDL frontend (frame pacing, overflow skip, underrun handling), and libretro frontend (48kHz constants). All changes modify existing files; new test files are added to the existing CMake test target.

## Tasks

- [x] 1. Add VDCState fields and update reset/sample-rate defaults
  - [x] 1.1 Add new VDCState fields and VDC method declaration
    - Add `cycles_since_toggle` (uint32), `previous_output_bit` (uint8), `audio_filter_state` (int16) to `VDCState` in `include/vdc.h`
    - Declare `void flush_audio_cycles()` as a private method on the `VDC` class in `include/vdc.h`
    - _Requirements: 2.1, 3.1, 1.1_

  - [x] 1.2 Update VDC reset and sample rate default
    - In `VDC::reset()` in `src/vdc.cpp`, initialize `cycles_since_toggle = 0`, `previous_output_bit = 0`, `audio_filter_state = 0`
    - Change default `audio_sample_rate_` from 44100 to 48000 in `VDC::reset()`
    - _Requirements: 2.1, 3.4, 7.1, 7.5_

  - [x] 1.3 Update FrontendConfig sample rate default
    - In `include/frontend.h` (or `include/emulator.h`), change `FrontendConfig::sample_rate` default from 44100 to 48000
    - _Requirements: 7.1_

- [x] 2. Implement flush-before-write in VDC audio engine
  - [x] 2.1 Implement `flush_audio_cycles()` in `src/vdc.cpp`
    - Implement the method to generate audio samples for all pending VDC cycles at the current shift register state before a register write takes effect
    - If no cycles have elapsed (accumulator is 0), the function is a no-op
    - _Requirements: 1.1, 1.2, 1.3_

  - [x] 2.2 Call `flush_audio_cycles()` from `write_register()` for sound registers
    - In `VDC::write_register()` in `src/vdc.cpp`, call `flush_audio_cycles()` before processing writes to SOUND0 (0xA7), SOUND1 (0xA8), SOUND2 (0xA9), and SOUND_CONTROL (0xAA)
    - The new register value must be applied after the flush so samples use the old state
    - _Requirements: 1.1, 1.2_

  - [ ]* 2.3 Write property test: flush preserves old shift register state
    - **Property 1: Flush preserves old shift register state in samples**
    - Generate random shift register values, volumes, frequencies, and a write timing. Run VDC ticks, perform a register write, and verify all samples before the write match the old configuration.
    - Create `tests/property_tests_audio.cpp` and add it to `CMakeLists.txt` test sources
    - **Validates: Requirements 1.1, 1.2**

- [x] 3. Implement DC offset elimination
  - [x] 3.1 Add toggle tracking to `update_audio()` in `src/vdc.cpp`
    - After shifting the register, check if bit 0 changed compared to `previous_output_bit`
    - If changed: reset `cycles_since_toggle` to 0, update `previous_output_bit`
    - If unchanged: increment `cycles_since_toggle`
    - _Requirements: 2.1, 2.2_

  - [x] 3.2 Add DC offset gate to `get_audio_sample()` in `src/vdc.cpp`
    - If `cycles_since_toggle > 2000`, return 0 instead of the volume-scaled sample
    - When the output bit changes after silence, non-zero output resumes immediately (handled by 3.1 resetting the counter)
    - _Requirements: 2.3, 2.4_

  - [ ]* 3.3 Write property test: DC offset toggle counter tracks correctly
    - **Property 2: DC offset toggle counter tracks correctly**
    - Generate random 24-bit shift register patterns and tick counts. Run `update_audio()` for each tick and verify `cycles_since_toggle` matches the expected count since the last bit-0 change.
    - **Validates: Requirements 2.1, 2.2**

  - [ ]* 3.4 Write property test: DC offset gate suppresses static output
    - **Property 3: DC offset gate suppresses static output**
    - Generate random VDC states with varying `cycles_since_toggle` values (above and below 2000). Call `get_audio_sample()` and verify output is 0 when counter > 2000, and non-zero otherwise (when audio enabled with non-zero volume).
    - **Validates: Requirements 2.3, 2.4**

- [x] 4. Implement low-pass filter
  - [x] 4.1 Apply IIR low-pass filter in `capture_audio_sample()` in `src/vdc.cpp`
    - After computing the raw sample via `get_audio_sample()`, compute: `filtered = audio_filter_state + (raw - audio_filter_state) * 3 / 205`
    - Use int32 intermediates to avoid overflow
    - Store `filtered` as the new `audio_filter_state`
    - Write `filtered` (not `raw`) into the ring buffer
    - _Requirements: 3.1, 3.2, 3.3_

  - [ ]* 4.2 Write property test: low-pass filter computes correct IIR output
    - **Property 4: Low-pass filter computes correct IIR output**
    - Generate random sequences of int16 raw samples. Apply the IIR formula step by step and verify the filter output matches `previous + (raw - previous) * 3 / 205` at each step.
    - **Validates: Requirements 3.1, 3.2, 3.3**

- [x] 5. Checkpoint - Ensure VDC audio engine tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [x] 6. Implement SDL frontend audio improvements
  - [x] 6.1 Add SDL frontend fields and update `init_audio()` sample rate
    - Add `frame_start_counter_` (uint64_t), `perf_frequency_` (uint64_t), `last_audio_sample_` (int16) to `SDLFrontend` in `include/frontend_sdl.h`
    - Change `desired_spec.freq` to 48000 in `init_audio()` in `src/frontend_sdl.cpp`
    - Cache `SDL_GetPerformanceFrequency()` once at init
    - _Requirements: 7.2, 4.1, 6.2_

  - [x] 6.2 Implement precise frame pacing with performance counters
    - Replace `SDL_Delay()` with a spin-wait loop using `SDL_GetPerformanceCounter()` in the main loop in `src/frontend_sdl.cpp`
    - Target 16.67ms for NTSC (60 Hz), 20.0ms for PAL (50 Hz)
    - Skip pacing entirely when `turbo_mode_` is active
    - _Requirements: 4.1, 4.2, 4.3, 4.4_

  - [x] 6.3 Implement ring buffer overflow skip in `process_audio()`
    - After writing samples to the circular buffer, check queued sample count
    - If queued > 3 frames worth (3 × 800 NTSC, 3 × 960 PAL), advance `audio_read_pos_` until queued ≤ 1 frame worth
    - _Requirements: 5.1, 5.2, 5.3_

  - [x] 6.4 Implement underrun handling in `audio_callback()`
    - On successful read, store sample in `last_audio_sample_`
    - On underrun (buffer empty), output `last_audio_sample_` instead of 0
    - _Requirements: 6.1, 6.2, 6.3_

  - [ ]* 6.5 Write property test: overflow skip bounds queued audio
    - **Property 5: Overflow skip bounds queued audio**
    - Generate random circular buffer states with varying fill levels (including > 3 frames). Run the overflow skip logic and verify the resulting queued count is ≤ 1 frame worth.
    - **Validates: Requirements 5.2, 5.3**

  - [ ]* 6.6 Write property test: underrun holds last sample
    - **Property 6: Underrun holds last sample**
    - Generate random `last_audio_sample_` values and simulate an empty buffer callback. Verify the output equals the last sample, not zero.
    - **Validates: Requirements 6.1, 6.2, 6.3**

- [x] 7. Update libretro frontend constants
  - [x] 7.1 Update libretro sample rate and per-frame constants
    - In `src/libretro.cpp`, change `AUDIO_SAMPLES_PER_FRAME_NTSC` to 800 (48000/60) and `AUDIO_SAMPLES_PER_FRAME_PAL` to 960 (48000/50)
    - Change `info->timing.sample_rate` from 44100.0 to 48000.0 in `retro_get_system_av_info()`
    - _Requirements: 7.3, 7.4_

- [x] 8. Add unit tests for audio changes
  - [x] 8.1 Write unit tests in `tests/test_audio.cpp`
    - Create `tests/test_audio.cpp` and add it to `CMakeLists.txt` test sources
    - Test flush no-op: write a sound register with zero accumulated cycles, verify sample count unchanged (edge case from Req 1.3)
    - Test filter reset: call `reset()`, verify `audio_filter_state` is 0 (Req 3.4)
    - Test sample rate constants: verify VDC default is 48000, libretro constants are 800/960 (Req 7.1, 7.3, 7.4)
    - Test DC offset threshold boundary: test at exactly 2000 cycles (non-zero output) and 2001 cycles (zero output) (Req 2.3)
    - Test filter with zero input: verify filter converges toward zero from a non-zero state (Req 3.2)
    - Test back-to-back register writes: two writes in the same VDC cycle, verify only one flush and no duplicate samples (Req 1.3)
    - _Requirements: 1.3, 2.3, 3.2, 3.4, 7.1, 7.3, 7.4_

- [x] 9. Final checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- All property tests go in `tests/property_tests_audio.cpp`; unit tests go in `tests/test_audio.cpp`
- Both new test files must be added to the `videopac_tests` target in `CMakeLists.txt`
- SDL frontend code is guarded by `ENABLE_SDL`; property tests for SDL buffer logic (Properties 5, 6) can test the logic in isolation without SDL
- Build: `cmake --build --preset dev-mingw` / Test: `ctest --preset dev-mingw`
