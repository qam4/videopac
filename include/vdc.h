#ifndef VIDEOPAC_VDC_H
#define VIDEOPAC_VDC_H

#include "types.h"
#include "master_clock.h"

namespace videopac {

// VDC Register addresses
// Reference: doc/o2doc.md Appendix D, doc/8245.md lines 600-650
namespace VDCRegisters {
    // Sprite control (4 sprites, 4 bytes each)
    constexpr uint8 SPRITE0_Y = 0x00;
    constexpr uint8 SPRITE0_X = 0x01;
    constexpr uint8 SPRITE0_COLOR = 0x02;
    constexpr uint8 SPRITE0_UNUSED = 0x03;
    
    constexpr uint8 SPRITE1_Y = 0x04;
    constexpr uint8 SPRITE1_X = 0x05;
    constexpr uint8 SPRITE1_COLOR = 0x06;
    constexpr uint8 SPRITE1_UNUSED = 0x07;
    
    constexpr uint8 SPRITE2_Y = 0x08;
    constexpr uint8 SPRITE2_X = 0x09;
    constexpr uint8 SPRITE2_COLOR = 0x0A;
    constexpr uint8 SPRITE2_UNUSED = 0x0B;
    
    constexpr uint8 SPRITE3_Y = 0x0C;
    constexpr uint8 SPRITE3_X = 0x0D;
    constexpr uint8 SPRITE3_COLOR = 0x0E;
    constexpr uint8 SPRITE3_UNUSED = 0x0F;
    
    // Character control (12 characters, 4 bytes each, 0x10-0x3F)
    constexpr uint8 CHAR_BASE = 0x10;
    
    // Quad character control (4 groups, 16 bytes each, 0x40-0x7F)
    constexpr uint8 QUAD_BASE = 0x40;
    
    // Sprite patterns (4 sprites, 8 bytes each)
    constexpr uint8 SPRITE0_PATTERN = 0x80;
    constexpr uint8 SPRITE1_PATTERN = 0x88;
    constexpr uint8 SPRITE2_PATTERN = 0x90;
    constexpr uint8 SPRITE3_PATTERN = 0x98;
    
    // Control and status registers
    constexpr uint8 CONTROL = 0xA0;
    constexpr uint8 STATUS = 0xA1;
    constexpr uint8 COLLISION = 0xA2;
    constexpr uint8 COLOR = 0xA3;
    constexpr uint8 BEAM_X = 0xA4;
    constexpr uint8 BEAM_Y = 0xA5;
    
    // Audio registers
    constexpr uint8 SOUND0 = 0xA7;
    constexpr uint8 SOUND1 = 0xA8;
    constexpr uint8 SOUND2 = 0xA9;
    constexpr uint8 SOUND_CONTROL = 0xAA;
    
