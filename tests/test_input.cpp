#include <gtest/gtest.h>
#include "input.h"

using namespace videopac;

// read_keyboard() API:
//   Input: port2 value where bits 0-2 = row number (0-5)
//   Output when key pressed: lower_nibble | (col^7 << 5), bit 4 = 0
//   Output when no key pressed: lower_nibble | 0xF0

TEST(InputTest, KeyboardMatrixReadWrite) {
    InputHandler input;
    input.set_key_state(0, 0, true);  // Row 0, Col 0
    
    uint8 result = input.read_keyboard(0x00);  // Select row 0
    // Key at col 0: inverted_col = 0^7 = 7, result = 0x00 | (7 << 5) = 0xE0
    EXPECT_EQ(result, 0xE0);
    
    // No key on row 1
    uint8 result1 = input.read_keyboard(0x01);
    EXPECT_EQ(result1, 0xF1);  // 0x01 | 0xF0
}

TEST(InputTest, KeyboardMultipleRows) {
    InputHandler input;
    
    // Press keys in different rows
    input.set_key_state(0, 0, true);  // Row 0, Col 0
    input.set_key_state(1, 1, true);  // Row 1, Col 1
    input.set_key_state(2, 2, true);  // Row 2, Col 2
    
    // Read row 0: col 0 pressed, inverted_col = 7, bits 5-7 = 7 = 0xE0
    uint8 result0 = input.read_keyboard(0x00);
    EXPECT_EQ(result0 & 0xF0, 0xE0);  // col 0 → inverted 7 → 0xE0
    EXPECT_EQ(result0 & 0x0F, 0x00);  // lower nibble preserved
    
    // Read row 1: col 1 pressed, inverted_col = 6, bits 5-7 = 6 = 0xC0
    uint8 result1 = input.read_keyboard(0x01);
    EXPECT_EQ(result1 & 0xF0, 0xC0);
    EXPECT_EQ(result1 & 0x0F, 0x01);
    
    // Read row 2: col 2 pressed, inverted_col = 5, bits 5-7 = 5 = 0xA0
    uint8 result2 = input.read_keyboard(0x02);
    EXPECT_EQ(result2 & 0xF0, 0xA0);
    EXPECT_EQ(result2 & 0x0F, 0x02);
}

TEST(InputTest, KeyboardVidKeyEnum) {
    InputHandler input;
    
    // Test using VidKey enum
    input.set_key_state(VidKey::Key0, true);   // Row 0, Col 0
    input.set_key_state(VidKey::KeyA, true);   // Row 4, Col 0
    input.set_key_state(VidKey::Space, true);  // Row 1, Col 4
    
    // Read row 0 (Key0 at col 0)
    uint8 result0 = input.read_keyboard(0x00);
    EXPECT_NE(result0 & 0x10, 0x10);  // Bit 4 clear = key pressed
    
    // Read row 4 (KeyA at col 0)
    uint8 result4 = input.read_keyboard(0x04);
    EXPECT_NE(result4 & 0x10, 0x10);  // Bit 4 clear = key pressed
    
    // Read row 1 (Space at col 4)
    uint8 result1 = input.read_keyboard(0x01);
    EXPECT_NE(result1 & 0x10, 0x10);  // Bit 4 clear = key pressed
    // Space is col 4, inverted = 3, bits 5-7 = 3 = 0x60
    EXPECT_EQ(result1 & 0xE0, 0x60);
}

TEST(InputTest, KeyboardNoKeyPressed) {
    InputHandler input;
    
    // No keys pressed, read any row
    uint8 result = input.read_keyboard(0x03);
    EXPECT_EQ(result, 0xF3);  // 0x03 | 0xF0
}

TEST(InputTest, KeyboardPreservesLowerNibble) {
    InputHandler input;
    input.set_key_state(0, 0, true);  // Row 0, Col 0
    
    // Port2 value with bits 3-0 = 0x08 (bit 3 set, row 0)
    uint8 result = input.read_keyboard(0x08);
    // Row = 0x08 & 0x07 = 0, col 0 found, inverted_col = 7
    // Result = (0x08 & 0x0F) | (7 << 5) = 0x08 | 0xE0 = 0xE8
    EXPECT_EQ(result, 0xE8);
}

TEST(InputTest, KeyboardInvalidRowReturnsNoKey) {
    InputHandler input;
    input.set_key_state(0, 0, true);  // Row 0, Col 0
    
    // Row 6 and 7 are invalid (only 0-5 valid)
    uint8 result6 = input.read_keyboard(0x06);
    EXPECT_EQ(result6, 0xF6);  // No key = 0x06 | 0xF0
    
    uint8 result7 = input.read_keyboard(0x07);
    EXPECT_EQ(result7, 0xF7);
}

