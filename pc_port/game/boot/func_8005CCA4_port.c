/*
 * Phase 6E-B28 — func_8005CCA4: resource-table initialization.
 * Full implementation; func_80053D2C is translated in its own B29 unit.
 *
 * Executable 0x8005CCA4..0x8005D01F (223 words), all verified against the
 * SHA-exact retail exe (452fb033…) and executed instruction-by-instruction
 * by pc_port/tools/b28_oracle.py.
 *
 * gp base is 0x8009CD70 (proven three ways: func_800438C0's 0x180(gp) =>
 * D_8009CEF0; func_80052C6C's $gp+offset comments; the b28 oracle).  The
 * four $gp-relative state words this rung writes are the SAME authoritative
 * host globals that func_80052C6C reads/writes — no host/guest duplication:
 *   $gp+0x2D8 = D_8009D048   $gp+0x2E0 = D_8009D050
 *   $gp+0x2E8 = D_8009D058   $gp+0x2F4 = D_8009D064
 */
#include "psx_compat.h"

extern unsigned int func_80052F70(void);
extern pe_addr_t func_8005DB8C(int idx);
extern pe_addr_t func_8005DBAC(int arg);
extern int func_800438C0(int arg);
extern void Bootstrap_ReturnVoid(const char *symbol, const char *caller);
extern int func_80053D2C(int arg);
extern void func_80042C78(void);

/* ── Guest RAM addresses written directly by func_8005CCA4 ─────────────── */
#define GA_E00    0x800C0E00u
#define GA_E06    0x800C0E06u
#define GA_E08    0x800C0E08u
#define GA_E0A    0x800C0E0Au
#define GA_E0C    0x800C0E0Cu
#define GA_E20    0x800C0E20u
#define GA_E22    0x800C0E22u
#define GA_E24    0x800C0E24u
#define GA_E28    0x800C0E28u
#define GA_E40    0x800C0E40u
#define GA_EAC    0x800C0EACu
#define GA_1EAC   0x800C1EACu
#define GA_2024   0x800C2024u
#define GA_1E6E   0x800A1E6Eu
#define GA_1E8E   0x800A1E8Eu
#define GA_1EAE   0x800A1EAEu

/* $gp+0x2D8 base value used by the descending zero loop (retail:
 * lw $gp+0x2D8 ; addiu +0x62 ; sh 0 down to $gp+0x2D8). */
#define ZERO_LOOP_TOP_OFFSET  0x62u

