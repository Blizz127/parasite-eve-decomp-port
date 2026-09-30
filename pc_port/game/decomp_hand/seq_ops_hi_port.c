/*
 * Hand adapters — sequence-stream opcode handlers (0x8008F000..0x80091100).
 *
 * Each handler takes the voice/track record `a0`, whose word 0 is a guest
 * byte cursor into the sequence stream.  Bodies follow the matched leaves
 * src/func_XXXXXXXX.c with the record and cursor as guest addresses.
 */
#include "pe_guest_decomp.h"
#include "hand_hi_protos.h"

/* Read the next stream byte and post-increment the cursor (the shared
 * `p = *(u8 **)a0; *(u8 **)a0 = p + 1; c = *p;` prologue). */
static unsigned char seq_next(pe_addr_t a0)
{
    pe_addr_t p = PE_LoadU32(a0);

    PE_StoreU32(a0, p + 1u);
    return PE_LoadU8(p);
}

static void flags38_or(pe_addr_t a0, uint32_t m) { PE_StoreU32(a0 + 0x38u, PE_LoadU32(a0 + 0x38u) | m); }
static void flags38_and(pe_addr_t a0, uint32_t m) { PE_StoreU32(a0 + 0x38u, PE_LoadU32(a0 + 0x38u) & m); }

/* src/func_8008FCB4.c: +0x82 = 0. */
/* func_8008FCB4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8008FCB4_port.c (src/func_8008FCB4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
/* src/func_800904A0.c: +0x84 = 1. */
/* func_800904A0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800904A0_port.c (src/func_800904A0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
/* src/func_80090A0C.c: +0x38 &= ~0x8. */
/* func_80090A0C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80090A0C_port.c (src/func_80090A0C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
/* src/func_80090C38.c / C4C / C60 / C74: +0x38 bit 4 / bit 5 set / clear. */
/* func_80090C38: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80090C38_port.c (src/func_80090C38.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
/* func_80090C4C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80090C4C_port.c (src/func_80090C4C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
/* func_80090C60: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80090C60_port.c (src/func_80090C60.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
/* func_80090C74: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80090C74_port.c (src/func_80090C74.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
/* src/func_80090F54.c: +0x38 |= 0x100000. */
/* func_80090F54: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80090F54_port.c (src/func_80090F54.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_8008F868.c / 8008F880.c: +0x7C = (+0x7C +/- 1) & 0xF. */
/* func_8008F868: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8008F868_port.c (src/func_8008F868.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
/* func_8008F880: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8008F880_port.c (src/func_8008F880.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_8008F84C.c: +0x7C = next byte. */
void func_8008F84C(pe_addr_t arg0) { PE_StoreU16(arg0 + 0x7Cu, seq_next(arg0)); }
/* src/func_8008FFC0.c: +0xA6 = next byte << 8. */
void func_8008FFC0(pe_addr_t a0) { PE_StoreU16(a0 + 0xA6u, (uint16_t)(seq_next(a0) << 8)); }
/* src/func_800900E4.c: +0xB4 = next byte << 7. */
void func_800900E4(pe_addr_t a0) { PE_StoreU16(a0 + 0xB4u, (uint16_t)(seq_next(a0) << 7)); }
/* src/func_8008FBD4.c / 8008FCBC.c: +0xDE / +0xE0 = (s8) next byte. */
void func_8008FBD4(pe_addr_t a0) { PE_StoreU16(a0 + 0xDEu, (uint16_t)(int16_t)(signed char)seq_next(a0)); }
void func_8008FCBC(pe_addr_t a0) { PE_StoreU16(a0 + 0xE0u, (uint16_t)(int16_t)(signed char)seq_next(a0)); }

/* src/func_80090948.c: v = next byte; +0xD2 = 0; +0x58 = +0x56 = +0xD0 = v. */
void func_80090948(pe_addr_t a0)
{
    unsigned int v = seq_next(a0);

    PE_StoreU16(a0 + 0xD2u, 0u);
    PE_StoreU16(a0 + 0x58u, (uint16_t)v);
    PE_StoreU16(a0 + 0x56u, (uint16_t)v);
    PE_StoreU16(a0 + 0xD0u, (uint16_t)v);
}

/* src/func_8008F4E8.c: +0xF4 |= 3; +0x6C = next byte << 8. */
void func_8008F4E8(pe_addr_t a0)
{
    unsigned char c = seq_next(a0);

    PE_StoreU32(a0 + 0xF4u, PE_LoadU32(a0 + 0xF4u) | 3u);
    PE_StoreU16(a0 + 0x6Cu, (uint16_t)(c << 8));
}

/* src/func_8008FBFC.c / 8008FCE4.c: +0xDE / +0xE0 += (s8) next byte. */
void func_8008FBFC(pe_addr_t a0)
{
    unsigned char c = seq_next(a0);

    PE_StoreU16(a0 + 0xDEu, (uint16_t)(PE_LoadU16(a0 + 0xDEu) + (signed char)c));
}
void func_8008FCE4(pe_addr_t a0)
{
    unsigned char c = seq_next(a0);

    PE_StoreU16(a0 + 0xE0u, (uint16_t)(PE_LoadU16(a0 + 0xE0u) + (signed char)c));
}

