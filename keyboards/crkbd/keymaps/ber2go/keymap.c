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

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_split_3x6_3(
  //,------------------------------------------------------------------------------------.                                         ,--------------------------------------------------------------------------------------.
          KC_NO,        KC_Q,         KC_W,         KC_E,         KC_R,         KC_T,                                                   KC_Y,        KC_U,          KC_I,        KC_O,            KC_P,          KC_NO,
  //|-------------+-------------+-------------+-------------+-------------+--------------|                                         |-------------+-------------+-------------+---------------+----------------+-------------|
          KC_NO,    LGUI_T(KC_A), LALT_T(KC_S), LCTL_T(KC_D), LSFT_T(KC_F),     KC_G,                                                   KC_H,      RSFT_T(KC_J), RCTL_T(KC_K), RALT_T(KC_L),  RGUI_T(KC_QUOT),    KC_NO,
  //|-------------+-------------+-------------+-------------+-------------+--------------|                                         |-------------+-------------+-------------+---------------+----------------+-------------|
          KC_NO,    ALL_T(KC_Z),   MEH_T(KC_X),     KC_C,         KC_V,         KC_B,                                                   KC_N,        KC_M,         KC_COMM,    MEH_T(KC_DOT),   ALL_T(KC_SLSH),    KC_NO,
  //|-------------+-------------+-------------+-----------------+------------------------+-------------|     |---------------------+-------------------+---------------------+---------------+----------------+-------------|
                                               LT(_MEDIA,KC_ESC), LT(_NAVIGATION,KC_TAB),   MO(_OTHER),        LT(_SYMBOLS,KC_ENT), LT(_NUMBER,KC_SPC), LT(_FUNCTION,KC_BSPC)
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
      KC_TRNS, KC_SCLN, KC_4,     KC_5,   KC_6,   KC_EQL,                        XXXXXXX, KC_RSFT, KC_RCTL, KC_RALT, KC_RGUI, XXXXXXX,
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
      KC_TRNS, KC_COLON, KC_DLR, KC_PERC, KC_CIRC, KC_PLUS,                       XXXXXXX, KC_RSFT, KC_RCTL, KC_RALT, KC_RGUI, XXXXXXX,
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
      KC_TRNS, KC_F11,  KC_F4,   KC_F5,   KC_F6,   XXXXXXX,                         XXXXXXX, KC_RSFT, KC_RCTL, KC_RALT, KC_RGUI, XXXXXXX,
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
      XXXXXXX, QK_BOOT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      XXXXXXX, RGB_TOG, RGB_HUI, RGB_SAI, RGB_VAI, XXXXXXX,                      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      XXXXXXX, RGB_MOD, RGB_HUD, RGB_SAD, RGB_VAD, XXXXXXX,                      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          KC_TRNS, KC_TRNS, _______,   _______, KC_TRNS, KC_TRNS
                                      //`--------------------------'  `--------------------------'
  )
};
