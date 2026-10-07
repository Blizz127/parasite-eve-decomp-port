/*
 * Hand adapters — AKAO nested-stream voice start and sequence-0 bank restore
 * (native-only pass, 2026-10-07).
 *
 * Line-for-line translations of the matched leaves
 *   src/func_8008A750.c  voice start for Seq_StartNestedStreams (func_8008A92C)
 *   src/func_8008AC40.c  bank restore for Seq_SelectPlaybackBank (func_8008AE94)
 * The generator rejects both ("pointer cast of non-D_ operand"), so until now
 * their generated callers reached them as boundaries; while the port ran the
 * retail AKAO tick in an interpreter that never mattered, but with the native
 * tick they are on the music path.  Guest objects are 32-bit guest addresses;
 * every access is a PE_Load/PE_Store at the leaf's own offset and width, in
 * the leaf's order.  The leaves' register pins and empty asm barriers only
 * steer the PSX scheduler and carry no host semantics.
 */
#include "pe_guest_decomp.h"

void func_80089F58(pe_addr_t pe_a0, int a1);
unsigned int func_80089FE0(pe_addr_t pe_a0, unsigned int a1);
void func_8008D820(pe_addr_t pe_arg0, pe_addr_t pe_arg1, unsigned int arg2);
void func_80089960(void);
void func_80089B28(void);
void func_80089CF0(void);

#define G32(a)     PE_LoadU32((pe_addr_t)(a))
#define S32(a, x)  PE_StoreU32((pe_addr_t)(a), (uint32_t)(x))
#define G16(a)     PE_LoadU16((pe_addr_t)(a))
#define S16(a, x)  PE_StoreU16((pe_addr_t)(a), (uint16_t)(x))

#define GA_D2C8      0x8009D2C8u   /* unsigned char *D_8009D2C8 (control record) */
#define GA_D2C4      0x8009D2C4u
#define GA_D2DC      0x8009D2DCu
#define GA_CDE8      0x8009CDE8u
#define GA_CD50      0x800BCD50u
#define GA_CD54      0x800BCD54u
#define GA_CD58      0x800BCD58u
#define GA_CD5C      0x800BCD5Cu
#define GA_CD60      0x800BCD60u
#define GA_CD6C      0x800BCD6Cu
#define GA_CD70      0x800BCD70u
#define GA_CD74      0x800BCD74u
#define GA_BANKSFX   0x800BC000u   /* D_800BC000 */
#define GA_BANK0     0x800B8AC0u   /* D_800B8AC0 */
#define GA_BANK1     0x800BA560u   /* D_800BA560 */
#define GA_SAVE_CTL  0x800B8968u   /* D_800B8968 */
#define GA_SAVE_BANK 0x800B6B80u   /* D_800B6B80 */
#define GA_B8994     0x800B8994u   /* int D_800B8994 */
#define GA_B89BC     0x800B89BCu   /* unsigned short D_800B89BC */
#define GA_B8F4      0x8009B8F4u   /* D_8009B8F4 (idle voice script) */
#define VOICE_SIZE   0x11Cu

/* ── src/func_8008A750.c ─────────────────────────────────────────────── */
void func_8008A750(pe_addr_t a0, pe_addr_t s, unsigned int m, int a3)
{
    pe_addr_t v = a0;
    unsigned int mask = m;
    unsigned int x;
    int w;
    unsigned int r0, r1;

    S32(v + 0x28u, G32(s + 4u));
    S32(v + 0x2Cu, G32(s + 8u));
    x = PE_LoadU8(s + 0xCu);
    S16(v + 0x78u, 0);
    S16(v + 0x76u, x << 8);
    w = (int)G32(s + 0x10u);
    S16(v + 0x56u, 2);
    S16(v + 0x58u, 1);
    S16(v + 0x54u, 1);
    S16(v + 0x74u, 0);
    S32(v + 0x50u, -2);
    S16(v + 0xD8u, (unsigned int)(w & 0x7F) << 8);
    func_80089F58(a0, a3);

    r0 = G32(GA_CD50);
    r1 = G32(GA_CD5C);
    r0 |= mask;
    r1 |= mask;
    S32(GA_CD50, r0);
    r0 = G32(GA_CD54);
    mask = ~mask;
    S32(GA_CD5C, r1);
    r1 = G32(GA_CD58);
    r0 &= mask;
    S32(GA_CD54, r0);
    r0 = G32(GA_CD6C);
    r1 &= mask;
    S32(GA_CD58, r1);
    r1 = G32(GA_CD70);
    r0 &= mask;
    S32(GA_CD6C, r0);
    r0 = G32(GA_CD74);
    r1 &= mask;
    S32(GA_CD70, r1);
    r1 = G32(GA_D2DC);
    r0 &= mask;
    r1 &= 2u;
    S32(GA_CD74, r0);
    if (r1) {
        pe_addr_t p = GA_BANKSFX;
        unsigned int bit = 0x2000000u;
        int c = 12;
        mask = 0x1000u;
        do {
            if ((G32(p + 0x2Cu) & bit) == 0) {
                S32(GA_CD50, G32(GA_CD50) & ~mask);
                S32(GA_CD60, G32(GA_CD60) | mask);
            }
            c -= 1;
            p += VOICE_SIZE;
            mask <<= 1;
        } while (c != 0);
    }
}

