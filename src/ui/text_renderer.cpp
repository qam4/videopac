#include "ui/text_renderer.h"
#include <algorithm>

TextRenderer::TextRenderer(SDL_Renderer* renderer)
    : renderer_(renderer)
#ifdef HAVE_SDL_TTF
    , font_small_(nullptr)
    , font_medium_(nullptr)
    , font_large_(nullptr)
    , ttf_initialized_(false)
#endif
{
}

TextRenderer::~TextRenderer() {
#ifdef HAVE_SDL_TTF
    if (font_small_) TTF_CloseFont(font_small_);
    if (font_medium_) TTF_CloseFont(font_medium_);
    if (font_large_) TTF_CloseFont(font_large_);
    
    if (ttf_initialized_) {
        TTF_Quit();
    }
#endif
}

bool TextRenderer::initialize() {
#ifdef HAVE_SDL_TTF
    if (TTF_Init() == -1) {
        return false;
    }
    ttf_initialized_ = true;

    // Try to load Windows system fonts
    // Common locations for fonts on Windows
    const char* font_paths[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/cour.ttf",
        nullptr
    };
    
    // Try each font path until one succeeds
    // Font sizes for crisp rendering at 2x scale (640x480 window)
    // These will be scaled down by logical size for sharp text
    for (int i = 0; font_paths[i] != nullptr; i++) {
        font_small_ = TTF_OpenFont(font_paths[i], 16);  // 2x of 8
        font_medium_ = TTF_OpenFont(font_paths[i], 20); // 2x of 10
        font_large_ = TTF_OpenFont(font_paths[i], 28);  // 2x of 14
        
        if (font_small_ && font_medium_ && font_large_) {
            // Enable hinting for better rendering at small sizes
            TTF_SetFontHinting(font_small_, TTF_HINTING_NORMAL);
            TTF_SetFontHinting(font_medium_, TTF_HINTING_NORMAL);
            TTF_SetFontHinting(font_large_, TTF_HINTING_NORMAL);
            
            // Successfully loaded all font sizes
            return true;
        }
        
        // Clean up partial loads
        if (font_small_) { TTF_CloseFont(font_small_); font_small_ = nullptr; }
        if (font_medium_) { TTF_CloseFont(font_medium_); font_medium_ = nullptr; }
        if (font_large_) { TTF_CloseFont(font_large_); font_large_ = nullptr; }
    }
    
    // If no fonts loaded, fall back to basic rendering
    return true;
#else
    // No SDL_ttf support, use fallback rendering
    return true;
#endif
}

bool TextRenderer::has_ttf_support() const {
#ifdef HAVE_SDL_TTF
    return ttf_initialized_ && (font_small_ || font_medium_ || font_large_);
#else
    return false;
#endif
}

int TextRenderer::get_font_size_pixels(FontSize size) const {
    switch (size) {
        case FontSize::Small:  return 12;
        case FontSize::Medium: return 16;
        case FontSize::Large:  return 24;
        default: return 16;
    }
}

bool TextRenderer::render_text(const std::string& text, int x, int y,
                                SDL_Color color, FontSize size) {
#ifdef HAVE_SDL_TTF
    if (has_ttf_support()) {
        return render_text_ttf(text, x, y, color, size);
    }
#endif
    
    return render_text_fallback(text, x, y, color, size);
}

#ifdef HAVE_SDL_TTF
bool TextRenderer::render_text_ttf(const std::string& text, int x, int y,
                                    SDL_Color color, FontSize size) {
    TTF_Font* font = nullptr;
    
    switch (size) {
        case FontSize::Small:  font = font_small_; break;
        case FontSize::Medium: font = font_medium_; break;
        case FontSize::Large:  font = font_large_; break;
    }
    
    if (!font) {
        return render_text_fallback(text, x, y, color, size);
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) {
        return false;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return false;
    }

    // Scale down by 2x to match logical resolution
    // Fonts are rendered at 2x size for crispness, then scaled to logical size
    SDL_Rect dest_rect = { x, y, surface->w / 2, surface->h / 2 };
    SDL_RenderCopy(renderer_, texture, nullptr, &dest_rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);

    return true;
}
#endif

bool TextRenderer::render_text_fallback(const std::string& text, int x, int y,
                                         SDL_Color color, FontSize size) {
    // Simple bitmap font fallback - draws recognizable characters
    // This creates readable text without requiring font files
    
    int char_width = 8;
    int char_height = 12;
    
    if (size == FontSize::Small) {
        char_width = 6;
        char_height = 8;
    } else if (size == FontSize::Large) {
        char_width = 10;
        char_height = 16;
    }
    
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    
    int current_x = x;
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        
        // Skip spaces
        if (c == ' ') {
            current_x += char_width;
            continue;
        }
        
        // Draw a simple representation of each character
        // For now, just draw filled rectangles with slight variations
        // to make different characters distinguishable
        
        if (c >= 'A' && c <= 'Z') {
            // Uppercase: full height rectangle
            SDL_Rect char_rect = { current_x, y, char_width - 2, char_height };
            SDL_RenderFillRect(renderer_, &char_rect);
        } else if (c >= 'a' && c <= 'z') {
            // Lowercase: shorter rectangle
            SDL_Rect char_rect = { current_x, y + char_height / 4, char_width - 2, char_height * 3 / 4 };
            SDL_RenderFillRect(renderer_, &char_rect);
        } else if (c >= '0' && c <= '9') {
            // Numbers: medium height
            SDL_Rect char_rect = { current_x, y + char_height / 6, char_width - 2, char_height * 2 / 3 };
            SDL_RenderFillRect(renderer_, &char_rect);
        } else if (c == '>') {
            // Arrow: draw a triangle-ish shape
            SDL_Rect r1 = { current_x, y + char_height / 3, char_width / 2, char_height / 3 };
            SDL_Rect r2 = { current_x + char_width / 2, y + char_height / 4, char_width / 2 - 2, char_height / 2 };
            SDL_RenderFillRect(renderer_, &r1);
            SDL_RenderFillRect(renderer_, &r2);
        } else {
            // Other characters: small rectangle
            SDL_Rect char_rect = { current_x, y + char_height / 3, char_width - 2, char_height / 3 };
            SDL_RenderFillRect(renderer_, &char_rect);
        }
        
        current_x += char_width;
    }
    
    return true;
}

bool TextRenderer::measure_text(const std::string& text, FontSize size,
                                 int* out_width, int* out_height) {
#ifdef HAVE_SDL_TTF
    if (has_ttf_support()) {
        TTF_Font* font = nullptr;
        
        switch (size) {
            case FontSize::Small:  font = font_small_; break;
            case FontSize::Medium: font = font_medium_; break;
            case FontSize::Large:  font = font_large_; break;
        }
        
        if (font) {
            return TTF_SizeText(font, text.c_str(), out_width, out_height) == 0;
        }
    }
#endif
    
    // Fallback measurement
    int char_width = get_font_size_pixels(size) / 2;
    int char_height = get_font_size_pixels(size);
    
    if (out_width) *out_width = static_cast<int>(text.length()) * char_width;
    if (out_height) *out_height = char_height;
    
    return true;
}
