# Implementation Plan: Libretro Enhancements

## Overview

Incremental implementation of the virtual keyboard overlay, joystick port swap, and info file updates for the videopac libretro core. Tasks build bottom-up: data structures and font first, then VKB logic, then libretro integration, then info file updates. Property-based tests validate each component as it's built.

## Tasks

- [x] 1. Create bitmap font data and VKB header with key layout table
  - [x] 1.1 Create `include/vkeyboard_font.h` with 5×7 bitmap font
    - Define `FONT_CHAR_WIDTH`, `FONT_CHAR_HEIGHT` constants
    - Define `FONT_DATA[96][7]` constexpr array covering printable ASCII 32–127
    - Each row is 5 bits MSB-aligned in a `uint8_t`
    - _Requirements: 4.1, 4.4_

  - [x] 1.2 Create `include/vkeyboard.h` with `VKBKey` struct, `VirtualKeyboard` class declaration, and the 49-key `LAYOUT` table
    - Define `VKBKey` struct with label, x, y, width, height, vidkey, nav links
    - Declare `VirtualKeyboard` class with all public/private members per design
    - Include `Direction` enum from `input.h`
    - _Requirements: 8.1, 8.2, 8.3, 8.4_

  - [x] 1.3 Create `src/vkeyboard.cpp` with the `LAYOUT` constant array (49 keys, 5 rows) and constructor
    - Hand-author navigation links for all 49 keys matching physical O2 layout
    - Standard keys ~14×13px, wide keys (SPC, ENT) ~28×13px, total VKB area 160×80
    - Initialize `visible_=false`, `position_bottom_=true`, `cursor_index_=0`
    - _Requirements: 8.1, 8.2, 8.3, 1.4, 6.2_

  - [ ]* 1.4 Write property test: Layout table structural invariants (Property 8)
    - **Property 8: Layout table structural invariants**
    - Verify all 49 keys: label non-null, width>0, height>0, x+width ≤ VKB_WIDTH, y+height ≤ VKB_HEIGHT, VidKey row<6 and col<8, nav links are -1 or valid index in [0,48]
    - Create `tests/property_tests_vkeyboard.cpp`, add to `CMakeLists.txt`
    - **Validates: Requirements 8.2**

- [x] 2. Implement VKB state management (toggle, navigation, key press)
  - [x] 2.1 Implement `toggle_visible()`, `is_visible()`, `toggle_position()`, `get_cursor_index()`, `get_current_vidkey()` in `src/vkeyboard.cpp`
    - `toggle_visible()` flips `visible_`; `toggle_position()` flips `position_bottom_`
    - `get_current_vidkey()` returns `LAYOUT[cursor_index_].vidkey`
    - _Requirements: 1.1, 1.4, 6.1, 6.2, 3.1_

  - [x] 2.2 Implement `move_cursor(Direction dir)` in `src/vkeyboard.cpp`
    - Read nav link from `LAYOUT[cursor_index_]` for given direction
    - If link ≥ 0, update `cursor_index_`; if -1, stay at current index
    - Clamp `cursor_index_` to [0, KEY_COUNT-1] defensively
    - _Requirements: 2.1, 2.4_

  - [x] 2.3 Implement `hit_test(int vkb_local_x, int vkb_local_y)` and `get_vidkey_at(int index)` in `src/vkeyboard.cpp`
    - Iterate LAYOUT, return first key index where point is inside bounding rect
    - Return -1 if no key hit
    - `get_vidkey_at` returns `LAYOUT[index].vidkey` with bounds check
    - _Requirements: 7.1, 7.2, 8.4_

  - [ ]* 2.4 Write property test: Toggle round-trip (Property 1)
    - **Property 1: Toggle round-trip**
    - Generate random N (1..100), verify even N restores original state, odd N flips it. Test both `toggle_visible()` and `toggle_position()`.
    - **Validates: Requirements 1.1, 6.1**

  - [ ]* 2.5 Write property test: Cursor navigation follows layout links (Property 2)
    - **Property 2: Cursor navigation follows layout links**
    - Generate random key index + direction, set cursor, call `move_cursor`, verify result matches LAYOUT nav link or stays if -1.
    - **Validates: Requirements 2.1, 2.4**

  - [ ]* 2.6 Write property test: Hit test consistency with layout bounds (Property 9)
    - **Property 9: Hit test consistency with layout bounds**
    - Generate random key index + random point strictly inside that key's rect, verify `hit_test` returns that index. Generate point outside all keys, verify returns -1.
    - **Validates: Requirements 7.1, 8.4**