/* Next byte into a voice parameter with its dirty flags ORed into +0xF4
 * (the cursor advance happens before the +0xF4 read, as in retail). */
static void byte_param16(pe_addr_t a0, uint32_t flags, uint32_t off)
{
    unsigned char byte = seq_next(a0);

    PE_StoreU32(a0 + 0xF4u, PE_LoadU32(a0 + 0xF4u) | flags);
    PE_StoreU16(a0 + off, byte);
}
static void byte_param32(pe_addr_t a0, uint32_t flags, uint32_t off)
{
    unsigned char byte = seq_next(a0);

    PE_StoreU32(a0 + 0xF4u, PE_LoadU32(a0 + 0xF4u) | flags);
    PE_StoreU32(a0 + off, byte);
}

/* src/func_80090574.c: flags 0x900, byte -> +0x10E (u16). */
void func_80090574(pe_addr_t a0) { byte_param16(a0, 0x900u, 0x10Eu); }
/* src/func_800905EC.c: flags 0x2200, byte -> +0x114 (u16). */
void func_800905EC(pe_addr_t a0) { byte_param16(a0, 0x2200u, 0x114u); }
/* src/func_80090614.c: flags 0x4400, byte -> +0x116 (u16). */
void func_80090614(pe_addr_t a0) { byte_param16(a0, 0x4400u, 0x116u); }
/* src/func_8009063C.c: flags 0x100, byte -> +0x100 (u32). */
void func_8009063C(pe_addr_t a0) { byte_param32(a0, 0x100u, 0x100u); }
/* src/func_80090664.c: flags 0x200, byte -> +0x104 (u32). */
void func_80090664(pe_addr_t a0) { byte_param32(a0, 0x200u, 0x104u); }
/* src/func_8009068C.c: flags 0x400, byte -> +0x108 (u32). */
void func_8009068C(pe_addr_t a0) { byte_param32(a0, 0x400u, 0x108u); }

/* src/func_8008F224.c: master volume word D_8009D2C8->+0x20 = b0 << 16,
 * then |= b1 << 24 with the fade counter +0x52 cleared in between. */
void func_8008F224(pe_addr_t a0)
{
    unsigned char b0 = seq_next(a0);
    pe_addr_t base = PE_LoadU32(0x8009D2C8u);
    unsigned char c;
    uint32_t t;

    PE_StoreU32(base + 0x20u, (uint32_t)b0 << 16);
    c = seq_next(a0);
    t = PE_LoadU32(base + 0x20u);
    PE_StoreU16(base + 0x52u, 0u);
    PE_StoreU32(base + 0x20u, t | ((uint32_t)c << 24));
}

/* src/func_8008F274.c / 8008F37C.c: fade to (b1 << 16 | b2 << 24) over
 * b0 steps (0 -> 0x100) held at +0x52 / +0x58: the current word +0x20 /
 * +0x40 has its low 16 bits cleared and the step (target - current) /
 * steps goes to +0x24 / +0x44 (signed int / u16, steps never 0). */
static void seq_fade(pe_addr_t a0, uint32_t cnt_off, uint32_t cur_off)
{
    unsigned char c = seq_next(a0);
    pe_addr_t base = PE_LoadU32(0x8009D2C8u);
    int v, old;

    PE_StoreU16(base + cnt_off, c);
    if (c == 0)
        PE_StoreU16(base + cnt_off, 0x100u);
    v = (int)((uint32_t)seq_next(a0) << 16);
    v |= (int)((uint32_t)seq_next(a0) << 24);
    base = PE_LoadU32(0x8009D2C8u);
    old = (int)(PE_LoadU32(base + cur_off) & ~0xFFFFu);
    PE_StoreU32(base + cur_off, (uint32_t)old);
    PE_StoreU32(base + cur_off + 4u,
                (uint32_t)((int)((uint32_t)v - (uint32_t)old) / (int)PE_LoadU16(base + cnt_off)));
}
void func_8008F274(pe_addr_t a0) { seq_fade(a0, 0x52u, 0x20u); }
void func_8008F37C(pe_addr_t a0) { seq_fade(a0, 0x58u, 0x40u); }

/* src/func_8008F328.c: D_8009D2C8->+0x40 = b0 << 16 | b1 << 24, fade
 * counter +0x58 cleared, D_8009D2C4 |= 0x80. */
void func_8008F328(pe_addr_t a0)
{
    uint32_t v = (uint32_t)seq_next(a0) << 16;
    pe_addr_t base;

    v |= (uint32_t)seq_next(a0) << 24;
    base = PE_LoadU32(0x8009D2C8u);
    PE_StoreU16(base + 0x58u, 0u);
    PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) | 0x80u);
    PE_StoreU32(base + 0x40u, v);
}

/* src/func_8008F430.c: relative jump — cursor = p + 2 + (s16)(b0 | b1 << 8). */
void func_8008F430(pe_addr_t a0)
{
    pe_addr_t p = PE_LoadU32(a0);
    unsigned char lo = PE_LoadU8(p + 0u);
    unsigned char hi = PE_LoadU8(p + 1u);

    PE_StoreU32(a0, p + 2u + (uint32_t)(int32_t)(int16_t)(lo | (hi << 8)));
}

