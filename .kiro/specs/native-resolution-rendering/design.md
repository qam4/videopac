# Design Document: Native Resolution Rendering

## Overview

This design refactors the Videopac emulator's aspect ratio handling to eliminate manual pixel doubling and leverage GPU-based scaling. The current implementation manually stretches the VDC's 160×240 framebuffer to 320×240 in software, mimicking o2em's approach. This refactoring will:

1. Keep the VDC framebuffer at native 160×240 resolution (hardware-accurate)
2. Remove manual 2x horizontal pixel doubling in software
3. Delegate aspect ratio scaling to SDL's GPU-accelerated renderer
4. Support multiple aspect ratio modes: Original (2:3), 4:3 (CRT-like), and Stretch

Benefits include cleaner separation of concerns, better scaling quality, reduced memory usage, more flexible aspect ratio options, and improved hardware accuracy.

## Architecture

### Current Architecture

```
VDC (160×240) → Manual 2x Horizontal Scaling → 320×240 Buffer → SDL Texture (320×240) → SDL Renderer → Display
```

The current implementation performs pixel doubling in `update_texture()`:
- VDC generates 160×240 framebuffer
- Frontend manually doubles each pixel horizontally to create 320×240 buffer
- SDL texture is 320×240
- SDL renderer displays with minimal scaling

### Proposed Architecture

```
VDC (160×240) → SDL Texture (160×240) → SDL Renderer (GPU Scaling) → Display
```

The new implementation delegates scaling to the GPU:
- VDC generates 160×240 framebuffer (unchanged)
- Frontend copies framebuffer directly to 160×240 SDL texture
- SDL renderer handles all aspect ratio scaling via GPU
- Aspect ratio mode determines viewport and scaling behavior

### Component Responsibilities

**VDC (Video Display Controller)**
- Maintains native 160×240 framebuffer
- Renders sprites, characters, grid, and background
- No scaling or aspect ratio concerns
- Hardware-accurate emulation

**SDL Frontend**
- Creates 160×240 SDL texture matching VDC resolution
- Copies VDC framebuffer to texture without modification
- Configures SDL renderer for aspect ratio scaling
- Calculates viewport based on aspect ratio mode
- Applies post-scaling effects (scanlines, CRT)

**Config Manager**
- Stores aspect ratio preference ("original", "4:3", "stretch")
- Stores scaling filter preference ("nearest", "linear")
- Persists settings across sessions

## Components and Interfaces

### VDC Component

**No changes required** - The VDC already outputs 160×240 natively. The manual pixel doubling happens in the frontend, not the VDC.

**Interface (unchanged):**
```cpp
class VDC {
public:
    const uint8* get_framebuffer() const;  // Returns 160×240 framebuffer
    // ... other methods unchanged
};
```

### SDL Frontend Component

**Modified Methods:**

```cpp
class SDLFrontend {
private:
    SDL_Texture* texture_;           // Now 160×240 instead of 320×240
    SDL_Renderer* renderer_;
    
    // Modified methods
    bool init_video();               // Create 160×240 texture
    void update_texture();           // Copy 160×240 directly, no doubling
    void render_frame();             // Use GPU scaling
    SDL_Rect calculate_viewport() const;  // Calculate viewport for aspect ratio
    
    // Unchanged methods
    void render_scanlines(const SDL_Rect& viewport);
    void render_crt_effects(const SDL_Rect& viewport);
};
```

**Key Changes:**

1. **init_video()**: Create SDL texture with 160×240 dimensions
   ```cpp
   texture_ = SDL_CreateTexture(
       renderer_,
       SDL_PIXELFORMAT_RGB24,
       SDL_TEXTUREACCESS_STREAMING,
       160,  // FRAMEBUFFER_WIDTH (was 320)
       240   // FRAMEBUFFER_HEIGHT (unchanged)
   );
   ```

