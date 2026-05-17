#include "quantum.h"
#include "ergohaven.h"
#include "ergohaven_rgb.h"
#include <eeconfig.h>
#include <string.h>

static_assert(KB_SETTINGS_LED_COLORS_SIZE == sizeof(kb_settings_led_colors_t), "Invalid KB_SETTINGS_LED_COLORS_SIZE");

typedef struct {
    uint8_t hue;
    uint8_t sat;
    uint8_t val;
} eh_rgb_palette_color_t;

static kb_settings_led_colors_t kb_settings_led_colors;

static const eh_rgb_palette_color_t eh_rgb_palette[EH_RGB_PALETTE_SIZE] = {
    {0, 0, 0},        // Off
    {0, 0, 255},      // White
    {0, 255, 255},    // Red
    {16, 255, 255},   // Orange
    {27, 255, 255},   // Goldenrod
    {38, 255, 255},   // Gold
    {53, 255, 255},   // Yellow
    {74, 255, 255},   // Chartreuse
    {90, 255, 255},   // Lime
    {106, 255, 255},  // Green
    {117, 255, 255},  // Spring Green
    {128, 255, 255},  // Turquoise
    {138, 255, 170},  // Teal
    {149, 255, 255},  // Cyan
    {160, 255, 255},  // Azure
    {165, 255, 255},  // Sky
    {170, 255, 255},  // Blue
    {186, 255, 255},  // Indigo
    {202, 255, 255},  // Purple
    {213, 255, 255},  // Magenta
    {234, 180, 255},  // Pink
    {8, 176, 255},    // Coral
    {14, 128, 255},   // Salmon
    {32, 64, 255},    // Warm White
    {22, 255, 255},   // Amber
};

