// Virtual keyboard overlay for the videopac libretro core.
// Contains the 49-key LAYOUT constant array and VirtualKeyboard constructor.
// Other methods (toggle, move, render, etc.) are added in later tasks.

#include "vkeyboard.h"
#include "vkeyboard_font.h"

namespace videopac {

// -------------------------------------------------------------------------
// Layout geometry constants (VKB-local pixel coordinates)
// -------------------------------------------------------------------------
// VKB area: 160 x 93 pixels (7 rows)
// Standard key: 14 x 12 pixels, 2px horizontal gap, 1px vertical gap
// 10-key rows: x starts at 1, stride 16  (1 + 9*16 + 14 = 160)
//  9-key rows: x starts at 9, stride 16  (centered in 160px)
//  5-key rows: 30px wide keys, x starts at 3, stride 31 (centered in 160px)
//  1-key row:  wide key 28 x 12, centered at x=66
//
// Row y positions (12px key + 1px gap = 13px per row):
//   Row 0: y= 1    Row 1: y=14    Row 2: y=27    Row 3: y=40
//   Row 4: y=53    Row 5: y=66    Row 6: y=79
//
// Index ranges:
//   Row 0 [ 0.. 9] digits     0-9
//   Row 1 [10..14] math       + - * / =
//   Row 2 [15..19] action     YES NO CLR ENT RST
//   Row 3 [20..29] QWERTY     Q W E R T Y U I O P
//   Row 4 [30..38] mid        A S D F G H J K L
//   Row 5 [39..47] bottom     Z X C V B N M . ?
//   Row 6 [48]     wide       SPC
// -------------------------------------------------------------------------

// Helper macros for readability
#define K10(i) static_cast<uint8>(1 + (i) * 16)   // x for 10-key row
#define K9(i)  static_cast<uint8>(9 + (i) * 16)   // x for 9-key row
#define K5(i)  static_cast<uint8>(3 + (i) * 31)   // x for 5-key row
#define KW     14   // standard key width
#define KH     12   // standard key height
#define WW     28   // wide key width
#define FW     30   // function key width (5-key rows)

const VKBKey VirtualKeyboard::LAYOUT[KEY_COUNT] = {
    // =====================================================================
    // Row 0 — digits (indices 0-9, y=1)
    // nav_up → row 6 (index 48), nav_down → row 1 (nearest 5-key)
    // Row 0→1 mapping (10→5): 0,1→10  2,3→11  4,5→12  6,7→13  8,9→14
    // =====================================================================
    // idx  label   x        y  w   h   vidkey          up  dn  lt  rt
    /*  0*/ {"0",  K10(0),   1, KW, KH, VidKey::Key0,   48, 10,  9,  1},
    /*  1*/ {"1",  K10(1),   1, KW, KH, VidKey::Key1,   48, 10,  0,  2},
    /*  2*/ {"2",  K10(2),   1, KW, KH, VidKey::Key2,   48, 11,  1,  3},
    /*  3*/ {"3",  K10(3),   1, KW, KH, VidKey::Key3,   48, 11,  2,  4},
    /*  4*/ {"4",  K10(4),   1, KW, KH, VidKey::Key4,   48, 12,  3,  5},
    /*  5*/ {"5",  K10(5),   1, KW, KH, VidKey::Key5,   48, 12,  4,  6},
    /*  6*/ {"6",  K10(6),   1, KW, KH, VidKey::Key6,   48, 13,  5,  7},
    /*  7*/ {"7",  K10(7),   1, KW, KH, VidKey::Key7,   48, 13,  6,  8},
    /*  8*/ {"8",  K10(8),   1, KW, KH, VidKey::Key8,   48, 14,  7,  9},
    /*  9*/ {"9",  K10(9),   1, KW, KH, VidKey::Key9,   48, 14,  8,  0},

    // =====================================================================
    // Row 1 — math operators (indices 10-14, y=14, 5 keys × 30px)
    // nav_up → row 0 (nearest), nav_down → row 2 (same column)
    // Row 1→0 mapping (5→10): 10→0  11→2  12→4  13→6  14→8
    // =====================================================================
    /* 10*/ {"+",   K5(0),  14, FW, KH, VidKey::Plus,      0, 15, 14, 11},
    /* 11*/ {"-",   K5(1),  14, FW, KH, VidKey::Minus,     2, 16, 10, 12},
    /* 12*/ {"*",   K5(2),  14, FW, KH, VidKey::Multiply,  4, 17, 11, 13},
    /* 13*/ {"/",   K5(3),  14, FW, KH, VidKey::Divide,    6, 18, 12, 14},
    /* 14*/ {"=",   K5(4),  14, FW, KH, VidKey::Equal,     8, 19, 13, 10},

    // =====================================================================
    // Row 2 — action keys (indices 15-19, y=27, 5 keys × 30px)
    // nav_up → row 1 (same column), nav_down → row 3 (nearest 10-key)
    // Row 2→3 mapping (5→10): 15→20  16→22  17→24  18→26  19→28
    // =====================================================================
    /* 15*/ {"YES", K5(0),  27, FW, KH, VidKey::KeyY,     10, 20, 19, 16},
    /* 16*/ {"NO",  K5(1),  27, FW, KH, VidKey::KeyN,     11, 22, 15, 17},
    /* 17*/ {"CLR", K5(2),  27, FW, KH, VidKey::Clear,    12, 24, 16, 18},
    /* 18*/ {"ENT", K5(3),  27, FW, KH, VidKey::Enter,    13, 26, 17, 19},
    /* 19*/ {"RST", K5(4),  27, FW, KH, VidKey::Reset,    14, 28, 18, 15},

    // =====================================================================
    // Row 3 — QWERTY top (indices 20-29, y=40)
    // nav_up → row 2 (nearest 5-key), nav_down → row 4 (nearest 9-key)
    // Row 3→2 mapping (10→5): 20,21→15  22,23→16  24,25→17  26,27→18  28,29→19
    // Row 3→4 mapping (10→9): 20→30  21→30  22→31  23→32  24→33  25→34
    //                          26→35  27→36  28→37  29→38
    // =====================================================================
    /* 20*/ {"Q",  K10(0),  40, KW, KH, VidKey::KeyQ,   15, 30, 29, 21},
    /* 21*/ {"W",  K10(1),  40, KW, KH, VidKey::KeyW,   15, 30, 20, 22},
    /* 22*/ {"E",  K10(2),  40, KW, KH, VidKey::KeyE,   16, 31, 21, 23},
    /* 23*/ {"R",  K10(3),  40, KW, KH, VidKey::KeyR,   16, 32, 22, 24},
    /* 24*/ {"T",  K10(4),  40, KW, KH, VidKey::KeyT,   17, 33, 23, 25},
    /* 25*/ {"Y",  K10(5),  40, KW, KH, VidKey::KeyY,   17, 34, 24, 26},
    /* 26*/ {"U",  K10(6),  40, KW, KH, VidKey::KeyU,   18, 35, 25, 27},
    /* 27*/ {"I",  K10(7),  40, KW, KH, VidKey::KeyI,   18, 36, 26, 28},
    /* 28*/ {"O",  K10(8),  40, KW, KH, VidKey::KeyO,   19, 37, 27, 29},
    /* 29*/ {"P",  K10(9),  40, KW, KH, VidKey::KeyP,   19, 38, 28, 20},

    // =====================================================================
    // Row 4 — QWERTY middle (indices 30-38, y=53)
    // nav_up → row 3 (nearest), nav_down → row 5 (same column)
    // Row 4→3 mapping (9→10): 30→20  31→22  32→23  33→24  34→25
    //                          35→26  36→27  37→28  38→29
    // =====================================================================
    /* 30*/ {"A",  K9(0),   53, KW, KH, VidKey::KeyA,   20, 39, 38, 31},
    /* 31*/ {"S",  K9(1),   53, KW, KH, VidKey::KeyS,   22, 40, 30, 32},
    /* 32*/ {"D",  K9(2),   53, KW, KH, VidKey::KeyD,   23, 41, 31, 33},
    /* 33*/ {"F",  K9(3),   53, KW, KH, VidKey::KeyF,   24, 42, 32, 34},
    /* 34*/ {"G",  K9(4),   53, KW, KH, VidKey::KeyG,   25, 43, 33, 35},
    /* 35*/ {"H",  K9(5),   53, KW, KH, VidKey::KeyH,   26, 44, 34, 36},
    /* 36*/ {"J",  K9(6),   53, KW, KH, VidKey::KeyJ,   27, 45, 35, 37},
    /* 37*/ {"K",  K9(7),   53, KW, KH, VidKey::KeyK,   28, 46, 36, 38},
    /* 38*/ {"L",  K9(8),   53, KW, KH, VidKey::KeyL,   29, 47, 37, 30},

    // =====================================================================
    // Row 5 — QWERTY bottom (indices 39-47, y=66)
    // nav_up → row 4 (same column), nav_down → row 6 (index 48)
    // =====================================================================
    /* 39*/ {"Z",  K9(0),   66, KW, KH, VidKey::KeyZ,   30, 48, 47, 40},
    /* 40*/ {"X",  K9(1),   66, KW, KH, VidKey::KeyX,   31, 48, 39, 41},
    /* 41*/ {"C",  K9(2),   66, KW, KH, VidKey::KeyC,   32, 48, 40, 42},
    /* 42*/ {"V",  K9(3),   66, KW, KH, VidKey::KeyV,   33, 48, 41, 43},
    /* 43*/ {"B",  K9(4),   66, KW, KH, VidKey::KeyB,   34, 48, 42, 44},
    /* 44*/ {"N",  K9(5),   66, KW, KH, VidKey::KeyN,   35, 48, 43, 45},
    /* 45*/ {"M",  K9(6),   66, KW, KH, VidKey::KeyM,   36, 48, 44, 46},
    /* 46*/ {".",  K9(7),   66, KW, KH, VidKey::Period,  37, 48, 45, 47},
    /* 47*/ {"?",  K9(8),   66, KW, KH, VidKey::Slash,   38, 48, 46, 39},

    // =====================================================================
    // Row 6 — space bar (index 48, y=79, wide key)
    // nav_up → row 5 (nearest = index 43, B key)
    // nav_down → row 0 (nearest = index 4)
    // left/right stay (single key in row)
    // =====================================================================
    /* 48*/ {"SPC", 66,      79, WW, KH, VidKey::Space,  43,  4, -1, -1},
};

#undef K10
#undef K9
#undef K5
#undef KW
#undef KH
#undef WW
#undef FW

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
VirtualKeyboard::VirtualKeyboard()
    : visible_(false)
    , position_bottom_(true)
    , cursor_index_(0)
{
}

// -------------------------------------------------------------------------
// State accessors and toggles (Task 2.1)
// -------------------------------------------------------------------------

bool VirtualKeyboard::is_visible() const
{
    return visible_;
}

void VirtualKeyboard::toggle_visible()
{
    visible_ = !visible_;
}

void VirtualKeyboard::toggle_position()
{
    position_bottom_ = !position_bottom_;
}

int VirtualKeyboard::get_cursor_index() const
{
    return cursor_index_;
}

VidKey VirtualKeyboard::get_current_vidkey() const
{
    return LAYOUT[cursor_index_].vidkey;
}

// -------------------------------------------------------------------------
// Cursor navigation (Task 2.2)
// -------------------------------------------------------------------------

void VirtualKeyboard::move_cursor(Direction dir)
{
    int8_t link = -1;
    switch (dir) {
        case Direction::Up:    link = LAYOUT[cursor_index_].nav_up;    break;
        case Direction::Down:  link = LAYOUT[cursor_index_].nav_down;  break;
        case Direction::Left:  link = LAYOUT[cursor_index_].nav_left;  break;
        case Direction::Right: link = LAYOUT[cursor_index_].nav_right; break;
    }

    if (link >= 0) {
        cursor_index_ = link;
    }

    // Defensive clamp
    if (cursor_index_ < 0) cursor_index_ = 0;
    if (cursor_index_ >= KEY_COUNT) cursor_index_ = KEY_COUNT - 1;
}

void VirtualKeyboard::set_cursor(int index)
{
    if (index >= 0 && index < KEY_COUNT) {
        cursor_index_ = index;
    }
}

// -------------------------------------------------------------------------
// Hit testing and key lookup (Task 2.3)
// -------------------------------------------------------------------------

int VirtualKeyboard::hit_test(int vkb_local_x, int vkb_local_y) const
{
    for (int i = 0; i < KEY_COUNT; ++i) {
        const VKBKey& key = LAYOUT[i];
        if (vkb_local_x >= key.x && vkb_local_x < key.x + key.width &&
            vkb_local_y >= key.y && vkb_local_y < key.y + key.height) {
            return i;
        }
    }
    return -1;
}

VidKey VirtualKeyboard::get_vidkey_at(int index) const
{
    if (index < 0 || index >= KEY_COUNT) {
        return VidKey::Key0;
    }
    return LAYOUT[index].vidkey;
}

// -------------------------------------------------------------------------
// Rendering helpers (Task 3.1)
// -------------------------------------------------------------------------

int VirtualKeyboard::get_vkb_y_offset(int fb_height) const
{
    return position_bottom_ ? fb_height - VKB_HEIGHT : 0;
}

void VirtualKeyboard::draw_rect(uint32_t* fb, int fb_width, int fb_height,
                                int x, int y, int w, int h,
                                uint32_t color, uint8_t alpha) const
{
    uint8_t ov_r = (color >> 16) & 0xFF;
    uint8_t ov_g = (color >>  8) & 0xFF;
    uint8_t ov_b = (color      ) & 0xFF;
    uint8_t inv_alpha = 255 - alpha;

    int x0 = (x < 0) ? 0 : x;
    int y0 = (y < 0) ? 0 : y;
    int x1 = (x + w > fb_width)  ? fb_width  : x + w;
    int y1 = (y + h > fb_height) ? fb_height : y + h;

    for (int py = y0; py < y1; ++py) {
        uint32_t* row = fb + py * fb_width;
        for (int px = x0; px < x1; ++px) {
            uint32_t bg = row[px];
            uint8_t bg_r = (bg >> 16) & 0xFF;
            uint8_t bg_g = (bg >>  8) & 0xFF;
            uint8_t bg_b = (bg      ) & 0xFF;

            uint8_t r = (ov_r * alpha + bg_r * inv_alpha) / 255;
            uint8_t g = (ov_g * alpha + bg_g * inv_alpha) / 255;
            uint8_t b = (ov_b * alpha + bg_b * inv_alpha) / 255;

            row[px] = 0xFF000000u | (r << 16) | (g << 8) | b;
        }
    }
}

void VirtualKeyboard::draw_char(uint32_t* fb, int fb_width, int fb_height,
                                int x, int y, char ch,
                                uint32_t color, uint8_t alpha) const
{
    if (ch < 32) return;

    const uint8_t* glyph = FONT_DATA[ch - 32];
    uint8_t ov_r = (color >> 16) & 0xFF;
    uint8_t ov_g = (color >>  8) & 0xFF;
    uint8_t ov_b = (color      ) & 0xFF;
    uint8_t inv_alpha = 255 - alpha;

    for (int row = 0; row < FONT_CHAR_HEIGHT; ++row) {
        int py = y + row;
        if (py < 0 || py >= fb_height) continue;

        uint8_t bits = glyph[row];
        for (int col = 0; col < FONT_CHAR_WIDTH; ++col) {
            if (!(bits & (0x80 >> col))) continue;

            int px = x + col;
            if (px < 0 || px >= fb_width) continue;

            uint32_t bg = fb[py * fb_width + px];
            uint8_t bg_r = (bg >> 16) & 0xFF;
            uint8_t bg_g = (bg >>  8) & 0xFF;
            uint8_t bg_b = (bg      ) & 0xFF;

            uint8_t r = (ov_r * alpha + bg_r * inv_alpha) / 255;
            uint8_t g = (ov_g * alpha + bg_g * inv_alpha) / 255;
            uint8_t b = (ov_b * alpha + bg_b * inv_alpha) / 255;

            fb[py * fb_width + px] = 0xFF000000u | (r << 16) | (g << 8) | b;
        }
    }
}

void VirtualKeyboard::draw_label(uint32_t* fb, int fb_width, int fb_height,
                                 int x, int y, const char* text,
                                 uint32_t color, uint8_t alpha) const
{
    if (!text) return;
    int cx = x;
    for (const char* p = text; *p; ++p) {
        draw_char(fb, fb_width, fb_height, cx, y, *p, color, alpha);
        cx += FONT_CHAR_WIDTH + 1;
    }
}

// -------------------------------------------------------------------------
// Public render method (Task 3.2)
// -------------------------------------------------------------------------

void VirtualKeyboard::render(uint32_t* framebuffer, int fb_width, int fb_height,
                             uint8_t transparency_pct) const
{
    uint8_t alpha = 255 - (transparency_pct * 255 / 100);
    int y_offset = get_vkb_y_offset(fb_height);

    // Color scheme
    constexpr uint32_t COLOR_BG_PANEL   = 0xFF1A1A2E;
    constexpr uint32_t COLOR_KEY_FACE   = 0xFF2D2D44;
    constexpr uint32_t COLOR_KEY_BORDER = 0xFF4A4A6A;
    constexpr uint32_t COLOR_CURSOR     = 0xFFFFCC00;
    constexpr uint32_t COLOR_TEXT       = 0xFFFFFFFF;
    constexpr uint32_t COLOR_TEXT_CUR   = 0xFF000000;

    // Draw background panel
    draw_rect(framebuffer, fb_width, fb_height,
              0, y_offset, VKB_WIDTH, VKB_HEIGHT, COLOR_BG_PANEL, alpha);

    // Draw each key
    for (int i = 0; i < KEY_COUNT; ++i) {
        const VKBKey& key = LAYOUT[i];
        bool is_cursor = (i == cursor_index_);

        int kx = key.x;
        int ky = key.y + y_offset;
        int kw = key.width;
        int kh = key.height;

        // Key face
        uint32_t face_color = is_cursor ? COLOR_CURSOR : COLOR_KEY_FACE;
        draw_rect(framebuffer, fb_width, fb_height,
                  kx, ky, kw, kh, face_color, alpha);

        // 1px border (top, bottom, left, right edges)
        draw_rect(framebuffer, fb_width, fb_height,
                  kx, ky, kw, 1, COLOR_KEY_BORDER, alpha);           // top
        draw_rect(framebuffer, fb_width, fb_height,
                  kx, ky + kh - 1, kw, 1, COLOR_KEY_BORDER, alpha);  // bottom
        draw_rect(framebuffer, fb_width, fb_height,
                  kx, ky, 1, kh, COLOR_KEY_BORDER, alpha);           // left
        draw_rect(framebuffer, fb_width, fb_height,
                  kx + kw - 1, ky, 1, kh, COLOR_KEY_BORDER, alpha);  // right

        // Center label text within key
        int label_len = 0;
        for (const char* p = key.label; *p; ++p) ++label_len;

        int text_w = label_len * (FONT_CHAR_WIDTH + 1) - 1;
        int text_x = kx + (kw - text_w) / 2;
        int text_y = ky + (kh - FONT_CHAR_HEIGHT) / 2;

        uint32_t text_color = is_cursor ? COLOR_TEXT_CUR : COLOR_TEXT;
        draw_label(framebuffer, fb_width, fb_height,
                   text_x, text_y, key.label, text_color, alpha);
    }
}

} // namespace videopac
