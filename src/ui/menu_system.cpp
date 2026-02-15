#include "ui/menu_system.h"
#include "ui/text_renderer.h"
#include "ui/config_manager.h"
#include "ui/save_state_manager.h"
#include <ctime>

MenuSystem::MenuSystem(SDL_Renderer* renderer, TextRenderer* text_renderer)
    : renderer_(renderer)
    , text_renderer_(text_renderer)
    , current_menu_(nullptr)
    , selected_index_(0)
    , scroll_offset_(0)
    , last_selected_slot_(-1)
    , visible_(false) {
    build_main_menu();
}

MenuSystem::~MenuSystem() {
}

void MenuSystem::show() {
    visible_ = true;
    current_menu_ = &main_menu_;
    selected_index_ = 0;
    scroll_offset_ = 0;
    
    // Clear menu stack
    while (!menu_stack_.empty()) {
        menu_stack_.pop();
    }
}

void MenuSystem::hide() {
    visible_ = false;
}

void MenuSystem::build_main_menu() {
    main_menu_.clear();

    using videopac::MenuAction;

    // File menu items
    main_menu_.push_back(MenuItem("Load BIOS", MenuAction::LoadBIOS));
    main_menu_.push_back(MenuItem("Load ROM", MenuAction::LoadROM));
    main_menu_.push_back(MenuItem("Reset", MenuAction::Reset));

    // Save state menu
    MenuItem save_state_menu("Save State", MenuAction::SaveState);
    save_state_menu.has_submenu = true;
    for (int i = 0; i < 10; i++) {
        MenuItem slot_item("Slot " + std::to_string(i), MenuAction::SaveState);
        slot_item.slot_number = i;
        save_state_menu.submenu.push_back(slot_item);
    }
    main_menu_.push_back(save_state_menu);

    // Load state menu
    MenuItem load_state_menu("Load State", MenuAction::LoadState);
    load_state_menu.has_submenu = true;
    for (int i = 0; i < 10; i++) {
        MenuItem slot_item("Slot " + std::to_string(i), MenuAction::LoadState);
        slot_item.slot_number = i;
        load_state_menu.submenu.push_back(slot_item);
    }
    main_menu_.push_back(load_state_menu);

    // Video Settings submenu
    MenuItem video_settings_menu("Video Settings", MenuAction::VideoSettings);
    video_settings_menu.has_submenu = true;
    
    // Scaling Filter submenu
    MenuItem scaling_filter_menu("Scaling Filter", MenuAction::None);
    scaling_filter_menu.has_submenu = true;
    scaling_filter_menu.submenu.push_back(MenuItem("Nearest", MenuAction::ScalingFilterNearest));
    scaling_filter_menu.submenu.push_back(MenuItem("Linear", MenuAction::ScalingFilterLinear));
    video_settings_menu.submenu.push_back(scaling_filter_menu);
    
    // Aspect Ratio submenu
    MenuItem aspect_ratio_menu("Aspect Ratio", MenuAction::None);
    aspect_ratio_menu.has_submenu = true;
    aspect_ratio_menu.submenu.push_back(MenuItem("Original", MenuAction::AspectRatioOriginal));
    aspect_ratio_menu.submenu.push_back(MenuItem("4:3", MenuAction::AspectRatio4_3));
    aspect_ratio_menu.submenu.push_back(MenuItem("Stretch", MenuAction::AspectRatioStretch));
    video_settings_menu.submenu.push_back(aspect_ratio_menu);
    
    // VSync toggle
    video_settings_menu.submenu.push_back(MenuItem("VSync", MenuAction::ToggleVSync));
    
    // CRT Effects submenu
    MenuItem crt_effects_menu("CRT Effects", MenuAction::None);
    crt_effects_menu.has_submenu = true;
    crt_effects_menu.submenu.push_back(MenuItem("None", MenuAction::CRTEffectNone));
    crt_effects_menu.submenu.push_back(MenuItem("Light", MenuAction::CRTEffectLight));
    crt_effects_menu.submenu.push_back(MenuItem("Medium", MenuAction::CRTEffectMedium));
    crt_effects_menu.submenu.push_back(MenuItem("Heavy", MenuAction::CRTEffectHeavy));
    video_settings_menu.submenu.push_back(crt_effects_menu);
    
    // Scanlines submenu
    MenuItem scanlines_menu("Scanlines", MenuAction::None);
    scanlines_menu.has_submenu = true;
    scanlines_menu.submenu.push_back(MenuItem("Off", MenuAction::ScanlinesOff));
    scanlines_menu.submenu.push_back(MenuItem("25%", MenuAction::Scanlines25));
    scanlines_menu.submenu.push_back(MenuItem("50%", MenuAction::Scanlines50));
    scanlines_menu.submenu.push_back(MenuItem("75%", MenuAction::Scanlines75));
    video_settings_menu.submenu.push_back(scanlines_menu);
    
    main_menu_.push_back(video_settings_menu);

    main_menu_.push_back(MenuItem("Display Info", MenuAction::DisplayInfo));
    main_menu_.push_back(MenuItem("Screenshot", MenuAction::Screenshot));
    main_menu_.push_back(MenuItem("Toggle Debugger", MenuAction::ToggleDebugger));
    main_menu_.push_back(MenuItem("Toggle Fullscreen", MenuAction::ToggleFullscreen));
    main_menu_.push_back(MenuItem("Quit", MenuAction::Quit));

    current_menu_ = &main_menu_;
}

