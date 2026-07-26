/*
 * bluetooth.h — публичный интерфейс к драйверу bluetooth.c для K530.
 *
 * bluetooth.c компилируется отдельной единицей трансляции (см. SRC += в
 * rules.mk), поэтому эти четыре функции — единственное, что видно снаружи.
 * Всё остальное (регистры SPI0/GPIO, состояние тумблеров, ISR) остаётся
 * static внутри bluetooth.c, инкапсуляция не нарушается.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "report.h" // report_keyboard_t

// Вызвать один раз при старте прошивки (из keyboard_post_init_user()).
void bluetooth_init(void);

// Вызывать периодически из главного цикла (из matrix_scan_user()) —
// опрашивает оба тумблера с debounce, включает/выключает SPI0-транспорт
// при смене положения тумблера BT on/off.
void bluetooth_task(void);

// Отправить текущий 6-key HID-отчёт по Bluetooth (если физически включено
// тумблером; если выключено — функция сама тихо ничего не делает).
void bluetooth_send_keyboard(report_keyboard_t *report);

// Геттеры для интеграции с keymap.c (например, для RGB-индикации режима
// BT/USB или для отладочного вывода в консоль).
bool k530_bluetooth_is_enabled(void);
uint8_t k530_bluetooth_host_slot(void); // 0=неизвестно, 1/2/3=слот
uint8_t k530_bluetooth_battery(void);    // заряд % (0..100) из A7-кадра, байт[7]
uint8_t k530_bluetooth_link_state(void); // A7[4]: 00 idle/01 bonded/02 adv/03 connected

// Сброс сопряжения / режим повторной привязки (Fn2+ESC, удержание 3 сек):
// разрывает линк с текущим хостом и снова начинает рекламироваться, чтобы
// подключился новый девайс (реализовано подтверждёнными A5 disconnect+advertise;
// см. подробности в bluetooth.c, раздел 4В). Тихо ничего не делает, если BT
// выключен тумблером или SPI0 не готов.
void k530_send_bt_unpair(void);
