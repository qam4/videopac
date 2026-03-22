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
    // When reading back (per o2doc.md section 1.2 and o2em vmachine.c read_P2):
    // - When key pressed: bits 7-5 = column XOR 0x07, bit 4 = 0
    // - When no key pressed: bits 7-4 = 0xF (all high)
    // - Bits 0-2 = echo back the row select value
    // - Bit 3 = preserved from written value
    //
    // Reference: o2em vmachine.c read_P2():
    //   if key found: p2 = (p2 & 0x0F) | (col_xor_7 << 5)
    //   if no key:    p2 = p2 | 0xF0
    
    uint8 row = selected_row & 0x07;  // Extract row from bits 0-2
    uint8 lower_nibble = selected_row & 0x0F;  // Preserve bits 0-3 as written
    
    if (row < 6) {  // Only 6 rows are valid (o2em: si < 6)
        for (int col = 0; col < 8; ++col) {
            if (state_.keyboard_matrix[row][col]) {
                // Key is pressed
                // Column is XOR'd with 0x07 (inverted), placed in bits 5-7
                // Bit 4 = 0 (indicates key pressed)
                uint8 inverted_col = col ^ 0x07;
                return (lower_nibble & 0x0F) | (inverted_col << 5);
            }
        }
    }
    
    // No key pressed - set bits 4-7 all high (0xF0)
    // This matches o2em: p2 = p2 | 0xF0
    return lower_nibble | 0xF0;
}

void InputHandler::set_joystick_state(uint8 joystick, Direction direction, bool pressed) {
    if (joystick > 1) return;
    
    bool* joy = (joystick == 0) ? state_.joystick1 : state_.joystick2;
    
    // Update state
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
    // P20-P22 select joystick (per doc/reference/o2doc.md section 5.0)
    // "to read the right joystick set P20...P22 to 0 and to read the left joystick
    //  set P20, P21 to 0 and P22 to 1"
    // So: P2 & 7 == 0 → right joystick, P2 & 7 == 4 → left joystick
    // o2em mapping: si==1 → joystick1 (left), else → joystick2 (right)
    
    uint8 joy_select = select_bits & 0x07;
    // P20=1 (value 1) selects left joystick (joystick1), P20-P22=0 selects right (joystick2)
    // Note: o2em uses si==1 for left joystick selection, matching the o2doc description
    bool* joy = (joy_select == 1) ? state_.joystick1 : state_.joystick2;
    
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