/* ── src/func_8008AC40.c ─────────────────────────────────────────────── */
void func_8008AC40(int arg0)
{
    int delta = arg0;
    pe_addr_t b, ctl, c2, c3;
    unsigned int bit = 1u, n = 0x18u, omask = 0x1FF93u, c4 = 4u;
    unsigned int cde, w0, w14, flags, r, inv, nd, w;

    func_8008D820(GA_SAVE_CTL, G32(GA_D2C8), 0x68u);
    func_8008D820(GA_SAVE_BANK, GA_BANK0, 0x1AA0u);
    b = GA_BANK0;

    ctl = G32(GA_D2C8);
    cde = G32(GA_CDE8);
    w0 = G32(ctl);
    w14 = G32(ctl + 0x14u);
    cde &= 0x100u;
    S32(ctl + 0x2Cu, delta);
    S32(ctl, w0 | cde);
    S32(ctl + 0x10u, w14);
    delta = delta - (int)G32(GA_B8994);
    S32(GA_D2C4, G32(GA_D2C4) | 0x90u);
    flags = G32(ctl + 4u);

    /* p = b + 0x58 throughout (both advance by one voice per pass). */
    do {
        if (flags & bit) {
            S32(b, G32(b) + (uint32_t)delta);
            S32(b + 0x14u, G32(b + 0x14u) + (uint32_t)delta);   /* p - 0x44 */
            S32(b + 0x04u, G32(b + 0x04u) + (uint32_t)delta);   /* p - 0x54 */
            S32(b + 0x08u, G32(b + 0x08u) + (uint32_t)delta);   /* p - 0x50 */
            S32(b + 0x0Cu, G32(b + 0x0Cu) + (uint32_t)delta);   /* p - 0x4C */
            S32(b + 0x10u, G32(b + 0x10u) + (uint32_t)delta);   /* p - 0x48 */
            S16(b + 0x56u, G16(b + 0x56u) + 2u);                /* p - 2 */
            S16(b + 0x58u, G16(b + 0x58u) + 2u);                /* p */
            S32(b + 0xF4u, G32(b + 0xF4u) | omask);             /* p + 0x9C */
            if (G32(ctl) & 0x100u) {
                unsigned int hv = G16(b + 0x5Au);               /* p + 2 */
                if (hv >= 0x20u)
                    S16(b + 0x5Au, hv + 0x30u);
            }
        } else {
            S16(b + 0x58u, 2);
            S16(b + 0x56u, c4);
            S32(b, GA_B8F4);
        }
        n--;
        b += VOICE_SIZE;
        bit <<= 1;
    } while (n != 0);

    c3 = G32(GA_D2C8);
    r = func_80089FE0(GA_BANK1, G32(c3 + 0x6Cu) & G32(c3 + 0x70u));
    c2 = G32(GA_D2C8);
    inv = ~r;
    S32(c2 + 0x18u, 0);
    nd = ~G32(GA_CD50) & 0xFFFFFFu;
    inv &= nd;
    S32(GA_CD5C, G32(GA_CD5C) | inv);
    func_80089960();
    func_80089B28();
    func_80089CF0();
    S16(GA_B89BC, 0);
    if (G32(GA_D2DC) & 1u) {
        c3 = G32(GA_D2C8);
        w = G32(c3 + 4u);
        S32(c3 + 4u, 0);
        S32(c3 + 0x1Cu, w);
    }
}
