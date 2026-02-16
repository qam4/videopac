#ifndef OSD_RENDERER_H
#define OSD_RENDERER_H

#include <string>
#include <cstdint>
#include <SDL.h>
#include "ui/text_renderer.h"

// On-Screen Display renderer for FPS, notifications, and status indicators
class OSDRenderer {
public:
    enum class OSDPosition {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    enum class FontSize {
        Small,
        Medium,
        Large
    };

    OSDRenderer(SDL_Renderer* renderer);
    ~OSDRenderer();

    // Initialize OSD renderer (must be called before use)
    bool initialize();

    // Render FPS display at specified position
    void render_fps(float fps, OSDPosition position);

    // Render notification message at specified position
    void render_notification(const std::string& message, OSDPosition position);

    // Render status indicator (e.g., mute icon) at specified position
    void render_status_indicator(const std::string& icon, OSDPosition position);
    
    // Render unified status bar at bottom with consistent styling
    void render_status_bar(const std::string& text);

    // Set font size for all OSD elements
    void set_font_size(FontSize size);

    // Set opacity for all OSD elements (0-100)
    void set_opacity(int opacity);

    // Show a notification for a specified duration
    void show_notification(const std::string& message, int duration_ms);

    // Update notification timeout (call each frame)
    void update(uint32_t current_time);

    // Get current settings
    FontSize get_font_size() const { return font_size_; }
    int get_opacity() const { return opacity_; }

private:
    // Convert FontSize to TextRenderer::FontSize
    TextRenderer::FontSize to_text_renderer_font_size(FontSize size) const;

    // Calculate position coordinates based on OSDPosition
    void calculate_position(OSDPosition position, int text_width, int text_height,
                           int* out_x, int* out_y) const;

    // Render text with current opacity settings
    void render_text_with_opacity(const std::string& text, int x, int y,
                                  SDL_Color color, FontSize size);

    // Get screen dimensions
    void get_screen_dimensions(int* width, int* height) const;

    SDL_Renderer* renderer_;
    TextRenderer text_renderer_;
    
    FontSize font_size_;
    int opacity_;  // 0-100

    // Notification state
    std::string current_notification_;
    uint32_t notification_start_time_;
    int notification_duration_;
    bool notification_active_;

    // Padding from screen edges
    static constexpr int PADDING = 10;
};

#endif // OSD_RENDERER_H