/* src/func_8008F470.c: conditional relative jump — when
 * D_8009D2C8->+0x56 >= b0 take the s16 offset (b1 | b2 << 8) relative to
 * the cursor after it; otherwise skip the three operand bytes. */
void func_8008F470(pe_addr_t a0)
{
    pe_addr_t p = PE_LoadU32(a0);
    int c = seq_next(a0);

    if ((int)PE_LoadU16(PE_LoadU32(0x8009D2C8u) + 0x56u) >= c) {
        unsigned int lo = seq_next(a0);
        unsigned int hi = seq_next(a0);

        PE_StoreU32(a0, PE_LoadU32(a0) + (uint32_t)(int32_t)(int16_t)(lo | (hi << 8)));
    } else {
        PE_StoreU32(a0, p + 3u);
    }
}

/* src/func_8008F514.c: volume fade — steps b0 (0 -> 0x100) at +0x6E, the
 * +0x6C level keeps only bits 8..14, step ((b1 << 8) - level) / steps
 * into +0xD4. */
void func_8008F514(pe_addr_t a0)
{
    unsigned char c = seq_next(a0);
    unsigned char b;
    int old;

    PE_StoreU16(a0 + 0x6Eu, c);
    if (c == 0)
        PE_StoreU16(a0 + 0x6Eu, 0x100u);
    b = seq_next(a0);
    old = PE_LoadU16(a0 + 0x6Cu) & 0x7F00;
    PE_StoreU16(a0 + 0xD4u, (uint16_t)(((b << 8) - old) / (int)PE_LoadU16(a0 + 0x6Eu)));
    PE_StoreU16(a0 + 0x6Cu, (uint16_t)old);
}

/* src/func_8008F59C.c: +0x38 bit 3 set: +0x6A = b << 7; otherwise pan:
 * +0x72 = 0, +0xF4 |= 3, +0x44 = (s8)b << 23. */
void func_8008F59C(pe_addr_t a0)
{
    if (PE_LoadU32(a0 + 0x38u) & 8u) {
        PE_StoreU16(a0 + 0x6Au, (uint16_t)(seq_next(a0) << 7));
    } else {
        int v = (signed char)seq_next(a0);

        PE_StoreU16(a0 + 0x72u, 0u);
        PE_StoreU32(a0 + 0xF4u, PE_LoadU32(a0 + 0xF4u) | 3u);
        PE_StoreU32(a0 + 0x44u, (uint32_t)v << 23);
    }
}

/* src/func_8008F608.c: pan fade — steps b0 (0 -> 0x100) at +0x72, the +0x44
 * word keeps its high half, step (((s8)b1 << 23) - current) / steps into
 * +0x48. */
void func_8008F608(pe_addr_t a0)
{
    unsigned char c = seq_next(a0);
    int t, old;

    PE_StoreU16(a0 + 0x72u, c);
    if (c == 0)
        PE_StoreU16(a0 + 0x72u, 0x100u);
    t = (int)((uint32_t)(int32_t)(signed char)seq_next(a0) << 23);
    old = (int)(PE_LoadU32(a0 + 0x44u) & ~0xFFFFu);
    PE_StoreU32(a0 + 0x44u, (uint32_t)old);
    PE_StoreU32(a0 + 0x48u,
                (uint32_t)((int)((uint32_t)t - (uint32_t)old) / (int)PE_LoadU16(a0 + 0x72u)));
}

/* src/func_8008F6B0.c: +0x74 = 0, +0xD8 = b << 8, +0xF4 |= 3 when +0x38
 * bit 8 is set. */
void func_8008F6B0(pe_addr_t a0)
{
    unsigned char c = seq_next(a0);
    uint32_t f = PE_LoadU32(a0 + 0x38u);

    PE_StoreU16(a0 + 0x74u, 0u);
    PE_StoreU16(a0 + 0xD8u, (uint16_t)(c << 8));
    if (f & 0x100u)
        PE_StoreU32(a0 + 0xF4u, PE_LoadU32(a0 + 0xF4u) | 3u);
}

/* src/func_8008F6F4.c: +0xD8 fade — steps b0 at +0x74 (0 -> 0x100), level
 * keeps its high byte, step ((b1 << 8) - (short)level) / steps into +0xDA
 * (note the signed view of the level, unlike func_8008F7BC). */
void func_8008F6F4(pe_addr_t a0)
{
    unsigned char c = seq_next(a0);
    unsigned char b;
    unsigned short old;

    PE_StoreU16(a0 + 0x74u, c);
    if (c == 0)
        PE_StoreU16(a0 + 0x74u, 0x100u);
    b = seq_next(a0);
    old = (unsigned short)(PE_LoadU16(a0 + 0xD8u) & 0xFF00u);
    PE_StoreU16(a0 + 0xDAu, (uint16_t)(((b << 8) - (short)old) / (int)PE_LoadU16(a0 + 0x74u)));
    PE_StoreU16(a0 + 0xD8u, old);
}

