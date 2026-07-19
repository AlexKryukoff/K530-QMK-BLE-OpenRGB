#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Анализатор дампа прошивки SN32F248BF (ARM Cortex-M0).
Скрипт 1: сканирование периферийных адресов в literal pools + дизассемблирование.
"""
import struct, re, sys, collections
from capstone import Cs, CS_ARCH_ARM, CS_MODE_THUMB

BIN = "1RCData4000.bin"
BASE = 0x00000000
FLASH_TOP = 0x00010000   # 64KB

with open(BIN, "rb") as f:
    data = f.read()
print(f"Размер дампа: {len(data)} байт")

# ---- Карта периферии SN32F248 (из SN32F245 V2.1 User's Manual) ----
PERIPH = {
    0x40000000: "CT16B0", 0x40000800:"CT16B1", 0x40001000:"CT16B2",
    0x40006000: "CT32B0", 0x40006800:"CT32B1", 0x40007000:"CT32B2",
    0x40010000: "SSP0/UART0?", 0x40012000:"SSP1?",
    0x40016000: "USART0",       # RX/TX к BT-модулю (кандидат)
    0x40018000: "I2C0",
    0x4001A000: "USART-ctrl?",
    0x4001C000: "SSP0",
    0x4001E000: "?",
    0x40026000: "PWM?",
    0x40032000: "LCD?",
    0x40034000: "ADC?",
    0x40042000: "PFPA",         # назначение пинов под UART
    0x40044000: "GPIO0 (P0.x)", # ← BT-линии P0.1/P0.2/P0.3/P0.4/P0.5
    0x40046000: "GPIO1 (P1.x)",
    0x40048000: "GPIO2 (P2.x)",
    0x4004A000: "GPIO3 (P3.x)",
    0x40056000: "USART1",
    0x4005C000: "SysCtrl?",
    0x4005E000: "PMU/Clock",
    0x40060000: "MISC/WDT",
    0x40062000: "FlashCtrl",
}
# GPIO register offsets
GPIO_REGS = {
    0x00:"DATA", 0x04:"MODE", 0x08:"CFG", 0x0C:"CFG2",
    0x10:"IS", 0x14:"IBS", 0x18:"IEV", 0x1C:"IE",
    0x20:"RIS", 0x24:"IC", 0x28:"BSET", 0x2C:"BCLR", 0x30:"ODCTRL",
}
USART0 = 0x40016000
USART_REGS = {
    0x00:"TH/DLL", 0x04:"IE/DLM", 0x08:"II", 0x0C:"LC",
    0x10:"??", 0x14:"LS", 0x18:"??", 0x1C:"??",
    0x20:"??", 0x24:"??", 0x28:"FD", 0x2C:"??",
    0x30:"??", 0x34:"RS485CTRL", 0x38:"RS485ADRMATCH", 0x3C:"RS485DLYV",
}

def classify(addr):
    """Определить, к какому регистру относится 32-битный адрес."""
    # периферия
    for base,name in PERIPH.items():
        if base <= addr < base+0x1000:
            off = addr - base
            reg = ""
            if "GPIO" in name and off in GPIO_REGS: reg = GPIO_REGS[off]
            elif "USART" in name and off in USART_REGS: reg = USART_REGS[off]
            return f"{name}+0x{off:02X}" + (f" ({reg})" if reg else "")
    if 0x20000000 <= addr < 0x20002000: return f"SRAM 0x{addr:08X}"
    if 0x00000000 <= addr < FLASH_TOP: return f"FLASH 0x{addr:08X}"
    return None

# ---- Скан: ищем все 32-битные little-endian слова, попадающие в периферию ----
print("\n" + "="*70)
print("СКАНИРОВАНИЕ: периферийные адреса в дампе (literal pools)")
print("="*70)
hits = collections.Counter()
periph_hits = collections.defaultdict(list)  # peripheral -> [(file_offset, addr)]
for off in range(0, len(data)-3):
    val = struct.unpack_from("<I", data, off)[0]
    c = classify(val)
    if c:
        # фильтр SRAM/FLASH — слишком много шума; оставим периферию + отдельные данные
        if c.startswith("SRAM") or c.startswith("FLASH"):
            continue
        hits[c] += 1
        periph_hits[val].append(off)

print("\nТоп периферийных регистров по частоте упоминания:")
for c, n in hits.most_common(40):
    print(f"  {n:4d}x  {c}")

# Детально: все попадания в USART0
print("\n" + "-"*70)
print(f"USART0 (0x{USART0:08X}) — ВСЕ обращения (offset в файле -> адрес -> рег):")
for addr in sorted(periph_hits):
    if USART0 <= addr < USART0+0x1000:
        c = classify(addr)
        offs = periph_hits[addr]
        # найдём xref: какая инструкция рядом грузит эту константу
        print(f"  0x{addr:08X} {c}: {len(offs)} попаданий, file-off={[hex(o) for o in offs[:8]]}")

# PFPA
print("\nPFPA (0x40042000) — назначение UART-пинов:")
for addr in sorted(periph_hits):
    if 0x40042000 <= addr < 0x40043000:
        c = classify(addr)
        print(f"  0x{addr:08X} {c}: {len(periph_hits[addr])} попаданий, off={[hex(o) for o in periph_hits[addr][:8]]}")

# GPIO0 (P0.x) — BT-линии
print("\nGPIO0 (0x40044000, порты P0.x — BT-линии) — обращения:")
for addr in sorted(periph_hits):
    if 0x40044000 <= addr < 0x40045000:
        c = classify(addr)
        print(f"  0x{addr:08X} {c}: {len(periph_hits[addr])} попаданий, off={[hex(o) for o in periph_hits[addr][:8]]}")
