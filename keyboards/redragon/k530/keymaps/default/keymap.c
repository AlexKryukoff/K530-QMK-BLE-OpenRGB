
#include QMK_KEYBOARD_H
#include "bluetooth.h"
#include "debug.h" // dprintf, только для опционального лога в keyboard_post_init_user

enum layer_names {
    _BASE,
    _FN,       // физический Fn: полный слой (F1-F12 и т.д.)
    _FN_CAPS,  // удержание CapsLock / Magic Fn: то же самое, но 1/2 играют макросы
    _FN2,      // физический Fn2
    _FN2_REC,  // вложенный слой: удержание Fn2 + "6"
};

enum custom_keycodes {
    FN2_REC = SAFE_RANGE, // "6" внутри _FN2 -> держит слой _FN2_REC, пока зажата
    REC1_TOGGLE,          // "1" внутри _FN2_REC -> старт/стоп записи в слот 1
    REC2_TOGGLE,          // "2" внутри _FN2_REC -> старт/стоп записи в слот 2
    REC3_TOGGLE,
    REC4_TOGGLE,
    REC5_TOGGLE,
    REC6_TOGGLE,
    MACRO_PLAY1,          // "1" внутри _FN_CAPS -> воспроизвести слот 1
    MACRO_PLAY2,          // "2" внутри _FN_CAPS -> воспроизвести слот 2
    MACRO_PLAY3,
    MACRO_PLAY4,
    MACRO_PLAY5,
    MACRO_PLAY6,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*  Row:        0                1           2           3         4           5        6        7           8           9           10          11          12          13       */
    [_BASE]    = { {   KC_ESC,          KC_1,       KC_2,       KC_3,     KC_4,       KC_5,    KC_6,    KC_7,       KC_8,       KC_9,       KC_0,       KC_MINS,    KC_EQL,     KC_BSPC, },
                   {   KC_TAB,          KC_Q,       KC_W,       KC_E,     KC_R,       KC_T,    KC_Y,    KC_U,       KC_I,       KC_O,       KC_P,       KC_LBRC,    KC_RBRC,    KC_BSLS, },
                   {   LT(_FN_CAPS,KC_CAPS), KC_A,  KC_S,       KC_D,     KC_F,       KC_G,    KC_H,    KC_J,       KC_K,       KC_L,       KC_SCLN,    KC_QUOT,    KC_NO,      KC_ENT,  },
                   {   KC_LSFT,         KC_NO,      KC_Z,       KC_X,     KC_C,       KC_V,    KC_B,    KC_N,       KC_M,       KC_COMM,    KC_DOT,     KC_SLSH,    KC_NO,      KC_RSFT, },
                   {   KC_LCTL,         KC_LGUI,    KC_LALT,    KC_NO,    KC_NO,      KC_NO,   KC_SPC,  KC_NO,      KC_NO,      KC_RALT,    MO(_FN),    KC_NO,      LT(_FN2,KC_APP),   KC_RCTL, },
                 },

    [_FN]      = { {   KC_GRV,          KC_F1,      KC_F2,      KC_F3,    KC_F4,      KC_F5,   KC_F6,   KC_F7,      KC_F8,      KC_F9,      KC_F10,     KC_F11,     KC_F12,     _______, },
                   {   _______,         _______,    KC_UP,      _______,  _______,    _______, _______, _______,    _______,    _______,    KC_PSCR,    KC_HOME,    KC_END,     _______, },
                   {   _______,         KC_LEFT,    KC_DOWN,    KC_RIGHT, _______,    _______, _______, _______,    _______,    _______,    KC_PGUP,    KC_PGDN,    _______,    _______, },
                   {   _______,         _______,    _______,    _______,  _______,    _______, _______, _______,    _______,    _______,    KC_INS,     KC_DEL,     _______,    _______, },
                   {   _______,         _______,    _______,    _______,  _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    RESET,   },
                 },

