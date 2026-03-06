// Videopac libretro core
// Wraps EmulatorCore for use with RetroArch and other libretro frontends

#include "libretro.h"
#include "emulator.h"
#include "savestate.h"
#include "types.h"
#include "vkeyboard.h"
#include <cstring>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>

// Callbacks
static retro_environment_t environ_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_sample_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_log_printf_t log_cb;

// Emulator instance
static videopac::EmulatorCore* emulator = nullptr;
static videopac::VideoStandard video_standard = videopac::VideoStandard::NTSC;

// Video buffer (XRGB8888)
static uint32_t video_buffer[videopac::FRAMEBUFFER_WIDTH * videopac::FRAMEBUFFER_HEIGHT];

// Audio buffer (stereo interleaved)
static constexpr size_t AUDIO_SAMPLES_PER_FRAME_NTSC = 800;  // 48000/60
static constexpr size_t AUDIO_SAMPLES_PER_FRAME_PAL = 960;   // 48000/50
static int16_t audio_mono_buffer[1024];
static int16_t audio_stereo_buffer[2048];

// BIOS data
static std::vector<uint8_t> bios_data;
static bool bios_loaded = false;
static std::string system_directory;

// Virtual keyboard state
static videopac::VirtualKeyboard vkb;
static uint8_t vkb_transparency_pct = 25;
static bool prev_select_pressed = false;
static bool prev_y_pressed = false;

// Joystick swap
static bool swap_joysticks = false;

// Core options
static struct retro_variable core_options[] = {
    { "videopac_region", "Region; NTSC|PAL" },
    { "videopac_palette", "Palette; Standard|Videopac+" },
    { "videopac_vkbd_transparency", "Virtual Keyboard Transparency; 25%|0%|50%|75%" },
    { "videopac_swap_joysticks", "Swap Joysticks; disabled|enabled" },
    { nullptr, nullptr }
};

// --- Helper functions ---

static bool load_bios_file() {
    if (bios_loaded) return true;

    // Try to find BIOS in system directory
    const char* sys_dir = nullptr;
    if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &sys_dir) && sys_dir) {
        system_directory = sys_dir;
        if (log_cb) log_cb(RETRO_LOG_INFO, "[videopac] System directory: %s\n", sys_dir);
    } else {
        system_directory = ".";
        if (log_cb) log_cb(RETRO_LOG_WARN, "[videopac] No system directory, using CWD\n");
    }

    // Try common BIOS filenames
    const char* bios_names[] = {
        "o2rom.bin", "bios_O2rom.bin", "odyssey2.bin",
        "c52.bin", "bios_c52.bin",
        "g7400.bin", "bios_g7400.bin",
        "jopac.bin", "bios_jopac.bin",
        nullptr
    };

    for (int i = 0; bios_names[i]; i++) {
        std::string path = system_directory + "/" + bios_names[i];
        FILE* f = fopen(path.c_str(), "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            fseek(f, 0, SEEK_SET);
            if (size > 0 && size <= 2048) {
                bios_data.resize(size);
                if (fread(bios_data.data(), 1, size, f) == static_cast<size_t>(size)) {
                    fclose(f);
                    bios_loaded = true;
                    if (log_cb) log_cb(RETRO_LOG_INFO, "[videopac] BIOS loaded: %s (%ld bytes)\n", path.c_str(), size);
                    return true;
                }
            }
            fclose(f);
        }
    }

    if (log_cb) log_cb(RETRO_LOG_ERROR, "[videopac] BIOS not found in %s\n", system_directory.c_str());
    return false;
}

static void check_variables() {
    struct retro_variable var;

    var.key = "videopac_region";
    var.value = nullptr;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
        if (strcmp(var.value, "PAL") == 0)
            video_standard = videopac::VideoStandard::PAL;
        else
            video_standard = videopac::VideoStandard::NTSC;
    }

    var.key = "videopac_swap_joysticks";
    var.value = nullptr;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
        swap_joysticks = (strcmp(var.value, "enabled") == 0);
    }

    var.key = "videopac_vkbd_transparency";
    var.value = nullptr;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value) {
        vkb_transparency_pct = static_cast<uint8_t>(atoi(var.value));
    }
}

