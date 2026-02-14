#ifndef DIALOGS_H
#define DIALOGS_H

#include <string>
#include <vector>
#include <SDL.h>

// Forward declaration
class TextRenderer;

// Base dialog class
class Dialog {
public:
    Dialog(SDL_Renderer* renderer, TextRenderer* text_renderer);
    virtual ~Dialog();

    // Show the dialog (makes it visible)
    virtual void show();

    // Hide the dialog
    virtual void hide();

    // Check if dialog is visible
    bool is_visible() const { return visible_; }

    // Process input (returns true if input was handled)
    virtual bool process_input(SDL_Keycode key) = 0;

    // Render the dialog
    virtual void render() = 0;

protected:
    // Render semi-transparent background overlay
    void render_overlay();

    // Render a box with border
    void render_box(int x, int y, int width, int height,
                    SDL_Color bg_color, SDL_Color border_color);

    // Word wrap text to fit within specified width
    std::vector<std::string> word_wrap(const std::string& text, int max_width);

    SDL_Renderer* renderer_;
    TextRenderer* text_renderer_;
    bool visible_;
};

// Message dialog - displays a message with OK button
class MessageDialog : public Dialog {
public:
    MessageDialog(SDL_Renderer* renderer, TextRenderer* text_renderer);
    ~MessageDialog() override;

    // Set the dialog title and message
    void set_message(const std::string& title, const std::string& message);

    // Process input (Enter or Escape dismisses)
    bool process_input(SDL_Keycode key) override;

    // Render the dialog
    void render() override;

private:
    std::string title_;
    std::string message_;
};

// Confirm dialog - displays a message with Yes/No options
class ConfirmDialog : public Dialog {
public:
    ConfirmDialog(SDL_Renderer* renderer, TextRenderer* text_renderer);
    ~ConfirmDialog() override;

    // Set the dialog title and message
    void set_message(const std::string& title, const std::string& message);

    // Show dialog and wait for response
    // Returns true for Yes, false for No
    bool show_and_wait();

    // Get the user's choice (true = Yes, false = No)
    bool get_result() const { return result_; }

    // Process input (arrow keys navigate, Enter selects, Escape cancels)
    bool process_input(SDL_Keycode key) override;

    // Render the dialog
    void render() override;

private:
    std::string title_;
    std::string message_;
    int selected_option_;  // 0 = Yes, 1 = No
    bool result_;
    bool waiting_for_input_;
};

// Progress dialog - displays a progress message
class ProgressDialog : public Dialog {
public:
    ProgressDialog(SDL_Renderer* renderer, TextRenderer* text_renderer);
    ~ProgressDialog() override;

    // Set the progress message
    void set_message(const std::string& message);

    // Process input (no input handling for progress dialog)
    bool process_input(SDL_Keycode key) override;

    // Render the dialog
    void render() override;

private:
    std::string message_;
};

#endif // DIALOGS_H
