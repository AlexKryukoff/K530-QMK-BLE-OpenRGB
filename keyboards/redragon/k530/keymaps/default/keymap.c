
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
    BT_UNPAIR,            // Fn2 + ESC (удержание 3 сек) -> сброс сопряжения / режим привязки
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

    // Fn2: ESC — сброс сопряжения BT по удержанию 3 сек (как в стоке). "6" — держит слой записи. MR (Backspace) — RGB_TOG.
    [_FN2]     = { {   BT_UNPAIR,       _______,    _______,    _______,  _______,    _______, _______, FN2_REC,    _______,    _______,    _______,    _______,    _______,    RGB_TOG, },
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

#define KEY_REACTIVE_DURATION 500  // мс, как долго клавиша остаётся подсвеченной
static uint16_t key_press_time[63] = {0};
static bool key_held[63] = {false};

// Мгновенное отслеживание физического состояния CapsLock (Magic Fn),
// в обход задержки tapping term — используется для LED-индикации.
static bool capslock_held = false;
static bool fn2_held = false;
static bool fn2_rec_select_held = false; // Fn2 + "7" держится

// Сброс сопряжения (Fn2+ESC) по удержанию 3 сек, как в стоке. Считаем время
// удержания в matrix_scan_user(), чтобы случайное касание не роняло линк.
#define BT_UNPAIR_HOLD_MS 3000u
static uint16_t bt_unpair_press_time = 0;
static bool     bt_unpair_held  = false;
static bool     bt_unpair_fired = false; // сброс уже сработал за текущее удержание

// ДИАГНОСТИКА перебора кандидатов сброса: после срабатывания несколько секунд
// подсвечиваем номер отправленного кандидата ЦВЕТОМ батарейного индикатора (61).
#define BT_UNPAIR_FEEDBACK_MS 6000u
static uint32_t bt_unpair_feedback_time = 0;

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

// =====================================================================
// СОН ПО БЕЗДЕЙСТВИЮ (по образцу стока)
// =====================================================================
// Логика стока, которую воспроизводим:
//   1. Счётчик бездействия сбрасывается ЛЮБЫМ нажатием.
//   2. При ПОДКЛЮЧЕННОЙ ЗАРЯДКЕ счётчик держится в максимуме ->
//      заснуть невозможно (в стоке — 0x9AD8, гейт по пину P1.14).
//   3. Досчитал до нуля -> гасим всю подсветку и усыпляем ядро.
//   4. Нажатие любой клавиши или подключение зарядки -> пробуждение.
//   5. Кадр подсветки живёт в RAM и не теряется, поэтому после
//      пробуждения картинка (включая DIRECT-режим OpenRGB) восстанавливается
//      сама, без рескана на ПК: USB-устройство с шины НЕ пропадает.
//
// ---------------------------------------------------------------------
// ГЛАВНЫЕ КРУТИЛКИ — менять только здесь:
// ---------------------------------------------------------------------
#define K530_IDLE_SLEEP_ENABLE 1        // 0 = выключить весь механизм одной строкой

// БОЕВОЕ ЗНАЧЕНИЕ: 20 минут бездействия на батарее.
// Для повторных тестов удобно временно ставить 30000u (= 30 секунд).
//   20 минут = 20 * 60 * 1000 = 1200000 мс
// >>> ТЕСТОВАЯ СБОРКА: 30 секунд, чтобы быстро проверить глубокий сон.
//     БОЕВОЕ ЗНАЧЕНИЕ после успешного теста: 1200000u (20 минут).
#define K530_IDLE_SLEEP_MS 1200000u

// Глубина сна: 1 = дополнительно останавливаем ядро инструкцией WFI
// между системными тиками (как в стоке на 0x2996), 0 = только гашение
// подсветки. ЕСЛИ КЛАВИАТУРА СТАЛА ТУПИТЬ ИЛИ ПРОПУСКАТЬ НАЖАТИЯ —
// ПОСТАВЬ ЗДЕСЬ 0, это первый подозреваемый.
#define K530_IDLE_SLEEP_STOP_CPU 1

// ---------------------------------------------------------------------
// ГЛУБОКИЙ СОН (стоковый уровень экономии):
//   1 = во сне дополнительно ОСТАНАВЛИВАЕМ "мотор" подсветки.
// Прерывание PWM-таймера CT16B1 крутится сотни тысяч раз в секунду и не
// даёт ядру отдыхать: пока оно живо, "сон" = просто погашенные диоды.
// В этом же прерывании сканируется клавиатурная матрица, поэтому во сне
// сканер включается короткими вспышками, а между ними ядро уходит в
// аппаратный режим пониженного потребления (ChibiOS сам пишет
// SN_PMU->CTRL = 4 в своём idle-хуке).
// ЕСЛИ ЧТО-ТО ПОЙДЁТ НЕ ТАК - ПОСТАВЬ ЗДЕСЬ 0, вернётся прошлое
// (уже проверенное) поведение: только гашение подсветки.
#define K530_DEEP_SLEEP_ENABLE 1

// Пауза между вспышками сканера = максимальная задержка пробуждения.
// Больше значение -> меньше потребление, но дольше просыпается.
#define K530_DEEP_SLEEP_POLL_MS 12u

// Длительность самой вспышки. Полный обход матрицы занимает около 2 мс
// (15 вызовов PWM-прерывания), поэтому 8 мс = четыре полных обхода
// за одну вспышку.
// ВМЕСТЕ С K530_DEEP_SLEEP_POLL_MS ЭТО ГЛАВНАЯ КРУТИЛКА НАДЁЖНОСТИ
// ПРОБУЖДЕНИЯ: сканер работает SCAN_MS из каждых
// (SCAN_MS + POLL_MS) миллисекунд. Сейчас 8 из 20 мс = 40% времени,
// то есть даже очень короткое касание клавиши гарантированно попадает
// в окно сканирования.
//   если всё ещё просыпается не с первого раза -> SCAN_MS 12u, POLL_MS 8u
//   если хочешь больше экономии и пробуждение надёжное -> POLL_MS 20u
#define K530_DEEP_SLEEP_SCAN_MS 8u

static uint32_t k530_last_activity  = 0;
static bool     k530_asleep         = false;
static bool     k530_rgb_was_on     = true;

// Любая активность: сброс счётчика (аналог стокового 0x362E).
static inline void k530_note_activity(void) {
    k530_last_activity = timer_read32();
}

#if K530_IDLE_SLEEP_ENABLE

#if K530_DEEP_SLEEP_ENABLE
// Строки подсветки берём из того же макроса, что и штатный драйвер.
static const pin_t k530_led_row_pins[LED_MATRIX_ROWS_HW] = LED_MATRIX_ROW_PINS;

// Из штатных драйверов SN32:
//   shared_matrix_rgb_enable()  - rgb_matrix_sn32f24xb.c, тот же вызов, которым
//                                 драйвер запускает "мотор": PWM-прерывание
//                                 плюс сканирование матрицы;
//   SN32F24xB_set_color_all()   - прямая запись в буфер кадра;
//   raw_matrix[]                - matrix_sn32f24xx.c, сырое состояние клавиш,
//                                 его заполняет сам сканер в прерывании.
extern void         shared_matrix_rgb_enable(void);
extern void         SN32F24xB_set_color_all(uint8_t r, uint8_t g, uint8_t b);
extern matrix_row_t raw_matrix[MATRIX_ROWS];

// Глушим "мотор": снимаем периодическое уведомление PWM-таймера
// (после этого rgb_callback() не вызывается и ядро отдыхает) и опускаем
// все строки RGB, чтобы ни одна не осталась подсвеченной.
static void k530_backlight_park(void) {
    pwmDisablePeriodicNotification(&PWMD1);
    for (uint8_t x = 0; x < LED_MATRIX_ROWS_HW; x++) {
        setPinOutput(k530_led_row_pins[x]);
        writePinLow(k530_led_row_pins[x]);
    }
}

// КОРОТКАЯ ВСПЫШКА ШТАТНОГО СКАНЕРА.
// ВАЖНО: столбцы клавиатурной матрицы и столбцы подсветки - ЭТО ОДНИ
// И ТЕ ЖЕ ПИНЫ (A8-A15, B0-B5), и во время сна ими владеет PWM.
// Читать их руками НЕЛЬЗЯ: получишь ноль на всех столбцах, то есть
// "нажаты все клавиши", и клавиатура будет просыпаться мгновенно после
// засыпания. Поэтому на несколько миллисекунд отдаём работу штатному
// сканеру и смотрим на его результат. Буфер кадра обнулён, поэтому
// диоды во время вспышки не загораются.
static void k530_scanner_burst(void) {
    shared_matrix_rgb_enable();
    wait_ms(K530_DEEP_SLEEP_SCAN_MS);
    k530_backlight_park();
}

// Любая нажатая клавиша по данным штатного сканера.
static bool k530_any_key_down(void) {
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        if (raw_matrix[r] != 0) return true;
    }
    return false;
}
#endif  // K530_DEEP_SLEEP_ENABLE

