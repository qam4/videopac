# Requirements Document

## Introduction

This document specifies requirements for refactoring aspect ratio handling in the Videopac emulator. The current implementation manually stretches the VDC's native 160×240 framebuffer to 320×240 (2x horizontal scaling) in software, mimicking o2em's approach. This refactoring will eliminate the manual pixel doubling, keep the VDC framebuffer at its hardware-accurate native resolution, and delegate aspect ratio scaling to the GPU via SDL, resulting in cleaner separation of concerns, better scaling quality, reduced memory usage, and more flexible aspect ratio options.

## Glossary

- **VDC**: Video Display Controller - The Intel 8245 chip that generates video output for the Videopac/Odyssey2 console
- **Framebuffer**: The pixel buffer containing the rendered frame data before display
- **Native Resolution**: The actual resolution output by the VDC hardware (160×240 pixels)
- **SDL**: Simple DirectMedia Layer - The graphics library used for rendering
- **SDL Texture**: The GPU texture object that holds the framebuffer data for rendering
- **SDL Renderer**: The SDL component that handles drawing textures to the window
- **Aspect Ratio**: The proportional relationship between width and height of the displayed image
- **GPU Scaling**: Hardware-accelerated image scaling performed by the graphics card
- **Pixel Doubling**: Software-based duplication of pixels to increase resolution
- **CRT**: Cathode Ray Tube - The display technology used with original Videopac consoles
- **Letterboxing**: Black bars added to maintain aspect ratio when scaling

## Requirements

### Requirement 1: VDC Native Resolution Preservation

**User Story:** As an emulator developer, I want the VDC to maintain its hardware-accurate native resolution, so that the emulation accurately represents the actual hardware behavior.

#### Acceptance Criteria

1. THE VDC SHALL maintain a framebuffer resolution of 160×240 pixels
2. THE VDC SHALL NOT perform any horizontal pixel doubling or scaling operations
3. WHEN the VDC renders a frame, THE VDC SHALL output exactly 160×240 pixels
4. THE VDC SHALL preserve all existing rendering logic for sprites, characters, grid, and background

### Requirement 2: SDL Texture Configuration

**User Story:** As an emulator developer, I want the SDL texture to match the VDC's native resolution, so that no intermediate scaling buffers are needed.

#### Acceptance Criteria

1. THE SDL_Frontend SHALL create an SDL texture with dimensions 160×240 pixels
2. THE SDL_Frontend SHALL use SDL_PIXELFORMAT_RGB24 pixel format for the texture
3. WHEN updating the texture, THE SDL_Frontend SHALL copy VDC framebuffer data directly without modification
4. THE SDL_Frontend SHALL NOT create any intermediate 320×240 scaling buffers

### Requirement 3: GPU-Based Aspect Ratio Scaling

**User Story:** As a user, I want the emulator to use GPU scaling for aspect ratio handling, so that I get better image quality and performance.

#### Acceptance Criteria

1. THE SDL_Frontend SHALL delegate all aspect ratio scaling to the SDL renderer
2. THE SDL_Frontend SHALL configure the SDL renderer to handle scaling from 160×240 to the display resolution
3. WHEN rendering a frame, THE SDL_Frontend SHALL use SDL_RenderCopy to scale the texture
4. THE SDL_Frontend SHALL use the SDL_HINT_RENDER_SCALE_QUALITY hint to control scaling quality

### Requirement 4: Original Aspect Ratio Mode

**User Story:** As a user interested in hardware accuracy, I want an "Original" aspect ratio mode that displays the native 2:3 ratio, so that I can see the exact pixel dimensions the VDC produces.

#### Acceptance Criteria

1. WHEN the aspect ratio setting is "Original", THE SDL_Frontend SHALL display the framebuffer at 2:3 aspect ratio (160:240)
2. WHEN the aspect ratio setting is "Original", THE SDL_Frontend SHALL apply letterboxing as needed to fit the window
3. THE SDL_Frontend SHALL center the display within the window when letterboxing is applied
4. THE SDL_Frontend SHALL maintain sharp pixel rendering without blurring in Original mode

### Requirement 5: 4:3 CRT Aspect Ratio Mode

**User Story:** As a user who wants an authentic CRT experience, I want a "4:3" aspect ratio mode that mimics how the image appeared on original CRT televisions, so that games look as they did on original hardware.

#### Acceptance Criteria