/* src/func_8008F784.c: +0x78 = 0, +0xF4 |= 3, +0x76 = ((b + 0x40) & 0xFF) << 8. */
void func_8008F784(pe_addr_t a0)
{
    unsigned char c = seq_next(a0);

    PE_StoreU16(a0 + 0x78u, 0u);
    PE_StoreU32(a0 + 0xF4u, PE_LoadU32(a0 + 0xF4u) | 3u);
    PE_StoreU16(a0 + 0x76u, (uint16_t)(((c + 0x40) & 0xFF) << 8));
}

/* src/func_8008F7BC.c: +0x76 fade — steps b0 at +0x78 (0 -> 0x100), level
 * keeps its high byte (unsigned), step ((((b1 + 0x40) & 0xFF) << 8) -
 * level) / steps into +0xDC. */
void func_8008F7BC(pe_addr_t a0)
{
    unsigned char c = seq_next(a0);
    unsigned char b;
    unsigned short old;

    PE_StoreU16(a0 + 0x78u, c);
    if (c == 0)
        PE_StoreU16(a0 + 0x78u, 0x100u);
    b = seq_next(a0);
    old = (unsigned short)(PE_LoadU16(a0 + 0x76u) & 0xFF00u);
    PE_StoreU16(a0 + 0xDCu,
                (uint16_t)(((((b + 0x40) & 0xFF) << 8) - (int)old) / (int)PE_LoadU16(a0 + 0x78u)));
    PE_StoreU16(a0 + 0x76u, old);
}

/* src/func_8008FB00.c: set the zone-table pointer +0x18 to the s16 offset
 * (b0 | b1 << 8) past the operand, program +0x5A = 0xFF (none),
 * +0xE2 = 0, +0x38 |= 0x1000. */
void func_8008FB00(pe_addr_t a0)
{
    unsigned int lo = seq_next(a0);
    unsigned int hi = seq_next(a0);
    pe_addr_t end = PE_LoadU32(a0);

    PE_StoreU16(a0 + 0x5Au, 0xFFu);
    PE_StoreU16(a0 + 0xE2u, 0u);
    PE_StoreU32(a0 + 0x38u, PE_LoadU32(a0 + 0x38u) | 0x1000u);
    PE_StoreU32(a0 + 0x18u, end + (uint32_t)(int32_t)(int16_t)(lo | (hi << 8)));
}

/* src/func_8008FC28.c: +0x7E = b0 (0 -> 0x100), +0xE4 = (s8) b1. */
void func_8008FC28(pe_addr_t a0)
{
    unsigned char c = seq_next(a0);

    PE_StoreU16(a0 + 0x7Eu, c);
    if (c == 0)
        PE_StoreU16(a0 + 0x7Eu, 0x100u);
    PE_StoreU16(a0 + 0xE4u, (uint16_t)(int16_t)(signed char)seq_next(a0));
}

/* src/func_8008FC78.c: +0x82 = b (0 -> 0x100), +0xE6 = +0x80 = 0,
 * +0x84 = 1; returns 1. */
int func_8008FC78(pe_addr_t obj)
{
    unsigned int v = seq_next(obj);

    PE_StoreU16(obj + 0x82u, (uint16_t)v);
    if (v == 0)
        PE_StoreU16(obj + 0x82u, 0x100u);
    PE_StoreU16(obj + 0xE6u, 0u);
    PE_StoreU16(obj + 0x80u, 0u);
    PE_StoreU16(obj + 0x84u, 1u);
    return 1;
}

/* src/func_8008FE10.c: +0x94 = b << 8; depth f = bits 8..14; bit 15
 * selects the raw +0x30 width, else (w * 15) >> 8; +0x92 = (f * w) >> 7
 * (unsigned shift of the int product). */
void func_8008FE10(pe_addr_t a0)
{
    int f, w, prod;
    unsigned int v94;

    PE_StoreU16(a0 + 0x94u, (uint16_t)(seq_next(a0) << 8));
    v94 = PE_LoadU16(a0 + 0x94u);
    f = (int)((v94 & 0x7F00u) >> 8);
    w = (int)PE_LoadU32(a0 + 0x30u);
    if (v94 & 0x8000u)
        prod = f * w;
    else
        prod = f * ((w * 15) >> 8);
    PE_StoreU16(a0 + 0x92u, (uint16_t)((unsigned int)prod >> 7));
}

/* src/func_8008FE68.c: n = b0 (0 -> 0x100); +0x98 = ((b1 << 8) - +0x94) /
 * n; +0x96 = n. */
void func_8008FE68(pe_addr_t a0)
{
    int n = seq_next(a0);
    int val;

    if (n == 0)
        n = 0x100;
    val = ((seq_next(a0) << 8) - (int)PE_LoadU16(a0 + 0x94u)) / n;
    PE_StoreU16(a0 + 0x96u, (uint16_t)n);
    PE_StoreU16(a0 + 0x98u, (uint16_t)val);
}

/* src/func_8008FFE4.c / 80090108.c: n = b0 (0 -> 0x100); step
 * ((b1 << 8) - +0xA6) / n into +0xAA (resp. ((b1 << 7) - +0xB4) / n into
 * +0xB8); n into +0xA8 (resp. +0xB6). */