TEST(InputTest, Joystick1Reading) {
    InputHandler input;
    
    // Set joystick 1 directions
    input.set_joystick_state(0, Direction::Up, true);
    input.set_joystick_button(0, true);
    
    // Read joystick 1 (P20=1 → value 1, per o2doc and o2em)
    uint8 result = input.read_joystick(0x01);
    
    // Check bits (active low)
    EXPECT_EQ(result & 0x01, 0x00);  // Up pressed
    EXPECT_EQ(result & 0x10, 0x00);  // Fire pressed
    EXPECT_EQ(result & 0x02, 0x02);  // Right not pressed
    EXPECT_EQ(result & 0x04, 0x04);  // Down not pressed
    EXPECT_EQ(result & 0x08, 0x08);  // Left not pressed
}

TEST(InputTest, Joystick2Reading) {
    InputHandler input;
    
    // Set joystick 2 directions
    input.set_joystick_state(1, Direction::Down, true);
    input.set_joystick_state(1, Direction::Right, true);
    
    // Read joystick 2 (P20-P22 = 0b000)
    uint8 result = input.read_joystick(0x00);
    
    // Check bits (active low)
    EXPECT_EQ(result & 0x02, 0x00);  // Right pressed
    EXPECT_EQ(result & 0x04, 0x00);  // Down pressed
    EXPECT_EQ(result & 0x01, 0x01);  // Up not pressed
    EXPECT_EQ(result & 0x08, 0x08);  // Left not pressed
    EXPECT_EQ(result & 0x10, 0x10);  // Fire not pressed
}

TEST(InputTest, JoystickAllDirections) {
    InputHandler input;
    
    // Test all directions for joystick 1
    input.set_joystick_state(0, Direction::Up, true);
    input.set_joystick_state(0, Direction::Down, true);
    input.set_joystick_state(0, Direction::Left, true);
    input.set_joystick_state(0, Direction::Right, true);
    input.set_joystick_button(0, true);
    
    uint8 result = input.read_joystick(0x01);  // Joystick 1: P20=1 → value 1
    
    // All bits should be clear (all pressed)
    EXPECT_EQ(result & 0x1F, 0x00);
}

TEST(InputTest, HostKeyMapping) {
    InputHandler input;
    
    // Map host key 65 (ASCII 'A') to Videopac Key0
    input.map_host_key(65, VidKey::Key0);
    
    // Process host key press
    input.process_host_key(65, true);
    
    // Verify key is pressed in matrix (row 0)
    uint8 result = input.read_keyboard(0x00);
    EXPECT_NE(result & 0x10, 0x10);  // Bit 4 clear = key pressed
    
    // Process host key release
    input.process_host_key(65, false);
    
    // Verify key is released
    result = input.read_keyboard(0x00);
    EXPECT_EQ(result, 0xF0);  // No key = 0x00 | 0xF0
}

TEST(InputTest, StateSaveRestore) {
    InputHandler input1;
    
    // Set some state
    input1.set_key_state(VidKey::Key5, true);  // Row 0, Col 5
    input1.set_joystick_state(0, Direction::Up, true);
    input1.set_joystick_button(1, true);
    
    // Save state
    InputState state = input1.get_state();
    
    // Create new handler and restore state
    InputHandler input2;
    input2.set_state(state);
    
    // Verify state matches - Key5 is in row 0
    uint8 kb_result = input2.read_keyboard(0x00);
    EXPECT_NE(kb_result & 0x10, 0x10);  // Bit 4 clear = key pressed
    // Key5 is col 5, inverted = 2, bits 5-7 = 2 = 0x40
    EXPECT_EQ(kb_result & 0xE0, 0x40);
    
    uint8 joy1_result = input2.read_joystick(0x01);  // Joystick 1: P20=1 → value 1
    EXPECT_EQ(joy1_result & 0x01, 0x00);  // Up pressed
    
    uint8 joy2_result = input2.read_joystick(0x00);  // Joystick 2 = 0b000
    EXPECT_EQ(joy2_result & 0x10, 0x00);  // Fire pressed
}

TEST(InputTest, ResetClearsState) {
    InputHandler input;
    
    // Set some state
    input.set_key_state(VidKey::KeyA, true);  // Row 4, Col 0
    input.set_joystick_state(0, Direction::Up, true);
    
    // Reset
    input.reset();
    
    // Verify all cleared - check row 4 where KeyA is
    uint8 kb_result = input.read_keyboard(0x04);
    EXPECT_EQ(kb_result, 0xF4);  // No key = 0x04 | 0xF0
    
    uint8 joy_result = input.read_joystick(0x07);
    EXPECT_EQ(joy_result, 0xFF);  // No joystick input
}
