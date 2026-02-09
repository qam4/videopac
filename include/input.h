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

// Videopac keyboard keys (based on o2doc.md Appendix A)
// The keyboard is organized as an 8x8 matrix
// Rows are selected via Port 2 bits P20-P22 (0-7)
// Columns are read via Port 2 bits P20-P27 (8 bits)
enum class VidKey {
    // Row 0
    Key0 = 0x00,
    Key1 = 0x01,
    Key2 = 0x02,
    Key3 = 0x03,
    Key4 = 0x04,
    Key5 = 0x05,
    Key6 = 0x06,
    Key7 = 0x07,
    
    // Row 1
    Key8 = 0x10,
    Key9 = 0x11,
    Minus = 0x12,      // -
    Plus = 0x13,       // +
    Multiply = 0x14,   // *
    Divide = 0x15,     // /
    Equal = 0x16,      // =
    Yes = 0x17,
    
    // Row 2
    KeyQ = 0x20,
    KeyW = 0x21,
    KeyE = 0x22,
    KeyR = 0x23,
    KeyT = 0x24,
    KeyY = 0x25,
    KeyU = 0x26,
    KeyI = 0x27,
    
    // Row 3
    KeyA = 0x30,
    KeyS = 0x31,
    KeyD = 0x32,
    KeyF = 0x33,
    KeyG = 0x34,
    KeyH = 0x35,
    KeyJ = 0x36,
    KeyK = 0x37,
    
    // Row 4
    KeyZ = 0x40,
    KeyX = 0x41,
    KeyC = 0x42,
    KeyV = 0x43,
    KeyB = 0x44,
    KeyN = 0x45,
    KeyM = 0x46,
    Period = 0x47,     // .
    
    // Row 5
    Space = 0x50,
    Question = 0x51,   // ?
    KeyL = 0x52,
    KeyP = 0x53,
    KeyO = 0x54,
    Clear = 0x55,
    Enter = 0x56,
    No = 0x57,
    
    // Rows 6-7 are typically unused or for special keys
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
