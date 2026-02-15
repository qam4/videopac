#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <string>
#include <vector>
#include <stack>
#include <SDL.h>
#include "frontend.h"  // For videopac::MenuAction

// Forward declaration
class TextRenderer;

// Menu item structure
struct MenuItem {
    std::string label;
    videopac::MenuAction action;
    std::vector<MenuItem> submenu;
    bool enabled;
    bool has_submenu;
    int slot_number;  // For save/load state slots
    std::string value;  // For displaying current setting values

    MenuItem(const std::string& lbl, videopac::MenuAction act = videopac::MenuAction::None, bool en = true)
        : label(lbl), action(act), enabled(en), has_submenu(false), slot_number(-1), value("") {}
};

// Menu system for in-game overlay menu
class MenuSystem {
public:
    MenuSystem(SDL_Renderer* renderer, TextRenderer* text_renderer);
    ~MenuSystem();

    // Show the menu
    void show();

    // Hide the menu
    void hide();

    // Check if menu is visible
    bool is_visible() const { return visible_; }

    // Process keyboard input
    // Returns the selected action (None if no action selected)
    videopac::MenuAction process_input(SDL_Keycode key);
    
    // Get the slot number of the last selected menu item
    int get_selected_slot() const;

    // Render the menu
    void render();

    // Build the main menu structure
    void build_main_menu();
    
    // Update menu items with current configuration values
    void update_menu_values(class ConfigManager* config_manager);
    
    // Update save state slot information
    void update_save_state_slots(class SaveStateManagerUI* save_state_manager, const std::string& rom_name);

private:
    // Navigation methods
    void navigate_up();
    void navigate_down();
    void select_current();
    void go_back();

    // Render a menu list
    void render_menu_list(const std::vector<MenuItem>& items, int selected_index);

    SDL_Renderer* renderer_;
    TextRenderer* text_renderer_;

    std::vector<MenuItem> main_menu_;
    std::vector<MenuItem>* current_menu_;
    std::stack<std::vector<MenuItem>*> menu_stack_;

    int selected_index_;
    int scroll_offset_;  // For scrolling long menus
    int last_selected_slot_;  // Track the slot number of last selected item
    bool visible_;
};

#endif // MENU_SYSTEM_H
