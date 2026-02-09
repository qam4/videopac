#ifndef VIDEOPAC_INPUT_H
#define VIDEOPAC_INPUT_H

#include "types.h"

namespace videopac {

// Joystick direction
enum class Direction {
    Up,
    Down,
    Left,
    Right
};

// Input handler state
struct InputState {
    bool keyboard_matrix[8][8];  // 8x8 keyboard matrix
    bool joystick1[5];           // Joystick 1: up, down, left, right, fire
    bool joystick2[5];           // Joystick 2: up, down, left, right, fire
};

// Input handler
class InputHandler {
public:
    InputHandler();
    ~InputHandler() = default;
    
    // Keyboard input
    void set_key_state(uint8 row, uint8 col, bool pressed);
    uint8 read_keyboard(uint8 row_select);
    
    // Joystick input
    void set_joystick_state(uint8 joystick, Direction direction, bool pressed);
    void set_joystick_button(uint8 joystick, bool pressed);
    uint8 read_joystick(uint8 select_bits);
    
    // Host key mapping
    void map_host_key(int host_key, uint8 row, uint8 col);
    
    // Reset
    void reset();
    
    // State management
    InputState get_state() const;
    void set_state(const InputState& state);

private:
    InputState state_;
};

} // namespace videopac

#endif // VIDEOPAC_INPUT_H
