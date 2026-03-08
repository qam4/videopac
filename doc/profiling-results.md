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

---

## Linux Profiling (perf) — Post update_counter Inline

### Test Configuration

- ROM: Satellite Attack (1981)(Philips)(EU).bin
- BIOS: Philips C52 BIOS (19xx)(Philips)(FR).bin
- Frames: 3000 (warmup: 60)
- Build: RelWithDebInfo (-O2 -g), GCC 7.3.1
- Platform: Amazon Linux 2 (x86_64)
- Profiler: perf record -g (37,254 samples)
- Date: 2026-03-08

### Linux Baseline Performance (Release -O2, no instrumentation)

Stable across 3 runs (unlike Windows):

| ROM | Frames | Run 1 FPS | Run 2 FPS | Run 3 FPS | Mean FPS |
|-----|--------|-----------|-----------|-----------|----------|
| Satellite Attack | 3000 | 414.8 | 421.1 | 421.0 | **419.0** |

### perf Flat Profile (Satellite Attack, 3000 frames)

| Function | Self % | Notes |
|----------|--------|-------|
| `VDC::is_character_pixel_at()` | 35.54% | **#1 hot path — per-pixel character lookup** |
| `MasterClock::tick()` | 21.28% | Per-tick scheduling (modulo ops) |
| `VDC::render_current_pixel()` | 12.23% | Per-pixel render orchestration |
| `VDC::is_grid_pixel_at()` | 6.97% | Per-pixel grid lookup |
| `VDC::tick_one_cycle()` | 6.91% | VDC main loop |
| `EmulatorCore::run_frame()` | 3.67% | Frame orchestration + debugger checks |
| `VDC::get_t1_state()` | 2.87% | VDC wrapper (not inlined) |
| `VDC::is_frame_complete()` | 2.84% | VDC wrapper (not inlined) |
| `VDC::is_hblank()` | 2.18% | VDC wrapper (not inlined) |
| `VDC::capture_audio_sample()` | 1.70% | Audio sampling |
| `EmulatorCore::handle_interrupts()` | 1.26% | Interrupt processing |
| `CPU::execute_instruction()` | 0.64% | Instruction dispatch |

### Key Findings (Linux perf vs Windows gprof)

#### 1. CPU::update_counter() eliminated from profile

Task 5 (inline to header) was effective. It was 48% on Windows gprof, now 0% on
Linux perf. The function call overhead was the entire cost — the actual logic
(edge detection + conditional increment) is negligible when inlined.

#### 2. VDC character pixel lookup is the new #1 hot path (35.5%)

`is_character_pixel_at()` was invisible in gprof (hidden inside `render_current_pixel`
call tree). perf's sampling profiler correctly attributes self-time to this function.
This is the inner loop that decodes character ROM data for every visible pixel.

#### 3. MasterClock::tick() confirmed at ~21%

Consistent with gprof's normalized ~20%. The modulo operations
(`master_tick_count_ % vdc_tick_divisor_` and `% cpu_tick_divisor_`) are the
likely bottleneck. Replacing with counter-based tracking would help.

#### 4. VDC rendering dominates at ~62% combined

| Category | Functions | Combined % |
|----------|-----------|-----------|
| VDC pixel rendering | is_character_pixel_at, render_current_pixel, is_grid_pixel_at | **54.7%** |
| VDC tick + audio | tick_one_cycle, capture_audio_sample | **8.6%** |
| VDC accessor wrappers | get_t1_state, is_frame_complete, is_hblank | **7.9%** |
| **VDC total** | | **71.2%** |

This matches the benchmark's own subsystem breakdown (VDC: 71%).

#### 5. VDC accessor wrappers still ~8% (not inlined from VDC side)

`MasterClock::is_hblank()` and `get_beam_x()` are already inline in the header,
but the VDC wrapper methods (`VDC::is_hblank()`, `VDC::is_frame_complete()`,
`VDC::get_t1_state()`) are still in `src/vdc.cpp`. These delegate to master_clock_
but the compiler can't inline across TUs. Task 6.2 addresses this.

### Revised Optimization Priority (Linux perf data)

1. **VDC::is_character_pixel_at()** — 35.5%, optimize character ROM lookup
2. **MasterClock::tick()** — 21.3%, eliminate modulo ops (Task 6.3)
3. **VDC::render_current_pixel()** — 12.2%, reduce dispatch overhead
4. **VDC accessor wrappers** — 7.9%, move to header (Task 6.2)
5. **VDC::is_grid_pixel_at()** — 7.0%, optimize grid lookup
6. **VDC::tick_one_cycle()** — 6.9%, collision skip + audio skip (Tasks 9, 10)
7. **EmulatorCore::run_frame()** — 3.7%, debugger skip (Task 8)
8. **VDC::capture_audio_sample()** — 1.7%, audio-disabled skip (Task 10)


---

## Optimization Results — MasterClock Countdown Counters

### Change
Replaced `master_tick_count_ % vdc_tick_divisor_` and `% cpu_tick_divisor_` modulo
operations in `MasterClock::tick()` with decrementing countdown counters. The modulo
expands to expensive `div` instructions on x86; countdown is `dec` + `jnz`.

### Measurement (perf stat, 600 frames, 3 runs each)

| Metric | Baseline (modulo) | Countdown | Change |
|--------|-------------------|-----------|--------|
| Cycles | 7.91B ± 0.01B | 7.43B ± 0.02B | **-6.1%** |
| Instructions | 32.25B | 31.89B | **-1.1%** |
| IPC | 4.08 | 4.30 | +5.4% |

Instruction count is deterministic across runs (confirms emulation correctness).


## Optimization Results — Character Pixel Early Reject

### Change
In `is_character_pixel_at()`, added quick Y-range reject using max character height
(16 scanlines) before computing exact height. For single characters: check `y < char_y`
and `y >= char_y + 16` before reading ptr/attr registers and computing height. For quad
characters: check quad Y once and skip all 4 sub-characters if out of range. Also
reordered X check before height calculation.

### Cumulative Measurement (perf stat, 600 frames, 3 runs)

| Metric | Baseline | + Countdown | + Char Early Reject | Total Change |
|--------|----------|-------------|---------------------|-------------|
| Cycles | 7.91B | 7.43B (-6%) | 6.38B | **-19.4%** |
| Instructions | 32.25B | 31.89B (-1%) | 21.98B | **-31.9%** |
| IPC | 4.08 | 4.30 | 3.45 | -15.4% |

IPC dropped because the branch predictor has more work with the early-exit branches,
but the massive instruction reduction (10B fewer) more than compensates.
