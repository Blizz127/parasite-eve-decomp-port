/*
 * port_absent lane (2026-09-23) — tests for the cd group
 * (pc_port/game/decomp_hand/absent_cd_port.c).  Expectations are computed
 * from the matched leaf src/func_XXXXXXXX.c named in each assertion, never
 * from the port.  Guest scratch: 0x8015C000..0x8015FFFF.
 *
 * Paths that retail can only leave through hardware progress are not driven
 * here (they would spin on the host): func_80082444's a0 == 1 arm (reaches
 * func_8007BF44's SIO poll) and func_80082B70's func_80083014 stepping loop
 * (it ends only when a transfer-step callback advances D_8009B764).
 */
#include "pe_guest_decomp.h"   /* brings game/decomp_hand/hand_absent_cd_protos.h */
#include "pe_sio0.h"
#include "pe_rcnt2.h"

#define PACD_S   0x8015C000u   /* generic record / fixture base */
#define PACD_R   0x8015C400u   /* second record                 */
#define PACD_SIO 0x8015C800u   /* SIO register block stand-in   */
#define PACD_K   0x8015C900u   /* kernel word pair (D_8009B784) */
#define PACD_B   0x8015CA00u   /* byte buffers                  */
#define PACD_UNK 0x80099990u   /* a code address with no host implementation */

static int pacd_call(int i, const char *sym, uint32_t target,
                     uint32_t a0, uint32_t a1, uint32_t a2)
{
    if (i >= g_bootstrap_arg4_call_count)
        return 0;
    return strcmp(g_bootstrap_arg4_calls[i].symbol, sym) == 0 &&
           (uint32_t)g_bootstrap_arg4_calls[i].target == target &&
           (uint32_t)g_bootstrap_arg4_calls[i].arg0 == a0 &&
           (uint32_t)g_bootstrap_arg4_calls[i].arg1 == a1 &&
           (uint32_t)g_bootstrap_arg4_calls[i].arg2 == a2;
}

static void pacd_reset(void)
{
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();
}

/* SIO / kernel-word fixture used by func_800834E8 and func_80083578. */
/* SIO0 / I_STAT at their retail addresses (pe_sio0.h model, no card):
 * used where the real func_800830DC / func_800832B4 (sio_port.c) run. */
static void pacd_sio_real(void)
{
    PE_Sio0_Reset();
    PE_Rcnt2_Reset();
    PE_StoreU32(0x8009B788u, 0x1F801040u);
    PE_StoreU32(0x8009B784u, 0x1F801070u);
    PE_StoreU16(0x1F80104Au, 0x1003u);
    PE_StoreU32(PACD_S + 0x3Cu, PACD_B + 0x300u);
    PE_StoreU32(PACD_S + 0x40u, PACD_B + 0x3F0u);
}

static void pacd_sio_fixture(uint16_t stat)
{
    PE_StoreU32(0x8009B788u, PACD_SIO);
    PE_StoreU32(0x8009B784u, PACD_K);
    PE_StoreU16(PACD_SIO + 4u, stat);
}

static void test_PA_cd_spu_and_small(void)
{
    TEST("PA_cd_spu_and_small");
    pacd_reset();

    /* func_8007D614: D_8009B44C != 0 skips the busy-wait; +0x1AA &= 0xFFCF;
     * no callback -> func_80073A34(0xF0000009, 0x20). */
    PE_StoreU32(0x8009B44Cu, 1u);
    PE_StoreU32(0x8009B3FCu, PACD_S);
    PE_StoreU16(PACD_S + 0x1AAu, 0xFFFFu);
    func_8007D614();
    ASSERT(PE_LoadU16(PACD_S + 0x1AAu) == 0xFFCFu, "8007D614 clears bits 4-5 only");
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           pacd_call(0, "func_80073A34", 0x80073A34u, 0xF0000009u, 0x20u, 0u),
           "8007D614 no-callback arm posts (0xF0000009, 0x20)");
    /* with a callback: no func_80073A34, the slot is called with no args */
    pacd_reset();
    PE_StoreU32(0x8009B44Cu, 1u);
    PE_StoreU32(0x8009B3FCu, PACD_S);
    PE_StoreU32(0x8009B434u, PACD_UNK);
    func_8007D614();
    ASSERT(PE_Decomp_BoundaryCount() == 1 &&
           strcmp(PE_Decomp_BoundaryName(0), "D_8009B434 (SPU DMA callback)") == 0 &&
           g_bootstrap_arg4_call_count == 0,
           "8007D614 calls D_8009B434() with no argument");

    /* func_80080F64: (a0 & 0xFF) == 2 -> func_80081D74(func_80080F98, -1) */
    pacd_reset();
    func_80080F64(0x103);
    ASSERT(g_bootstrap_arg4_call_count == 0, "80080F64 low byte 3 is a no-op");
    PE_StoreU32(0x8009B70Cu, 0u);                /* func_80081D74 idle */
    func_80080F64(0x7702);
    /* the real func_80081D74 (src/func_80081D74.c) records its arguments at
     * D_8009B70C - 24 / - 16 and marks the job busy. */
    ASSERT(PE_LoadU32(0x8009B70Cu - 24u) == 0x80080F98u &&
           PE_LoadU32(0x8009B70Cu - 16u) == 0xFFFFFFFFu && PE_LoadU32(0x8009B70Cu) == 1u,
           "80080F64 passes func_80080F98's VMA and -1");

    /* func_80084B44: install B73C/B740/B744 */
    pacd_reset();
    func_80084B44();
    ASSERT(PE_LoadU32(0x8009B73Cu) == 0x80084B78u &&
           PE_LoadU32(0x8009B740u) == 0x80084F8Cu &&
           PE_LoadU32(0x8009B744u) == 0x80084C4Cu, "80084B44 hook VMAs");
    PASS();
}