static void k530_sleep_enter(void) {
    if (k530_asleep) return;
    k530_rgb_was_on = rgb_matrix_is_enabled();
    if (k530_rgb_was_on) {
        // Гасим всю матрицу без записи в EEPROM (режим/яркость пользователя
        // не трогаем - после пробуждения всё вернётся как было).
        rgb_matrix_disable_noeeprom();
    }
    k530_asleep = true;
#if K530_DEEP_SLEEP_ENABLE
    // Обнуляем буфер кадра НАПРЯМУЮ, чтобы короткие вспышки сканера
    // не подсвечивали оставшийся в памяти кадр.
    SN32F24xB_set_color_all(0, 0, 0);
    k530_backlight_park();
#endif
}

static void k530_sleep_exit(void) {
    if (!k530_asleep) return;
    k530_asleep = false;
#if K530_DEEP_SLEEP_ENABLE
    shared_matrix_rgb_enable();   // запускаем "мотор" насовсем
#endif
    if (k530_rgb_was_on) {
        rgb_matrix_enable_noeeprom();
    }
    k530_note_activity();
}

// Вызывается из matrix_scan_user() на каждом цикле сканирования.
static void k530_idle_task(void) {
    // Гейт по зарядке: кабель вставлен -> счётчик не идёт вообще,
    // а если клавиатура уже спала - пробуждаемся (точно как в стоке,
    // где изменение пина зарядки - второй источник пробуждения).
    if (k530_bluetooth_is_charging()) {
        if (k530_asleep) k530_sleep_exit();
        k530_note_activity();
        return;
    }

    if (!k530_asleep) {
        if (timer_elapsed32(k530_last_activity) >= K530_IDLE_SLEEP_MS) {
            k530_sleep_enter();
        }
        return;
    }

#if K530_DEEP_SLEEP_ENABLE
    // Самолечение на случай, если PWM-прерывание включило себя обратно.
    k530_backlight_park();

    // Вспышка сканера и проверка его результата.
    k530_scanner_burst();
    if (k530_any_key_down()) {
        k530_sleep_exit();
        return;
    }

    // Пауза отдаёт время ядру: ChibiOS уводит процессор в режим пониженного
    // потребления до следующего события. BT-модуль при этом продолжает
    // опрашиваться (bluetooth_task), связь и USB не рвутся -> рескан в OpenRGB не нужен.
    wait_ms(K530_DEEP_SLEEP_POLL_MS);
#elif K530_IDLE_SLEEP_STOP_CPU
    // Старое поведение: просто ждём следующего прерывания.
    __asm volatile ("wfi");
#endif
}
#endif  // K530_IDLE_SLEEP_ENABLE

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Любое событие клавиши = активность + пробуждение.
    // НАЖАТИЕ НЕ ГЛОТАЕМ: клавиша, которой ты будишь клавиатуру,
    // всё равно уйдёт на хост. Если захочешь «первое нажатие только
    // будит» — скажи, добавлю `if (was_asleep) return false;`.
    k530_note_activity();
