#ifndef VIDEOPAC_VKEYBOARD_H
#define VIDEOPAC_VKEYBOARD_H

#include "input.h"
#include "types.h"
#include <cstdint>

namespace videopac {

struct VKBKey {
    const char* label;      // Display text (e.g. "Q", "SPC", "ENT")
    uint8 x, y;             // Position in VKB-local coordinates
    uint8 width, height;    // Key dimensions in pixels
    VidKey vidkey;           // Corresponding keyboard matrix entry
    int8_t nav_up;          // Index of key above (-1 = stay)
    int8_t nav_down;        // Index of key below (-1 = stay)
    int8_t nav_left;        // Index of key to left (-1 = stay)
    int8_t nav_right;       // Index of key to right (-1 = stay)
};

class VirtualKeyboard {
public:
    VirtualKeyboard();

    // State
    bool is_visible() const;
    void toggle_visible();
    void toggle_position();       // Swap top/bottom

    // Navigation (D-pad)
    void move_cursor(Direction dir);
    void set_cursor(int index);    // Set cursor directly (for touch highlight)
    int  get_cursor_index() const;

    // Key press (returns VidKey of current cursor key)
    VidKey get_current_vidkey() const;

    // Touch hit testing
    // Returns index into layout table, or -1 if no hit
    int hit_test(int vkb_local_x, int vkb_local_y) const;
    VidKey get_vidkey_at(int index) const;

    // Rendering
    // Blends VKB overlay onto an XRGB8888 buffer
    void render(uint32_t* framebuffer, int fb_width, int fb_height,
                uint8_t transparency_pct) const;

    // Layout access (for testing)
    static constexpr int KEY_COUNT = 49;
    static const VKBKey LAYOUT[KEY_COUNT];

    // VKB dimensions
    static constexpr int VKB_WIDTH = 160;
    static constexpr int VKB_HEIGHT = 93;

private:
    bool visible_ = false;
    bool position_bottom_ = true;  // true = bottom, false = top
    int cursor_index_ = 0;

    // Internal rendering helpers
    void draw_rect(uint32_t* fb, int fb_width, int fb_height,
                   int x, int y, int w, int h, uint32_t color,
                   uint8_t alpha) const;
    void draw_char(uint32_t* fb, int fb_width, int fb_height,
                   int x, int y, char ch, uint32_t color,
                   uint8_t alpha) const;
    void draw_label(uint32_t* fb, int fb_width, int fb_height,
                    int x, int y, const char* text, uint32_t color,
                    uint8_t alpha) const;
    int  get_vkb_y_offset(int fb_height) const;
};

} // namespace videopac

#endif // VIDEOPAC_VKEYBOARD_H