static void test_PA_cd_poll(void)
{
    uint32_t t0;

    TEST("PA_cd_poll");
    /* func_80081110, p = &D_8009B6AC[1] = 0x8009B6B0. */
    pacd_reset();
    PE_StoreU32(0x8009B6ACu, 3u);          /* p[-1] */
    PE_StoreU32(0x8009B6B0u, 10u);         /* p[0]  */
    PE_StoreU32(0x8009B6B4u, 5u);          /* p[1]  */
    PE_StoreU32(0x8009B6C4u, 0x10000000u); /* p[5]: far deadline */
    PE_StoreU32(0x8009B6D0u, PACD_UNK);
    func_80081110();
    ASSERT(PE_LoadU32(0x8009B6B0u) == 22u, "80081110 p[0] += p[-1] << 2");
    ASSERT(PE_LoadU32(0x8009B6B4u) == 4u, "80081110 p[1]--");
    ASSERT(PE_Decomp_BoundaryCount() == 0, "80081110 pending, in time: return");

    /* countdown reaches 0: finish (func_80081268 clears D_8009B6CC) and
     * call D_8009B6D0(2, p[3]) */
    pacd_reset();
    PE_StoreU32(0x8009B6B4u, 1u);
    PE_StoreU32(0x8009B6BCu, 0x1234u);     /* p[3] */
    PE_StoreU32(0x8009B6C4u, 0x10000000u);
    PE_StoreU32(0x8009B6CCu, 7u);
    PE_StoreU32(0x8009B6D0u, PACD_UNK);
    func_80081110();
    ASSERT(PE_LoadU32(0x8009B6CCu) == 0u, "80081110 runs func_80081268");
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           pacd_call(0, "D_8009B6D0 (card poll callback)", PACD_UNK, 2u, 0x1234u, 0u),
           "80081110 callback gets (2, p[3])");

    /* timeout: p[5] + 0x4B0 < VSync(-1) -> p[1] = -1, callback gets 5 */
    pacd_reset();
    t0 = func_80073A44(-1);
    PE_StoreU32(0x8009B6B4u, 9u);
    PE_StoreU32(0x8009B6C4u, (uint32_t)((int)t0 - 0x4B0 - 1));
    PE_StoreU32(0x8009B6D0u, PACD_UNK);
    func_80081110();
    ASSERT(PE_LoadU32(0x8009B6B4u) == 0xFFFFFFFFu, "80081110 timeout forces -1");
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           (uint32_t)g_bootstrap_arg4_calls[0].arg0 == 5u,
           "80081110 timed-out callback code is 5");
    /* no callback installed: nothing called */
    pacd_reset();
    PE_StoreU32(0x8009B6B4u, 1u);
    PE_StoreU32(0x8009B6C4u, 0x10000000u);
    func_80081110();
    ASSERT(PE_Decomp_BoundaryCount() == 0, "80081110 null D_8009B6D0 is skipped");
    PASS();
}

