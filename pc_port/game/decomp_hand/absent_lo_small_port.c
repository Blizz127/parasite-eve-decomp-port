/*
 * Hand adapters — small matched leaves below 0x80060000 that
 * tools/analysis/gen_decomp_ports.py cannot derive mechanically (port_absent
 * lane).  Each body follows src/func_XXXXXXXX.c statement for statement:
 *
 *   * a guest pointer held in a local, or read out of guest RAM, is a 32-bit
 *     `pe_addr_t` loaded with PE_LoadU32 (retail `lw`), never a host pointer;
 *   * a data symbol D_XXXXXXXX is accessed at its retail address with the
 *     leaf's declared width and signedness;
 *   * era-only codegen hints in the leaf (`register ... asm("$N")` pins,
 *     empty `asm volatile` fences) carry no semantics and are dropped;
 *   * a function address argument is the callee's retail VMA.
 *
 * Nothing here is invented: where the leaf's C leaves a value
 * implementation-defined on a 64-bit host (shift counts, signed overflow),
 * the retail MIPS semantics are spelled out and noted at the site.
 */
#include "pe_guest_decomp.h"

/* Decomp-derived callees (pc_port/game/decomp/, no header prototype): the
 * generated definitions' host signatures. */
int func_800677A0(int a0, short a1, short a2);
short func_8006599C(int a0);

/* ── field-VM opcode handlers (D_800910A0 table) ───────────────────── */

/* src/func_80015AB8.c: forward (*(u32 *)a0[0], *(short *)a0[1],
 * *(short *)a0[2]) to func_800677A0. */
int func_80015AB8(pe_addr_t a0)
{
    pe_addr_t v0 = PE_LoadU32(a0 + 0u);
    pe_addr_t v1 = PE_LoadU32(a0 + 4u);
    pe_addr_t a2 = PE_LoadU32(a0 + 8u);

    func_800677A0((int)PE_LoadU32(v0), (short)PE_LoadU16(v1),
                  (short)PE_LoadU16(a2));
    return 1;
}

/* src/func_80017E68.c: *a0[1] = ((D_8009D2F0->+0x98 & w) ^ w) < 1, where
 * w = *a0[0] — i.e. 1 iff every bit of w is set in the state word. */
int func_80017E68(pe_addr_t a0)
{
    pe_addr_t o = PE_LoadU32(0x8009D2F0u);
    unsigned int v = PE_LoadU32(o + 0x98u);
    unsigned int w = PE_LoadU32(PE_LoadU32(a0));
    unsigned int notall = (v & w) ^ w;

    PE_StoreU32(PE_LoadU32(a0 + 4u), (unsigned int)(notall < 1u));
    return 1;
}

/* src/func_80018818.c: func_8006F6D4(*a0[0], 1, *a0[1], a0[2], a0[3], a0[4])
 * — the last three are the guest out-pointers themselves. */
int func_80018818(pe_addr_t a0)
{
    int x0 = (int)PE_LoadU32(PE_LoadU32(a0 + 0u));
    int x2 = (int)PE_LoadU32(PE_LoadU32(a0 + 4u));

    func_8006F6D4((unsigned int)x0, 1u, (unsigned int)x2,
                  PE_LoadU32(a0 + 8u), PE_LoadU32(a0 + 12u),
                  PE_LoadU32(a0 + 16u));
    return 1;
}

/* src/func_80018864.c: func_8006F820(*first, 0, *second). */
int func_80018864(pe_addr_t arg0)
{
    pe_addr_t first = PE_LoadU32(arg0);
    pe_addr_t second = PE_LoadU32(arg0 + 4u);

    func_8006F820(PE_LoadU32(first), 0u, PE_LoadU32(second));
    return 1;
}

/* src/func_80018B30.c: func_800679C4(*(short *)a0[0], *(short *)a0[1],
 * *(short *)a0[2]).  func_800679C4 has no pc_port implementation yet, so the
 * call is a loud boundary carrying the three guest arguments. */
int func_80018B30(pe_addr_t a0)
{
    pe_addr_t v0 = PE_LoadU32(a0 + 0u);
    pe_addr_t v1 = PE_LoadU32(a0 + 4u);
    pe_addr_t a2 = PE_LoadU32(a0 + 8u);

    (void)func_800679C4((int)(short)PE_LoadU16(v0),
                        (int)(short)PE_LoadU16(v1),
                        (int)(short)PE_LoadU16(a2));
    return 1;
}

