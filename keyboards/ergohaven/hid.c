#include "hid.h"
#include <string.h>

#include "via.h"
#include "raw_hid.h"
#include "ergohaven_rgb.h"
#include "quantum.h"
#include "src/eh_ruen.h"

static hid_data_t hid_data;

hid_data_t *get_hid_data(void) {
    return &hid_data;
}

#define HID_HELLO_TIMEOUT_MS 75000  // 2.5x host PING interval (30s)

static uint32_t hid_sync_time = 0;

bool is_hid_active(void) {
    return (hid_sync_time != 0) && timer_elapsed32(hid_sync_time) < HID_HELLO_TIMEOUT_MS;
}

typedef enum {
    _TIME = 0xAA, // random value that does not conflict with VIA, must match companion app
    _VOLUME,
    _LAYOUT,
    _MEDIA_ARTIST,
    _MEDIA_TITLE,

    _HID_HELLO = 0xBB, // host liveness ping, must match companion app

    _RELAY_FROM_DEVICE = 0xCC,
    _RELAY_TO_DEVICE,

    _HID_KB_STATE = 0xDD,
} hid_data_type;

typedef enum {
    _POINTING = 10,
} relay_data_type;

typedef enum {
    _HID_LAYER = 1,
    _HID_LANG,
    _HID_MAC_MODE,
    _HID_RUEN_LAYOUT,
} hid_kb_state_subtype;

void read_string(uint8_t *data, char *string_data) {
    uint8_t data_length = MIN(31, data[1]);
    memcpy(string_data, data + 2, data_length);
    string_data[data_length] = '\0';
}

bool process_raw_hid_data(uint8_t *data, uint8_t length) {
    uint8_t data_type = data[0];

    bool host_alive = false;  // any recognized packet — updates hid_sync_time, suppresses Vial echo, syncs to slave
    bool ui_changed = false;  // display data modified — sets hid_data.hid_changed

    switch (data_type) {
        case _TIME:
            hid_data.hours        = data[1];
            hid_data.minutes      = data[2];
            hid_data.time_changed = true;
            ui_changed            = true;
            host_alive            = true;
            break;

        case _VOLUME:
            hid_data.volume         = data[1];
            hid_data.volume_changed = true;
            ui_changed              = true;
            host_alive              = true;
            break;

        case _LAYOUT:
            hid_data.layout         = data[1];
            hid_data.layout_changed = true;
            ui_changed              = true;
            host_alive              = true;
            break;

        case _MEDIA_ARTIST:
            read_string(data, hid_data.media_artist);
            hid_data.media_artist_changed = true;
            ui_changed                    = true;
            host_alive                    = true;
            break;

        case _MEDIA_TITLE:
            read_string(data, hid_data.media_title);
            hid_data.media_title_changed = true;
            ui_changed                   = true;
            host_alive                   = true;
            break;

        case _RELAY_TO_DEVICE:
            switch (data[1]) {
                case _POINTING:
                    set_pointing_mode_from_hid(data[2]);
                    break;
            }
            host_alive = true;  // slave sync needed: pointing_mode global must stay aligned across halves
            break;

        case _HID_HELLO:
            host_alive = true;
            break;

        default:
            break;
    }

    if (host_alive) hid_sync_time = timer_read32();
    if (ui_changed) hid_data.hid_changed = true;

    return host_alive;
}

void hid_send_pointing_mode(pointing_mode_t mode) {
    uint8_t data[32];
    memset(data, 0, 32);
    data[0] = _RELAY_FROM_DEVICE;
    data[1] = _POINTING;
    data[2] = mode;
    raw_hid_send(data, 32);
}

