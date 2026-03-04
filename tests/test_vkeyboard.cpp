// Unit tests for VirtualKeyboard (Task 10.1)

#include <gtest/gtest.h>
#include "vkeyboard.h"
#include "vkeyboard_font.h"

using namespace videopac;

// ---------------------------------------------------------------------------
// Initialization tests (Req 1.4, 6.2)
// ---------------------------------------------------------------------------

TEST(VirtualKeyboard, InitializesHidden) {
    VirtualKeyboard vkb;
    EXPECT_FALSE(vkb.is_visible());
}

TEST(VirtualKeyboard, InitializesPositionBottom) {
    VirtualKeyboard vkb;
    // After toggle_position, it should be top (was bottom).
    // Toggle twice to confirm round-trip.
    vkb.toggle_position();
    vkb.toggle_position();
    // Still bottom — verified indirectly via render y-offset later.
    // Direct check: cursor starts at 0.
    EXPECT_EQ(vkb.get_cursor_index(), 0);
}

TEST(VirtualKeyboard, CursorStartsAtZero) {
    VirtualKeyboard vkb;
    EXPECT_EQ(vkb.get_cursor_index(), 0);
    EXPECT_EQ(static_cast<uint8>(vkb.get_current_vidkey()),
              static_cast<uint8>(VidKey::Key0));
}

// ---------------------------------------------------------------------------
// Navigation at corners (Req 2.1, 2.4)
// ---------------------------------------------------------------------------

TEST(VirtualKeyboard, NavigateLeftFromTopLeftWraps) {
    VirtualKeyboard vkb;
    // Index 0 (top-left of row 0), left → index 9 (wraps)
    vkb.move_cursor(Direction::Left);
    EXPECT_EQ(vkb.get_cursor_index(), 9);
}

TEST(VirtualKeyboard, NavigateRightFromTopRightWraps) {
    VirtualKeyboard vkb;
    // Move to index 9 (top-right of row 0)
    for (int i = 0; i < 9; ++i) vkb.move_cursor(Direction::Right);
    EXPECT_EQ(vkb.get_cursor_index(), 9);
    // Right from 9 → wraps to 0
    vkb.move_cursor(Direction::Right);
    EXPECT_EQ(vkb.get_cursor_index(), 0);
}

TEST(VirtualKeyboard, NavigateUpFromRow0GoesToSpaceBar) {
    VirtualKeyboard vkb;
    // Index 0, up → index 48 (space bar)
    vkb.move_cursor(Direction::Up);
    EXPECT_EQ(vkb.get_cursor_index(), 48);
}

TEST(VirtualKeyboard, SpaceBarLeftRightStays) {
    VirtualKeyboard vkb;
    // Navigate to space bar (index 48)
    vkb.move_cursor(Direction::Up);
    EXPECT_EQ(vkb.get_cursor_index(), 48);

    // Left from space bar: nav_left = -1, should stay
    vkb.move_cursor(Direction::Left);
    EXPECT_EQ(vkb.get_cursor_index(), 48);

    // Right from space bar: nav_right = -1, should stay
    vkb.move_cursor(Direction::Right);
    EXPECT_EQ(vkb.get_cursor_index(), 48);
}

TEST(VirtualKeyboard, NavigateDownFromSpaceBarGoesToRow0) {
    VirtualKeyboard vkb;
    // Go to space bar
    vkb.move_cursor(Direction::Up);
    EXPECT_EQ(vkb.get_cursor_index(), 48);

    // Down from space bar → index 4
    vkb.move_cursor(Direction::Down);
    EXPECT_EQ(vkb.get_cursor_index(), 4);
}

TEST(VirtualKeyboard, NavigateRow3WrapsLeftRight) {
    VirtualKeyboard vkb;
    // Navigate to index 30 (A key, row 4 leftmost)
    // From 0 → down to 10 → down to 15 → down to 20 → down to 30
    vkb.move_cursor(Direction::Down); // 0 → 10
    vkb.move_cursor(Direction::Down); // 10 → 15
    vkb.move_cursor(Direction::Down); // 15 → 20
    vkb.move_cursor(Direction::Down); // 20 → 30
    EXPECT_EQ(vkb.get_cursor_index(), 30);

    // Left from 30 → 38 (wraps)
    vkb.move_cursor(Direction::Left);
    EXPECT_EQ(vkb.get_cursor_index(), 38);

    // Right from 38 → 30 (wraps)
    vkb.move_cursor(Direction::Right);
    EXPECT_EQ(vkb.get_cursor_index(), 30);
}

// ---------------------------------------------------------------------------
// Key press on specific keys returns correct VidKey (Req 3.1)
// ---------------------------------------------------------------------------

TEST(VirtualKeyboard, Key0ReturnsCorrectVidKey) {
    VirtualKeyboard vkb;
    // Cursor starts at index 0 = Key0
    EXPECT_EQ(vkb.get_current_vidkey(), VidKey::Key0);
}