void func_8008FFE4(pe_addr_t a0)
{
    int n = seq_next(a0);
    int val;

    if (n == 0)
        n = 0x100;
    val = ((seq_next(a0) << 8) - (int)PE_LoadU16(a0 + 0xA6u)) / n;
    PE_StoreU16(a0 + 0xA8u, (uint16_t)n);
    PE_StoreU16(a0 + 0xAAu, (uint16_t)val);
}
void func_80090108(pe_addr_t a0)
{
    int n = seq_next(a0);
    int val;

    if (n == 0)
        n = 0x100;
    val = ((seq_next(a0) << 7) - (int)PE_LoadU16(a0 + 0xB4u)) / n;
    PE_StoreU16(a0 + 0xB6u, (uint16_t)n);
    PE_StoreU16(a0 + 0xB8u, (uint16_t)val);
}

/* src/func_80090078.c: +0x38 |= 4; +0xAE = b0 (0 -> 0x100); +0xB2 = b1;
 * +0xB0 = 1; +0x24 = D_8009C080[b1]. */
void func_80090078(pe_addr_t a0)
{
    int c;
    unsigned char d;
    uint32_t val;

    PE_StoreU32(a0 + 0x38u, PE_LoadU32(a0 + 0x38u) | 4u);
    c = seq_next(a0);
    PE_StoreU16(a0 + 0xAEu, (uint16_t)c);
    if (c == 0)
        PE_StoreU16(a0 + 0xAEu, 0x100u);
    d = seq_next(a0);
    PE_StoreU16(a0 + 0xB2u, d);
    val = PE_LoadU32(0x8009C080u + d * 4u);
    PE_StoreU16(a0 + 0xB0u, 1u);
    PE_StoreU32(a0 + 0x24u, val);
}

/* src/func_8009071C.c: call-stack push — slot h = (+0xCE + 1) & 3 gets
 * the current cursor (word 0) at +4 + h*4, and the slot's loop counter
 * +0x62 + h*2 is cleared. */
void func_8009071C(pe_addr_t a0)
{
    unsigned short h = (unsigned short)((PE_LoadU16(a0 + 0xCEu) + 1u) & 3u);
    uint32_t v = PE_LoadU32(a0);

    PE_StoreU16(a0 + 0xCEu, h);
    PE_StoreU32(a0 + 4u + ((uint32_t)h << 2), v);
    PE_StoreU16(a0 + 0x62u + ((uint32_t)PE_LoadU16(a0 + 0xCEu) << 1), 0u);
}

/* src/func_8009090C.c: bump the current slot's loop counter
 * (+0x62 + idx*2) and rewind the cursor to the slot's saved position
 * (+4 + idx*4); returns that position. */
int func_8009090C(pe_addr_t a0)
{
    unsigned short idx = PE_LoadU16(a0 + 0xCEu);
    pe_addr_t p = a0 + 0x62u + idx * 2u;
    uint32_t v;

    PE_StoreU16(p, (uint16_t)(PE_LoadU16(p) + 1u));
    idx = PE_LoadU16(a0 + 0xCEu);
    v = PE_LoadU32(a0 + idx * 4u + 4u);
    PE_StoreU32(a0, v);
    return (int)v;
}

/* src/func_80090754.c: loop end — n = b (0 -> 0x100); bump the current
 * slot's counter; until it reaches n rewind to the slot's saved cursor,
 * then pop the slot ((+0xCE - 1) & 3). */
void func_80090754(pe_addr_t a0)
{
    int n = seq_next(a0);
    pe_addr_t base;

    if (n == 0)
        n = 0x100;
    base = a0 + PE_LoadU16(a0 + 0xCEu) * 2u;
    PE_StoreU16(base + 0x62u, (uint16_t)(PE_LoadU16(base + 0x62u) + 1u));
    if ((int)PE_LoadU16(base + 0x62u) != n)
        PE_StoreU32(a0, PE_LoadU32(a0 + PE_LoadU16(a0 + 0xCEu) * 4u + 4u));
    else
        PE_StoreU16(a0 + 0xCEu, (uint16_t)((PE_LoadU16(a0 + 0xCEu) - 1u) & 3u));
}

/* src/func_800907DC.c / 8009086C.c: loop break on the last pass — when
 * counter + 1 == n (b0, 0 -> 0x100) jump by the s16 (b1 | b2 << 8)
 * (8009086C also pops the slot); otherwise skip the three operand bytes. */
static void loop_break(pe_addr_t a0, int pop)
{
    pe_addr_t p = PE_LoadU32(a0);
    int n = seq_next(a0);
    pe_addr_t base;

    if (n == 0)
        n = 0x100;
    base = a0 + PE_LoadU16(a0 + 0xCEu) * 2u;
    if ((int)PE_LoadU16(base + 0x62u) + 1 != n) {
        PE_StoreU32(a0, p + 3u);
    } else {
        unsigned int lo = seq_next(a0);
        unsigned int hi = seq_next(a0);

        PE_StoreU32(a0, PE_LoadU32(a0) + (uint32_t)(int32_t)(int16_t)(lo | (hi << 8)));
        if (pop)
            PE_StoreU16(a0 + 0xCEu, (uint16_t)((PE_LoadU16(a0 + 0xCEu) - 1u) & 3u));
    }
}
void func_800907DC(pe_addr_t a0) { loop_break(a0, 0); }
void func_8009086C(pe_addr_t a0) { loop_break(a0, 1); }

