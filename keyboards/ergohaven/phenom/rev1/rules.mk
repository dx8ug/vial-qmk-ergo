MCU = RP2040
BOARD = GENERIC_RP_RP2040
BOOTLOADER = rp2040
# ALLOW_WARNINGS = yes
# PICO_INTRINSICS_ENABLED = no

# Build options
SLEEP_LED_ENABLE = no
SPLIT_KEYBOARD = yes
SERIAL_DRIVER = vendor

NKRO_ENABLE = yes
BOOTMAGIC_ENABLE = yes
MOUSEKEY_ENABLE = yes
EXTRAKEY_ENABLE = yes
LTO_ENABLE = no
VIA_ENABLE = yes
CONSOLE_ENABLE = yes
COMBO_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes
DYNAMIC_MACRO_ENABLE = yes
CAPS_WORD_ENABLE = yes
REPEAT_KEY_ENABLE = yes
AUTO_SHIFT_ENABLE = yes
NO_USB_STARTUP_CHECK = no
RAW_ENABLE = yes

UNICODE_COMMON = yes
UNICODE_ENABLE = yes

ENCODER_ENABLE = yes
ENCODER_MAP_ENABLE = yes

POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = custom
I2C_DRIVER_REQUIRED = yes

RGBLIGHT_ENABLE = yes
RGBLIGHT_DRIVER = ws2812
WS2812_DRIVER = vendor

SRC += keyboards/ergohaven/ergohaven_main.c
SRC += keyboards/ergohaven/ergohaven_rgb.c
SRC += keyboards/ergohaven/src/eh_settings.c
SRC += keyboards/ergohaven/src/eh_ruen.c
SRC += keyboards/ergohaven/hid.c
SRC += keyboards/ergohaven/src/eh_pointing.c
SRC += keyboards/ergohaven/phenom/rev1/pointing.c
SRC += drivers/sensors/azoteq_iqs5xx.c
SRC += drivers/sensors/pmw3610.c
