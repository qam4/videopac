# Profiling the Videopac Emulator

This guide covers three profiling tools for identifying performance bottlenecks
in the videopac emulator. Each tool has different strengths and platform
requirements.

## Prerequisites

Build the benchmark harness first:

```bash
cmake --preset ci-mingw   # or your preferred preset
cmake --build build/ci-mingw --target videopac_benchmark
```

You will need a BIOS file and a ROM file for all profiling workflows.

---

## gprof (MinGW/GCC)

**Platform:** Windows (MinGW) or Linux (GCC). Requires GCC toolchain.

gprof uses compile-time instrumentation to record function call counts and
time spent in each function. It adds overhead but gives a complete call graph.

### Workflow

1. Build with the `profile-mingw` preset (adds `-pg` to compiler and linker flags):

   ```bash
   cmake --preset profile-mingw
   cmake --build build/profile-mingw --target videopac_benchmark
   ```

2. Run the benchmark binary. This generates a `gmon.out` file in the current
   directory:

   ```bash
   ./build/profile-mingw/videopac_benchmark.exe --bios <bios> <rom> --frames 6000
   ```

3. Generate the profile report:

   ```bash
   gprof videopac_benchmark.exe gmon.out > profile.txt
   ```

4. Open `profile.txt` to see the flat profile (time per function) and call
   graph (caller/callee relationships).

### Tips

- Use `--frames 6000` or more for stable measurements (short runs have noisy profiles).
- The flat profile shows the "self" time — time spent in the function itself,
  excluding callees. Look for functions with high self-time percentage.
- The call graph section shows which functions call which, useful for
  understanding the hot path hierarchy.

---

## Valgrind / Callgrind (Linux)

**Platform:** Linux only. Install with `sudo apt install valgrind` (Debian/Ubuntu)
or your distribution's package manager.

Callgrind is a Valgrind tool that simulates the CPU cache and branch predictor
to produce instruction-level profiles. It runs ~20-50x slower than native but
gives very accurate call counts and instruction costs.

### Workflow

1. Build with a standard debug or release-with-debug-info configuration:

   ```bash
   cmake --preset ci-linux
   cmake --build build/ci-linux --target videopac_benchmark
   ```

2. Run under Callgrind (use fewer frames since Callgrind is slow):

   ```bash
   valgrind --tool=callgrind ./videopac_benchmark --bios <bios> <rom> --frames 600
   ```

   This produces a `callgrind.out.<pid>` file in the current directory.

3. View the results with the command-line annotator:

   ```bash
   callgrind_annotate callgrind.out.<pid>
   ```

   Or use KCachegrind for a visual call graph:

   ```bash
   kcachegrind callgrind.out.<pid>
   ```

### Tips

- Use `--frames 600` (10 seconds of emulation) — Callgrind is slow, so fewer
  frames keeps the profiling run manageable.
- KCachegrind provides a treemap and call graph visualization that makes it
  easy to spot hot paths. Install with `sudo apt install kcachegrind`.
- Callgrind counts instructions, not wall-clock time. This makes results
  reproducible across runs (no noise from system load).

---

## perf (Linux)

**Platform:** Linux only. Install with `sudo apt install linux-tools-common
linux-tools-$(uname -r)` (Debian/Ubuntu).

perf is a sampling-based profiler that uses hardware performance counters. It
has very low overhead and works on optimized release builds without
recompilation.

### Workflow

1. Build with a release or release-with-debug-info configuration:

   ```bash
   cmake --preset ci-linux
   cmake --build build/ci-linux --target videopac_benchmark
   ```

2. Record a profile (use more frames for better sampling accuracy):

   ```bash
   perf record ./videopac_benchmark --bios <bios> <rom> --frames 6000
   ```

   This produces a `perf.data` file in the current directory.

3. View the interactive report:

   ```bash
   perf report
   ```

   Navigate with arrow keys. Press Enter to drill into a function and see
   annotated assembly.

### Tips

- Use `--frames 6000` or more for statistically significant samples.
- Build with `-g` (debug info) to get source-level annotation in `perf report`.
  The `RelWithDebInfo` build type includes this by default.
- `perf stat ./videopac_benchmark --bios <bios> <rom> --frames 6000` gives a
  quick summary of IPC, cache misses, and branch mispredictions without
  recording a full profile.
- If you get permission errors, try `sudo sysctl kernel.perf_event_paranoid=-1`
  or run with `sudo`.

---

## Tool Comparison

| Tool              | Platform       | Overhead   | Accuracy        | Best For                        |
|-------------------|----------------|------------|-----------------|---------------------------------|
| gprof             | MinGW/GCC      | Moderate   | Function-level  | Quick call graph overview       |
| Valgrind/Callgrind| Linux          | High (20-50x) | Instruction-level | Precise instruction counts   |
| perf              | Linux          | Very low   | Statistical     | Production-like sampling        |

## Recommended Workflow

1. **Profile on Linux** — Windows/MinGW benchmarks are unreliable due to
   background system load and gprof limitations. Use Linux for all profiling
   and benchmarking work.
2. Start with **perf** to identify the top hot functions.
3. Use **Callgrind** to drill into specific hot functions and understand
   instruction-level costs.
4. After applying optimizations, re-run the benchmark with `--check
   --baseline-fps <previous>` to verify no regressions.
