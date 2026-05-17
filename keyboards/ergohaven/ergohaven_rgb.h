#pragma once

#include "quantum.h"

#define EH_RGB_LAYER_COUNT 16
#define EH_RGB_PALETTE_SIZE 25

typedef struct {
    uint8_t layer_color[EH_RGB_LAYER_COUNT];
    uint8_t brightness;
    uint8_t timeout_mins;
} kb_settings_led_colors_t;

void keyboard_post_init_rgb(void);
void layer_state_set_rgb(layer_state_t layer_state);
void rgb_on(void);
void rgb_off(void);

void kb_settings_led_colors_init(void);
void kb_settings_led_colors_reset(void);
kb_settings_led_colors_t get_settings_led_colors(void);
void set_settings_led_colors(kb_settings_led_colors_t config);
uint8_t get_layer_rgb_color(uint8_t layer);
void set_layer_rgb_color(uint8_t layer, uint8_t color);
uint8_t get_led_rgb_brightness(void);
void set_led_rgb_brightness(uint8_t brightness);
uint8_t get_led_rgb_timeout_mins(void);
void set_led_rgb_timeout_mins(uint8_t timeout_mins);
uint32_t get_led_rgb_timeout_ms(void);
void ergohaven_rgb_refresh_layer_colors(void);