    // Grid registers
    constexpr uint8 GRID_H_BASE = 0xC0;
    constexpr uint8 GRID_H9_BASE = 0xD0;
    constexpr uint8 GRID_V_BASE = 0xE0;
}

// Grid layout constants
namespace GridLayout {
    constexpr int START_X = 8;
    constexpr int START_Y = 24;
    constexpr int ROW_HEIGHT = 24;
    constexpr int LINE_HEIGHT = 3;
    constexpr int COL_WIDTH = 16;
    constexpr int HBAR_WIDTH = 18;
    constexpr int VBAR_WIDTH_NORMAL = 2;
    constexpr int VBAR_WIDTH_FILL = 16;
}

// Framebuffer mapping constants
namespace FramebufferMapping {
    constexpr int FRAMEBUFFER_START_X = 0;
    constexpr int FRAMEBUFFER_START_Y = 0;
    constexpr int EXTENDED_FB_START_X = 0;
    constexpr int EXTENDED_FB_START_Y = 0;
}

// Control register (0xA0) bit definitions
namespace ControlBits {
    constexpr uint8 ENABLE_HBLANK_INT = 0x01;
    constexpr uint8 LATCH_BEAM_POS = 0x02;
    constexpr uint8 ENABLE_SOUND_INT = 0x04;
    constexpr uint8 ENABLE_GRID = 0x08;
    constexpr uint8 ENABLE_EXT_OVERLAP = 0x10;
    constexpr uint8 ENABLE_DISPLAY = 0x20;
    constexpr uint8 ENABLE_DOT_GRID = 0x40;
    constexpr uint8 ENABLE_FILL_MODE = 0x80;
}

// Status register (0xA1) bit definitions
namespace StatusBits {
    constexpr uint8 HBLANK = 0x01;
    constexpr uint8 POS_STROBE_STATUS = 0x02;
    constexpr uint8 SOUND_NEEDS_SERVICE = 0x04;
    constexpr uint8 VBLANK = 0x08;
    constexpr uint8 EXT_OVERLAP = 0x40;
    constexpr uint8 CHAR_OVERLAP = 0x80;
}

// Collision register (0xA2) bit definitions
namespace CollisionBits {
    constexpr uint8 SPRITE0 = 0x01;
    constexpr uint8 SPRITE1 = 0x02;
    constexpr uint8 SPRITE2 = 0x04;
    constexpr uint8 SPRITE3 = 0x08;
    constexpr uint8 VERT_GRID = 0x10;
    constexpr uint8 HORIZ_GRID = 0x20;
    constexpr uint8 EXT_COLLISION = 0x40;
    constexpr uint8 CHARACTERS = 0x80;
}

// Sprite color register (byte 2) bit definitions
namespace SpriteColorBits {
    constexpr uint8 SHIFT_FULL = 0x01;
    constexpr uint8 SHIFT_EVEN = 0x02;
    constexpr uint8 DOUBLE_SIZE = 0x04;
    constexpr uint8 COLOR_MASK = 0x38;
    constexpr uint8 COLOR_SHIFT = 3;
}

// Sound control register (0xAA) bit definitions
namespace SoundControlBits {
    constexpr uint8 VOLUME_MASK = 0x0F;
    constexpr uint8 ENABLE_NOISE = 0x10;
    constexpr uint8 SHIFT_FREQ = 0x20;
    constexpr uint8 LOOP_MODE = 0x40;
    constexpr uint8 ENABLE_SOUND = 0x80;
}

constexpr uint16 AUDIO_FREQ_LOW = 983;
constexpr uint16 AUDIO_FREQ_HIGH = 3933;

// Video timing constants
// The master clock is the single source of truth for all timing.
// These constants are kept for backward compatibility with tests.
namespace VideoTiming {
    constexpr uint16 NTSC_SCANLINES = 262;
    constexpr uint16 NTSC_VBLANK_START = 242;
    constexpr uint16 PAL_SCANLINES = 312;
    constexpr uint16 PAL_VBLANK_START = 284;
    // Blanking start in VDC X coordinates (master tick 366 / 2 = 183)
    constexpr uint16 BLANKING_START_X = 183;
}

// VDC state structure
struct VDCState {
    // Memory-mapped registers (0x00-0xFF)
    uint8 registers[256];
    
    // Framebuffer output (160x200 pixels, palette indices 0-7)
    uint8 framebuffer[FRAMEBUFFER_HEIGHT][FRAMEBUFFER_WIDTH];
    
    // Extended debug framebuffer (240x250 pixels)
    uint8 extended_framebuffer[EXTENDED_FB_HEIGHT][EXTENDED_FB_WIDTH];
    
    // Video timing state — these are now CACHED copies of master clock state,
    // kept for save/restore and debugger display. The master clock is authoritative.
    uint16 beam_x;              // Cached X position (for debugger/savestate)
    uint16 beam_y;              // Cached Y position (for debugger/savestate)
    uint64 total_cycles;
    uint64 frame_number;
    VideoStandard video_standard;
    bool frame_complete;
    
