#ifndef VIDEOPAC_VDC_H
#define VIDEOPAC_VDC_H

#include "types.h"

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
    constexpr uint8 CONTROL = 0xA0;           // VDC control register
    constexpr uint8 STATUS = 0xA1;            // VDC status register
    constexpr uint8 COLLISION = 0xA2;         // Collision register
    constexpr uint8 COLOR = 0xA3;             // Color register
    constexpr uint8 BEAM_X = 0xA4;            // X beam position (horizontal)
    constexpr uint8 BEAM_Y = 0xA5;            // Y beam position (vertical)
    
    // Audio registers
    constexpr uint8 SOUND0 = 0xA7;            // Sound shift register byte 0
    constexpr uint8 SOUND1 = 0xA8;            // Sound shift register byte 1
    constexpr uint8 SOUND2 = 0xA9;            // Sound shift register byte 2
    constexpr uint8 SOUND_CONTROL = 0xAA;     // Sound control register
    
    // Grid registers
    constexpr uint8 GRID_H_BASE = 0xC0;       // Horizontal grid lines 0-8 (0xC0-0xC8)
    constexpr uint8 GRID_H9_BASE = 0xD0;      // Horizontal grid line 9 (0xD0-0xD8)
    constexpr uint8 GRID_V_BASE = 0xE0;       // Vertical grid lines (0xE0-0xE9)
}

// Grid layout constants
// Values derived from o2em implementation and hardware testing
// Reference: o2em vdc.c, Intel 8245 datasheet
namespace GridLayout {
    constexpr int START_X = 8;           // Horizontal start position (adjusted for alignment)
    constexpr int START_Y = 24;          // Vertical start position (hardware specification)
    constexpr int ROW_HEIGHT = 24;       // Each row spans 24 scanlines (3 line + 21 spacing)
    constexpr int LINE_HEIGHT = 3;       // Grid lines are 3 scanlines thick
    constexpr int COL_WIDTH = 16;        // Each column is 16 pixels wide
}

// Control register (0xA0) bit definitions
// Reference: doc/o2doc.md section 4.6, doc/8245.md lines 440-470
namespace ControlBits {
    constexpr uint8 ENABLE_HBLANK_INT = 0x01;  // Bit 0: Enable horizontal interrupt
    constexpr uint8 LATCH_BEAM_POS = 0x02;     // Bit 1: Latch beam position
    constexpr uint8 ENABLE_SOUND_INT = 0x04;   // Bit 2: Enable sound interrupt
    constexpr uint8 ENABLE_GRID = 0x08;        // Bit 3: Enable grid
    constexpr uint8 ENABLE_EXT_OVERLAP = 0x10; // Bit 4: Enable external overlap (unused in O2)
    constexpr uint8 ENABLE_DISPLAY = 0x20;     // Bit 5: Enable display
    constexpr uint8 ENABLE_DOT_GRID = 0x40;    // Bit 6: Enable dot grid
    constexpr uint8 ENABLE_FILL_MODE = 0x80;   // Bit 7: Enable fill mode
}

// Status register (0xA1) bit definitions
// Reference: doc/o2doc.md section 4.7, doc/8245.md lines 480-500
namespace StatusBits {
    constexpr uint8 HBLANK = 0x01;             // Bit 0: Horizontal blank active
    constexpr uint8 POS_STROBE_STATUS = 0x02;  // Bit 1: Position strobe status
    constexpr uint8 SOUND_NEEDS_SERVICE = 0x04;// Bit 2: Sound register empty
    constexpr uint8 VBLANK = 0x08;             // Bit 3: Vertical blank active
    constexpr uint8 EXT_OVERLAP = 0x40;        // Bit 6: External chip overlap
    constexpr uint8 CHAR_OVERLAP = 0x80;       // Bit 7: Character overlap
}

// Collision register (0xA2) bit definitions
// Reference: doc/o2doc.md section 4.8, doc/8245.md lines 500-520
namespace CollisionBits {
    constexpr uint8 SPRITE0 = 0x01;            // Bit 0: Sprite 0
    constexpr uint8 SPRITE1 = 0x02;            // Bit 1: Sprite 1
    constexpr uint8 SPRITE2 = 0x04;            // Bit 2: Sprite 2
    constexpr uint8 SPRITE3 = 0x08;            // Bit 3: Sprite 3
    constexpr uint8 VERT_GRID = 0x10;          // Bit 4: Vertical grid
    constexpr uint8 HORIZ_GRID = 0x20;         // Bit 5: Horizontal grid and dots
    constexpr uint8 EXT_COLLISION = 0x40;      // Bit 6: External collision (unused in O2)
    constexpr uint8 CHARACTERS = 0x80;         // Bit 7: Characters
}