static void test_PA_cd_tables(void)
{
    TEST("PA_cd_tables");
    /* func_800819D8: keys at D_800A3CB4 + i*0x2C, names at D_800A3CBC. */
    pacd_reset();
    PE_StoreU32(0x800A3CB4u, 7u);
    PE_StoreU32(0x800A3CB4u + 0x2Cu, 9u);
    ASSERT(func_800819D8(5, PACD_B) == -1, "800819D8 zero key ends the search");
    ASSERT(g_bootstrap_arg4_call_count == 0, "800819D8 no key match, no compare");
    /* key 9 at index 1: BIOS A(17h) strcmp (pe_bios_string.c) of a1 and
     * &name[1] -> equal -> 2; a different name -> no match (-1 at the end) */
    PE_StoreU8(PACD_B, 'M'); PE_StoreU8(PACD_B + 1u, 'C'); PE_StoreU8(PACD_B + 2u, 0u);
    PE_StoreU8(0x800A3CBCu + 0x2Cu, 'M'); PE_StoreU8(0x800A3CBDu + 0x2Cu, 'C');
    PE_StoreU8(0x800A3CBEu + 0x2Cu, 0u);
    ASSERT(func_800819D8(9, PACD_B) == 2, "800819D8 returns index + 1");
    PE_StoreU8(0x800A3CBDu + 0x2Cu, 'D');
    ASSERT(func_800819D8(9, PACD_B) != 2,
           "800819D8 compares (a1, &name[i]): different name, no match");

    /* func_8008214C, p = &D_8009B6EC[7] = 0x8009B708 */
    pacd_reset();
    func_8008214C(5, 77);
    ASSERT(PE_Decomp_BoundaryCount() == 0, "8008214C p[0] == 0 returns");
    PE_StoreU32(0x8009B708u, 1u);
    PE_StoreU32(0x8009B70Cu, 1u);
    PE_StoreU32(0x8009B700u, 0x80011111u);   /* p[-2] -> func_800824C8 */
    PE_StoreU32(0x8009B704u, 0x2222u);       /* p[-1] -> func_800824DC */
    PE_StoreU32(0x8009B6F4u, PACD_UNK);      /* p[-5] callback */
    func_8008214C(0x105 & 0xFF, 77);
    ASSERT(PE_LoadU32(0x800B8AB4u) == 0x80011111u &&
           PE_LoadU32(0x800B8AB8u) == 0x2222u, "8008214C installs p[-2], p[-1]");
    ASSERT(PE_LoadU32(0x8009B70Cu) == 0u, "8008214C clears p[1]");
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           pacd_call(0, "@8008214C:f", PACD_UNK, 5u, 77u, 0u), /* generated TU, round 6 */
           "8008214C callback gets (a0, a1, 0)");

    /* func_80082444, a0 != 1: D_800B28F8 = 2, func_80081DF8 (clears
     * D_8009B70C when it is not 1) */
    pacd_reset();
    PE_StoreU32(0x8009B70Cu, 5u);
    func_80082444(0);
    ASSERT(PE_LoadU32(0x800B28F8u) == 2u, "80082444 other a0 -> 2");
    ASSERT(PE_LoadU32(0x8009B70Cu) == 0u, "80082444 tail-calls func_80081DF8");

    /* func_80082778 / func_8008284C through D_8009B738 = func_80084B20:
     * record 0x800A5B70 + (port & 0xF0 ? 0xF0 : 0). */
    pacd_reset();
    PE_StoreU32(0x8009B738u, 0x80084B20u);
    PE_StoreU8(0x800A5C60u + 0xE9u, 3u);
    PE_StoreU32(0x800A5C60u + 4u, PACD_B);
    for (unsigned k = 0; k < 15u; k++)
        PE_StoreU8(PACD_B + k, (uint8_t)(0x10u + k));
    ASSERT(func_80082778(0x10, -1, 0) == 3, "80082778 a1 < 0 -> count +0xE9");
    ASSERT(func_80082778(0x10, 3, 1) == 0, "80082778 a1 >= count -> 0");
    ASSERT(func_80082778(0x10, 1, 2) == 0x16, "80082778 row 1 byte 1");
    ASSERT(func_80082778(0x10, 2, 5) == 0x1E, "80082778 row 2 byte 4");
    ASSERT(func_80082778(0x10, 1, 6) == 0 && func_80082778(0x10, 1, 0) == 0,
           "80082778 a2 outside 1..5 -> 0");
    ASSERT(func_80082778(0x00, -1, 0) == 0, "80082778 port 0 record");

    PE_StoreU8(0x800A5B70u + 0xEAu, 2u);
    PE_StoreU32(0x800A5B70u + 8u, PACD_R);
    PE_StoreU8(PACD_R + 8u, 3u);                  /* row 1 count */
    PE_StoreU32(PACD_R + 8u + 4u, PACD_B + 0x40u);
    PE_StoreU8(PACD_B + 0x42u, 0xAB);
    ASSERT(func_8008284C(0, -1, 0) == 2, "8008284C a1 < 0 -> +0xEA");
    ASSERT(func_8008284C(0, 2, 0) == 0, "8008284C a1 >= count -> 0");
    ASSERT(func_8008284C(0, 1, -1) == 3, "8008284C a2 < 0 -> row count");
    ASSERT(func_8008284C(0, 1, 3) == 0, "8008284C a2 >= row count -> 0");
    ASSERT(func_8008284C(0, 1, 2) == 0xAB, "8008284C row data byte");
    PASS();
}

static void test_PA_cd_ports(void)
{
    TEST("PA_cd_ports");
    /* func_800829C4 */
    pacd_reset();
    PE_StoreU32(0x8009B774u, 1u);
    PE_StoreU32(0x8009B778u, 0u);
    ASSERT(func_800829C4(0) == 0, "800829C4 cur = (0 << 1) | (1 == 0)");
    ASSERT(PE_LoadU32(0x8009B774u) == 1u, "800829C4 unchanged mask is a no-op");
    PE_StoreU32(0x8009B758u, PACD_R);
    PE_StoreU32(0x8009B728u, PACD_UNK);
    PE_StoreU32(0x800A5AC0u, 0x96u);
    PE_StoreU32(0x800A5AC4u, 0x95u);
    ASSERT(func_800829C4(3) == 0, "800829C4 returns the previous mask");
    ASSERT(PE_LoadU32(0x8009B774u) == 0u && PE_LoadU32(0x8009B778u) == 1u,
           "800829C4 enables both ports");
    ASSERT(PE_LoadU32(0x800A5AC0u) == 0u && PE_LoadU32(0x800A5AC4u) == 0u,
           "800829C4 zeroes both idle counters");
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           pacd_call(0, "@800829C4:D_8009B728", PACD_UNK, PACD_R, 0u, 0u),
           "800829C4 resets only the record idle >= 0x96");
    ASSERT(PE_LoadU32(0x8009B75Cu) == 1u, "800829C4 re-arms D_8009B75C");
    ASSERT(func_800829C4(0) == 3, "800829C4 cur = (1 << 1) | (0 == 0)");
    PASS();
}