- [x] 3. Implement VKB rendering (rectangles, font, alpha blending)
  - [x] 3.1 Implement `get_vkb_y_offset()`, `draw_rect()`, `draw_char()`, `draw_label()` private helpers in `src/vkeyboard.cpp`
    - `get_vkb_y_offset`: returns 0 for top, `fb_height - VKB_HEIGHT` for bottom
    - `draw_rect`: fill rectangle with XRGB8888 color, alpha-blended onto framebuffer
    - `draw_char`: render single character from `FONT_DATA` with alpha blending
    - `draw_label`: render multi-char string using `draw_char`
    - Alpha blend formula: `result_ch = (overlay_ch * alpha + bg_ch * (255 - alpha)) / 255`
    - _Requirements: 4.1, 4.2, 4.3, 4.4_

  - [x] 3.2 Implement `render(uint32_t* framebuffer, int fb_width, int fb_height, uint8_t transparency_pct)` in `src/vkeyboard.cpp`
    - Compute `alpha = 255 - (transparency_pct * 255 / 100)`
    - Draw semi-transparent background panel
    - Iterate LAYOUT: draw key rect, highlight cursor key differently, draw label text
    - Offset all y-coordinates by `get_vkb_y_offset(fb_height)`
    - _Requirements: 1.2, 2.3, 4.1, 4.2, 4.3, 4.4, 5.3, 5.4_

  - [ ]* 3.3 Write property test: Alpha blending formula (Property 5)
    - **Property 5: Alpha blending formula**
    - Generate random bg/fg XRGB8888 pixels + transparency in {0,25,50,75}, verify each channel matches `(overlay_ch * alpha + bg_ch * (255 - alpha)) / 255`.
    - **Validates: Requirements 4.3, 5.3, 5.4**

- [x] 4. Checkpoint — VKB module builds and passes tests
  - Ensure `vkeyboard.cpp` compiles into `videopac_libretro` target and all property/unit tests pass. Ask the user if questions arise.

- [x] 5. Update `CMakeLists.txt` for new VKB source and test files
  - [x] 5.1 Add `src/vkeyboard.cpp` to the `videopac_libretro` shared library sources in `CMakeLists.txt`
    - Also add to `videopac_core` static library so tests can link against it
    - _Requirements: 4.1_

  - [x] 5.2 Add `tests/test_vkeyboard.cpp` and `tests/property_tests_vkeyboard.cpp` to the `videopac_tests` executable sources in `CMakeLists.txt`
    - _Requirements: 8.1_

- [x] 6. Integrate VKB into `src/libretro.cpp` — input routing and rendering
  - [x] 6.1 Add VKB state variables and include `vkeyboard.h` in `src/libretro.cpp`
    - Add `static VirtualKeyboard vkb;`, `static uint8_t vkb_transparency_pct = 25;`, `static bool prev_select_pressed = false;`, `static bool prev_y_pressed = false;`
    - _Requirements: 1.4, 5.1, 6.2_

  - [x] 6.2 Modify `update_input()` in `src/libretro.cpp` to handle VKB toggle, navigation, key press, and input suppression
    - SELECT rising-edge toggles VKB visibility
    - When VKB visible: D-pad calls `vkb.move_cursor()`, B sets/clears VidKey via `vkb.get_current_vidkey()`, Y rising-edge calls `vkb.toggle_position()`
    - When VKB visible: suppress D-pad from joystick1, suppress B from keyboard_matrix[0][0], suppress Y from keyboard_matrix[0][1]
    - When VKB hidden: pass inputs through as before
    - _Requirements: 1.1, 1.3, 2.1, 2.2, 3.1, 3.2, 3.3, 6.1, 6.3_

  - [x] 6.3 Add touch input handling via `RETRO_DEVICE_POINTER` in `update_input()`
    - Add `RETRO_DEVICE_POINTER` and ID constants to `include/libretro.h` if missing
    - When VKB visible: read pointer X/Y/PRESSED, translate from [-0x7FFF, 0x7FFF] to pixel coords, call `vkb.hit_test()`, set/clear VidKey
    - Clamp pointer values to valid framebuffer range
    - _Requirements: 7.1, 7.2, 7.3, 7.4_

  - [x] 6.4 Modify `retro_run()` in `src/libretro.cpp` to call `vkb.render()` after `convert_framebuffer()` when VKB is visible
    - Call `vkb.render(video_buffer, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, vkb_transparency_pct)` before `video_cb()`
    - _Requirements: 1.2, 4.2, 4.3_

  - [ ]* 6.5 Write property test: VKB input suppression (Property 3)
    - **Property 3: VKB input suppression**
    - Generate random D-pad/B/Y bitmask, simulate `update_input()` with VKB visible, verify joystick1 D-pad entries are false, keyboard_matrix[0][0] not set by B, keyboard_matrix[0][1] not set by Y.
    - **Validates: Requirements 2.2, 3.3, 6.3**

  - [ ]* 6.6 Write property test: VKB key activation sets correct VidKey (Property 4)
    - **Property 4: VKB key activation sets correct VidKey**
    - Generate random key index, activate via cursor+B, verify `keyboard_matrix[row][col]` matches LAYOUT vidkey. Deactivate and verify cleared.
    - **Validates: Requirements 3.1, 3.2, 7.1, 7.2**

  - [ ]* 6.7 Write property test: Pointer coordinate translation (Property 7)
    - **Property 7: Pointer coordinate translation**
    - Generate random (x,y) in [-0x7FFF, 0x7FFF], verify translated pixel coords are in [0, FRAMEBUFFER_WIDTH) × [0, FRAMEBUFFER_HEIGHT) and mapping is monotonically increasing.
    - **Validates: Requirements 7.4**

