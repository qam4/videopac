#include "ui/input_mapper.h"
#include "ui/config_manager.h"
#include <iostream>

InputMapper::InputMapper() {
    init_default_mappings();
}

InputMapper::~InputMapper() {
    // Close all opened joysticks
    for (auto& pair : joysticks_) {
        if (pair.second) {
            SDL_JoystickClose(pair.second);
        }
    }
    joysticks_.clear();
}

void InputMapper::init_default_mappings() {
    // Player 1: Arrow keys + Space
    keyboard_mappings_[{0, Action::Up}] = SDLK_UP;
    keyboard_mappings_[{0, Action::Down}] = SDLK_DOWN;
    keyboard_mappings_[{0, Action::Left}] = SDLK_LEFT;
    keyboard_mappings_[{0, Action::Right}] = SDLK_RIGHT;
    keyboard_mappings_[{0, Action::Button}] = SDLK_SPACE;

    // Player 2: WASD + Left Shift
    keyboard_mappings_[{1, Action::Up}] = SDLK_w;
    keyboard_mappings_[{1, Action::Down}] = SDLK_s;
    keyboard_mappings_[{1, Action::Left}] = SDLK_a;
    keyboard_mappings_[{1, Action::Right}] = SDLK_d;
    keyboard_mappings_[{1, Action::Button}] = SDLK_LSHIFT;

    // Default to keyboard for both players
    player_devices_[0] = InputDevice::Keyboard;
    player_devices_[1] = InputDevice::Keyboard;
}

void InputMapper::set_keyboard_mapping(int player, Action action, SDL_Keycode key) {
    keyboard_mappings_[{player, action}] = key;
}

SDL_Keycode InputMapper::get_keyboard_mapping(int player, Action action) const {
    auto it = keyboard_mappings_.find({player, action});
    if (it != keyboard_mappings_.end()) {
        return it->second;
    }
    return SDLK_UNKNOWN;
}

void InputMapper::set_joystick_mapping(int player, Action action, const JoystickInput& input) {
    joystick_mappings_[{player, action}] = input;
}

JoystickInput InputMapper::get_joystick_mapping(int player, Action action) const {
    auto it = joystick_mappings_.find({player, action});
    if (it != joystick_mappings_.end()) {
        return it->second;
    }
    return JoystickInput();
}

void InputMapper::set_player_device(int player, InputDevice device) {
    player_devices_[player] = device;
}

InputDevice InputMapper::get_player_device(int player) const {
    auto it = player_devices_.find(player);
    if (it != player_devices_.end()) {
        return it->second;
    }
    return InputDevice::Keyboard;
}

void InputMapper::detect_joysticks() {
    // Close any previously opened joysticks
    for (auto& pair : joysticks_) {
        if (pair.second) {
            SDL_JoystickClose(pair.second);
        }
    }
    joysticks_.clear();
    joystick_info_.clear();

    // Detect and open all connected joysticks
    int num_joysticks = SDL_NumJoysticks();
    std::cout << "[InputMapper] Detected " << num_joysticks << " joystick(s)" << std::endl;

    for (int i = 0; i < num_joysticks; ++i) {
        SDL_Joystick* joystick = SDL_JoystickOpen(i);
        if (joystick) {
            int instance_id = SDL_JoystickInstanceID(joystick);
            const char* name = SDL_JoystickName(joystick);
            
            joysticks_[instance_id] = joystick;
            joystick_info_[instance_id] = {instance_id, name ? name : "Unknown Joystick"};
            
            std::cout << "[InputMapper] Opened joystick " << instance_id 
                      << ": " << joystick_info_[instance_id].name << std::endl;
        } else {
            std::cerr << "[InputMapper] Failed to open joystick " << i << std::endl;
        }
    }
}

std::vector<JoystickInfo> InputMapper::get_connected_joysticks() const {
    std::vector<JoystickInfo> result;
    for (const auto& pair : joystick_info_) {
        result.push_back(pair.second);
    }
    return result;
}

int InputMapper::get_joystick_count() const {
    return static_cast<int>(joysticks_.size());
}

void InputMapper::load_from_config(ConfigManager& config) {
    // Load keyboard mappings for both players
    for (int player = 0; player < 2; ++player) {
        for (int a = 0; a < 5; ++a) {
            Action action = static_cast<Action>(a);
            std::string action_str = action_to_string(action);
            std::string key_str = config.get_keyboard_mapping(player, action_str);
            
            if (!key_str.empty()) {
                SDL_Keycode key = string_to_keycode(key_str);
                if (key != SDLK_UNKNOWN) {
                    keyboard_mappings_[{player, action}] = key;
                }
            }
        }

        // Load joystick device assignment
        int device_id = config.get_joystick_device(player);
        if (device_id >= 0) {
            // Map device_id to InputDevice enum
            if (device_id == 0) player_devices_[player] = InputDevice::Joystick0;
            else if (device_id == 1) player_devices_[player] = InputDevice::Joystick1;
            else if (device_id == 2) player_devices_[player] = InputDevice::Joystick2;
            else if (device_id == 3) player_devices_[player] = InputDevice::Joystick3;
        } else {
            player_devices_[player] = InputDevice::Keyboard;
        }
    }
}

void InputMapper::save_to_config(ConfigManager& config) {
    // Save keyboard mappings for both players
    for (int player = 0; player < 2; ++player) {
        for (int a = 0; a < 5; ++a) {
            Action action = static_cast<Action>(a);
            std::string action_str = action_to_string(action);
            SDL_Keycode key = get_keyboard_mapping(player, action);
            
            if (key != SDLK_UNKNOWN) {
                std::string key_str = keycode_to_string(key);
                config.set_keyboard_mapping(player, action_str, key_str);
            }
        }

        // Save joystick device assignment
        InputDevice device = get_player_device(player);
        int device_id = -1;
        if (device == InputDevice::Joystick0) device_id = 0;
        else if (device == InputDevice::Joystick1) device_id = 1;
        else if (device == InputDevice::Joystick2) device_id = 2;
        else if (device == InputDevice::Joystick3) device_id = 3;
        
        config.set_joystick_device(player, device_id);
    }
}

std::string InputMapper::action_to_string(Action action) {
    switch (action) {
        case Action::Up: return "up";
        case Action::Down: return "down";
        case Action::Left: return "left";
        case Action::Right: return "right";
        case Action::Button: return "button";
        default: return "unknown";
    }
}

Action InputMapper::string_to_action(const std::string& str) {
    if (str == "up") return Action::Up;
    if (str == "down") return Action::Down;
    if (str == "left") return Action::Left;
    if (str == "right") return Action::Right;
    if (str == "button") return Action::Button;
    return Action::Up;  // Default
}

std::string InputMapper::keycode_to_string(SDL_Keycode key) {
    const char* name = SDL_GetKeyName(key);
    return name ? name : "Unknown";
}

SDL_Keycode InputMapper::string_to_keycode(const std::string& str) {
    return SDL_GetKeyFromName(str.c_str());
}