static void convert_framebuffer() {
    const videopac::uint8* fb = emulator->get_framebuffer();
    if (!fb) return;

    // Convert palette-indexed framebuffer to XRGB8888
    for (int i = 0; i < videopac::FRAMEBUFFER_WIDTH * videopac::FRAMEBUFFER_HEIGHT; i++) {
        uint8_t idx = fb[i] & 0x0F;
        const videopac::Color& c = videopac::PALETTE_STANDARD[idx];
        video_buffer[i] = (0xFF << 24) | (c.r << 16) | (c.g << 8) | c.b;
    }
}

static void update_input() {
    input_poll_cb();

    videopac::InputState state = {};

    // Edge detection state for D-pad (VKB navigation)
    static bool prev_up_pressed = false;
    static bool prev_down_pressed = false;
    static bool prev_left_pressed = false;
    static bool prev_right_pressed = false;
    static bool prev_reset_active = false;  // Edge detection for RST key

    // Read SELECT for VKB toggle (rising edge)
    bool select_pressed = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT);
    if (select_pressed && !prev_select_pressed) {
        vkb.toggle_visible();
    }
    prev_select_pressed = select_pressed;

    // Read D-pad and button states from port 0
    bool up    = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
    bool down  = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
    bool left  = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
    bool right = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);
    bool fire  = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A);
    bool b_btn = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B);
    bool y_btn = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);

    // Read port 1 joystick raw data
    bool p1_up    = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
    bool p1_down  = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
    bool p1_left  = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
    bool p1_right = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);
    bool p1_fire  = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A);

    if (vkb.is_visible()) {
        // D-pad navigates VKB cursor (rising edge)
        if (up && !prev_up_pressed)       vkb.move_cursor(videopac::Direction::Up);
        if (down && !prev_down_pressed)    vkb.move_cursor(videopac::Direction::Down);
        if (left && !prev_left_pressed)    vkb.move_cursor(videopac::Direction::Left);
        if (right && !prev_right_pressed)  vkb.move_cursor(videopac::Direction::Right);

        // B button activates/deactivates the current VKB key
        bool reset_active = false;  // Track RST across both D-pad and touch paths
        videopac::VidKey vk = vkb.get_current_vidkey();
        if (vk == videopac::VidKey::Reset) {
            // RST key via D-pad+B
            if (b_btn) reset_active = true;
        } else {
            uint8_t row = (static_cast<uint8_t>(vk) >> 4) & 0x07;
            uint8_t col = static_cast<uint8_t>(vk) & 0x07;
            if (b_btn) {
                state.keyboard_matrix[row][col] = true;
            }
        }
        // When released, matrix entry stays false (default)

        // Y toggles VKB position (rising edge)
        if (y_btn && !prev_y_pressed) {
            vkb.toggle_position();
        }

        // Touch input via RETRO_DEVICE_POINTER
        int16_t ptr_pressed = input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_PRESSED);
        if (ptr_pressed) {
            int16_t ptr_x = input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
            int16_t ptr_y = input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);

            // Translate from [-0x7FFF, 0x7FFF] to pixel coordinates
            int pixel_x = (ptr_x + 0x7FFF) * videopac::FRAMEBUFFER_WIDTH / (2 * 0x7FFF);
            int pixel_y = (ptr_y + 0x7FFF) * videopac::FRAMEBUFFER_HEIGHT / (2 * 0x7FFF);

            // Clamp to valid framebuffer range
            if (pixel_x < 0) pixel_x = 0;
            if (pixel_x >= videopac::FRAMEBUFFER_WIDTH) pixel_x = videopac::FRAMEBUFFER_WIDTH - 1;
            if (pixel_y < 0) pixel_y = 0;
            if (pixel_y >= videopac::FRAMEBUFFER_HEIGHT) pixel_y = videopac::FRAMEBUFFER_HEIGHT - 1;

            // Try hit test at both possible VKB positions (top and bottom)
            // since position_bottom_ is private. Only one position will yield a valid hit.
            int bottom_offset = videopac::FRAMEBUFFER_HEIGHT - videopac::VirtualKeyboard::VKB_HEIGHT;
            int hit = vkb.hit_test(pixel_x, pixel_y - bottom_offset);
            if (hit < 0) {
                hit = vkb.hit_test(pixel_x, pixel_y);
            }

            if (hit >= 0) {
                vkb.set_cursor(hit);  // Highlight touched key
                videopac::VidKey touch_vk = vkb.get_vidkey_at(hit);
                if (touch_vk == videopac::VidKey::Reset) {
                    reset_active = true;
                } else {
                    uint8_t touch_row = (static_cast<uint8_t>(touch_vk) >> 4) & 0x07;
                    uint8_t touch_col = static_cast<uint8_t>(touch_vk) & 0x07;
                    state.keyboard_matrix[touch_row][touch_col] = true;
                }
            }
        }

        // Fire reset on rising edge only (prevents repeated resets while held)
        if (reset_active && !prev_reset_active) {
            if (emulator) emulator->reset();
        }
        prev_reset_active = reset_active;

        // Suppress D-pad from joystick1, B from keyboard_matrix[0][0], Y from keyboard_matrix[0][1]
        // (D-pad not written to joystick1, B/Y not written to their normal keyboard mappings)

        // Fire (A) still passes through to joystick (swap-aware)
        // Port 1 data routes to the other joystick (swap-aware)
        if (swap_joysticks) {
            state.joystick2[4] = fire;
            state.joystick1[0] = p1_up;
            state.joystick1[1] = p1_down;
            state.joystick1[2] = p1_left;
            state.joystick1[3] = p1_right;
            state.joystick1[4] = p1_fire;
        } else {
            state.joystick1[4] = fire;
            state.joystick2[0] = p1_up;
            state.joystick2[1] = p1_down;
            state.joystick2[2] = p1_left;
            state.joystick2[3] = p1_right;
            state.joystick2[4] = p1_fire;
        }

        // Other keyboard mappings still pass through
        if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X))
            state.keyboard_matrix[0][2] = true;  // Key 2
        if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L))
            state.keyboard_matrix[0][3] = true;  // Key 3
        if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START))
            state.keyboard_matrix[5][7] = true;  // Enter
    } else {
        // VKB hidden: pass all inputs through (swap-aware)
        if (swap_joysticks) {
            // Port 0 → joystick2, Port 1 → joystick1
            state.joystick2[0] = up;
            state.joystick2[1] = down;
            state.joystick2[2] = left;
            state.joystick2[3] = right;
            state.joystick2[4] = fire;

            state.joystick1[0] = p1_up;
            state.joystick1[1] = p1_down;
            state.joystick1[2] = p1_left;
            state.joystick1[3] = p1_right;
            state.joystick1[4] = p1_fire;
        } else {
            // Port 0 → joystick1, Port 1 → joystick2 (default)
            state.joystick1[0] = up;
            state.joystick1[1] = down;
            state.joystick1[2] = left;
            state.joystick1[3] = right;
            state.joystick1[4] = fire;

            state.joystick2[0] = p1_up;
            state.joystick2[1] = p1_down;
            state.joystick2[2] = p1_left;
            state.joystick2[3] = p1_right;
            state.joystick2[4] = p1_fire;
        }

        if (b_btn)
            state.keyboard_matrix[0][0] = true;  // Key 0
        if (y_btn)
            state.keyboard_matrix[0][1] = true;  // Key 1
        if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X))
            state.keyboard_matrix[0][2] = true;  // Key 2
        if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L))
            state.keyboard_matrix[0][3] = true;  // Key 3
        if (select_pressed)
            state.keyboard_matrix[1][4] = true;  // Space
        if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START))
            state.keyboard_matrix[5][7] = true;  // Enter
    }

    // Update prev states
    prev_up_pressed = up;
    prev_down_pressed = down;
    prev_left_pressed = left;
    prev_right_pressed = right;
    prev_y_pressed = y_btn;

    // Physical keyboard passthrough (PC keyboard → Videopac matrix)
    // Works in both VKB-visible and VKB-hidden modes for desktop testing
    struct KeyMapping { unsigned retrok; uint8_t row; uint8_t col; };
    static const KeyMapping kb_map[] = {
        // Row 0: digits 0-7
        {RETROK_0, 0, 0}, {RETROK_1, 0, 1}, {RETROK_2, 0, 2}, {RETROK_3, 0, 3},
        {RETROK_4, 0, 4}, {RETROK_5, 0, 5}, {RETROK_6, 0, 6}, {RETROK_7, 0, 7},
        // Row 1: 8, 9, space, /, L, P
        {RETROK_8, 1, 0}, {RETROK_9, 1, 1},
        {RETROK_SPACE, 1, 4}, {RETROK_SLASH, 1, 5}, {RETROK_l, 1, 6}, {RETROK_p, 1, 7},
        // Row 2: +, W, E, R, T, U, I, O
        {RETROK_PLUS, 2, 0}, {RETROK_w, 2, 1}, {RETROK_e, 2, 2}, {RETROK_r, 2, 3},
        {RETROK_t, 2, 4}, {RETROK_u, 2, 5}, {RETROK_i, 2, 6}, {RETROK_o, 2, 7},
        // Row 3: Q, S, D, F, G, H, J, K
        {RETROK_q, 3, 0}, {RETROK_s, 3, 1}, {RETROK_d, 3, 2}, {RETROK_f, 3, 3},
        {RETROK_g, 3, 4}, {RETROK_h, 3, 5}, {RETROK_j, 3, 6}, {RETROK_k, 3, 7},
        // Row 4: A, Z, X, C, V, B, M, .
        {RETROK_a, 4, 0}, {RETROK_z, 4, 1}, {RETROK_x, 4, 2}, {RETROK_c, 4, 3},
        {RETROK_v, 4, 4}, {RETROK_b, 4, 5}, {RETROK_m, 4, 6}, {RETROK_PERIOD, 4, 7},
        // Row 5: -, *, /, =, Y, N, CLR, ENTER
        {RETROK_MINUS, 5, 0}, {RETROK_ASTERISK, 5, 1}, {RETROK_EQUALS, 5, 3},
        {RETROK_y, 5, 4}, {RETROK_n, 5, 5},
        {RETROK_BACKSPACE, 5, 6}, {RETROK_DELETE, 5, 6},  // Both map to CLR
        {RETROK_RETURN, 5, 7},
    };
    for (const auto& km : kb_map) {
        if (input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, km.retrok)) {
            state.keyboard_matrix[km.row][km.col] = true;
        }
    }

    emulator->set_input(state);
}


