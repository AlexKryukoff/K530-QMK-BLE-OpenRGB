#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Дизассемблер контекстов ключевых обращений к периферии."""
import struct
from capstone import Cs, CS_ARCH_ARM, CS_MODE_THUMB

with open("1RCData4000.bin","rb") as f:
    data = f.read()
md = Cs(CS_ARCH_ARM, CS_MODE_THUMB)
md.detail = True

def disasm_range(start_off, length, base_addr=None, label=""):
    """Дизассемблировать диапазон, с аннотацией периферийных адресов."""
    if base_addr is None:
        base_addr = start_off  # flash address = file offset (base 0)
    code = data[start_off:start_off+length]
    print(f"\n----- {label}  [file=0x{start_off:X}, addr=0x{base_addr:X}] -----")
    for ins in md.disasm(code, base_addr):
        ann = ""
        # аннотируем LDR с последующим комментарием адреса
        if ins.mnemonic.startswith("ldr") and "[pc" in ins.op_str:
            # вычислим target literal pool address
            # PC в thumb = addr+4 (с выравниванием)
            pc = (ins.address & ~3) + 4
            try:
                imm = int(ins.op_str.split('#')[-1].rstrip(']'),16) if '#' in ins.op_str else 0
            except:
                imm = 0
            lit_addr = pc + imm
            if lit_addr+4 <= len(data):
                val = struct.unpack_from("<I", data, lit_addr)[0]
                ann = f"  ; =0x{val:08X}"
                if 0x40000000 <= val <= 0x40070000:
                    ann += "  <PERIPH>"
        print(f"  0x{ins.address:05X}: {ins.bytes.hex():16s} {ins.mnemonic:8s} {ins.op_str}{ann}")
    # raw literal pool bytes в конце диапазона
    print(f"  [конец диапазона]")

# Ключевые смещения из scan_all.py:
# PFPA 0x981c (значение = что назначено под UART)
# UART0 0x40010000 попадания: off0=0x648 (6x)
# GPIO0 0x40044000: off0=0x684

print("="*70)
print("PFPA (0x40042000) — ЧТО пишется? (назначение UART-пинов)")
print("="*70)
disasm_range(0x981c - 0x40, 0x80, label="контекст вокруг PFPA hit @0x981C")

print("\n" + "="*70)
print("UART0 (0x40010000) — инициализация / TX")
print("="*70)
# Найдём все попадания 0x40010000 и дизассемблируем контекст каждого
val = 0x40010000
hits = []
for off in range(0, len(data)-3):
    if struct.unpack_from("<I", data, off)[0] == val:
        hits.append(off)
print(f"Все попадания 0x40010000 в файле: {[hex(h) for h in hits]}")
for h in hits:
    # ищем LDR PC-relative ссылающийся на h, в окрестности ±0x100
    disasm_range(h - 0x30, 0x60, label=f"literal@0x{h:X}")

print("\n" + "="*70)
print("GPIO0 DATA (0x40044000) — обращения к BT-линиям P0.x")
print("="*70)
val = 0x40044000
hits = []
for off in range(0, len(data)-3):
    if struct.unpack_from("<I", data, off)[0] == val:
        hits.append(off)
print(f"Все попадания 0x40044000: {[hex(h) for h in hits]}")
for h in hits[:6]:
    disasm_range(h - 0x30, 0x60, label=f"literal@0x{h:X}")