void MenuSystem::navigate_up() {
    if (selected_index_ > 0) {
        selected_index_--;
        
        // Adjust scroll offset if needed
        if (selected_index_ < scroll_offset_) {
            scroll_offset_ = selected_index_;
        }
    }
}

void MenuSystem::navigate_down() {
    if (current_menu_ && selected_index_ < static_cast<int>(current_menu_->size()) - 1) {
        selected_index_++;
        
        // Adjust scroll offset if needed
        // Calculate max visible items based on 320x240 logical size
        const int screen_height = 240;
        
        int margin = screen_height / 24;
        int menu_height = screen_height - (margin * 2);
        int title_height = screen_height / 15;
        int hint_height = screen_height / 20;
        int available_height = menu_height - title_height - hint_height;
        int line_height = screen_height / 25;
        int max_visible_items = available_height / line_height;
        
        if (selected_index_ >= scroll_offset_ + max_visible_items) {
            scroll_offset_ = selected_index_ - max_visible_items + 1;
        }
    }
}

void MenuSystem::select_current() {
    if (!current_menu_ || current_menu_->empty()) {
        return;
    }

    MenuItem& item = (*current_menu_)[selected_index_];

    if (!item.enabled) {
        return;
    }

    if (item.has_submenu && !item.submenu.empty()) {
        // Navigate into submenu
        menu_stack_.push(current_menu_);
        current_menu_ = &item.submenu;
        selected_index_ = 0;
        scroll_offset_ = 0;  // Reset scroll for new menu
    }
}

void MenuSystem::go_back() {
    if (!menu_stack_.empty()) {
        current_menu_ = menu_stack_.top();
        menu_stack_.pop();
        selected_index_ = 0;
        scroll_offset_ = 0;  // Reset scroll when going back
    } else {
        hide();
    }
}

videopac::MenuAction MenuSystem::process_input(SDL_Keycode key) {
    using videopac::MenuAction;
    
    if (!visible_) {
        return MenuAction::None;
    }

    switch (key) {
        case SDLK_UP:
            navigate_up();
            return MenuAction::None;

        case SDLK_DOWN:
            navigate_down();
            return MenuAction::None;

        case SDLK_RETURN:
            if (current_menu_ && !current_menu_->empty()) {
                MenuItem& item = (*current_menu_)[selected_index_];
                
                if (item.has_submenu) {
                    select_current();
                    return MenuAction::None;
                } else {
                    // Store the slot number before returning the action
                    last_selected_slot_ = item.slot_number;
                    // Return the action
                    return item.action;
                }
            }
            return MenuAction::None;

        case SDLK_ESCAPE:
            go_back();
            return MenuAction::None;

        default:
            return MenuAction::None;
    }
}