static void test_PA_cd_irq(void)
{
    TEST("PA_cd_irq");
    /* func_80082B08 */
    pacd_reset();
    PE_StoreU32(0x8009B784u, PACD_K);
    ASSERT(func_80082B08() == 0, "80082B08 p[1] bit 0 clear -> 0");
    PE_StoreU32(PACD_K + 4u, 1u);
    ASSERT(func_80082B08() == 0, "80082B08 p[0] bit 0 clear -> 0");
    PE_StoreU32(PACD_K, 3u);
    ASSERT(func_80082B08() == 1 && PE_Decomp_BoundaryCount() == 0,
           "80082B08 null hook -> 1");
    PE_StoreU32(0x8009B74Cu, PACD_UNK);
    ASSERT(func_80082B08() == 1 && PE_Decomp_BoundaryCount() == 1 &&
           g_bootstrap_arg4_call_count == 0, "80082B08 hook called with no args");

    /* func_80082B70 guard paths (see header) */
    pacd_reset();
    PE_StoreU32(0x8009B774u, 1u);
    PE_StoreU32(0x8009B778u, 0u);
    PE_StoreU32(0x800A5AC0u, 0x95u);
    PE_StoreU32(0x800A5AC4u, 0x96u);
    PE_StoreU32(0x8009B75Cu, 1u);
    ASSERT(func_80082B70() == 0, "80082B70 returns 0");
    ASSERT(PE_LoadU32(0x8009B78Cu) == 1u, "80082B70 D_8009B78C = 1");
    ASSERT(PE_LoadU32(0x800A5AC0u) == 0x96u && PE_LoadU32(0x800A5AC4u) == 0x96u,
           "80082B70 idle counters saturate at 0x96");
    ASSERT(PE_LoadU32(0x8009B764u) == 0u && PE_Decomp_BoundaryCount() == 0,
           "80082B70 D_8009B774 > D_8009B778 skips the transfer");
    PE_StoreU32(0x8009B774u, 0u);
    PE_StoreU32(0x8009B778u, 1u);
    PE_StoreU32(0x800A5AC0u, 0x10u);
    PE_StoreU32(0x800A5AC4u, 0x10u);
    PE_StoreU32(0x8009B75Cu, 0u);
    (void)func_80082B70();
    ASSERT(PE_LoadU32(0x800A5AC0u) == 0x10u && PE_LoadU32(0x800A5AC4u) == 0x10u,
           "80082B70 enabled ports keep their counters");

    /* func_80082CF0 */
    pacd_reset();
    PE_StoreU32(0x8009B784u, PACD_K);
    PE_StoreU32(PACD_K + 4u, 0x10u);
    PE_StoreU32(0x8009B758u, PACD_R);
    PE_StoreU32(0x8009B728u, PACD_UNK);
    PE_StoreU32(0x800A5AC0u, 5u);
    PE_StoreU32(0x800A5AC4u, 6u);
    func_80082CF0();
    ASSERT(PE_LoadU32(PACD_K) == 0xFFFFFFFEu && PE_LoadU32(PACD_K + 4u) == 0x11u,
           "80082CF0 kernel words -2 and |= 1");
    ASSERT(PE_LoadU32(0x800A5AC0u) == 0u && PE_LoadU32(0x800A5AC4u) == 0u &&
           PE_LoadU32(0x8009B75Cu) == 1u, "80082CF0 zeroes counters, re-arms");
    ASSERT(g_bootstrap_arg4_call_count == 4 &&
           pacd_call(0, "func_8007E1F4", 0x8007E1F4u, 2u, 0x800A5AB0u, 0u) &&
           pacd_call(1, "func_8007E1E4", 0x8007E1E4u, 2u, 0x800A5AB0u, 0u) &&
           pacd_call(2, "D_8009B728 (card record reset)", PACD_UNK, PACD_R, 0u, 0u) &&
           pacd_call(3, "D_8009B728 (card record reset)", PACD_UNK, PACD_R + 0xF0u, 0u, 0u),
           "80082CF0 dequeue/enqueue then reset both records");
    PASS();
}