    // Collision detection state
    uint8 collision_state;
    bool collision_detected;
    
    // Display enable state
    bool display_enabled;
    bool grid_enabled;
    
    // Latched color register
    uint8 latched_color;
    
    // Previous scanline (for detecting scanline transitions in tick_one_cycle)
    uint16 prev_scanline;
    
    // Latched graphic registers (0x00-0x9F) — snapshot taken at each scanline start
    // Rendering reads from here; CPU writes go to registers[] and take effect next scanline
    // Reference: 8245 datasheet — shift registers load from object registers per scanline
    uint8 latched_registers[160];
    
    // Port 1 P17 luminance enable (set by CPU, used for background/grid color intensity)
    // When P17=1 (luminance enabled), background/grid use dark palette (indices 0-7)
    // When P17=0 (luminance disabled), background/grid use bright palette (indices 8-15)
    // Reference: o2doc section 1.1, o2em vmachine.c ColorVector
    bool luminance_enabled;
    
    // Audio state
    uint32 audio_shift_register;
    uint8 audio_shift_counter;
    uint16 audio_frequency;
    uint8 audio_volume;
    bool audio_enabled;
    bool audio_loop;
    bool audio_noise;
    uint32 audio_cycle_accumulator;
    
    // DC offset detection
    uint32 cycles_since_toggle;    // VDC cycles since shift register output bit last changed
    uint8 previous_output_bit;     // Previous value of shift register bit 0
    
    // Low-pass filter
    int16 audio_filter_state;      // Previous filtered sample (IIR state)
    
    // Audio sample ring buffer
    static constexpr size_t AUDIO_BUFFER_SIZE = 1024;
    int16 audio_sample_buffer[AUDIO_BUFFER_SIZE];
    uint16 audio_sample_write_pos;
    uint16 audio_sample_count;
    uint32 audio_sample_accumulator;
    
    // Character ROM data
    uint8 character_rom[64 * 8];
};

// Intel 8245 VDC emulation
class VDC {
public:
    explicit VDC(VideoStandard standard = VideoStandard::NTSC);
    ~VDC() = default;
    
    // Core interface
    void reset();
    void tick(uint8 cycles);
    void tick_one_cycle();
    
    // Register access
    void write_register(uint8 address, uint8 value);
    uint8 read_register(uint8 address);
    
    // Rendering
    void render_scanline();
    void render_current_pixel();
    const uint8* get_framebuffer() const;
    
    // Extended debug framebuffer
    void set_extended_framebuffer_mode(bool enabled);
    bool is_extended_framebuffer_mode() const { return extended_fb_mode_; }
    const uint8* get_extended_framebuffer() const;
    int get_extended_framebuffer_width() const { return EXTENDED_FB_WIDTH; }
    int get_extended_framebuffer_height() const { return EXTENDED_FB_HEIGHT; }
    
    // Status queries — inlined for performance, delegate to master clock
    bool is_hblank() const {
        if (master_clock_) return master_clock_->is_hblank();
        return (state_.beam_x >= VideoTiming::BLANKING_START_X);
    }
    bool is_vblank() const {
        if (master_clock_) return master_clock_->is_vblank();
        if (state_.beam_y == vblank_start_) return (state_.beam_x >= VideoTiming::BLANKING_START_X);
        else if (state_.beam_y == 0) return (state_.beam_x < VideoTiming::BLANKING_START_X);
        else if (state_.beam_y > vblank_start_) return true;
        return false;
    }
    bool is_frame_complete() const {
        if (master_clock_) return master_clock_->is_frame_complete();
        return state_.frame_complete;
    }
    void clear_frame_complete() {
        state_.frame_complete = false;
    }
    
