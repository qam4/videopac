# Implementation Plan

- [ ] 1. Write bug condition exploration test
  - **Property 1: Bug Condition** - Mid-Scanline Graphic Register Write Causes Tearing
  - **CRITICAL**: This test MUST FAIL on unfixed code - failure confirms the bug exists
  - **DO NOT attempt to fix the test or the code when it fails**
  - **NOTE**: This test encodes the expected behavior - it will validate the fix when it passes after implementation
  - **GOAL**: Surface counterexamples that demonstrate mid-scanline writes immediately affect rendering
  - **Scoped PBT Approach**: Scope the property to concrete failing cases:
    - Place a character (e.g. char 0 at base_addr 0x10) at known position Y=50, X=40
    - Advance VDC to scanline 50, tick to mid-visible area (scanline_tick between 0 and HBLANK_START_TICK)
    - Write new X=80 to `state_.registers[0x11]` via `write_register(0x11, 80)`
    - Continue rendering remaining pixels on that scanline
    - Assert: ALL pixels on scanline 50 use the OLD X position (X=40), not the new value
  - For all graphic register addresses in 0x00-0x9F and any mid-scanline tick position, writing a new value must not change the current scanline's rendering output
  - The test assertions match Expected Behavior Properties from design (Property 1): rendering uses pre-write register values for the remainder of the scanline
  - Use rapidcheck to generate random register addresses (0x00-0x9F), random mid-scanline tick positions, and random register values
  - Run test on UNFIXED code
  - **EXPECTED OUTCOME**: Test FAILS (pixels after the write use the NEW value, proving the bug exists)
  - Document counterexamples found (e.g., "write_register(0x11, 80) at scanline_tick=200 causes character to render at X=80 instead of X=40 for remaining pixels")
  - Mark task complete when test is written, run, and failure is documented
  - _Requirements: 1.1, 1.2, 1.5, 2.1, 2.2, 2.5_

- [ ] 2. Write preservation property tests (BEFORE implementing fix)
  - **Property 2: Preservation** - Rendering Without Mid-Scanline Writes Is Unchanged
  - **IMPORTANT**: Follow observation-first methodology
  - Observe on UNFIXED code:
    - Set up VDC with random character/sprite configurations written during VBLANK
    - Render a full scanline with no mid-scanline writes
    - Record the framebuffer output for that scanline
  - Observe: characters and sprites at various positions render correctly when registers are only written during VBLANK/HBLANK
  - Observe: non-graphic register writes (0xA0-0xFF) take effect immediately and produce expected behavior
  - Write property-based tests using rapidcheck:
    - **Preservation A (VBLANK writes)**: For all random graphic register values (0x00-0x9F) written during VBLANK, the rendered scanline output matches the observed baseline from unfixed code
    - **Preservation B (HBLANK writes)**: For all random graphic register values written during HBLANK, the next visible scanline uses the written values (same as unfixed code)
    - **Preservation C (No-write rendering)**: For all random VDC states with no mid-scanline writes, rendered output is pixel-for-pixel identical to unfixed code output
  - Verify all preservation tests PASS on UNFIXED code
  - **EXPECTED OUTCOME**: Tests PASS (confirms baseline behavior to preserve)
  - Mark task complete when tests are written, run, and passing on unfixed code
  - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6_

