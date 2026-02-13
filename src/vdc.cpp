#include "vdc.h"
#include <cstring>
#include <iostream>
#include <iomanip>

namespace videopac {

// Constructor
// Reference: doc/o2doc.md section 4.11, doc/8245.md lines 520-560
VDC::VDC(VideoStandard standard) {
    state_.video_standard = standard;
    extended_fb_mode_ = false;
    calculate_timing();
    reset();
}

// Reset VDC to initial state
// Reference: doc/o2doc.md section 4.0, doc/8245.md lines 1-30
void VDC::reset() {
    // Clear all registers
    std::memset(&state_.registers, 0, sizeof(state_.registers));
    
    // Clear framebuffer
    std::memset(&state_.framebuffer, 0, sizeof(state_.framebuffer));
    
    // Clear extended framebuffer
    std::memset(&state_.extended_framebuffer, 0, sizeof(state_.extended_framebuffer));
    
    // Reset timing state
    state_.beam_x = 0;
    state_.beam_y = 0;
    state_.total_cycles = 0;
    
    // Reset collision state
    state_.collision_state = 0;
    state_.collision_detected = false;
    
    // Reset display state
    state_.display_enabled = false;
    state_.grid_enabled = false;
    
    // Reset audio state
    state_.audio_shift_register = 0;
    state_.audio_shift_counter = 0;
    state_.audio_frequency = AUDIO_FREQ_LOW;
    state_.audio_volume = 0;
    state_.audio_enabled = false;
    state_.audio_loop = false;
    state_.audio_noise = false;
    state_.audio_cycle_accumulator = 0;
}

// Advance VDC by specified number of cycles
// Reference: doc/o2doc.md section 4.11, doc/8245.md lines 520-560
void VDC::tick(uint8 cycles) {
    // For backward compatibility, call tick_one_cycle() multiple times
    for (uint8 i = 0; i < cycles; i++) {
        tick_one_cycle();
    }
}

// Advance VDC by exactly one clock cycle
// Reference: Requirements 2.1, 2.3, 2.4
void VDC::tick_one_cycle() {
    // Save previous beam position to detect scanline completion
    uint32 prev_beam_y = state_.beam_y;
    
    // Advance total cycles
    state_.total_cycles++;
    
    // Calculate beam position from total cycles
    uint32 cycles_per_frame = total_scanlines_ * VideoTiming::CYCLES_PER_SCANLINE;
    uint32 frame_cycles = state_.total_cycles % cycles_per_frame;
    state_.beam_y = frame_cycles / VideoTiming::CYCLES_PER_SCANLINE;
    state_.beam_x = frame_cycles % VideoTiming::CYCLES_PER_SCANLINE;
    
    // Update audio
    update_audio();
    
    // Update beam position registers based on latch bit
    // Reference: doc/o2doc.md section 4.14 - Bit 1: 1 = Follow Beam, 0 = Latched
    if (state_.registers[VDCRegisters::CONTROL] & ControlBits::LATCH_BEAM_POS) {
        // When bit is SET, position follows beam (updates continuously)
        state_.registers[VDCRegisters::BEAM_X] = static_cast<uint8>(state_.beam_x);
        state_.registers[VDCRegisters::BEAM_Y] = static_cast<uint8>(state_.beam_y);
    }
    // When bit is CLEAR, registers remain latched at their current value
    
    // Update status register
    uint8 status = 0;
    
    // HBLANK status - active when beam_x >= visible width
    if (state_.beam_x >= FRAMEBUFFER_WIDTH) {
        status |= StatusBits::HBLANK;
    }
    
    // VBLANK status
    if (is_vblank()) {
        status |= StatusBits::VBLANK;
    }
    
    // Position strobe status
    if (state_.registers[VDCRegisters::CONTROL] & ControlBits::LATCH_BEAM_POS) {
        status &= ~StatusBits::POS_STROBE_STATUS;  // Latched
    } else {
        status |= StatusBits::POS_STROBE_STATUS;   // Following beam
    }
    
    // Preserve collision and sound status bits
    status |= (state_.registers[VDCRegisters::STATUS] & (StatusBits::SOUND_NEEDS_SERVICE | 
                                                          StatusBits::EXT_OVERLAP | 
                                                          StatusBits::CHAR_OVERLAP));
    
    state_.registers[VDCRegisters::STATUS] = status;
    
    // Render pixel at current beam position if visible
    if (is_beam_visible()) {
        render_current_pixel();
    }
    
    // Detect collisions at the end of each scanline
    // This must happen after all pixels on the scanline have been rendered
    if (state_.beam_y != prev_beam_y && prev_beam_y < FRAMEBUFFER_HEIGHT) {
        detect_collisions(prev_beam_y);
    }
}

// Write to VDC register
// Reference: doc/o2doc.md Appendix D, doc/8245.md lines 600-650
void VDC::write_register(uint8 address, uint8 value) {
    state_.registers[address] = value;
    
    // Handle special registers that update internal state
    switch (address) {
        case VDCRegisters::CONTROL: {
            // Control register - update display and grid enable flags
            // Reference: doc/o2doc.md section 4.6, doc/8245.md lines 440-470
            state_.display_enabled = (value & ControlBits::ENABLE_DISPLAY) != 0;
            state_.grid_enabled = (value & ControlBits::ENABLE_GRID) != 0;          
             break;
        }
            
        case VDCRegisters::COLLISION:
            // Collision register - writing sets which objects to track
            // Clear collision state when new enable mask is written
            // Reference: doc/o2doc.md section 4.8
            state_.collision_state = 0;
            state_.collision_detected = false;
            break;
            
        case VDCRegisters::SOUND0:
        case VDCRegisters::SOUND1:
        case VDCRegisters::SOUND2:
            // Audio shift register bytes
            // Reference: doc/o2doc.md section 4.10, doc/8245.md lines 380-420
            {
                uint8 byte_index = address - VDCRegisters::SOUND0;
                uint32 mask = 0xFF << (byte_index * 8);
                state_.audio_shift_register = (state_.audio_shift_register & ~mask) | 
                                              (static_cast<uint32>(value) << (byte_index * 8));
            }
            break;
            
        case VDCRegisters::SOUND_CONTROL:
            // Audio control register
            // Reference: doc/o2doc.md section 4.10, doc/8245.md lines 380-420
            state_.audio_enabled = (value & SoundControlBits::ENABLE_SOUND) != 0;
            state_.audio_loop = (value & SoundControlBits::LOOP_MODE) != 0;
            state_.audio_frequency = (value & SoundControlBits::SHIFT_FREQ) ? AUDIO_FREQ_HIGH : AUDIO_FREQ_LOW;
            state_.audio_noise = (value & SoundControlBits::ENABLE_NOISE) != 0;
            state_.audio_volume = value & SoundControlBits::VOLUME_MASK;
            
            // Reset shift counter when sound control changes
            if (state_.audio_enabled) {
                state_.audio_shift_counter = 0;
            }
            break;
            
        default:
            // Other registers are just stored
            break;
    }
}

// Read from VDC register
// Reference: doc/o2doc.md Appendix D, doc/8245.md lines 600-650
uint8 VDC::read_register(uint8 address) {
    // Special handling for read-only registers
    switch (address) {
        case VDCRegisters::STATUS:
            // Reading status register clears certain bits
            // Reference: doc/o2doc.md section 4.7, doc/8245.md lines 480-500
            {
                uint8 status = state_.registers[address];
                // Clear sound needs service bit on read
                state_.registers[address] &= ~StatusBits::SOUND_NEEDS_SERVICE;
                return status;
            }
            
        case VDCRegisters::COLLISION:
            // Reading collision register returns collision state and clears it
            // Reference: doc/o2doc.md section 4.8, doc/8245.md lines 500-520
            {
                // Return the accumulated collision state
                uint8 collision = state_.collision_state;
                
                // Clear collision state after read
                state_.collision_state = 0;
                state_.collision_detected = false;
                
                // Keep the enable mask in the register
                // (the register value is the enable mask, collision_state is the result)
                return collision;
            }
            
        default:
            return state_.registers[address];
    }
}

// Render current scanline to framebuffer
// NOTE: This function is primarily used by tests. During normal emulation,
// rendering happens per-pixel via render_current_pixel() called from tick_one_cycle().
// Reference: doc/o2doc.md section 4.0, doc/8245.md lines 200-300
void VDC::render_scanline() {
    // Determine max height based on extended framebuffer mode
    int max_height = extended_fb_mode_ ? EXTENDED_FB_HEIGHT : FRAMEBUFFER_HEIGHT;
    
    // Only render during visible scanlines (not in VBLANK)
    if (is_vblank() || state_.beam_y >= max_height) {
        return;
    }
    
    // Check if display is enabled
    if (!state_.display_enabled) {
        return;
    }
    
    int y = static_cast<int>(state_.beam_y);
    
    // Render in priority order (background to foreground)
    // Reference: doc/o2doc.md section 4.0, design.md Property 21
    render_background(y);
    render_grid(y);
    render_characters(y);
    render_sprites(y);
    
    // Detect collisions for this scanline
    detect_collisions(y);
}

// Render pixel at current beam position
// Reference: Requirements 2.1, 2.3, 2.6, 12.1-12.4
void VDC::render_current_pixel() {
    // Only render if beam is in visible area
    if (!is_beam_visible()) {
        return;
    }
    
    // Check if display is enabled
    if (!state_.display_enabled) {
        return;
    }
    
    int x = static_cast<int>(state_.beam_x);
    int y = static_cast<int>(state_.beam_y);
    
    // Render in priority order (background to foreground)
    // Priority: sprites (highest) > characters > grid > background (lowest)
    // Reference: Requirements 12.1, 12.2, 12.3, 12.4
    
    // Start with background color
    uint8 color_reg = state_.registers[VDCRegisters::COLOR];
    uint8 bg_color = (color_reg >> 3) & 0x07;
    uint8 pixel_color = bg_color;
    
    // Check grid at this position (if enabled)
    if (state_.grid_enabled) {
        uint8 grid_color = color_reg & 0x07;
        if (is_grid_pixel_at(x, y)) {
            pixel_color = grid_color;
        }
    }
    
    // Check characters at this position
    uint8 char_color;
    if (is_character_pixel_at(x, y, char_color)) {
        pixel_color = char_color;
    }
    
    // Check sprites at this position (highest priority)
    uint8 sprite_color;
    if (is_sprite_pixel_at(x, y, sprite_color)) {
        pixel_color = sprite_color;
    }
    
    // Write to normal framebuffer if within bounds
    if (x < FRAMEBUFFER_WIDTH && y < FRAMEBUFFER_HEIGHT) {
        state_.framebuffer[y][x] = pixel_color;
    }
    
    // Write to extended framebuffer if enabled and within extended bounds
    if (extended_fb_mode_ && x < EXTENDED_FB_WIDTH && y < EXTENDED_FB_HEIGHT) {
        state_.extended_framebuffer[y][x] = pixel_color;
    }
}

// Get framebuffer pointer
const uint8* VDC::get_framebuffer() const {
    return &state_.framebuffer[0][0];
}

// Extended framebuffer mode control
// Enables rendering to a larger framebuffer to see what's outside visible area
void VDC::set_extended_framebuffer_mode(bool enabled) {
    extended_fb_mode_ = enabled;
    if (enabled) {
        std::cout << "Extended framebuffer mode enabled (" 
                  << EXTENDED_FB_WIDTH << "x" << EXTENDED_FB_HEIGHT << ")" << std::endl;
    }
}

// Get extended framebuffer pointer
const uint8* VDC::get_extended_framebuffer() const {
    return &state_.extended_framebuffer[0][0];
}

// Check if in vertical blank period
// Reference: doc/o2doc.md section 4.11, doc/8245.md lines 520-560
bool VDC::is_vblank() const {
    return state_.beam_y >= vblank_start_;
}

// Check if in horizontal blank period
// Reference: doc/o2doc.md section 4.11, doc/8245.md lines 520-560
bool VDC::is_hblank() const {
    // HBLANK is active when beam_x is beyond visible width
    return state_.beam_x >= FRAMEBUFFER_WIDTH;
}

// Check if beam is in visible area
// Reference: Requirements 2.4, 2.6
bool VDC::is_beam_visible() const {
    int max_width = extended_fb_mode_ ? EXTENDED_FB_WIDTH : FRAMEBUFFER_WIDTH;
    int max_height = extended_fb_mode_ ? EXTENDED_FB_HEIGHT : FRAMEBUFFER_HEIGHT;
    
    return state_.beam_x < max_width && 
           state_.beam_y < max_height &&
           !is_hblank() && 
           !is_vblank();
}

// Get current audio sample
// Reference: doc/o2doc.md section 4.10, doc/8245.md lines 380-420
int16 VDC::get_audio_sample() {
    if (!state_.audio_enabled) {
        return 0;
    }

    // Get current bit from shift register (bit 0 is output)
    uint8 bit = state_.audio_shift_register & 1;

    // Scale by volume (0-15 maps to -32767 to +32767)
    int16 sample = bit ? (state_.audio_volume * 2184) : -(state_.audio_volume * 2184);

    return sample;
}

// Get complete VDC state
VDCState VDC::get_state() const {
    return state_;
}

// Restore VDC state
void VDC::set_state(const VDCState& state) {
    state_ = state;
    calculate_timing();  // Recalculate timing based on video standard
}

// Calculate timing parameters based on video standard
// Reference: doc/o2doc.md section 4.11, doc/8245.md lines 520-560
void VDC::calculate_timing() {
    if (state_.video_standard == VideoStandard::NTSC) {
        total_scanlines_ = VideoTiming::NTSC_SCANLINES;
        vblank_start_ = VideoTiming::NTSC_VBLANK_START;
        cycles_per_scanline_ = VideoTiming::NTSC_CYCLES_PER_SCANLINE;
    } else {  // PAL
        total_scanlines_ = VideoTiming::PAL_SCANLINES;
        vblank_start_ = VideoTiming::PAL_VBLANK_START;
        cycles_per_scanline_ = VideoTiming::PAL_CYCLES_PER_SCANLINE;
    }
}

// Rendering helper: Fill scanline with background color
// NOTE: This function is only used by render_scanline() for testing.
// During normal emulation, background rendering happens in render_current_pixel().
// Reference: doc/o2doc.md section 4.9, doc/8245.md lines 440-470
void VDC::render_background(int y) {
    // Get background color from color register (bits 3-5)
    uint8 color_reg = state_.registers[VDCRegisters::COLOR];
    uint8 bg_color = (color_reg >> 3) & 0x07;
    
    // Fill entire scanline with background color
    for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
        state_.framebuffer[y][x] = bg_color;
    }
    
