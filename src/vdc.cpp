#include "vdc.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

// ============================================================================
// IMPORTANT: Pattern Bit Ordering (Undocumented Hardware Behavior)
// ============================================================================
//
// The Intel 8245 VDC uses DIFFERENT bit ordering for characters vs sprites:
//
// CHARACTERS: MSB-first (bit 7 = leftmost pixel, bit 0 = rightmost pixel)
//   Example: Pattern byte 0b10110000
//            Renders as: ██ ██    
//                        ^       ^
//                      bit 7   bit 0
//
// SPRITES: LSB-first (bit 0 = leftmost pixel, bit 7 = rightmost pixel)
//   Example: Pattern byte 0b00001101
//            Renders as: █ ██    
//                        ^       ^
//                      bit 0   bit 7
//
// This difference is NOT documented in:
// - o2doc.md section 4.3.2 (only says "each bit controls one column")
// - o2doc.md section 4.4 (no bit ordering mentioned for characters)
// - Intel 8245 datasheet (no explicit bit ordering specification)
//
// Discovery:
// - Bug observed in Course de Voitures: cars faced wrong direction
// - Sprites were horizontally flipped when using MSB-first order
// - Confirmed by examining o2em reference emulator (doc/vdc.c):
//   * Line 477: Characters use (d1 & 0x80) with left shift (MSB-first)
//   * Line 548: Sprites use (d1 & 0x01) with right shift (LSB-first)
//
// Implementation:
// - render_characters(): Uses (pattern & (0x80 >> x))
// - render_sprites(): Uses (pattern & (0x01 << x))
// - is_character_pixel_at(): Uses (pattern & (0x80 >> x))
// - is_sprite_pixel_at(): Uses (pattern & (0x01 << x))
// ============================================================================