static rgblight_segment_t layer_segments[EH_RGB_LAYER_COUNT][2] = {
    {{0, 2, 0, 0, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 0, 255, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 170, 255, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 30, 255, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 128, 255, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 191, 255, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 170, 255, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 234, 180, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 106, 255, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 64, 255, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 128, 255, 170}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 21, 255, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 149, 255, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 85, 255, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 11, 176, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
    {{0, 2, 43, 255, 255}, {RGBLIGHT_END_SEGMENT_INDEX, 0, 0, 0, 0}},
};

static const rgblight_segment_t *const my_rgb_layers[] = {
    layer_segments[0],  layer_segments[1],  layer_segments[2],  layer_segments[3],
    layer_segments[4],  layer_segments[5],  layer_segments[6],  layer_segments[7],
    layer_segments[8],  layer_segments[9],  layer_segments[10], layer_segments[11],
    layer_segments[12], layer_segments[13], layer_segments[14], layer_segments[15],
};

static kb_settings_led_colors_t get_settings_led_colors_default(void) {
    kb_settings_led_colors_t dflt = {
#if defined(KEYBOARD_ergohaven_phenom_rev1)
        .layer_color  = {1, 2, 16, 24, 6, 8, 10, 11, 15, 19, 20, 21, 22, 23, 3, 17},
#else
        .layer_color  = {1, 2, 16, 4, 9, 18, 16, 20, 10, 6, 12, 3, 14, 7, 21, 5},
#endif
        .brightness   = 128,
        .timeout_mins = 10,
    };
    return dflt;
}

static kb_settings_led_colors_t kb_settings_led_colors_sanitize(kb_settings_led_colors_t config) {
    kb_settings_led_colors_t dflt = get_settings_led_colors_default();
    for (uint8_t i = 0; i < EH_RGB_LAYER_COUNT; ++i) {
        if (config.layer_color[i] >= EH_RGB_PALETTE_SIZE) {
            config.layer_color[i] = dflt.layer_color[i];
        }
    }
    /* timeout_mins is uint8_t, full 0..255 range is allowed */
    return config;
}

static void kb_settings_led_colors_update(kb_settings_led_colors_t new_config) {
    new_config = kb_settings_led_colors_sanitize(new_config);
    if (memcmp(&new_config, &kb_settings_led_colors, sizeof(new_config)) != 0) {
        kb_settings_led_colors = new_config;
        eeconfig_update_kb_datablock(&kb_settings_led_colors, KB_SETTINGS_LED_COLORS_OFFSET, sizeof(kb_settings_led_colors));
    }
}

void kb_settings_led_colors_init(void) {
    eeconfig_read_kb_datablock(&kb_settings_led_colors, KB_SETTINGS_LED_COLORS_OFFSET, sizeof(kb_settings_led_colors));
    kb_settings_led_colors = kb_settings_led_colors_sanitize(kb_settings_led_colors);
    ergohaven_rgb_refresh_layer_colors();
    layer_state_set_rgb(layer_state | default_layer_state);
}

void kb_settings_led_colors_reset(void) {
    kb_settings_led_colors_update(get_settings_led_colors_default());
    ergohaven_rgb_refresh_layer_colors();
    layer_state_set_rgb(layer_state | default_layer_state);
}

kb_settings_led_colors_t get_settings_led_colors(void) {
    return kb_settings_led_colors;
}

void set_settings_led_colors(kb_settings_led_colors_t config) {
    kb_settings_led_colors_update(config);
    ergohaven_rgb_refresh_layer_colors();
    layer_state_set_rgb(layer_state | default_layer_state);
    if (get_led_rgb_brightness() > 0) {
        rgb_on();
    }
}

uint8_t get_layer_rgb_color(uint8_t layer) {
    if (layer >= EH_RGB_LAYER_COUNT) {
        return 0;
    }
    return kb_settings_led_colors.layer_color[layer];
}

uint8_t get_led_rgb_brightness(void) {
    return kb_settings_led_colors.brightness;
}

void set_led_rgb_brightness(uint8_t brightness) {
    kb_settings_led_colors_t new_config = kb_settings_led_colors;
    new_config.brightness               = brightness;
    kb_settings_led_colors_update(new_config);
    ergohaven_rgb_refresh_layer_colors();
    rgblight_sethsv_noeeprom(rgblight_get_hue(), rgblight_get_sat(), brightness);
    layer_state_set_rgb(layer_state | default_layer_state);
    if (brightness > 0) {
        rgb_on();
    } else {
        rgb_off();
    }
}

uint8_t get_led_rgb_timeout_mins(void) {
    return kb_settings_led_colors.timeout_mins;
}

void set_led_rgb_timeout_mins(uint8_t timeout_mins) {
    kb_settings_led_colors_t new_config = kb_settings_led_colors;
    new_config.timeout_mins             = timeout_mins;
    kb_settings_led_colors_update(new_config);
}

uint32_t get_led_rgb_timeout_ms(void) {
    uint8_t timeout_mins = get_led_rgb_timeout_mins();
    if (timeout_mins == 0) {
        return 0;
    }
    return (uint32_t)timeout_mins * 60 * 1000;
}

void ergohaven_rgb_refresh_layer_colors(void) {
    for (uint8_t layer = 0; layer < EH_RGB_LAYER_COUNT; ++layer) {
        eh_rgb_palette_color_t color = eh_rgb_palette[get_layer_rgb_color(layer)];
        uint16_t scaled_val          = ((uint16_t)color.val * (uint16_t)get_led_rgb_brightness()) / 255;
        layer_segments[layer][0].hue = color.hue;
        layer_segments[layer][0].sat = color.sat;
        layer_segments[layer][0].val = (uint8_t)scaled_val;
    }
}

void set_layer_rgb_color(uint8_t layer, uint8_t color) {
    if (layer >= EH_RGB_LAYER_COUNT) {
        return;
    }
    kb_settings_led_colors_t new_config = kb_settings_led_colors;
    new_config.layer_color[layer]       = color;
    kb_settings_led_colors_update(new_config);
    ergohaven_rgb_refresh_layer_colors();
    layer_state_set_rgb(layer_state | default_layer_state);
    if (get_led_rgb_brightness() > 0) {
        rgb_on();
    }
}

void keyboard_post_init_rgb(void) {
    rgblight_sethsv_noeeprom(rgblight_get_hue(), rgblight_get_sat(), get_led_rgb_brightness());
    ergohaven_rgb_refresh_layer_colors();
    rgblight_layers = my_rgb_layers;
    layer_state_set_rgb(layer_state | default_layer_state);
}

void layer_state_set_rgb(layer_state_t layer_state) {
    rgblight_set_layer_state(0, get_highest_layer(layer_state) == 0);
    for (int layer = 1; layer <= _FIFTEEN; ++layer) {
        rgblight_set_layer_state(layer, layer_state_cmp(layer_state, layer));
    }
}

static bool is_rgb_on = false;

void rgb_on(void) {
    if (!is_rgb_on) {
        rgblight_wakeup();
        rgblight_sethsv_noeeprom(rgblight_get_hue(), rgblight_get_sat(), get_led_rgb_brightness());
        is_rgb_on = true;
    }
}

void rgb_off(void) {
    if (is_rgb_on) {
        rgblight_suspend();
        is_rgb_on = false;
    }
}
