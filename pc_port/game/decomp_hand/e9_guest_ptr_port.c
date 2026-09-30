/*
 * Hand adapters for five leaves that the generator used to emit but that
 * are NOT host-representable verbatim: each one loads a 32-bit guest pointer
 * out of guest RAM and uses it as a C pointer (a `(T **)` read of 8 host
 * bytes, or a `(T *)` cast of a loaded word).  gen_decomp_ports.py now
 * rejects them (E9b / E9, 2026-09-23); here every guest pointer is a
 * pe_addr_t loaded with PE_LoadU32 and every access goes through
 * PE_Load / PE_Store at the leaf's own offsets and widths.
 *
 * Host signatures are the ones the generated TUs exported, so callers are
 * unchanged.  Authority for every line: the matched src/func_X.c named in
 * each comment.  Tests: pc_port/tests/test_port_absent.h
 * (test_PORTABSENT_e9_guest_ptr).
 */
#include "pe_guest_decomp.h"

/* src/func_80030640.c: RNG gate.  rec = D_8009D278; inner = *(u32 **)(rec +
 * 0x68); return unless inner[4] & 0x10000; thresh = *(u16 *)(rec + 0x22);
 * rnd = func_80071A54(); if (rnd % 100 < (int)thresh) *(short *)(D_8009D278
 * reloaded + 0x10) = 9000.  `rnd` is `int` in the leaf, so `%` is signed. */
void func_80030640(void)
{
    pe_addr_t rec = PE_LoadU32(0x8009D278u);
    pe_addr_t inner = PE_LoadU32(rec + 0x68u);
    unsigned short thresh;
    int rnd;

    if ((PE_LoadU32(inner + 4u * 4u) & 0x10000u) == 0)
        return;
    thresh = PE_LoadU16(rec + 0x22u);
    rnd = (int)func_80071A54();
    if ((rnd % 100) < (int)thresh) {
        pe_addr_t rec2 = PE_LoadU32(0x8009D278u);
        PE_StoreU16(rec2 + 0x10u, (uint16_t)(short)9000);
    }
}

/* src/func_80019FE0.c: s = D_8009D2F0; s[0x63] = 0; s[0x26] &= 0xFF9FFFFF;
 * walk the list p = D_8009D20C via p = p[1]: return 1 if some p != s has
 * p[0x63] == s[0x63]; otherwise p = D_8009D2F0[0x63] (the word just
 * zeroed — the leaf re-reads it through the global) and p[0x26] &=
 * 0xFFEFFFFF; return 1.  Word indices are u32 elements.
 * NOTE: the leaf zeroes s[0x63] before the walk and re-reads that same word
 * (D_8009D2F0 == s) on the fall-through path, so retail's final RMW targets
 * KUSEG 0x00000098 — low main RAM through the KUSEG mirror, which
 * PE_Translate folds onto KSEG0 (PE_RamCanonical). */
int func_80019FE0(void)
{
    pe_addr_t s = PE_LoadU32(0x8009D2F0u);
    pe_addr_t p;

    PE_StoreU32(s + 0x63u * 4u, 0u);
    PE_StoreU32(s + 0x26u * 4u, PE_LoadU32(s + 0x26u * 4u) & 0xFF9FFFFFu);
    p = PE_LoadU32(0x8009D20Cu);
    while (p != 0) {
        if (p != s) {
            if (PE_LoadU32(p + 0x63u * 4u) == PE_LoadU32(s + 0x63u * 4u))
                return 1;
        }
        p = PE_LoadU32(p + 1u * 4u);
    }
    p = PE_LoadU32(PE_LoadU32(0x8009D2F0u) + 0x63u * 4u);
    PE_StoreU32(p + 0x26u * 4u, PE_LoadU32(p + 0x26u * 4u) & 0xFFEFFFFFu);
    return 1;
}

/* src/func_80018D50.c / func_80018DD4.c: field-VM opcode pair.  base =
 * **a0 * (D_8009D1D8 == 0 ? 22 : 28) + D_8009D1FC[7] (a guest address held
 * in word 7 of the D_8009D1FC block); 80018D50 sets bit 7 of *base,
 * 80018DD4 clears it.  Returns 1.  `**a0` is `int` (signed multiply). */
