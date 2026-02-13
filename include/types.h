#ifndef VIDEOPAC_TYPES_H
#define VIDEOPAC_TYPES_H

#include <cstdint>
#include <string>
#include <optional>

namespace videopac {

// Type aliases for clarity
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using int16 = std::int16_t;

// Video standard
enum class VideoStandard {
    NTSC,  // 60Hz, 262 scanlines
    PAL    // 50Hz, 312 scanlines
};

// Result type for error handling
template<typename T>
struct Result {
    std::optional<T> value;
    std::string error;
    
    bool is_ok() const { return value.has_value(); }
    bool is_err() const { return !value.has_value(); }
    
    static Result ok(T val) {
        Result r;
        r.value = val;
        return r;
    }
    
    static Result err(const std::string& msg) {
        Result r;
        r.error = msg;
        return r;
    }
};

// Specialization for void
template<>
struct Result<void> {
    std::string error;
    
    bool is_ok() const { return error.empty(); }
    bool is_err() const { return !error.empty(); }
    
    static Result ok() {
        return Result{};
    }
    
    static Result err(const std::string& msg) {
        Result r;
        r.error = msg;
        return r;
    }
};

// Color structure
struct Color {
    uint8 r;
    uint8 g;
    uint8 b;
    
    constexpr Color() : r(0), g(0), b(0) {}
    constexpr Color(uint8 red, uint8 green, uint8 blue) : r(red), g(green), b(blue) {}
};

// Videopac 8-color palette (full brightness)
// Reference: Intel 8245 VDC standard digital RGB values
// Used for sprites and characters
constexpr Color PALETTE_BRIGHT[8] = {
    {0x49, 0x49, 0x49},  // 0: Dark Grey
    {0xFF, 0x49, 0x49},  // 1: Red
    {0x49, 0xFF, 0x49},  // 2: Green
    {0xFF, 0xFF, 0x49},  // 3: Yellow (Cyan in some docs)
    {0x49, 0x49, 0xFF},  // 4: Blue
    {0xFF, 0x49, 0xFF},  // 5: Magenta (Violet)
    {0xFF, 0xFF, 0xFF},  // 6: White (Light Grey in some docs)
    {0xFF, 0xFF, 0xFF}   // 7: White
};

// Videopac 8-color palette (half brightness)
// Reference: Intel 8245 VDC standard digital RGB values
// Used for background and grid elements
constexpr Color PALETTE_DIM[8] = {
    {0x00, 0x00, 0x00},  // 0: Black
    {0xB6, 0x00, 0x00},  // 1: Red (half)
    {0x00, 0xB6, 0x00},  // 2: Green (half)
    {0xB6, 0xB6, 0x00},  // 3: Yellow (half)
    {0x00, 0x00, 0xB6},  // 4: Blue (half)
    {0xB6, 0x00, 0xB6},  // 5: Magenta (half)
    {0xB6, 0xB6, 0xB6},  // 6: Light Grey (half)
    {0xB6, 0xB6, 0xB6}   // 7: Light Grey (half)
};

// VDC framebuffer dimensions
// The VDC can render up to Y=240 (or ~242 on PAL), not just 200
// Games like Satellite Attack place status bars at Y=199-207
constexpr int FRAMEBUFFER_WIDTH = 160;
constexpr int FRAMEBUFFER_HEIGHT = 240;  // Full VDC height to capture status bars

// Extended debug framebuffer dimensions (shows area beyond visible display)
// VDC can address Y positions up to ~242 (PAL), X up to 227 (full scanline)
// We'll use a reasonable extended area for debugging
constexpr int EXTENDED_FB_WIDTH = 240;   // Show extra 80 pixels horizontally
constexpr int EXTENDED_FB_HEIGHT = 250;  // Show extra 50 lines vertically

// Timing constants
constexpr uint32 NTSC_SCANLINES = 262;
constexpr uint32 PAL_SCANLINES = 312;
constexpr uint32 NTSC_VBLANK_LINES = 22;
constexpr uint32 PAL_VBLANK_LINES = 28;

constexpr uint32 NTSC_CPU_CLOCK = 5370000;  // 5.37 MHz
constexpr uint32 PAL_CPU_CLOCK = 5910000;   // 5.91 MHz
constexpr uint32 CPU_CLOCK_DIVIDER = 15;

constexpr uint32 NTSC_FRAME_RATE = 60;
constexpr uint32 PAL_FRAME_RATE = 50;

} // namespace videopac

#endif // VIDEOPAC_TYPES_H