/* src/func_80090970.c: v = (s8) b; nonzero v is offset by (s16)+0xD0 and
 * clamped to [1, 0xFF]; +0xD2 = v (0 stays 0). */
void func_80090970(pe_addr_t a0)
{
    int v = (signed char)seq_next(a0);

    if (v != 0) {
        v += (int16_t)PE_LoadU16(a0 + 0xD0u);
        if (v <= 0)
            v = 1;
        else if (v >= 0x100)
            v = 0xFF;
    }
    PE_StoreU16(a0 + 0xD2u, (uint16_t)v);
}

/* src/func_800909C0.c: subroutine-style target — the s16 (b0 | b1 << 8)
 * relative to the cursor after it goes to +0x14; +0x6A = (s16)+0x46;
 * +0x38 |= 8. */
void func_800909C0(pe_addr_t a0)
{
    pe_addr_t p = PE_LoadU32(a0);
    int h = (int16_t)PE_LoadU16(a0 + 0x46u);
    unsigned int v1;
    pe_addr_t a2 = p + 2u;

    PE_StoreU32(a0, p + 1u);
    v1 = PE_LoadU8(p);
    PE_StoreU32(a0, a2);
    v1 |= (unsigned int)PE_LoadU8(p + 1u) << 8;
    PE_StoreU16(a0 + 0x6Au, (uint16_t)h);
    PE_StoreU32(a0 + 0x38u, PE_LoadU32(a0 + 0x38u) | 8u);
    PE_StoreU32(a0 + 0x14u, a2 + (uint32_t)(int32_t)(int16_t)v1);
}

/* src/func_80090A20.c: D_8009D2C8->+0x60 = b0, +0x5C = b1, +0x62 and
 * +0x5E cleared. */
void func_80090A20(pe_addr_t a0)
{
    pe_addr_t r = PE_LoadU32(0x8009D2C8u);
    unsigned int b;

    PE_StoreU16(r + 0x60u, seq_next(a0));
    b = seq_next(a0);
    PE_StoreU16(r + 0x62u, 0u);
    PE_StoreU16(r + 0x5Eu, 0u);
    PE_StoreU16(r + 0x5Cu, (uint16_t)b);
}

/* src/func_80090A64.c: D_8009D2C8->+0x64 = b0 | b1 << 8. */
void func_80090A64(pe_addr_t a0)
{
    pe_addr_t r = PE_LoadU32(0x8009D2C8u);

    PE_StoreU16(r + 0x64u, seq_next(a0));
    PE_StoreU16(r + 0x64u, (uint16_t)(PE_LoadU16(r + 0x64u) | ((unsigned int)seq_next(a0) << 8)));
}

/* src/func_80090B30.c / 80090BA0.c: +0xBA / +0xBC = b + 1 (0 -> 0x101). */
void func_80090B30(pe_addr_t a0)
{
    unsigned int v = seq_next(a0);

    PE_StoreU16(a0 + 0xBAu, (uint16_t)(v != 0 ? v + 1u : 0x101u));
}
void func_80090BA0(pe_addr_t a0)
{
    unsigned int v = seq_next(a0);

    PE_StoreU16(a0 + 0xBCu, (uint16_t)(v != 0 ? v + 1u : 0x101u));
}

extern void func_800903A0(pe_addr_t pe_a0, unsigned int a1);   /* generated TU */

/* The free-reverb-slot claim shared by src/func_80090D54.c / 80090E20.c:
 * unless +0x38 bit 11 is already set, take the lowest bit (of 24) clear in
 * D_8009D2C8->+4 | +0x30, mark it in +0x30, store its index at +0x5C and
 * set bit 11. */
static void claim_slot(pe_addr_t a0)
{
    pe_addr_t r = PE_LoadU32(0x8009D2C8u);
    uint32_t m, bit = 1u;
    int i = 0;

    if (PE_LoadU32(a0 + 0x38u) & 0x800u)
        return;
    m = PE_LoadU32(r + 4u) | PE_LoadU32(r + 0x30u);
    for (; bit & 0xFFFFFFu; bit <<= 1, i++)
        if (!(m & bit))
            break;
    if (bit & 0xFFFFFFu) {
        PE_StoreU32(r + 0x30u, PE_LoadU32(r + 0x30u) | bit);
        PE_StoreU16(a0 + 0x5Cu, (uint16_t)i);
        PE_StoreU32(a0 + 0x38u, PE_LoadU32(a0 + 0x38u) | 0x800u);
    }
}

/* src/func_80090D54.c: +0x5E = b << 8, +0x60 = 0, claim a slot, then
 * func_800903A0(a0, a1). */
void func_80090D54(pe_addr_t a0, int a1)
{
    PE_StoreU16(a0 + 0x5Eu, (uint16_t)(seq_next(a0) << 8));
    PE_StoreU16(a0 + 0x60u, 0u);
    claim_slot(a0);
    func_800903A0(a0, (unsigned int)a1);
}