1. WHEN the aspect ratio setting is "4:3", THE SDL_Frontend SHALL scale the 160×240 framebuffer to maintain a 4:3 display aspect ratio
2. WHEN the aspect ratio setting is "4:3", THE SDL_Frontend SHALL apply letterboxing as needed to fit the window
3. THE SDL_Frontend SHALL center the display within the window when letterboxing is applied
4. THE SDL_Frontend SHALL use GPU scaling to stretch the image horizontally to achieve 4:3 ratio
5. THE SDL_Frontend SHALL use "4:3" as the default aspect ratio mode when no configuration exists

### Requirement 6: Stretch Aspect Ratio Mode

**User Story:** As a user who wants to maximize screen usage, I want a "Stretch" aspect ratio mode that fills the entire window, so that I can use all available display space.

#### Acceptance Criteria

1. WHEN the aspect ratio setting is "Stretch", THE SDL_Frontend SHALL scale the framebuffer to fill the entire window
2. WHEN the aspect ratio setting is "Stretch", THE SDL_Frontend SHALL NOT apply letterboxing
3. THE SDL_Frontend SHALL allow the image to be distorted if the window aspect ratio differs from the framebuffer

### Requirement 7: Aspect Ratio Configuration Persistence

**User Story:** As a user, I want my aspect ratio preference to be saved, so that it persists across emulator sessions.

#### Acceptance Criteria

1. THE ConfigManager SHALL store the aspect ratio setting in the configuration file
2. WHEN the emulator starts, THE SDL_Frontend SHALL load the saved aspect ratio setting
3. WHEN the user changes the aspect ratio, THE ConfigManager SHALL save the new setting immediately
4. THE ConfigManager SHALL support aspect ratio values: "original", "4:3", and "stretch"

### Requirement 8: Backward Compatibility

**User Story:** As an emulator developer, I want the refactoring to maintain all existing functionality, so that no features are broken by the change.

#### Acceptance Criteria

1. THE SDL_Frontend SHALL maintain all existing menu functionality for aspect ratio selection
2. THE SDL_Frontend SHALL maintain all existing keyboard shortcuts
3. THE SDL_Frontend SHALL maintain all existing OSD and UI overlay rendering
4. THE SDL_Frontend SHALL maintain all existing screenshot functionality
5. WHEN taking a screenshot, THE SDL_Frontend SHALL capture the native 160×240 framebuffer

### Requirement 9: Scaling Quality Control

**User Story:** As a user, I want control over the scaling quality, so that I can choose between sharp pixels or smooth scaling based on my preference.

#### Acceptance Criteria

1. THE SDL_Frontend SHALL support "nearest" (sharp) and "linear" (smooth) scaling filters
2. WHEN the scaling filter is "nearest", THE SDL_Frontend SHALL use SDL_HINT_RENDER_SCALE_QUALITY value "0"
3. WHEN the scaling filter is "linear", THE SDL_Frontend SHALL use SDL_HINT_RENDER_SCALE_QUALITY value "1"
4. THE ConfigManager SHALL persist the scaling filter setting across sessions

### Requirement 10: Memory Efficiency

**User Story:** As an emulator developer, I want to eliminate unnecessary memory allocations, so that the emulator uses resources efficiently.

#### Acceptance Criteria

1. THE SDL_Frontend SHALL NOT allocate any intermediate 320×240 pixel buffers
2. THE SDL_Frontend SHALL use only the native 160×240 VDC framebuffer for rendering
3. WHEN converting palette indices to RGB, THE SDL_Frontend SHALL write directly to the SDL texture
4. THE SDL_Frontend SHALL maintain the same memory usage pattern for the VDC framebuffer

### Requirement 11: Rendering Pipeline Integrity

**User Story:** As an emulator developer, I want the rendering pipeline to remain clean and maintainable, so that future modifications are straightforward.

#### Acceptance Criteria

1. THE SDL_Frontend SHALL maintain a clear separation between VDC rendering and display scaling
2. THE VDC SHALL remain responsible only for generating the native resolution framebuffer
3. THE SDL_Frontend SHALL remain responsible only for displaying the framebuffer with user-selected scaling
4. WHEN rendering effects (scanlines, CRT) are applied, THE SDL_Frontend SHALL apply them after scaling

### Requirement 12: Extended Framebuffer Mode Compatibility

**User Story:** As a developer debugging rendering issues, I want the extended framebuffer debug mode to continue working, so that I can visualize the full VDC rendering area.

#### Acceptance Criteria

1. WHEN extended framebuffer mode is enabled, THE VDC SHALL continue to provide the 240×250 extended framebuffer
2. THE SDL_Frontend SHALL support rendering the extended framebuffer when enabled
3. THE SDL_Frontend SHALL apply the same aspect ratio scaling logic to the extended framebuffer
4. THE SDL_Frontend SHALL create an appropriately sized texture for the extended framebuffer when needed