// --- libretro API implementation ---

RETRO_API void retro_init(void) {
    // Nothing to do here; emulator created in retro_load_game
}

RETRO_API void retro_deinit(void) {
    delete emulator;
    emulator = nullptr;
    bios_data.clear();
    bios_loaded = false;
}

RETRO_API unsigned retro_api_version(void) {
    return RETRO_API_VERSION;
}

RETRO_API void retro_get_system_info(struct retro_system_info* info) {
    memset(info, 0, sizeof(*info));
    info->library_name = "videopac";
    info->library_version = "0.3.0";
    info->valid_extensions = "bin|rom|zip";
    info->need_fullpath = false;
    info->block_extract = false;
}

RETRO_API void retro_get_system_av_info(struct retro_system_av_info* info) {
    memset(info, 0, sizeof(*info));
    info->geometry.base_width = videopac::FRAMEBUFFER_WIDTH;
    info->geometry.base_height = videopac::FRAMEBUFFER_HEIGHT;
    info->geometry.max_width = videopac::FRAMEBUFFER_WIDTH;
    info->geometry.max_height = videopac::FRAMEBUFFER_HEIGHT;
    info->geometry.aspect_ratio = 4.0f / 3.0f;
    info->timing.fps = (video_standard == videopac::VideoStandard::PAL) ? 50.0 : 60.0;
    info->timing.sample_rate = 48000.0;
}

