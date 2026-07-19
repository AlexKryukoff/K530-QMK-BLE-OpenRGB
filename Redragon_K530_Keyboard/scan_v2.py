#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Скан дампа VS11K09A с ПРАВИЛЬНОЙ картой периферии (из реального datasheet).
Цель: определить, через какой блок (USART0/USART1/SSP0/SSP1) идёт связь с BT.
"""
import struct, collections

import os
BIN = os.path.join(os.path.dirname(os.path.abspath(__file__)), "1RCData4000.bin")
with open(BIN,"rb") as f:
    data = f.read()

# --- Реальная карта VS11K09A (из datasheet, стр. 11 memory map) ---
MAP = {
    0x40000000:"CT16B0", 0x40002000:"CT32B1", 0x40004000:"CT32B2",
    0x40006000:"CT16B1", 0x40008000:"WDT", 0x4000A000:"RTC",
    0x4000C000:"CT16B2?",
    0x40012000:"USART0",   # ← BT-кандидат
    0x40014000:"I2C0",
    0x4001A000:"SSP0",     # ← BT-кандидат (SPI)
    0x4001C000:"reserved",
    0x40026000:"ADC",
    0x40032000:"LCD",
    0x40034000:"PMU",
    0x40042000:"PFPA",
    0x40044000:"GPIO0", 0x40046000:"GPIO1", 0x40048000:"GPIO2", 0x4004A000:"GPIO3",
    0x40058000:"USART1", 0x4005A000:"SSP1",
    0x4005C000:"I2C1",
    0x4005E000:"USB",
    0x40060000:"SYS1", 0x40062000:"SYS0", 0x40064000:"FMC",
}
# Группируем адреса в дампе по периферийным блокам 0x1000
print("="*72)
print("ПЕРИФЕРИЯ В ДАМПЕ (по правильной карте VS11K09A)")
print("="*72)
blocks = collections.defaultdict(lambda: collections.Counter())  # base -> {addr:count}
for off in range(0, len(data)-3):
    val = struct.unpack_from("<I", data, off)[0]
    if 0x40000000 <= val <= 0x40070000:
        base = val & 0xFFFFF000
        if base in MAP:
            blocks[base][val] += 1

print(f"\n{'Блок':18s} {'адрес':12s} {'имя':10s} {'всего':>5s}  детали")
for base in sorted(blocks):
    name = MAP.get(base,"?")
    total = sum(blocks[base].values())
    # детали — какие конкретно смещения внутри блока
    detail = ", ".join(f"+0x{off:02X}({n})" for (a,n) in blocks[base].items()
                       if (a-base) < 0x100 and n>=1)
    print(f"0x{base:08X}  {name:10s} {total:>5d}  {detail[:80]}")

# === КЛЮЧЕВОЕ: USART/SSP блоки детально ===
print("\n"+"="*72)
print("UART / SPI блоки — кандидаты на связь с BT-модулем:")
print("="*72)
for base in [0x40012000, 0x40058000, 0x4001A000, 0x4005A000]:
    name = MAP.get(base,"?")
    if base not in blocks:
        print(f"\n0x{base:08X} ({name}): НЕ ИСПОЛЬЗУЕТСЯ в дампе (0 обращений)")
    else:
        print(f"\n0x{base:08X} ({name}): ИСПОЛЬЗУЕТСЯ")
        for val in sorted(blocks[base]):
            off = val-base
            n = blocks[base][val]
            print(f"   0x{val:08X} +0x{off:02X}: {n}x попаданий")

# === PFPA: ЧТО записано? ===
print("\n"+"="*72)
print("PFPA (0x40042000) — назначение пинов под UART/SPI")
print("="*72)
# значение 0x1F пишется в 0x40042000 — раскодируем
# Формат из UM: [3:0]=UTXD0 сел, [7:4]=URXD0 сел, [11:8]=UTXD1, [15:12]=URXD1
pfpavals = []
for off in range(0, len(data)-3):
    if struct.unpack_from("<I", data, off)[0] == 0x40042000:
        pfpavals.append(off)
print(f"Адрес PFPA в дампе встречается на off: {[hex(o) for o in pfpavals]}")

# Рядом со ссылкой ищем значение, которое STR'ится в PFPA
# Из find_pfpa: movs r0,#0x1f; ldr r1,[=PFPA]; str r0,[r1]
# 0x1F = 0001 1111
# Расшифровка: UTXD0[3:0]=1111(=15), URXD0[7:4]=0001(=1)... но надо по UM-таблице
print("\nЗначение 0x1F = 0b00011111")
print("  [3:0] UTXD0 sel = 0xF (15)")
print("  [7:4] URXD0 sel = 0x1 (1)")
# По UM таблица PFPA:
#   0000: P0.0 / P1.0 ... ; 0001: P0.7/P0.6 ; 0010: P0.12/P0.13 ...
# НО это для F245. Для VS11K09A таблица PFPA может отличаться!