static void test_PA_cd_transfer(void)
{
    TEST("PA_cd_transfer");
    /* func_80082E00: idle port 0, count 0 -> -1 + done hooks; return 1 */
    pacd_reset();
    pacd_sio_fixture(0u);
    PE_StoreU32(0x8009B744u, PACD_UNK);
    PE_StoreU32(0x8009B748u, PACD_UNK + 4u);
    PE_StoreU32(PACD_S + 0x3Cu, PACD_B);
    PE_StoreU8(PACD_B, 0x55u);
    ASSERT(func_80082E00(PACD_S) == 1, "80082E00 completes -> 1");
    ASSERT(PE_LoadU16(PACD_SIO + 8u) == 0xDu && PE_LoadU16(PACD_SIO + 0xEu) == 0x88u &&
           PE_LoadU16(PACD_SIO + 0xAu) == 0x1003u, "80082E00 SIO setup, port 0 ctl");
    ASSERT(PE_LoadU32(0x8009B77Cu) == 0xFFFFFFFFu, "80082E00 count 0 -> -1");
    ASSERT(PE_LoadU8(PACD_B) == 0u, "80082E00 clears **(a0 + 0x3C)");
    ASSERT(PE_LoadU32(0x800BD02Cu) == 0x91u, "80082E00 RCNT2 timeout limit 0x91");
    ASSERT(g_bootstrap_arg4_call_count == 2 &&
           pacd_call(0, "D_8009B744 (card sector flush)", PACD_UNK, PACD_S, 0u, 0u) &&
           pacd_call(1, "D_8009B748 (card record done)", PACD_UNK + 4u, PACD_S, 0u, 0u),
           "80082E00 timer 0x91, then flush/done on the record");

    /* port 1 with 2 pending sectors, a0[0xE8] == 8, and a stuck SIO bit 9 */
    pacd_reset();
    pacd_sio_fixture(0x202u);
    PE_StoreU32(0x8009B764u, 1u);
    PE_StoreU32(0x8009B780u, 2u);
    PE_StoreU32(0x8009B744u, PACD_UNK);
    PE_StoreU32(0x8009B748u, PACD_UNK + 4u);
    PE_StoreU8(PACD_S + 0xE8u, 8u);
    PE_StoreU32(PACD_S + 0xCu, 0x80160000u);
    ASSERT(func_80082E00(PACD_S) == 0, "80082E00 SIO bit 9 stuck -> 0");
    ASSERT(PE_LoadU16(PACD_SIO + 0xAu) == (0x3003u | 0x10u), "80082E00 port 1 ctl | 0x10");
    ASSERT(PE_LoadU8(PACD_SIO) == 1u, "80082E00 writes 1 to the SIO data byte");
    ASSERT(PE_LoadU32(0x8009B780u) == 0xFFFFFFFFu, "80082E00 count drains to -1");
    ASSERT(PE_LoadU32(0x800BD02Cu) == 0x50u, "80082E00 RCNT2 timeout limit 0x50");
    ASSERT(g_bootstrap_arg4_call_count == 4 &&
           pacd_call(0, "D_8009B744 (card sector flush)", PACD_UNK, 0x80160000u + 240u, 0u, 0u) &&
           pacd_call(1, "D_8009B744 (card sector flush)", PACD_UNK, 0x80160000u, 0u, 0u) &&
           pacd_call(2, "D_8009B744 (card sector flush)", PACD_UNK, PACD_S, 0u, 0u),
           "80082E00 flushes sectors n-1 .. 0 then the record");

    /* bit 9 clears after the |= 0x10: *D_8009B784 = -0x81, a0[0x50] &&
     * a0[0x36] -> 0 */
    pacd_reset();
    pacd_sio_fixture(0u);
    PE_StoreU32(0x8009B77Cu, 0xFFFFFFFFu);      /* n < 0: no hooks */
    PE_StoreU8(PACD_S + 0x50u, 1u);
    PE_StoreU8(PACD_S + 0x36u, 1u);
    ASSERT(func_80082E00(PACD_S) == 0, "80082E00 a0[0x50] && a0[0x36] -> 0");
    ASSERT(g_bootstrap_arg4_call_count == 0, "80082E00 negative count: no hooks");

    /* func_80083014: step D_8009B7A8[0] with the record; func_800834E8 */
    pacd_reset();
    pacd_sio_fixture(0u);
    PE_StoreU32(0x8009B7A8u, PACD_UNK);
    PE_StoreU32(0x8009B724u, PACD_UNK + 8u);
    func_80083014(PACD_S);
    ASSERT(PE_LoadU32(0x8009B768u) == 1u, "80083014 post-increments the step");
    ASSERT(PE_LoadU32(PACD_K) == 0xFFFFFF7Fu && (PE_LoadU16(PACD_SIO + 0xAu) & 0x10u),
           "80083014 runs func_800834E8");
    ASSERT(PE_LoadU32(0x800BD02Cu) == 0x3Cu, "80083014 RCNT2 timeout limit 0x3C");
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           pacd_call(0, "D_8009B7A8[] (card transfer step)", PACD_UNK, PACD_S, 0u, 0u),
           "80083014 step then the 0x3C timer");
    /* step >= 5 is pulled back by one */
    pacd_reset();
    pacd_sio_fixture(0u);
    PE_StoreU32(0x8009B768u, 5u);
    PE_StoreU32(0x8009B7A8u + 20u, PACD_UNK);
    func_80083014(PACD_S);
    ASSERT(PE_LoadU32(0x8009B768u) == 5u, "80083014 step 6 -> 5");
    PASS();
}

