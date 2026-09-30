/*
 * func_8005E588: two menu draw/display environments and packet arenas.
 *
 * Raw body: 87 words / 0x15C, exe 0x8005E588–0x8005E6E3, file 0x4ED88,
 * live split asm/disc1/4ED88.s:12–100; all 87 instruction words
 * verified exact against the SHA-exact retail executable.
 *
 * Two call sites:
 *   1. func_800527C8 @0x800527FC (nop delay slot, no args, return ignored)
 *      — the sixth callee in dispatcher ROM order, now the strict frontier.
 *   2. func_8006E9A0 @0x8006EB1C (addu $s0,$zero,$zero in delay slot,
 *      no args, return ignored)
 *
 * Operation (ROM order):
 *   1. Load four guest pointers from D_800B0E38/3C/50/54 (set earlier
 *      by func_8006A8D4) and store them to D_800A21F4/226C/21F0/2268.
 *   2. Two SetDefDrawEnv calls (func_80074924) initializing DRAWENV
 *      structures at D_800A2180 and D_800A21F8.
 *   3. Set flags: D_800A2210=1, D_800A2198=1, D_800A2199/9A/9B=0,
 *      D_800A2211/12/13=0.
 *   4. Two SetDefDispEnv calls (func_800749D8) at D_800A21DC/A2254.
 *   5. Halfword stores: sh 8 at D_800A225E/21E6, sh 0xE0 at D_800A2262/21EA.
 *   6. $gp-relative stores: D_8009D128=0, D_8009D124=0, D_8009D12C=&D_800A2270.
 *   7. func_8005E968(0x00808080) — pack color.
 *   8. D_8009D130 = 0.
 *   9. func_8005F844(0).
 *  10. D_8009D134 = 0.
 *
 * NAM12 corrects an older mistranslation of the call at8005E664: it is
 * SetDefDispEnv, not SetDefDrawEnv. The larger wrong structure overwrote
 * the first bank's two arena pointers. Original instruction comparisons
 * now cover the complete initializer followed by selection of each bank.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_port_compat.h"

/* Retail $s0 base: D_800A21F4.  Offsets relative to $s0 are resolved
 * with retail $gp = 0x8009CD70. */
#define GA_S0_BASE      0x800A21F4u

/* ── $s0-relative addresses ─────────────────────────────────────────── */
#define GA_DRAWENV_A    (GA_S0_BASE - 0x74u)   /* 0x800A2180 */
#define GA_DRAWENV_B    (GA_S0_BASE + 0x04u)   /* 0x800A21F8 */
#define GA_DISPENV_A    (GA_S0_BASE - 0x18u)   /* 0x800A21DC */
#define GA_DISPENV_B    (GA_S0_BASE + 0x60u)   /* 0x800A2254 */

#define GA_FLAG_2210    (0x800A2210u)          /* sb 1 */
#define GA_FLAG_2198    (0x800A2198u)          /* sb 1 */
#define GA_FLAG_2199    (0x800A2199u)          /* sb 0 */
#define GA_FLAG_219A    (0x800A219Au)          /* sb 0 */
#define GA_FLAG_219B    (0x800A219Bu)          /* sb 0 */
#define GA_FLAG_2211    (0x800A2211u)          /* sb 0 */
#define GA_FLAG_2212    (0x800A2212u)          /* sb 0 */
#define GA_FLAG_2213    (0x800A2213u)          /* sb 0 */

#define GA_HW_225E      (0x800A225Eu)          /* sh 8 */
#define GA_HW_21E6      (0x800A21E6u)          /* sh 8 */
#define GA_HW_2262      (0x800A2262u)          /* sh 0xE0 */
#define GA_HW_21EA      (0x800A21EAu)          /* sh 0xE0 */

/* ── $gp-relative addresses ($gp = 0x8009CD70) ──────────────────────── */
#define GA_GP_3B8       (0x8009CD70u + 0x3B8u) /* D_8009D128  sw 0 */
#define GA_GP_3B4       (0x8009CD70u + 0x3B4u) /* D_8009D124  sw 0 */
#define GA_GP_3BC       (0x8009CD70u + 0x3BCu) /* D_8009D12C  sw &D_800A2270 */
#define GA_GP_3C0       (0x8009CD70u + 0x3C0u) /* D_8009D130  sw 0 */
#define GA_GP_3C4       (0x8009CD70u + 0x3C4u) /* D_8009D134  sw 0 */

#define GA_TABLE_2270   (0x800A2270u)

void func_8005E588(void)
{
    /* 1. Load guest pointers from func_8006A8D4 outputs and store
     *    to destination slots. */
    PE_StoreU32(GA_S0_BASE,       D_800B0E50);   /* $s0[0]     */
    PE_StoreU32(0x800A226Cu,      D_800B0E54);   /* $s0 + 0x78 */
    PE_StoreU32(0x800A21F0u,      D_800B0E38);   /* $s0 - 0x4  */
    PE_StoreU32(0x800A2268u,      D_800B0E3C);   /* $s0 + 0x74 */

    /* 2. Two SetDefDrawEnv calls — REAL SDK.
     *    func_80074924(env, x, y, w, h) with h on the stack at sp+0x10.
     *    DRAWENV_A: x=0, y=0, w=320, h=224 */
    func_80074924(GA_DRAWENV_A, 0, 0, 0x140, 0xE0);

    /*    DRAWENV_B: x=0, y=224, w=320, h=224 */
    func_80074924(GA_DRAWENV_B, 0, 0xE0, 0x140, 0xE0);

    /* 3. Flag byte stores. */
    PE_StoreU8(GA_FLAG_2210, 1);
    PE_StoreU8(GA_FLAG_2198, 1);
    PE_StoreU8(GA_FLAG_2199, 0);
    PE_StoreU8(GA_FLAG_219A, 0);
    PE_StoreU8(GA_FLAG_219B, 0);
    PE_StoreU8(GA_FLAG_2211, 0);
    PE_StoreU8(GA_FLAG_2212, 0);
    PE_StoreU8(GA_FLAG_2213, 0);

    /* 4. Two SetDefDispEnv calls — REAL SDK.
     *    func_800749D8(env, x, y, w, h) */
    func_800749D8(GA_DISPENV_A, 0, 0xE0, 0x140, 0xE0);
    func_800749D8(GA_DISPENV_B, 0, 0, 0x140, 0xE0);

    /* 5. Halfword stores. */
    PE_StoreU16(GA_HW_225E, 8);
    PE_StoreU16(GA_HW_21E6, 8);
    PE_StoreU16(GA_HW_2262, 0xE0);
    PE_StoreU16(GA_HW_21EA, 0xE0);

    /* 6. $gp-relative word stores. */
    PE_StoreU32(GA_GP_3B8, 0);
    PE_StoreU32(GA_GP_3B4, 0);
    PE_StoreU32(GA_GP_3BC, GA_TABLE_2270);

    /* 7. func_8005E968(0x00808080) — pack color. */
    func_8005E968(0x00808080u);

    /* 8. D_8009D130 = 0. */
    PE_StoreU32(GA_GP_3C0, 0);

    /* 9. func_8005F844(0) — (TRANSLATED B19b). */
    func_8005F844(0);

    /* 10. D_8009D134 = 0. */
    PE_StoreU32(GA_GP_3C4, 0);
}