int MenuSystem::get_selected_slot() const {
    return last_selected_slot_;
}

void MenuSystem::render() {
    if (!visible_ || !current_menu_) {
        return;
    }

    // Always use logical size (320x240) for consistent menu rendering
    int screen_width = 320;
    int screen_height = 240;

    // Semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 180);
    SDL_Rect overlay = { 0, 0, screen_width, screen_height };
    SDL_RenderFillRect(renderer_, &overlay);

    // Menu box - use most of screen with reasonable margins
    int margin = screen_height / 24;  // Scale margin with screen size
    int menu_width = screen_width - (margin * 2);
    int menu_height = screen_height - (margin * 2);
    int menu_x = margin;
    int menu_y = margin;

    SDL_SetRenderDrawColor(renderer_, 40, 40, 40, 255);
    SDL_Rect menu_box = { menu_x, menu_y, menu_width, menu_height };
    SDL_RenderFillRect(renderer_, &menu_box);

    SDL_SetRenderDrawColor(renderer_, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer_, &menu_box);

    // Title - use larger font at native resolution
    SDL_Color title_color = { 255, 255, 255, 255 };
    std::string title = menu_stack_.empty() ? "Main Menu" : "Menu";
    int title_padding = screen_height / 80;  // Scale padding
    text_renderer_->render_text(title, menu_x + title_padding, menu_y + title_padding,
                                title_color, TextRenderer::FontSize::Large);

    // Menu items
    render_menu_list(*current_menu_, selected_index_);

    // Hints - at the very bottom
    SDL_Color hint_color = { 150, 150, 150, 255 };
    std::string hint = menu_stack_.empty() ? 
        "Arrows: Move | Enter: Select | Esc: Close" :
        "Arrows: Move | Enter: Select | Esc: Back";
    
    int hint_padding = screen_height / 60;
    text_renderer_->render_text(hint, menu_x + hint_padding, menu_y + menu_height - hint_padding - 10,
                                hint_color, TextRenderer::FontSize::Medium);
}

void MenuSystem::render_menu_list(const std::vector<MenuItem>& items, int selected_index) {
    // Always use logical size (320x240) for consistent rendering
    int screen_width = 320;
    int screen_height = 240;

    int margin = screen_height / 24;
    int menu_width = screen_width - (margin * 2);
    int menu_height = screen_height - (margin * 2);
    int menu_x = margin;
    int menu_y = margin;

    // Scale spacing based on screen height
    int title_height = screen_height / 15;  // Space for title
    int hint_height = screen_height / 20;   // Space for hints
    int item_y = menu_y + title_height;
    int line_height = screen_height / 25;   // Scale line height with screen
    
    // Calculate how many items can fit
    int available_height = menu_height - title_height - hint_height;
    int max_visible_items = available_height / line_height;
    
    // Calculate which items to display
    int start_index = scroll_offset_;
    int end_index = std::min(start_index + max_visible_items, static_cast<int>(items.size()));

    // Render visible items
    for (int i = start_index; i < end_index; i++) {
        const MenuItem& item = items[i];
        
        bool is_selected = (i == selected_index);
        SDL_Color text_color;
        
        if (!item.enabled) {
            text_color = { 100, 100, 100, 255 };
        } else if (is_selected) {
            text_color = { 255, 255, 100, 255 };
        } else {
            text_color = { 200, 200, 200, 255 };
        }

        std::string display_text = item.label;
        if (item.has_submenu) {
            display_text += " >";
        }
        
        // Add value if present
        if (!item.value.empty()) {
            display_text += ": " + item.value;
        }

        int item_padding = screen_height / 60;
        text_renderer_->render_text(display_text, menu_x + item_padding, item_y,
                                    text_color, TextRenderer::FontSize::Medium);

        item_y += line_height;
    }
    
    // Draw scroll indicators if needed
    if (scroll_offset_ > 0) {
        // Up arrow indicator
        SDL_Color arrow_color = { 150, 150, 150, 255 };
        text_renderer_->render_text("^", menu_x + menu_width - (screen_width / 40), menu_y + title_height,
                                    arrow_color, TextRenderer::FontSize::Medium);
    }
    
    if (end_index < static_cast<int>(items.size())) {
        // Down arrow indicator
        SDL_Color arrow_color = { 150, 150, 150, 255 };
        text_renderer_->render_text("v", menu_x + menu_width - (screen_width / 40), 
                                    menu_y + menu_height - hint_height - (screen_height / 40),
                                    arrow_color, TextRenderer::FontSize::Medium);
    }
}