2. **update_texture()**: Remove pixel doubling loop
   ```cpp
   // OLD: Manual 2x horizontal scaling
   for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
       for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
           // Write pixel twice horizontally
           rgb_pixels[...] = color;
           rgb_pixels[...] = color;  // Duplicate
       }
   }
   
   // NEW: Direct copy without scaling
   for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
       for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
           rgb_pixels[...] = color;  // Write once
       }
   }
   ```

3. **render_frame()**: Use calculated viewport for GPU scaling
   ```cpp
   SDL_Rect viewport = calculate_viewport();
   SDL_RenderCopy(renderer_, texture_, nullptr, &viewport);
   ```

4. **calculate_viewport()**: Implement aspect ratio logic
   ```cpp
   SDL_Rect SDLFrontend::calculate_viewport() const {
       int window_width, window_height;
       SDL_GetRendererOutputSize(renderer_, &window_width, &window_height);
       
       SDL_Rect viewport;
       std::string aspect_ratio = config_manager_->get_aspect_ratio();
       
       if (aspect_ratio == "original") {
           // 2:3 aspect ratio (160:240)
           // Calculate viewport maintaining 2:3 ratio
           float target_aspect = 160.0f / 240.0f;  // 2:3
           // ... letterboxing logic
       } else if (aspect_ratio == "4:3") {
           // 4:3 aspect ratio (CRT-like)
           float target_aspect = 4.0f / 3.0f;
           // ... letterboxing logic
       } else {  // "stretch"
           // Fill entire window
           viewport = {0, 0, window_width, window_height};
       }
       
       return viewport;
   }
   ```

### Config Manager Component

**Existing Interface (no changes needed):**
```cpp
class ConfigManager {
public:
    std::string get_aspect_ratio() const;
    void set_aspect_ratio(const std::string& ratio);
    
    std::string get_scaling_filter() const;
    void set_scaling_filter(const std::string& filter);
    
    void save();
    void load();
};
```

The ConfigManager already supports aspect ratio and scaling filter settings. No modifications required.

## Data Models

### Aspect Ratio Modes

**Original Mode (2:3)**
- Viewport aspect ratio: 160:240 (2:3)
- Displays native VDC resolution
- Letterboxing applied to fit window
- Sharp, hardware-accurate pixels

**4:3 Mode (CRT)**
- Viewport aspect ratio: 4:3
- Mimics CRT television display
- Horizontal stretching via GPU
- Letterboxing applied to fit window

**Stretch Mode**
- Viewport fills entire window
- No aspect ratio preservation
- No letterboxing
- May distort image

### Viewport Calculation

For Original and 4:3 modes, viewport is calculated to maintain aspect ratio:

```
target_aspect = desired_width / desired_height
window_aspect = window_width / window_height

if window_aspect > target_aspect:
    // Window is wider, fit to height
    viewport.h = window_height
    viewport.w = window_height * target_aspect
    viewport.x = (window_width - viewport.w) / 2
    viewport.y = 0
else:
    // Window is taller, fit to width
    viewport.w = window_width
    viewport.h = window_width / target_aspect
    viewport.x = 0
    viewport.y = (window_height - viewport.h) / 2
```

### Scaling Quality

SDL provides scaling quality hints:
- `SDL_HINT_RENDER_SCALE_QUALITY = "0"`: Nearest neighbor (sharp, pixelated)
- `SDL_HINT_RENDER_SCALE_QUALITY = "1"`: Linear filtering (smooth, blurred)

The hint must be set before texture creation to take effect.

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: VDC Framebuffer Dimensions

*For any* VDC state, the framebuffer dimensions should be exactly 160 pixels wide and 240 pixels tall.

**Validates: Requirements 1.1, 1.3**

### Property 2: No Pixel Doubling in VDC

*For any* rendered frame, the VDC framebuffer should contain exactly 160×240 unique pixel positions without any horizontal duplication.

**Validates: Requirements 1.2**

### Property 3: SDL Texture Matches VDC Resolution

*For any* SDL texture created for the framebuffer, the texture dimensions should match the VDC framebuffer dimensions (160×240).

