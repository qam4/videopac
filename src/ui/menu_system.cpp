#include "ui/menu_system.h"
#include "ui/text_renderer.h"

MenuSystem::MenuSystem(SDL_Renderer* renderer, TextRenderer* text_renderer)
    : renderer_(renderer)
    , text_renderer_(text_renderer)
    , current_menu_(nullptr)
    , selected_index_(0)
    , visible_(false) {
    build_main_menu();
}

MenuSystem::~MenuSystem() {
}

void MenuSystem::show() {
    visible_ = true;
    current_menu_ = &main_menu_;
    selected_index_ = 0;
    
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

    main_menu_.push_back(MenuItem("Screenshot", MenuAction::Screenshot));
    main_menu_.push_back(MenuItem("Toggle Debugger", MenuAction::ToggleDebugger));
    main_menu_.push_back(MenuItem("Quit", MenuAction::Quit));

    current_menu_ = &main_menu_;
}

void MenuSystem::navigate_up() {
    if (selected_index_ > 0) {
        selected_index_--;
    }
}

void MenuSystem::navigate_down() {
    if (current_menu_ && selected_index_ < static_cast<int>(current_menu_->size()) - 1) {
        selected_index_++;
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
    }
}

void MenuSystem::go_back() {
    if (!menu_stack_.empty()) {
        current_menu_ = menu_stack_.top();
        menu_stack_.pop();
        selected_index_ = 0;
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

void MenuSystem::render() {
    if (!visible_ || !current_menu_) {
        return;
    }

    // Get renderer logical size (which may differ from window size)
    int screen_width, screen_height;
    SDL_RenderGetLogicalSize(renderer_, &screen_width, &screen_height);
    
    // If no logical size is set, fall back to output size
    if (screen_width == 0 || screen_height == 0) {
        SDL_GetRendererOutputSize(renderer_, &screen_width, &screen_height);
    }

    // Semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 180);
    SDL_Rect overlay = { 0, 0, screen_width, screen_height };
    SDL_RenderFillRect(renderer_, &overlay);

    // Menu box - scale to fit screen with margins
    int menu_width = (screen_width * 3) / 4;  // 75% of screen width
    int menu_height = (screen_height * 3) / 4; // 75% of screen height
    int menu_x = (screen_width - menu_width) / 2;
    int menu_y = (screen_height - menu_height) / 2;

    SDL_SetRenderDrawColor(renderer_, 40, 40, 40, 255);
    SDL_Rect menu_box = { menu_x, menu_y, menu_width, menu_height };
    SDL_RenderFillRect(renderer_, &menu_box);

    SDL_SetRenderDrawColor(renderer_, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer_, &menu_box);

    // Title
    SDL_Color title_color = { 255, 255, 255, 255 };
    std::string title = menu_stack_.empty() ? "Main Menu" : "Menu";
    text_renderer_->render_text(title, menu_x + 10, menu_y + 5,  // Reduced margins
                                title_color, TextRenderer::FontSize::Large);

    // Menu items
    render_menu_list(*current_menu_, selected_index_);

    // Hints
    SDL_Color hint_color = { 150, 150, 150, 255 };
    std::string hint = menu_stack_.empty() ? 
        "Arrows: Move | Enter: Select | Esc: Close" :  // Shortened text
        "Arrows: Move | Enter: Select | Esc: Back";
    
    text_renderer_->render_text(hint, menu_x + 10, menu_y + menu_height - 15,  // Adjusted position
                                hint_color, TextRenderer::FontSize::Small);
}

void MenuSystem::render_menu_list(const std::vector<MenuItem>& items, int selected_index) {
    // Get renderer logical size (which may differ from window size)
    int screen_width, screen_height;
    SDL_RenderGetLogicalSize(renderer_, &screen_width, &screen_height);
    
    // If no logical size is set, fall back to output size
    if (screen_width == 0 || screen_height == 0) {
        SDL_GetRendererOutputSize(renderer_, &screen_width, &screen_height);
    }

    int menu_width = (screen_width * 3) / 4;  // 75% of screen width
    int menu_height = (screen_height * 3) / 4; // 75% of screen height
    int menu_x = (screen_width - menu_width) / 2;
    int menu_y = (screen_height - menu_height) / 2;

    int item_y = menu_y + 25;  // Reduced from 60 for smaller resolution
    int line_height = 16;      // Reduced from 30 for smaller resolution

    for (size_t i = 0; i < items.size(); i++) {
        const MenuItem& item = items[i];
        
        bool is_selected = (static_cast<int>(i) == selected_index);
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

        text_renderer_->render_text(display_text, menu_x + 15, item_y,  // Reduced margin from 40 to 15
                                    text_color, TextRenderer::FontSize::Medium);

        item_y += line_height;
    }
}
