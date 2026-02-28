#ifndef INPUT_MAPPER_H
#define INPUT_MAPPER_H

#include <SDL.h>
#include <string>
#include <map>
#include <vector>
#include <utility>

// Forward declaration
class ConfigManager;

// Input action types for emulator controls
enum class Action {
    Up,
    Down,
    Left,
    Right,
    Button
};

// Input device types
enum class InputDevice {
    Keyboard,
    Joystick0,
    Joystick1,
    Joystick2,
    Joystick3
};

// Joystick input representation (button or axis)
struct JoystickInput {
    int joystick_id;      // SDL joystick instance ID
    int button;           // Button index (-1 if axis)
    int axis;             // Axis index (-1 if button)
    int axis_direction;   // -1 or 1 for axis direction

    JoystickInput() : joystick_id(-1), button(-1), axis(-1), axis_direction(0) {}
    
    bool is_button() const { return button >= 0; }
    bool is_axis() const { return axis >= 0; }
};

// Joystick information
struct JoystickInfo {
    int id;               // SDL joystick instance ID
    std::string name;     // Joystick name
};

// Input mapper for keyboard and joystick configuration
// Manages input mappings for both players and joystick detection
class InputMapper {
public:
    InputMapper();
    ~InputMapper();

    // Keyboard mapping
    void set_keyboard_mapping(int player, Action action, SDL_Keycode key);
    SDL_Keycode get_keyboard_mapping(int player, Action action) const;

    // Joystick mapping
    void set_joystick_mapping(int player, Action action, const JoystickInput& input);
    JoystickInput get_joystick_mapping(int player, Action action) const;

    // Device assignment
    void set_player_device(int player, InputDevice device);
    InputDevice get_player_device(int player) const;

    // Joystick detection and management
    void detect_joysticks();
    std::vector<JoystickInfo> get_connected_joysticks() const;
    int get_joystick_count() const;

    // Configuration persistence
    void load_from_config(ConfigManager& config);
    void save_to_config(ConfigManager& config);

    // Helper methods
    static std::string action_to_string(Action action);
    static Action string_to_action(const std::string& str);
    static std::string keycode_to_string(SDL_Keycode key);
    static SDL_Keycode string_to_keycode(const std::string& str);

private:
    // Initialize default keyboard mappings
    void init_default_mappings();

    // Keyboard mappings: (player, action) -> keycode
    std::map<std::pair<int, Action>, SDL_Keycode> keyboard_mappings_;

    // Joystick mappings: (player, action) -> joystick input
    std::map<std::pair<int, Action>, JoystickInput> joystick_mappings_;

    // Player device assignments: player -> device
    std::map<int, InputDevice> player_devices_;

    // Connected joysticks: instance ID -> SDL_Joystick*
    std::map<int, SDL_Joystick*> joysticks_;

    // Joystick info: instance ID -> info
    std::map<int, JoystickInfo> joystick_info_;
};

#endif // INPUT_MAPPER_H