namespace videopac {

// Constructor
// Reference: doc/o2doc.md section 4.11, doc/8245.md lines 520-560
VDC::VDC(VideoStandard standard) {
    state_.video_standard = standard;
    extended_fb_mode_ = false;
    vdc_trace_enabled_ = false;
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
    state_.frame_number = 0;
    state_.frame_complete = false;
    
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
// 
// With the master clock implementation, the VDC increments beam_x on each tick.
// The master clock calls end_scanline() when a scanline completes (after 455 master
// ticks for NTSC, 1135 for PAL), which naturally produces varying VDC cycle counts
// per scanline (227/228 alternating for NTSC, exactly 227 for PAL).
// 
// The VDC does NOT wrap beam_x internally - it relies on end_scanline() from the
// master clock to provide the authoritative scanline boundary.
void VDC::tick_one_cycle() {
    // Advance total cycles (for compatibility and debugging)
    state_.total_cycles++;
    
    
    // Render pixel at current beam position BEFORE incrementing
    // This fixes the off-by-one error that caused black bands and broken collision detection
    render_current_pixel();
    
    // Increment H-Counter (horizontal beam position in VDC cycles)
    // This will be reset to 0 by end_scanline() when the master clock
    // indicates a scanline boundary
    state_.beam_x++;
    
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
}

// End of scanline - wrap counters for next scanline
// Called by emulator when master clock transitions to a new scanline
// This is the authoritative scanline boundary - the master clock determines
// when scanlines end based on master tick count (455 ticks NTSC, 1135 PAL)
void VDC::end_scanline() {
    // Detect collisions for the scanline that just finished rendering
    // Only check visible scanlines (hardware scanlines 24-263 for 240-line framebuffer)
    constexpr int VISIBLE_START_Y = GridLayout::START_Y;  // 24
    constexpr int VISIBLE_END_Y = VISIBLE_START_Y + FRAMEBUFFER_HEIGHT;  // 264
    
    if (state_.beam_y >= VISIBLE_START_Y && state_.beam_y < VISIBLE_END_Y) {
        // Convert hardware scanline to framebuffer row
        int y = static_cast<int>(state_.beam_y) - VISIBLE_START_Y;
        detect_collisions(y);
    }
    
    // Reset horizontal counter for next scanline
    state_.beam_x = 0;
    
    // Advance to next scanline
    state_.beam_y++;
    
    // Check for end of frame
    if (state_.beam_y >= total_scanlines_) {
        state_.beam_y = 0;
        state_.frame_number++;
        state_.frame_complete = true;
    }
}


// Reference: doc/o2doc.md Appendix D, doc/8245.md lines 600-650
void VDC::write_register(uint8 address, uint8 value) {
    // Generate VDC trace if enabled
    if (vdc_trace_enabled_) {
        std::ostringstream trace;
        trace << "[VDC] write_register(0x" << std::hex << std::setw(2) << std::setfill('0') 
              << (int)address << ", 0x" << (int)value << ")";
        
        // Add register name for important registers
        if (address == VDCRegisters::CONTROL) {
            trace << " [CONTROL: DISP=" << ((value & ControlBits::ENABLE_DISPLAY) ? "1" : "0")
                  << " GRID=" << ((value & ControlBits::ENABLE_GRID) ? "1" : "0")
                  << "]";
        } else if (address >= 0xC0 && address <= 0xC8) {
            trace << " [GRID_H" << (address - 0xC0) << "]";
        } else if (address >= 0xE0 && address <= 0xE9) {
            trace << " [GRID_V" << (address - 0xE0) << "]";
        } else if (address == VDCRegisters::COLOR) {
            trace << " [COLOR]";
        }
        
        trace << std::dec;
        last_vdc_trace_ = trace.str();
    }
    
    // Note: doc/reference/o2doc.md section 4.0 says graphic registers (0x00-0x7F)
    // "cannot be changed while the VDC is enabled by the VDC control register."
    // However, this is a programming guideline, NOT hardware enforcement.
    // The reference emulator o2em (doc/o2em/vmachine.c ext_write()) allows all
    // writes unconditionally. Games like Killer Bees write to graphic registers
    // during VBlank without disabling display first, relying on the fact that
    // the VDC is not actively scanning during VBlank. Blocking these writes
    // causes the display to freeze after the game switches to its gameplay
    // VBlank handler (bank 3, address 0xC02).
    
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
    uint8 value = 0;
    
    // Special handling for read-only registers
    switch (address) {
        case VDCRegisters::STATUS:
            // Reading status register clears certain bits
            // Reference: doc/o2doc.md section 4.7, doc/8245.md lines 480-500
            {
                value = state_.registers[address];
                // Clear sound needs service bit on read
                state_.registers[address] &= ~StatusBits::SOUND_NEEDS_SERVICE;
            }
            break;
            
        case VDCRegisters::COLLISION:
            // Reading collision register returns collision state and clears it
            // Reference: doc/o2em section 4.8, doc/8245.md lines 500-520
            {
                // Return the accumulated collision state
                value = state_.collision_state;
                
                // Clear collision state after read
                state_.collision_state = 0;
                state_.collision_detected = false;
                
                // Keep the enable mask in the register
                // (the register value is the enable mask, collision_state is the result)
            }
            break;
            
        default:
            value = state_.registers[address];
            break;
    }
    
    // Generate VDC trace if enabled
    if (vdc_trace_enabled_) {
        std::ostringstream trace;
        trace << "[VDC] read_register(0x" << std::hex << std::setw(2) << std::setfill('0') 
              << (int)address << ") = 0x" << (int)value;
        
        // Add register name and decoded bits for important registers
        if (address == VDCRegisters::STATUS) {
            trace << " [STATUS: VBLANK=" << ((value & StatusBits::VBLANK) ? "1" : "0")
                  << " HBLANK=" << ((value & StatusBits::HBLANK) ? "1" : "0")
                  << " CHAR_OVL=" << ((value & StatusBits::CHAR_OVERLAP) ? "1" : "0")
                  << " EXT_OVL=" << ((value & StatusBits::EXT_OVERLAP) ? "1" : "0")
                  << " SND=" << ((value & StatusBits::SOUND_NEEDS_SERVICE) ? "1" : "0")
                  << "]";
        } else if (address == VDCRegisters::COLLISION) {
            trace << " [COLLISION]";
        }
        
        trace << std::dec;
        last_vdc_trace_ = trace.str();
    }
    
    return value;
}

// Render current scanline to framebuffer
// NOTE: This function is primarily used by tests. During normal emulation,
// rendering happens per-pixel via render_current_pixel() called from tick_one_cycle().
// Reference: doc/o2doc.md section 4.0, doc/8245.md lines 200-300
void VDC::render_scanline() {
    // Check if display is enabled
    if (!state_.display_enabled) {
        return;
    }
    
    // Use beam_y directly (hardware coordinate)
    int beam_y = static_cast<int>(state_.beam_y);
    
    // Convert to framebuffer coordinate
    int fb_y = beam_y - FramebufferMapping::FRAMEBUFFER_START_Y;
    
    // Only render if within framebuffer bounds
    if (fb_y < 0 || fb_y >= FRAMEBUFFER_HEIGHT) {
        return;
    }
    
    // Render in priority order (background to foreground)
    // Reference: doc/o2doc.md section 4.0, design.md Property 21
    // Pass fb_y (framebuffer coordinate) to helper functions
    render_background(fb_y);
    render_grid(fb_y);
    render_characters(fb_y);
    render_sprites(fb_y);
    
    // Detect collisions for this scanline (uses framebuffer coordinate)
    detect_collisions(fb_y);
}

// Render pixel at current beam position
// Reference: Requirements 2.1, 2.3, 2.6, 12.1-12.4
void VDC::render_current_pixel() {
    // Check if display is enabled
    if (!state_.display_enabled) {
        return;
    }
    
    // Use beam coordinates for rendering logic (hardware coordinates)
    int beam_x = static_cast<int>(state_.beam_x);
    int beam_y = static_cast<int>(state_.beam_y);
    
    // Render in priority order (background to foreground)
    // Priority: sprites (highest) > characters > grid > background (lowest)
    // Reference: Requirements 12.1, 12.2, 12.3, 12.4
    
    // Start with background color
    // Background color formula (see types.h for details)
    // Formula: (color & 0x38) >> 3 | (color & 0x80 ? 0 : 8)
    // Bits 3-5: BGR components, Bit 7: inverted luminance (0=bright, 1=dark)
    uint8 color_reg = state_.registers[VDCRegisters::COLOR];
    uint8 bg_color = ((color_reg & 0x38) >> 3) | (color_reg & 0x80 ? 0 : 8);
    uint8 pixel_color = bg_color;
    
    // Check grid at this position (if enabled)
    // Grid color formula (see types.h for details)
    // Formula: (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8)
    // Bits 0-2: BGR components, Bit 6: luminance, Bit 7: inverted luminance
    if (state_.grid_enabled) {
        if (is_grid_pixel_at(beam_x, beam_y)) {
            uint8 grid_color = (color_reg & 0x07) | ((color_reg & 0x40) >> 3) | (color_reg & 0x80 ? 0 : 8);
            pixel_color = grid_color;
        }
    }
    
    // Check characters at this position
    // Character color formula (see types.h for details)
    // Formula: ((cl & 2) | ((cl & 1) << 2) | ((cl & 4) >> 2)) + 8
    // Reorders BGR bits to RGB and adds 8 for high-intensity palette
    uint8 char_color;
    if (is_character_pixel_at(beam_x, beam_y, char_color)) {
        pixel_color = char_color;
    }
    
    // Check sprites at this position (highest priority)
    // Sprite color formula (see types.h for details)
    // Formula: ((cl & 2) | ((cl & 1) << 2) | ((cl & 4) >> 2)) + 8
    // Reorders BGR bits to RGB and adds 8 for high-intensity palette
    uint8 sprite_color;
    if (is_sprite_pixel_at(beam_x, beam_y, sprite_color)) {
        pixel_color = sprite_color;
    }
    
    // Convert beam coordinates to framebuffer coordinates
    int fb_x = beam_x - FramebufferMapping::FRAMEBUFFER_START_X;
    int fb_y = beam_y - FramebufferMapping::FRAMEBUFFER_START_Y;
    
    // Write to normal framebuffer if within bounds
    if (fb_x >= 0 && fb_x < FRAMEBUFFER_WIDTH && fb_y >= 0 && fb_y < FRAMEBUFFER_HEIGHT) {
        state_.framebuffer[fb_y][fb_x] = pixel_color;
    }
    
    // Convert beam coordinates to extended framebuffer coordinates
    int ext_fb_x = beam_x - FramebufferMapping::EXTENDED_FB_START_X;
    int ext_fb_y = beam_y - FramebufferMapping::EXTENDED_FB_START_Y;
    
    // Write to extended framebuffer if enabled and within extended bounds
    if (extended_fb_mode_ && ext_fb_x >= 0 && ext_fb_x < EXTENDED_FB_WIDTH && 
        ext_fb_y >= 0 && ext_fb_y < EXTENDED_FB_HEIGHT) {
        state_.extended_framebuffer[ext_fb_y][ext_fb_x] = pixel_color;
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
}

// Get extended framebuffer pointer
const uint8* VDC::get_extended_framebuffer() const {
    return &state_.extended_framebuffer[0][0];
}

// Check if in vertical blank period
// Reference: doc/o2doc.md section 4.11, doc/8245.md lines 520-560
// Reference: doc/hardware/odyssey2_timing.txt - Vblank transitions at master tick 365
//
// Hardware timing: Vblank transitions at master tick 365 (between master ticks 364-365)
// VDC timing: Master ticks 364-365 correspond to VDC cycle 182 (365/2 = 182.5)
//             Transition happens at END of VDC cycle 182
//             At beam_x=183, Vblank has transitioned
//
// Implementation: Use beam_x >= BLANKING_START_X (183) for transition point
bool VDC::is_vblank() const {
    if (state_.beam_y == vblank_start_) {
        // On vblank start scanline, Vblank goes high at beam_x = 183
        return (state_.beam_x >= VideoTiming::BLANKING_START_X);
    } else if (state_.beam_y == 0) {
        // On scanline 0, Vblank goes low at beam_x = 183
        return (state_.beam_x < VideoTiming::BLANKING_START_X);
    } else if (state_.beam_y > vblank_start_) {
        // Between vblank start and end of frame, Vblank is active
        return true;
    } else {
        // Before vblank start, Vblank is inactive
        return false;
    }
}

// Get T1 pin state (for CPU counter mode)
// T1 = !(Hblank OR Vblank) - T1 is HIGH during visible period, LOW during blanking
// Reference: doc/hardware/odyssey2_timing.txt "T1 input caveat" section
// "The Hblank and Vblank are OR'd together and connect to the T1 input on the 8048."
//
// Hardware timing (master ticks 0-454 per scanline):
// - Hblank: tick > 365 && tick < 453 (active from tick 366-452)
// - Vblank: transitions at tick 365 on scanlines vblank_start and 0
//
// VDC cycle timing (0-227 per scanline, VDC ticks every 2 master ticks):
// - Master ticks 366-367 = VDC cycle 183
// - Both Hblank and Vblank transition at beam_x = 183
// - The 140ns gap (1 master tick) between them is not observable at VDC granularity
//
// T1 behavior:
// - T1 is HIGH (1) during visible period (not blanking)
// - T1 is LOW (0) during blanking (hblank or vblank)
// - Counter increments on falling edge (visible → blanking transition)
//
// Note: VDC cycle granularity is sufficient because:
// - CPU samples T1 every 20 master ticks (10 VDC cycles)
// - VDC updates every 2 master ticks (1 VDC cycle)
// - The 1 master tick gap is too small to observe
bool VDC::get_t1_state() const {
    // Hblank is active when beam_x >= BLANKING_START_X
    bool hblank = (state_.beam_x >= VideoTiming::BLANKING_START_X);
    
    // Vblank uses same logic as is_vblank()
    bool vblank = is_vblank();
    
    // T1 is inverted: HIGH during visible, LOW during blanking
    return !(hblank || vblank);
}

// Check if frame just completed (beam wrapped to scanline 0)
bool VDC::is_frame_complete() const {
    return state_.frame_complete;
}

// Clear frame_complete flag (called at start of new frame)
void VDC::clear_frame_complete() {
    state_.frame_complete = false;
}

// Get current frame number from total cycles
uint64 VDC::get_frame_number() const {
    return state_.frame_number;
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

// Get character ROM data (for debugger display)
void VDC::get_character_rom(uint8* dest) const {
    std::memcpy(dest, character_rom_, sizeof(character_rom_));
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
    } else {  // PAL
        total_scanlines_ = VideoTiming::PAL_SCANLINES;
        vblank_start_ = VideoTiming::PAL_VBLANK_START;
    }
}

// Rendering helper: Fill scanline with background color
// NOTE: This function is only used by render_scanline() for testing.
// During normal emulation, background rendering happens in render_current_pixel().
// Reference: doc/o2doc.md section 4.9, doc/8245.md lines 440-470
void VDC::render_background(int y) {
    // Get background color from color register
    // Background color formula (see types.h for details)
    // Formula: (color & 0x38) >> 3 | (color & 0x80 ? 0 : 8)
    // Bits 3-5: BGR components, Bit 7: inverted luminance (0=bright, 1=dark)
    uint8 color_reg = state_.registers[VDCRegisters::COLOR];
    uint8 bg_color = ((color_reg & 0x38) >> 3) | (color_reg & 0x80 ? 0 : 8);
    
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
    
    // Get grid color from color register
    // Grid color formula (see types.h for details)
    // Formula: (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8)
    // Bits 0-2: BGR components, Bit 6: luminance, Bit 7: inverted luminance
    uint8 color_reg = state_.registers[VDCRegisters::COLOR];
    uint8 grid_color = (color_reg & 0x07) | ((color_reg & 0x40) >> 3) | (color_reg & 0x80 ? 0 : 8);
    
    // Check control register for grid modes
    uint8 control = state_.registers[VDCRegisters::CONTROL];
    bool fill_mode = (control & ControlBits::ENABLE_FILL_MODE) != 0;
    bool dot_mode = (control & ControlBits::ENABLE_DOT_GRID) != 0;
    
    // Grid layout: 9 rows × 9 columns of horizontal bars, 10 columns × 8 rows of vertical bars
    // This creates 9 columns × 8 rows = 72 enclosed areas (boxes)
    // Each horizontal bar is 3 scanlines tall, spaced by 21 scanlines
    // First horizontal bar starts at scanline 24 (relative to end of VBLANK)
    // Reference: doc/8245.md lines 720-760
    
    // IMPORTANT: Grid register layout (bytes go left to right, bits go top to bottom)
    //
    // Visual representation of the grid:
    //     Col:  0   1   2   3   4   5   6   7   8   9
    //          ┌───┬───┬───┬───┬───┬───┬───┬───┬───┐
    // Row 0    │ H │ H │ H │ H │ H │ H │ H │ H │ H │   H = Horizontal bar segment
    //          ├ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V  V = Vertical bar segment
    // Row 1    │ H │ H │ H │ H │ H │ H │ H │ H │ H │
    //          ├ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V
    // Row 2    │ H │ H │ H │ H │ H │ H │ H │ H │ H │
    //          ├ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V
    // Row 3    │ H │ H │ H │ H │ H │ H │ H │ H │ H │
    //          ├ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V
    // Row 4    │ H │ H │ H │ H │ H │ H │ H │ H │ H │
    //          ├ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V
    // Row 5    │ H │ H │ H │ H │ H │ H │ H │ H │ H │
    //          ├ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V
    // Row 6    │ H │ H │ H │ H │ H │ H │ H │ H │ H │
    //          ├ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V
    // Row 7    │ H │ H │ H │ H │ H │ H │ H │ H │ H │
    //          ├ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V ┼ V
    // Row 8    │ H │ H │ H │ H │ H │ H │ H │ H │ H │
    //          └───┴───┴───┴───┴───┴───┴───┴───┴───┘
    //
    // Register mapping:
    // Horizontal bars C0-C8: Each byte represents a COLUMN (0-8), bits 0-7 represent ROWS (0-7)
    //   C0 = column 0: bit 0=row 0, bit 1=row 1, ..., bit 7=row 7
    //   C1 = column 1: bit 0=row 0, bit 1=row 1, ..., bit 7=row 7
    //   ...
    //   C8 = column 8: bit 0=row 0, bit 1=row 1, ..., bit 7=row 7
    //   Total: 9 columns × 8 rows = 72 segments
    //
    // Horizontal bar row 8 (D0-D8): 9 bytes, bit 0 only
    //   D0 bit 0 = column 0 row 8
    //   D1 bit 0 = column 1 row 8
    //   ...
    //   D8 bit 0 = column 8 row 8
    //   Total: 9 columns × 1 row = 9 segments
    //   Grand total horizontal: 72 + 9 = 81 segments (9 rows × 9 columns)
    //
    // Vertical bars E0-E9: Each byte represents a COLUMN (0-9), bits 0-7 represent ROWS (0-7)
    //   E0 = column 0: bit 0=row 0, bit 1=row 1, ..., bit 7=row 7
    //   E1 = column 1: bit 0=row 0, bit 1=row 1, ..., bit 7=row 7
    //   ...
    //   E9 = column 9: bit 0=row 0, bit 1=row 1, ..., bit 7=row 7
    //   Total: 10 columns × 8 rows = 80 segments
    
    // Calculate grid row (0-8) based on scanline
    const int GRID_START_Y = GridLayout::START_Y;
    const int GRID_ROW_HEIGHT = GridLayout::ROW_HEIGHT;
    const int GRID_LINE_HEIGHT = GridLayout::LINE_HEIGHT;
    const int GRID_START_X = GridLayout::START_X;
    const int GRID_COL_WIDTH = GridLayout::COL_WIDTH;
    
    // Check if we're on a horizontal grid line
    if (y >= GRID_START_Y) {
        int y_offset = y - GRID_START_Y;
        int grid_row = y_offset / GRID_ROW_HEIGHT;
        int row_offset = y_offset % GRID_ROW_HEIGHT;
        
        // Render horizontal grid lines (9 rows total: 0-8)
        if (grid_row < 9 && row_offset < GRID_LINE_HEIGHT) {
            // Render horizontal line segments
            
            // Loop through columns (0-8) and check if segment at this row is enabled
            for (int col = 0; col < 9; col++) {
                bool segment_on = false;
                
                if (grid_row < 8) {
                    // Rows 0-7: Check bit grid_row of byte C0+col
                    // Example: For row 2, col 3: check bit 2 of register C3
                    segment_on = (state_.registers[VDCRegisters::GRID_H_BASE + col] & (1 << grid_row)) != 0;
                } else {
                    // Row 8: Check bit 0 of byte D0+col
                    // Example: For row 8, col 3: check bit 0 of register D3
                    segment_on = (state_.registers[VDCRegisters::GRID_H9_BASE + col] & 0x01) != 0;
                }
                
                if (segment_on) {
                    int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                    int x_end = x_start + GridLayout::HBAR_WIDTH;
                    
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
    // Vertical bars span the full 24-scanline height of their row
    // Reference: doc/o2doc.md section 4.2
    const int VERT_LINE_WIDTH = fill_mode ? GridLayout::VBAR_WIDTH_FILL : GridLayout::VBAR_WIDTH_NORMAL;
    
    // Calculate which grid row we're in for vertical line rendering
    if (y >= GRID_START_Y) {
        int y_offset = y - GRID_START_Y;
        int grid_row = y_offset / GRID_ROW_HEIGHT;
        
        // Only render vertical bars for rows 0-7 (8 rows total)
        // Each vertical bar spans the full 24 scanlines of its row
        if (grid_row < 8) {
            for (int col = 0; col < 10; col++) {
                uint8 v_line_data = state_.registers[VDCRegisters::GRID_V_BASE + col];
                
                // Check if this vertical bar segment is enabled for current row
                bool segment_on = (v_line_data & (1 << grid_row)) != 0;
                
                if (segment_on) {
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
        // Character color formula (see types.h): reorders BGR bits to RGB and adds 8 for high-intensity
        uint8 cl = (char_attr >> 1) & 0x07;
        uint8 color = ((cl & 2) | ((cl & 1) << 2) | ((cl & 4) >> 2)) + 8;
        
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
            // IMPORTANT: Characters use MSB-first bit order (bit 7 = leftmost, bit 0 = rightmost)
            // This is DIFFERENT from sprites which use LSB-first order (bit 0 = leftmost)
            // This bit ordering is NOT documented in o2doc or Intel 8245 datasheet
            // Reference: Verified by comparing with o2em source (doc/vdc.c line 477)
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
            // Character color formula (see types.h): reorders BGR bits to RGB and adds 8 for high-intensity
            uint8 cl = (char_attr >> 1) & 0x07;
            uint8 color = ((cl & 2) | ((cl & 1) << 2) | ((cl & 4) >> 2)) + 8;
            
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
        uint8 sprite_color_bits = (sprite_color_attr & SpriteColorBits::COLOR_MASK) >> SpriteColorBits::COLOR_SHIFT;
        // Sprite color formula (see types.h): reorders BGR bits to RGB and adds 8 for high-intensity
        uint8 color = ((sprite_color_bits & 2) | ((sprite_color_bits & 1) << 2) | ((sprite_color_bits & 4) >> 2)) + 8;
        bool double_size = (sprite_color_attr & SpriteColorBits::DOUBLE_SIZE) != 0;
        bool shift_even = (sprite_color_attr & SpriteColorBits::SHIFT_EVEN) != 0;
        bool shift_full = (sprite_color_attr & SpriteColorBits::SHIFT_FULL) != 0;
        
        // Calculate sprite height and check if current scanline intersects sprite
        // Normal sprites: 8 pattern rows × 2 scanlines per row = 16 scanlines
        // Double-size sprites: 8 pattern rows × 4 scanlines per row = 32 scanlines
        // Reference: Verified in o2em source (doc/vdc.c lines 502-509)
        int sprite_height = double_size ? 32 : 16;
        if (y < sprite_y || y >= sprite_y + sprite_height) {
            continue;  // Scanline doesn't intersect this sprite
        }
        
        // Calculate which row of the sprite pattern to render
        // Each pattern row spans multiple scanlines
        int sprite_row = (y - sprite_y) / (double_size ? 4 : 2);
        
        // Get sprite pattern byte for this row
        uint8 pattern_addr = VDCRegisters::SPRITE0_PATTERN + (sprite_num * 8) + sprite_row;
        uint8 pattern = state_.registers[pattern_addr];
        
        // Render sprite pixels for this scanline
        int sprite_width = double_size ? 16 : 8;
        for (int x = 0; x < sprite_width; x++) {
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
            
            // Get bit from pattern
            // IMPORTANT: Sprites use LSB-first bit order (bit 0 = leftmost, bit 7 = rightmost)
            // This is DIFFERENT from characters which use MSB-first order (bit 7 = leftmost)
            // 
            // This bit ordering difference is NOT documented in:
            // - o2doc.md section 4.3.2 (only says "each bit controls one column")
            // - Intel 8245 datasheet
            // 
            // Discovery process:
            // 1. Bug observed: Cars in Course de Voitures faced wrong direction
            // 2. Testing showed sprites were horizontally flipped with MSB-first order
            // 3. Confirmed by examining o2em reference emulator (doc/vdc.c):
            //    - Line 477: Characters use (d1 & 0x80) with left shift (MSB-first)
            //    - Line 548: Sprites use (d1 & 0x01) with right shift (LSB-first)
            int pattern_x = double_size ? (x / 2) : x;
            bool pixel_on = (pattern & (0x01 << pattern_x)) != 0;
            
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

    // IMPORTANT: Track ALL objects, not just enabled ones!
    // The collision_enable mask determines which objects we're asking about,
    // but ALL objects participate in collision detection.
    // Example: If tracking sprite 0 (enable=0x01), and sprite 0 collides with sprite 1,
    // we need sprite 1 to be in the object_buffer to detect the collision.
    
    // Always track grid objects (so sprites/characters can collide with them)
    track_grid_objects(y, object_buffer, collision_enable);
    
    // Always track character objects (so sprites can collide with them)
    track_character_objects(y, object_buffer, collision_enable);
    
    // Track ALL sprite objects (not just enabled ones)
    // This is necessary because if we're tracking sprite 0, we need to know
    // where sprite 1 is to detect collisions between them
    for (int sprite_num = 0; sprite_num < 4; sprite_num++) {
        track_sprite_object(y, sprite_num, object_buffer, collision_enable);
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
// Source: Intel 8245 datasheet character set data
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

    const int GRID_START_Y = GridLayout::START_Y;
    const int GRID_ROW_HEIGHT = GridLayout::ROW_HEIGHT;
    const int GRID_LINE_HEIGHT = GridLayout::LINE_HEIGHT;
    const int GRID_START_X = GridLayout::START_X;
    const int GRID_COL_WIDTH = GridLayout::COL_WIDTH;
    const int VERT_LINE_WIDTH = fill_mode ? 16 : 2;
    
    bool h_grid_enabled = (collision_enable & CollisionBits::HORIZ_GRID) != 0;
    bool v_grid_enabled = (collision_enable & CollisionBits::VERT_GRID) != 0;

    // Track horizontal grid lines
    if (y >= GRID_START_Y) {
        int y_offset = y - GRID_START_Y;
        int grid_row = y_offset / GRID_ROW_HEIGHT;
        int row_offset = y_offset % GRID_ROW_HEIGHT;

        if (grid_row < 9 && row_offset < GRID_LINE_HEIGHT) {
            // Loop through columns (0-8) and check if segment at this row is enabled
            // Grid uses column-based layout: bytes = columns, bits = rows
            // This matches the rendering logic in render_grid()
            for (int col = 0; col < 9; col++) {
                bool segment_on = false;
                
                if (grid_row < 8) {
                    // Rows 0-7: Check bit grid_row of byte C0+col
                    segment_on = (state_.registers[VDCRegisters::GRID_H_BASE + col] & (1 << grid_row)) != 0;
                } else {
                    // Row 8: Check bit 0 of byte D0+col
                    segment_on = (state_.registers[VDCRegisters::GRID_H9_BASE + col] & 0x01) != 0;
                }
                
                if (segment_on) {
                    int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                    int x_end = x_start + GridLayout::HBAR_WIDTH;

                    for (int x = x_start; x < x_end && x < FRAMEBUFFER_WIDTH; x++) {
                        // Check for collision with existing objects
                        if (object_buffer[x] != 0) {
                            // Check if ANY enabled object is involved in this collision
                            uint8 enabled_objects_in_collision = object_buffer[x] & collision_enable;
                            
                            if (h_grid_enabled) {
                                // Horizontal grid is enabled, so report what it collided with
                                state_.collision_state |= object_buffer[x];
                                state_.collision_detected = true;
                            } else if (enabled_objects_in_collision != 0) {
                                // Grid is not enabled, but it's colliding with an enabled object
                                // So report the grid to the enabled object
                                state_.collision_state |= CollisionBits::HORIZ_GRID;
                                state_.collision_detected = true;
                            }
                        }
                        // Always add grid to buffer (for other objects to collide with)
                        object_buffer[x] |= CollisionBits::HORIZ_GRID;
                    }
                }
            }
        }
    }

    // Track vertical grid lines
    if (y >= GRID_START_Y) {
        int y_offset = y - GRID_START_Y;
        int grid_row = y_offset / GRID_ROW_HEIGHT;
        int row_offset = y_offset % GRID_ROW_HEIGHT;

        if (grid_row < 8) {
            for (int col = 0; col < 10; col++) {
                uint8 v_line_data = state_.registers[VDCRegisters::GRID_V_BASE + col];
                bool segment_on = false;
                
                // Check if vertical bar at current row is enabled
                segment_on = (v_line_data & (1 << grid_row)) != 0;
                
                // ALSO check if previous row's vertical bar extends down into this row's h-bar area
                if (!segment_on && grid_row > 0 && row_offset < GRID_LINE_HEIGHT) {
                    segment_on = (v_line_data & (1 << (grid_row - 1))) != 0;
                }

                if (segment_on) {
                    int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                    int x_end = x_start + VERT_LINE_WIDTH;

                    for (int x = x_start; x < x_end && x < FRAMEBUFFER_WIDTH; x++) {
                        if (object_buffer[x] != 0) {
                            // Check if ANY enabled object is involved in this collision
                            uint8 enabled_objects_in_collision = object_buffer[x] & collision_enable;
                            
                            if (v_grid_enabled) {
                                // Vertical grid is enabled, so report what it collided with
                                state_.collision_state |= object_buffer[x];
                                state_.collision_detected = true;
                            } else if (enabled_objects_in_collision != 0) {
                                // Grid is not enabled, but it's colliding with an enabled object
                                // So report the grid to the enabled object
                                state_.collision_state |= CollisionBits::VERT_GRID;
                                state_.collision_detected = true;
                            }
                        }
                        // Always add grid to buffer (for other objects to collide with)
                        object_buffer[x] |= CollisionBits::VERT_GRID;
                    }
                }
            }
        } else if (grid_row == 8 && row_offset < GRID_LINE_HEIGHT) {
            // Row 8, first 3 scanlines (horizontal bar area):
            // Check if vertical bar from row 7 extends down to connect
            for (int col = 0; col < 10; col++) {
                uint8 v_line_data = state_.registers[VDCRegisters::GRID_V_BASE + col];
                bool segment_on = (v_line_data & (1 << 7)) != 0;
                
                if (segment_on) {
                    int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                    int x_end = x_start + VERT_LINE_WIDTH;

                    for (int x = x_start; x < x_end && x < FRAMEBUFFER_WIDTH; x++) {
                        if (object_buffer[x] != 0) {
                            uint8 enabled_objects_in_collision = object_buffer[x] & collision_enable;
                            
                            if (v_grid_enabled) {
                                state_.collision_state |= object_buffer[x];
                                state_.collision_detected = true;
                            } else if (enabled_objects_in_collision != 0) {
                                state_.collision_state |= CollisionBits::VERT_GRID;
                                state_.collision_detected = true;
                            }
                        }
                        object_buffer[x] |= CollisionBits::VERT_GRID;
                    }
                }
            }
        }
    }

    // Track dot grid
    if (dot_mode && y >= GRID_START_Y) {
        int y_offset = y - GRID_START_Y;
        int grid_row = y_offset / GRID_ROW_HEIGHT;
        int row_offset = y_offset % GRID_ROW_HEIGHT;

        if (grid_row < 9 && row_offset < 3) {
            for (int col = 0; col < 10; col++) {
                int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                int x_end = x_start + 2;

                for (int x = x_start; x < x_end && x < FRAMEBUFFER_WIDTH; x++) {
                    if (object_buffer[x] != 0) {
                        // Check if ANY enabled object is involved in this collision
                        uint8 enabled_objects_in_collision = object_buffer[x] & collision_enable;
                        
                        if (h_grid_enabled) {
                            // Horizontal grid is enabled, so report what it collided with
                            state_.collision_state |= object_buffer[x];
                            state_.collision_detected = true;
                        } else if (enabled_objects_in_collision != 0) {
                            // Grid is not enabled, but it's colliding with an enabled object
                            // So report the grid to the enabled object
                            state_.collision_state |= CollisionBits::HORIZ_GRID;
                            state_.collision_detected = true;
                        }
                    }
                    // Always add grid to buffer (for other objects to collide with)
                    object_buffer[x] |= CollisionBits::HORIZ_GRID;
                }
            }
        }
    }
}

// Collision tracking helper: Track character objects
// Reference: doc/o2doc.md section 4.8
void VDC::track_character_objects(int y, uint8* object_buffer, uint8 collision_enable) {
    if (!state_.display_enabled) {
        return;
    }

    bool char_to_char_collision = false;
    bool is_enabled = (collision_enable & CollisionBits::CHARACTERS) != 0;

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
                    // Check if ANY enabled object is involved in this collision
                    uint8 enabled_objects_in_collision = object_buffer[screen_x] & collision_enable;
                    
                    if (is_enabled) {
                        // Characters are enabled, so report what they collided with
                        state_.collision_state |= object_buffer[screen_x];
                        state_.collision_detected = true;
                        
                        // If the other object is also enabled, report characters too (bidirectional)
                        if (enabled_objects_in_collision != 0) {
                            state_.collision_state |= CollisionBits::CHARACTERS;
                        }
                    } else if (enabled_objects_in_collision != 0) {
                        // Characters not enabled, but colliding with an enabled object
                        // So report the character to the enabled object
                        state_.collision_state |= CollisionBits::CHARACTERS;
                        state_.collision_detected = true;
                    }

                    // Check for character-to-character collision
                    if (object_buffer[screen_x] & CollisionBits::CHARACTERS) {
                        char_to_char_collision = true;
                    }
                }
                // Always add character to buffer (for other objects to collide with)
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
                        // Check if ANY enabled object is involved in this collision
                        uint8 enabled_objects_in_collision = object_buffer[screen_x] & collision_enable;
                        
                        if (is_enabled) {
                            // Characters are enabled, so report what they collided with
                            state_.collision_state |= object_buffer[screen_x];
                            state_.collision_detected = true;
                            
                            // If the other object is also enabled, report characters too (bidirectional)
                            if (enabled_objects_in_collision != 0) {
                                state_.collision_state |= CollisionBits::CHARACTERS;
                            }
                        } else if (enabled_objects_in_collision != 0) {
                            // Characters not enabled, but colliding with an enabled object
                            // So report the character to the enabled object
                            state_.collision_state |= CollisionBits::CHARACTERS;
                            state_.collision_detected = true;
                        }

                        if (object_buffer[screen_x] & CollisionBits::CHARACTERS) {
                            char_to_char_collision = true;
                        }
                    }
                    // Always add character to buffer (for other objects to collide with)
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
void VDC::track_sprite_object(int y, int sprite_num, uint8* object_buffer, uint8 collision_enable) {
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

    // Use correct sprite height (same as rendering code)
    // Normal sprites: 8 pattern rows × 2 scanlines per row = 16 scanlines
    // Double-size sprites: 8 pattern rows × 4 scanlines per row = 32 scanlines
    int sprite_height = double_size ? 32 : 16;
    if (y < sprite_y || y >= sprite_y + sprite_height) {
        return;
    }

    // Calculate which row of the sprite pattern (same as rendering code)
    int sprite_row = (y - sprite_y) / (double_size ? 4 : 2);

    uint8 pattern_addr = VDCRegisters::SPRITE0_PATTERN + (sprite_num * 8) + sprite_row;
    uint8 pattern = state_.registers[pattern_addr];

    uint8 sprite_bit = (1 << sprite_num);
    int sprite_width = double_size ? 16 : 8;
    
    // Check if this sprite is enabled for collision tracking
    bool is_enabled = (collision_enable & sprite_bit) != 0;

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
        // Use LSB-first bit order for sprites (same as rendering code)
        bool pixel_on = (pattern & (0x01 << pattern_x)) != 0;

        if (pixel_on) {
            // Check for collision with existing objects
            if (object_buffer[screen_x] != 0) {
                // Check if ANY enabled object is involved in this collision
                // If this sprite is enabled, or if any object in the buffer is enabled
                uint8 enabled_objects_in_collision = object_buffer[screen_x] & collision_enable;
                
                if (is_enabled) {
                    // This sprite is enabled, so report what it collided with
                    state_.collision_state |= object_buffer[screen_x];
                    state_.collision_detected = true;
                    
                    // If the other object is also enabled, report this sprite too (bidirectional)
                    if (enabled_objects_in_collision != 0) {
                        state_.collision_state |= sprite_bit;
                    }
                } else if (enabled_objects_in_collision != 0) {
                    // This sprite is not enabled, but it's colliding with an enabled object
                    // So report this sprite to the enabled object
                    state_.collision_state |= sprite_bit;
                    state_.collision_detected = true;
                }
            }
            // Always add this sprite to the buffer (for other objects to collide with)
            object_buffer[screen_x] |= sprite_bit;
        }
    }
}

// Per-pixel rendering helper: Check if grid pixel exists at position
// Reference: Requirements 12.1, 12.2
// 
// Grid register layout (bytes go left to right, bits go top to bottom):
// - Horizontal bars: 9 rows × 9 columns = 81 segments
//   - C0-C8: Each byte = COLUMN (0-8), bits 0-7 = ROWS (0-7) [72 segments]
//   - D0-D8: Row 8, bit 0 only (one per column) [9 segments]
// - Vertical bars: 10 columns × 8 rows = 80 segments
//   - E0-E9: Each byte = COLUMN (0-9), bits 0-7 = ROWS (0-7)
bool VDC::is_grid_pixel_at(int x, int y) const {
    if (!state_.grid_enabled) {
        return false;
    }
    
    uint8 control = state_.registers[VDCRegisters::CONTROL];
    bool fill_mode = (control & ControlBits::ENABLE_FILL_MODE) != 0;
    bool dot_mode = (control & ControlBits::ENABLE_DOT_GRID) != 0;
    
    const int GRID_START_Y = GridLayout::START_Y;
    const int GRID_ROW_HEIGHT = GridLayout::ROW_HEIGHT;
    const int GRID_LINE_HEIGHT = GridLayout::LINE_HEIGHT;
    const int GRID_START_X = GridLayout::START_X;
    const int GRID_COL_WIDTH = GridLayout::COL_WIDTH;
    const int VERT_LINE_WIDTH = fill_mode ? GridLayout::VBAR_WIDTH_FILL : GridLayout::VBAR_WIDTH_NORMAL;
    
    if (y < GRID_START_Y) {
        return false;
    }
    
    int y_offset = y - GRID_START_Y;
    int grid_row = y_offset / GRID_ROW_HEIGHT;
    int row_offset = y_offset % GRID_ROW_HEIGHT;
    
    // Check horizontal grid lines (9 rows: 0-8)
    // Loop through columns and check if segment at this row is enabled
    if (grid_row < 9 && row_offset < GRID_LINE_HEIGHT) {
        for (int col = 0; col < 9; col++) {
            bool segment_on = false;
            
            if (grid_row < 8) {
                // Rows 0-7: Check bit grid_row of byte C0+col
                // Example: For row 2, col 3: check bit 2 of register C3
                segment_on = (state_.registers[VDCRegisters::GRID_H_BASE + col] & (1 << grid_row)) != 0;
            } else {
                // Row 8: Check bit 0 of byte D0+col
                // Example: For row 8, col 3: check bit 0 of register D3
                segment_on = (state_.registers[VDCRegisters::GRID_H9_BASE + col] & 0x01) != 0;
            }
            
            if (segment_on) {
                int x_start = GRID_START_X + (col * GRID_COL_WIDTH);
                int x_end = x_start + GridLayout::HBAR_WIDTH;
                
                if (x >= x_start && x < x_end) {
                    return true;
                }
            }
        }
    }
    
    // Check vertical grid lines (10 columns: 0-9)
    // Each vertical bar spans the full 24 scanlines of its row
    if (grid_row < 8) {
        for (int col = 0; col < 10; col++) {
            uint8 v_line_data = state_.registers[VDCRegisters::GRID_V_BASE + col];
            
            // Check if this vertical bar segment is enabled for current row
            bool segment_on = (v_line_data & (1 << grid_row)) != 0;
            
            if (segment_on) {
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
        // Character height calculation (see doc/reference/o2doc.md section 4.4)
        // Characters can be "cut off" at the top based on Y position and pattern pointer alignment
        // This calculates how many of the 8 pattern rows are actually visible
        int ypos_half = char_y / 2;
        int n = 8 - (ypos_half % 8) - (char_ptr_low % 8);
        if (n < 3) {
            n = n + 7;  // Minimum 3 rows, wraps around for very small values
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
            color = ((char_attr >> 1) & 0x07) + 8;  // Characters use high-intensity palette
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
            
            // Character height calculation (see doc/reference/o2doc.md section 4.5)
            // Characters can be "cut off" at the top based on Y position and pattern pointer alignment
            // This calculates how many of the 8 pattern rows are actually visible
            int ypos_half = char_y / 2;
            int n = 8 - (ypos_half % 8) - (char_ptr_low % 8);
            if (n < 3) {
                n = n + 7;  // Minimum 3 rows, wraps around for very small values
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
                // Character color formula (see types.h): reorders BGR bits to RGB and adds 8 for high-intensity
                uint8 cl = (char_attr >> 1) & 0x07;
                color = ((cl & 2) | ((cl & 1) << 2) | ((cl & 4) >> 2)) + 8;
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
        // Normal sprites: 8 pattern rows × 2 scanlines per row = 16 scanlines
        // Double-size sprites: 8 pattern rows × 4 scanlines per row = 32 scanlines
        // Reference: Verified in o2em source (doc/vdc.c lines 502-509)
        int sprite_height = double_size ? 32 : 16;
        if (y < sprite_y || y >= sprite_y + sprite_height) {
            continue;
        }
        
        // Calculate which row of the sprite pattern to check
        // Each pattern row spans multiple scanlines
        int sprite_row = (y - sprite_y) / (double_size ? 4 : 2);
        
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
        // IMPORTANT: Sprites use LSB-first bit order (bit 0 = leftmost, bit 7 = rightmost)
        // This is DIFFERENT from characters which use MSB-first order (bit 7 = leftmost)
        // 
        // This bit ordering difference is NOT documented in:
        // - o2doc.md section 4.3.2 (only says "each bit controls one column")
        // - Intel 8245 datasheet
        // 
        // Discovery process:
        // 1. Bug observed: Cars in Course de Voitures faced wrong direction
        // 2. Testing showed sprites were horizontally flipped with MSB-first order
        // 3. Confirmed by examining o2em reference emulator (doc/vdc.c):
        //    - Line 477: Characters use (d1 & 0x80) with left shift (MSB-first)
        //    - Line 548: Sprites use (d1 & 0x01) with right shift (LSB-first)
        int pattern_x = double_size ? (pixel_x / 2) : pixel_x;
        bool pixel_on = (pattern & (0x01 << pattern_x)) != 0;
        
        if (pixel_on) {
            // Sprite color formula (see types.h): reorders BGR bits to RGB and adds 8 for high-intensity
            color = ((sprite_color & 2) | ((sprite_color & 1) << 2) | ((sprite_color & 4) >> 2)) + 8;
            return true;
        }
    }
    
    return false;
}

} // namespace videopac

