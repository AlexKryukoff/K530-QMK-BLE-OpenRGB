#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Найти НАСТОЯЩИЕ обращения к GPIO0/GPIO1/GPIO2/GPIO3 через literal-pool xref'ы.
Отфильтровать артефакты (случайные склейки байт).
Затем дизассемблировать функции, обращающиеся к GPIO0 (BT-линии).
"""
import os, struct, re, collections
from capstone import Cs, CS_ARCH_ARM, CS_MODE_THUMB

BIN = os.path.join(os.path.dirname(os.path.abspath(__file__)), "1RCData4000.bin")
with open(BIN,"rb") as f:
    data = f.read()
md = Cs(CS_ARCH_ARM, CS_MODE_THUMB)
md.detail = True

GPIO_BASES = {0x40044000:"GPIO0 (P0.x — BT-линии)",
              0x40046000:"GPIO1 (матрица)",
              0x40048000:"GPIO2",
              0x4004A000:"GPIO3"}

# GPIO0 register map (из UM F245, совместим с VS11K09A)
GPIO_REG = {0x00:"DATA",0x04:"MODE",0x08:"CFG",0x0C:"CFG2",
            0x10:"IS",0x14:"IBS",0x18:"IEV",0x1C:"IE",
            0x20:"RIS",0x24:"IC",0x28:"BSET",0x2C:"BCLR",0x30:"ODCTRL"}

def xref_value(target):
    """Все LDR [pc,#imm], ссылающиеся на literal, содержащий target."""
    out=[]
    for la in range(0,len(data)-3):
        if struct.unpack_from("<I",data,la)[0]!=target: continue
        # ищем ldr в диапазоне [la-0x400, la)
        for lo in range(max(0,la-0x400), la, 2):
            hw=struct.unpack_from("<H",data,lo)[0]
            if (hw>>11)==0b01001:  # ldr Rt,[pc,#imm8*4]
                imm8=hw&0xFF
                pc=(lo&~3)+4
                if pc+imm8*4==la:
                    out.append((lo,la))
    return out

def find_func_start(addr, max_back=0x300):
    for back in range(0,max_back,2):
        a=addr-back
        if a<0: break
        hw=struct.unpack_from("<H",data,a)[0]
        if (hw&0xFF00)==0xB500:  # push {lr,...}
            return a
    return addr

def show_func(fs, length=0x120, label=""):
    print(f"\n----- {label} func@0x{fs:X} -----")
    code=data[fs:fs+length]
    for ins in md.disasm(code,fs):
        ann=""
        if ins.mnemonic.startswith("ldr") and "[pc" in ins.op_str:
            pc=(ins.address&~3)+4
            m=re.search(r"#(0x[0-9a-fA-F]+|\d+)\]",ins.op_str)
            if m:
                imm=int(m.group(1),0); la=pc+imm
                if la+4<=len(data):
                    val=struct.unpack_from("<I",data,la)[0]
                    ann=f"  ; =0x{val:08X}"
                    for b,nm in GPIO_BASES.items():
                        if b<=val<b+0x1000:
                            off=val-b; rn=GPIO_REG.get(off,f"?{off:02X}")
                            ann+=f" <{nm} {rn}>"
                            break
                    else:
                        if 0x40000000<=val<=0x40070000: ann+=" <PERIPH>"
                        elif 0x20000000<=val<0x20002000: ann+=" <SRAM>"
        print(f"  0x{ins.address:05X}: {ins.bytes.hex():12s} {ins.mnemonic:8s} {ins.op_str}{ann}")
        if ins.mnemonic=="pop" and "pc" in ins.op_str: break
        if ins.mnemonic=="bx" and ins.op_str=="lr": break

# === Найти xref'ы к GPIO0 base и всем его регистрам ===
print("="*72)
print("XREF на GPIO-блоки (реальные обращения через LDR):")
print("="*72)
gpio0_xrefs=[]
for base,nm in GPIO_BASES.items():
    # ищем все 4-байтные literal'ы, попадающие в блок base..base+0x40
    lit_hits=collections.Counter()
    for off in range(0,len(data)-3):
        val=struct.unpack_from("<I",data,off)[0]
        if base<=val<base+0x40 and val%4==0:  # выровненный адрес регистра
            lit_hits[val]+=1
    print(f"\n{nm} (0x{base:08X}):")
    for val in sorted(lit_hits):
        off=val-base; rn=GPIO_REG.get(off,f"?{off:02X}")
        n=lit_hits[val]
        xrefs=xref_value(val)
        print(f"  0x{val:08X} +0x{off:02X} {rn:8s}: literal={n}x, xref-ldr={[hex(l) for l,_ in xrefs]}")
        if base==0x40044000:
            gpio0_xrefs.extend(xrefs)

# === Дизассемблировать все функции, обращающиеся к GPIO0 ===
print("\n"+"="*72)
print(f"ФУНКЦИИ, обращающиеся к GPIO0 (BT-линии): {len(gpio0_xrefs)} xref'ов")
print("="*72)
seen_funcs=set()
for lo,la in gpio0_xrefs:
    fs=find_func_start(lo)
    if fs in seen_funcs: continue
    seen_funcs.add(fs)
    show_func(fs, 0x150, label=f"GPIO0-доступ (ldr@0x{lo:X})")