    // Fill extended framebuffer if enabled
    if (extended_fb_mode_ && y < EXTENDED_FB_HEIGHT) {
        for (int x = 0; x < EXTENDED_FB_WIDTH; x++) {
            state_.extended_framebuffer[y][x] = bg_color;
        }
    }
}

// Rendering helper: Render grid elements for scanline
// NOTE: This function is only used by render_scanline() for testing.
// During normal emulation, grid rendering happens via is_grid_pixel_at() in render_current_pixel().
// Reference: doc/o2doc.md section 4.2, doc/8245.md lines 300-350
void VDC::render_grid(int y) {
    // Check if grid is enabled
    if (!state_.grid_enabled) {
        return;
    }
    
    // Get grid color from color register (bits 0-2)
    uint8 color_reg = state_.registers[VDCRegisters::COLOR];
    uint8 grid_color = color_reg & 0x07;
    
    // Check control register for grid modes
    uint8 control = state_.registers[VDCRegisters::CONTROL];
    bool fill_mode = (control & ControlBits::ENABLE_FILL_MODE) != 0;
    bool dot_mode = (control & ControlBits::ENABLE_DOT_GRID) != 0;
    
    // Grid layout: 8 rows and 9 columns
    // Each horizontal bar is 3 scanlines tall, spaced by 21 scanlines
    // First horizontal bar starts at scanline 24 (relative to end of VBLANK)
    // Reference: doc/o2doc.md section 4.2, doc/8245.md lines 300-350
    
    // Calculate grid row (0-8) based on scanline
    // Grid starts at scanline 24, each row is 24 scanlines apart (3 lines + 21 spacing)
    const int GRID_START_Y = 24;
    const int GRID_ROW_HEIGHT = 24;
    const int GRID_LINE_HEIGHT = 3;
    
    // Check if we're on a horizontal grid line
    if (y >= GRID_START_Y) {
        int y_offset = y - GRID_START_Y;
        int grid_row = y_offset / GRID_ROW_HEIGHT;
        int row_offset = y_offset % GRID_ROW_HEIGHT;
        
        // Render horizontal grid lines (9 lines total, including line 9)
        if (grid_row < 9 && row_offset < GRID_LINE_HEIGHT) {
            // Get horizontal grid line data
            uint8 h_line_data;
            if (grid_row < 8) {
                // Lines 0-7 from 0xC0-0xC7
                h_line_data = state_.registers[VDCRegisters::GRID_H_BASE + grid_row];
            } else {
                // Line 8 (9th line) from 0xD0-0xD8, only bit 0 used per column
                h_line_data = 0;
                for (int col = 0; col < 9; col++) {
                    if (state_.registers[VDCRegisters::GRID_H9_BASE + col] & 0x01) {
                        h_line_data |= (1 << col);
                    }
                }
            }
            
            // Render horizontal line segments
            const int GRID_START_X = 10;  // Grid starts at column 10 (10 clock cycles from HBL end)
            const int GRID_COL_WIDTH = 16; // 14 spacing + 2 for vertical line
            
            for (int col = 0; col < 9; col++) {
                if (h_line_data & (1 << col)) {
                    int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                    int x_end = x_start + 14;  // Segment is 14 pixels wide
                    
                    for (int x = x_start; x < x_end && x < FRAMEBUFFER_WIDTH; x++) {
                        state_.framebuffer[y][x] = grid_color;
                        
                        // Write to extended framebuffer if enabled
                        if (extended_fb_mode_ && x < EXTENDED_FB_WIDTH && y < EXTENDED_FB_HEIGHT) {
                            state_.extended_framebuffer[y][x] = grid_color;
                        }
                    }
                }
            }
        }
    }
    
    // Render vertical grid lines (10 lines, columns 0-9)
    // Each vertical bar is 2 or 16 clock intervals wide depending on fill mode
    // Reference: doc/o2doc.md section 4.2
    const int GRID_START_X = 10;
    const int GRID_COL_WIDTH = 16;
    const int VERT_LINE_WIDTH = fill_mode ? 16 : 2;
    
    // Calculate which grid row we're in for vertical line rendering
    if (y >= GRID_START_Y) {
        int y_offset = y - GRID_START_Y;
        int grid_row = y_offset / GRID_ROW_HEIGHT;
        
        if (grid_row < 8) {
            // Render vertical grid lines
            for (int col = 0; col < 10; col++) {
                uint8 v_line_data = state_.registers[VDCRegisters::GRID_V_BASE + col];
                
                if (v_line_data & (1 << grid_row)) {
                    int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                    int x_end = x_start + VERT_LINE_WIDTH;
                    
                    for (int x = x_start; x < x_end && x < FRAMEBUFFER_WIDTH; x++) {
                        state_.framebuffer[y][x] = grid_color;
                        
                        // Write to extended framebuffer if enabled
                        if (extended_fb_mode_ && x < EXTENDED_FB_WIDTH && y < EXTENDED_FB_HEIGHT) {
                            state_.extended_framebuffer[y][x] = grid_color;
                        }
                    }
                }
            }
        }
    }
    
    // Render dot grid if enabled
    // Dots appear at intersections of grid lines
    // Reference: doc/o2doc.md section 4.2
    if (dot_mode && y >= GRID_START_Y) {
        int y_offset = y - GRID_START_Y;
        int grid_row = y_offset / GRID_ROW_HEIGHT;
        int row_offset = y_offset % GRID_ROW_HEIGHT;
        
        // Dots are 3 scanlines tall, 2 pixels wide, at grid intersections
        if (grid_row < 9 && row_offset < 3) {
            for (int col = 0; col < 10; col++) {
                int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                int x_end = x_start + 2;
                
                for (int x = x_start; x < x_end && x < FRAMEBUFFER_WIDTH; x++) {
                    state_.framebuffer[y][x] = grid_color;
                }
            }
        }
    }
}

