/*
 * PE-CH3 — field message subsystem, ported for field-VM opcode 0xE9.
 * Authority: SHA-1-exact retail EXE
 * (build/extracted/disc1/SLUS_006.62, sha1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 *
 * Translated (not matching src/) from:
 *   41D10.s  func_800515C0 (0x38)          — message record +0xC setter
 *   3420.s   func_800629B0                 — "message queued" flag leaf
 *   48530.s  func_80057E14 (0xB8)          — window-id remap table
 *   42D94.s  func_8005270C (0x58)          — window fade target
 *   3BD84.s  func_8004BE4C (0xBC)          — "item get" window node pair
 *   3BD84.s  func_8004BCE8 (0x164)         — close/commit active window
 *   4CC98.s  func_8005D2B4 (0x440)         — 21-arm message dispatcher
 *   3420.s   func_80015C7C (0x130)         — field-VM opcode 0xE9
 *   field leaves func_80051684 (0x30), func_8004BF08, func_8005C144.
 *
 * func_80051510 already lives in func_80051980_port.c and is called, not
 * redefined.
 *
 * ARM DISCIPLINE. func_8005D2B4 is a 21-slot jump table: `jr $v0` after
 * `lw $v0,%lo(jtbl_800112DC)(at)`, with `addiu $a0,-0x44C` / `sltiu 0x15`.
 * The in-tree VM cannot execute a computed jump, so each arm is translated
 * explicitly. The five arms the game actually uses from `m0351i` module 1
 * (cmd 0x453 / 0x454 / 0x456 / 0x457 / 0x45E, decoded from the retail
 * package) are real ports. Every other arm reaches a subsystem that is not
 * translated yet; those arms are DOMAIN GUARDS: loud, one-shot, returning
 * retail's default-miss value 0. They are never a silent wrong answer and
 * are pinned by test_SEW26_arm_domain_guard.
 */
#include <stdio.h>
#include <string.h>
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

#define GM_D_8009D254 0x8009D254u
#define GM_D_8009D300 0x8009D300u
#define GM_D_8009CE00 0x8009CE00u
#define GM_D_8009D2A4 0x8009D2A4u
#define GM_D_8009D154 0x8009D154u
#define GM_D_8009D1A0 0x8009D1A0u
#define GM_D_800B0CD8 0x800B0CD8u
#define GM_D_800C0E06 0x800C0E06u
#define GM_D_800C0E08 0x800C0E08u
#define GM_D_800C0E20 0x800C0E20u
#define GM_D_8009D02C 0x8009D02Cu
#define GM_D_800A1FD4 0x800A1FD4u
#define GM_D_800A1E6E 0x800A1E6Eu
#define GM_D_800A18B4 0x800A18B4u
#define GM_D_800A18D8 0x800A18D8u
#define GM_D_800A18FC 0x800A18FCu
#define GM_D_800A18FC_END 0x800A191Cu
#define GM_D_80092314 0x80092314u
#define GM_D_8009D2AC 0x8009D03Cu   /* gp+0x2CC: D_8005D39C window-id base */
#define GM_D_8009D01C 0x8009D01Cu   /* gp+0x2AC: func_8005270C fade target */
#define GM_D_8009CF80 0x8009CF80u   /* gp+0x210: "item-get node live" flag */
#define GM_D_8009CF84 0x8009CF84u   /* gp+0x214: message state (2 = closing) */

/* The five `m0351i` cmd constants (script operand, decoded from the retail
 * package m0351i module 1). */
#define GM_CMD_READ_C06  0x453
#define GM_CMD_SET_RECORD 0x454
#define GM_CMD_READ_MAX   0x456
#define GM_CMD_SET_LIMIT  0x457
#define GM_CMD_CLOSE      0x45E
#define GM_CMD_BASE       0x44C

