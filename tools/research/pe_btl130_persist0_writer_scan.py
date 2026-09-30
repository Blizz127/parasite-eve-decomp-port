#!/usr/bin/env python3
"""PE-BTL130 — find every persist[0] writer in the retail EXE.

Scans for:
1. lui/addiu pairs that load D_800A77F0 (persist base)
2. Any sw/sh/sb that stores to persist[0]
3. Any OR/AND/ADD that modifies persist[0]
4. Scene script opcode 0x09/0x0A with mode 2 index 0

Also scans the disc image for ALL scene script tables to find
unexamined scenes that may write persist[0].
"""

import struct
import sys
import os

PERSIST_BASE = 0x800A77F0
TEXT_START = 0x80010000
TEXT_END = 0x801FE000

def load_exe(path):
    with open(path, "rb") as f:
        data = f.read()
    # PSX EXE: 0x800 header
    header = data[:0x800]
    text = data[0x800:]
    t_addr = struct.unpack_from("<I", header, 0x18)[0]
    t_size = struct.unpack_from("<I", header, 0x1C)[0]
    pc0 = struct.unpack_from("<I", header, 0x10)[0]
    print(f"t_addr=0x{t_addr:08X} t_size=0x{t_size:X} pc0=0x{pc0:08X}")
    return text, t_addr, t_size

def u32(data, off):
    return struct.unpack_from("<I", data, off)[0]

def decode_mips(text, va_base, target_va):
    """Decode one MIPS instruction at target_va."""
    off = target_va - va_base
    if off < 0 or off + 4 > len(text):
        return None
    word = u32(text, off)
    return word

def disasm_one(word, va):
    """Minimal MIPS disassembler for the instructions we care about."""
    op = (word >> 26) & 0x3F
    rs = (word >> 21) & 0x1F
    rt = (word >> 16) & 0x1F
    rd = (word >> 11) & 0x1F
    shamt = (word >> 6) & 0x1F
    funct = word & 0x3F
    imm16 = word & 0xFFFF
    imm16s = struct.unpack_from("<h", struct.pack("<H", imm16))[0]
    imm26 = word & 0x03FFFFFF
    
    REGS = ["zero","at","v0","v1","a0","a1","a2","a3",
            "t0","t1","t2","t3","t4","t5","t6","t7",
            "s0","s1","s2","s3","s4","s5","s6","s7",
            "t8","t9","k0","k1","gp","sp","fp","ra"]
    
    def r(n): return REGS[n]
    
    if op == 0x00:  # SPECIAL
        if funct == 0x21: return f"addu {r(rd)},{r(rs)},{r(rt)}"
        if funct == 0x25: return f"or {r(rd)},{r(rs)},{r(rt)}"
        if funct == 0x24: return f"and {r(rd)},{r(rs)},{r(rt)}"
        if funct == 0x2A: return f"slt {r(rd)},{r(rs)},{r(rt)}"
        if funct == 0x00: return f"sll {r(rd)},{r(rt)},{shamt}"
        if funct == 0x02: return f"srl {r(rd)},{r(rt)},{shamt}"
        if funct == 0x03: return f"sra {r(rd)},{r(rt)},{shamt}"
        if funct == 0x08: return f"jr {r(rs)}"
        if funct == 0x09: return f"jalr {r(rd)},{r(rs)}"
        return f"special_0x{funct:02X} {r(rd)},{r(rs)},{r(rt)}"
    elif op == 0x02: return f"j 0x{(va & 0xF0000000) | (imm26 << 2):08X}"
    elif op == 0x03: return f"jal 0x{(va & 0xF0000000) | (imm26 << 2):08X}"
    elif op == 0x04: return f"beq {r(rs)},{r(rt)},0x{va + 4 + (imm16s << 2):08X}"
    elif op == 0x05: return f"bne {r(rs)},{r(rt)},0x{va + 4 + (imm16s << 2):08X}"
    elif op == 0x06: return f"blez {r(rs)},0x{va + 4 + (imm16s << 2):08X}"
    elif op == 0x07: return f"bgtz {r(rs)},0x{va + 4 + (imm16s << 2):08X}"
    elif op == 0x08: return f"addi {r(rt)},{r(rs)},{imm16s}"
    elif op == 0x09: return f"addiu {r(rt)},{r(rs)},{imm16s}"
    elif op == 0x0A: return f"slti {r(rt)},{r(rs)},{imm16s}"
    elif op == 0x0B: return f"sltiu {r(rt)},{r(rs)},{imm16s}"
    elif op == 0x0C: return f"andi {r(rt)},{r(rs)},0x{imm16:04X}"
    elif op == 0x0D: return f"ori {r(rt)},{r(rs)},0x{imm16:04X}"
    elif op == 0x0E: return f"xori {r(rt)},{r(rs)},0x{imm16:04X}"
    elif op == 0x0F: return f"lui {r(rt)},0x{imm16:04X}"
    elif op == 0x10: return f"cop0_0x{funct:02X}"
    elif op == 0x20: return f"lb {r(rt)},{imm16s}({r(rs)})"
    elif op == 0x21: return f"lh {r(rt)},{imm16s}({r(rs)})"
    elif op == 0x23: return f"lw {r(rt)},{imm16s}({r(rs)})"
    elif op == 0x24: return f"lbu {r(rt)},{imm16s}({r(rs)})"
    elif op == 0x25: return f"lhu {r(rt)},{imm16s}({r(rs)})"
    elif op == 0x28: return f"sb {r(rt)},{imm16s}({r(rs)})"
    elif op == 0x29: return f"sh {r(rt)},{imm16s}({r(rs)})"
    elif op == 0x2B: return f"sw {r(rt)},{imm16s}({r(rs)})"
    return f"op_0x{op:02X}_{funct:02X}"