void func_8005CCA4(void)
{
    pe_addr_t s0, s1, s3;
    int s2;
    int i;

    /* 8005CCA4..CCE4: seven-halfword init loop.  retail:
     *   v0 = *(u16*)func_8005DB8C(i) ; sh v0 -> 0x800C0E28 + 2*i */
    s1 = GA_E28;
    for (i = 0; i < 7; i++) {
        pe_addr_t p = func_8005DB8C(i);
        PE_StoreU16(s1, PE_LoadU16(p));
        s1 += 2u;
    }

    /* 8005CCE8..CCF0: s0 = func_8005DBAC(0) */
    s0 = func_8005DBAC(0);

    /* 8005CCF4..CD0C: v0 = *(u16*)s0 ; sh v0 -> 0x800C0E08 and 0x800C0E06 */
    {
        uint16_t hw = PE_LoadU16(s0);
        PE_StoreU16(GA_E08, hw);
        PE_StoreU16(GA_E06, hw);
    }

    /* 8005CD00: s1 = 0x800C0E06 + 0x42 = 0x800C0E48 (retail addiu s1,v1,0x42) */
    s1 = 0x800C0E06u + 0x42u;             /* 0x800C0E48 */

    /* 8005CD10..CD40 */
    {
        uint8_t b = PE_LoadU8(s0 + 7u);
        PE_StoreU32(GA_E24, 1u);          /* sw 1 -> 0x800C0E24 */
        PE_StoreU16(GA_1EAE, 0u);
        PE_StoreU16(GA_1E8E, 0u);
        PE_StoreU16(GA_1E6E, 0u);
        D_8009D048 = s1;                  /* sw s1 -> $gp+0x2D8 */
        PE_StoreU8(GA_E0C, b);            /* sb  a0 -> 0x800C0E0C */
    }

    s2 = 2;
    s3 = 0x8009D05Cu;

    /* 8005CD44..CD5C */
    D_8009D050 = func_80052F70();         /* sw v0 -> $gp+0x2E0 */
    D_8009D058 = s3;                      /* sw s3 -> $gp+0x2E8 */
    D_8009D064 = (unsigned int)s2;        /* sw s2 -> $gp+0x2F4 */

    /* 8005CD60..CD94 */
    {
        uint8_t b7 = PE_LoadU8(s0 + 7u);
        if (b7 >= 0x33u) b7 = 0x32u;      /* slti 0x33 clamp */
        PE_StoreU8(GA_E0C, b7);           /* sb v1 -> 0x800C0E0C */
        if (D_8009D048 != s1) {           /* bne (lw $gp+0x2D8) != s1 */
            D_8009D050 = func_80052F70();
        }
    }

    /* 8005CD98..CDB0 */
    D_8009D048 = s1;                      /* sw s1 -> $gp+0x2D8 */
    D_8009D050 = func_80052F70();         /* sw v0 -> $gp+0x2E0 */
    D_8009D058 = s3;                      /* sw s3 -> $gp+0x2E8 */
    D_8009D064 = (unsigned int)s2;        /* sw s2 -> $gp+0x2F4 */

    /* 8005CDB4..CDC4: descending zero loop.
     *   v1 = (lw $gp+0x2D8) + 0x62 = 0x800C0EAA
     *   50 halfwords stored down to $gp+0x2D8 value 0x800C0E48. */
    {
        pe_addr_t p = (pe_addr_t)D_8009D048 + ZERO_LOOP_TOP_OFFSET;  /* 0x800C0EAA */
        int count = 0x31;
        while (count >= 0) {
            PE_StoreU16(p, 0u);
            count--;
            p -= 2u;
        }
    }

    /* 8005CDC8..CDEC: 5 boundary calls */
    (void)func_80053D2C(0x44);
    (void)func_80053D2C(0x96);
    (void)func_80053D2C(0x3F);
    (void)func_80053D2C(1);
    (void)func_80053D2C(6);

    /* 8005CDF0..CEC4: first scan loop (records with byte6 != 9) */
    s0 = 0u;
    {
        pe_addr_t a1 = GA_EAC;
        pe_addr_t end = a1 + 0x1000u;
        pe_addr_t v1 = a1 + 5u;
        while (a1 < end) {
            uint8_t b0 = PE_LoadU8(a1);
            if (b0 != 0u) {
                uint8_t b6 = PE_LoadU8(v1 + 1u);
                if (b6 != 9u) {
                    uint8_t b5 = PE_LoadU8(v1);
                    if ((b5 & 0x10u) != 0u) {
                        pe_addr_t a2 = GA_1EAC;
                        if (a1 < a2) {
                            pe_addr_t base = a2 - 0x1000u;
                            int idx = (int)((a1 - base) >> 5) + 0x100;
                            pe_addr_t v = a2 + 0xD4u;
                            pe_addr_t a2end = a2 + 0x178u;
                            while (v < a2end) {
                                int16_t hw = (int16_t)PE_LoadU16(v);
                                if (hw == (int16_t)idx) {
                                    if (v < GA_2024) PE_StoreU16(v, 0u);
                                    break;
                                }
                                v += 2u;
                            }
                            s0 = (pe_addr_t)idx;
                        }
                        break;
                    }
                }
            }
            a1 += 0x20u;
            v1 += 0x20u;
        }
    }

    if (s0 != 0u) (void)func_80053D2C((int)s0);   /* retail a0 = found id (0x8005CED0) */

    /* 8005CED8..CFAC: second scan loop (records with byte6 == 9).  Retail
     * zeroes a0 at 0x8005CEEC and exits with s0 = a0, so s0 is 0 unless
     * this scan finds a record. */
    s0 = 0u;
    {
        pe_addr_t a1 = GA_EAC;
        pe_addr_t end = a1 + 0x1000u;
        pe_addr_t v1 = a1 + 5u;
        while (a1 < end) {
            uint8_t b0 = PE_LoadU8(a1);
            if (b0 != 0u) {
                uint8_t b6 = PE_LoadU8(v1 + 1u);
                if (b6 == 9u) {
                    uint8_t b5 = PE_LoadU8(v1);
                    if ((b5 & 0x10u) != 0u) {
                        pe_addr_t a2 = GA_1EAC;
                        if (a1 < a2) {
                            pe_addr_t base = a2 - 0x1000u;
                            int idx = (int)((a1 - base) >> 5) + 0x100;
                            pe_addr_t v = a2 + 0xD4u;
                            pe_addr_t a2end = a2 + 0x178u;
                            while (v < a2end) {
                                int16_t hw = (int16_t)PE_LoadU16(v);
                                if (hw == (int16_t)idx) {
                                    if (v < GA_2024) PE_StoreU16(v, 0u);
                                    break;
                                }
                                v += 2u;
                            }
                            s0 = (pe_addr_t)idx;
                        }
                        break;
                    }
                }
            }
            a1 += 0x20u;
            v1 += 0x20u;
        }
    }

    if (s0 != 0u) (void)func_80053D2C((int)s0);   /* retail a0 = found id (0x8005CFB8) */

    /* 8005CFB0..CFF4: retail sets v0 = 1 in the beq delay slot AND on the
     * fall-through path, so GA_E22 is written 1 unconditionally. */
    PE_StoreU8(GA_E22, 1u);
    PE_StoreU8(GA_E20, 0u);
    PE_StoreU16(GA_E40, 0x3Du);
    func_800438C0(0x3D);
    PE_StoreU32(GA_E00, 0u);
    PE_StoreU8(GA_E0A, 0u);

    func_80042C78();
}
