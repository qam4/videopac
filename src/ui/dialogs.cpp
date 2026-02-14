#include "ui/dialogs.h"
#include "ui/text_renderer.h"
#include <sstream>

// Base Dialog implementation
Dialog::Dialog(SDL_Renderer* renderer, TextRenderer* text_renderer)
    : renderer_(renderer)
    , text_renderer_(text_renderer)
    , visible_(false) {
}

Dialog::~Dialog() {
}

void Dialog::show() {
    visible_ = true;
}

void Dialog::hide() {
    visible_ = false;
}

void Dialog::render_overlay() {
    // Semi-transparent black overlay
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 180);
    
    int width, height;
    SDL_RenderGetLogicalSize(renderer_, &width, &height);
    if (width == 0 || height == 0) {
        SDL_GetRendererOutputSize(renderer_, &width, &height);
    }
    
    SDL_Rect overlay_rect = { 0, 0, width, height };
    SDL_RenderFillRect(renderer_, &overlay_rect);
}

void Dialog::render_box(int x, int y, int width, int height,
                        SDL_Color bg_color, SDL_Color border_color) {
    // Background
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_Rect bg_rect = { x, y, width, height };
    SDL_RenderFillRect(renderer_, &bg_rect);
    
    // Border
    SDL_SetRenderDrawColor(renderer_, border_color.r, border_color.g, border_color.b, border_color.a);
    SDL_RenderDrawRect(renderer_, &bg_rect);
}

std::vector<std::string> Dialog::word_wrap(const std::string& text, int max_width) {
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word;
    std::string current_line;
    
    while (words >> word) {
        std::string test_line = current_line.empty() ? word : current_line + " " + word;
        
        int text_width = 0;
        text_renderer_->measure_text(test_line, TextRenderer::FontSize::Medium, &text_width, nullptr);
        
        if (text_width > max_width && !current_line.empty()) {
            lines.push_back(current_line);
            current_line = word;
        } else {
            current_line = test_line;
        }
    }
    
    if (!current_line.empty()) {
        lines.push_back(current_line);
    }
    
    return lines;
}

// MessageDialog implementation
MessageDialog::MessageDialog(SDL_Renderer* renderer, TextRenderer* text_renderer)
    : Dialog(renderer, text_renderer) {
}

MessageDialog::~MessageDialog() {
}

void MessageDialog::set_message(const std::string& title, const std::string& message) {
    title_ = title;
    message_ = message;
}

bool MessageDialog::process_input(SDL_Keycode key) {
    if (!visible_) return false;
    
    if (key == SDLK_RETURN || key == SDLK_ESCAPE) {
        hide();
        return true;
    }
    
    return false;
}

void MessageDialog::render() {
    if (!visible_) return;
    
    // Render overlay
    render_overlay();
    
    // Get screen dimensions (logical size for 320x240 coordinate system)
    int screen_width, screen_height;
    SDL_RenderGetLogicalSize(renderer_, &screen_width, &screen_height);
    if (screen_width == 0 || screen_height == 0) {
        SDL_GetRendererOutputSize(renderer_, &screen_width, &screen_height);
    }
    
    // Dialog dimensions - scaled for 320x240 resolution
    int dialog_width = (screen_width * 3) / 4;  // 75% of screen
    int dialog_height = screen_height / 2;      // 50% of screen
    int dialog_x = (screen_width - dialog_width) / 2;
    int dialog_y = (screen_height - dialog_height) / 2;
    
    // Render dialog box
    SDL_Color bg_color = { 40, 40, 40, 255 };
    SDL_Color border_color = { 200, 200, 200, 255 };
    render_box(dialog_x, dialog_y, dialog_width, dialog_height, bg_color, border_color);
    
    // Render title
    SDL_Color title_color = { 255, 255, 255, 255 };
    text_renderer_->render_text(title_, dialog_x + 10, dialog_y + 8, 
                                title_color, TextRenderer::FontSize::Large);
    
    // Render message (word-wrapped)
    SDL_Color text_color = { 200, 200, 200, 255 };
    std::vector<std::string> lines = word_wrap(message_, dialog_width - 20);
    
    int line_height = 12;
    int message_y = dialog_y + 28;
    
    for (const auto& line : lines) {
        text_renderer_->render_text(line, dialog_x + 10, message_y,
                                    text_color, TextRenderer::FontSize::Medium);
        message_y += line_height;
    }
    
    // Render "Press Enter to continue" hint
    SDL_Color hint_color = { 150, 150, 150, 255 };
    text_renderer_->render_text("Press Enter to continue", 
                                dialog_x + 10, dialog_y + dialog_height - 18,
                                hint_color, TextRenderer::FontSize::Small);
}

// ConfirmDialog implementation
ConfirmDialog::ConfirmDialog(SDL_Renderer* renderer, TextRenderer* text_renderer)
    : Dialog(renderer, text_renderer)
    , selected_option_(0)
    , result_(false)
    , waiting_for_input_(false) {
}

ConfirmDialog::~ConfirmDialog() {
}

void ConfirmDialog::set_message(const std::string& title, const std::string& message) {
    title_ = title;
    message_ = message;
}

