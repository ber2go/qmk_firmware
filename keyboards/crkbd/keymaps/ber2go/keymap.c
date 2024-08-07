/*
Copyright 2019 @foostan
Copyright 2020 Drashna Jaelre <@drashna>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H
#include "features/achordion.h"

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
  if (!process_achordion(keycode, record)) { return false; }

  switch(keycode) {

  case KC_BSPC: {  // Backspace with exponential repeating.
    // Initial delay before the first repeat.
    static const uint8_t INIT_DELAY_MS = 250;

    // This array customizes the rate at which the Backspace key
    // repeats. The delay after the ith repeat is REP_DELAY_MS[i].
    // Values must be between 1 and 255.
    static const uint8_t REP_DELAY_MS[] PROGMEM = {
        99, 79, 65, 57, 49, 43, 40, 35, 33, 30, 28, 26, 25, 23, 22, 20,
        20, 19, 18, 17, 16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10};
    static deferred_token token = INVALID_DEFERRED_TOKEN;
    static uint8_t rep_count = 0;

    if (!record->event.pressed) {  // Backspace released: stop repeating.
        cancel_deferred_exec(token);
        token = INVALID_DEFERRED_TOKEN;
    } else if (!token) {  // Backspace pressed: start repeating.
        tap_code(KC_BSPC);  // Initial tap of Backspace key.
        rep_count = 0;

        uint32_t bspc_callback(uint32_t trigger_time, void* cb_arg) {
            tap_code(KC_BSPC);
            if (rep_count < sizeof(REP_DELAY_MS)) { ++rep_count; }
            return pgm_read_byte(REP_DELAY_MS - 1 + rep_count);
        }

        token = defer_exec(INIT_DELAY_MS, bspc_callback, NULL);
    }
  } return false;  // Skip normal handling.
  }

  return true;
}

void matrix_scan_user(void) {
  achordion_task();
}

enum {
    TD_SPC_ENT
};

// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    // Tap one for Space, twice for Enter
    [TD_SPC_ENT] = ACTION_TAP_DANCE_DOUBLE(KC_SPC, KC_ENT)
};

void keyboard_post_init_user(void) {
  // Customise these values to desired behaviour
  debug_enable=true;
  //debug_matrix=true;
  //debug_keyboard=true;
  //debug_mouse=true;
}

enum layers {
    _BASE,
    _NAVIGATION,
    _NUMBER,
    _SYMBOLS,
    _FUNCTION,
    _MEDIA,
    _OTHER
};

#define HOME_A LGUI_T(KC_A)
#define HOME_S LALT_T(KC_S)
#define HOME_D LCTL_T(KC_D)
#define HOME_F LSFT_T(KC_F)
#define HOME_J RSFT_T(KC_J)
#define HOME_K RCTL_T(KC_K)
#define HOME_L LALT_T(KC_L)
#define HOME_QUOT RGUI_T(KC_QUOT)
#define MEDIA_ESC LT(_MEDIA,KC_ESC)
#define NAV_SPC LT(_NAVIGATION,KC_SPC)
#define OTHER_TAB LT(_OTHER,KC_TAB)
#define SYM_ENT LT(_SYMBOLS,KC_ENT)
#define NUM_BSPC LT(_NUMBER,KC_BSPC)
#define FUN_DEL LT(_FUNCTION,KC_DEL)

#define HYPR_Z ALL_T(KC_Z)


uint16_t get_quick_tap_term(uint16_t keycode, keyrecord_t* record) {
  // If you quickly hold a tap-hold key after tapping it, the tap action is
  // repeated. Key repeating is useful e.g. for Vim navigation keys, but can
  // lead to missed triggers in fast typing. Here, returning 0 means we
  // instead want to "force hold" and disable key repeating.
  switch (keycode) {
    case HOME_J:
    case HOME_K:
    case HOME_L:
    case NUM_BSPC:
      return QUICK_TAP_TERM;  // Enable key repeating.
    default:
      return 0;  // Otherwise, force hold and disable key repeating.
  }
}

bool achordion_chord(uint16_t tap_hold_keycode,
                     keyrecord_t* tap_hold_record,
                     uint16_t other_keycode,
                     keyrecord_t* other_record) {
  dprintln("Custom achordion_chord");
  switch (tap_hold_keycode) {
    case HYPR_Z:
      if (other_keycode == KC_F) { return true; }
      break;
    case HOME_A:
      if (other_keycode == OTHER_TAB || other_keycode == NAV_SPC) { return true; }
    case OTHER_TAB:
      if (other_keycode == KC_Q || other_keycode == KC_W) { return true; }
      break;
  }

  // Also allow same-hand holds when the other key is in the rows below the
  // alphas. I need the `% (MATRIX_ROWS / 2)` because my keyboard is split.
  // if (other_record->event.key.row % (MATRIX_ROWS / 2) >= 4) { return true; }

  // Otherwise, follow the opposite hands rule.
  return achordion_opposite_hands(tap_hold_record, other_record);
}

uint16_t achordion_timeout(uint16_t tap_hold_keycode) {
  switch (tap_hold_keycode) {
    case HYPR_Z:
      return 0;
  }

  return 1000;
}

uint16_t achordion_streak_chord_timeout(
    uint16_t tap_hold_keycode, uint16_t next_keycode) {
  if (IS_QK_LAYER_TAP(tap_hold_keycode)) {
    return 0;  // Disable streak detection on layer-tap keys.
  }

  // Otherwise, tap_hold_keycode is a mod-tap key.
  uint8_t mod = mod_config(QK_MOD_TAP_GET_MODS(tap_hold_keycode));
  if ((mod & MOD_LSFT) != 0) {
    return 100;  // A shorter streak timeout for Shift mod-tap keys.
  }

  return 200;  // Default of 100 ms.
}

bool achordion_streak_continue(uint16_t keycode) {
  // If mods other than shift or AltGr are held, don't continue the streak.
  if (get_mods() & (MOD_MASK_CG | MOD_BIT_LALT)) return false;
  // This function doesn't get called for holds, so convert to tap keycodes.
  if (IS_QK_MOD_TAP(keycode)) {
    keycode = QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
  }
  if (IS_QK_LAYER_TAP(keycode)) {
    keycode = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
  }
  // Regular letters and punctuation continue the streak.
  if (keycode >= KC_A && keycode <= KC_Z) return true;
  switch (keycode) {
    case KC_DOT:
    case KC_COMMA:
    case KC_QUOTE:
    case KC_SPACE:
      return true;
  }
  return false;  // All other keys end the streak.
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_split_3x6_3(
  //,------------------------------------------------------------------------------------.                                         ,--------------------------------------------------------------------------------------.
          KC_NO,        KC_Q,         KC_W,         KC_E,         KC_R,         KC_T,                                                   KC_Y,        KC_U,          KC_I,        KC_O,            KC_P,          KC_NO,
  //|-------------+-------------+-------------+-------------+-------------+--------------|                                         |-------------+-------------+-------------+---------------+----------------+-------------|
          KC_NO,        HOME_A,     HOME_S,         HOME_D,       HOME_F,       KC_G,                                                   KC_H,        HOME_J,        HOME_K,      HOME_L,          HOME_QUOT,    KC_NO,
  //|-------------+-------------+-------------+-------------+-------------+--------------|                                         |-------------+-------------+-------------+---------------+----------------+-------------|
          KC_NO,    HYPR_Z,   MEH_T(KC_X),     KC_C,         KC_V,         KC_B,                                                   KC_N,        KC_M,         KC_COMM,    MEH_T(KC_DOT),   ALL_T(KC_SLSH),    KC_NO,
  //|-------------+-------------+-------------+-----------------+------------------------+-------------|     |---------------------+-------------------+---------------------+---------------+----------------+-------------|
                                                                MEDIA_ESC,      NAV_SPC,   OTHER_TAB,         SYM_ENT,    NUM_BSPC,      FUN_DEL
                                            //`--------------------------------------------------------'     `---------------------------------------------------------------'

  ),

    [_NAVIGATION] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
      KC_TRNS, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_TRNS, KC_LGUI, KC_LALT, KC_LCTL, KC_LSFT, XXXXXXX,                      KC_LEFT, KC_DOWN,  KC_UP, KC_RIGHT, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_TRNS, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                      KC_INS, KC_HOME, KC_PGUP, KC_PGDN, KC_END,   XXXXXXX,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          KC_TRNS, KC_TRNS, KC_TRNS,    KC_TRNS,   KC_TRNS,  KC_TRNS
                                      //`--------------------------'  `--------------------------'
  ),

    [_NUMBER] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
      KC_TRNS, KC_LBRC, KC_7,     KC_8,   KC_9,   KC_RBRC,                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_TRNS, KC_SCLN, KC_4,     KC_5,   KC_6,   KC_EQL,                        XXXXXXX, KC_RSFT, KC_LCTL, KC_RALT, KC_RGUI, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_TRNS, KC_GRV,  KC_1,     KC_2,   KC_3,   KC_BSLS,                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          KC_TRNS, KC_0,   KC_MINUS,   KC_TRNS, KC_TRNS, KC_TRNS
                                      //`--------------------------'  `--------------------------'
  ),

    [_SYMBOLS] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
      KC_TRNS, S(KC_LBRC), KC_AMPR, KC_ASTR, KC_LPRN, S(KC_RBRC),                XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_TRNS, KC_COLON, KC_DLR, KC_PERC, KC_CIRC, KC_PLUS,                       XXXXXXX, KC_RSFT, KC_LCTL, KC_RALT, KC_RGUI, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_TRNS, KC_TILD, KC_EXLM, KC_AT,  KC_HASH,  KC_PIPE,                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                         KC_LPRN, KC_RPRN, KC_UNDS,    KC_TRNS, KC_TRNS, KC_TRNS
                                      //`--------------------------'  `--------------------------'
  ),

    [_FUNCTION] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
      KC_TRNS, KC_F12,  KC_F7,   KC_F8,   KC_F9,   XXXXXXX,                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_TRNS, KC_F11,  KC_F4,   KC_F5,   KC_F6,   XXXXXXX,                         XXXXXXX, KC_RSFT, KC_LCTL, KC_RALT, KC_RGUI, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_TRNS, KC_F10,  KC_F1,   KC_F2,   KC_F3,   XXXXXXX,                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          KC_TRNS, KC_TRNS, KC_TRNS,   KC_TRNS, KC_TRNS, KC_TRNS
                                      //`--------------------------'  `--------------------------'
  ),

    [_MEDIA] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
      KC_TRNS, XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX,                      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_TRNS, KC_LGUI, KC_LALT, KC_LCTL, KC_LSFT, XXXXXXX,                       KC_MPRV, KC_VOLD, KC_VOLU, KC_MNXT, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_TRNS, XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX,                      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          KC_TRNS, KC_TRNS, KC_TRNS,    KC_MSTP, KC_MPLY, KC_MUTE
                                      //`--------------------------'  `--------------------------'
  ),

    [_OTHER] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
      XXXXXXX, QK_BOOT, QK_MAKE, XXXXXXX, XXXXXXX, XXXXXXX,                      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      XXXXXXX, RGB_TOG, RGB_HUI, RGB_SAI, RGB_VAI, XXXXXXX,                      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      XXXXXXX, RGB_MOD, RGB_HUD, RGB_SAD, RGB_VAD, XXXXXXX,                      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          KC_TRNS, KC_TRNS, _______,   _______, KC_TRNS, KC_TRNS
                                      //`--------------------------'  `--------------------------'
  )
};
