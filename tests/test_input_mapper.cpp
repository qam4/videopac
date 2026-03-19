#include <gtest/gtest.h>
#include "ui/input_mapper.h"
#include "ui/config_manager.h"
#include <SDL.h>

class InputMapperTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize SDL for joystick support
        SDL_Init(SDL_INIT_JOYSTICK);
        
        mapper_ = new InputMapper();
        config_ = new ConfigManager();
    }
    
    void TearDown() override {
        delete mapper_;
        delete config_;
        SDL_Quit();
    }
    
    InputMapper* mapper_;
    ConfigManager* config_;
};

// Test default keyboard mappings
TEST_F(InputMapperTest, DefaultKeyboardMappings) {
    // Player 1 should have numpad keys
    EXPECT_EQ(mapper_->get_keyboard_mapping(0, Action::Up), SDLK_KP_8);
    EXPECT_EQ(mapper_->get_keyboard_mapping(0, Action::Down), SDLK_KP_2);
    EXPECT_EQ(mapper_->get_keyboard_mapping(0, Action::Left), SDLK_KP_4);
    EXPECT_EQ(mapper_->get_keyboard_mapping(0, Action::Right), SDLK_KP_6);
    EXPECT_EQ(mapper_->get_keyboard_mapping(0, Action::Button), SDLK_KP_0);
    
    // Player 2 should have arrow keys + right ctrl
    EXPECT_EQ(mapper_->get_keyboard_mapping(1, Action::Up), SDLK_UP);
    EXPECT_EQ(mapper_->get_keyboard_mapping(1, Action::Down), SDLK_DOWN);
    EXPECT_EQ(mapper_->get_keyboard_mapping(1, Action::Left), SDLK_LEFT);
    EXPECT_EQ(mapper_->get_keyboard_mapping(1, Action::Right), SDLK_RIGHT);
    EXPECT_EQ(mapper_->get_keyboard_mapping(1, Action::Button), SDLK_SPACE);
}

// Test keyboard mapping changes
TEST_F(InputMapperTest, SetKeyboardMapping) {
    // Change Player 1 up key to 'i'
    mapper_->set_keyboard_mapping(0, Action::Up, SDLK_i);
    EXPECT_EQ(mapper_->get_keyboard_mapping(0, Action::Up), SDLK_i);
    
    // Other mappings should remain unchanged
    EXPECT_EQ(mapper_->get_keyboard_mapping(0, Action::Down), SDLK_KP_2);
    EXPECT_EQ(mapper_->get_keyboard_mapping(0, Action::Left), SDLK_KP_4);
}

// Test joystick mapping
TEST_F(InputMapperTest, JoystickMapping) {
    JoystickInput input;
    input.joystick_id = 0;
    input.button = 0;
    input.axis = -1;
    
    mapper_->set_joystick_mapping(0, Action::Button, input);
    
    JoystickInput retrieved = mapper_->get_joystick_mapping(0, Action::Button);
    EXPECT_EQ(retrieved.joystick_id, 0);
    EXPECT_EQ(retrieved.button, 0);
    EXPECT_EQ(retrieved.axis, -1);
    EXPECT_TRUE(retrieved.is_button());
    EXPECT_FALSE(retrieved.is_axis());
}

// Test joystick axis mapping
TEST_F(InputMapperTest, JoystickAxisMapping) {
    JoystickInput input;
    input.joystick_id = 0;
    input.button = -1;
    input.axis = 1;
    input.axis_direction = -1;
    
    mapper_->set_joystick_mapping(0, Action::Up, input);
    
    JoystickInput retrieved = mapper_->get_joystick_mapping(0, Action::Up);
    EXPECT_EQ(retrieved.joystick_id, 0);
    EXPECT_EQ(retrieved.button, -1);
    EXPECT_EQ(retrieved.axis, 1);
    EXPECT_EQ(retrieved.axis_direction, -1);
    EXPECT_FALSE(retrieved.is_button());
    EXPECT_TRUE(retrieved.is_axis());
}

// Test device assignment
TEST_F(InputMapperTest, DeviceAssignment) {
    // Default should be keyboard
    EXPECT_EQ(mapper_->get_player_device(0), InputDevice::Keyboard);
    EXPECT_EQ(mapper_->get_player_device(1), InputDevice::Keyboard);
    
    // Change Player 1 to Joystick0
    mapper_->set_player_device(0, InputDevice::Joystick0);
    EXPECT_EQ(mapper_->get_player_device(0), InputDevice::Joystick0);
    
    // Player 2 should remain keyboard
    EXPECT_EQ(mapper_->get_player_device(1), InputDevice::Keyboard);
}