// Rendering helper: Render characters for scanline
// NOTE: This function is only used by render_scanline() for testing.
// During normal emulation, character rendering happens via is_character_pixel_at() in render_current_pixel().
// Reference: doc/o2doc.md section 4.4-4.5, doc/8245.md lines 200-250
void VDC::render_characters(int y) {
    // Check if display is enabled
    if (!state_.display_enabled) {
        return;
    }
    
    // Render single characters (12 characters, 4 bytes each, starting at 0x10)
    // Reference: doc/o2doc.md section 4.4
    for (int char_num = 0; char_num < 12; char_num++) {
        uint8 base_addr = VDCRegisters::CHAR_BASE + (char_num * 4);
        uint8 char_y = state_.registers[base_addr + 0];
        uint8 char_x = state_.registers[base_addr + 1];
        uint8 char_ptr_low = state_.registers[base_addr + 2];
        uint8 char_attr = state_.registers[base_addr + 3];
        
        // Character visibility bounds checking
        // Requirements 4.1, 4.2, 4.3: Check if character Y position is within visible range
        // In extended framebuffer mode, use extended bounds
        int max_height = extended_fb_mode_ ? EXTENDED_FB_HEIGHT : FRAMEBUFFER_HEIGHT;
        int max_width = extended_fb_mode_ ? EXTENDED_FB_WIDTH : FRAMEBUFFER_WIDTH;
        
        if (char_y >= max_height) {
            continue;  // Character is entirely outside visible/extended area
        }
        
        // Requirements 4.1, 4.2, 4.3: Check if character X position allows at least partial visibility
        // Character is 8 pixels wide, so check if any part is visible
        if (char_x >= max_width) {
            continue;  // Character is entirely to the right of visible/extended area
        }
        
        // Extract character attributes
        uint16 char_ptr = char_ptr_low | ((char_attr & 0x01) << 8);  // 9-bit character pointer
        uint8 color = (char_attr >> 1) & 0x07;  // Bits 1-3: color
        
        // Characters are 8x7 (8 pixels wide, 7 lines tall, but stored as 8 bytes)
        // Check if current scanline intersects this character
        if (y < char_y || y >= char_y + 14) {  // 14 lines (7 rows * 2 scan lines each)
            continue;
        }
        
        // Calculate which row of the character to render
        int char_row = (y - char_y) / 2;  // Each character row spans 2 scanlines
        
        // Get character pattern from ROM
        // The character pointer is a displacement value that must be added to
        // the vertical position to get the ROM address
        // Reference: doc/8245.md section on Major Display System
        // Formula: ROM Address = char_ptr + (char_y / 2) + char_row
        uint16 rom_addr = (char_ptr + (char_y / 2) + char_row) & 0x1FF;  // 9-bit address, wrap at 512
        
        // Requirements 5.2, 11.3: Validate ROM address is within bounds
        if (rom_addr >= 512) {
            continue;  // Invalid ROM address, skip this character
        }
        
        uint8 pattern = character_rom_[rom_addr];
        
        // Render character pixels (8 pixels wide, but only 7 are used)
        for (int x = 0; x < 8; x++) {
            int screen_x = char_x + x;
            
            // Get bit from pattern (bit 7 = leftmost pixel)
            bool pixel_on = (pattern & (0x80 >> x)) != 0;
            
            // Draw pixel if it's on
            if (pixel_on) {
                // Write to normal framebuffer if within bounds
                // Requirements 11.1, 11.4: Per-pixel bounds checking before framebuffer writes
                if (screen_x >= 0 && screen_x < FRAMEBUFFER_WIDTH && y >= 0 && y < FRAMEBUFFER_HEIGHT) {
                    state_.framebuffer[y][screen_x] = color;
                }
                
                // Write to extended framebuffer if enabled and within extended bounds
                if (extended_fb_mode_ && screen_x >= 0 && screen_x < EXTENDED_FB_WIDTH && 
                    y >= 0 && y < EXTENDED_FB_HEIGHT) {
                    state_.extended_framebuffer[y][screen_x] = color;
                }
            }
        }
    }
    
    // Render quad characters (4 groups, 16 bytes each, starting at 0x40)
    // 
    // IMPORTANT: Quad Character Position Register Layout
    // ==================================================
    // Each quad group has 16 bytes (4 characters × 4 bytes each):
    //   Bytes 0-3:   Character 0 (Y, X, pattern_low, color+pattern_high)
    //   Bytes 4-7:   Character 1 (Y, X, pattern_low, color+pattern_high)
    //   Bytes 8-11:  Character 2 (Y, X, pattern_low, color+pattern_high)
    //   Bytes 12-15: Character 3 (Y, X, pattern_low, color+pattern_high)
    //
    // NOTE: doc/o2doc.md says "the X position and Y position of the LAST character
    // sets the position of the whole set", but testing shows Satellite Attack writes
    // position to the FIRST character (bytes 0-1). Using bytes 0-1 for compatibility.
    //
    // Reference: doc/o2doc.md section 4.5
    
    for (int quad_num = 0; quad_num < 4; quad_num++) {
        uint8 base_addr = VDCRegisters::QUAD_BASE + (quad_num * 16);
        
        // Read position from FIRST character (bytes 0-1)
        // This is the Y/X position for the ENTIRE quad group
        uint8 quad_y = state_.registers[base_addr + 0];  // Y of FIRST character
        uint8 quad_x = state_.registers[base_addr + 1];  // X of FIRST character
        
        // Render all 4 characters in the quad
        for (int sub_char = 0; sub_char < 4; sub_char++) {
            uint8 char_offset = sub_char * 4;
            
            // Read pattern and color from each sub-character's bytes
            // Note: Y/X position (bytes 0-1 of each sub-char) are IGNORED by hardware
            // Only the quad's Y/X (from first character) are used
            uint8 char_ptr_low = state_.registers[base_addr + char_offset + 2];
            uint8 char_attr = state_.registers[base_addr + char_offset + 3];
            
            // Calculate actual screen position
            // Y position: All sub-characters share quad_y (from first character)
            // X position: Each sub-character is offset by 16 pixels from quad_x
            // (8 pixels for character + 8 pixels space between, per doc/o2doc.md section 4.5)
            int char_x = quad_x + (sub_char * 16);  // Characters spaced 16 pixels apart
            int char_y = quad_y;  // All sub-characters use the quad's Y position
            
            // Character visibility bounds checking
            // Requirements 4.1, 4.2, 4.3: Check if character position is within visible/extended range
            // In extended framebuffer mode, use extended bounds
            int max_height = extended_fb_mode_ ? EXTENDED_FB_HEIGHT : FRAMEBUFFER_HEIGHT;
            int max_width = extended_fb_mode_ ? EXTENDED_FB_WIDTH : FRAMEBUFFER_WIDTH;
            
            if (char_y >= max_height) {
                continue;  // Character is entirely outside visible/extended area
            }
            
            // Requirements 4.1, 4.2, 4.3: Check if character X position allows at least partial visibility
            if (char_x >= max_width) {
                continue;  // Character is entirely to the right of visible/extended area
            }
            
            // Extract character attributes
            uint16 char_ptr = char_ptr_low | ((char_attr & 0x01) << 8);
            uint8 color = (char_attr >> 1) & 0x07;
            
            // Check if current scanline intersects this character
            if (y < char_y || y >= char_y + 14) {
                continue;
            }
            
            // Calculate which row to render
            int char_row = (y - char_y) / 2;
            
            // Get character pattern from ROM
            // The character pointer is a displacement value
            // Formula: ROM Address = char_ptr + (char_y / 2) + char_row
            uint16 rom_addr = (char_ptr + (char_y / 2) + char_row) & 0x1FF;
           
            // Requirements 5.2, 11.3: Validate ROM address is within bounds
            if (rom_addr >= 512) {
                continue;  // Invalid ROM address, skip this character
            }
            
            uint8 pattern = character_rom_[rom_addr];
            
            // Render character pixels
            for (int x = 0; x < 8; x++) {
                int screen_x = char_x + x;
                
                bool pixel_on = (pattern & (0x80 >> x)) != 0;
                
                if (pixel_on) {
                    // Write to normal framebuffer if within bounds
                    // Requirements 11.1, 11.4: Per-pixel bounds checking before framebuffer writes
                    if (screen_x >= 0 && screen_x < FRAMEBUFFER_WIDTH && y >= 0 && y < FRAMEBUFFER_HEIGHT) {
                        state_.framebuffer[y][screen_x] = color;
                    }
                    
                    // Write to extended framebuffer if enabled and within extended bounds
                    if (extended_fb_mode_ && screen_x >= 0 && screen_x < EXTENDED_FB_WIDTH && 
                        y >= 0 && y < EXTENDED_FB_HEIGHT) {
                        state_.extended_framebuffer[y][screen_x] = color;
                    }
                }
            }
        }
    }
}