static pe_addr_t e9_flag_byte(pe_addr_t a0)
{
    int v = (int)PE_LoadU32(PE_LoadU32(a0));
    uint32_t w = PE_LoadU32(PE_LoadU32(0x8009D1FCu) + 7u * 4u);

    if ((int)PE_LoadU32(0x8009D1D8u) == 0)
        return (pe_addr_t)((uint32_t)(v * 22) + w);
    return (pe_addr_t)((uint32_t)(v * 28) + w);
}

int func_80018D50(pe_addr_t pe_a0)
{
    pe_addr_t base = e9_flag_byte(pe_a0);

    PE_StoreU8(base, (uint8_t)(PE_LoadU8(base) | 0x80u));
    return 1;
}

int func_80018DD4(pe_addr_t pe_a0)
{
    pe_addr_t base = e9_flag_byte(pe_a0);

    PE_StoreU8(base, (uint8_t)(PE_LoadU8(base) & 0x7Fu));
    return 1;
}

/* src/func_8004006C.c: tiny formatter.  Arguments come from the word array
 * D_800A1708 (a cursor `int *arg`).  Copies fmt to dst; on '%':
 *   'd'  two SJIS digits (tens, units) of the next int: each digit c is
 *        written as the halfword 0x824F + c, high byte first;
 *   'D'  one SJIS digit (units);
 *   's'  the next word is a guest string pointer; copy it (NULL = nothing);
 *   other conversion characters are consumed and emit nothing.
 * Terminates dst with 0.  `v / 10`, `t % 10`, `v % 10` are signed int. */
void func_8004006C(pe_addr_t pe_dst, pe_addr_t pe_fmt)
{
    pe_addr_t arg = 0x800A1708u;
    int v;
    int t;
    unsigned int w;
    pe_addr_t s;

    while (PE_LoadU8(pe_fmt) != 0) {
        if (PE_LoadU8(pe_fmt++) == '%') {
            switch (PE_LoadU8(pe_fmt++)) {
            case 'd':
                v = (int)PE_LoadU32(arg);
                arg += 4u;
                t = v / 10;
                w = (unsigned int)(t % 10 + 0x824F);
                PE_StoreU8(pe_dst++, (uint8_t)(w >> 8));
                PE_StoreU8(pe_dst++, (uint8_t)w);
                w = (unsigned int)(v % 10 + 0x824F);
                PE_StoreU8(pe_dst++, (uint8_t)(w >> 8));
                PE_StoreU8(pe_dst++, (uint8_t)w);
                break;
            case 'D':
                v = (int)PE_LoadU32(arg);
                arg += 4u;
                w = (unsigned int)(v % 10 + 0x824F);
                PE_StoreU8(pe_dst++, (uint8_t)(w >> 8));
                PE_StoreU8(pe_dst++, (uint8_t)w);
                break;
            case 's':
                s = PE_LoadU32(arg);
                arg += 4u;
                if (s != 0) {
                    while (PE_LoadU8(s) != 0)
                        PE_StoreU8(pe_dst++, PE_LoadU8(s++));
                }
                break;
            }
        } else {
            PE_StoreU8(pe_dst++, PE_LoadU8(pe_fmt - 1u));
        }
    }
    PE_StoreU8(pe_dst, 0);
}

/* src/func_80088F6C.c: func_800878F0(a0->+0xF0, a0 + 0xF0) [the matched
 * call's third argument a0->+0x38 is dropped: func_800878F0's own matched C
 * takes two]; p = D_800B8AC0 + a1 * 0x11C; a0->+0x118 = p->+0x118 (short);
 * a0->+0xF4 |= 0x1FF93; a0->+0x11A = p->+0x11A (short); then
 * func_800878F0(a1, a0 + 0xF0).  Rejected by the generator (E14: the host
 * pointer `a0 + 0xF0` passed to a pe_addr_t parameter). */