#if K530_IDLE_SLEEP_ENABLE
    if (k530_asleep) k530_sleep_exit();
#endif

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
    keycode != MACRO_PLAY1 && keycode != MACRO_PLAY2 && keycode != MACRO_PLAY3 && keycode != MACRO_PLAY4 && keycode != MACRO_PLAY5 && keycode != MACRO_PLAY6 &&
    keycode != BT_UNPAIR) {
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

        case BT_UNPAIR:
            // Fn2 + ESC: сброс сопряжения по УДЕРЖАНИЮ 3 сек (как в стоке).
            // Здесь только запускаем/сбрасываем таймер удержания; сам сброс
            // (disconnect + re-advertise) вызывается из matrix_scan_user(),
            // когда клавиша продержана BT_UNPAIR_HOLD_MS. Быстрое нажатие —
            // ничего не делает, чтобы случайно не уронить линк.
            if (record->event.pressed) {
                bt_unpair_held  = true;
                bt_unpair_fired = false;
                bt_unpair_press_time = timer_read();
            } else {
                bt_unpair_held = false;
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
                    // На проводе (BT off) проигрываем через USB; при BT on USB
                    // подавляем — макрос уходит только в BT (через bt_track ниже).
                    if (!k530_bluetooth_is_enabled()) {
                        if (macro_buf[slot - 1][i].pressed) {
                            register_code16(macro_buf[slot - 1][i].keycode);
                        } else {
                            unregister_code16(macro_buf[slot - 1][i].keycode);
                        }
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

    // ВЗАИМОБЛОКИРОВКА USB/BT (как на стоке): когда тумблер BT в ON, базовые
    // клавиши (keycode <= 0xFF: буквы/цифры/модификаторы) уходят ТОЛЬКО по
    // Bluetooth — USB-отчёт подавляем (return false), «провод» становится немым.
    // Составные/спец-коды (LT/MO/RESET/RGB_TOG/кастомные, keycode > 0xFF)
    // пропускаем в QMK всегда, иначе сломаются слои и служебные клавиши.
    // При BT off (тумблер OFF) — всё как обычно, провод активен.
    if (k530_bluetooth_is_enabled() && keycode <= 0xFFu) {
        return false;
    }
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

#if K530_IDLE_SLEEP_ENABLE
    k530_idle_task();   // счётчик бездействия / сон / пробуждение
#endif

    // Fn2+ESC удержано >= 3 сек -> сброс сопряжения (disconnect + re-advertise),
    // как в стоке. Срабатывает один раз за удержание.
    if (bt_unpair_held && !bt_unpair_fired &&
        timer_elapsed(bt_unpair_press_time) >= BT_UNPAIR_HOLD_MS) {
        bt_unpair_fired = true;
        k530_send_bt_unpair();
        bt_unpair_feedback_time = timer_read32();  // запустить окно подсветки кандидата
    }
}

// ---------------------------------------------------------------------
// LED-индикация:
// 1. Подсветка клавиш активного слоя (Fn1 / Fn2 / Magic Fn)
// 2. Активная запись макрос�� (белым)
// 3. Статус слотов макросов на Magic Fn (зелёный/красный)
// 4. Реальный CapsLock (красным)
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// LED-индикация (advanced-версия — вызывается на каждый обрабатываемый
// пакет LED с диапазоном led_min/led_max, что убирает мерцание от
// гонки с батчевой отрисовкой базового эффекта на медленном MCU).
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ВСПОМОГАТЕЛЬНОЕ ДЛЯ ИНДИКАТОРА БАТАРЕИ
// ---------------------------------------------------------------------
// «Дыхание»: треугольная волна, период 2 с, нижний порог ~10% яркости
// (чтобы светодиод не гас полностью — видно, что идёт заряд).
#define K530_BREATH_PERIOD_MS 2000u
#define K530_BREATH_FLOOR     26u    // минимальная яркость (0..255)

static uint8_t k530_breath_level(void) {
    uint16_t phase = (uint16_t)(timer_read32() % K530_BREATH_PERIOD_MS);
    uint16_t half  = K530_BREATH_PERIOD_MS / 2u;
    uint16_t tri   = (phase < half) ? phase : (uint16_t)(K530_BREATH_PERIOD_MS - phase); // 0..half
    // важно: счёт в uint32_t — tri*(255-floor) не влезает в uint16_t
    return (uint8_t)(K530_BREATH_FLOOR +
                     ((uint32_t)tri * (255u - K530_BREATH_FLOOR)) / half);
}

// Залить светодиод цветом, промасштабированным по яркости 0..255.
static void k530_set_color_scaled(uint8_t led, uint8_t r, uint8_t g, uint8_t b, uint8_t level) {
    rgb_matrix_set_color(led,
                         (uint8_t)(((uint32_t)r * level) / 255u),
                         (uint8_t)(((uint32_t)g * level) / 255u),
                         (uint8_t)(((uint32_t)b * level) / 255u));
}

#define K530_BATT_LED 61
#define K530_HOST_LED 62

// ---------------------------------------------------------------------
// ИНДИКАТОР БАТАРЕИ (LED 61)
// ---------------------------------------------------------------------
// Работает НЕЗАВИСИМО от тумблера BT: факт зарядки берётся с
// локального пина P1.14, а не из кадра BT-модуля — в стоке этот
// светодиод тоже светит при выключенном BT.
//
//  зарядка подключена:  FULL        -> зелёный ровно
//                        норма       -> оранжевое дыхание
//                        LOW/CRIT    -> красное дыхание
//                        нет данных -> оранжевое дыхание (заряд всё равно идёт)
//  от батареи:           FULL        -> зелёный ровно
//                        норма       -> оранжевый ровно
//                        LOW         -> красный ровно
//                        CRIT        -> красный мига��т + сирена (п. 7)
//                        нет данных -> погашен
//
//  +-------------------------------------------------------------------+
//  | ЕСЛИ НИЗКИЙ И КРИТИЧЕСКИЙ ПЕРЕПУТАНЫ — поменяй местами  |
//  | ЗНАЧЕНИЯ K530_BATT_LOW и K530_BATT_CRIT в файле bluetooth.h.     |
//  | Сейчас: LOW = 0x01, CRIT = 0x02 (гипотеза, на железе не       |
//  | проверено). Больше нигде менять ничего не надо.          |
//  +-------------------------------------------------------------------+
static void k530_draw_batt_led(uint8_t led_min, uint8_t led_max) {
    if (K530_BATT_LED < led_min || K530_BATT_LED >= led_max) return;

    bool    charging = k530_bluetooth_is_charging();      // пин P1.14
    bool    have     = k530_bluetooth_status_valid();     // был ли хоть один кадр A7
    uint8_t lvl      = k530_bluetooth_battery_level();    // A7[5]
    uint8_t breath   = k530_breath_level();
    bool    blink    = (timer_read32() % 700u) < 350u;

    if (charging) {
        if (have && lvl == K530_BATT_FULL) {
            rgb_matrix_set_color(K530_BATT_LED, 0, 255, 0);            // заряжено — зелёный ровно
        } else if (have && (lvl == K530_BATT_LOW || lvl == K530_BATT_CRIT)) {
            k530_set_color_scaled(K530_BATT_LED, 255, 0, 0, breath);   // низкий/критический — красное дыхание
        } else {
            k530_set_color_scaled(K530_BATT_LED, 255, 80, 0, breath);  // идёт заряд — оранжевое дыхание
        }
    } else {
        if (!have) {
            rgb_matrix_set_color(K530_BATT_LED, 0, 0, 0);              // нет данных об уровне — лучше погасить
        } else if (lvl == K530_BATT_FULL) {
            rgb_matrix_set_color(K530_BATT_LED, 0, 255, 0);            // полный — зелёный ровно
        } else if (lvl == K530_BATT_CRIT) {
            rgb_matrix_set_color(K530_BATT_LED, blink ? 255 : 0, 0, 0); // критический — красное мигание
        } else if (lvl == K530_BATT_LOW) {
            rgb_matrix_set_color(K530_BATT_LED, 255, 0, 0);            // низкий — красный ровно
        } else {
            rgb_matrix_set_color(K530_BATT_LED, 255, 80, 0);           // норма — оранжевый ровно
        }
    }
}

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

    // 6. Индикаторы у тумблера (индексы 61/62, позиции из рабочего конфига
    //    Steryy/k530: row4 col3/col4). Ставим ПОСЛЕДНИМИ — перекрывают эффект/OpenRGB.
    //      LED 62 = состояние линка BT-модуля (из A7-кадра, байт[4]);
    //      LED 61 = батарея (из A7-кадра, байт[7]).
    if (k530_bluetooth_is_enabled()) {
        // Мигание ~1.4 Гц (350 мс вкл / 350 мс выкл).
        bool blink = (timer_read32() % 700u) < 350u;

        // LED 62 — состояние линка (A7[4]):
        //   0x03 connected -> зелёный РОВНО
        //   0x01 bonded    -> зелёный МИГАЕТ
        //   0x02 advertise -> синий  МИГАЕТ
        //   0x00 idle      -> красный МИГАЕТ
        if (K530_HOST_LED >= led_min && K530_HOST_LED < led_max) {
            // Первые BT_UNPAIR_FEEDBACK_MS после Fn2+ESC — ПОДТВЕРЖДЕНИЕ отправки
            // команды сброса бонда: этот же светодиод состояния мигает КРАСНЫМ.
            if (bt_unpair_feedback_time != 0 &&
                (timer_read32() - bt_unpair_feedback_time) < BT_UNPAIR_FEEDBACK_MS) {
                rgb_matrix_set_color(K530_HOST_LED, blink ? 255 : 0, 0, 0);  // красное мигание = «сброс отправлен»
            } else {
                uint8_t link = k530_bluetooth_link_state();
                uint8_t r = 0, g = 0, b = 0;
                switch (link) {
                    case 0x03u:              g = 255; break;   // connected — ровно
                    case 0x01u: if (blink) { g = 255; } break; // bonded — мигает
                    case 0x02u: if (blink) { b = 255; } break; // advertising — мигает
                    case 0x00u:
                    default:    if (blink) { r = 255; } break; // idle — мигает
                }
                rgb_matrix_set_color(K530_HOST_LED, r, g, b);
            }
        }

        // Батарея (LED 61) рисуется ниже — ВНЕ этого if, чтобы работать
        // и при тумблере BT в OFF (факт зарядки — локальный пин, не BT).
    } else {
        // Тумблер BT в OFF — гасим только индикатор линка (62).
        if (K530_HOST_LED >= led_min && K530_HOST_LED < led_max) rgb_matrix_set_color(K530_HOST_LED, 0, 0, 0);
    }

    // 6b. Индикатор батареи/зарядки — ВСЕГДА, независимо от тумблера BT.
    k530_draw_batt_led(led_min, led_max);

    // =================================================================
    // 7. «СИРЕНА» — критический заряд и работа ОТ БАТАРЕИ.
    // Раз в 10 с вся клавиатура даёт два красных импульса:
    //   0..100 мс      — красный
    //   100..600 мс    — обычная анимация (пауза 500 мс)
    //   600..700 мс    — красный
    //   700..10000 мс  — обычная анимация
    // Ставится САМЫМ ПОСЛЕДНИМ — перекрывает и эффекты RGB matrix,
    // и direct-режим OpenRGB, и подсветку слоя, и реакцию на нажатия.
    // Трогаем ТОЛЬКО клавишные светодиоды 0..60; индикаторы 61/62 не
    // трога��м. Ничего не сохраняем/не восстанавливаем — просто
    // перестаём перекрывать, и следующий кадр рисует текущий режим.
    // Если срабатывает не на том уровне — см. летку об обмене
    // K530_BATT_LOW / K530_BATT_CRIT в bluetooth.h.
    // =================================================================
    #define K530_SIREN_PERIOD_MS 10000u
    #define K530_SIREN_PULSE_MS  100u    // длительность одного импульса
    #define K530_SIREN_GAP_MS    500u    // пауза между импульсами
    if (k530_bluetooth_status_valid() &&
        !k530_bluetooth_is_charging() &&
        k530_bluetooth_battery_level() == K530_BATT_CRIT) {

        uint32_t t     = timer_read32() % K530_SIREN_PERIOD_MS;
        uint32_t p2beg = K530_SIREN_PULSE_MS + K530_SIREN_GAP_MS;          // 600
        uint32_t p2end = p2beg + K530_SIREN_PULSE_MS;                      // 700
        bool     pulse = (t < K530_SIREN_PULSE_MS) || (t >= p2beg && t < p2end);

        if (pulse) {
            for (uint8_t i = led_min; i < led_max && i < K530_BATT_LED; i++) {
                rgb_matrix_set_color(i, 255, 0, 0);
            }
        }
    }
}