// Rendering helper: Render sprites for scanline
// NOTE: This function is only used by render_scanline() for testing.
// During normal emulation, sprite rendering happens via is_sprite_pixel_at() in render_current_pixel().
// Reference: doc/o2doc.md section 4.3, doc/8245.md lines 150-200
void VDC::render_sprites(int y) {
    // Check if display is enabled
    if (!state_.display_enabled) {
        return;
    }
    
    // Render all 4 sprites (in reverse order for proper priority)
    // Sprite 0 has highest priority, so render it last
    for (int sprite_num = 3; sprite_num >= 0; sprite_num--) {
        // Get sprite control registers
        uint8 base_addr = VDCRegisters::SPRITE0_Y + (sprite_num * 4);
        uint8 sprite_y = state_.registers[base_addr + 0];
        uint8 sprite_x = state_.registers[base_addr + 1];
        uint8 sprite_color_attr = state_.registers[base_addr + 2];
        
        // Extract sprite attributes from color register
        // Reference: doc/o2doc.md section 4.3.1
        uint8 color = (sprite_color_attr & SpriteColorBits::COLOR_MASK) >> SpriteColorBits::COLOR_SHIFT;
        bool double_size = (sprite_color_attr & SpriteColorBits::DOUBLE_SIZE) != 0;
        bool shift_even = (sprite_color_attr & SpriteColorBits::SHIFT_EVEN) != 0;
        bool shift_full = (sprite_color_attr & SpriteColorBits::SHIFT_FULL) != 0;
        
        // Calculate sprite height and check if current scanline intersects sprite
        int sprite_height = double_size ? 16 : 8;
        if (y < sprite_y || y >= sprite_y + sprite_height) {
            continue;  // Scanline doesn't intersect this sprite
        }
        
        // Calculate which row of the sprite pattern to render
        int sprite_row = y - sprite_y;
        if (double_size) {
            sprite_row /= 2;  // Each pattern row is rendered twice in double-size mode
        }
        
        // Get sprite pattern byte for this row
        uint8 pattern_addr = VDCRegisters::SPRITE0_PATTERN + (sprite_num * 8) + sprite_row;
        uint8 pattern = state_.registers[pattern_addr];
        
        // Render sprite pixels for this scanline
        int sprite_width = double_size ? 16 : 8;
        for (int x = 0; x < sprite_width; x++) {
            // Calculate pixel position on screen
            int screen_x = sprite_x + x;
            
            // Apply horizontal shift if enabled
            // Note: "even rows" refers to screen rows (y - sprite_y), not pattern rows
            int screen_row = y - sprite_y;
            bool is_even_row = (screen_row & 1) == 0;
            if (shift_full) {
                screen_x += 1;
            } else if (shift_even && is_even_row) {
                screen_x += 1;
            }
            
            // Check if pixel is within framebuffer bounds
            if (screen_x < 0 || screen_x >= FRAMEBUFFER_WIDTH) {
                // Still check extended framebuffer even if outside normal bounds
                if (!extended_fb_mode_ || screen_x < 0 || screen_x >= EXTENDED_FB_WIDTH) {
                    continue;
                }
            }
            
            // Get bit from pattern (bit 7 = leftmost pixel)
            int pattern_x = double_size ? (x / 2) : x;
            bool pixel_on = (pattern & (0x80 >> pattern_x)) != 0;
            
            // Draw pixel if it's on (sprites are transparent where pattern bit is 0)
            if (pixel_on) {
                // Write to normal framebuffer if within bounds
                if (screen_x >= 0 && screen_x < FRAMEBUFFER_WIDTH && y >= 0 && y < FRAMEBUFFER_HEIGHT) {
                    state_.framebuffer[y][screen_x] = color;
                }
                
                // Write to extended framebuffer if enabled and within extended bounds
                if (extended_fb_mode_ && screen_x >= 0 && screen_x < EXTENDED_FB_WIDTH && 
                    y >= 0 && y < EXTENDED_FB_HEIGHT) {
                    state_.extended_framebuffer[y][screen_x] = color;
                }
            }
        }
    }
}