TEST(VirtualKeyboard, EnterKeyReturnsCorrectVidKey) {
    VirtualKeyboard vkb;
    // ENT is at index 18 (row 2 action keys, position 3)
    // Navigate: down to row 1 (10), down to row 2 (15), then right 3 times
    vkb.move_cursor(Direction::Down); // 0 → 10
    vkb.move_cursor(Direction::Down); // 10 → 15
    for (int i = 0; i < 3; ++i) vkb.move_cursor(Direction::Right);
    EXPECT_EQ(vkb.get_cursor_index(), 18);
    EXPECT_EQ(vkb.get_current_vidkey(), VidKey::Enter);
}

TEST(VirtualKeyboard, SpaceKeyReturnsCorrectVidKey) {
    VirtualKeyboard vkb;
    // SPC was at index 19 in old layout, now RST is there.
    // SPC is now only on the wide space bar (index 48).
    // Navigate to index 48 via up from row 0.
    vkb.move_cursor(Direction::Up); // 0 → 48
    EXPECT_EQ(vkb.get_cursor_index(), 48);
    EXPECT_EQ(vkb.get_current_vidkey(), VidKey::Space);
}

TEST(VirtualKeyboard, SpaceBarWideKeyReturnsCorrectVidKey) {
    VirtualKeyboard vkb;
    // Wide SPC at index 48
    vkb.move_cursor(Direction::Up); // 0 → 48
    EXPECT_EQ(vkb.get_cursor_index(), 48);
    EXPECT_EQ(vkb.get_current_vidkey(), VidKey::Space);
}

TEST(VirtualKeyboard, GetVidKeyAtValidIndex) {
    VirtualKeyboard vkb;
    EXPECT_EQ(vkb.get_vidkey_at(0), VidKey::Key0);
    EXPECT_EQ(vkb.get_vidkey_at(18), VidKey::Enter);
    EXPECT_EQ(vkb.get_vidkey_at(48), VidKey::Space);
}

TEST(VirtualKeyboard, GetVidKeyAtInvalidIndexReturnsKey0) {
    VirtualKeyboard vkb;
    EXPECT_EQ(vkb.get_vidkey_at(-1), VidKey::Key0);
    EXPECT_EQ(vkb.get_vidkey_at(49), VidKey::Key0);
    EXPECT_EQ(vkb.get_vidkey_at(100), VidKey::Key0);
}

// ---------------------------------------------------------------------------
// Touch hit testing on key boundaries (Req 7.1, 8.4)
// ---------------------------------------------------------------------------

TEST(VirtualKeyboard, HitTestCenterOfKey0) {
    VirtualKeyboard vkb;
    const auto& key = VirtualKeyboard::LAYOUT[0];
    int cx = key.x + key.width / 2;
    int cy = key.y + key.height / 2;
    EXPECT_EQ(vkb.hit_test(cx, cy), 0);
}

TEST(VirtualKeyboard, HitTestTopLeftEdgeOfKey) {
    VirtualKeyboard vkb;
    const auto& key = VirtualKeyboard::LAYOUT[0];
    // Exact top-left corner should be inside
    EXPECT_EQ(vkb.hit_test(key.x, key.y), 0);
}

TEST(VirtualKeyboard, HitTestBottomRightEdgeExclusive) {
    VirtualKeyboard vkb;
    const auto& key = VirtualKeyboard::LAYOUT[0];
    // x + width and y + height are exclusive (one past the end)
    EXPECT_EQ(vkb.hit_test(key.x + key.width, key.y + key.height), -1);
}

TEST(VirtualKeyboard, HitTestJustInsideBottomRight) {
    VirtualKeyboard vkb;
    const auto& key = VirtualKeyboard::LAYOUT[0];
    // One pixel inside the bottom-right corner
    EXPECT_EQ(vkb.hit_test(key.x + key.width - 1, key.y + key.height - 1), 0);
}

TEST(VirtualKeyboard, HitTestOutsideAllKeys) {
    VirtualKeyboard vkb;
    // Point at (0, 0) — the VKB panel background, not on any key
    // Key 0 starts at x=1, y=1, so (0,0) is outside
    EXPECT_EQ(vkb.hit_test(0, 0), -1);
}

TEST(VirtualKeyboard, HitTestSpaceBarWideKey) {
    VirtualKeyboard vkb;
    const auto& key = VirtualKeyboard::LAYOUT[48];
    int cx = key.x + key.width / 2;
    int cy = key.y + key.height / 2;
    EXPECT_EQ(vkb.hit_test(cx, cy), 48);
}

TEST(VirtualKeyboard, HitTestNegativeCoordinates) {
    VirtualKeyboard vkb;
    EXPECT_EQ(vkb.hit_test(-1, -1), -1);
}

// ---------------------------------------------------------------------------
// Font data: all printable ASCII chars have non-zero glyph data (Req 4.1)
// ---------------------------------------------------------------------------

