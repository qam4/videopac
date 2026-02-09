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

// Videopac 8-color palette (bright luminance)
constexpr Color PALETTE_BRIGHT[8] = {
    {0x00, 0x00, 0x00},  // 0: Black
    {0x00, 0x00, 0xFF},  // 1: Blue
    {0x00, 0xFF, 0x00},  // 2: Green
    {0x00, 0xFF, 0xFF},  // 3: Cyan
    {0xFF, 0x00, 0x00},  // 4: Red
    {0xFF, 0x00, 0xFF},  // 5: Magenta
    {0xFF, 0xFF, 0x00},  // 6: Yellow
    {0xFF, 0xFF, 0xFF}   // 7: White
};

// Videopac 8-color palette (dim luminance - 50% brightness)
constexpr Color PALETTE_DIM[8] = {
    {0x00, 0x00, 0x00},  // 0: Black
    {0x00, 0x00, 0x7F},  // 1: Blue (dim)
    {0x00, 0x7F, 0x00},  // 2: Green (dim)
    {0x00, 0x7F, 0x7F},  // 3: Cyan (dim)
    {0x7F, 0x00, 0x00},  // 4: Red (dim)
    {0x7F, 0x00, 0x7F},  // 5: Magenta (dim)
    {0x7F, 0x7F, 0x00},  // 6: Yellow (dim)
    {0x7F, 0x7F, 0x7F}   // 7: White (dim)
};

// VDC framebuffer dimensions
constexpr int FRAMEBUFFER_WIDTH = 160;
constexpr int FRAMEBUFFER_HEIGHT = 200;

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