RETRO_API void retro_set_environment(retro_environment_t cb) {
    environ_cb = cb;

    // Get log interface
    struct retro_log_callback logging;
    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;

    // Set core options
    cb(RETRO_ENVIRONMENT_SET_VARIABLES, core_options);

    // Set pixel format
    unsigned format = RETRO_PIXEL_FORMAT_XRGB8888;
    cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &format);

    // Set input descriptors
    static struct retro_input_descriptor desc[] = {
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "Fire" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "VKB Press / Key 0" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,      "VKB Position / Key 1" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,      "Key 2" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,      "Key 3" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Toggle Virtual Keyboard" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Enter" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "P2 Up" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "P2 Down" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "P2 Left" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "P2 Right" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "P2 Fire" },
        { 0, 0, 0, 0, nullptr }
    };
    cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
}

RETRO_API void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
RETRO_API void retro_set_audio_sample(retro_audio_sample_t cb) { audio_sample_cb = cb; }
RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
RETRO_API void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
RETRO_API void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }
RETRO_API void retro_set_controller_port_device(unsigned, unsigned) {}

RETRO_API void retro_reset(void) {
    if (emulator) emulator->reset();
}

RETRO_API void retro_run(void) {
    // Check for option changes
    bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated) {
        check_variables();
    }

    // Poll and apply input
    update_input();

    // Run one frame
    emulator->run_frame();

    // Video: convert palette framebuffer to XRGB8888
    convert_framebuffer();

    // Render VKB overlay if visible
    if (vkb.is_visible()) {
        vkb.render(video_buffer, videopac::FRAMEBUFFER_WIDTH, videopac::FRAMEBUFFER_HEIGHT, vkb_transparency_pct);
    }

    video_cb(video_buffer, videopac::FRAMEBUFFER_WIDTH, videopac::FRAMEBUFFER_HEIGHT,
             videopac::FRAMEBUFFER_WIDTH * sizeof(uint32_t));

    // Audio: get mono samples, convert to stereo, send batch
    size_t samples = (video_standard == videopac::VideoStandard::PAL)
                     ? AUDIO_SAMPLES_PER_FRAME_PAL
                     : AUDIO_SAMPLES_PER_FRAME_NTSC;
    emulator->get_audio_buffer(audio_mono_buffer, samples);

    for (size_t i = 0; i < samples; i++) {
        audio_stereo_buffer[i * 2]     = audio_mono_buffer[i];
        audio_stereo_buffer[i * 2 + 1] = audio_mono_buffer[i];
    }
    audio_batch_cb(audio_stereo_buffer, samples);
}

