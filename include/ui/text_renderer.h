#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <string>
#include <SDL.h>

#ifdef HAVE_SDL_TTF
#include <SDL_ttf.h>
#endif

// Text rendering system with SDL_ttf support and fallback
class TextRenderer {
public:
    enum class FontSize {
        Small,
        Medium,
        Large
    };

    TextRenderer(SDL_Renderer* renderer);
    ~TextRenderer();

    // Initialize text rendering (loads fonts if SDL_ttf available)
    bool initialize();

    // Render text at specified position with color
    // Returns true on success
    bool render_text(const std::string& text, int x, int y, 
                     SDL_Color color, FontSize size = FontSize::Medium);

    // Measure text dimensions (width and height)
    // Returns true on success, fills out_width and out_height
    bool measure_text(const std::string& text, FontSize size,
                      int* out_width, int* out_height);

    // Check if SDL_ttf is available
    bool has_ttf_support() const;

private:
    // Render using SDL_ttf (if available)
    bool render_text_ttf(const std::string& text, int x, int y,
                         SDL_Color color, FontSize size);

    // Fallback rendering using SDL's built-in surface rendering
    bool render_text_fallback(const std::string& text, int x, int y,
                              SDL_Color color, FontSize size);

    // Get font size in pixels
    int get_font_size_pixels(FontSize size) const;

    SDL_Renderer* renderer_;
    
#ifdef HAVE_SDL_TTF
    TTF_Font* font_small_;
    TTF_Font* font_medium_;
    TTF_Font* font_large_;
    bool ttf_initialized_;
#endif
};

#endif // TEXT_RENDERER_H