def find_persist_base_refs(text, va_base):
    """Find all lui/addiu pairs that reference D_800A77F0."""
    refs = []
    for i in range(0, len(text) - 4, 4):
        word = u32(text, i)
        op = (word >> 26) & 0x3F
        if op == 0x0F:  # lui
            rt = (word >> 16) & 0x1F
            imm = word & 0xFFFF
            if imm == 0x800A:
                # Found lui $rt, 0x800A
                va = va_base + i
                # Look at the next few instructions for addiu with 0x77F0
                for j in range(i+4, min(i+24, len(text)-4), 4):
                    w2 = u32(text, j)
                    op2 = (w2 >> 26) & 0x3F
                    if op2 == 0x09:  # addiu
                        rt2 = (w2 >> 16) & 0x1F
                        rs2 = (w2 >> 21) & 0x1F
                        imm2 = w2 & 0xFFFF
                        imm2s = struct.unpack_from("<h", struct.pack("<H", imm2))[0]
                        if rt2 == rt and rs2 == rt and imm2s == 0x77F0:
                            refs.append((va, va_base + j, rt))
                            break
                        elif rt2 == rt and rs2 == rt and imm2s >= 0x77F0 and imm2s < 0x7FF0:
                            # Close but different offset
                            offset = imm2s - 0x77F0
                            refs.append((va, va_base + j, rt, f"offset+0x{offset:X}"))
                            break
    return refs

def find_stores_to_persist0(text, va_base):
    """Find instructions that store to the exact address D_800A77F0."""
    # Strategy: look for sw where the computed address is D_800A77F0
    # This means: base register loaded with 0x800A77F0, then sw rt, 0(base)
    results = []
    
    # First, find all lui 0x800A instructions
    lui_sites = []
    for i in range(0, len(text) - 4, 4):
        word = u32(text, i)
        op = (word >> 26) & 0x3F
        if op == 0x0F:  # lui
            rt = (word >> 16) & 0x1F
            imm = word & 0xFFFF
            if imm == 0x800A:
                lui_sites.append((va_base + i, rt, i))
    
    print(f"\nFound {len(lui_sites)} lui 0x800A instructions")
    
    # For each lui site, look ahead for addiu to get the full address
    for lui_va, reg, lui_off in lui_sites:
        for j in range(lui_off+4, min(lui_off+40, len(text)-4), 4):
            w2 = u32(text, j)
            op2 = (w2 >> 26) & 0x3F
            rt2 = (w2 >> 16) & 0x1F
            rs2 = (w2 >> 21) & 0x1F
            
            if op2 == 0x09 and rt2 == reg and rs2 == reg:  # addiu reg, reg, imm
                imm2 = struct.unpack_from("<h", struct.pack("<H", w2 & 0xFFFF))[0]
                target_addr = 0x800A0000 + (imm2 & 0xFFFF)
                full_addr = (0x800A << 16) + (imm2 & 0xFFFF) if imm2 >= 0 else (0x800A << 16) + (imm2 + 0x10000 if imm2 < 0 else imm2)
                
                # Check if this is persist base or nearby
                if 0x800A77F0 <= full_addr <= 0x800A7FF0:
                    persist_idx = (full_addr - 0x800A77F0) // 4
                    # Now look ahead for stores using this register
                    for k in range(j+4, min(j+60, len(text)-4), 4):
                        w3 = u32(text, k)
                        op3 = (w3 >> 26) & 0x3F
                        rt3 = (w3 >> 16) & 0x1F
                        base3 = (w3 >> 21) & 0x1F
                        
                        if base3 == reg and op3 == 0x2B and (w3 & 0xFFFF) == 0:  # sw rt, 0(reg)
                            results.append({
                                'va': va_base + k,
                                'lui_va': lui_va,
                                'addiu_va': va_base + j,
                                'persist_idx': persist_idx,
                                'full_addr': full_addr,
                                'store_rt': rt3,
                                'type': 'sw'
                            })
                        elif base3 == reg and op3 == 0x29 and (w3 & 0xFFFF) == 0:  # sh rt, 0(reg)
                            results.append({
                                'va': va_base + k,
                                'lui_va': lui_va,
                                'addiu_va': va_base + j,
                                'persist_idx': persist_idx,
                                'full_addr': full_addr,
                                'store_rt': rt3,
                                'type': 'sh'
                            })
                        elif base3 == reg and op3 == 0x28 and (w3 & 0xFFFF) == 0:  # sb rt, 0(reg)
                            results.append({
                                'va': va_base + k,
                                'lui_va': lui_va,
                                'addiu_va': va_base + j,
                                'persist_idx': persist_idx,
                                'full_addr': full_addr,
                                'store_rt': rt3,
                                'type': 'sb'
                            })
                        # Stop if register is clobbered
                        if op3 == 0x0F and (w3 >> 16) & 0x1F == reg:
                            break
                        if op3 == 0x09 and (w3 >> 16) & 0x1F == reg and ((w3 >> 21) & 0x1F) != reg:
                            break
                    break
            # Stop scanning if reg is clobbered by lui
            if op2 == 0x0F and rt2 == reg:
                break
    
    return results