void func_80088F6C(pe_addr_t a0, int a1)
{
    pe_addr_t p;
    unsigned short t2;

    func_800878F0((int)PE_LoadU32(a0 + 0xF0u), a0 + 0xF0u);
    p = 0x800B8AC0u + (uint32_t)(a1 * 0x11C);
    PE_StoreU16(a0 + 0x118u, PE_LoadU16(p + 0x118u));
    t2 = PE_LoadU16(p + 0x11Au);
    PE_StoreU32(a0 + 0xF4u, PE_LoadU32(a0 + 0xF4u) | 0x1FF93u);
    PE_StoreU16(a0 + 0x11Au, t2);
    func_800878F0(a1, a0 + 0xF0u);
}

/* Guest scratch for retail stack buffers of the leaves in this file whose
 * address is handed to a guest-address callee (a host stack address has no
 * guest meaning).  0x801FF700..0x801FF7FF, disjoint from the other lanes'
 * scratch (0x801FF800.. ovl2, 0x801FF900.. lo2, 0x801FF980.. hi2,
 * 0x801FFA00.. ovl, 0x801FFB00.. sdk/cd, 0x801FFC00.. hi, 0x801FFD00.. lo). */
#define PE_HAND_E9_STACK 0x801FF700u

/* src/func_8004A6CC.c: status-panel text for the selected stat
 * (D_8009CFD0).  Pen moves (func_8005E8A4) and glyphs (func_8005EB64) in the
 * leaf's order.  D_8009CFD0 < 3: value D_8009CFD8, label (+124 when
 * D_8009CF18 else +127), then for stat k: v = D_800A1A00.b[k] (u8 at +7+k)
 * + h[k] (short at +0xE+2k) clamped to 999, the base b[k] and the bonus
 * h[k].  Otherwise: label +140, then func_8005B91C(stat, D_8009CFDC,
 * &buf[0], 0) -> func_800605F8(buf[0] + 1) and func_8005BA78(stat,
 * D_8009CFDC, &buf[1], &buf[2]) -> func_8006062C(buf[1], buf[2]); `buf` is
 * the retail stack array, here PE_HAND_E9_STACK (+0/+4/+8).  Rejected by the
 * generator (E14: &buf[0] passed to a pe_addr_t parameter). */
void func_8004A6CC(void)
{
    const pe_addr_t buf = PE_HAND_E9_STACK;
    int stat;
    int v;

    func_8005E8A4(20, 21);
    func_8005EB64((uint32_t)((int)PE_LoadU32(0x8009CFACu) + 77));
    func_8005E8A4(-14, -17);
    func_8005EB64(147u);
    stat = (int)PE_LoadU32(0x8009CFD0u);
    if (stat < 3) {
        func_8005E8A4(72, 0);
        func_8006055C((int)PE_LoadU32(0x8009CFD8u));
        func_8005E8A4(-117, 30);
        func_8005EB64((uint32_t)(PE_LoadU32(0x8009CF18u) != 0u ? stat + 124 : stat + 127));
        func_8005E8A4(30, 0);
        if (stat >= 0) {       /* the leaf's switch has cases 0, 1, 2 only */
            int b = PE_LoadU8(0x800A1A00u + 7u + (uint32_t)stat);
            int h = (int16_t)PE_LoadU16(0x800A1A00u + 0xEu + 2u * (uint32_t)stat);

            v = b + h;
            if (v >= 1000)
                v = 999;
            func_80060590(v);
            func_8005E8A4(4, 2);
            func_8005FDF0(b);
            func_8005E8A4(5, 0);
            func_8005FF28(h);
        }
        func_8005E8A4(-45, -10);
        func_8005EB64(135u);
        func_8005E8A4(25, 0);
        func_8005EB64(136u);
    } else {
        func_8005E8A4(92, 0);
        func_8006055C((int)PE_LoadU32(0x8009CFD8u));
        func_8005E8A4(-138, 31);
        func_8005EB64((uint32_t)(stat + 140));
        func_8005E8A4(66, -1);
        func_8005B91C(stat, (int)PE_LoadU32(0x8009CFDCu), buf + 0u, 0u);
        func_800605F8((int)PE_LoadU32(buf + 0u) + 1);
        func_8005BA78(stat, (int)PE_LoadU32(0x8009CFDCu), buf + 4u, buf + 8u);
        func_8005E8A4(2, 0);
        func_8006062C((int)PE_LoadU32(buf + 4u), (int)PE_LoadU32(buf + 8u));
    }
}