/* src/func_80018B98.c: func_80065954(*first, *second). */
int func_80018B98(pe_addr_t arg0)
{
    pe_addr_t first = PE_LoadU32(arg0);
    pe_addr_t second = PE_LoadU32(arg0 + 4u);

    func_80065954(PE_LoadU32(first), PE_LoadU32(second));
    return 1;
}

/* src/func_80018C10.c: *(int *)a0[1] = func_8006599C(*(int *)a0[0]). */
int func_80018C10(pe_addr_t a0)
{
    pe_addr_t v0 = PE_LoadU32(a0);
    int r = func_8006599C((int)PE_LoadU32(v0));

    PE_StoreU32(PE_LoadU32(a0 + 4u), (unsigned int)r);
    return 1;
}

/* src/func_80019450.c: scale the state's +0x224 short (<<1) by *a0[0] and
 * store the high half of the 32-bit product at (*(state+0x1B4))+0x14.
 * The multiply wraps at 32 bits exactly like retail `mult`/`mflo`. */
int func_80019450(pe_addr_t a0)
{
    pe_addr_t p = PE_LoadU32(0x8009D2F0u);
    int n = (int)(short)PE_LoadU16(p + 0x224u) << 1;
    int v = (int)PE_LoadU32(PE_LoadU32(a0));
    pe_addr_t q = PE_LoadU32(p + 0x1B4u);
    int prod = (int)((unsigned int)n * (unsigned int)v);

    PE_StoreU16(q + 0x14u, (unsigned short)(short)(prod >> 16));
    return 1;
}

/* src/func_800194F8.c: *(int *)a0[1] = func_80053E6C(*(int *)a0[0]). */
int func_800194F8(pe_addr_t a0)
{
    pe_addr_t v0 = PE_LoadU32(a0);
    int r = func_80053E6C((int)PE_LoadU32(v0));

    PE_StoreU32(PE_LoadU32(a0 + 4u), (unsigned int)r);
    return 1;
}

/* src/func_80019A1C.c: copy **a0 (short) to state+0x24E, the low bytes of
 * *a0[1..3] to +0x24B/+0x24C/+0x24D, then state+0x250 |= 8. */
int func_80019A1C(pe_addr_t a0)
{
    PE_StoreU16(PE_LoadU32(0x8009D2F0u) + 0x127u * 2u,
                (unsigned short)(short)PE_LoadU32(PE_LoadU32(a0 + 0u)));
    PE_StoreU8(PE_LoadU32(0x8009D2F0u) + 0x24Bu,
               (unsigned char)PE_LoadU32(PE_LoadU32(a0 + 4u)));
    PE_StoreU8(PE_LoadU32(0x8009D2F0u) + 0x24Cu,
               (unsigned char)PE_LoadU32(PE_LoadU32(a0 + 8u)));
    PE_StoreU8(PE_LoadU32(0x8009D2F0u) + 0x24Du,
               (unsigned char)PE_LoadU32(PE_LoadU32(a0 + 12u)));
    {
        pe_addr_t s = PE_LoadU32(0x8009D2F0u) + 0x128u * 2u;
        PE_StoreU16(s, (unsigned short)(PE_LoadU16(s) | 8u));
    }
    return 1;
}

/* src/func_8001A0CC.c: *(int *)*(arg0 + 4) = func_80077CF4(**arg0) << 4. */
int func_8001A0CC(int arg0)
{
    pe_addr_t a = (pe_addr_t)arg0;
    int v = func_80077CF4((int32_t)PE_LoadU32(PE_LoadU32(a)));

    PE_StoreU32(PE_LoadU32(a + 4u), (unsigned int)v << 4);
    return 1;
}

/* src/func_8001A114.c: *(int *)*(arg0 + 4) = func_80077DC4(**arg0) << 4. */
int func_8001A114(int arg0)
{
    pe_addr_t a = (pe_addr_t)arg0;
    int v = func_80077DC4((int32_t)PE_LoadU32(PE_LoadU32(a)));

    PE_StoreU32(PE_LoadU32(a + 4u), (unsigned int)v << 4);
    return 1;
}

/* src/func_8001A43C.c: **a0 = func_8005401C(). */
int func_8001A43C(pe_addr_t a0)
{
    int r = func_8005401C();

    PE_StoreU32(PE_LoadU32(a0), (unsigned int)r);
    return 1;
}

