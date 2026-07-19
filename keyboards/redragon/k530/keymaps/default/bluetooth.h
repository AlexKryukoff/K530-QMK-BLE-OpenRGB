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