    // Как _FN, но 1 и 2 в цифровом ряду заменены на воспроизведение макросов.
    [_FN_CAPS] = { {   _______,         MACRO_PLAY1,MACRO_PLAY2,MACRO_PLAY3,MACRO_PLAY4,MACRO_PLAY5, MACRO_PLAY6, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                   {   _______,         _______,    KC_UP,      _______,  _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                   {   _______,         KC_LEFT,    KC_DOWN,    KC_RIGHT, _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                   {   _______,         _______,    _______,    _______,  _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                   {   _______,         _______,    _______,    _______,  _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                 },

    // Fn2: ESC — заглушка (как в стоке). "6" — держит слой записи. MR (Backspace) — RGB_TOG.
    [_FN2]     = { {   _______,         _______,    _______,    _______,  _______,    _______, _______, FN2_REC,    _______,    _______,    _______,    _______,    _______,    RGB_TOG, },
                   {   _______,         _______,    _______,    _______,  _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                   {   _______,         _______,    _______,    _______,  _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                   {   _______,         _______,    _______,    _______,  _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                   {   _______,         _______,    _______,    _______,  _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                 },

    // Fn2 + "6": 1/2 = старт/стоп записи слота 1/2 (та же клавиша запускает и завершает).
    [_FN2_REC] = { {   _______,         REC1_TOGGLE,REC2_TOGGLE,REC3_TOGGLE,REC4_TOGGLE,REC5_TOGGLE,REC6_TOGGLE, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                   {   _______,         _______,    _______,    _______,  _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                   {   _______,         _______,    _______,    _______,  _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                   {   _______,         _______,    _______,    _______,  _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                   {   _______,         _______,    _______,    _______,  _______,    _______, _______, _______,    _______,    _______,    _______,    _______,    _______,    _______, },
                 },
};

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case LT(_FN_CAPS,KC_CAPS):
        case LT(_FN2, KC_APP):
            return 300; // 0.3 секунды 
        default:
            return TAPPING_TERM;
    }
}

// ---------------------------------------------------------------------
// Собственный (не встроенный) движок записи макросов — даёт честную
// отмену без сохранения и статус "слот записан/пуст" для LED, чего нет
// у стандартного QMK Dynamic Macro (там DM_RSTP сохраняет, а не отменяет).
// ---------------------------------------------------------------------
#define MACRO_MAX_EVENTS 32

typedef struct {
    uint16_t keycode;
    bool     pressed;
} macro_event_t;

static macro_event_t macro_buf[6][MACRO_MAX_EVENTS];
static uint8_t        macro_len[6]         = {0, 0, 0, 0, 0, 0};
static bool           macro_has_content[6] = {false, false,false, false,false, false}; // для LED: записан ли слот

static bool    recording_active = false;
static uint8_t recording_slot   = 0;   // 1 или 2, пока recording_active
static uint8_t recording_index  = 0;
static bool    fn2_rec_key_used = false; // была ли нажата 1/2 во время текущего удержания "6"

#define KEY_REACTIVE_DURATION 800  // мс, как долго клавиша остаётся подсвеченной
static uint16_t key_press_time[63] = {0};
static bool key_held[63] = {false};

// Мгновенное отслеживание физического состояния CapsLock (Magic Fn),
// в обход задержки tapping term — используется для LED-индикации.
static bool capslock_held = false;
static bool fn2_held = false;
static bool fn2_rec_select_held = false; // Fn2 + "7" держится

// ---------------------------------------------------------------------
// Bluetooth: параллельный трекер 6-key rollover + модификаторов.
// ---------------------------------------------------------------------
// Независимый от внутреннего репорта QMK (который при NKRO_ENABLE=yes
// в rules.mk может в рантайме переключаться в NKRO-формат, несовместимый
// с k530_build_keyboard_frame() в bluetooth.c, рассчитанной на простой
// 6-key report_keyboard_t) — ведём свой собственный компактный массив,
// обновляемый прямо здесь на каждое базовое нажатие/отпускание. Это тот
// же подход, что и в оригинальной прошивке K530 (см. отчёт по реверс-
// инжинирингу, раунд 8: add_or_remove_key ведёт свой отдельный компактный
// массив параллельно любому другому представлению отчёта).
static uint8_t bt_report_mods    = 0;
static uint8_t bt_report_keys[6] = {0, 0, 0, 0, 0, 0};

static uint8_t bt_modifier_bit(uint16_t keycode) {
    switch (keycode) {
        case KC_LCTL: return 0x01;
        case KC_LSFT: return 0x02;
        case KC_LALT: return 0x04;
        case KC_LGUI: return 0x08;
        case KC_RCTL: return 0x10;
        case KC_RSFT: return 0x20;
        case KC_RALT: return 0x40;
        case KC_RGUI: return 0x80;
        default:      return 0;
    }
}

static void bt_add_key(uint8_t kc) {
    if (kc == 0) return;
    for (uint8_t i = 0; i < 6; i++) {
        if (bt_report_keys[i] == kc) return; // уже есть
    }
    for (uint8_t i = 0; i < 6; i++) {
        if (bt_report_keys[i] == 0) {
            bt_report_keys[i] = kc;
            return;
        }
    }
    // все 6 слотов заняты — молча игнорируем (как add_or_remove_key в
    // оригинале при переполнении, см. отчёт раунд 8)
}

static void bt_del_key(uint8_t kc) {
    if (kc == 0) return;
    for (uint8_t i = 0; i < 6; i++) {
        if (bt_report_keys[i] == kc) {
            // уплотнение — сдвигаем хвост, как в оригинале (раунд 8)
            for (uint8_t j = i; j < 5; j++) {
                bt_report_keys[j] = bt_report_keys[j + 1];
            }
            bt_report_keys[5] = 0;
            return;
        }
    }
}

// Собирает report_keyboard_t из нашего локального состояния и шлёт по BT.
// bluetooth_send_keyboard() сама ничего не делает, если тумблер физически
// выключен — но проверяем k530_bluetooth_is_enabled() уже здесь тоже,
// чтобы не тратить время на сборку структуры, когда BT точно выключен.
static void bt_sync_and_send(void) {
    if (!k530_bluetooth_is_enabled()) {
        return;
    }
    report_keyboard_t r = {0};
    r.mods = bt_report_mods;
    for (uint8_t i = 0; i < 6; i++) {
        r.keys[i] = bt_report_keys[i];
    }
    bluetooth_send_keyboard(&r);
}

// Обрабатывает "обычную" клавишу (не наши внутренние управляющие коды) для
// целей Bluetooth-репорта. keycode > 0xFF намеренно исключён — составные
// значения (LT()/MO()/MT()/кастомные SAFE_RANGE-коды из enum custom_keycodes
// выше) кодируются числами выше 0xFF в этой версии QMK, и сам факт
// активации слоя/удержания не должен попадать в HID-отчёт как "нажатая
// клавиша". process_record_user получает LT()-обёрнутый keycode на
// press/release ДАЖЕ когда интерпретация — hold (см. capslock_held/
// fn2_held выше в этом же файле) — этот фильтр защищает именно от такого
// случая.
static void bt_track_basic_keycode(uint16_t keycode, bool pressed) {
    if (keycode > 0xFF) {
        return;
    }
    uint8_t mod_bit = bt_modifier_bit(keycode);
    if (mod_bit) {
        if (pressed) {
            bt_report_mods |= mod_bit;
        } else {
            bt_report_mods &= ~mod_bit;
        }
    } else {
        if (pressed) {
            bt_add_key((uint8_t)keycode);
        } else {
            bt_del_key((uint8_t)keycode);
        }
    }
    bt_sync_and_send();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    uint8_t reactive_led = g_led_config.matrix_co[record->event.key.row][record->event.key.col];
    if (reactive_led != NO_LED) {
    if (record->event.pressed) {
        key_held[reactive_led] = true;
        key_press_time[reactive_led] = 0; // отменяем возможное незавершённое затухание
    } else {
        key_held[reactive_led] = false;
        key_press_time[reactive_led] = timer_read(); // старт затухания именно с момента отпускания
    }
    }
    if (keycode == LT(_FN_CAPS, KC_CAPS)) {
        capslock_held = record->event.pressed;
        // не return — даём коду ниже продолжить обычную обработку tap/hold
    }
    if (keycode == LT(_FN2, KC_APP)) {
    fn2_held = record->event.pressed;
    }

    // Захват событий во время записи — работает для ЛЮБЫХ клавиш, кроме
    // наших управляющих кодов и самой клавиши Fn2, чтобы случайно не
    // записать саму комбинацию остановки в тело макроса.
    if (recording_active &&
    keycode != FN2_REC && keycode != REC1_TOGGLE && keycode != REC2_TOGGLE && keycode != REC3_TOGGLE && keycode != REC4_TOGGLE && keycode != REC5_TOGGLE && keycode != REC6_TOGGLE &&
    keycode != LT(_FN2, KC_APP) && keycode != LT(_FN_CAPS,KC_CAPS) &&
    keycode != MACRO_PLAY1 && keycode != MACRO_PLAY2 && keycode != MACRO_PLAY3 && keycode != MACRO_PLAY4 && keycode != MACRO_PLAY5 && keycode != MACRO_PLAY6) {
        if (recording_index < MACRO_MAX_EVENTS) {
            macro_buf[recording_slot - 1][recording_index].keycode = keycode;
            macro_buf[recording_slot - 1][recording_index].pressed = record->event.pressed;
            recording_index++;
        }
        // Событие не блокируется — оно продолжит обрабатываться штатно ниже,
        // поэтому вы видите набираемый текст на экране во время записи.
    }

    switch (keycode) {
        case FN2_REC:
            if (record->event.pressed) {
                fn2_rec_key_used = false;
                fn2_rec_select_held = true;
                layer_on(_FN2_REC);
            } else {
                fn2_rec_select_held = false;
                layer_off(_FN2_REC);
                // "6" отпущена без 1/2 — если запись идёт, отменяем её (без сохранения).
                if (!fn2_rec_key_used && recording_active) {
                    recording_active = false; // буфер просто отбрасывается
                }
            }
            return false;

        case REC1_TOGGLE:
        case REC2_TOGGLE:
        case REC3_TOGGLE:
        case REC4_TOGGLE:
        case REC5_TOGGLE:
        case REC6_TOGGLE:

            if (record->event.pressed) {
                uint8_t slot = (keycode == REC1_TOGGLE) ? 1 : (keycode == REC2_TOGGLE) ? 2 : (keycode == REC3_TOGGLE) ? 3 : (keycode == REC4_TOGGLE) ? 4 : (keycode == REC5_TOGGLE) ? 5 : 6;                fn2_rec_key_used = true;

                if (!recording_active) {
                    // Старт записи в этот слот
                    recording_active = true;
                    recording_slot   = slot;
                    recording_index  = 0;
                } else if (recording_slot == slot) {
                    // Та же клавиша снова — стоп и сохранение
                    recording_active            = false;
                    macro_len[slot - 1]         = recording_index;
                    macro_has_content[slot - 1] = (recording_index > 0);
                }
                // Иначе (другой слот нажат, пока идёт запись первого) — игнорируем.
            }
            return false;

        case MACRO_PLAY1:
        case MACRO_PLAY2:
        case MACRO_PLAY3:
        case MACRO_PLAY4:
        case MACRO_PLAY5:
        case MACRO_PLAY6: {
            if (record->event.pressed) {
                uint8_t slot = (keycode == MACRO_PLAY1) ? 1 : (keycode == MACRO_PLAY2) ? 2 : (keycode == MACRO_PLAY3) ? 3 : (keycode == MACRO_PLAY4) ? 4 : (keycode == MACRO_PLAY5) ? 5 : 6;
                for (uint8_t i = 0; i < macro_len[slot - 1]; i++) {
                    if (macro_buf[slot - 1][i].pressed) {
                        register_code16(macro_buf[slot - 1][i].keycode);
                    } else {
                        unregister_code16(macro_buf[slot - 1][i].keycode);
                    }
                    // Воспроизведение макроса идёт мимо обычного пути
                    // process_record_user (register_code16/unregister_code16
                    // не вызывают его снова) — поэтому синхронизируем BT-
                    // трекер здесь же явно, иначе воспроизведённые макросом
                    // клавиши не долетят до BT-хоста вообще.
                    bt_track_basic_keycode(macro_buf[slot - 1][i].keycode, macro_buf[slot - 1][i].pressed);
                }
            }
            return false;
        }
    }
    // Обычная клавиша, дошедшая до штатной обработки QMK (все наши
    // управляющие коды выше уже вернули false и сюда не попадают) —
    // синхронизируем с Bluetooth-трекером.
    bt_track_basic_keycode(keycode, record->event.pressed);
    
    return true;
}

void keyboard_post_init_user(void) {
    debug_enable = true;
    bluetooth_init();
#ifdef K530_BT_DEBUG
    dprintf("[keymap] keyboard_post_init_user: bluetooth_init() called\n");
#endif
}

// В этой версии QMK (0.15.12, см. лог сборки) housekeeping_task_user()
// может отсутствовать — используем matrix_scan_user(), который точно
// поддерживается на любой версии и вызывается из главного цикла.
void matrix_scan_user(void) {
    bluetooth_task();
}

// ---------------------------------------------------------------------
// LED-индикация:
// 1. Подсветка клавиш активного слоя (Fn1 / Fn2 / Magic Fn)
// 2. Активная запись макроса (белым)
// 3. Статус слотов макросов на Magic Fn (зелёный/красный)
// 4. Реальный CapsLock (красным)
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// LED-индикация (advanced-версия — вызывается на каждый обрабатываемый
// пакет LED с диапазоном led_min/led_max, что убирает мерцание от
// гонки с батчевой отрисовкой базового эффекта на медленном MCU).
// ---------------------------------------------------------------------
void rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
// 1. Подсветка клавиш активного слоя (разный цвет на каждый слой)
uint8_t highlight_layer = 255;
uint8_t highlight_r = 0, highlight_g = 0, highlight_b = 0;

if (capslock_held) {
    highlight_layer = _FN_CAPS;
    highlight_r = 255; highlight_g = 110; highlight_b = 0; // оранжевый — Magic Fn
} else if (fn2_held) {
    highlight_layer = _FN2;
    highlight_r = 255; highlight_g = 0; highlight_b = 0;   // красный — Fn2
} else if (layer_state_is(_FN)) {
    highlight_layer = _FN;
    highlight_r = 0; highlight_g = 128; highlight_b = 0;   // зелёный — Fn1
}

if (highlight_layer != 255) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led_index = g_led_config.matrix_co[row][col];
            if (led_index == NO_LED) continue;
            if (led_index < led_min || led_index >= led_max) continue;

            uint16_t kc = keymap_key_to_keycode(highlight_layer, (keypos_t){.row = row, .col = col});
            if (kc == RESET) {
                // Клавиша сброса в bootloader — всегда красная, независимо от цвета слоя
                rgb_matrix_set_color(led_index, 255, 0, 0);
            } 
            else if (kc == RGB_TOG) {
                // Клавиша RGB_TOG — всегда фиолетовая, независимо от цвета слоя
                rgb_matrix_set_color(led_index, 205, 0, 205);
            } else if (kc != KC_TRNS && kc != KC_NO) {
                rgb_matrix_set_color(led_index, highlight_r, highlight_g, highlight_b);
            }
        }
    }
}

// 2. Идёт запись — светим клавишу активного слота (1-6)
if (recording_active) {
    uint8_t led = recording_slot; // слот N = ключ N напрямую
    if (led >= led_min && led < led_max) {
        rgb_matrix_set_color(led, RGB_RED);
    }
}

// 3. Держится Magic Fn — статус всех 6 слотов
if (capslock_held || fn2_rec_select_held) {
    for (uint8_t slot = 1; slot <= 6; slot++) {
        if (slot >= led_min && slot < led_max) {
            if (macro_has_content[slot - 1]) {
                rgb_matrix_set_color(slot, RGB_GREEN);
            } else {
                rgb_matrix_set_color(slot, RGB_RED);
            }
        }
    }
}

    // 4. Реальное состояние Caps Lock (не Magic Fn) — сплошной красный на
    // самой клавише CapsLock, как в стоке.
    if (host_keyboard_led_state().caps_lock) {
        if (28 >= led_min && 28 < led_max) {
            rgb_matrix_set_color(28, RGB_RED);
        }
    }
    // 5. Реакция на нажатие клавиши — держим ярко, пока зажата; после отпускания — плавное затухание
    #define FADE_BASE_R 40
    #define FADE_BASE_G 40
    #define FADE_BASE_B 40

    for (uint8_t i = led_min; i < led_max; i++) {
    if (key_held[i]) {
        // Клавиша всё ещё зажата — полная яркость, без затухания
        rgb_matrix_set_color(i, RGB_WHITE);
    } else if (key_press_time[i] != 0) {
        uint16_t elapsed = timer_elapsed(key_press_time[i]);
        if (elapsed < KEY_REACTIVE_DURATION) {
            uint8_t brightness = 255 - ((uint32_t)(elapsed * elapsed) * 255 / ((uint32_t)KEY_REACTIVE_DURATION * KEY_REACTIVE_DURATION));
            uint8_t r = FADE_BASE_R + (((int16_t)255 - FADE_BASE_R) * brightness) / 255;
            uint8_t g = FADE_BASE_G + (((int16_t)255 - FADE_BASE_G) * brightness) / 255;
            uint8_t b = FADE_BASE_B + (((int16_t)255 - FADE_BASE_B) * brightness) / 255;
            rgb_matrix_set_color(i, r, g, b);
        } else {
            key_press_time[i] = 0;
        }
    }
    }
}