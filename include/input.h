#ifndef VIDEOPAC_INPUT_H
#define VIDEOPAC_INPUT_H

#include "types.h"
#include <map>

namespace videopac {

// Joystick direction
enum class Direction {
    Up,
    Down,
    Left,
    Right
};

// Videopac keyboard keys
// The keyboard is organized as a 6x8 matrix (rows 0-5, cols 0-7)
// Rows are selected via Port 2 bits P20-P22 (0-5)
// Columns are read via Port 2 bits P25-P27 (3 bits, XOR'd with 0x07)
// Layout matches o2em vmachine.c key_map[6][8]
// Encoding: high nibble = row, low nibble = column
enum class VidKey : uint8 {
    // Row 0: Number keys 0-7
    Key0 = 0x00, Key1 = 0x01, Key2 = 0x02, Key3 = 0x03,
    Key4 = 0x04, Key5 = 0x05, Key6 = 0x06, Key7 = 0x07,
    
    // Row 1: 8, 9, (unused), (unused), SPACE, /, L, P
    Key8 = 0x10, Key9 = 0x11,
    Space = 0x14, Slash = 0x15, KeyL = 0x16, KeyP = 0x17,
    
    // Row 2: +, W, E, R, T, U, I, O
    Plus = 0x20, KeyW = 0x21, KeyE = 0x22, KeyR = 0x23,
    KeyT = 0x24, KeyU = 0x25, KeyI = 0x26, KeyO = 0x27,
    
    // Row 3: Q, S, D, F, G, H, J, K
    KeyQ = 0x30, KeyS = 0x31, KeyD = 0x32, KeyF = 0x33,
    KeyG = 0x34, KeyH = 0x35, KeyJ = 0x36, KeyK = 0x37,
    
    // Row 4: A, Z, X, C, V, B, M, .
    KeyA = 0x40, KeyZ = 0x41, KeyX = 0x42, KeyC = 0x43,
    KeyV = 0x44, KeyB = 0x45, KeyM = 0x46, Period = 0x47,
    
    // Row 5: -, *, /(numpad), =, Y, N, DEL(Clear), ENTER
    Minus = 0x50, Multiply = 0x51, Divide = 0x52, Equal = 0x53,
    KeyY = 0x54, KeyN = 0x55, Clear = 0x56, Enter = 0x57,
};

// Input handler state
struct InputState {
    bool keyboard_matrix[8][8];  // 8x8 keyboard matrix (row, col)
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
    void set_key_state(VidKey key, bool pressed);
    uint8 read_keyboard(uint8 row_select);
    
    // Joystick input
    void set_joystick_state(uint8 joystick, Direction direction, bool pressed);
    void set_joystick_button(uint8 joystick, bool pressed);
    uint8 read_joystick(uint8 select_bits);
    
    // Host key mapping
    void map_host_key(int host_key, VidKey vid_key);
    void process_host_key(int host_key, bool pressed);
    
    // Reset
    void reset();
    
    // State management
    InputState get_state() const;
    void set_state(const InputState& state);

private:
    InputState state_;
    std::map<int, VidKey> key_mapping_;  // Host key to Videopac key mapping
    
    // Helper to extract row and column from VidKey
    static uint8 get_row(VidKey key) { return (static_cast<uint8>(key) >> 4) & 0x07; }
    static uint8 get_col(VidKey key) { return static_cast<uint8>(key) & 0x07; }
};

} // namespace videopac

#endif // VIDEOPAC_INPUT_H
