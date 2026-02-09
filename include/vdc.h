#ifndef VIDEOPAC_VDC_H
#define VIDEOPAC_VDC_H

#include "types.h"

namespace videopac {

// VDC state structure
struct VDCState {
    uint8 registers[256];                           // Memory-mapped registers
    uint8 framebuffer[FRAMEBUFFER_HEIGHT][FRAMEBUFFER_WIDTH];  // Output framebuffer (palette indices)
    uint16 scanline;                                // Current scanline
    uint8 beam_x;                                   // Horizontal beam position
    uint8 beam_y;                                   // Vertical beam position
    uint8 collision_state;                          // Collision detection state
    VideoStandard video_standard;                   // PAL or NTSC
    bool enabled;                                   // VDC enabled flag
    
    // Audio state
    uint32 audio_shift_register;                    // 24-bit shift register
    uint16 audio_frequency;                         // Shift frequency (983Hz or 3933Hz)
    uint8 audio_volume;                             // Volume (0-15)
    bool audio_enabled;                             // Audio enable
    bool audio_loop;                                // Loop mode
    bool audio_noise;                               // Noise mode
};

// Intel 8245 VDC emulation
class VDC {
public:
    explicit VDC(VideoStandard standard = VideoStandard::NTSC);
    ~VDC() = default;
    
    // Core interface
    void reset();
    void tick(uint8 cycles);
    
    // Register access
    void write_register(uint8 address, uint8 value);
    uint8 read_register(uint8 address);
    
    // Rendering
    void render_scanline();
    const uint8* get_framebuffer() const;
    
    // Status queries
    bool is_vblank() const;
    bool is_hblank() const;
    
    // Audio
    int16 get_audio_sample();
    
    // State management
    VDCState get_state() const;
    void set_state(const VDCState& state);
    
    // Accessors
    uint16 get_scanline() const { return state_.scanline; }
    VideoStandard get_video_standard() const { return state_.video_standard; }

private:
    VDCState state_;
    
    // Timing
    uint32 cycles_per_scanline_;
    uint32 total_scanlines_;
    uint32 vblank_start_;
    
    // Rendering helpers
    void render_background(int y);
    void render_grid(int y);
    void render_characters(int y);
    void render_sprites(int y);
    void detect_collisions(int y);
    
    // Audio helpers
    void update_audio();
    void shift_audio_register();
    
    // Timing helpers
    void calculate_timing();
};

} // namespace videopac

#endif // VIDEOPAC_VDC_H
