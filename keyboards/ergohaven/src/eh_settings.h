#pragma once

#include <stdint.h>
#include <stdbool.h>

void kb_settings_init(void);

/* should override if needed */
const char *default_layer_label(uint8_t layer);

const char *layer_name(uint8_t layer);

extern bool layer_name_updated;

void kb_settings_lcd_init(void);
void kb_settings_lcd_reset(void);

uint8_t get_lcd_brightness(void);
void set_lcd_brightness(uint8_t brightness);
uint8_t get_lcd_timeout_mins(void);
void set_lcd_timeout_mins(uint8_t timeout_mins);
uint32_t get_lcd_timeout_ms(void);

uint8_t get_split_lcd_brightness(void);
uint32_t get_split_lcd_timeout_ms(void);