// Test joystick detection
TEST_F(InputMapperTest, JoystickDetection) {
    mapper_->detect_joysticks();
    
    // We can't guarantee joysticks are connected in test environment
    // Just verify the method doesn't crash and returns a valid count
    int count = mapper_->get_joystick_count();
    EXPECT_GE(count, 0);
    
    std::vector<JoystickInfo> joysticks = mapper_->get_connected_joysticks();
    EXPECT_EQ(joysticks.size(), static_cast<size_t>(count));
}

// Test configuration persistence
TEST_F(InputMapperTest, ConfigurationPersistence) {
    // Set custom mappings
    mapper_->set_keyboard_mapping(0, Action::Up, SDLK_i);
    mapper_->set_keyboard_mapping(0, Action::Down, SDLK_k);
    mapper_->set_player_device(0, InputDevice::Joystick0);
    
    // Save to config
    mapper_->save_to_config(*config_);
    
    // Create new mapper and load from config
    InputMapper* new_mapper = new InputMapper();
    new_mapper->load_from_config(*config_);
    
    // Verify mappings were restored
    EXPECT_EQ(new_mapper->get_keyboard_mapping(0, Action::Up), SDLK_i);
    EXPECT_EQ(new_mapper->get_keyboard_mapping(0, Action::Down), SDLK_k);
    EXPECT_EQ(new_mapper->get_player_device(0), InputDevice::Joystick0);
    
    delete new_mapper;
}

// Test action string conversion
TEST_F(InputMapperTest, ActionStringConversion) {
    EXPECT_EQ(InputMapper::action_to_string(Action::Up), "up");
    EXPECT_EQ(InputMapper::action_to_string(Action::Down), "down");
    EXPECT_EQ(InputMapper::action_to_string(Action::Left), "left");
    EXPECT_EQ(InputMapper::action_to_string(Action::Right), "right");
    EXPECT_EQ(InputMapper::action_to_string(Action::Button), "button");
    
    EXPECT_EQ(InputMapper::string_to_action("up"), Action::Up);
    EXPECT_EQ(InputMapper::string_to_action("down"), Action::Down);
    EXPECT_EQ(InputMapper::string_to_action("left"), Action::Left);
    EXPECT_EQ(InputMapper::string_to_action("right"), Action::Right);
    EXPECT_EQ(InputMapper::string_to_action("button"), Action::Button);
}

// Test keycode string conversion
TEST_F(InputMapperTest, KeycodeStringConversion) {
    // Test some common keys
    std::string up_name = InputMapper::keycode_to_string(SDLK_UP);
    EXPECT_FALSE(up_name.empty());
    
    SDL_Keycode up_key = InputMapper::string_to_keycode(up_name);
    EXPECT_EQ(up_key, SDLK_UP);
    
    // Test space key
    std::string space_name = InputMapper::keycode_to_string(SDLK_SPACE);
    EXPECT_FALSE(space_name.empty());
    
    SDL_Keycode space_key = InputMapper::string_to_keycode(space_name);
    EXPECT_EQ(space_key, SDLK_SPACE);
}

// Test multiple players don't interfere
TEST_F(InputMapperTest, MultiplePlayersIndependent) {
    // Change Player 1 mappings
    mapper_->set_keyboard_mapping(0, Action::Up, SDLK_i);
    mapper_->set_player_device(0, InputDevice::Joystick0);
    
    // Player 2 should still have defaults
    EXPECT_EQ(mapper_->get_keyboard_mapping(1, Action::Up), SDLK_UP);
    EXPECT_EQ(mapper_->get_player_device(1), InputDevice::Keyboard);
    
    // Change Player 2 mappings
    mapper_->set_keyboard_mapping(1, Action::Up, SDLK_t);
    mapper_->set_player_device(1, InputDevice::Joystick1);
    
    // Player 1 should remain unchanged
    EXPECT_EQ(mapper_->get_keyboard_mapping(0, Action::Up), SDLK_i);
    EXPECT_EQ(mapper_->get_player_device(0), InputDevice::Joystick0);
}

// Test getting unmapped action returns default
TEST_F(InputMapperTest, UnmappedActionReturnsDefault) {
    // Create a fresh mapper
    InputMapper fresh_mapper;
    
    // Clear a mapping by setting it to unknown
    fresh_mapper.set_keyboard_mapping(0, Action::Up, SDLK_UNKNOWN);
    
    // Should return SDLK_UNKNOWN
    EXPECT_EQ(fresh_mapper.get_keyboard_mapping(0, Action::Up), SDLK_UNKNOWN);
}