// Collision detection helper
// Reference: doc/o2doc.md section 4.8, doc/8245.md lines 500-520
void VDC::detect_collisions(int y) {
    // Collision detection only works during VBLANK
    // We track collisions during rendering and update the register during VBLANK

    // Get which objects are enabled for collision detection
    uint8 collision_enable = state_.registers[VDCRegisters::COLLISION];

    if (collision_enable == 0) {
        return;  // No collision detection enabled
    }

    // Create a buffer to track which objects are present at each pixel
    // Bit flags: 0=sprite0, 1=sprite1, 2=sprite2, 3=sprite3, 4=vgrid, 5=hgrid, 7=char
    uint8 object_buffer[FRAMEBUFFER_WIDTH];
    std::memset(object_buffer, 0, sizeof(object_buffer));

    // Track grid objects first (lowest priority)
    if (collision_enable & (CollisionBits::VERT_GRID | CollisionBits::HORIZ_GRID)) {
        track_grid_objects(y, object_buffer, collision_enable);
    }

    // Track character objects
    if (collision_enable & CollisionBits::CHARACTERS) {
        track_character_objects(y, object_buffer, collision_enable);
    }

    // Track sprite objects (highest priority)
    for (int sprite_num = 0; sprite_num < 4; sprite_num++) {
        uint8 sprite_bit = (1 << sprite_num);
        if (collision_enable & sprite_bit) {
            track_sprite_object(y, sprite_num, object_buffer, collision_enable);
        }
    }

    // The collision bits are accumulated during the frame
    // They will be read during VBLANK
}

// Audio update helper
// Reference: doc/o2doc.md section 4.10, doc/8245.md lines 380-420
void VDC::update_audio() {
    if (!state_.audio_enabled) {
        return;
    }

    // Calculate how many cycles have passed
    // Audio shift frequency determines how often we shift
    // 983Hz = shift every 1017us, 3933Hz = shift every 254us
    // At 1.79MHz / 5 = 358kHz instruction cycle
    // 983Hz: ~364 cycles per shift, 3933Hz: ~91 cycles per shift

    uint32 cycles_per_shift = (state_.audio_frequency == AUDIO_FREQ_LOW) ? 364 : 91;

    state_.audio_cycle_accumulator += 1;  // Called once per VDC cycle

    while (state_.audio_cycle_accumulator >= cycles_per_shift) {
        state_.audio_cycle_accumulator -= cycles_per_shift;
        shift_audio_register();
    }
}

// Audio shift register helper
// Reference: doc/o2doc.md section 4.10, doc/8245.md lines 380-420
void VDC::shift_audio_register() {
    if (!state_.audio_enabled) {
        return;
    }

    // Get the output bit (bit 0)
    uint8 output_bit = state_.audio_shift_register & 1;

    // Shift right by 1
    state_.audio_shift_register >>= 1;

    // Handle noise mode with XOR feedback
    if (state_.audio_noise) {
        // Noise generation using Linear Feedback Shift Register (LFSR)
        // Creates pseudo-random bit sequence by XORing tap positions
        // XOR feedback: new_bit_23 = old_bit_0 XOR old_bit_1
        // This creates a pseudo-random sequence that sounds like white noise
        // Note: Actual Intel 8245 tap positions are undocumented; this is a
        // reasonable approximation using a 2-tap LFSR configuration
        uint8 feedback_bit = output_bit ^ ((state_.audio_shift_register >> 1) & 1);
        state_.audio_shift_register |= (feedback_bit << 23);
    } else if (state_.audio_loop) {
        // Loop mode: recirculate bit 0 to bit 23
        // Pattern repeats continuously for sustained tones
        state_.audio_shift_register |= (output_bit << 23);
    }

    // Increment shift counter
    state_.audio_shift_counter++;

    // Check if we've shifted all 24 bits
    if (state_.audio_shift_counter >= 24) {
        state_.audio_shift_counter = 0;

        // If not in loop mode and not noise mode, disable audio
        if (!state_.audio_loop && !state_.audio_noise) {
            state_.audio_enabled = false;
            // Set sound needs service bit in status register
            state_.registers[VDCRegisters::STATUS] |= StatusBits::SOUND_NEEDS_SERVICE;
        }
    }
}