/* Retail's arm 0x456 passes `$sp+0x10` to func_800515F8 as a nullable
 * out-pointer. The native port keeps that within-call temporary in guest
 * scratch instead of a host stack address (which is never a guest address).
 * This is a storage-location substitution only; the value written and read
 * back is exactly retail's. */
#define GM_TEMP_OUT (0x1F800380u)

static int gm_arm_guard_reported[21];

/* One-shot, loud guard for an arm whose callee is not translated yet. */
static int gm_arm_guard(unsigned index, unsigned cmd)
{
    if (index < 21u && !gm_arm_guard_reported[index]) {
        gm_arm_guard_reported[index] = 1;
        fprintf(stderr,
            "[MSG] func_8005D2B4 arm cmd=0x%03X reaches an unported "
            "field-message subsystem; returning retail default 0\n",
            cmd);
    }
    return 0;
}

/* ---- func_800515C0: message record +0xC / D_800C0E08 setter ------------- */
int func_800515C0(uint32_t value)
{
    pe_addr_t record = PE_LoadU32(GM_D_8009D254);
    if (record) {
        record = PE_LoadU32(record);
        if (record) PE_StoreU16(record + 0xCu, (uint16_t)value);
    }
    PE_StoreU16(GM_D_800C0E08, (uint16_t)value);
    return 0;
}

/* ---- func_80051684: message record +0x8 = value << 16 ------------------- */
int func_80051684(uint32_t value)
{
    pe_addr_t record = PE_LoadU32(GM_D_8009D254);
    if (record) {
        record = PE_LoadU32(record);
        if (record) PE_StoreU32(record + 8u, value << 16);
    }
    return 0;
}

/* func_800629B0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800629B0_port.c (src/func_800629B0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* ---- func_80057E14: window-id remap table + func_80055760 --------------- */
int32_t func_80057E14(pe_addr_t list)
{
    int32_t count = 0;
    pe_addr_t out;
    uint32_t base, end;

    if (!list) {
        PE_StoreU32(0x8009D078u, 0u); /* gp+0x308 */
        return 0;
    }
    out = GM_D_800A1FD4;
    base = PE_LoadU32(GM_D_8009D2AC);
    end = base + 3u;
    while (count < 10) {
        int32_t id = (int16_t)PE_LoadU16(list);
        if (id == 0) break;
        if (id >= (int32_t)base && id < (int32_t)end) {
            int32_t remapped = id + 6 - (int32_t)base;
            PE_StoreU16(out, (uint16_t)(remapped + 0x200));
            PE_StoreU16(GM_D_800A1E6E + (uint32_t)remapped * 32u,
                        PE_LoadU16(list + 2u));
            out += 2u;
        } else {
            PE_StoreU16(out, (uint16_t)id);
            out += 2u;
        }
        count++;
        list += 4u;
    }
    PE_StoreU32(0x8009D04Cu, GM_D_800A1FD4);   /* gp+0x2DC */
    PE_StoreU32(0x8009D054u, (uint32_t)count); /* gp+0x2E4 */
    func_80055760();
    PE_StoreU32(0x8009D078u, (uint32_t)count); /* gp+0x308 */
    return count;
}

/* func_8005270C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005270C_port.c (src/func_8005270C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8004BF08: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004BF08_port.c (src/func_8004BF08.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* ---- func_8004BE4C: build the "item get" window node pair --------------- */
void func_8004BE4C(void)
{
    pe_addr_t parent = func_80062D2C(0x15u, 0u, 0u, 0u);
    pe_addr_t child = func_8006322C(0x2Fu, parent, parent);

    PE_StoreU32(parent + 0x30u, 0x8004BF40u);
    PE_StoreU32(parent + 0x2Cu, 0x8004C1E0u);
    PE_StoreU32(child + 0x30u, 0x8004FFF8u);
    PE_StoreU32(child + 0x44u, 0xFFFFFFFFu);
    PE_StoreU32(child + 0x18u, PE_LoadU32(child + 0x18u) + 0x44u);
    PE_StoreU32(child + 0x40u, PE_LoadU32(child + 0x40u) - 2u);
    PE_StoreU32(child + 0x1Cu, PE_LoadU32(child + 0x1Cu) + 2u);
    func_80062CB8(parent);
    PE_StoreU32(parent + 0x4Cu, GM_D_80092314);
    PE_StoreU32(GM_D_8009CF80, 1u);   /* gp+0x210 */
    func_8004BF08();
}