// Sprite color register (byte 2) bit definitions
// Reference: doc/o2doc.md section 4.3.1
namespace SpriteColorBits {
    constexpr uint8 SHIFT_FULL = 0x01;         // Bit 0: Shift sprite 1 pixel right
    constexpr uint8 SHIFT_EVEN = 0x02;         // Bit 1: Shift even rows 1 pixel right
    constexpr uint8 DOUBLE_SIZE = 0x04;        // Bit 2: Double size sprite (16x16)
    constexpr uint8 COLOR_MASK = 0x38;         // Bits 3-5: Sprite color (0-7)
    constexpr uint8 COLOR_SHIFT = 3;
}

// Sound control register (0xAA) bit definitions
// Reference: doc/o2doc.md section 4.10, doc/8245.md lines 380-420
namespace SoundControlBits {
    constexpr uint8 VOLUME_MASK = 0x0F;        // Bits 0-3: Volume (0-15)
    constexpr uint8 ENABLE_NOISE = 0x10;       // Bit 4: Enable noise generation
    constexpr uint8 SHIFT_FREQ = 0x20;         // Bit 5: Shift frequency (0=983Hz, 1=3933Hz)
    constexpr uint8 LOOP_MODE = 0x40;          // Bit 6: Loop mode
    constexpr uint8 ENABLE_SOUND = 0x80;       // Bit 7: Enable sound
}

// Audio frequencies (Hz)
// Reference: doc/o2doc.md section 4.10, doc/8245.md lines 400-410
constexpr uint16 AUDIO_FREQ_LOW = 983;         // Low shift frequency
constexpr uint16 AUDIO_FREQ_HIGH = 3933;       // High shift frequency

// Video timing constants
// Reference: doc/o2doc.md section 4.11, doc/8245.md lines 520-560
namespace VideoTiming {
    // NTSC timing (60Hz)
    constexpr uint16 NTSC_SCANLINES = 262;
    
    // VBlank start scanline
    // Hardware spec: 240 (VBlank starts after 240 visible scanlines)
    // O2EM uses: 241 (due to scanline batching implementation)
    // Use O2EM_VBLANK_TIMING compile flag to match o2em for trace comparison debugging
    #ifdef O2EM_VBLANK_TIMING
        constexpr uint16 NTSC_VBLANK_START = 241;  // O2EM-compatible timing for debugging
    #else
        constexpr uint16 NTSC_VBLANK_START = 240;  // Hardware-accurate timing (default)
    #endif
    
    constexpr uint16 NTSC_VBLANK_LINES = 22;
    
    // Cycles per scanline (integer approximation)
    // Hardware: ~227.5 cycles/scanline (3.579545 MHz / 59.94 Hz / 262 scanlines)
    // Using 227 as integer approximation: 262 × 227 = 59,474 cycles/frame
    constexpr uint32 NTSC_CYCLES_PER_SCANLINE = 227;
    
    // PAL timing (50Hz)
    constexpr uint16 PAL_SCANLINES = 312;
    constexpr uint16 PAL_VBLANK_START = 284;
    constexpr uint16 PAL_VBLANK_LINES = 28;
    
    // Cycles per scanline (integer approximation)
    // Hardware: ~227.36 cycles/scanline (3.546895 MHz / 50 Hz / 312 scanlines)
    // Using 227 as integer approximation: 312 × 227 = 70,824 cycles/frame
    constexpr uint32 PAL_CYCLES_PER_SCANLINE = 227;
}

// VDC state structure
// References: doc/o2doc.md (sections 4.0-4.14), doc/8245.md (lines 200-700)
struct VDCState {
    // Memory-mapped registers (0x00-0xFF)
    // Reference: doc/o2doc.md Appendix D, doc/8245.md lines 600-650
    uint8 registers[256];                           // All VDC registers
    
    // Framebuffer output (160x200 pixels, palette indices 0-7)
    // Reference: doc/o2doc.md section 4.0, doc/8245.md lines 1-30
    uint8 framebuffer[FRAMEBUFFER_HEIGHT][FRAMEBUFFER_WIDTH];
    
    // Extended debug framebuffer (240x250 pixels, shows area beyond visible display)
    // Only used when extended_fb_mode is enabled
    uint8 extended_framebuffer[EXTENDED_FB_HEIGHT][EXTENDED_FB_WIDTH];
    
    // Video timing state
    // Reference: doc/o2doc.md section 4.11, doc/8245.md lines 520-560
    uint16 beam_x;                                  // Horizontal beam position (0-227 for full scanline including HBLANK)
    uint16 beam_y;                                  // Vertical beam position (0-261 NTSC, 0-311 PAL)
    uint64 total_cycles;                            // Total VDC cycles since reset
    VideoStandard video_standard;                   // PAL or NTSC
    bool frame_complete;                            // Frame just completed (beam wrapped to scanline 0)
    
    // Collision detection state
    // Reference: doc/o2doc.md section 4.8, doc/8245.md lines 480-500
    uint8 collision_state;                          // Current collision bits (register 0xA2)
    bool collision_detected;                        // Collision occurred this frame
    
    // Display enable state
    // Reference: doc/o2doc.md section 4.6, doc/8245.md lines 440-470
    bool display_enabled;                           // Display enable (bit 5 of 0xA0)
    bool grid_enabled;                              // Grid enable (bit 3 of 0xA0)
    