// Character ROM data (64 characters, 8 bytes each = 512 bytes total)
// Reference: doc/o2doc.md Appendix C
// Character patterns from Intel 8245 VDC internal ROM
// Source: O2EM emulator character set data
const uint8 VDC::character_rom_[64 * 8] = {
    // Character 0: '0'
    0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,
    // Character 1: '1'
    0x18,0x38,0x18,0x18,0x18,0x18,0x3C,0x00,
    // Character 2: '2'
    0x3C,0x66,0x0C,0x18,0x30,0x60,0x7E,0x00,
    // Character 3: '3'
    0x7C,0xC6,0x06,0x3C,0x06,0xC6,0x7C,0x00,
    // Character 4: '4'
    0xCC,0xCC,0xCC,0xFE,0x0C,0x0C,0x0C,0x00,
    // Character 5: '5'
    0xFE,0xC0,0xC0,0x7C,0x06,0xC6,0x7C,0x00,
    // Character 6: '6'
    0x7C,0xC6,0xC0,0xFC,0xC6,0xC6,0x7C,0x00,
    // Character 7: '7'
    0xFE,0x06,0x0C,0x18,0x30,0x60,0xC0,0x00,
    // Character 8: '8'
    0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0x7C,0x00,
    // Character 9: '9'
    0x7C,0xC6,0xC6,0x7E,0x06,0xC6,0x7C,0x00,
    // Character 10: ':'
    0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00,
    // Character 11: '$'
    0x18,0x7E,0x58,0x7E,0x1A,0x7E,0x18,0x00,
    // Character 12: (blank)
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    // Character 13: '?'
    0x3C,0x66,0x0C,0x18,0x18,0x00,0x18,0x00,
    // Character 14: 'L'
    0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xFE,0x00,
    // Character 15: 'P'
    0xFC,0xC6,0xC6,0xFC,0xC0,0xC0,0xC0,0x00,
    // Character 16: '+'
    0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00,
    // Character 17: 'W'
    0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00,
    // Character 18: 'E'
    0xFE,0xC0,0xC0,0xF8,0xC0,0xC0,0xFE,0x00,
    // Character 19: 'R'
    0xFC,0xC6,0xC6,0xFC,0xD8,0xCC,0xC6,0x00,
    // Character 20: 'T'
    0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00,
    // Character 21: 'U'
    0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,
    // Character 22: 'I'
    0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,
    // Character 23: 'O'
    0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,
    // Character 24: 'Q'
    0x7C,0xC6,0xC6,0xC6,0xDE,0xCC,0x76,0x00,
    // Character 25: 'S'
    0x7C,0xC6,0xC0,0x7C,0x06,0xC6,0x7C,0x00,
    // Character 26: 'D'
    0xFC,0xC6,0xC6,0xC6,0xC6,0xC6,0xFC,0x00,
    // Character 27: 'F'
    0xFE,0xC0,0xC0,0xF8,0xC0,0xC0,0xC0,0x00,
    // Character 28: 'G'
    0x7C,0xC6,0xC0,0xC0,0xCE,0xC6,0x7E,0x00,
    // Character 29: 'H'
    0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00,
    // Character 30: 'J'
    0x06,0x06,0x06,0x06,0x06,0xC6,0x7C,0x00,
    // Character 31: 'K'
    0xC6,0xCC,0xD8,0xF0,0xD8,0xCC,0xC6,0x00,
    // Character 32: 'A'
    0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00,
    // Character 33: 'Z'
    0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00,
    // Character 34: 'X'
    0xC6,0xC6,0x6C,0x38,0x6C,0xC6,0xC6,0x00,
    // Character 35: 'C'
    0x7C,0xC6,0xC0,0xC0,0xC0,0xC6,0x7C,0x00,
    // Character 36: 'V'
    0xC6,0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x00,
    // Character 37: 'B'
    0xFC,0xC6,0xC6,0xFC,0xC6,0xC6,0xFC,0x00,
    // Character 38: 'M'
    0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00,
    // Character 39: '.'
    0x00,0x00,0x00,0x00,0x00,0x38,0x38,0x00,
    // Character 40: '-'
    0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,
    // Character 41: 'x' (multiply)
    0x00,0x66,0x3C,0x18,0x3C,0x66,0x00,0x00,
    // Character 42: '÷' (divide)
    0x00,0x18,0x00,0x7E,0x00,0x18,0x00,0x00,
    // Character 43: '='
    0x00,0x00,0x7C,0x00,0x7C,0x00,0x00,0x00,
    // Character 44: 'Y'
    0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00,
    // Character 45: 'N'
    0xC6,0xE6,0xF6,0xFE,0xDE,0xCE,0xC6,0x00,
    // Character 46: '/'
    0x03,0x06,0x0C,0x18,0x30,0x60,0xC0,0x00,
    // Character 47: block (solid)
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
    // Character 48: '10' (special)
    0xCE,0xDB,0xDB,0xDB,0xDB,0xDB,0xCE,0x00,
    // Character 49: ball
    0x00,0x00,0x3C,0x7E,0x7E,0x7E,0x3C,0x00,
    // Character 50: man right
    0x1C,0x1C,0x18,0x1E,0x18,0x18,0x1C,0x00,
    // Character 51: man right walk
    0x1C,0x1C,0x18,0x1E,0x18,0x34,0x26,0x00,
    // Character 52: man left walk
    0x38,0x38,0x18,0x78,0x18,0x2C,0x64,0x00,
    // Character 53: man left
    0x38,0x38,0x18,0x78,0x18,0x18,0x38,0x00,
    // Character 54: arrow right
    0x00,0x18,0x0C,0xFE,0x0C,0x18,0x00,0x00,
    // Character 55: tree
    0x18,0x3C,0x7E,0xFF,0xFF,0x18,0x18,0x00,
    // Character 56: slope left
    0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF,0x00,
    // Character 57: slope right
    0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF,0x00,
    // Character 58: man forward
    0x38,0x38,0x12,0xFE,0xB8,0x28,0x6C,0x00,
    // Character 59: '\'
    0xC0,0x60,0x30,0x18,0x0C,0x06,0x03,0x00,
    // Character 60: ship 1
    0x00,0x00,0x0C,0x08,0x08,0xFF,0x7E,0x00,
    // Character 61: plane
    0x00,0x03,0x63,0xFF,0xFF,0x18,0x08,0x00,
    // Character 62: ship 2
    0x00,0x00,0x00,0x10,0x38,0xFF,0x7E,0x00,
    // Character 63: ship 3
    0x00,0x00,0x00,0x06,0x6E,0xFF,0x7E,0x00
};
// Collision tracking helper: Track grid objects
// Reference: doc/o2doc.md section 4.8
void VDC::track_grid_objects(int y, uint8* object_buffer, uint8 collision_enable) {
    if (!state_.grid_enabled) {
        return;
    }

    uint8 control = state_.registers[VDCRegisters::CONTROL];
    bool fill_mode = (control & ControlBits::ENABLE_FILL_MODE) != 0;
    bool dot_mode = (control & ControlBits::ENABLE_DOT_GRID) != 0;

    const int GRID_START_Y = 24;
    const int GRID_ROW_HEIGHT = 24;
    const int GRID_LINE_HEIGHT = 3;
    const int GRID_START_X = 10;
    const int GRID_COL_WIDTH = 16;
    const int VERT_LINE_WIDTH = fill_mode ? 16 : 2;

    // Track horizontal grid lines
    if ((collision_enable & CollisionBits::HORIZ_GRID) && y >= GRID_START_Y) {
        int y_offset = y - GRID_START_Y;
        int grid_row = y_offset / GRID_ROW_HEIGHT;
        int row_offset = y_offset % GRID_ROW_HEIGHT;

        if (grid_row < 9 && row_offset < GRID_LINE_HEIGHT) {
            uint8 h_line_data;
            if (grid_row < 8) {
                h_line_data = state_.registers[VDCRegisters::GRID_H_BASE + grid_row];
            } else {
                h_line_data = 0;
                for (int col = 0; col < 9; col++) {
                    if (state_.registers[VDCRegisters::GRID_H9_BASE + col] & 0x01) {
                        h_line_data |= (1 << col);
                    }
                }
            }

            for (int col = 0; col < 9; col++) {
                if (h_line_data & (1 << col)) {
                    int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                    int x_end = x_start + 14;

                    for (int x = x_start; x < x_end && x < FRAMEBUFFER_WIDTH; x++) {
                        // Check for collision with existing objects
                        if (object_buffer[x] != 0) {
                            state_.collision_state |= CollisionBits::HORIZ_GRID;
                            state_.collision_detected = true;
                        }
                        object_buffer[x] |= CollisionBits::HORIZ_GRID;
                    }
                }
            }
        }
    }

    // Track vertical grid lines
    if ((collision_enable & CollisionBits::VERT_GRID) && y >= GRID_START_Y) {
        int y_offset = y - GRID_START_Y;
        int grid_row = y_offset / GRID_ROW_HEIGHT;

        if (grid_row < 8) {
            for (int col = 0; col < 10; col++) {
                uint8 v_line_data = state_.registers[VDCRegisters::GRID_V_BASE + col];

                if (v_line_data & (1 << grid_row)) {
                    int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                    int x_end = x_start + VERT_LINE_WIDTH;

                    for (int x = x_start; x < x_end && x < FRAMEBUFFER_WIDTH; x++) {
                        if (object_buffer[x] != 0) {
                            state_.collision_state |= CollisionBits::VERT_GRID;
                            state_.collision_detected = true;
                        }
                        object_buffer[x] |= CollisionBits::VERT_GRID;
                    }
                }
            }
        }
    }

    // Track dot grid
    if ((collision_enable & CollisionBits::HORIZ_GRID) && dot_mode && y >= GRID_START_Y) {
        int y_offset = y - GRID_START_Y;
        int grid_row = y_offset / GRID_ROW_HEIGHT;
        int row_offset = y_offset % GRID_ROW_HEIGHT;

        if (grid_row < 9 && row_offset < 3) {
            for (int col = 0; col < 10; col++) {
                int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                int x_end = x_start + 2;

                for (int x = x_start; x < x_end && x < FRAMEBUFFER_WIDTH; x++) {
                    if (object_buffer[x] != 0) {
                        state_.collision_state |= CollisionBits::HORIZ_GRID;
                        state_.collision_detected = true;
                    }
                    object_buffer[x] |= CollisionBits::HORIZ_GRID;
                }
            }
        }
    }
}

// Collision tracking helper: Track character objects
// Reference: doc/o2doc.md section 4.8
void VDC::track_character_objects(int y, uint8* object_buffer, uint8 /* collision_enable */) {
    if (!state_.display_enabled) {
        return;
    }

    bool char_to_char_collision = false;

    // Track single characters
    for (int char_num = 0; char_num < 12; char_num++) {
        uint8 base_addr = VDCRegisters::CHAR_BASE + (char_num * 4);
        uint8 char_y = state_.registers[base_addr + 0];
        uint8 char_x = state_.registers[base_addr + 1];
        uint8 char_ptr_low = state_.registers[base_addr + 2];
        uint8 char_attr = state_.registers[base_addr + 3];

        if (y < char_y || y >= char_y + 14) {
            continue;
        }

        int char_row = (y - char_y) / 2;
        uint16 char_ptr = char_ptr_low | ((char_attr & 0x01) << 8);
        uint16 rom_addr = (char_ptr + (char_y / 2) + char_row) & 0x1FF;

        if (rom_addr >= 512) {
            continue;
        }

        uint8 pattern = character_rom_[rom_addr];

        for (int x = 0; x < 8; x++) {
            int screen_x = char_x + x;

            if (screen_x < 0 || screen_x >= FRAMEBUFFER_WIDTH) {
                continue;
            }

            bool pixel_on = (pattern & (0x80 >> x)) != 0;

            if (pixel_on) {
                // Check for collision with existing objects
                if (object_buffer[screen_x] != 0) {
                    state_.collision_state |= CollisionBits::CHARACTERS;
                    state_.collision_detected = true;

                    // Check for character-to-character collision
                    if (object_buffer[screen_x] & CollisionBits::CHARACTERS) {
                        char_to_char_collision = true;
                    }
                }
                object_buffer[screen_x] |= CollisionBits::CHARACTERS;
            }
        }
    }

    // Track quad characters
    // See render_characters() for detailed explanation of quad character layout
    // IMPORTANT: First character's Y/X (bytes 0-1) control entire quad group
    for (int quad_num = 0; quad_num < 4; quad_num++) {
        uint8 base_addr = VDCRegisters::QUAD_BASE + (quad_num * 16);
        uint8 quad_y = state_.registers[base_addr + 0];  // Y of 1st character (controls entire quad)
        uint8 quad_x = state_.registers[base_addr + 1];  // X of 1st character (controls entire quad)

        for (int sub_char = 0; sub_char < 4; sub_char++) {
            uint8 char_offset = sub_char * 4;
            // Y/X from sub-character bytes are ignored; only pattern and color are used
            uint8 char_ptr_low = state_.registers[base_addr + char_offset + 2];
            uint8 char_attr = state_.registers[base_addr + char_offset + 3];

            int char_x = quad_x + (sub_char * 8);
            int char_y = quad_y;  // All sub-characters use the quad's Y position

            if (y < char_y || y >= char_y + 14) {
                continue;
            }

            int char_row = (y - char_y) / 2;
            uint16 char_ptr = char_ptr_low | ((char_attr & 0x01) << 8);
            uint16 rom_addr = (char_ptr + (char_y / 2) + char_row) & 0x1FF;

            if (rom_addr >= 512) {
                continue;
            }

            uint8 pattern = character_rom_[rom_addr];

            for (int x = 0; x < 8; x++) {
                int screen_x = char_x + x;

                if (screen_x < 0 || screen_x >= FRAMEBUFFER_WIDTH) {
                    continue;
                }

                bool pixel_on = (pattern & (0x80 >> x)) != 0;

                if (pixel_on) {
                    if (object_buffer[screen_x] != 0) {
                        state_.collision_state |= CollisionBits::CHARACTERS;
                        state_.collision_detected = true;

                        if (object_buffer[screen_x] & CollisionBits::CHARACTERS) {
                            char_to_char_collision = true;
                        }
                    }
                    object_buffer[screen_x] |= CollisionBits::CHARACTERS;
                }
            }
        }
    }

    // Set status register bit 7 for character-to-character collisions
    // Reference: doc/o2doc.md section 4.7
    if (char_to_char_collision) {
        state_.registers[VDCRegisters::STATUS] |= StatusBits::CHAR_OVERLAP;
    }
}

