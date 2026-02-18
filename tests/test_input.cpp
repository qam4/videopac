#include <gtest/gtest.h>
#include "input.h"

using namespace videopac;

TEST(InputTest, KeyboardMatrixReadWrite) {
    InputHandler input;
    input.set_key_state(0, 0, true);
    
    uint8 result = input.read_keyboard(0xFE);  // Select row 0
    EXPECT_NE(result, 0xFF);  // Should have at least one key pressed
}

TEST(InputTest, KeyboardMultipleRows) {
    InputHandler input;
    
    // Press keys in different rows
    input.set_key_state(0, 0, true);  // Row 0, Col 0
    input.set_key_state(1, 1, true);  // Row 1, Col 1
    input.set_key_state(2, 2, true);  // Row 2, Col 2
    
    // Read row 0 (active low, so 0xFE selects row 0)
    uint8 result0 = input.read_keyboard(0xFE);
    EXPECT_EQ(result0 & 0x01, 0x00);  // Bit 0 should be clear (key pressed)
    EXPECT_EQ(result0 & 0x02, 0x02);  // Bit 1 should be set (no key)
    
    // Read row 1
    uint8 result1 = input.read_keyboard(0xFD);
    EXPECT_EQ(result1 & 0x01, 0x01);  // Bit 0 should be set (no key)
    EXPECT_EQ(result1 & 0x02, 0x00);  // Bit 1 should be clear (key pressed)
    
    // Read row 2
    uint8 result2 = input.read_keyboard(0xFB);
    EXPECT_EQ(result2 & 0x04, 0x00);  // Bit 2 should be clear (key pressed)
}

TEST(InputTest, KeyboardVidKeyEnum) {
    InputHandler input;
    
    // Test using VidKey enum
    input.set_key_state(VidKey::Key0, true);
    input.set_key_state(VidKey::KeyA, true);
    input.set_key_state(VidKey::Space, true);
    
    // Read row 0 (Key0)
    uint8 result0 = input.read_keyboard(0);
    EXPECT_NE(result0 & 0x10, 0x10);  // Bit 4 clear = key pressed
    
    // Read row 3 (KeyA)
    uint8 result3 = input.read_keyboard(3);
    EXPECT_NE(result3 & 0x10, 0x10);  // Bit 4 clear = key pressed
    
    // Read row 5 (Space)
    uint8 result5 = input.read_keyboard(5);
    EXPECT_NE(result5 & 0x10, 0x10);  // Bit 4 clear = key pressed
}

TEST(InputTest, Joystick1Reading) {
    InputHandler input;
    
    // Set joystick 1 directions
    input.set_joystick_state(0, Direction::Up, true);
    input.set_joystick_button(0, true);
    
    // Read joystick 1 (P20-P22 = 0b001 = 1)
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
    
    uint8 result = input.read_joystick(0x01);  // Joystick 1 = 0b001
    
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
    uint8 result = input.read_keyboard(0);
    EXPECT_NE(result & 0x10, 0x10);  // Bit 4 clear = key pressed
    
    // Process host key release
    input.process_host_key(65, false);
    
    // Verify key is released
    result = input.read_keyboard(0);
    EXPECT_EQ(result & 0x10, 0x10);  // Bit 4 set = no key pressed
}

TEST(InputTest, StateSaveRestore) {
    InputHandler input1;
    
    // Set some state
    input1.set_key_state(VidKey::Key5, true);
    input1.set_joystick_state(0, Direction::Up, true);
    input1.set_joystick_button(1, true);
    
    // Save state
    InputState state = input1.get_state();
    
    // Create new handler and restore state
    InputHandler input2;
    input2.set_state(state);
    
    // Verify state matches - Key5 is in row 0
    uint8 kb_result = input2.read_keyboard(0);
    EXPECT_NE(kb_result & 0x10, 0x10);  // Bit 4 clear = key pressed
    
    uint8 joy1_result = input2.read_joystick(0x01);  // Joystick 1 = 0b001
    EXPECT_EQ(joy1_result & 0x01, 0x00);  // Up pressed
    
    uint8 joy2_result = input2.read_joystick(0x00);  // Joystick 2 = 0b000
    EXPECT_EQ(joy2_result & 0x10, 0x00);  // Fire pressed
}

TEST(InputTest, ResetClearsState) {
    InputHandler input;
    
    // Set some state
    input.set_key_state(VidKey::KeyA, true);
    input.set_joystick_state(0, Direction::Up, true);
    
    // Reset
    input.reset();
    
    // Verify all cleared - check row 3 where KeyA is
    uint8 kb_result = input.read_keyboard(3);
    EXPECT_EQ(kb_result & 0x10, 0x10);  // Bit 4 set = no key pressed
    
    uint8 joy_result = input.read_joystick(0x07);
    EXPECT_EQ(joy_result, 0xFF);  // No joystick input
}
