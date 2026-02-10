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

void InputHandler::set_key_state(VidKey key, bool pressed) {
    uint8 row = get_row(key);
    uint8 col = get_col(key);
    set_key_state(row, col, pressed);
}

uint8 InputHandler::read_keyboard(uint8 selected_row) {
    // The BIOS writes a row select value to Port 2 (0xF0, 0xF1, 0xF2, etc.)
    // where bits 0-2 encode the row number, and upper bits are 0xF0
    // When reading back:
    // - Bit 4 = 0 if key pressed, 1 if no key (BIOS checks with JB4)
    // - Bits 1-3 = column number when key pressed
    // - Bits 5-7 = preserved as 0xE0 or 0xF0 depending on key state
    // - Bit 0 = varies based on column
    
    if (selected_row < 8) {
        for (int col = 0; col < 8; ++col) {
            if (state_.keyboard_matrix[selected_row][col]) {
                // Key is pressed
                // Upper nibble should reflect the row, bit 4=0 (pressed), bits 1-3=column
                uint8 result = 0xE0 | (selected_row) | (col << 1);
                return result;
            }
        }
    }
    
    // No key pressed - set bit 4 to indicate no key
    // Upper nibble 0xF, bit 4 set, row in bits 0-2
    uint8 result = 0xF0 | 0x10 | selected_row;
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

void InputHandler::map_host_key(int host_key, VidKey vid_key) {
    key_mapping_[host_key] = vid_key;
}

void InputHandler::process_host_key(int host_key, bool pressed) {
    auto it = key_mapping_.find(host_key);
    if (it != key_mapping_.end()) {
        set_key_state(it->second, pressed);
    }
}

InputState InputHandler::get_state() const {
    return state_;
}

void InputHandler::set_state(const InputState& state) {
    state_ = state;
}

} // namespace videopac
