// Standalone VKB preview — renders the virtual keyboard to a BMP file.
// Build: g++ -std=c++17 -I../include -o preview_vkb preview_vkb.cpp ../src/vkeyboard.cpp
// Usage: ./preview_vkb [output.bmp] [cursor_index]

#include "vkeyboard.h"
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

// Write a 24-bit BMP from an XRGB8888 buffer
static bool write_bmp(const char* path, const uint32_t* fb, int w, int h) {
    FILE* f = fopen(path, "wb");
    if (!f) return false;

    int row_bytes = w * 3;
    int pad = (4 - (row_bytes % 4)) % 4;
    int data_size = (row_bytes + pad) * h;
    int file_size = 54 + data_size;

    uint8_t hdr[54] = {};
    hdr[0] = 'B'; hdr[1] = 'M';
    hdr[2] = file_size & 0xFF; hdr[3] = (file_size >> 8) & 0xFF;
    hdr[4] = (file_size >> 16) & 0xFF; hdr[5] = (file_size >> 24) & 0xFF;
    hdr[10] = 54;
    hdr[14] = 40;
    hdr[18] = w & 0xFF; hdr[19] = (w >> 8) & 0xFF;
    hdr[22] = h & 0xFF; hdr[23] = (h >> 8) & 0xFF;
    hdr[26] = 1;
    hdr[28] = 24;

    fwrite(hdr, 1, 54, f);

    std::vector<uint8_t> row_buf(row_bytes + pad, 0);
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            uint32_t px = fb[y * w + x];
            row_buf[x * 3 + 0] = (px      ) & 0xFF;
            row_buf[x * 3 + 1] = (px >>  8) & 0xFF;
            row_buf[x * 3 + 2] = (px >> 16) & 0xFF;
        }
        fwrite(row_buf.data(), 1, row_bytes + pad, f);
    }

    fclose(f);
    return true;
}

int main(int argc, char* argv[]) {
    const char* output = "vkb_preview.bmp";
    int cursor = 0;

    if (argc > 1) output = argv[1];
    if (argc > 2) cursor = atoi(argv[2]);

    // Tight framebuffer: just the VKB height + a few pixels of "game" above
    const int W = videopac::VirtualKeyboard::VKB_WIDTH;
    const int GAME_ROWS = 10; // small strip of "game screen" above VKB
    const int H = videopac::VirtualKeyboard::VKB_HEIGHT + GAME_ROWS;

    // Fill with dark blue simulating game screen
    std::vector<uint32_t> fb(W * H, 0xFF102040);

    videopac::VirtualKeyboard vkb;
    vkb.toggle_visible();

    // Clamp cursor to valid range
    if (cursor < 0) cursor = 0;
    if (cursor >= videopac::VirtualKeyboard::KEY_COUNT)
        cursor = videopac::VirtualKeyboard::KEY_COUNT - 1;

    // Navigate to requested cursor index by walking the layout
    int target = cursor;
    // Row starts: 0=0, 1=10, 2=15, 3=20, 4=30, 5=39, 6=48
    int row_starts[] = {0, 10, 15, 20, 30, 39, 48};
    int num_rows = 7;
    int target_row = 0;
    for (int r = num_rows - 1; r >= 0; --r) {
        if (target >= row_starts[r]) { target_row = r; break; }
    }
    int col_in_row = target - row_starts[target_row];

    // Move down to target row
    for (int r = 0; r < target_row; ++r)
        vkb.move_cursor(videopac::Direction::Down);
    // Move right to target column
    for (int c = 0; c < col_in_row; ++c)
        vkb.move_cursor(videopac::Direction::Right);

    // Render with 0% transparency for clear preview
    vkb.render(fb.data(), W, H, 0);

    if (write_bmp(output, fb.data(), W, H)) {
        printf("Wrote %s (%dx%d, cursor at key %d: \"%s\")\n",
               output, W, H, vkb.get_cursor_index(),
               videopac::VirtualKeyboard::LAYOUT[vkb.get_cursor_index()].label);
    } else {
        fprintf(stderr, "Failed to write %s\n", output);
        return 1;
    }

    return 0;
}