// Collision tracking helper: Track sprite object
// Reference: doc/o2doc.md section 4.8
void VDC::track_sprite_object(int y, int sprite_num, uint8* object_buffer, uint8 /* collision_enable */) {
    if (!state_.display_enabled) {
        return;
    }

    uint8 base_addr = VDCRegisters::SPRITE0_Y + (sprite_num * 4);
    uint8 sprite_y = state_.registers[base_addr + 0];
    uint8 sprite_x = state_.registers[base_addr + 1];
    uint8 sprite_color_attr = state_.registers[base_addr + 2];

    bool double_size = (sprite_color_attr & SpriteColorBits::DOUBLE_SIZE) != 0;
    bool shift_even = (sprite_color_attr & SpriteColorBits::SHIFT_EVEN) != 0;
    bool shift_full = (sprite_color_attr & SpriteColorBits::SHIFT_FULL) != 0;

    int sprite_height = double_size ? 16 : 8;
    if (y < sprite_y || y >= sprite_y + sprite_height) {
        return;
    }

    int sprite_row = y - sprite_y;
    if (double_size) {
        sprite_row /= 2;
    }

    uint8 pattern_addr = VDCRegisters::SPRITE0_PATTERN + (sprite_num * 8) + sprite_row;
    uint8 pattern = state_.registers[pattern_addr];

    uint8 sprite_bit = (1 << sprite_num);
    int sprite_width = double_size ? 16 : 8;

    for (int x = 0; x < sprite_width; x++) {
        int screen_x = sprite_x + x;

        // Apply shift based on screen row, not pattern row
        int screen_row = y - sprite_y;
        bool is_even_row = (screen_row & 1) == 0;
        if (shift_full) {
            screen_x += 1;
        } else if (shift_even && is_even_row) {
            screen_x += 1;
        }

        if (screen_x < 0 || screen_x >= FRAMEBUFFER_WIDTH) {
            continue;
        }

        int pattern_x = double_size ? (x / 2) : x;
        bool pixel_on = (pattern & (0x80 >> pattern_x)) != 0;

        if (pixel_on) {
            // Check for collision with existing objects
            if (object_buffer[screen_x] != 0) {
                // Set collision bit for current sprite
                state_.collision_state |= sprite_bit;
                state_.collision_detected = true;
                
                // Also set collision bits for any other objects at this pixel
                // This ensures both objects in a collision get their bits set
                
                // Check for other sprites
                for (int other_sprite = 0; other_sprite < 4; other_sprite++) {
                    uint8 other_bit = (1 << other_sprite);
                    if (object_buffer[screen_x] & other_bit) {
                        state_.collision_state |= other_bit;
                    }
                }
                
                // Check for characters
                if (object_buffer[screen_x] & CollisionBits::CHARACTERS) {
                    state_.collision_state |= CollisionBits::CHARACTERS;
                }
                
                // Check for grid objects
                if (object_buffer[screen_x] & CollisionBits::VERT_GRID) {
                    state_.collision_state |= CollisionBits::VERT_GRID;
                }
                if (object_buffer[screen_x] & CollisionBits::HORIZ_GRID) {
                    state_.collision_state |= CollisionBits::HORIZ_GRID;
                }
            }
            object_buffer[screen_x] |= sprite_bit;
        }
    }
}

// Debug helper: Dump VDC registers to console
void VDC::dump_registers() const {
    std::cout << "\n=== VDC Register Dump ===" << std::endl;
    std::cout << "Control (0xA0): 0x" << std::hex << static_cast<int>(state_.registers[0xA0]) << std::dec;
    std::cout << " [Display:" << (state_.display_enabled ? "ON" : "OFF");
    std::cout << " Grid:" << (state_.grid_enabled ? "ON" : "OFF") << "]" << std::endl;
    
    std::cout << "Status (0xA1): 0x" << std::hex << static_cast<int>(state_.registers[0xA1]) << std::dec << std::endl;
    std::cout << "Collision (0xA2): 0x" << std::hex << static_cast<int>(state_.registers[0xA2]) << std::dec << std::endl;
    std::cout << "Color (0xA3): 0x" << std::hex << static_cast<int>(state_.registers[0xA3]) << std::dec;
    std::cout << " (palette index " << (state_.registers[0xA3] & 0x07) << ")" << std::endl;
    
    std::cout << "\nSprite 0 (Player):" << std::endl;
    std::cout << "  Y: " << static_cast<int>(state_.registers[0x00]) << std::endl;
    std::cout << "  X: " << static_cast<int>(state_.registers[0x01]) << std::endl;
    std::cout << "  Color/Attr: 0x" << std::hex << static_cast<int>(state_.registers[0x02]) << std::dec;
    bool double_size = (state_.registers[0x02] & 0x04) != 0;
    uint8 color = (state_.registers[0x02] >> 3) & 0x07;
    std::cout << " [double_size=" << double_size << " color=" << (int)color << "]" << std::endl;
    
    std::cout << "\nCharacters (first 4):" << std::endl;
    for (int i = 0; i < 4; i++) {
        int base = 0x10 + (i * 4);
        std::cout << "  Char " << i << ": X=" << static_cast<int>(state_.registers[base + 1]);
        std::cout << " Y=" << static_cast<int>(state_.registers[base + 2]);
        std::cout << " Attr=0x" << std::hex << static_cast<int>(state_.registers[base]) << std::dec << std::endl;
    }
    
    std::cout << "\nAudio:" << std::endl;
    std::cout << "  Control (0xAA): 0x" << std::hex << static_cast<int>(state_.registers[0xAA]) << std::dec;
    std::cout << " [Enabled:" << (state_.audio_enabled ? "YES" : "NO");
    std::cout << " Volume:" << static_cast<int>(state_.audio_volume) << "]" << std::endl;
    
    std::cout << "\nTiming:" << std::endl;
    std::cout << "  Beam Position: (" << state_.beam_x << ", " << state_.beam_y << ")" << std::endl;
    std::cout << "  Total Scanlines: " << total_scanlines_ << std::endl;
    std::cout << "  VBLANK: " << (is_vblank() ? "YES" : "NO") << std::endl;
    std::cout << "========================\n" << std::endl;
}

