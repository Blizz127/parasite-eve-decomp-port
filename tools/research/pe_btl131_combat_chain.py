#!/usr/bin/env python3
"""BTL131 — trace D20C body → targeting → tid406 → combat chain."""
import struct

with open('build/extracted/disc1/SLUS_006.62', 'rb') as f:
    data = f.read()

text = data[0x800:]
va_base = 0x80010000

def u32(va):
    off = va - va_base
    return struct.unpack_from('<I', text, off)[0] if 0 <= off < len(text) else None

R = ['zero','at','v0','v1','a0','a1','a2','a3','t0','t1','t2','t3','t4','t5','t6','t7',
     's0','s1','s2','s3','s4','s5','s6','s7','t8','t9','k0','k1','gp','sp','fp','ra']

def disasm(word, va):
    if word is None: return '???'
    op = (word >> 26) & 0x3F
    rs = (word >> 21) & 0x1F
    rt = (word >> 16) & 0x1F
    rd = (word >> 11) & 0x1F
    funct = word & 0x3F
    imm16 = word & 0xFFFF
    imm16s = struct.unpack_from('<h', struct.pack('<H', imm16))[0]
    imm26 = word & 0x03FFFFFF
    if op == 0x00:
        if funct == 0x21: return f'addu {R[rd]},{R[rs]},{R[rt]}'
        if funct == 0x25: return f'or {R[rd]},{R[rs]},{R[rt]}'
        if funct == 0x24: return f'and {R[rd]},{R[rs]},{R[rt]}'
        if funct == 0x2A: return f'slt {R[rd]},{R[rs]},{R[rt]}'
        if funct == 0x00: return f'sll {R[rd]},{R[rt]},{(word>>6)&0x1F}'
        if funct == 0x08: return f'jr {R[rs]}'
        if funct == 0x09: return f'jalr {R[rd]},{R[rs]}'
        if funct == 0x23: return f'subu {R[rd]},{R[rs]},{R[rt]}'
        if funct == 0x2B: return f'sltu {R[rd]},{R[rs]},{R[rt]}'
        if funct == 0x18: return f'mult {R[rs]},{R[rt]}'
        if funct == 0x12: return f'mflo {R[rd]}'
        if funct == 0x10: return f'mfhi {R[rd]}'
        return f'special_0x{funct:02X}'
    elif op == 0x02: return f'j 0x{(va&0xF0000000)|(imm26<<2):08X}'
    elif op == 0x03: return f'jal 0x{(va&0xF0000000)|(imm26<<2):08X}'
    elif op == 0x04: return f'beq {R[rs]},{R[rt]},0x{va+4+(imm16s<<2):08X}'
    elif op == 0x05: return f'bne {R[rs]},{R[rt]},0x{va+4+(imm16s<<2):08X}'
    elif op == 0x06: return f'blez {R[rs]},0x{va+4+(imm16s<<2):08X}'
    elif op == 0x07: return f'bgtz {R[rs]},0x{va+4+(imm16s<<2):08X}'
    elif op == 0x08: return f'addi {R[rt]},{R[rs]},{imm16s}'
    elif op == 0x09: return f'addiu {R[rt]},{R[rs]},{imm16s}'
    elif op == 0x0C: return f'andi {R[rt]},{R[rs]},0x{imm16:04X}'
    elif op == 0x0D: return f'ori {R[rt]},{R[rs]},0x{imm16:04X}'
    elif op == 0x0E: return f'xori {R[rt]},{R[rs]},0x{imm16:04X}'
    elif op == 0x0F: return f'lui {R[rt]},0x{imm16:04X}'
    elif op == 0x1C: return f'special2_0x{funct:02X}'
    elif op == 0x20: return f'lb {R[rt]},{imm16s}({R[rs]})'
    elif op == 0x21: return f'lh {R[rt]},{imm16s}({R[rs]})'
    elif op == 0x23: return f'lw {R[rt]},{imm16s}({R[rs]})'
    elif op == 0x24: return f'lbu {R[rt]},{imm16s}({R[rs]})'
    elif op == 0x25: return f'lhu {R[rt]},{imm16s}({R[rs]})'
    elif op == 0x28: return f'sb {R[rt]},{imm16s}({R[rs]})'
    elif op == 0x29: return f'sh {R[rt]},{imm16s}({R[rs]})'
    elif op == 0x2B: return f'sw {R[rt]},{imm16s}({R[rs]})'
    return f'op_0x{op:02X}'

def dump_func(addr, name, limit=80):
    print(f'\n=== {name} ===')
    for i in range(addr, addr + limit, 4):
        w = u32(i)
        if w is None: break
        print(f'  0x{i:08X}: {w:08X} {disasm(w, i)}')
        if (w & 0x3F) == 0x08 and (w >> 26) == 0 and i > addr:
            print(f'  [jr ra — end]')
            break

# 0x6F handler (body creation)
dump_func(0x8002F7D8, 'func_8002F7D8 (0x6F — enemy body creation)', 100)

# 5C498 (targeting)
dump_func(0x8005C498, 'func_8005C498 (targeting)', 80)

# 26824 (tid publisher — from BTL121)
dump_func(0x80026824, 'func_80026824 (tid publisher)', 80)

# 512AC (record writer)
dump_func(0x800512AC, 'func_800512AC (record writer)', 60)

# Search for sh/sw to gp+0x534 (D2A4 — targeting return)
print('\n=== Stores to gp+0x534 (D2A4) ===')
count = 0
for i in range(0, len(text) - 4, 4):
    w = u32(i + va_base)
    op = (w >> 26) & 0x3F
    base = (w >> 21) & 0x1F
    if base == 28 and op in (0x29, 0x2B):  # sh or sw to gp
        imm16 = w & 0xFFFF
        imm16s = struct.unpack_from('<h', struct.pack('<H', imm16))[0]
        if imm16s == 0x534:
            va = va_base + i
            rt = (w >> 16) & 0x1F
            stype = 'sh' if op == 0x29 else 'sw'
            print(f'  0x{va:08X}: {stype} {R[rt]},0x534(gp)  ({disasm(w, va)})')
            count += 1
            if count >= 15:
                break

# Search for 5E30C references (from BTL112 — the 5C498 result handler)
print('\n=== func_8005E30C (5C498 result dispatch) ===')
dump_func(0x8005E30C, 'func_8005E30C', 60)