- [ ] 3. Implement scanline register latching fix

  - [ ] 3.1 Add latched register array to VDCState and latch method
    - In `include/vdc.h`, add `uint8 latched_registers[160]` to `VDCState` (covers 0x00-0x9F)
    - In `include/vdc.h`, declare `void latch_graphic_registers()` as private method on `VDC`
    - In `src/vdc.cpp`, implement `latch_graphic_registers()` as `memcpy(state_.latched_registers, state_.registers, 160)` — snapshots registers 0x00-0x9F
    - _Bug_Condition: isBugCondition(input) where input.operation == WRITE AND input.address <= 0x9F AND scanline < VBLANK_START AND 0 < scanline_tick < HBLANK_START_TICK_
    - _Expected_Behavior: Rendering pipeline reads from latched_registers instead of live registers, so mid-scanline writes don't affect current scanline output_
    - _Preservation: write_register() continues writing to state_.registers[] — no change to write path. Latch captures VBLANK/HBLANK writes before next visible scanline._
    - _Requirements: 2.1, 2.2, 2.5, 3.1_

  - [ ] 3.2 Call latch on scanline transition in tick_one_cycle()
    - In `src/vdc.cpp` `tick_one_cycle()`, in the scanline transition block where `new_y != state_.prev_scanline`, call `latch_graphic_registers()`
    - This ensures the snapshot is taken at the start of each new scanline before any pixels are rendered
    - Also call `latch_graphic_registers()` during `reset()` to initialize the latched array
    - _Bug_Condition: Without latching, render_current_pixel reads live registers that change mid-scanline_
    - _Expected_Behavior: Latch fires once per scanline transition, freezing graphic register values for the rendering pipeline_
    - _Preservation: Writes during HBLANK/VBLANK are captured by the latch at the start of the next visible scanline (Req 3.1)_
    - _Requirements: 2.1, 2.2, 2.5, 3.1, 3.6_

  - [ ] 3.3 Switch is_character_pixel_at() to read from latched registers
    - In `src/vdc.cpp` `is_character_pixel_at()`, replace all reads of `state_.registers[base_addr + N]` with `state_.latched_registers[base_addr + N]` for character position (bytes 0-1), pattern pointer (byte 2), and color/attr (byte 3)
    - Applies to both single characters (base_addr 0x10-0x3F) and quad characters (base_addr 0x40-0x7F)
    - _Bug_Condition: Currently reads live state_.registers[] which changes mid-scanline on CPU write_
    - _Expected_Behavior: Reads latched values frozen at scanline start — mid-scanline writes don't affect current scanline_
    - _Requirements: 2.1, 2.5_

  - [ ] 3.4 Switch is_sprite_pixel_at() to read from latched registers
    - In `src/vdc.cpp` `is_sprite_pixel_at()`, replace all reads of `state_.registers[base_addr + N]` with `state_.latched_registers[base_addr + N]` for sprite position (bytes 0-1), color/attr (byte 2), and pattern data (0x80-0x9F)
    - Applies to all 4 sprites (base_addr 0x00-0x0F)
    - _Bug_Condition: Currently reads live state_.registers[] which changes mid-scanline on CPU write_
    - _Expected_Behavior: Reads latched values frozen at scanline start — mid-scanline writes don't affect current scanline_
    - _Requirements: 2.2_

  - [ ] 3.5 Switch detect_collision_at_pixel() to read from latched registers
    - In `src/vdc.cpp` `detect_collision_at_pixel()`, ensure any direct sprite register reads use `state_.latched_registers[]`
    - The calls to `is_character_pixel_at()` and `is_sprite_pixel_at()` are already covered by 3.3/3.4, but verify any additional direct register reads in collision detection also use latched values
    - _Bug_Condition: Collision detection must see same object positions as rendering for consistency_
    - _Expected_Behavior: Collision detection uses latched register values, consistent with rendering pipeline_
    - _Requirements: 2.1, 2.2_

  - [ ] 3.6 Verify and fix get_beam_x_at_cpu_read() determinism
    - In `src/master_clock.cpp` `get_beam_x_at_cpu_read()`, verify the `bus_read_offset = cpu_tick_divisor_` correctly models the MOVX RD strobe timing
    - The current offset adds a full CPU tick divisor (20 NTSC / 45 PAL master ticks) which may produce ±1 X jitter depending on when within the CPU tick window the read executes
    - If jitter is confirmed, adjust the offset to use the exact tick at which the RD strobe fires relative to the CPU tick slot start
    - Test: for the same `scanline_tick_` value, `get_beam_x_at_cpu_read()` must always return the same X value (deterministic function of master clock state)
    - _Bug_Condition: Non-deterministic beam X causes variable loop exit timing in Satellite Attack's wait_until_scanline loop (0x71a-0x723)_
    - _Expected_Behavior: Beam X is a deterministic function of scanline_tick_ at the CPU read strobe point_
    - _Requirements: 2.3, 2.4_

  - [ ] 3.7 Verify bug condition exploration test now passes
    - **Property 1: Expected Behavior** - Mid-Scanline Writes Do Not Affect Current Scanline
    - **IMPORTANT**: Re-run the SAME test from task 1 - do NOT write a new test
    - The test from task 1 encodes the expected behavior (all pixels use pre-write register values)
    - When this test passes, it confirms the latching mechanism correctly isolates mid-scanline writes
    - Run bug condition exploration test from step 1
    - **EXPECTED OUTCOME**: Test PASSES (confirms bug is fixed)
    - _Requirements: 2.1, 2.2, 2.5_

  - [ ] 3.8 Verify preservation tests still pass
    - **Property 2: Preservation** - Rendering Without Mid-Scanline Writes Is Unchanged
    - **IMPORTANT**: Re-run the SAME tests from task 2 - do NOT write new tests
    - Run all preservation property tests from step 2
    - **EXPECTED OUTCOME**: Tests PASS (confirms no regressions)
    - Confirm VBLANK writes still visible on next scanline, HBLANK writes captured by latch, no-write rendering identical, non-graphic registers unaffected
    - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6_

- [ ] 4. Checkpoint - Ensure all tests pass
  - Run full test suite: property tests, preservation tests, integration tests
  - Verify no regressions in existing MasterClock property tests
  - Ensure all tests pass, ask the user if questions arise