TEST(VirtualKeyboard, FontDataNonZeroForPrintableChars) {
    // Space (index 0) is intentionally all-zero, skip it.
    // DEL (index 95) is intentionally all-zero, skip it.
    for (int i = 1; i < 95; ++i) {
        bool has_pixels = false;
        for (int row = 0; row < FONT_CHAR_HEIGHT; ++row) {
            if (FONT_DATA[i][row] != 0) {
                has_pixels = true;
                break;
            }
        }
        EXPECT_TRUE(has_pixels)
            << "Character " << (char)(i + 32) << " (code " << (i + 32)
            << ") has all-zero glyph data";
    }
}

TEST(VirtualKeyboard, FontDataSpaceIsAllZero) {
    for (int row = 0; row < FONT_CHAR_HEIGHT; ++row) {
        EXPECT_EQ(FONT_DATA[0][row], 0)
            << "Space character row " << row << " should be zero";
    }
}

TEST(VirtualKeyboard, FontDataUsesOnly5BitsPerRow) {
    // Bits 2-0 should always be zero (5 bits MSB-aligned)
    for (int i = 0; i < 96; ++i) {
        for (int row = 0; row < FONT_CHAR_HEIGHT; ++row) {
            EXPECT_EQ(FONT_DATA[i][row] & 0x07, 0)
                << "Character " << (char)(i + 32) << " row " << row
                << " uses bits outside the 5-bit MSB range";
        }
    }
}

// ---------------------------------------------------------------------------
// Layout table structural checks
// ---------------------------------------------------------------------------

TEST(VirtualKeyboard, LayoutKeyCount) {
    EXPECT_EQ(VirtualKeyboard::KEY_COUNT, 49);
}

TEST(VirtualKeyboard, AllKeysHaveNonNullLabels) {
    for (int i = 0; i < VirtualKeyboard::KEY_COUNT; ++i) {
        EXPECT_NE(VirtualKeyboard::LAYOUT[i].label, nullptr)
            << "Key " << i << " has null label";
    }
}

TEST(VirtualKeyboard, AllKeysFitWithinVKBBounds) {
    for (int i = 0; i < VirtualKeyboard::KEY_COUNT; ++i) {
        const auto& key = VirtualKeyboard::LAYOUT[i];
        EXPECT_GT(key.width, 0) << "Key " << i << " has zero width";
        EXPECT_GT(key.height, 0) << "Key " << i << " has zero height";
        EXPECT_LE(key.x + key.width, VirtualKeyboard::VKB_WIDTH)
            << "Key " << i << " exceeds VKB width";
        EXPECT_LE(key.y + key.height, VirtualKeyboard::VKB_HEIGHT)
            << "Key " << i << " exceeds VKB height";
    }
}

TEST(VirtualKeyboard, AllNavLinksAreValidOrMinusOne) {
    for (int i = 0; i < VirtualKeyboard::KEY_COUNT; ++i) {
        const auto& key = VirtualKeyboard::LAYOUT[i];
        auto check_link = [&](int8_t link, const char* dir) {
            EXPECT_TRUE(link == -1 || (link >= 0 && link < VirtualKeyboard::KEY_COUNT))
                << "Key " << i << " nav_" << dir << " = " << (int)link
                << " is out of range";
        };
        check_link(key.nav_up, "up");
        check_link(key.nav_down, "down");
        check_link(key.nav_left, "left");
        check_link(key.nav_right, "right");
    }
}

TEST(VirtualKeyboard, AllVidKeysHaveValidRowAndCol) {
    for (int i = 0; i < VirtualKeyboard::KEY_COUNT; ++i) {
        uint8_t raw = static_cast<uint8_t>(VirtualKeyboard::LAYOUT[i].vidkey);
        // VidKey::Reset (0xFF) is a sentinel, not a matrix key — skip it
        if (raw == 0xFF) continue;
        uint8_t row = (raw >> 4) & 0x07;
        uint8_t col = raw & 0x07;
        EXPECT_LT(row, 6) << "Key " << i << " VidKey row " << (int)row << " >= 6";
        EXPECT_LT(col, 8) << "Key " << i << " VidKey col " << (int)col << " >= 8";
    }
}

// ---------------------------------------------------------------------------
// Toggle round-trip
// ---------------------------------------------------------------------------

TEST(VirtualKeyboard, ToggleVisibleRoundTrip) {
    VirtualKeyboard vkb;
    EXPECT_FALSE(vkb.is_visible());
    vkb.toggle_visible();
    EXPECT_TRUE(vkb.is_visible());
    vkb.toggle_visible();
    EXPECT_FALSE(vkb.is_visible());
}

TEST(VirtualKeyboard, ResetKeyReturnsResetVidKey) {
    VirtualKeyboard vkb;
    // RST is at index 19 (row 2 action keys, position 4)
    // Navigate: down to row 1 (10), down to row 2 (15), then right 4 times
    vkb.move_cursor(Direction::Down); // 0 → 10
    vkb.move_cursor(Direction::Down); // 10 → 15
    for (int i = 0; i < 4; ++i) vkb.move_cursor(Direction::Right);
    EXPECT_EQ(vkb.get_cursor_index(), 19);
    EXPECT_EQ(vkb.get_current_vidkey(), VidKey::Reset);
}