static bool process_via_custom_lighting(uint8_t *data, uint8_t length) {
#if defined(VIA_CUSTOM_LIGHTING_ENABLE)
    if (length < 4) {
        return false;
    }

    uint8_t *command_id = &data[0];
    uint8_t *channel_id = &data[1];
    uint8_t *value_id   = &data[2];
    uint8_t *value_data = &data[3];

    if (*channel_id != id_custom_channel) {
        *command_id = id_unhandled;
        return true;
    }

    switch (*command_id) {
        case id_lighting_get_value:
            if (*value_id == 1) {
                value_data[0] = get_led_rgb_brightness();
                return true;
            }
            if (*value_id >= 2 && *value_id < 2 + EH_RGB_LAYER_COUNT) {
                value_data[0] = get_layer_rgb_color(*value_id - 2);
                return true;
            }
            *command_id = id_unhandled;
            return true;

        case id_lighting_set_value:
            if (*value_id == 1) {
                set_led_rgb_brightness(value_data[0]);
                return true;
            }
            if (*value_id >= 2 && *value_id < 2 + EH_RGB_LAYER_COUNT) {
                set_layer_rgb_color(*value_id - 2, value_data[0]);
                return true;
            }
            *command_id = id_unhandled;
            return true;

        case id_lighting_save:
            return true;
    }
#endif

    return false;
}

static void hid_send_kb_state(uint8_t subtype, uint8_t value) {
    uint8_t data[32] = {0};
    data[0] = _HID_KB_STATE;
    data[1] = subtype;
    data[2] = value;
    raw_hid_send(data, 32);
}

void hid_send_layer_change(uint8_t layer) {
    hid_send_kb_state(_HID_LAYER, layer);
}

void hid_send_lang_change(uint8_t lang) {
    hid_send_kb_state(_HID_LANG, lang);
}

void hid_send_mac_mode(bool mac) {
    hid_send_kb_state(_HID_MAC_MODE, mac ? 1 : 0);
}

void hid_send_ruen_layout(bool mac) {
    hid_send_kb_state(_HID_RUEN_LAYOUT, mac ? 1 : 0);
}

void housekeeping_task_hid(void) {
    static bool    hid_was_active = false;
    static uint8_t prev_layer     = 0xFF;
    static uint8_t prev_lang      = 0xFF;
    static uint8_t prev_mac       = 0xFF;
    static uint8_t prev_ruen_lo   = 0xFF;

    bool hid_now = is_hid_active();
    if (hid_now) {
        uint8_t cur_layer   = get_highest_layer(layer_state | default_layer_state);
        uint8_t cur_lang    = get_cur_lang();
        uint8_t cur_mac     = keymap_config.swap_lctl_lgui ? 1 : 0;
        uint8_t cur_ruen_lo = get_ruen_mac_layout() ? 1 : 0;

        bool full_sync = !hid_was_active;
        if (full_sync || prev_layer != cur_layer) hid_send_layer_change(cur_layer);
        if (full_sync || prev_lang != cur_lang) hid_send_lang_change(cur_lang);
        if (full_sync || prev_mac != cur_mac) hid_send_mac_mode(cur_mac);
        if (full_sync || prev_ruen_lo != cur_ruen_lo) hid_send_ruen_layout(cur_ruen_lo);

        prev_layer   = cur_layer;
        prev_lang    = cur_lang;
        prev_mac     = cur_mac;
        prev_ruen_lo = cur_ruen_lo;
    }
    hid_was_active = hid_now;
}

#if defined(SPLIT_KEYBOARD) && (defined(OLED_ENABLE) || defined(EH_HAS_DISPLAY) || defined(EH_FORCE_SPLIT_HID_SYNC))
#    include "transactions.h"

void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    if (process_via_custom_lighting(data, length)) {
        return;
    }

    bool res = process_raw_hid_data(data, length);
    if (res && is_keyboard_master()) transaction_rpc_send(RPC_SYNC_HID, length, data);
    if (res) *((uint64_t *)data) = VIAL_HID_MAGIC;
}

void hid_sync(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    (void)out_buflen;
    (void)out_data;
    process_raw_hid_data((uint8_t *)in_data, in_buflen);
}

void keyboard_post_init_hid(void) {
    transaction_register_rpc(RPC_SYNC_HID, hid_sync);
}

#else

void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    if (process_via_custom_lighting(data, length)) {
        return;
    }

    bool res = process_raw_hid_data(data, length);
    if (res) *((uint64_t *)data) = VIAL_HID_MAGIC;
}

void keyboard_post_init_hid(void) {}

#endif
