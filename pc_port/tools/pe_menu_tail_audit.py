#!/usr/bin/env python3
"""Static last-writer audit for the M0000I menu-list tail (func_801942FC).

The tail is original 801942FC sp+138..145.  8001220C runs the M0000I loop on
the scratchpad stack, so these are scratchpad bytes 1F800392..99 (loop sp S =
1F8003C8, tail = S-54..S-47).  This tool re-derives, from build/disc1.elf and
the disc's M0000I overlay (read at run time, never stored), every fact the
native model in func_801942FC_port.c relies on, and asserts it.
"""
import os, re, struct, subprocess, sys, tempfile
sys.path.insert(0, os.path.dirname(__file__))
from pe_m0000i_leaves_oracle import load_overlay
from pe_battle_hud_oracle import ROOT

OBJDUMP = 'mips-linux-gnu-objdump'


def disasm_bytes(data, vma):
    with tempfile.NamedTemporaryFile(dir=ROOT / 'build', suffix='.bin') as f:
        f.write(data); f.flush()
        out = subprocess.run([OBJDUMP, '-D', '-b', 'binary', '-m', 'mips:3000', '-EL',
                              f'--adjust-vma=0x{vma:x}', f.name], capture_output=True, text=True, check=True).stdout
    return parse(out)


def parse(out):
    ins = {}
    for line in out.splitlines():
        m = re.match(r'\s*([0-9a-f]{8}):\s+[0-9a-f]{8}\s+(.*)', line)
        if m: ins[int(m.group(1), 16)] = re.sub(r'\s+<.*>', '', m.group(2)).strip().replace('\t', ' ')
    return ins


def main():
    exe, overlay, base = load_overlay()
    elf = parse(subprocess.run([OBJDUMP, '-d', '-EL', str(ROOT / 'build/disc1.elf')],
                               capture_output=True, text=True, check=True).stdout)
    ovl = disasm_bytes(overlay, base)
    u32 = lambda a: struct.unpack_from('<I', exe, a - 0x80010000 + 0x800)[0]
    o32 = lambda a: struct.unpack_from('<I', overlay, a - base)[0]
    facts = []

    def need(code, addr, text, why):
        got = code.get(addr, '')
        assert got == text, (hex(addr), got, text)
        facts.append(f'{addr:08X} {text:28s} {why}')

    # Scratchpad stack for the M0000I loop.
    need(elf, 0x80012334, 'move sp,t0', '8001220C: sp = 1F8003F8 (scratchpad) before jal 8019234C')
    need(ovl, 0x8019234C, 'addiu sp,sp,-48', 'loop sp S = 1F8003C8')
    need(ovl, 0x80194304, 'addiu sp,sp,-192', '801942FC frame: list at sp+136 = S-56')
    need(ovl, 0x80194318, 'sh zero,136(sp)', 'only list[0] is initialized')
    need(ovl, 0x80195678, 'addiu a2,sp,136', 'menu list passed to 800375E0')
    assert o32(0x8019C018) == 1; facts.append('8019C018 = 1 in the loaded overlay: menu path runs on the first frame')
    # Constructor locals (frame 240 => sp+184 = S-56, sp+192 = S-48).
    need(ovl, 0x80196498, 'addiu sp,sp,-240', 'constructor frame')
    need(ovl, 0x801966C8, 'sw zero,184(sp)', 'record offset starts at 0')
    need(ovl, 0x80196F88, 'addiu t9,t9,52', '... +52 per record')
    need(ovl, 0x801967DC, 'addiu t9,s8,4', 'cursor = positions+4')
    need(ovl, 0x80196EF4, 'addiu t9,t9,8', '... +8 per record')
    need(ovl, 0x8019710C, 'slti v0,t9,10', '10 records => 520, positions+84')
    # VSync(-1) returns early without its 80073BBC helper.
    need(elf, 0x80073A9C, 'bgez a0,80073ab4', 'negative mode skips the wait helper')
    # libgpu function table (static data) and the two register-save writers.
    table = u32(0x80095744)
    assert table == 0x80095704 and u32(table + 8) == 0x80076C34 and u32(table + 44) == 0x80076354 and u32(table + 16) == 0x80076B20
    facts.append('80095744 -> 80095704: [2]=80076C34 [4]=80076B20 [11]=80076354')
    need(elf, 0x80075470, 'addiu s0,s1,28', 'PutDrawEnv: s0 = env+28, s1 = env')
    need(elf, 0x800754B0, 'jalr v0', 'PutDrawEnv -> table[2] at sp S-32')
    need(elf, 0x80076C34, 'addiu sp,sp,-40', '80076C34 sp = S-72')
    need(elf, 0x80076C40, 'sw s0,16(sp)', '=> S-56 = env+28 (tail0 = hi)')
    need(elf, 0x80076C48, 'sw s1,20(sp)', '=> S-52 = env (tail1 = lo, tail2 = hi)')
    need(elf, 0x800752BC, 'move s0,a0', 'DrawOTag: s0 = ot')
    need(elf, 0x80076354, 'addiu sp,sp,-32', '80076354 sp = S-64')
    need(elf, 0x80076358, 'sw s0,16(sp)', '=> S-48 = ot (tail3 = lo)')
    # Input actuator path.
    need(elf, 0x80082914, 'jal 80083bb8', 'ra = 8008291C')
    need(elf, 0x80083BD0, 'sw ra,24(sp)', '80083BB8 sp = S-80 => S-56 = 8008291C (tail0 = 8008)')
    # Interrupt dispatcher uses its own stack.
    need(elf, 0x80073EAC, 'addiu v0,s0,4060', 'libetc interrupt jmp_buf sp = 800955FC')
    print('\n'.join(facts))
    print('MENU_TAIL_AUDIT=PASS')


if __name__ == '__main__':
    main()