    // Audio
    int16 get_audio_sample();
    void set_audio_sample_rate(uint32 sample_rate);
    uint16 get_audio_sample_count() const { return state_.audio_sample_count; }
    const int16* get_audio_sample_buffer() const { return state_.audio_sample_buffer; }
    void reset_audio_sample_buffer();
    
    // State management
    VDCState get_state() const;
    void set_state(const VDCState& state);
    void get_character_rom(uint8* dest) const;
    
    // Accessors — beam position comes from master clock (inlined for performance)
    uint16 get_scanline() const {
        if (master_clock_) return master_clock_->get_scanline();
        return state_.beam_y;
    }
    uint16 get_beam_x() const {
        if (master_clock_) return master_clock_->get_beam_x();
        return state_.beam_x;
    }
    uint16 get_beam_y() const {
        if (master_clock_) return master_clock_->get_scanline();
        return state_.beam_y;
    }
    uint64 get_total_cycles() const { return state_.total_cycles; }
    uint64 get_frame_number() const { return state_.frame_number; }
    VideoStandard get_video_standard() const { return state_.video_standard; }
    
    // Connect to master clock (must be called before first tick)
    void set_master_clock(const MasterClock* clock) { master_clock_ = clock; }
    
    // Set VBLANK status flag (A1.3) — called by emulator at VBlank transition
    void set_vblank_flag() { state_.registers[VDCRegisters::STATUS] |= StatusBits::VBLANK; }
    
    // Port 1 P17 luminance control — called by CPU on Port 1 writes
    void set_luminance_enabled(bool enabled) { state_.luminance_enabled = enabled; }
    
    // T1 pin output — delegates to master clock (inlined for performance)
    bool get_t1_state() const {
        if (master_clock_) return master_clock_->get_t1_state();
        return !(is_hblank() || is_vblank());
    }
    
    // VDC trace
    void enable_vdc_trace(bool enabled) { vdc_trace_enabled_ = enabled; }
    bool is_vdc_trace_enabled() const { return vdc_trace_enabled_; }
    std::string get_last_vdc_trace() const { return last_vdc_trace_; }
    void clear_last_vdc_trace() { last_vdc_trace_.clear(); }

    // Legacy: end_scanline() for tests that don't use master clock
    void end_scanline();

private:
    VDCState state_;
    bool extended_fb_mode_;
    bool vdc_trace_enabled_;
    std::string last_vdc_trace_;
    
    // Master clock reference (single source of truth for beam position)
    const MasterClock* master_clock_;
    
    // Latched Y value (latched when X register is read)
    uint8 latched_beam_y_;
    
    // Timing (standard-specific, for tests without master clock)
    uint32 total_scanlines_;
    uint32 vblank_start_;
    
    // Character ROM
    static const uint8 character_rom_[64 * 8];
    
    // Rendering helpers (scanline-based, for tests)
    void render_background(int y);
    void render_grid(int y);
    void render_characters(int y);
    void render_sprites(int y);
    void detect_collisions(int y);
    
    // Per-pixel rendering helpers
    bool is_grid_pixel_at(int x, int y) const;
    bool is_character_pixel_at(int x, int y, uint8& color) const;
    bool is_sprite_pixel_at(int x, int y, uint8& color) const;
    
    // Per-pixel collision detection
    void detect_collision_at_pixel(int x, int y);
    
    // Collision tracking helpers (scanline-based, for tests)
    void track_grid_objects(int y, uint8* object_buffer, uint8 collision_enable);
    void track_character_objects(int y, uint8* object_buffer, uint8 collision_enable);
    void track_sprite_object(int y, int sprite_num, uint8* object_buffer, uint8 collision_enable);
    
    // Audio helpers
    void update_audio();
    void shift_audio_register();
    void capture_audio_sample();
    void flush_audio_cycles();
    
    uint32 audio_sample_rate_;
    uint32 vdc_cycles_per_audio_sample_;
    
    void calculate_timing();
};

} // namespace videopac

#endif // VIDEOPAC_VDC_H
