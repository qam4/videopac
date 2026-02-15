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
    // When reading back (per o2doc.md section 1.2):
    // - P24 (bit 4) = 0 if key pressed, 1 if no key (BIOS checks with JB4)
    // - P25-P27 (bits 5-7) = column number (0-7) when key pressed
    // - P20-P22 (bits 0-2) = echo back the row select value
    // - P23 (bit 3) = unused
    // Note: The BIOS XORs the column with 0x07 (see french_bios_annotated.txt:0x0cd)
    // so we need to invert the column bits
    
    uint8 row = selected_row & 0x07;  // Extract row from bits 0-2
    
    if (row < 8) {
        for (int col = 0; col < 8; ++col) {
            if (state_.keyboard_matrix[row][col]) {
                // Key is pressed
                // Format: [col:3][0:1][0:1][row:3]
                // Bits 7-5: column (0-7) - inverted by XOR with 0x07
                // Bit 4: key pressed indicator (0 = pressed)
                // Bit 3: unused (0)
                // Bits 2-0: row echo
                uint8 inverted_col = (7 - col) & 0x07;  // Invert column
                uint8 result = (inverted_col << 5) | row;
                return result;
            }
        }
    }
    
    // No key pressed - set bit 4 to indicate no key
    // Format: [xxx][1][0][row:3]
    uint8 result = 0x10 | row;
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