/* func_8005C144: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005C144_port.c (src/func_8005C144.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* ---- func_8004BCE8: close/commit the active message window -------------- */
void func_8004BCE8(int32_t arg0)
{
    int32_t i;
    int32_t delta;
    pe_addr_t record;

    func_80051510();

    /* Publish the window geometry words. All nine stores target $gp offsets
     * 0x278/0x27C/0x1F0/0x280/0x1FC/0x1F4/0x200/0x1F8/0x204; gp is
     * 0x8009CD70, giving the guest literals below. */
    PE_StoreU32(0x8009CFE8u, PE_LoadU32(0x800C0E00u));   /* +0x278 */
    PE_StoreU32(0x8009CFECu, PE_LoadU32(0x800C0E00u));   /* +0x27C */
    PE_StoreU32(0x8009CF60u, (uint32_t)PE_LoadU8(0x800C0E0Au)); /* +0x1F0 */
    PE_StoreU32(0x8009CFF0u, (uint32_t)PE_LoadU8(0x800C0E0Au)); /* +0x280 */
    PE_StoreU32(0x8009CF6Cu, (uint32_t)PE_LoadU8(0x800C0E0Au)); /* +0x1FC */
    PE_StoreU32(0x8009CF64u, (uint32_t)PE_LoadU16(GM_D_800C0E06)); /* +0x1F4 */
    PE_StoreU32(0x8009CF70u, (uint32_t)PE_LoadU16(GM_D_800C0E06)); /* +0x200 */
    PE_StoreU32(0x8009CF68u, PE_LoadU32(0x800C0E10u));   /* +0x1F8 */
    delta = (int32_t)PE_LoadU32(0x800C0E10u) + (arg0 < 0 ? 0 : arg0);
    PE_StoreU32(0x8009CF74u, (uint32_t)delta);           /* +0x204 */

    /* Seven signed halfword keys -> func_8005B91C(i, key, &D_800A18B4[i], 0);
     * the key lands in D_800A18D8[i] and D_800A18FC[i] is cleared.  Retail
     * stores D_800A18D8[i] in the call's delay slot, so publish before the
     * call; the D_800A18FC[i] clear follows it. */
    {
        pe_addr_t key_src = 0x800C0E28u;
        for (i = 0; i < 7; i++) {
            int32_t key = (int16_t)PE_LoadU16(key_src + (uint32_t)i * 2u);
            PE_StoreU32(GM_D_800A18D8 + (uint32_t)i * 4u, (uint32_t)key);
            func_8005B91C(i, key, GM_D_800A18B4 + (uint32_t)i * 4u, 0u);
            PE_StoreU32(GM_D_800A18FC + (uint32_t)i * 4u, 0u);
        }
    }

    /* The scalar-temp `unsigned int *out` at $sp+0x10 lives in guest scratch,
     * since a host stack address is not a guest address. */
    func_8005B91C(0, (int32_t)PE_LoadU32(GM_D_800A18D8), GM_TEMP_OUT, 0u);
    PE_StoreU32(GM_D_8009CF84, 2u);   /* gp+0x214 */
    func_80057E14(0u);
    func_8004BE4C();
    record = func_80062A34(1u, 0x15u);
    func_80063158(record, 0, 0x20);
    if (arg0 > 0) func_8005270C();
    func_8005C144();
}