// Per-pixel rendering helper: Check if grid pixel exists at position
// Reference: Requirements 12.1, 12.2
bool VDC::is_grid_pixel_at(int x, int y) const {
    if (!state_.grid_enabled) {
        return false;
    }
    
    uint8 control = state_.registers[VDCRegisters::CONTROL];
    bool fill_mode = (control & ControlBits::ENABLE_FILL_MODE) != 0;
    bool dot_mode = (control & ControlBits::ENABLE_DOT_GRID) != 0;
    
    const int GRID_START_Y = 24;
    const int GRID_ROW_HEIGHT = 24;
    const int GRID_LINE_HEIGHT = 3;
    const int GRID_START_X = 10;
    const int GRID_COL_WIDTH = 16;
    const int VERT_LINE_WIDTH = fill_mode ? 16 : 2;
    
    if (y < GRID_START_Y) {
        return false;
    }
    
    int y_offset = y - GRID_START_Y;
    int grid_row = y_offset / GRID_ROW_HEIGHT;
    int row_offset = y_offset % GRID_ROW_HEIGHT;
    
    // Check horizontal grid lines
    if (grid_row < 9 && row_offset < GRID_LINE_HEIGHT) {
        uint8 h_line_data;
        if (grid_row < 8) {
            h_line_data = state_.registers[VDCRegisters::GRID_H_BASE + grid_row];
        } else {
            h_line_data = 0;
            for (int col = 0; col < 9; col++) {
                if (state_.registers[VDCRegisters::GRID_H9_BASE + col] & 0x01) {
                    h_line_data |= (1 << col);
                }
            }
        }
        
        for (int col = 0; col < 9; col++) {
            if (h_line_data & (1 << col)) {
                int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                int x_end = x_start + 14;
                
                if (x >= x_start && x < x_end) {
                    return true;
                }
            }
        }
    }
    
    // Check vertical grid lines
    if (grid_row < 8) {
        for (int col = 0; col < 10; col++) {
            uint8 v_line_data = state_.registers[VDCRegisters::GRID_V_BASE + col];
            
            if (v_line_data & (1 << grid_row)) {
                int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                int x_end = x_start + VERT_LINE_WIDTH;
                
                if (x >= x_start && x < x_end) {
                    return true;
                }
            }
        }
    }
    
    // Check dot grid
    if (dot_mode && grid_row < 9 && row_offset < 3) {
        for (int col = 0; col < 10; col++) {
            int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
            int x_end = x_start + 2;
            
            if (x >= x_start && x < x_end) {
                return true;
            }
        }
    }
    
    return false;
}

// Per-pixel rendering helper: Check if character pixel exists at position
// Reference: Requirements 12.2, 12.3
bool VDC::is_character_pixel_at(int x, int y, uint8& color) const {
    if (!state_.display_enabled) {
        return false;
    }
    
    // Check single characters (12 characters, 4 bytes each, starting at 0x10)
    for (int char_num = 0; char_num < 12; char_num++) {
        uint8 base_addr = VDCRegisters::CHAR_BASE + (char_num * 4);
        uint8 char_y = state_.registers[base_addr + 0];
        uint8 char_x = state_.registers[base_addr + 1];
        uint8 char_ptr_low = state_.registers[base_addr + 2];
        uint8 char_attr = state_.registers[base_addr + 3];
        
        // Check if pixel is within character bounds
        // Calculate how many rows to render based on o2em logic
        int ypos_half = char_y / 2;
        int n = 8 - (ypos_half % 8) - (char_ptr_low % 8);
        if (n < 3) {
            n = n + 7;
        }
        
        // Character renders for n rows (each row is 2 scanlines)
        int char_height = n * 2;
        if (y < char_y || y >= char_y + char_height) {
            continue;
        }
        
        if (x < char_x || x >= char_x + 8) {
            continue;
        }
        
        // Calculate which row of the character to check
        int char_row = (y - char_y) / 2;
        
        // Get character pattern from ROM
        uint16 char_ptr = char_ptr_low | ((char_attr & 0x01) << 8);
        uint16 rom_addr = (char_ptr + (char_y / 2) + char_row) & 0x1FF;
        
        if (rom_addr >= 512) {
            continue;
        }
        
        uint8 pattern = character_rom_[rom_addr];
        
        // Check if pixel is on
        int pixel_x = x - char_x;
        bool pixel_on = (pattern & (0x80 >> pixel_x)) != 0;
        
        if (pixel_on) {
            color = (char_attr >> 1) & 0x07;
            return true;
        }
    }
    
    // Check quad characters (4 groups, 16 bytes each, starting at 0x40)
    // See render_characters() for detailed explanation of quad character layout
    // IMPORTANT: Testing shows game writes position to FIRST character (bytes 0-1), not LAST
    for (int quad_num = 0; quad_num < 4; quad_num++) {
        uint8 base_addr = VDCRegisters::QUAD_BASE + (quad_num * 16);
        
        // Use FIRST character position (bytes 0-1)
        uint8 quad_y = state_.registers[base_addr + 0];
        uint8 quad_x = state_.registers[base_addr + 1];
        
        for (int sub_char = 0; sub_char < 4; sub_char++) {
            uint8 char_offset = sub_char * 4;
            // Y/X from sub-character bytes are ignored; only pattern and color are used
            uint8 char_ptr_low = state_.registers[base_addr + char_offset + 2];
            uint8 char_attr = state_.registers[base_addr + char_offset + 3];
            
            int char_x = quad_x + (sub_char * 16);  // 8 pixels character + 8 pixels space
            int char_y = quad_y;  // All sub-characters use the quad's Y position
            
            // Calculate how many rows to render based on o2em logic
            int ypos_half = char_y / 2;
            int n = 8 - (ypos_half % 8) - (char_ptr_low % 8);
            if (n < 3) {
                n = n + 7;
            }
            
            // Character renders for n rows (each row is 2 scanlines)
            int char_height = n * 2;
            
            // Check if pixel is within character bounds
            if (y < char_y || y >= char_y + char_height) {
                continue;
            }
            
            if (x < char_x || x >= char_x + 8) {
                continue;
            }
            
            // Calculate which row to check
            int char_row = (y - char_y) / 2;
            
            // Get character pattern from ROM
            uint16 char_ptr = char_ptr_low | ((char_attr & 0x01) << 8);
            uint16 rom_addr = (char_ptr + (char_y / 2) + char_row) & 0x1FF;
            
            if (rom_addr >= 512) {
                continue;
            }
            
            uint8 pattern = character_rom_[rom_addr];
            
            // Check if pixel is on
            int pixel_x = x - char_x;
            bool pixel_on = (pattern & (0x80 >> pixel_x)) != 0;
            
            if (pixel_on) {
                color = (char_attr >> 1) & 0x07;
                return true;
            }
        }
    }
    
    return false;
}

// Per-pixel rendering helper: Check if sprite pixel exists at position
// Reference: Requirements 12.3, 12.4
bool VDC::is_sprite_pixel_at(int x, int y, uint8& color) const {
    if (!state_.display_enabled) {
        return false;
    }
    
    // Check all 4 sprites (in reverse order for proper priority)
    // Sprite 0 has highest priority, so check it last
    for (int sprite_num = 3; sprite_num >= 0; sprite_num--) {
        uint8 base_addr = VDCRegisters::SPRITE0_Y + (sprite_num * 4);
        uint8 sprite_y = state_.registers[base_addr + 0];
        uint8 sprite_x = state_.registers[base_addr + 1];
        uint8 sprite_color_attr = state_.registers[base_addr + 2];
        
        // Extract sprite attributes
        uint8 sprite_color = (sprite_color_attr & SpriteColorBits::COLOR_MASK) >> SpriteColorBits::COLOR_SHIFT;
        bool double_size = (sprite_color_attr & SpriteColorBits::DOUBLE_SIZE) != 0;
        bool shift_even = (sprite_color_attr & SpriteColorBits::SHIFT_EVEN) != 0;
        bool shift_full = (sprite_color_attr & SpriteColorBits::SHIFT_FULL) != 0;
        
        // Calculate sprite height and check if pixel is within sprite bounds
        int sprite_height = double_size ? 16 : 8;
        if (y < sprite_y || y >= sprite_y + sprite_height) {
            continue;
        }
        
        // Calculate which row of the sprite pattern to check
        int sprite_row = y - sprite_y;
        if (double_size) {
            sprite_row /= 2;
        }
        
        // Get sprite pattern byte for this row
        uint8 pattern_addr = VDCRegisters::SPRITE0_PATTERN + (sprite_num * 8) + sprite_row;
        uint8 pattern = state_.registers[pattern_addr];
        
        // Calculate pixel position with horizontal shift
        int pixel_x = x - sprite_x;
        
        // Apply shift based on screen row, not pattern row
        int screen_row = y - sprite_y;
        bool is_even_row = (screen_row & 1) == 0;
        if (shift_full) {
            pixel_x -= 1;
        } else if (shift_even && is_even_row) {
            pixel_x -= 1;
        }
        
        // Check if pixel is within sprite width
        int sprite_width = double_size ? 16 : 8;
        if (pixel_x < 0 || pixel_x >= sprite_width) {
            continue;
        }
        
        // Get bit from pattern
        int pattern_x = double_size ? (pixel_x / 2) : pixel_x;
        bool pixel_on = (pattern & (0x80 >> pattern_x)) != 0;
        
        if (pixel_on) {
            color = sprite_color;
            return true;
        }
    }
    
    return false;
}

} // namespace videopac