/* src/func_80090E20.c: fade — steps b0 at +0x60 (0 -> 0x100), level +0x5E
 * keeps its high byte, step ((short)(b1 << 8) - level) / steps into
 * +0xD6; claim a slot, then func_800903A0(a0, a1). */
void func_80090E20(pe_addr_t a0, int a1)
{
    unsigned char c = seq_next(a0);
    int hi;

    PE_StoreU16(a0 + 0x60u, c);
    if (c == 0)
        PE_StoreU16(a0 + 0x60u, 0x100u);
    hi = PE_LoadU16(a0 + 0x5Eu) & 0xFF00;
    c = seq_next(a0);
    PE_StoreU16(a0 + 0xD6u, (uint16_t)(((short)(c << 8) - hi) / (int)PE_LoadU16(a0 + 0x60u)));
    PE_StoreU16(a0 + 0x5Eu, (uint16_t)hi);
    claim_slot(a0);
    func_800903A0(a0, (unsigned int)a1);
}

/* src/func_80090C88.c: two u16 little-endian offsets at the cursor (0 ->
 * NULL, else relative to the byte after each) name the zone and sample
 * tables; the D_800B89D0 note block gets +4/+8 = 0, +0xC = +0x76 >> 8,
 * +0x10 = +0x44 >> 23 (arithmetic); func_8008A92C(D_800B89D0, r1, r2) has
 * no pc_port implementation (boundary); cursor += 4. */
void func_80090C88(pe_addr_t a0)
{
    pe_addr_t p = PE_LoadU32(a0);
    unsigned int o = ((unsigned int)PE_LoadU8(p + 1u) << 8) | PE_LoadU8(p);
    pe_addr_t r1 = o != 0 ? p + o + 2u : 0u;
    pe_addr_t r2;

    p += 2u;
    o = ((unsigned int)PE_LoadU8(p + 1u) << 8) | PE_LoadU8(p);
    r2 = o != 0 ? p + o + 2u : 0u;
    PE_StoreU32(0x800B89D4u, 0u);
    PE_StoreU32(0x800B89D8u, 0u);
    PE_StoreU32(0x800B89DCu, PE_LoadU16(a0 + 0x76u) >> 8);
    PE_StoreU32(0x800B89E0u, (uint32_t)((int32_t)PE_LoadU32(a0 + 0x44u) >> 23));
    PE_D_COMP_BOUNDARY3("func_8008A92C", 0x8008A92Cu, 0x800B89D0u, r1, r2);
    PE_StoreU32(a0, PE_LoadU32(a0) + 4u);
}

extern void func_8008F0D0(pe_addr_t pe_a0, pe_addr_t pe_a1, int a2);   /* generated TU */

/* Program change shared by src/func_8008F898.c / 8008F9CC.c: program c
 * (+0x30 in bank-B mode for c >= 0x20 on the main track), instrument
 * record D_800B2900 + c * 0x40.  When a previous program is set and the
 * voice is not held (track != 0, or its bit of D_8009D2C8->+0x14 & a1 is
 * clear in D_800BCD50), rescale the pitch word +0x30 by new/old base rate
 * (unsigned; a zero old rate is retail's `break 7`, a loud boundary here)
 * and flag 0x10.  Then func_8008F0D0(a0, rec, attr) and clear +0x38 bit 12. */
static void program_change(pe_addr_t a0, unsigned int a1, int fixed_attr)
{
    unsigned int c = seq_next(a0);
    unsigned int old;
    pe_addr_t rec;

    if (PE_LoadU16(a0 + 0x54u) == 0u &&
        (PE_LoadU32(PE_LoadU32(0x8009D2C8u)) & 0x100u) && c >= 0x20u)
        c += 0x30u;
    rec = 0x800B2900u + c * 0x40u;
    old = PE_LoadU16(a0 + 0x5Au);
    PE_StoreU16(a0 + 0x5Au, (uint16_t)c);
    if (old != 0xFFu &&
        (PE_LoadU16(a0 + 0x54u) != 0u ||
         ((PE_LoadU32(PE_LoadU32(0x8009D2C8u) + 0x14u) & a1) & PE_LoadU32(0x800BCD50u)) == 0u)) {
        uint32_t den = PE_LoadU32(0x800B2910u + old * 0x40u);

        PE_StoreU32(a0 + 0xF4u, PE_LoadU32(a0 + 0xF4u) | 0x10u);
        if (den == 0u) {
            PE_D_COMP_BOUNDARY2("program_change_divide_trap", 0u, a0, old);
        } else {
            PE_StoreU32(a0 + 0x30u, (PE_LoadU32(a0 + 0x30u) * PE_LoadU32(rec + 0x10u)) / den);
        }
    }
    func_8008F0D0(a0, rec, fixed_attr ? 0x1010 : (int)PE_LoadU32(rec));
    PE_StoreU32(a0 + 0x38u, PE_LoadU32(a0 + 0x38u) & ~0x1000u);
}
/* src/func_8008F898.c: attributes from the record's first word. */
void func_8008F898(pe_addr_t a0, unsigned int a1) { program_change(a0, a1, 0); }
/* src/func_8008F9CC.c: fixed attributes 0x1010. */
void func_8008F9CC(pe_addr_t a0, unsigned int a1) { program_change(a0, a1, 1); }

