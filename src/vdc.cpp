#include "vdc.h"
#include <cstring>

namespace videopac {

// Constructor
// Reference: doc/o2doc.md section 4.11, doc/8245.md lines 520-560
VDC::VDC(VideoStandard standard) {
    state_.video_standard = standard;
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
    
    // Reset timing state
    state_.scanline = 0;
    state_.beam_x = 0;
    state_.beam_y = 0;
    state_.cycle_counter = 0;
    
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
    state_.cycle_counter += cycles;
    
    // Check if we've completed a scanline
    if (state_.cycle_counter >= cycles_per_scanline_) {
        state_.cycle_counter -= cycles_per_scanline_;
        state_.scanline++;
        
        // Wrap scanline counter at end of frame
        if (state_.scanline >= total_scanlines_) {
            state_.scanline = 0;
        }
        
        // Update beam position for current scanline
        if (!is_vblank()) {
            state_.beam_y = static_cast<uint8>(state_.scanline);
        }
    }
    
    // Update horizontal beam position based on cycles within scanline
    state_.beam_x = static_cast<uint8>((state_.cycle_counter * FRAMEBUFFER_WIDTH) / cycles_per_scanline_);
    
    // Update beam position registers if not latched
    // Reference: doc/o2doc.md section 4.14, doc/8245.md lines 440-470
    if (!(state_.registers[VDCRegisters::CONTROL] & ControlBits::LATCH_BEAM_POS)) {
        state_.registers[VDCRegisters::BEAM_Y] = state_.beam_y;
        state_.registers[VDCRegisters::BEAM_X] = state_.beam_x;
    }
    
    // Update status register
    // Reference: doc/o2doc.md section 4.7, doc/8245.md lines 480-500
    uint8 status = 0;
    
    // HBLANK status (simplified - active during last portion of scanline)
    if (state_.cycle_counter > (cycles_per_scanline_ * 3 / 4)) {
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
}

// Write to VDC register
// Reference: doc/o2doc.md Appendix D, doc/8245.md lines 600-650
void VDC::write_register(uint8 address, uint8 value) {
    state_.registers[address] = value;
    
    // Handle special registers that update internal state
    switch (address) {
        case VDCRegisters::CONTROL:
            // Control register - update display and grid enable flags
            // Reference: doc/o2doc.md section 4.6, doc/8245.md lines 440-470
            state_.display_enabled = (value & ControlBits::ENABLE_DISPLAY) != 0;
            state_.grid_enabled = (value & ControlBits::ENABLE_GRID) != 0;
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
            // Reading collision register clears it
            // Reference: doc/o2doc.md section 4.8, doc/8245.md lines 500-520
            {
                uint8 collision = state_.registers[address];
                state_.registers[address] = 0;
                state_.collision_detected = false;
                return collision;
            }
            
        default:
            return state_.registers[address];
    }
}

// Render current scanline to framebuffer
// Reference: doc/o2doc.md section 4.0, doc/8245.md lines 200-300
void VDC::render_scanline() {
    // Only render during visible scanlines (not in VBLANK)
    if (is_vblank() || state_.scanline >= FRAMEBUFFER_HEIGHT) {
        return;
    }
    
    // Check if display is enabled
    if (!state_.display_enabled) {
        return;
    }
    
    int y = static_cast<int>(state_.scanline);
    
    // Render in priority order (background to foreground)
    // Reference: doc/o2doc.md section 4.0, design.md Property 21
    render_background(y);
    render_grid(y);
    render_characters(y);
    render_sprites(y);
    
    // Detect collisions for this scanline
    detect_collisions(y);
}

// Get framebuffer pointer
const uint8* VDC::get_framebuffer() const {
    return &state_.framebuffer[0][0];
}

// Check if in vertical blank period
// Reference: doc/o2doc.md section 4.11, doc/8245.md lines 520-560
bool VDC::is_vblank() const {
    return state_.scanline >= vblank_start_;
}

// Check if in horizontal blank period
// Reference: doc/o2doc.md section 4.11, doc/8245.md lines 520-560
bool VDC::is_hblank() const {
    // HBLANK is active during last portion of scanline
    return state_.cycle_counter > (cycles_per_scanline_ * 3 / 4);
}

// Get current audio sample
// Reference: doc/o2doc.md section 4.10, doc/8245.md lines 380-420
int16 VDC::get_audio_sample() {
    if (!state_.audio_enabled) {
        return 0;
    }
    
    // Get current bit from shift register
    uint8 bit = (state_.audio_shift_register >> state_.audio_shift_counter) & 1;
    
    // Scale by volume (0-15 maps to 0-32767)
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
// Reference: doc/o2doc.md section 4.9, doc/8245.md lines 440-470
void VDC::render_background(int y) {
    // Get background color from color register (bits 3-5)
    uint8 color_reg = state_.registers[VDCRegisters::COLOR];
    uint8 bg_color = (color_reg >> 3) & 0x07;
    
    // Fill entire scanline with background color
    for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
        state_.framebuffer[y][x] = bg_color;
    }
}

// Rendering helper: Render grid elements for scanline
// Reference: doc/o2doc.md section 4.2, doc/8245.md lines 300-350
void VDC::render_grid(int y) {
    // Grid rendering will be implemented in task 6.6
    (void)y;
}

// Rendering helper: Render characters for scanline
// Reference: doc/o2doc.md section 4.4-4.5, doc/8245.md lines 200-250
void VDC::render_characters(int y) {
    // Character rendering will be implemented in task 6.5
    (void)y;
}

// Rendering helper: Render sprites for scanline
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
            bool is_even_row = (sprite_row & 1) == 0;
            if (shift_full) {
                screen_x += 1;
            } else if (shift_even && is_even_row) {
                screen_x += 1;
            }
            
            // Check if pixel is within framebuffer bounds
            if (screen_x < 0 || screen_x >= FRAMEBUFFER_WIDTH) {
                continue;
            }
            
            // Get bit from pattern (bit 7 = leftmost pixel)
            int pattern_x = double_size ? (x / 2) : x;
            bool pixel_on = (pattern & (0x80 >> pattern_x)) != 0;
            
            // Draw pixel if it's on (sprites are transparent where pattern bit is 0)
            if (pixel_on) {
                state_.framebuffer[y][screen_x] = color;
            }
        }
    }
}

// Collision detection helper
// Reference: doc/o2doc.md section 4.8, doc/8245.md lines 500-520
void VDC::detect_collisions(int y) {
    // Collision detection will be implemented in task 6.8
    (void)y;
}

// Audio update helper
// Reference: doc/o2doc.md section 4.10, doc/8245.md lines 380-420
void VDC::update_audio() {
    // Audio update will be implemented in task 6.9
}

// Audio shift register helper
// Reference: doc/o2doc.md section 4.10, doc/8245.md lines 380-420
void VDC::shift_audio_register() {
    // Audio shift will be implemented in task 6.9
}

} // namespace videopac
