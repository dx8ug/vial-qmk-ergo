# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

This fork of [vial-qmk](https://get.vial.today/) (itself a fork of QMK firmware) is used **exclusively** for the **Ergohaven** keyboard line. Only `keyboards/ergohaven/` is built and maintained here — non-Ergohaven boards are out of scope.

## Hard Rules

- **NEVER push to `upstream`** (`github.com/ergohaven/vial-qmk`). It is read-only — pull/fetch only. The remote has `no_push` set as a safety net, but do not try to circumvent it (no `--repo`, no temporary URL swap, no `set-url`). All pushes go to `origin` only.

## Working Tree

Sparse-checkout (cone mode) restricts the working tree to `keyboards/ergohaven/` and `layouts/default/` — this is the permanent setup, not a temporary filter. Run `git sparse-checkout reapply` after upstream merges if new paths appear. Hidden directories still exist in git history.

## Build Commands

```bash
# Build a specific keyboard/keymap
qmk compile -kb ergohaven/k03/rev3 -km v3_v4
qmk compile -kb ergohaven/hpd/rev2 -km v2_enc_ball
qmk compile -kb ergohaven/imperial44/rev3 -km v3

# Build all ergohaven keyboards (as CI does)
# See .github/workflows/build-ergohaven.yml for the full matrix
```

Output is a `.uf2` file (RP2040 drag-and-drop flashing).

All keyboards use **RP2040** (GENERIC_RP_RP2040 board, rp2040 bootloader).

## Supported Keyboards

| Keyboard | Revisions | Key keymaps |
|---|---|---|
| `k03` | rev1, rev3 | v1_v2, v3_v4 |
| `k03pro` | rev1/43mm, rev1/65mm, rev2 | v1, v2 |
| `imperial44` | rev1, rev3 | v1_v2, v3 |
| `velvet` | rev1, rev2, rev3 | v1, v2, v3 |
| `hpd` | rev1, rev2 | v1, v2, v2_enc_ball, v2_ball_enc, v2_enc_touch, v2_touch_enc, v2_enc_enc, v2_enc_joy |
| `k02` | (flat) | v1 |
| `remnant` | (flat) | v1 |
| `planeta` | rev1, rev2 | v1, v2 |
| `macropad` | rev1, rev2, rev3 | v1, v2, v2_ccw, v3 |
| `trackball` | (flat) | v1, v2 |

## Shared Ergohaven Code (`keyboards/ergohaven/`)

All keyboards share a common codebase. Key source files:

- **`ergohaven_main.c`** — `process_record_kb`, `housekeeping_task_kb`, kb_config (EEPROM), alt-tab logic
- **`ergohaven_ruen.{c,h}`** — Russian/English bilingual input. Tracks OS-level language state and remaps keycodes transparently. Three toggle modes (TG_DEFAULT, TG_M0, TG_M1M2). Custom keycodes `LG_*` for language-aware symbols.
- **`ergohaven_oled.{c,h}`** — SSD1306 OLED output (k03, imperial44, velvet)
- **`ergohaven_display.{c,h}`, `ergohaven_display_modes.c`** — LVGL TFT display (hpd, k03pro). Guarded by `EH_HAS_DISPLAY`. Screens: splash, volume, home.
- **`ergohaven_pointing.{c,h}`** — Trackball/touchpad pointing device logic
- **`ergohaven_rgb.{c,h}`** — RGB lighting helpers
- **`hid.{c,h}`** — RAW HID protocol for talking to a companion host application over USB. Host pushes display data (time/volume/layout/media — `_TIME`/`_VOLUME`/`_LAYOUT`/`_MEDIA_*`) into `hid_data` for the OLED/TFT to render; keyboard pushes its own UI state (`_HID_KB_STATE` with subtypes `_HID_LAYER`/`_HID_LANG`/`_HID_MAC_MODE`/`_HID_RUEN_LAYOUT`) back via `hid_send_kb_state` whenever it changes. `_HID_HELLO` (`0xBB`) is a liveness handshake the host sends on connect and every 30s thereafter; `data[2]==1` distinguishes an "initial" HELLO (host just connected — forces a full state resync via the `hid_force_resync` flag so the host receives current `layer`/`lang`/`macMode`/`ruenLayout` without waiting for the user to press a key) from periodic pings (`data[2]==0`, only refreshes the 75s `is_hid_active()` window). `housekeeping_task_hid` gates its send block on `is_keyboard_master() && hid_force_resync` because on split builds with `EH_FORCE_SPLIT_HID_SYNC` (or `OLED_ENABLE`/`EH_HAS_DISPLAY`) the slave half also receives the HELLO via RPC sync and would otherwise run a no-op `raw_hid_send` round. The full protocol byte map lives in this file's `hid_data_type` and `hid_kb_state_subtype` enums — that's the contract any host must implement.
- **`ergohaven.h`** — Shared layer indices (`_BASE`…`_FIFTEEN`), macros, custom keycode enum, `EH_VERSION_STR`

## Keymap Layout Files

Each keymap has a `vial.json` defining the Vial GUI layout. Shared Vial custom keycode definitions live in:
- `keyboards/ergohaven/vial_keycodes_base.json`
- `keyboards/ergohaven/vial_keycodes_ruen.json`
- `keyboards/ergohaven/vial_keycodes_pointing.json`
- `keyboards/ergohaven/vial_custom_keycodes.json`

## Key QMK Features Enabled

Most keyboards enable: `SPLIT_KEYBOARD`, `VIA_ENABLE`, `TAP_DANCE_ENABLE`, `COMBO_ENABLE`, `KEY_OVERRIDE_ENABLE`, `DYNAMIC_MACRO_ENABLE`, `CAPS_WORD_ENABLE`, `REPEAT_KEY_ENABLE`, `AUTO_SHIFT_ENABLE`, `UNICODE_ENABLE`, `NKRO_ENABLE`.

Split transport uses `SERIAL_DRIVER = vendor` (RP2040 PIO UART) with `SPLIT_HAND_PIN`.