static void test_PA_cd_records(void)
{
    TEST("PA_cd_records");
    /* func_800837C8 */
    pacd_reset();
    PE_StoreU32(0x8009B740u, 0x80084F8Cu);   /* func_80084F8C: busy query */
    ASSERT(func_800837C8(PACD_S, 0) == 0, "800837C8 a1 == 0 -> 0");
    PE_StoreU32(PACD_S + 4u, 1u);
    ASSERT(func_800837C8(PACD_S, 10) == 0, "800837C8 busy record -> 0");
    PE_StoreU32(PACD_S + 4u, 0u);
    /* func_80084F8C: !+0xE6 || +0x46 != 255 -> nonzero means busy */
    ASSERT(func_800837C8(PACD_S, 10) == 0 && PE_LoadU8(PACD_S + 0x46u) == 0u,
           "800837C8 D_8009B740 != 0 -> 0");
    PE_StoreU16(PACD_S + 0xE6u, 1u);
    PE_StoreU8(PACD_S + 0x46u, 0xFFu);
    PE_StoreU8(PACD_S + 0xE3u, 5u);
    PE_StoreU8(PACD_S + 0xE9u, 3u);
    PE_StoreU8(PACD_S + 0x47u, 9u);
    ASSERT(func_800837C8(PACD_S, 10) == 1, "800837C8 starts -> 1");
    ASSERT(PE_LoadU8(PACD_S + 0x49u) == 4u && PE_LoadU8(PACD_S + 0x46u) == 1u &&
           PE_LoadU8(PACD_S + 0x47u) == 0u, "800837C8 state bytes");
    ASSERT(PE_LoadU32(PACD_S + 0x14u) == 0x8008389Cu &&
           PE_LoadU32(PACD_S + 0x18u) == 0x80083944u, "800837C8 handler VMAs");
    /* ((10+3)>>2)<<2 = 12; + ((5+1)>>1)<<2 = 24; + ((3*5+3) & 0xFFC) = 40 */
    ASSERT(PE_LoadU32(PACD_S) == 12u && PE_LoadU32(PACD_S + 4u) == 24u &&
           PE_LoadU32(PACD_S + 8u) == 40u, "800837C8 cumulative sizes");

    /* func_80083DF0 */
    pacd_reset();
    PE_StoreU8(PACD_S + 0x53u, 1u);
    PE_StoreU8(PACD_S + 0x46u, 2u);
    ASSERT(func_80083DF0(PACD_S) == 1, "80083DF0 +0x53 && state 2 -> 1");
    PE_StoreU8(PACD_S + 0x46u, 3u);
    ASSERT(func_80083DF0(PACD_S) == 0 && PE_LoadU8(PACD_S + 0x46u) == 0xFEu,
           "80083DF0 +0x53 other state -> 0xFE");
    PE_StoreU8(PACD_S + 0x53u, 0u);
    PE_StoreU32(0x8009B728u, PACD_UNK);
    ASSERT(func_80083DF0(PACD_S) == 0 &&
           pacd_call(0, "@80083DF0:(*host_D_8009B728)", PACD_UNK, PACD_S, 0u, 0u),
           "80083DF0 +0x53 clear -> D_8009B728(a0)");

    /* func_80083F04 */
    pacd_reset();
    pacd_sio_real();
    PE_StoreU32(0x8009B7A4u, 7u);
    PE_StoreU32(0x8009B73Cu, PACD_UNK);
    func_80083F04((int)PACD_S);
    ASSERT(PE_LoadU32(0x8009B7A4u) == 0u, "80083F04 D_8009B7A4 = D_8009B73C(a0)");
    /* then the real func_800830DC(a0, -2) (src/func_800830DC.c a1 < 0):
     * +0x44 = 0xFF, +0x45 = 1, *(+0x40) = ~-2 = 1, the byte ~a1 sent */
    ASSERT(pacd_call(0, "@80083F04:D_8009B73C", PACD_UNK, PACD_S, 0u, 0u) &&
           g_bootstrap_arg4_call_count == 1 &&
           PE_LoadU8(PACD_S + 0x44u) == 0xFFu && PE_LoadU8(PACD_S + 0x45u) == 1u &&
           PE_LoadU8(PACD_B + 0x3F0u) == 1u && (PE_LoadU16(0x1F801044u) & 2u),
           "80083F04 then func_800830DC(a0, -2)");

    /* func_8008401C: the real func_800832B4 (sio_port.c) collects the
     * previous send's byte and, with no card attached (no /ACK, pe_sio0.h),
     * times out on RCNT2 -> -2, which src/func_8008401C.c returns as is.
     * (Its -9 / nibble branches need a responding card: not modelled.) */
    pacd_reset();
    pacd_sio_real();
    PE_StoreU8(0x1F801040u, 0u);                 /* previous send */
    PE_StoreU32(0x8009B7A4u, 1u);
    PE_StoreU32(0x8009B73Cu, PACD_UNK);
    PE_StoreU32(0x8009B770u, 0x33u);
    PE_StoreU32(0x8009B79Cu, 0x77u);
    PE_StoreU32(PACD_S + 0xCu, 0x80160000u);
    ASSERT(func_8008401C(PACD_S) == -2, "8008401C no card: func_800832B4 -2 returned");
    ASSERT(PE_LoadU32(0x8009B79Cu) == 0x77u, "8008401C r < 0 leaves D_8009B79C");
    ASSERT(g_bootstrap_arg4_call_count == 2 &&
           pacd_call(0, "@8008401C:(*host_D_8009B73C)", PACD_UNK, 0x80160000u + 0x1E0u, 0u, 0u) &&
           pacd_call(1, "@8008401C:(*host_D_8009B73C)", PACD_UNK, 0x80160000u + 0x2D0u, 0u, 0u),
           "8008401C sectors +0x1E0/+0x2D0 before the exchange");
    ASSERT(PE_LoadU32(0x800BD02Cu) == 0x190u, "8008401C exchange armed RCNT2 0x190");
    pacd_reset();
    pacd_sio_real();
    PE_StoreU8(0x1F801040u, 0u);
    PE_StoreU8(PACD_S + 0x36u, 1u);
    PE_StoreU32(0x8009B770u, 0x33u);
    ASSERT(func_8008401C(PACD_S) == -2 && g_bootstrap_arg4_call_count == 0,
           "8008401C a0[0x36] != 0: no sector hooks, exchange times out");

    /* func_800840DC: flag = (nibble 8 && !a0[0x36]); the exchange times out
     * (no card) -> -2 returned (r < 0). */
    pacd_reset();
    pacd_sio_real();
    PE_StoreU8(0x1F801040u, 0u);
    PE_StoreU8(PACD_B + 0x300u, 0x81u);          /* **(a0 + 0x3C) */
    PE_StoreU32(0x8009B72Cu, PACD_UNK);
    ASSERT(func_800840DC(PACD_S) == -2, "800840DC no card -> -2");
    ASSERT(pacd_call(0, "D_8009B72C (card frame exchange)", PACD_UNK, PACD_S, 1u, 0u),
           "800840DC flag = (nibble 8 && !a0[0x36])");
    pacd_reset();
    pacd_sio_real();
    PE_StoreU8(0x1F801040u, 0u);
    PE_StoreU8(PACD_B + 0x300u, 0x71u);
    PE_StoreU32(0x8009B72Cu, PACD_UNK);
    (void)func_800840DC(PACD_S);
    ASSERT(pacd_call(0, "D_8009B72C (card frame exchange)", PACD_UNK, PACD_S, 0u, 0u),
           "800840DC nibble 7 -> flag 0");

    /* func_80084EB0 */
    pacd_reset();
    PE_StoreU32(PACD_S + 0x3Cu, PACD_B);
    PE_StoreU32(PACD_S + 0x30u, PACD_B + 0x10u);
    PE_StoreU8(PACD_B, 0xF3u);
    PE_StoreU8(PACD_S + 0xE8u, 9u);
    func_80084EB0(PACD_S);                         /* state 0, 0xF3 */
    ASSERT(PE_LoadU32(PACD_S + 0x4Cu) == 1u && PE_LoadU8(PACD_S + 0xE8u) == 9u,
           "80084EB0 tick; 0xF3 leaves the buffer");
    PE_StoreU8(PACD_B, 0x00u);
    func_80084EB0(PACD_S);
    ASSERT(PE_LoadU8(PACD_B + 0x10u) == 0xFFu && PE_LoadU8(PACD_B + 0x11u) == 0u &&
           PE_LoadU8(PACD_S + 0xE8u) == 0u, "80084EB0 tail writes {0xFF, 0}");
    PE_StoreU8(PACD_S + 0x46u, 1u);
    PE_StoreU8(PACD_S + 0x4Au, 1u);
    func_80084EB0(PACD_S);
    ASSERT(PE_LoadU8(PACD_S + 0x4Au) == 2u, "80084EB0 state 1 retry bump below 2");
    func_80084EB0(PACD_S);
    ASSERT(PE_LoadU8(PACD_S + 0x49u) == 2u && PE_LoadU8(PACD_S + 0x46u) == 0xFFu,
           "80084EB0 state 1 retries out -> +0x49 = 2, +0x46 = 0xFF");
    PE_StoreU8(PACD_S + 0x46u, 5u);
    PE_StoreU8(PACD_S + 0x4Au, 3u);
    func_80084EB0(PACD_S);
    ASSERT(PE_LoadU8(PACD_S + 0x4Au) == 4u, "80084EB0 other state bump below 4");
    PE_StoreU8(PACD_S + 0x49u, 1u);
    PE_StoreU32(0x8009B728u, PACD_UNK);
    func_80084EB0(PACD_S);
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           pacd_call(0, "D_8009B728 (card record reset)", PACD_UNK, PACD_S, 0u, 0u),
           "80084EB0 retries out with +0x49 -> D_8009B728(a0)");
    PASS();
}