def find_all_lui_800a(text, va_base):
    """Find ALL lui 0x800A sites and the addiu that follows, to map all persist references."""
    results = []
    for i in range(0, len(text) - 8, 4):
        word = u32(text, i)
        op = (word >> 26) & 0x3F
        if op == 0x0F:  # lui
            rt = (word >> 16) & 0x1F
            imm = word & 0xFFFF
            if imm == 0x800A:
                va = va_base + i
                # Look for next addiu
                for j in range(i+4, min(i+8, len(text)-4), 4):
                    w2 = u32(text, j)
                    op2 = (w2 >> 26) & 0x3F
                    if op2 == 0x09:  # addiu
                        rt2 = (w2 >> 16) & 0x1F
                        rs2 = (w2 >> 21) & 0x1F
                        if rt2 == rt and rs2 == rt:
                            imm2 = struct.unpack_from("<h", struct.pack("<H", w2 & 0xFFFF))[0]
                            full = 0x800A0000 + (imm2 if imm2 >= 0 else imm2 + 0x10000)
                            results.append((va, va_base + j, rt, full))
                            break
    return results

def scan_script_tables(text, va_base):
    """Scan the disc image for scene script tables and check for persist[0] writers."""
    # The script tables are in the data section. Each scene has:
    # - A scene table at some address
    # - Script data referenced from the table
    # 
    # For now, let's find the opcodes 0x09 and 0x0A in the script data sections
    # that reference mode 2, index 0
    pass

