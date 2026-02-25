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
using int32 = std::int32_t;
using int64 = std::int64_t;

// Video standard (timing only - does not affect colors)
enum class VideoStandard {
    NTSC,  // 60Hz, 262 scanlines - Intel 8244
    PAL    // 50Hz, 312 scanlines - Intel 8245
};

// Palette mode (colors)
// Note: Both NTSC and PAL use the same color mappings (unlike some other systems)
// The main difference is between standard O2/Videopac and the enhanced VP+ G7400
enum class PaletteMode {
    STANDARD,  // Standard O2/Videopac palette (8244/8245) - organic, desaturated colors
    VIDEOPAC_PLUS  // Videopac+ G7400 palette (EF9340/EF9341) - vibrant, digitally-calculated colors
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

// Videopac 16-color palettes
// 
// IMPORTANT: Both NTSC (8244) and PAL (8245) use the SAME color mappings!
// The difference between regions is timing/resolution only, not colors.
// The main palette distinction is between standard O2/Videopac and VP+ G7400.
//
// Color mapping uses special formulas (see render_current_pixel in vdc.cpp):
// - Grid/Background: (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8)
// - Sprites/Characters: ((cl & 2) | ((cl & 1) << 2) | ((cl & 4) >> 2)) + 8
//   This reorders BGR bits to RGB and adds 8 for high-intensity

// Standard O2/Videopac Palette (Intel 8244/8245)
// Used by: Magnavox Odyssey 2, Philips Videopac G7000, French C52
// Characteristics: Organic, slightly desaturated colors mimicking CRT output
constexpr Color PALETTE_STANDARD[16] = {
    // Low-intensity (0-7)
    {0x00, 0x00, 0x00},  // 0: Black
    {0x08, 0x39, 0xD6},  // 1: Dark Blue
    {0x00, 0x9C, 0x18},  // 2: Dark Green
    {0x00, 0xBD, 0xDE},  // 3: Light Blue (Cyan)
    {0xC6, 0x00, 0x08},  // 4: Dark Red
    {0xCE, 0x10, 0xB5},  // 5: Violet (Magenta)
    {0x9C, 0x84, 0x10},  // 6: Orange/Gold (Yellow)
    {0xCE, 0xCE, 0xCE},  // 7: Grey
    // High-intensity (8-15)
    {0x49, 0x49, 0x49},  // 8: Light Grey (Dark Grey)
    {0x49, 0x49, 0xFF},  // 9: Bright Blue
    {0x49, 0xFF, 0x49},  // 10: Bright Green
    {0x49, 0xFF, 0xFF},  // 11: Bright Cyan
    {0xFF, 0x49, 0x49},  // 12: Bright Red
    {0xFF, 0x49, 0xFF},  // 13: Bright Magenta
    {0xFF, 0xFF, 0x49},  // 14: Bright Yellow
    {0xFF, 0xFF, 0xFF}   // 15: White
};

// Videopac+ G7400 Palette (EF9340/EF9341)
// Used by: Videopac+ G7400, Jopac (French VP+)
// Characteristics: Vibrant, digitally-calculated colors with uniform intensity levels
// 
// Formula: VP+ palette is derived from Standard palette by quantizing to digital levels:
//   - Low intensity (0-7): RGB components quantized to {0x00, 0xB6}
//   - High intensity (8-15): RGB components quantized to {0x49, 0xFF}
//   - Quantization threshold: < 0x80 → low value, >= 0x80 → high value
//   - Low value: 0x00 (dark) or 0x49 (bright), High value: 0xB6 (dark) or 0xFF (bright)
constexpr Color PALETTE_VIDEOPAC_PLUS[16] = {
    // Low-intensity (0-7)
    {0x00, 0x00, 0x00},  // 0: Black
    {0x00, 0x00, 0xB6},  // 1: Blue
    {0x00, 0xB6, 0x00},  // 2: Green
    {0x00, 0xB6, 0xB6},  // 3: Cyan
    {0xB6, 0x00, 0x00},  // 4: Red
    {0xB6, 0x00, 0xB6},  // 5: Magenta
    {0xB6, 0xB6, 0x00},  // 6: Yellow
    {0xB6, 0xB6, 0xB6},  // 7: Light Grey
    // High-intensity (8-15)
    {0x49, 0x49, 0x49},  // 8: Dark Grey
    {0x49, 0x49, 0xFF},  // 9: Bright Blue
    {0x49, 0xFF, 0x49},  // 10: Bright Green
    {0x49, 0xFF, 0xFF},  // 11: Bright Cyan
    {0xFF, 0x49, 0x49},  // 12: Bright Red
    {0xFF, 0x49, 0xFF},  // 13: Bright Magenta
    {0xFF, 0xFF, 0x49},  // 14: Bright Yellow
    {0xFF, 0xFF, 0xFF}   // 15: White
};

// Color mapping formulas (implemented in vdc.cpp render_current_pixel)
// Reference: doc/reference/o2doc.md Appendix B, doc/hardware/8245.md
//
// IMPORTANT: These formulas map VDC's BGR color bits to palette indices.
//
// Grid/Background colors (register 0xA3):
//   Grid (bits 0-2):       (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8)
//   Background (bits 3-5): ((color & 0x38) >> 3) | (color & 0x80 ? 0 : 8)
//   - Bits 0-2 (grid) or 3-5 (background): BGR component bits
//   - Bit 6: Luminance (shifted to bit 3 of result for grid)
//   - Bit 7: Inverted luminance (if 0, add 8 to result for bright colors)
//
// Sprite/Character colors (sprite register byte 2 bits 3-5, character register byte 3 bits 1-3):
//   Formula: ((cl & 2) | ((cl & 1) << 2) | ((cl & 4) >> 2)) + 8
//   - Reorders BGR bits to RGB: B→bit2, G→bit1, R→bit0
//   - Adds 8 for high-intensity palette range (colors 8-15)
//
// Example: Register 0xA3 = 0x69 (grid color from Course de Voitures racing game)
//   - Bits 0-2 (BGR): 001 = Blue component only
//   - Bit 6 (luminance): 1 (shifted to bit 3)
//   - Bit 7: 0 (inverted, so add 8)
//   - Result: 0b0001 | 0b1000 | 0b1000 = 0b1001 = 9 = Bright Blue ✓



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

} // namespace videopac

#endif // VIDEOPAC_TYPES_H