- [x] 7. Implement joystick port swap in `src/libretro.cpp`
  - [x] 7.1 Add `static bool swap_joysticks = false;` and update `check_variables()` to read `videopac_swap_joysticks` option
    - Default to `false` if value is unknown
    - _Requirements: 9.1, 9.4_

  - [x] 7.2 Modify joystick routing in `update_input()` to swap port 0↔1 when `swap_joysticks` is true
    - When enabled: port 0 joypad → `joystick2[]`, port 1 joypad → `joystick1[]`
    - When disabled: port 0 → `joystick1[]`, port 1 → `joystick2[]` (current behavior)
    - _Requirements: 9.2, 9.3_

  - [ ]* 7.3 Write property test: Joystick swap routing (Property 6)
    - **Property 6: Joystick swap routing**
    - Generate random 5-bool arrays for port 0 and port 1 + swap flag, verify `joystick1 == port(swap ? 1 : 0)` and `joystick2 == port(swap ? 0 : 1)`.
    - **Validates: Requirements 9.2, 9.3**

- [x] 8. Update core options, input descriptors, and version string
  - [x] 8.1 Add `videopac_vkbd_transparency` and `videopac_swap_joysticks` to `core_options[]` in `src/libretro.cpp`
    - Transparency: `"Virtual Keyboard Transparency; 25%|0%|50%|75%"`
    - Swap: `"Swap Joysticks; disabled|enabled"`
    - Update `check_variables()` to parse transparency value and set `vkb_transparency_pct`
    - _Requirements: 5.1, 5.2, 9.1_

  - [x] 8.2 Update input descriptors in `retro_set_environment()` to match design
    - SELECT → "Toggle Virtual Keyboard", B → "VKB Press / Key 0", Y → "VKB Position / Key 1"
    - Keep existing labels for A, X, L, START, D-pad, P2 buttons
    - _Requirements: 12.1, 12.2, 12.3, 12.4, 12.5_

  - [x] 8.3 Update `library_version` in `retro_get_system_info()` from "0.1.0" to "0.3.0"
    - _Requirements: 11.1_

- [x] 9. Update `videopac_libretro.info` with BIOS entries and version bump
  - [x] 9.1 Set `display_version` to "0.3.0", `firmware_count` to 4, and add firmware1–firmware3 entries for C52, G7400, and Jopac BIOSes
    - firmware0: o2rom.bin, "Odyssey 2 BIOS", opt=false
    - firmware1: c52.bin, "Videopac C52 BIOS (French)", opt=true
    - firmware2: g7400.bin, "Videopac+ G7400 BIOS", opt=true
    - firmware3: jopac.bin, "Jopac BIOS (French VP+)", opt=true
    - _Requirements: 10.1, 10.2, 10.3, 10.4, 10.5, 11.1_

- [x] 10. Write unit tests for VKB and integration points
  - [x] 10.1 Create `tests/test_vkeyboard.cpp` with unit tests
    - VKB initializes hidden, position bottom (Req 1.4, 6.2)
    - Navigation at corners (top-left, bottom-right keys stay at edge)
    - B press on specific keys (Key 0, Enter, Space) returns correct VidKey
    - Touch on key boundaries (exact edge pixels)
    - Font data: all printable ASCII chars have non-zero glyph data
    - _Requirements: 1.4, 2.1, 2.4, 3.1, 6.2, 7.1, 8.4_

- [x] 11. Final checkpoint — full build and all tests pass
  - Build with `cmake --build --preset dev-mingw`, run `ctest --preset dev-mingw`. Ensure all tests pass. Ask the user if questions arise.

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Property tests use RapidCheck (already in the project) and validate design correctness properties
- The VKB module (`vkeyboard.h/.cpp`) has no SDL dependency and is fully testable in CI
- Follow pre-commit steering workflow for commits