bool ConfirmDialog::show_and_wait() {
    show();
    waiting_for_input_ = true;
    selected_option_ = 0;
    return result_;
}

bool ConfirmDialog::process_input(SDL_Keycode key) {
    if (!visible_) return false;
    
    if (key == SDLK_LEFT || key == SDLK_RIGHT) {
        selected_option_ = 1 - selected_option_;  // Toggle between 0 and 1
        return true;
    }
    
    if (key == SDLK_RETURN) {
        result_ = (selected_option_ == 0);  // 0 = Yes
        waiting_for_input_ = false;
        hide();
        return true;
    }
    
    if (key == SDLK_ESCAPE) {
        result_ = false;
        waiting_for_input_ = false;
        hide();
        return true;
    }
    
    return false;
}

void ConfirmDialog::render() {
    if (!visible_) return;
    
    // Render overlay
    render_overlay();
    
    // Get screen dimensions (logical size for 320x240 coordinate system)
    int screen_width, screen_height;
    SDL_RenderGetLogicalSize(renderer_, &screen_width, &screen_height);
    if (screen_width == 0 || screen_height == 0) {
        SDL_GetRendererOutputSize(renderer_, &screen_width, &screen_height);
    }
    
    // Dialog dimensions - scaled for 320x240 resolution
    int dialog_width = (screen_width * 3) / 4;  // 75% of screen
    int dialog_height = screen_height / 2;      // 50% of screen
    int dialog_x = (screen_width - dialog_width) / 2;
    int dialog_y = (screen_height - dialog_height) / 2;
    
    // Render dialog box
    SDL_Color bg_color = { 40, 40, 40, 255 };
    SDL_Color border_color = { 200, 200, 200, 255 };
    render_box(dialog_x, dialog_y, dialog_width, dialog_height, bg_color, border_color);
    
    // Render title
    SDL_Color title_color = { 255, 255, 255, 255 };
    text_renderer_->render_text(title_, dialog_x + 10, dialog_y + 8,
                                title_color, TextRenderer::FontSize::Large);
    
    // Render message
    SDL_Color text_color = { 200, 200, 200, 255 };
    std::vector<std::string> lines = word_wrap(message_, dialog_width - 20);
    
    int line_height = 12;
    int message_y = dialog_y + 28;
    
    for (const auto& line : lines) {
        text_renderer_->render_text(line, dialog_x + 10, message_y,
                                    text_color, TextRenderer::FontSize::Medium);
        message_y += line_height;
    }
    
    // Render Yes/No buttons
    int button_y = dialog_y + dialog_height - 35;
    int button_spacing = 50;
    int yes_x = dialog_x + dialog_width / 2 - button_spacing;
    int no_x = dialog_x + dialog_width / 2 + 10;
    
    SDL_Color selected_color = { 255, 255, 100, 255 };
    SDL_Color unselected_color = { 200, 200, 200, 255 };
    
    text_renderer_->render_text("Yes", yes_x, button_y,
                                selected_option_ == 0 ? selected_color : unselected_color,
                                TextRenderer::FontSize::Medium);
    
    text_renderer_->render_text("No", no_x, button_y,
                                selected_option_ == 1 ? selected_color : unselected_color,
                                TextRenderer::FontSize::Medium);
    
    // Render hint
    SDL_Color hint_color = { 150, 150, 150, 255 };
    text_renderer_->render_text("Arrows: Select | Enter: Confirm",
                                dialog_x + 10, dialog_y + dialog_height - 18,
                                hint_color, TextRenderer::FontSize::Small);
}

// ProgressDialog implementation
ProgressDialog::ProgressDialog(SDL_Renderer* renderer, TextRenderer* text_renderer)
    : Dialog(renderer, text_renderer) {
}

ProgressDialog::~ProgressDialog() {
}

void ProgressDialog::set_message(const std::string& message) {
    message_ = message;
}

bool ProgressDialog::process_input(SDL_Keycode key) {
    // Progress dialog doesn't handle input
    (void)key;
    return false;
}

void ProgressDialog::render() {
    if (!visible_) return;
    
    // Render overlay
    render_overlay();
    
    // Get screen dimensions (logical size for 320x240 coordinate system)
    int screen_width, screen_height;
    SDL_RenderGetLogicalSize(renderer_, &screen_width, &screen_height);
    if (screen_width == 0 || screen_height == 0) {
        SDL_GetRendererOutputSize(renderer_, &screen_width, &screen_height);
    }
    
    // Dialog dimensions - scaled for 320x240 resolution
    int dialog_width = screen_width / 2;   // 50% of screen
    int dialog_height = screen_height / 4; // 25% of screen
    int dialog_x = (screen_width - dialog_width) / 2;
    int dialog_y = (screen_height - dialog_height) / 2;
    
    // Render dialog box
    SDL_Color bg_color = { 40, 40, 40, 255 };
    SDL_Color border_color = { 200, 200, 200, 255 };
    render_box(dialog_x, dialog_y, dialog_width, dialog_height, bg_color, border_color);
    
    // Render message
    SDL_Color text_color = { 200, 200, 200, 255 };
    text_renderer_->render_text(message_, dialog_x + 10, dialog_y + dialog_height / 2 - 5,
                                text_color, TextRenderer::FontSize::Medium);
}