RETRO_API bool retro_load_game(const struct retro_game_info* game) {
    if (!game || !game->data || game->size == 0) {
        if (log_cb) log_cb(RETRO_LOG_ERROR, "[videopac] No game data provided\n");
        return false;
    }

    if (log_cb) log_cb(RETRO_LOG_INFO, "[videopac] Loading ROM: %s (%zu bytes)\n",
                        game->path ? game->path : "(buffer)", game->size);

    check_variables();

    // Create emulator
    videopac::Configuration config;
    config.video_standard = video_standard;
    emulator = new videopac::EmulatorCore(config);

    // Load BIOS
    if (!load_bios_file()) {
        if (log_cb) log_cb(RETRO_LOG_ERROR, "[videopac] Failed to load BIOS — cannot start\n");
        delete emulator;
        emulator = nullptr;
        return false;
    }
    auto bios_result = emulator->load_bios(bios_data.data(), bios_data.size());
    if (!bios_result.is_ok()) {
        if (log_cb) log_cb(RETRO_LOG_ERROR, "[videopac] BIOS rejected: %s\n", bios_result.error.c_str());
        delete emulator;
        emulator = nullptr;
        return false;
    }

    // Load ROM
    auto rom_result = emulator->load_rom(
        static_cast<const videopac::uint8*>(game->data), game->size);
    if (!rom_result.is_ok()) {
        if (log_cb) log_cb(RETRO_LOG_ERROR, "[videopac] ROM rejected: %s\n", rom_result.error.c_str());
        delete emulator;
        emulator = nullptr;
        return false;
    }

    if (log_cb) log_cb(RETRO_LOG_INFO, "[videopac] Game loaded successfully\n");
    return true;
}