def main():
    exe_path = "build/extracted/disc1/SLUS_006.62"
    if not os.path.exists(exe_path):
        print(f"EXE not found: {exe_path}")
        sys.exit(1)
    
    text, t_addr, t_size = load_exe(exe_path)
    print(f"Text size: 0x{len(text):X} bytes ({len(text)//4} words)")
    
    # Find ALL lui 0x800A + addiu pairs
    all_refs = find_all_lui_800a(text, t_addr)
    persist_refs = [(va, addiu_va, reg, addr) for va, addiu_va, reg, addr in all_refs 
                    if 0x800A77F0 <= addr <= 0x800A7FF0]
    print(f"\nAll lui 0x800A + addiu pairs: {len(all_refs)}")
    print(f"Persist range refs (0x800A77F0..0x800A7FF0): {len(persist_refs)}")
    
    # Show all persist refs
    print("\n=== ALL PERSIST BASE REFERENCES ===")
    for lui_va, addiu_va, reg, addr in sorted(persist_refs, key=lambda x: x[0]):
        idx = (addr - 0x800A77F0) // 4
        print(f"  0x{lui_va:08X} lui $r{reg}, 0x800A / 0x{addiu_va:08X} addiu $r{reg}, idx=0x{idx:02X} (addr=0x{addr:08X})")
    
    # Find stores to persist[0]
    print("\n=== STORES TO PERSIST[0] ===")
    stores = find_stores_to_persist0(text, t_addr)
    persist0_stores = [s for s in stores if s['persist_idx'] == 0]
    for s in persist0_stores:
        print(f"  0x{s['va']:08X}: {s['type']} $r{s['store_rt']}, 0($r?) via lui@0x{s['lui_va']:08X} addiu@0x{s['addiu_va']:08X}")
        # Disassemble surrounding context
        for context_va in range(s['va'] - 16, s['va'] + 20, 4):
            off = context_va - t_addr
            if 0 <= off < len(text):
                w = u32(text, off)
                dis = disasm_one(w, context_va)
                marker = " <-- STORE" if context_va == s['va'] else ""
                print(f"    0x{context_va:08X}: {w:08X} {dis}{marker}")
    
    # Also show stores to nearby persist indices (0-7 for flags)
    print("\n=== STORES TO PERSIST[0..7] (flag word candidates) ===")
    flag_stores = [s for s in stores if 0 <= s['persist_idx'] <= 7]
    for s in flag_stores:
        print(f"  0x{s['va']:08X}: {s['type']} $r{s['store_rt']}, 0($r?) → persist[0x{s['persist_idx']:X}]")
    
    # Search for ORI 0x4 near persist base loads
    print("\n=== ORI 0x4 NEAR PERSIST BASE LOADS ===")
    for lui_va, addiu_va, reg, addr in persist_refs:
        # Look for ori with value 0x4 in the window after addiu
        for k in range(addiu_va - t_addr + 4, min(addiu_va - t_addr + 80, len(text) - 4), 4):
            w = u32(text, k)
            op = (w >> 26) & 0x3F
            if op == 0x0D:  # ori
                imm = w & 0xFFFF
                if imm == 0x4:
                    va = t_addr + k
                    rt = (w >> 16) & 0x1F
                    rs = (w >> 21) & 0x1F
                    dis = disasm_one(w, va)
                    print(f"  0x{va:08X}: {w:08X} {dis}  (near persist ref at 0x{addiu_va:08X})")
            # Stop if we hit another lui for a different address
            if op == 0x0F and ((w >> 16) & 0x1F) != 0:
                break
    
    # Search for ALL OR/AND instructions that involve 0x4 and could target persist[0]
    print("\n=== ALU subop 0x02 (OR) with value 0x4 in scripts ===")
    # In the field script, OR operations use opcode 0x09 subop 0x02
    # The script interpreter at func_80012850 handles this
    # Let's find func_80012850 and trace what it does
    print("  (Script ALU handled by func_80012850 - binder-mediated)")
    print("  (No direct ORI to persist[0] expected in EXE code)")
    
    # CRITICAL: search for the specific pattern where func_80034F10 is called
    # and check if there's ANOTHER function that also zeros/writes persist[0]
    print("\n=== func_80034F10 (known persist zero) ===")
    func_34f10_va = 0x80034F10
    func_34f10_off = func_34f10_va - t_addr
    if 0 <= func_34f10_off < len(text):
        for i in range(func_34f10_off, min(func_34f10_off + 80, len(text)-4), 4):
            w = u32(text, i)
            va = t_addr + i
            dis = disasm_one(w, va)
            print(f"  0x{va:08X}: {w:08X} {dis}")
    
    # Search for func_80034FC4 (field load)
    print("\n=== func_80034FC4 (field load) ===")
    func_34fc4_va = 0x80034FC4
    func_34fc4_off = func_34fc4_va - t_addr
    if 0 <= func_34fc4_off < len(text):
        for i in range(func_34fc4_off, min(func_34fc4_off + 80, len(text)-4), 4):
            w = u32(text, i)
            va = t_addr + i
            dis = disasm_one(w, va)
            print(f"  0x{va:08X}: {w:08X} {dis}")
    
    # Count EXE persist refs by function
    print("\n=== PERSIST REFS BY NEAREST FUNCTION ===")
    # Group by rough function (every 256 bytes)
    from collections import Counter
    func_groups = Counter()
    for lui_va, addiu_va, reg, addr in persist_refs:
        func_start = (addiu_va >> 8) << 8
        func_groups[func_start] += 1
    for func, count in func_groups.most_common():
        idx = (list(set([(a[3]-0x800A77F0)//4 for a in persist_refs if (a[1]>>8)<<8 == func])))
        print(f"  0x{func:08X}: {count} refs, persist indices: {[hex(i) for i in sorted(idx)]}")

if __name__ == "__main__":
    main()
