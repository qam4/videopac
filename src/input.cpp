#include "input.h"
#include <cstring>

namespace videopac {

InputHandler::InputHandler() {
    reset();
}

void InputHandler::reset() {
    std::memset(&state_, 0, sizeof(state_));
}

void InputHandler::set_key_state(uint8 row, uint8 col, bool pressed) {
    if (row < 8 && col < 8) {
        state_.keyboard_matrix[row][col] = pressed;
    }
}

uint8 InputHandler::read_keyboard(uint8 row_select) {
    uint8 result = 0xFF;
    
    // Check each row
    for (int row = 0; row < 8; ++row) {
        if ((row_select & (1 << row)) == 0) {  // Row is selected (active low)
            // Read columns for this row
            for (int col = 0; col < 8; ++col) {
                if (state_.keyboard_matrix[row][col]) {
                    result &= ~(1 << col);  // Clear bit if key is pressed
                }
            }
        }
    }
    
    return result;
}

void InputHandler::set_joystick_state(uint8 joystick, Direction direction, bool pressed) {
    if (joystick > 1) return;
    
    bool* joy = (joystick == 0) ? state_.joystick1 : state_.joystick2;
    
    switch (direction) {
        case Direction::Up:    joy[0] = pressed; break;
        case Direction::Down:  joy[1] = pressed; break;
        case Direction::Left:  joy[2] = pressed; break;
        case Direction::Right: joy[3] = pressed; break;
    }
}

void InputHandler::set_joystick_button(uint8 joystick, bool pressed) {
    if (joystick > 1) return;
    
    bool* joy = (joystick == 0) ? state_.joystick1 : state_.joystick2;
    joy[4] = pressed;  // Fire button
}

uint8 InputHandler::read_joystick(uint8 select_bits) {
    // P20-P22 select joystick
    // 0b000 = Joystick 2
    // 0b111 = Joystick 1
    
    bool* joy = ((select_bits & 0x07) == 0x07) ? state_.joystick1 : state_.joystick2;
    
    uint8 result = 0xFF;
    
    // Encode joystick state (active low)
    if (joy[0]) result &= ~0x01;  // Up
    if (joy[3]) result &= ~0x02;  // Right
    if (joy[1]) result &= ~0x04;  // Down
    if (joy[2]) result &= ~0x08;  // Left
    if (joy[4]) result &= ~0x10;  // Fire
    
    return result;
}

void InputHandler::map_host_key(int host_key, uint8 row, uint8 col) {
    (void)host_key;
    (void)row;
    (void)col;
    // TODO: Implement host key mapping table
    // This will be used by the frontend
}

InputState InputHandler::get_state() const {
    return state_;
}

void InputHandler::set_state(const InputState& state) {
    state_ = state;
}

} // namespace videopac
