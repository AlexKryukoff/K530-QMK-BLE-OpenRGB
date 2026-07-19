#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Найти все LDR, которые грузят конкретное literal-значение (xref-поиск),
и показать функцию-обёртку.
"""
import struct, re
from capstone import Cs, CS_ARCH_ARM, CS_MODE_THUMB

with open("1RCData4000.bin","rb") as f:
    data = f.read()
md = Cs(CS_ARCH_ARM, CS_MODE_THUMB)
md.detail = True

def thumb_decode_literals(func_start, func_end):
    """Вернуть словарь literal_addr -> (loaded_value, ldr_addr)."""
    code = data[func_start:func_end]
    lits = {}
    for ins in md.disasm(code, func_start):
        if ins.mnemonic.startswith("ldr") and "[pc" in ins.op_str:
            pc = (ins.address & ~3) + 4
            m = re.search(r"#(0x[0-9a-fA-F]+|\d+)\]", ins.op_str)
            if not m: continue
            imm = int(m.group(1),0)
            lit_addr = pc + imm
            if lit_addr+4 <= len(data):
                val = struct.unpack_from("<I", data, lit_addr)[0]
                lits[lit_addr] = (val, ins.address)
    return lits

def find_func_start(addr, max_back=0x200):
    """Идти назад, искать push {..., lr} или bxx — типичное начало функции."""
    # начало функции в Thumb часто начинается с push {lr, ...} = 0xB5xx
    # или sub sp / mov
    for back in range(2, max_back, 2):
        a = addr - back
        if a < 0: break
        hw = struct.unpack_from("<H", data, a)[0]
        # push {..., lr}: opcode 1011 010x xxxx xxxx
        if (hw & 0xFF00) == 0xB500:
            return a
    return addr

def xref_value(target_val):
    """Найти все LDR, грузящие target_val."""
    results = []
    # ищем literal pool вхождения target_val
    lit_addrs = []
    for off in range(0, len(data)-3):
        if struct.unpack_from("<I", data, off)[0] == target_val:
            lit_addrs.append(off)
    # теперь ищем LDR [pc,#imm] где pc+imm == lit_addr
    # для каждого возможного ldr-адрата
    for la in lit_addrs:
        # ldr может быть в диапазоне la-1020 .. la-4 (thumb imm ограничен 0xFFC, ~±1KB вперёд)
        for ldr_off in range(max(0,la-1024), la):
            if ldr_off % 2: continue
            hw = struct.unpack_from("<H", data, ldr_off)[0]
            # thumb-16 LDR (literal): 01001 Rt:3 imm8 ; opcode bits 15:11 = 01001
            if (hw >> 11) == 0b01001:
                imm8 = hw & 0xFF
                pc = (ldr_off & ~3) + 4
                if pc + imm8*4 == la:
                    results.append((ldr_off, la))
    return results

def show_func(func_start, length=0x80, label=""):
    print(f"\n----- {label} func@0x{func_start:X} -----")
    code = data[func_start:func_start+length]
    for ins in md.disasm(code, func_start):
        ann = ""
        if ins.mnemonic.startswith("ldr") and "[pc" in ins.op_str:
            pc = (ins.address & ~3) + 4
            m = re.search(r"#(0x[0-9a-fA-F]+|\d+)\]", ins.op_str)
            if m:
                imm = int(m.group(1),0)
                la = pc + imm
                if la+4 <= len(data):
                    val = struct.unpack_from("<I", data, la)[0]
                    ann = f"  ; =0x{val:08X}"
                    if 0x40000000 <= val <= 0x40070000: ann += " <PERIPH>"
                    elif 0x20000000 <= val <= 0x20002000: ann += " <SRAM>"
        # стоп на pop pc / bx lr
        print(f"  0x{ins.address:05X}: {ins.bytes.hex():12s} {ins.mnemonic:8s} {ins.op_str}{ann}")
        if ins.mnemonic == "pop" and "pc" in ins.op_str: break
        if ins.mnemonic == "bx" and ins.op_str=="lr": break

# === PFPA 0x40042000 ===
print("="*70)
print("XREF на PFPA (0x40042000):")
print("="*70)
xrefs = xref_value(0x40042000)
print(f"Найдено xref'ов: {len(xrefs)}")
for ldr_off, la in xrefs:
    fs = find_func_start(ldr_off)
    show_func(fs, 0xC0, label=f"PFPA-xref (ldr@0x{ldr_off:X} lit@0x{la:X})")

# === UART0 0x40010000 ===
print("\n" + "="*70)
print("XREF на UART0 (0x40010000):")
print("="*70)
xrefs = xref_value(0x40010000)
print(f"Найдено xref'ов: {len(xrefs)}")
for ldr_off, la in xrefs:
    fs = find_func_start(ldr_off)
    show_func(fs, 0xC0, label=f"UART0-xref (ldr@0x{ldr_off:X} lit@0x{la:X})")