static void test_PA_cd_multisector(void)
{
    TEST("PA_cd_multisector");
    /* func_80084168, flag 0, D_8009B79C = 1: straight to the tail. */
    pacd_reset();
    pacd_sio_fixture(0x2u);                  /* func_80083578 ready bit */
    PE_StoreU32(0x8009B730u, PACD_UNK);
    PE_StoreU32(0x8009B724u, PACD_UNK + 8u);
    PE_StoreU32(0x8009B79Cu, 1u);
    PE_StoreU32(0x8009B7A0u, PACD_B + 0x80u);
    PE_StoreU8(PACD_B + 0x80u, 0x5Au);
    PE_StoreU32(PACD_S + 0x3Cu, PACD_B);
    PE_StoreU8(PACD_S + 0x44u, 2u);
    ASSERT(func_80084168(PACD_S) == 0, "80084168 tail returns 0");
    ASSERT(PE_LoadU32(0x8009B79Cu) == 0u, "80084168 tail decrements D_8009B79C");
    ASSERT(PE_LoadU8(PACD_S + 0x44u) == 3u && PE_LoadU8(PACD_B + 2u) == 0x5Au,
           "80084168 appends *D_8009B7A0 at *(a0+0x3C) + old +0x44");
    ASSERT(g_bootstrap_arg4_call_count == 2 &&
           pacd_call(0, "D_8009B730 (card sector prepare)", PACD_UNK, PACD_S, 0u, 0u) &&
           pacd_call(1, "D_8009B724 (card error hook)", PACD_UNK + 8u, 0u, 0u, 0u),
           "80084168 prepare(a0) ... D_8009B724(0)");

    /* D_8009B79C = 2, D_8009B764 = 0 -> z = 1: record 1 (off 0xF0), its
     * count D_8009B780 = 0 -> is01: flush+done on D_8009B758 + 0xF0, -1. */
    pacd_reset();
    pacd_sio_fixture(0x2u);
    PE_StoreU32(0x8009B730u, PACD_UNK);
    PE_StoreU32(0x8009B72Cu, PACD_UNK + 0xCu);
    PE_StoreU32(0x8009B744u, PACD_UNK + 0x10u);
    PE_StoreU32(0x8009B748u, PACD_UNK + 0x14u);
    PE_StoreU32(0x8009B724u, PACD_UNK + 8u);
    PE_StoreU32(0x8009B758u, PACD_R);
    PE_StoreU32(0x8009B79Cu, 2u);
    PE_StoreU32(0x8009B7A0u, PACD_B + 0x80u);
    PE_StoreU32(PACD_S + 0x3Cu, PACD_B);
    /* the RAM stand-in plays a responding card: /ACK asserted (I_STAT bit 7
     * in the D_8009B784 word) so the real func_800830DC exchange completes */
    PE_StoreU32(PACD_K, 0x80u);
    ASSERT(func_80084168(PACD_S) == 0, "80084168 one loop2 pass then tail");
    ASSERT(PE_LoadU32(0x8009B780u) == 0xFFFFFFFFu, "80084168 is01 marks -1");
    ASSERT(PE_LoadU32(0x8009B79Cu) == 0u, "80084168 D_8009B79C 2 -> 1 -> 0");
    ASSERT(PE_LoadU32(PACD_K) == 0xFFFFFF7Fu, "80084168 polls func_800834E8");
    ASSERT(pacd_call(1, "D_8009B744 (card sector flush)", PACD_UNK + 0x10u, PACD_R + 0xF0u, 0u, 0u) &&
           pacd_call(2, "D_8009B748 (card record done)", PACD_UNK + 0x14u, PACD_R + 0xF0u, 0u, 0u) &&
           pacd_call(3, "D_8009B72C (card frame exchange)", PACD_UNK + 0xCu, PACD_S, 0u, 0u) &&
           PE_LoadU32(0x800BD02Cu) == 0x3Cu,
           "80084168 is01 flush/done, frame exchange, send, timer");
    /* the real func_800830DC(a0, 0) (src/func_800830DC.c): BAUD = 0x88,
     * +0x45 1, frame[0] = the collected byte, +0x44 0 -> 1 (tail -> 2) */
    ASSERT(PE_LoadU16(PACD_SIO + 0xEu) == 0x88u && PE_LoadU8(PACD_S + 0x45u) == 1u &&
           PE_LoadU8(PACD_S + 0x44u) == 2u,
           "80084168 send through func_800830DC");

    /* count 4 -> rewritten to 3 (no is3 call on this pass) */
    pacd_reset();
    pacd_sio_fixture(0x2u);
    PE_StoreU32(0x8009B744u, PACD_UNK + 0x10u);
    PE_StoreU32(0x8009B758u, PACD_R);
    PE_StoreU32(PACD_R + 0xF0u + 0xCu, 0x80160000u);
    PE_StoreU32(0x8009B780u, 4u);
    PE_StoreU32(0x8009B79Cu, 2u);
    PE_StoreU32(0x8009B7A0u, PACD_B + 0x80u);
    PE_StoreU32(PACD_S + 0x3Cu, PACD_B);
    (void)func_80084168(PACD_S);
    ASSERT(PE_LoadU32(0x8009B780u) == 3u, "80084168 count 4 -> 3");
    ASSERT(pacd_call(1, "D_8009B744 (card sector flush)", PACD_UNK + 0x10u,
                     0x80160000u + 4u * 240u - 0xF0u, 0u, 0u),
           "80084168 flushes sector r1 - 1 of the record buffer");
    PASS();
}

static void test_PA_cd_all(void)
{
    test_PA_cd_spu_and_small();
    test_PA_cd_poll();
    test_PA_cd_tables();
    test_PA_cd_ports();
    test_PA_cd_irq();
    test_PA_cd_transfer();
    test_PA_cd_records();
    test_PA_cd_multisector();
}