**Validates: Requirements 2.1**

### Property 4: Direct Framebuffer Copy

*For any* pixel at position (x, y) in the VDC framebuffer, the corresponding pixel in the SDL texture should have the same RGB color value after update_texture() completes.

**Validates: Requirements 2.3**

### Property 5: Viewport Aspect Ratio Correctness

*For any* aspect ratio mode ("original", "4:3", "stretch") and window dimensions, the calculated viewport should maintain the correct aspect ratio for that mode:
- Original mode: viewport width / viewport height ≈ 2/3 (within floating point tolerance)
- 4:3 mode: viewport width / viewport height ≈ 4/3 (within floating point tolerance)
- Stretch mode: viewport dimensions equal window dimensions

**Validates: Requirements 4.1, 5.1, 6.1**

### Property 6: Letterboxing and Centering

*For any* aspect ratio mode that requires letterboxing (Original or 4:3), when the window aspect ratio differs from the target aspect ratio, the viewport should be:
- Smaller than the window in at least one dimension
- Centered within the window (viewport.x or viewport.y positioned to center the viewport)

**Validates: Requirements 4.2, 4.3, 5.2, 5.3**

### Property 7: Stretch Mode Fills Window

*For any* window dimensions, when aspect ratio mode is "stretch", the viewport should exactly match the window dimensions with no letterboxing.

**Validates: Requirements 6.1, 6.2**

### Property 8: Configuration Persistence Round Trip

*For any* valid aspect ratio value ("original", "4:3", "stretch") or scaling filter value ("nearest", "linear"), setting the value, saving the configuration, reloading the configuration, and reading the value should return the same value.

**Validates: Requirements 7.1, 7.2, 7.3, 7.4, 9.4**

### Property 9: Scaling Filter Hint Correctness

*For any* scaling filter setting:
- When filter is "nearest", SDL_HINT_RENDER_SCALE_QUALITY should be set to "0"
- When filter is "linear", SDL_HINT_RENDER_SCALE_QUALITY should be set to "1"

**Validates: Requirements 9.2, 9.3**

### Property 10: Screenshot Captures Native Resolution

*For any* screenshot taken, the output image dimensions should be 160×240 pixels (the native VDC resolution).

**Validates: Requirements 8.5**

### Property 11: Extended Framebuffer Mode Support

*For any* VDC state with extended framebuffer mode enabled, the SDL frontend should:
- Create a texture with dimensions matching the extended framebuffer (240×250)
- Apply the same aspect ratio scaling logic to the extended framebuffer
- Successfully render the extended framebuffer without errors

**Validates: Requirements 12.1, 12.2, 12.3, 12.4**

### Property 12: Rendering Effects Applied Post-Scaling

*For any* enabled rendering effect (scanlines or CRT), the effect should be applied to the scaled viewport, not the original 160×240 texture.

**Validates: Requirements 11.4**

## Error Handling

### Texture Creation Failure

If SDL texture creation fails:
```cpp
texture_ = SDL_CreateTexture(...);
if (!texture_) {
    std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
    return false;
}
```

### Invalid Aspect Ratio Setting

If an invalid aspect ratio value is loaded from config:
```cpp
std::string aspect_ratio = config_manager_->get_aspect_ratio();
if (aspect_ratio != "original" && aspect_ratio != "4:3" && aspect_ratio != "stretch") {
    std::cerr << "Invalid aspect ratio: " << aspect_ratio << ", defaulting to '4:3'" << std::endl;
    aspect_ratio = "4:3";
}
```

### Window Size Edge Cases

Handle zero or negative window dimensions:
```cpp
int window_width, window_height;
SDL_GetRendererOutputSize(renderer_, &window_width, &window_height);
if (window_width <= 0 || window_height <= 0) {
    // Return default viewport
    return {0, 0, 160, 240};
}
```

## Testing Strategy

### Dual Testing Approach

This feature requires both unit tests and property-based tests for comprehensive coverage:

**Unit Tests** - Verify specific examples, edge cases, and error conditions:
- Texture creation with correct dimensions
- Viewport calculation for specific window sizes
- Configuration persistence for specific values
- Error handling for invalid inputs

**Property-Based Tests** - Verify universal properties across all inputs:
- Viewport aspect ratio correctness for random window dimensions
- Configuration round-trip for random valid values
- Framebuffer copy correctness for random pixel data
- Letterboxing behavior for random window sizes

### Property-Based Testing Configuration

Use **fast-check** (JavaScript/TypeScript) or **Hypothesis** (Python) or **QuickCheck** (C++) for property-based testing:
- Minimum 100 iterations per property test
- Each test tagged with: **Feature: native-resolution-rendering, Property N: [property text]**
- Generate random window dimensions, aspect ratios, and pixel data
- Verify properties hold across all generated inputs

### Test Coverage

**Unit Tests:**
1. Test texture creation with 160×240 dimensions
2. Test viewport calculation for Original mode with specific window sizes
3. Test viewport calculation for 4:3 mode with specific window sizes
4. Test viewport calculation for Stretch mode
5. Test configuration save/load for each aspect ratio value
6. Test scaling filter hint setting for "nearest" and "linear"
7. Test screenshot dimensions are 160×240
8. Test extended framebuffer texture creation
9. Test error handling for invalid aspect ratio values
10. Test error handling for zero/negative window dimensions

**Property-Based Tests:**
1. Property 1: VDC framebuffer dimensions (100+ random VDC states)
2. Property 4: Direct framebuffer copy (100+ random pixel patterns)
3. Property 5: Viewport aspect ratio correctness (100+ random window sizes and aspect modes)
4. Property 6: Letterboxing and centering (100+ random window sizes)
5. Property 7: Stretch mode fills window (100+ random window sizes)
6. Property 8: Configuration persistence round trip (100+ random valid values)
7. Property 9: Scaling filter hint correctness (100+ random filter values)
8. Property 10: Screenshot resolution (100+ random framebuffer states)
9. Property 11: Extended framebuffer support (100+ random extended framebuffer states)

### Integration Tests

1. Test complete rendering pipeline: VDC → Texture → Viewport → Display
2. Test aspect ratio switching during runtime
3. Test fullscreen mode with different aspect ratios
4. Test window resizing with different aspect ratios
5. Test menu integration for aspect ratio selection
6. Test backward compatibility with existing save states

### Visual Regression Tests

1. Capture screenshots in each aspect ratio mode
2. Compare with reference images to detect visual regressions
3. Verify scanlines and CRT effects render correctly after scaling
4. Verify UI overlays render correctly with new viewport logic

## Implementation Notes

### Backward Compatibility

The refactoring maintains full backward compatibility:
- All existing menu options continue to work
- All keyboard shortcuts remain functional
- Configuration file format unchanged
- Save states unaffected (VDC state unchanged)
- Screenshot functionality preserved

### Performance Considerations

GPU scaling is significantly faster than software pixel doubling:
- Eliminates CPU-based pixel doubling loop
- Reduces memory bandwidth (smaller texture)
- Leverages hardware-accelerated scaling
- No performance regression expected

### Migration Path

The change is transparent to users:
- No configuration migration needed
- Existing aspect ratio settings continue to work
- Visual output remains consistent (4:3 mode matches old behavior)
- No user action required

### Extended Framebuffer Mode

The extended framebuffer debug mode (240×250) requires special handling:
- Check if extended mode is enabled before creating texture
- Create appropriately sized texture (240×250 vs 160×240)
- Apply same viewport calculation logic
- Ensure aspect ratio modes work correctly with extended dimensions

### Scaling Quality

The scaling filter setting affects visual quality:
- "nearest": Sharp, pixelated look (retro aesthetic)
- "linear": Smooth, blurred look (modern aesthetic)
- Default to "nearest" for authentic retro feel
- Allow users to choose based on preference
