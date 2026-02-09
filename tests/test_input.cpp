#include <gtest/gtest.h>
#include "input.h"

using namespace videopac;

TEST(InputTest, KeyboardMatrixReadWrite) {
    InputHandler input;
    input.set_key_state(0, 0, true);
    
    uint8 result = input.read_keyboard(0xFE);  // Select row 0
    EXPECT_NE(result, 0xFF);  // Should have at least one key pressed
}

// TODO: Add more input tests
// This will be implemented in task 7
