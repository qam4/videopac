# Implementation Plan: Native Resolution Rendering

## Overview

This implementation plan refactors the Videopac emulator's aspect ratio handling to eliminate manual pixel doubling and leverage GPU-based scaling. The work is organized into discrete tasks that build incrementally, with testing integrated throughout to catch errors early.

## Tasks

- [x] 1. Update SDL texture creation to native resolution
  - Modify `init_video()` in `src/frontend_sdl.cpp` to create 160×240 texture instead of 320×240
  - Update texture creation call to use `FRAMEBUFFER_WIDTH` (160) and `FRAMEBUFFER_HEIGHT` (240) directly
  - Remove any hardcoded 320×240 dimensions
  - Verify texture is created successfully with new dimensions
  - _Requirements: 2.1, 2.2_

- [ ]* 1.1 Write unit test for texture dimensions
  - Test that SDL texture is created with 160×240 dimensions
  - Test that texture uses SDL_PIXELFORMAT_RGB24 format
  - _Requirements: 2.1, 2.2_

- [x] 2. Remove manual pixel doubling from update_texture()
  - Modify `update_texture()` in `src/frontend_sdl.cpp` to copy pixels directly without horizontal doubling
  - Change the inner loop to write each pixel once instead of twice
  - Update pitch calculations to match 160-pixel width
  - Verify framebuffer data is copied correctly
  - _Requirements: 1.2, 2.3_

- [ ]* 2.1 Write property test for direct framebuffer copy
  - **Property 4: Direct Framebuffer Copy**
  - **Validates: Requirements 2.3**
  - Generate random pixel patterns
  - Verify each pixel in texture matches corresponding VDC framebuffer pixel
  - Test with 100+ random framebuffer states

- [x] 3. Implement viewport calculation for aspect ratio modes
  - Create or modify `calculate_viewport()` method in `src/frontend_sdl.cpp`
  - Implement Original mode (2:3 aspect ratio) with letterboxing
  - Implement 4:3 mode with letterboxing
  - Implement Stretch mode (fill window)
  - Handle edge cases (zero/negative window dimensions)
  - _Requirements: 4.1, 4.2, 4.3, 5.1, 5.2, 5.3, 6.1, 6.2_

- [ ]* 3.1 Write property test for viewport aspect ratio correctness
  - **Property 5: Viewport Aspect Ratio Correctness**
  - **Validates: Requirements 4.1, 5.1, 6.1**
  - Generate random window dimensions and aspect ratio modes
  - Verify viewport maintains correct aspect ratio for each mode
  - Test with 100+ random window sizes

- [ ]* 3.2 Write property test for letterboxing and centering
  - **Property 6: Letterboxing and Centering**
  - **Validates: Requirements 4.2, 4.3, 5.2, 5.3**
  - Generate random window dimensions for Original and 4:3 modes
  - Verify viewport is centered when letterboxing is applied
  - Test with 100+ random window sizes

- [ ]* 3.3 Write property test for stretch mode
  - **Property 7: Stretch Mode Fills Window**
  - **Validates: Requirements 6.1, 6.2**
  - Generate random window dimensions
  - Verify viewport exactly matches window dimensions in stretch mode
  - Test with 100+ random window sizes

- [ ]* 3.4 Write unit tests for viewport edge cases
  - Test viewport calculation with zero window dimensions
  - Test viewport calculation with negative window dimensions
  - Test viewport calculation with very small window dimensions
  - Verify error handling returns sensible defaults

- [x] 4. Update render_frame() to use GPU scaling
  - Modify `render_frame()` in `src/frontend_sdl.cpp` to use calculated viewport
  - Replace hardcoded destination rectangle with `calculate_viewport()` result
  - Ensure SDL_RenderCopy uses the viewport for scaling
  - Update logical size handling if needed
  - _Requirements: 3.1, 3.2, 3.3_

- [x] 5. Update post-scaling effects to use viewport
  - Modify `render_scanlines()` to apply effects to the scaled viewport
  - Modify `render_crt_effects()` to apply effects to the scaled viewport
  - Ensure effects are applied after GPU scaling, not before
  - _Requirements: 11.4_

