#include "ui/osd_renderer.h"
#include <algorithm>

OSDRenderer::OSDRenderer(SDL_Renderer* renderer)
    : renderer_(renderer)
    , text_renderer_(renderer)
    , font_size_(FontSize::Medium)
    , opacity_(75)
    , notification_start_time_(0)
    , notification_duration_(0)
    , notification_active_(false)
{
}

OSDRenderer::~OSDRenderer() {
}

bool OSDRenderer::initialize() {
    return text_renderer_.initialize();
}

TextRenderer::FontSize OSDRenderer::to_text_renderer_font_size(FontSize size) const {
    switch (size) {
        case FontSize::Small:  return TextRenderer::FontSize::Small;
        case FontSize::Medium: return TextRenderer::FontSize::Medium;
        case FontSize::Large:  return TextRenderer::FontSize::Large;
        default: return TextRenderer::FontSize::Medium;
    }
}

void OSDRenderer::get_screen_dimensions(int* width, int* height) const {
    // Always get actual window/renderer output size
    // No logical size is used anymore - UI renders at native resolution
    SDL_GetRendererOutputSize(renderer_, width, height);
}

void OSDRenderer::calculate_position(OSDPosition position, int text_width, int text_height,
                                     int* out_x, int* out_y) const {
    int screen_width, screen_height;
    get_screen_dimensions(&screen_width, &screen_height);

    switch (position) {
        case OSDPosition::TopLeft:
            *out_x = PADDING;
            *out_y = PADDING;
            break;
        
        case OSDPosition::TopRight:
            *out_x = screen_width - text_width - PADDING;
            *out_y = PADDING;
            break;
        
        case OSDPosition::BottomLeft:
            *out_x = PADDING;
            *out_y = screen_height - text_height - PADDING;
            break;
        
        case OSDPosition::BottomRight:
            *out_x = screen_width - text_width - PADDING;
            *out_y = screen_height - text_height - PADDING;
            break;
    }
}

void OSDRenderer::render_text_with_opacity(const std::string& text, int x, int y,
                                           SDL_Color color, FontSize size) {
    // Apply opacity to the color
    SDL_Color adjusted_color = color;
    adjusted_color.a = static_cast<Uint8>((opacity_ * 255) / 100);

    // Render the text
    text_renderer_.render_text(text, x, y, adjusted_color, 
                              to_text_renderer_font_size(size));
}

void OSDRenderer::render_fps(float fps, OSDPosition position) {
    // Format FPS string
    char fps_text[32];
    snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", fps);

    // Measure text dimensions
    int text_width, text_height;
    if (!text_renderer_.measure_text(fps_text, to_text_renderer_font_size(font_size_),
                                     &text_width, &text_height)) {
        return;
    }

    // Calculate position
    int x, y;
    calculate_position(position, text_width, text_height, &x, &y);

    // Render semi-transparent background box for better visibility
    const int padding = 4;
    SDL_Rect bg_rect = {
        x - padding,
        y - padding,
        text_width + (padding * 2),
        text_height + (padding * 2)
    };
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 180);  // Semi-transparent black
    SDL_RenderFillRect(renderer_, &bg_rect);

    // Render with white color
    SDL_Color text_color = {255, 255, 255, 255};
    render_text_with_opacity(fps_text, x, y, text_color, font_size_);
}

void OSDRenderer::render_notification(const std::string& message, OSDPosition position) {
    if (message.empty()) {
        return;
    }

    // Measure text dimensions
    int text_width, text_height;
    if (!text_renderer_.measure_text(message, to_text_renderer_font_size(font_size_),
                                     &text_width, &text_height)) {
        return;
    }

    // Calculate position
    int x, y;
    calculate_position(position, text_width, text_height, &x, &y);

    // Render background box for better visibility
    SDL_Rect bg_rect = {
        x - 5,
        y - 3,
        text_width + 10,
        text_height + 6
    };

    // Semi-transparent background
    SDL_Color bg_color = {0, 0, 0, static_cast<Uint8>((opacity_ * 180) / 100)};
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderFillRect(renderer_, &bg_rect);

    // Render text
    SDL_Color text_color = {255, 255, 255, 255};
    render_text_with_opacity(message, x, y, text_color, font_size_);
}

void OSDRenderer::render_status_indicator(const std::string& icon, OSDPosition position) {
    if (icon.empty()) {
        return;
    }

    // Measure text dimensions
    int text_width, text_height;
    if (!text_renderer_.measure_text(icon, to_text_renderer_font_size(font_size_),
                                     &text_width, &text_height)) {
        return;
    }

    // Calculate position
    int x, y;
    calculate_position(position, text_width, text_height, &x, &y);

    // Render with yellow color for status indicators
    SDL_Color outline_color = {0, 0, 0, 255};
    SDL_Color text_color = {255, 255, 0, 255};

    // Render outline
    render_text_with_opacity(icon, x + 1, y + 1, outline_color, font_size_);
    
    // Render main text
    render_text_with_opacity(icon, x, y, text_color, font_size_);
}

void OSDRenderer::render_status_bar(const std::string& text) {
    if (text.empty()) {
        return;
    }

    // Measure text dimensions
    int text_width, text_height;
    if (!text_renderer_.measure_text(text, to_text_renderer_font_size(font_size_),
                                     &text_width, &text_height)) {
        return;
    }

    // Get screen dimensions
    int screen_width, screen_height;
    get_screen_dimensions(&screen_width, &screen_height);

    // Create a full-width bar at the bottom
    const int bar_height = text_height + 8;  // Padding top and bottom
    const int bar_y = screen_height - bar_height;
    
    // Render semi-transparent background bar
    SDL_Rect bar_rect = {0, bar_y, screen_width, bar_height};
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 200);  // Semi-transparent black
    SDL_RenderFillRect(renderer_, &bar_rect);

    // Center the text in the bar
    int text_x = (screen_width - text_width) / 2;
    int text_y = bar_y + 4;  // 4px padding from top of bar

    // Render text in white
    SDL_Color text_color = {255, 255, 255, 255};
    render_text_with_opacity(text, text_x, text_y, text_color, font_size_);
}

void OSDRenderer::set_font_size(FontSize size) {
    font_size_ = size;
}

void OSDRenderer::set_opacity(int opacity) {
    opacity_ = std::max(0, std::min(100, opacity));
}

void OSDRenderer::show_notification(const std::string& message, int duration_ms) {
    current_notification_ = message;
    notification_start_time_ = SDL_GetTicks();
    notification_duration_ = duration_ms;
    notification_active_ = true;
}

void OSDRenderer::update(uint32_t current_time) {
    if (notification_active_) {
        uint32_t elapsed = current_time - notification_start_time_;
        if (elapsed >= static_cast<uint32_t>(notification_duration_)) {
            notification_active_ = false;
            current_notification_.clear();
        }
    }
}
