# Profiling Results — Baseline

## Test Configuration

- ROM: Satellite Attack (1981)(Philips)(EU).bin
- BIOS: bios_O2rom.bin
- Frames: 3000 (warmup: 60)
- Build: profile-mingw preset (GCC, -pg -no-pie -static, RelWithDebInfo)
- Platform: Windows/MinGW-w64
- Date: 2026-03-07

## Baseline Performance (ci-mingw, no instrumentation)

**WARNING**: Windows benchmarks are unreliable due to background system load.
Repeated runs of the same binary gave wildly different results (156.8, 74.2,
30.4 FPS). All optimization benchmarking should be done on Linux for stable
measurements.

| ROM | Frames | FPS (best of 3) | Notes |
|-----|--------|-----------------|-------|
| Satellite Attack | 3000 | 156.8 | Highly variable on Windows |
| Killer Bees | 6000 | 70.5 | Highly variable on Windows |

## gprof Flat Profile (Satellite Attack, 3000 frames)

gprof instrumentation overhead: `_mcount_private` (38.1%) and `__fentry__` (12.6%)
account for ~50.7% of measured time. The percentages below are raw gprof numbers;
the "Normalized %" column excludes instrumentation overhead to estimate real-world
contribution.

| Function | Self % (raw) | Normalized % | Calls | Notes |
|----------|-------------|--------------|-------|-------|
| `CPU::update_counter(bool)` | 23.9% | ~48% | 418M | **#1 hot path — unexpected** |
| `MasterClock::tick()` | 9.9% | ~20% | 366M | Per-tick scheduling |
| `VDC::render_current_pixel()` | 3.7% | ~7% | 183M | Per-pixel rendering |
| `VDC::tick_one_cycle()` | 3.4% | ~7% | 183M | VDC main loop |
| `EmulatorCore::run_frame()` | 2.0% | ~4% | 3K | Frame orchestration |
| `MasterClock::is_hblank()` | 1.3% | ~3% | 366M | Called every master tick |
| `VDC::is_frame_complete()` | 1.2% | ~2% | 366M | Called every master tick |
| `VDC::capture_audio_sample()` | 0.9% | ~2% | 183M | Audio sampling |
| `VDC::get_t1_state()` | 0.7% | ~1.5% | 183M | T1 pin state |
| `MasterClock::get_beam_x()` | 0.7% | ~1.4% | 183M | Beam position query |
| `CPU::execute_instruction()` | 0.6% | ~1.2% | 9.2M | Instruction dispatch |
| `MasterClock::vdc_executed()` | 0.6% | ~1.1% | 183M | Post-VDC tick bookkeeping |

## Key Findings

### 1. CPU::update_counter() is the dominant hot path (~48%)

The spec assumed VDC rendering would dominate. In reality, `CPU::update_counter()`
is called 418M times (2.3x per VDC cycle) and consumes nearly half of all
execution time. This function was not targeted by any optimization task in the
original spec.

### 2. MasterClock::tick() is the second hot path (~20%)

Called 366M times. The per-tick scheduling overhead is significant. The spec's
Task 9 (master clock optimization) targets this correctly but underestimated
its importance.

### 3. VDC rendering is less dominant than expected (~14% combined)

`render_current_pixel()` + `tick_one_cycle()` together account for ~14% of
execution time, not the ~71% assumed in the spec. The VDC optimizations
(Tasks 4-6) are still worthwhile but are not the highest-impact targets.

### 4. Small accessor functions add up (~8% combined)

`is_hblank()`, `is_frame_complete()`, `get_t1_state()`, `get_beam_x()`,
`vdc_executed()` are each called 183-366M times. Individually small, but
collectively ~8%. Inlining these would help.

### 5. Audio capture is measurable (~2%)

`capture_audio_sample()` at 0.9% raw is worth optimizing with the audio-disabled
skip (Task 6.1).

## Impact on Optimization Plan

The original spec's optimization priority was:
1. VDC render pipeline (Tasks 4) — estimated highest impact
2. Collision detection (Task 5)
3. Audio path (Task 6)
4. CPU dispatch (Task 8)
5. Master clock (Task 9)
6. Framebuffer conversion (Task 10)

Based on profiling, the revised priority should be:
1. **CPU::update_counter()** — ~48%, NOT in original spec, needs new task
2. **MasterClock::tick() + accessors** — ~28%, inline accessors + optimize tick
3. **VDC tick_one_cycle + render_current_pixel** — ~14%, existing Tasks 4-6
4. **EmulatorCore::run_frame()** — ~4%, existing Task 8
5. **Framebuffer conversion** — not visible in profile (per-frame, not per-tick)

## Recommendations

- Add a new optimization task targeting `CPU::update_counter()`
- Elevate Task 9 (master clock) priority
- Consider inlining `is_hblank()`, `is_frame_complete()`, `get_t1_state()`,
  `get_beam_x()`, `vdc_executed()` into headers
- VDC Tasks 4-6 are still valid but lower priority than originally estimated
