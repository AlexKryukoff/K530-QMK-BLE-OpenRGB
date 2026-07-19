#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Скан №2: ВСЕ периферийные адреса в дампе, без априорной карты."""
import struct, collections

with open("1RCData4000.bin","rb") as f:
    data = f.read()

# Собираем все 32-битные LE-слова, попадающие в периферийную область 0x4000xxxx-0x4006xxxx
periph = collections.Counter()
periph_off = collections.defaultdict(list)
for off in range(0, len(data)-3):
    val = struct.unpack_from("<I", data, off)[0]
    if 0x40000000 <= val <= 0x40070000:
        # выровненность по 4 — признак настоящего адреса
        periph[val] += 1
        periph_off[val].append(off)

# Группируем по блокам 0x1000
blocks = collections.Counter()
for addr in periph:
    blocks[addr & 0xFFFFF000] += periph[addr]

print("Периферийные БЛОКИ (по 0x1000), встречающиеся в дампе:")
for blk, n in sorted(blocks.items()):
    print(f"  0x{blk:08X}: {n} попаданий")

print("\nДетально: все периферийные адреса (>=2 попаданий), отсортированы по адресу:")
for addr in sorted(periph):
    n = periph[addr]
    if n < 1: continue
    offs = periph_off[addr]
    print(f"  0x{addr:08X}: {n}x  off0={hex(offs[0])} {'(выровнен)' if addr%4==0 else '(НЕвыровнен!)'}")

# Отдельно: конкретные UART-кандидаты
print("\nКандидаты на UART-блоки (ищем 0x40010000/0x40012000/0x40016000):")
for target in [0x40010000,0x40012000,0x40016000,0x40056000,0x40052000]:
    n = periph.get(target,0)
    print(f"  0x{target:08X}: {n} попаданий")