/* ---- func_8005D2B4: message-system query/mutate dispatcher -------------- */
/* Audit item 27: the complete 21-command dispatcher, line-for-line from
 * src/func_8005D2B4.c (inventory capacity, slot counts, item/key-item
 * removal, counters).  D_800C0E00 layout: +0x06/+0x08 halfwords, +0x0B/+0x0C
 * bytes, list[50] shorts @+0x48, items[128] x 0x20 @+0xAC (b4 +4, hA +0xA),
 * keys[100] shorts @+0x10B8.  The leaf pins arg2 in $a1 ($5), so the
 * one-argument calls to func_8005C688 / func_80042CC4 pass arg2 as a1. */
unsigned int func_80052F70(void);
int func_80051E58(void);
int func_8005C688(int min_b, int min_c);
int32_t func_80057D30(int32_t index);
int func_8005CAEC(void);
void func_8005DE88(void);
void func_80048918(int a0, int a1, int a2);
void func_8004C594(void);
void func_80042CC4(int a0, int a1);
void func_80042EDC(void);
void func_80042F20(void);
void func_80053128(void);
void func_8004D288(void);
void func_8005CCA4(void);
void func_8005D020(void);
#define GM_PL        0x800C0E00u
#define GM_LIST      (GM_PL + 0x48u)
#define GM_ITEMS     (GM_PL + 0xACu)
#define GM_KEYS      (GM_PL + 0x10B8u)
#define GM_INV       0x8009D048u     /* short *D_8009D048 */
#define GM_INV_N     0x8009D050u     /* int D_8009D050 */

static void gm_select_carried(void)
{
    PE_StoreU32(GM_INV, GM_LIST);
    PE_StoreU32(GM_INV_N, func_80052F70());
    PE_StoreU32(0x8009D058u, 0x8009D05Cu);
    PE_StoreU32(0x8009D064u, 2u);
}

int func_8005D2B4(int32_t cmd, int32_t arg, int32_t arg2)
{
    pe_addr_t p, base, end;
    int32_t n, k;
    uint16_t v;

    switch (cmd) {
    case 1100:
        gm_select_carried();
        arg = 0;
        base = PE_LoadU32(GM_INV);
        end = base + PE_LoadU32(GM_INV_N) * 2u;
        for (p = base; p < end; p += 2u) arg += PE_LoadU16(p) != 0u;
        return arg;
    case 1101: {
        int32_t s = (int32_t)PE_LoadU8(0x800C0E0Cu) + func_80051E58();
        return s < 51 ? (int32_t)PE_LoadU8(0x800C0E0Cu) + func_80051E58() : 50;
    }
    case 1102:
        k = (int32_t)PE_LoadU32(0x8009D03Cu);
        arg2 = 0;
        if (arg >= k && arg < k + 3) {
            arg2 = PE_LoadU16(0x800A1E64u + (uint32_t)(arg - k) * 0x20u + 0xAu);
        } else {
            base = PE_LoadU32(GM_INV);
            end = base + PE_LoadU32(GM_INV_N) * 2u;
            for (p = base; p < end; p += 2u) {
                int32_t id;
                v = PE_LoadU16(p);
                id = ((uint32_t)(v - 0x100u) < 0x80u)
                         ? (int32_t)PE_LoadU8(GM_ITEMS + (uint32_t)((int16_t)v - 0x100) * 0x20u + 4u)
                         : (int32_t)(int16_t)v;
                arg2 += id == arg;
            }
        }
        return arg2;
    case 1103:
        PE_StoreU32(0x8009D0CCu, (uint32_t)arg);
        PE_StoreU32(0x8009D0D0u, (uint32_t)arg2);
        return 0;
    case 1104:
        return func_8005C688(arg, arg2);
    case 1105:
        PE_StoreU8(0x800C0E0Cu, (uint8_t)(arg < 51 ? arg : 50));
        if (PE_LoadU32(GM_INV) == GM_LIST)
            PE_StoreU32(GM_INV_N, func_80052F70());
        return 0;
    case 1106:
        return PE_LoadU16(GM_PL + 8u);
    case 1107:
        return PE_LoadU16(GM_PL + 6u);
    case 1108:
        (void)func_800515C0((uint32_t)arg);
        return 0;
    case 1109:
        return func_800515F8(0u);
    case 1110:
        (void)func_800515F8(GM_TEMP_OUT);
        return (int32_t)PE_LoadU32(GM_TEMP_OUT);
    case 1111:
        (void)func_80051684((uint32_t)arg);
        return 0;
    case 1112:
        gm_select_carried();
        base = PE_LoadU32(GM_INV);
        end = base + PE_LoadU32(GM_INV_N) * 2u;
        for (p = base; p < end; p += 2u)
            if ((int32_t)(int16_t)PE_LoadU16(p) == arg) break;
        n = (p < end) ? (int32_t)((p - base) / 2u) : -1;
        if (n < 0) return n;
        (void)func_80057D30(n);
        return 0;
    case 1113:
        (void)func_8005CAEC();
        func_8005DE88();
        func_80048918(0, -3, -1);
        func_8004C594();
        return 0;
    case 1114:
        func_80042CC4(arg, arg2);
        return 0;
    case 1115:
        func_80042EDC();
        return 0;
    case 1116:
        func_80042F20();
        return 0;
    case 1117:
        func_80053128();
        PE_StoreU8(0x800B0CE5u, 1u);
        func_8004D288();
        PE_StoreU8(GM_PL + 0xBu, (uint8_t)((uint32_t)PE_LoadU8(GM_PL + 0xBu) + 1u >= 100u
                                           ? 99u : (uint32_t)PE_LoadU8(GM_PL + 0xBu) + 1u));
        func_8005CCA4();
        return 0;
    case 1118:
        func_8004BCE8(arg);
        return 0;
    case 1119: {   /* remove_key */
        pe_addr_t s;
        for (s = GM_KEYS; s < GM_KEYS + 200u; s += 2u)
            if ((int32_t)(int16_t)PE_LoadU16(s) == arg) break;
        n = s >= GM_KEYS + 200u;
        if (!n) PE_StoreU16(s, 0u);
        return n;
    }
    case 1120:
        func_8005D020();
        return 0;
    }
    return 0;
}