/* src/func_8001A474.c: **a0 = func_80052F70(). */
int func_8001A474(pe_addr_t a0)
{
    int r = (int)func_80052F70();

    PE_StoreU32(PE_LoadU32(a0), (unsigned int)r);
    return 1;
}

/* ── small helpers ─────────────────────────────────────────────────── */

/* src/func_80038CE4.c: base = D_80091A28; return base[4 + base[0x1D + (a0 & 0xFF)]]. */
/* func_80038CE4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80038CE4_port.c (src/func_80038CE4.c); hand port retired (port3 switch-over A). */

/* src/func_8004D690.c: func_8005E8A4(0, 2);
 * func_80062A7C(*(int *)(*(int *)(arg0 + 4) + 0x24) - 0x11). */
void func_8004D690(int arg0)
{
    func_8005E8A4(0, 2);
    func_80062A7C((uint32_t)((int)PE_LoadU32(
        PE_LoadU32((pe_addr_t)arg0 + 4u) + 0x24u) - 0x11));
}

/* src/func_8004EC3C.c: func_80052E30(0); func_800638D8(a0, func_80050690). */
/* func_8004EC3C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004EC3C_port.c (src/func_8004EC3C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_8004EC78.c: func_80052E30(1); func_800638D8(a0, func_800506E8). */
/* func_8004EC78: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004EC78_port.c (src/func_8004EC78.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_8004EF30.c: func_800638D8(slot, func_80050708). */
/* func_8004EF30: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004EF30_port.c (src/func_8004EF30.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_80050038.c: func_800638D8(slot, func_80050DC0). */
/* func_80050038: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050038_port.c (src/func_80050038.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_80050060.c: func_800638D8(slot, func_80050E70). */
/* func_80050060: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050060_port.c (src/func_80050060.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_8005421C.c: return func_8005DB44(value - 1)[6]. */
unsigned char func_8005421C(int value)
{
    return PE_LoadU8(func_8005DB44((unsigned int)(value - 1)) + 6u);
}

/* src/func_8005D940.c: decode D_8009D280 into an 8-byte stack name with
 * func_8006E2D0, then hand that name to func_8006E454.  The stack buffer is
 * the lane's guest temp (see PE_HAND_LO_STACK_TEMP).
 *
 * The leaf is declared `void`, but its caller func_80040210 uses the result:
 * retail returns with func_8006E454's $v0 untouched (pe_dis.sh 0x8005D940:
 * `jal 0x8006e454` is followed only by the epilogue), so the host adapter
 * returns that value explicitly. */
int func_8005D940(void)
{
    const pe_addr_t buf = PE_HAND_LO_STACK_TEMP;

    func_8006E2D0(buf, PE_LoadU32(0x8009D280u));
    return func_8006E454(buf);
}

/* src/func_8005DE70.c: return D_800A8044 + (int)(&D_800A8044 - 0x1C)
 * = D_800A8044 + 0x800A8028 (the record-array base, a guest address). */
int func_8005DE70(void)
{
    int value = (int)PE_LoadU32(0x800A8044u);

    return (int)((unsigned int)value + (0x800A8044u - 0x1Cu));
}

/* ── operand-struct VM handlers (E18: guest layout, 4-byte pointers) ─── */

/* src/func_80017A24.c: Arguments { u32 *source; u32 *bit; u32 *destination; }
 * at +0/+4/+8 (32-bit guest pointers): *destination = *source & (1 << *bit).
 * Retail `sllv` uses the low five bits of the count. */
int func_80017A24(pe_addr_t arg0)
{
    unsigned int src = PE_LoadU32(PE_LoadU32(arg0 + 0u));
    unsigned int bit = PE_LoadU32(PE_LoadU32(arg0 + 4u));

    PE_StoreU32(PE_LoadU32(arg0 + 8u), src & (1u << (bit & 31u)));
    return 1;
}

/* src/func_80017C8C.c: Arguments { short *first; short *second;
 * u16 *third; } at +0/+4/+8: func_800661EC(*first, *second, *third, 8). */
int func_80017C8C(pe_addr_t arg0)
{
    short first = (short)PE_LoadU16(PE_LoadU32(arg0 + 0u));
    short second = (short)PE_LoadU16(PE_LoadU32(arg0 + 4u));
    unsigned short third = PE_LoadU16(PE_LoadU32(arg0 + 8u));

    (void)func_800661EC(first, second, third, 8u);
    return 1;
}