/* src/func_8008FD10.c: vibrato on (+0x38 bit 0).  Sub-tracks (+0x54 != 0)
 * clear the delay +0x88 and take an optional depth b0 (<< 8 into +0x94);
 * the main track takes b0 as the delay.  Rate b1 at +0x8C (0 -> 0x100),
 * waveform b2 at +0x90; depth +0x92 = (f * width) >> 7 with f = +0x94
 * bits 8..14 and width = u16 +0x30 (scaled by 15/256 unless bit 15);
 * +0x8A = delay, +0x8E = 1, +0x1C = D_8009C080[waveform]. */
void func_8008FD10(pe_addr_t a0)
{
    unsigned int t, f, prod, idx;
    int w;

    PE_StoreU32(a0 + 0x38u, PE_LoadU32(a0 + 0x38u) | 1u);
    if (PE_LoadU16(a0 + 0x54u) != 0u) {
        unsigned int v;

        PE_StoreU16(a0 + 0x88u, 0u);
        v = seq_next(a0);
        if (v != 0)
            PE_StoreU16(a0 + 0x94u, (uint16_t)(v << 8));
    } else {
        PE_StoreU16(a0 + 0x88u, seq_next(a0));
    }
    {
        unsigned int v = seq_next(a0);

        PE_StoreU16(a0 + 0x8Cu, (uint16_t)v);
        if (v == 0)
            PE_StoreU16(a0 + 0x8Cu, 0x100u);
    }
    w = PE_LoadU16(a0 + 0x30u);
    PE_StoreU16(a0 + 0x90u, seq_next(a0));
    t = PE_LoadU16(a0 + 0x94u);
    f = (t & 0x7F00u) >> 8;
    prod = (t & 0x8000u) == 0u ? f * (unsigned int)((w * 15) >> 8) : f * (unsigned int)w;
    idx = PE_LoadU16(a0 + 0x90u);
    PE_StoreU16(a0 + 0x92u, (uint16_t)(prod >> 7));
    PE_StoreU16(a0 + 0x8Au, PE_LoadU16(a0 + 0x88u));
    PE_StoreU16(a0 + 0x8Eu, 1u);
    PE_StoreU32(a0 + 0x1Cu, PE_LoadU32(0x8009C080u + idx * 4u));
}

/* src/func_8008FEFC.c: tremolo on (+0x38 bit 1) — as func_8008FD10 with
 * delay +0x9C (optional depth b0 << 8 into +0xA6 on sub-tracks), rate
 * +0xA0, waveform +0xA4; +0x9E = delay, +0xA2 = 1, +0x20 = D_8009C080[w]. */
void func_8008FEFC(pe_addr_t a0)
{
    unsigned int idx;

    PE_StoreU32(a0 + 0x38u, PE_LoadU32(a0 + 0x38u) | 2u);
    if (PE_LoadU16(a0 + 0x54u) != 0u) {
        unsigned int v;

        PE_StoreU16(a0 + 0x9Cu, 0u);
        v = seq_next(a0);
        if (v != 0)
            PE_StoreU16(a0 + 0xA6u, (uint16_t)(v << 8));
    } else {
        PE_StoreU16(a0 + 0x9Cu, seq_next(a0));
    }
    {
        unsigned int v = seq_next(a0);

        PE_StoreU16(a0 + 0xA0u, (uint16_t)v);
        if (v == 0)
            PE_StoreU16(a0 + 0xA0u, 0x100u);
    }
    idx = seq_next(a0);
    PE_StoreU16(a0 + 0xA4u, (uint16_t)idx);
    PE_StoreU16(a0 + 0x9Eu, PE_LoadU16(a0 + 0x9Cu));
    PE_StoreU16(a0 + 0xA2u, 1u);
    PE_StoreU32(a0 + 0x20u, PE_LoadU32(0x8009C080u + idx * 4u));
}

/* src/func_800904C4.c: transpose — c & 0xC0 adds (c & 0x3F) to the
 * current value mod 64, else sets c; the main track (+0x54 == 0) uses
 * D_8009D2C8->+0x5A, sub-tracks D_800BCD78; D_8009D2C4 |= 0x10. */
void func_800904C4(pe_addr_t a0)
{
    unsigned int c = seq_next(a0);

    if (PE_LoadU16(a0 + 0x54u) == 0u) {
        pe_addr_t b = PE_LoadU32(0x8009D2C8u) + 0x5Au;

        PE_StoreU16(b, (uint16_t)((c & 0xC0u) ? ((PE_LoadU16(b) + (c & 0x3Fu)) & 0x3Fu) : c));
    } else {
        PE_StoreU16(0x800BCD78u,
                    (uint16_t)((c & 0xC0u) ? ((PE_LoadU16(0x800BCD78u) + (c & 0x3Fu)) & 0x3Fu) : c));
    }
    PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) | 0x10u);
}

/* src/func_800906B4.c: +0x38 |= 0x200 first, then as func_80090614. */
void func_800906B4(pe_addr_t a0)
{
    PE_StoreU32(a0 + 0x38u, PE_LoadU32(a0 + 0x38u) | 0x200u);
    byte_param16(a0, 0x4400u, 0x116u);
}