/* ---- func_80015C7C: field-VM opcode 0xE9 -------------------------------- */
int func_80015C7C(pe_addr_t args)
{
    pe_addr_t task = PE_LoadU32(GM_D_8009D300);
    pe_addr_t out  = PE_LoadU32(args + 0xCu);

    if ((PE_LoadU16(task + 8u) & 0x20u) == 0u)
        PE_StoreU32(out, (uint32_t)func_8005D2B4(
            (int32_t)PE_LoadU32(PE_LoadU32(args)),
            (int32_t)PE_LoadU32(PE_LoadU32(args + 4u)),
            (int32_t)PE_LoadU32(PE_LoadU32(args + 8u))));

    if (func_800629B0() != 0) {
        if ((PE_LoadU16(task + 8u) & 0x20u) == 0u) {
            PE_StoreU32(GM_D_800B0CD8, PE_LoadU32(GM_D_800B0CD8) | 0x9000u);
            PE_StoreU16(task + 8u, (uint16_t)(PE_LoadU16(task + 8u) | 0x20u));
            func_80067CBC();
        } else {
            PE_StoreU32(GM_D_8009D1A0, PE_LoadU32(GM_D_8009D1A0) | 4u);
        }
        PE_StoreU32(GM_D_8009CE00, PE_LoadU32(GM_D_8009CE00) - 0x1Cu);
        PE_StoreU32(task + 0x10u, 1u);
        return 0;
    }

    if ((int16_t)PE_LoadU16(GM_D_8009D2A4) != 0) {
        PE_StoreU32(out, (uint32_t)(int32_t)(int16_t)PE_LoadU16(GM_D_8009D2A4));
        PE_StoreU16(task + 8u, (uint16_t)(PE_LoadU16(task + 8u) & 0xFFDFu));
    }
    return 1;
}