void MenuSystem::update_menu_values(ConfigManager* config_manager) {
    if (!config_manager) {
        return;
    }
    
    // Find Video Settings menu
    for (auto& item : main_menu_) {
        if (item.action == videopac::MenuAction::VideoSettings && item.has_submenu) {
            // Update VSync value
            for (auto& video_item : item.submenu) {
                if (video_item.action == videopac::MenuAction::ToggleVSync) {
                    video_item.value = config_manager->get_vsync_enabled() ? "On" : "Off";
                }
            }
        }
    }
}

void MenuSystem::update_save_state_slots(SaveStateManagerUI* save_state_manager, const std::string& rom_name) {
    if (!save_state_manager) {
        return;
    }
    
    // If no ROM name, use "unknown"
    std::string actual_rom_name = rom_name.empty() ? "unknown" : rom_name;
    
    // Extract just the filename without extension for display
    std::string display_name = actual_rom_name;
    size_t last_dot = display_name.find_last_of('.');
    if (last_dot != std::string::npos) {
        display_name = display_name.substr(0, last_dot);
    }
    // Truncate if too long (keep first 15 chars)
    if (display_name.length() > 15) {
        display_name = display_name.substr(0, 15) + "...";
    }
    
    // Get list of save states for this ROM
    auto states = save_state_manager->list_states(actual_rom_name);
    
    // Find Save State menu
    for (auto& item : main_menu_) {
        if (item.label == "Save State" && item.has_submenu) {
            // Update each slot
            for (size_t i = 0; i < item.submenu.size() && i < states.size(); ++i) {
                if (states[i].exists) {
                    // Format: ROM - timestamp
                    time_t timestamp = states[i].timestamp;
                    struct tm* timeinfo = localtime(&timestamp);
                    char time_str[32];
                    strftime(time_str, sizeof(time_str), "%m/%d %H:%M", timeinfo);
                    item.submenu[i].value = display_name + " - " + std::string(time_str);
                } else {
                    item.submenu[i].value = "[Empty]";
                }
            }
        }
        
        // Find Load State menu
        if (item.label == "Load State" && item.has_submenu) {
            // Update each slot
            for (size_t i = 0; i < item.submenu.size() && i < states.size(); ++i) {
                if (states[i].exists) {
                    // Format: ROM - timestamp
                    time_t timestamp = states[i].timestamp;
                    struct tm* timeinfo = localtime(&timestamp);
                    char time_str[32];
                    strftime(time_str, sizeof(time_str), "%m/%d %H:%M", timeinfo);
                    item.submenu[i].value = display_name + " - " + std::string(time_str);
                    item.submenu[i].enabled = true;
                } else {
                    item.submenu[i].value = "[Empty]";
                    item.submenu[i].enabled = false;  // Disable empty slots
                }
            }
        }
    }
}