- [ ]* 5.1 Write property test for post-scaling effects
  - **Property 12: Rendering Effects Applied Post-Scaling**
  - **Validates: Requirements 11.4**
  - Verify scanlines and CRT effects are applied to viewport dimensions, not texture dimensions
  - Test with various viewport sizes

- [x] 6. Verify and update scaling filter hint handling
  - Verify `SDL_HINT_RENDER_SCALE_QUALITY` is set correctly in `init_video()`
  - Ensure hint is set before texture creation
  - Verify "nearest" maps to "0" and "linear" maps to "1"
  - _Requirements: 9.1, 9.2, 9.3_

- [ ]* 6.1 Write property test for scaling filter hints
  - **Property 9: Scaling Filter Hint Correctness**
  - **Validates: Requirements 9.2, 9.3**
  - Test that "nearest" sets hint to "0"
  - Test that "linear" sets hint to "1"
  - Verify hint is applied before texture creation

- [x] 7. Update screenshot functionality
  - Verify `dump_framebuffer()` captures native 160×240 resolution
  - Ensure screenshot dimensions match VDC framebuffer dimensions
  - Update PPM header to reflect 160×240 dimensions
  - _Requirements: 8.5_

- [ ]* 7.1 Write property test for screenshot dimensions
  - **Property 10: Screenshot Captures Native Resolution**
  - **Validates: Requirements 8.5**
  - Generate random framebuffer states
  - Verify screenshot output is always 160×240 pixels
  - Test with 100+ random framebuffer states

- [x] 8. Checkpoint - Ensure all tests pass
  - Run all unit tests and property tests
  - Verify no regressions in existing functionality
  - Test each aspect ratio mode visually
  - Ensure all tests pass, ask the user if questions arise.

- [x] 9. Add extended framebuffer mode support
  - Update texture creation to handle extended framebuffer mode (240×250)
  - Check `is_extended_framebuffer_mode()` before creating texture
  - Create appropriately sized texture based on mode
  - Apply viewport calculation to extended framebuffer dimensions
  - _Requirements: 12.1, 12.2, 12.3, 12.4_

- [ ]* 9.1 Write property test for extended framebuffer support
  - **Property 11: Extended Framebuffer Mode Support**
  - **Validates: Requirements 12.1, 12.2, 12.3, 12.4**
  - Test texture creation with extended framebuffer dimensions
  - Test viewport calculation with extended framebuffer
  - Verify aspect ratio modes work correctly with extended dimensions

- [x] 10. Update configuration persistence tests
  - Verify existing ConfigManager tests cover aspect ratio persistence
  - Add tests for round-trip persistence if missing
  - Verify all aspect ratio values ("original", "4:3", "stretch") are supported
  - Ensure "4:3" is used as the default when no configuration exists
  - _Requirements: 5.5, 7.1, 7.2, 7.3, 7.4_

- [ ]* 10.1 Write property test for configuration persistence
  - **Property 8: Configuration Persistence Round Trip**
  - **Validates: Requirements 7.1, 7.2, 7.3, 7.4, 9.4**
  - Generate random valid aspect ratio and scaling filter values
  - Test save/load round trip preserves values
  - Test with 100+ random valid configurations

- [x] 11. Verify backward compatibility
  - Test all existing menu options for aspect ratio selection
  - Test all keyboard shortcuts continue to work
  - Verify OSD and UI overlays render correctly
  - Test save state loading and saving
  - _Requirements: 8.1, 8.2, 8.3, 8.4_

- [ ]* 11.1 Write integration tests for backward compatibility
  - Test menu actions for aspect ratio switching
  - Test keyboard shortcuts trigger correct actions
  - Test UI overlays render at correct positions
  - Test save states are unaffected by changes

- [x] 12. Final checkpoint - Comprehensive testing
  - Run full test suite (unit + property + integration tests)
  - Perform visual regression testing for each aspect ratio mode
  - Test fullscreen mode with all aspect ratios
  - Test window resizing with all aspect ratios
  - Verify performance is equal or better than before
  - Ensure all tests pass, ask the user if questions arise.

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Checkpoints ensure incremental validation
- Property tests validate universal correctness properties with 100+ iterations
- Unit tests validate specific examples and edge cases
- The refactoring maintains full backward compatibility with existing features