    // Audio state (24-bit shift register system)
    // Reference: doc/o2doc.md section 4.10, doc/8245.md lines 380-420
    uint32 audio_shift_register;                    // 24-bit shift register (registers 0xA7-0xA9)
    uint8 audio_shift_counter;                      // Shift counter (0-23)
    uint16 audio_frequency;                         // Shift frequency (983Hz or 3933Hz)
    uint8 audio_volume;                             // Volume (0-15, bits 0-3 of 0xAA)
    bool audio_enabled;                             // Audio enable (bit 7 of 0xAA)
    bool audio_loop;                                // Loop mode (bit 6 of 0xAA)
    bool audio_noise;                               // Noise mode (bit 4 of 0xAA)
    uint32 audio_cycle_accumulator;                 // Cycle accumulator for audio timing
    
    // Character ROM data (64 characters, 8 bytes each)
    // Reference: doc/o2doc.md Appendix C
    uint8 character_rom[64 * 8];                    // Character pattern ROM
};

// Intel 8245 VDC emulation
class VDC {
public:
    explicit VDC(VideoStandard standard = VideoStandard::NTSC);
    ~VDC() = default;
    
    // Core interface
    void reset();
    void tick(uint8 cycles);
    void tick_one_cycle();                          // Advance VDC by exactly 1 clock cycle
    
    // Register access
    void write_register(uint8 address, uint8 value);
    uint8 read_register(uint8 address);
    
    // Rendering
    void render_scanline();
    void render_current_pixel();                    // Render pixel at current beam position
    const uint8* get_framebuffer() const;
    
    // Extended debug framebuffer (shows area beyond visible 160×200)
    void set_extended_framebuffer_mode(bool enabled);
    bool is_extended_framebuffer_mode() const { return extended_fb_mode_; }
    const uint8* get_extended_framebuffer() const;
    int get_extended_framebuffer_width() const { return EXTENDED_FB_WIDTH; }
    int get_extended_framebuffer_height() const { return EXTENDED_FB_HEIGHT; }
    
    // Status queries
    bool is_vblank() const;
    bool is_hblank() const;
    bool is_beam_visible() const;                   // Check if beam is in visible area
    bool is_frame_complete() const;                 // Check if frame just completed (beam wrapped to scanline 0)
    
    // Audio
    int16 get_audio_sample();
    
    // State management
    VDCState get_state() const;
    void set_state(const VDCState& state);
    void get_character_rom(uint8* dest) const;     // Copy character ROM data (for debugger)
    
    // Accessors
    uint16 get_scanline() const { return state_.beam_y; }  // For backward compatibility
    uint16 get_beam_x() const { return state_.beam_x; }
    uint16 get_beam_y() const { return state_.beam_y; }
    uint64 get_total_cycles() const { return state_.total_cycles; }
    uint64 get_frame_number() const;                    // Calculate current frame number from total cycles
    VideoStandard get_video_standard() const { return state_.video_standard; }
    
    // VDC trace (for debugging VDC register writes)
    void enable_vdc_trace(bool enabled) { vdc_trace_enabled_ = enabled; }
    bool is_vdc_trace_enabled() const { return vdc_trace_enabled_; }
    std::string get_last_vdc_trace() const { return last_vdc_trace_; }
    void clear_last_vdc_trace() { last_vdc_trace_.clear(); }

private:
    VDCState state_;
    
    // Extended framebuffer mode flag
    bool extended_fb_mode_;
    
    // VDC trace
    bool vdc_trace_enabled_;
    std::string last_vdc_trace_;
    
    // Timing
    uint32 cycles_per_scanline_;  // VDC cycles per scanline (standard-specific)
    uint32 total_scanlines_;
    uint32 vblank_start_;
    
    // Character ROM (64 characters, 8 bytes each for 8x7 patterns)
    // Reference: doc/o2doc.md Appendix C, doc/8245.md lines 700-750
    // Internal ROM patterns from Intel 8245 VDC chip
    static const uint8 character_rom_[64 * 8];
    
    // Rendering helpers
    void render_background(int y);
    void render_grid(int y);
    void render_characters(int y);
    void render_sprites(int y);
    void detect_collisions(int y);
    
    // Per-pixel rendering helpers (for continuous rendering)
    bool is_grid_pixel_at(int x, int y) const;
    bool is_character_pixel_at(int x, int y, uint8& color) const;
    bool is_sprite_pixel_at(int x, int y, uint8& color) const;
    
    // Collision tracking helpers
    void track_grid_objects(int y, uint8* object_buffer, uint8 collision_enable);
    void track_character_objects(int y, uint8* object_buffer, uint8 collision_enable);
    void track_sprite_object(int y, int sprite_num, uint8* object_buffer, uint8 collision_enable);
    
    // Audio helpers
    void update_audio();
    void shift_audio_register();
    
    // Timing helpers
    void calculate_timing();
};

} // namespace videopac

#endif // VIDEOPAC_VDC_H