RETRO_API void retro_unload_game(void) {
    delete emulator;
    emulator = nullptr;
}

RETRO_API unsigned retro_get_region(void) {
    return (video_standard == videopac::VideoStandard::PAL)
           ? RETRO_REGION_PAL : RETRO_REGION_NTSC;
}

// --- Save states ---
// SaveState contains non-trivial types (std::vector in MemoryState),
// so we serialize component states individually into a flat buffer.

struct LibretroSaveBuffer {
    videopac::CPUState cpu;
    videopac::VDCState vdc;
    videopac::InputState input;
    videopac::uint64 frame_count;
    // Memory: fixed-size portions only (no cart_rom vector)
    videopac::uint8 bios_rom[1024];
    videopac::uint8 external_ram[128];
    videopac::uint8 current_bank;
    videopac::uint8 rom_size_kb;
};

RETRO_API size_t retro_serialize_size(void) {
    return sizeof(LibretroSaveBuffer);
}

RETRO_API bool retro_serialize(void* data, size_t size) {
    if (!emulator || size < sizeof(LibretroSaveBuffer))
        return false;

    LibretroSaveBuffer buf = {};
    buf.cpu = emulator->get_cpu().get_state();
    buf.vdc = emulator->get_vdc().get_state();
    buf.input = emulator->get_input_handler().get_state();
    buf.frame_count = emulator->get_frame_count();

    auto mem = emulator->get_memory().get_state();
    memcpy(buf.bios_rom, mem.bios_rom, sizeof(buf.bios_rom));
    memcpy(buf.external_ram, mem.external_ram, sizeof(buf.external_ram));
    buf.current_bank = mem.current_bank;
    buf.rom_size_kb = mem.rom_size_kb;

    memcpy(data, &buf, sizeof(buf));
    return true;
}

RETRO_API bool retro_unserialize(const void* data, size_t size) {
    if (!emulator || size < sizeof(LibretroSaveBuffer))
        return false;

    LibretroSaveBuffer buf;
    memcpy(&buf, data, sizeof(buf));

    emulator->get_cpu().set_state(buf.cpu);
    emulator->get_vdc().set_state(buf.vdc);
    emulator->get_input_handler().set_state(buf.input);

    // Restore memory (preserve cart_rom from current load)
    auto mem = emulator->get_memory().get_state();
    memcpy(mem.bios_rom, buf.bios_rom, sizeof(buf.bios_rom));
    memcpy(mem.external_ram, buf.external_ram, sizeof(buf.external_ram));
    mem.current_bank = buf.current_bank;
    mem.rom_size_kb = buf.rom_size_kb;
    emulator->get_memory().set_state(mem);

    return true;
}

// --- Cheats (not supported) ---

RETRO_API void retro_cheat_reset(void) {}
RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char* code) {
    (void)index; (void)enabled; (void)code;
}

// --- Load game special (not supported) ---

RETRO_API bool retro_load_game_special(unsigned game_type, const struct retro_game_info* info, size_t num_info) {
    (void)game_type; (void)info; (void)num_info;
    return false;
}

// --- Memory access (for RetroArch cheat/RAM watch) ---

RETRO_API void* retro_get_memory_data(unsigned id) {
    (void)id;
    // TODO: expose direct RAM pointer from MemorySystem for RetroArch RAM watch
    return nullptr;
}

RETRO_API size_t retro_get_memory_size(unsigned id) {
    (void)id;
    return 0;
}
