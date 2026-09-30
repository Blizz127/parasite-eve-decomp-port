#include "pe_sio0.h"
#include "pe_rcnt2.h"
#include "pe_mmio.h"
/*
 * port_absent lane round 3 (2026-09-24) — tests for
 * pc_port/game/decomp_hand/absent_r3_port.c.  Expectations are computed
 * from the matched leaf src/func_XXXXXXXX.c named in each block.
 * Guest scratch 0x80170000..0x80173FFF.
 */
#define PA_R3 0x80170000u

unsigned int func_8006E3D4(pe_addr_t pe_a0);   /* generated TU */

/* field-VM operand vector at PA_R3: element k points at PA_R3 + 0x100 + 4k,
 * which holds vals[k]. */
static pe_addr_t r3_vec(const int32_t *vals, unsigned n)
{
    for (unsigned k = 0; k < n; k++) {
        PE_StoreU32(PA_R3 + 4u * k, PA_R3 + 0x100u + 4u * k);
        PE_StoreU32(PA_R3 + 0x100u + 4u * k, (uint32_t)vals[k]);
    }
    return PA_R3;
}

/* src/func_80015964.c: the k == 0xFF paths. */
static void test_PA_r3_80015964(void)
{
    TEST("PA_r3_80015964_page_code");
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    const char code_a[] = "ab0c1d", code_b[] = "zz9y8x";
    unsigned i;
    uint32_t want;

    for (i = 0; i < 6; i++) {
        PE_StoreU8(0x8009CD78u + i, (uint8_t)code_a[i]);
        PE_StoreU8(0x8009CD80u + i, (uint8_t)code_b[i]);
    }
    /* key 1 (< 2): D_80091A1D = 1 -> func_800392EC() == 1 < 2 -> CD78.
     * D_80091A1C == 0 so func_80038D74 (no pc_port implementation) runs. */
    const int32_t key1[] = { 0 };
    pe_addr_t a0 = r3_vec(key1, 1);
    PE_StoreU8(PA_R3 + 0x100u, 1u);          /* **a0 is the operand's byte */
    PE_StoreU8(0x80091A1Cu, 0u);
    PE_StoreU32(0x8009D1A0u, 0u);
    want = func_8006E3D4(0x8009CD78u);
    ASSERT(func_80015964(a0) == 1, "15964 returns 1");
    ASSERT(PE_LoadU32(0x8009D280u) == want, "15964 key<2 -> code at D_8009CD78");
    ASSERT(PE_Decomp_BoundaryCount() == 1 &&
           strcmp(PE_Decomp_BoundaryName(0), "func_80038D74") == 0,
           "15964 !func_80038D0C() -> func_80038D74 boundary");
    ASSERT(PE_LoadU32(0x8009D1A0u) == 0u, "15964 0xFF path sets no flag");

    /* key 80 (>= 71): D_80091A20 = 1, D_80091A1D stays 5 -> CD80.
     * D_80091A1C != 0: no func_80038D74. */
    PE_Decomp_ResetBoundaries();
    PE_StoreU8(PA_R3 + 0x100u, 80u);
    PE_StoreU8(0x80091A1Cu, 1u);
    PE_StoreU8(0x80091A1Du, 5u);
    want = func_8006E3D4(0x8009CD80u);
    ASSERT(func_80015964(a0) == 1, "15964 returns 1 (b)");
    ASSERT(PE_LoadU32(0x8009D280u) == want, "15964 page >= 2 -> code at D_8009CD80");
    ASSERT(PE_LoadU8(0x80091A20u) == 1u, "15964 key >= 71 marks D_80091A20");
    ASSERT(PE_LoadU8(0x80091A1Cu) == 0u, "15964 func_80039970 reset the font");
    ASSERT(PE_Decomp_BoundaryCount() == 0, "15964 no func_80038D74 when loaded");
    PASS();
}

/* src/func_80016910.c: the state-only opcodes. */
static void test_PA_r3_80016910(void)
{
    TEST("PA_r3_80016910_system_ops");
    ResetTestState();
    pe_addr_t a;
    const pe_addr_t ent = PA_R3 + 0x400u, e2 = PA_R3 + 0x800u;

    PE_StoreU32(0x800B0CD8u, 0x1u);
    { const int32_t v[] = { 0x835 }; a = r3_vec(v, 1); }
    ASSERT(func_80016910(a) == 1 && PE_LoadU32(0x800B0CD8u) == 0x801u, "16910 0x835 |= 0x800");
    { const int32_t v[] = { 0xB54 }; a = r3_vec(v, 1); }
    (void)func_80016910(a);
    { const int32_t v[] = { 0xC80 }; a = r3_vec(v, 1); }
    (void)func_80016910(a);
    ASSERT(PE_LoadU32(0x800B0CD8u) == (0x801u | 0x400000u | 0x8000000u),
           "16910 0xB54 / 0xC80 flag bits");

    { const int32_t v[] = { 0x899, -3, 0x12345 }; a = r3_vec(v, 3); }
    (void)func_80016910(a);
    ASSERT(PE_LoadU16(0x800BD028u) == 0xFFFDu && PE_LoadU16(0x800BD02Au) == 0x2345u,
           "16910 0x899 short pair (truncating)");

    { const int32_t v[] = { 0x961, 0x101, 2, 3 }; a = r3_vec(v, 4); }
    (void)func_80016910(a);
    ASSERT(PE_LoadU8(0x8009CDF8u) == 1u && PE_LoadU8(0x8009CDF9u) == 2u &&
           PE_LoadU8(0x8009CDFAu) == 3u, "16910 0x961 colour bytes");
    { const int32_t v[] = { 0xA8C, 7, 8, 0x1FF }; a = r3_vec(v, 4); }
    (void)func_80016910(a);
    ASSERT(PE_LoadU8(0x800BD025u) == 7u && PE_LoadU8(0x800BD026u) == 8u &&
           PE_LoadU8(0x800BD027u) == 0xFFu, "16910 0xA8C bytes");

    PE_StoreU32(0x8009D2F0u, ent);
    { const int32_t v[] = { 0xA28, 0x55 }; a = r3_vec(v, 2); }
    (void)func_80016910(a);
    { const int32_t v[] = { 0xA29, 0x66 }; a = r3_vec(v, 2); }
    (void)func_80016910(a);
    ASSERT(PE_LoadU8(ent + 0x27Cu) == 0x55u && PE_LoadU8(ent + 0x27Du) == 0x66u,
           "16910 0xA28/0xA29 player bytes");

    /* 0xBB8 walks the list and returns 1 either way; 0 operand skips it. */
    PE_StoreU32(0x8009D20Cu, e2);
    PE_StoreU32(e2 + 4u, 0u);
    PE_StoreU8(e2 + 0xCu, 4u); PE_StoreU8(e2 + 0xDu, 5u);
    { const int32_t v[] = { 0xBB8, 4, 5 }; a = r3_vec(v, 3); }
    ASSERT(func_80016910(a) == 1, "16910 0xBB8 returns 1");

    /* unknown opcode: no effect, 1. */
    { const int32_t v[] = { 0x123 }; a = r3_vec(v, 1); }
    ASSERT(func_80016910(a) == 1, "16910 default returns 1");
    PASS();
}

/* src/func_8003495C.c: mode 2 with the countdown expired picks the lowest
 * pending title (0x2000 -> D_800914AC rows of 20) and re-arms 0x4B - 1. */
static void test_PA_r3_8003495C(void)
{
    TEST_RETAIL_DISC1("PA_r3_8003495C_title_banner"); TEST_RETAIL_FIXUPS(RETAILFIX_menu_text);
    ResetTestState();
    for (unsigned i = 0; i < sizeof(NAM5_menu_text_common) / sizeof(NAM5_menu_text_common[0]); i++)
        PE_StoreU32(0x80000000u + NAM5_menu_text_common[i][0], NAM5_menu_text_common[i][1]);
    const pe_addr_t slot = PA_R3 + 0x1000u, obj = PA_R3 + 0x1100u;
    PE_StoreU32(0x8009D1A8u, slot);
    PE_StoreU32(slot, obj);
    PE_StoreU32(obj + 0x10u, 5u);
    PE_StoreU32(0x8009D218u, 1u);                    /* func_8005BCB0() */
    PE_StoreU32(0x8009D1ACu, 0x6200u);               /* mode 2, 0x2000|0x4000, cd 0 */
    func_8003495C();
    ASSERT(PE_LoadU32(0x800BCEACu) == 0x800914ACu + 20u, "3495C 0x2000 title row");
    ASSERT(PE_LoadU32(0x8009D1ACu) == (0x4200u | 0x4Au),
           "3495C clears 0x2000 only, countdown 0x4B - 1");
    /* countdown running: nothing but the decrement. */
    func_8003495C();
    ASSERT(PE_LoadU32(0x8009D1ACu) == (0x4200u | 0x49u), "3495C countdown ticks");
    PASS();
}

/* src/func_80042020.c */
static void test_PA_r3_80042020(void)
{
    TEST("PA_r3_80042020_card_commit");
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();
    const pe_addr_t c = 0x800A0ED4u + 0x418u;        /* slot 1 */
    const pe_addr_t e = c + 0x1Cu + 3u * 0x44u;       /* entry 3 */

    PE_StoreU8(c + 0u, 2u);
    func_80042020(1, 3);
    ASSERT(PE_LoadU8(c + 3u) == 0u && PE_Decomp_BoundaryCount() == 0,
           "42020 sel != 1 does nothing");

    PE_StoreU8(c + 0u, 1u);
    PE_StoreU8(c + 0xAu, 4u);
    PE_StoreU8(e + 0u, 0u);                           /* state != 1 */
    PE_StoreU8(e + 1u, 9u);
    PE_StoreU8(e + 0x29u, 7u);
    PE_StoreU32(0x80092224u, 0x80012340u);
    func_80042020(1, 3);
    ASSERT(PE_LoadU8(c + 0xAu) == 5u, "42020 count += (state != sel)");
    ASSERT(PE_LoadU8(c + 3u) == 3u && PE_LoadU8(c + 5u) == 3u && PE_LoadU8(c + 6u) == 0u &&
           PE_LoadU8(c + 1u) == 1u && PE_LoadU8(c + 7u) == 1u, "42020 cursor fields");
    ASSERT(PE_LoadU16(c + 0x14u) == 0x2000u && PE_LoadU16(c + 0x16u) == 10u, "42020 h14/h16");
    ASSERT(PE_LoadU32(0x800A1854u) == c && PE_LoadU32(0x800A1858u) == 0x2000u,
           "42020 D_800A1854 = c, D_800A1858 = 0x2000");
    ASSERT(PE_LoadU8(c + 0xBu) == 4u && PE_LoadU8(e + 0u) == 1u && PE_LoadU8(e + 1u) == 0u,
           "42020 fB = 4 for a fresh entry; entry state/f1");
    ASSERT(g_bootstrap_arg4_call_count >= 1 &&
           g_bootstrap_arg4_calls[0].arg0 == 0x8009EE70u &&
           g_bootstrap_arg4_calls[0].arg1 == 0x80012340u &&
           g_bootstrap_arg4_calls[0].arg2 == 1u &&
           g_bootstrap_arg4_calls[0].arg3 == (uint32_t)('0' + 7),
           "42020 sprintf(D_8009EE70, D_80092224, c > base, f29 + '0', ...)");
    ASSERT(PE_LoadU32(0x800A5D50u) == 0x2000u, "42020 func_80040B80 built the block");
    PASS();
}

/* src/func_8005C688.c */
static void test_PA_r3_8005C688(void)
{
    TEST("PA_r3_8005C688_equippable");
    ResetTestState();
    const pe_addr_t r0 = 0x800BEEACu + (0x100u << 5), r1 = r0 + 32u, r2 = r0 + 64u;

    PE_StoreU8(0x800C0E0Cu, 3u);
    PE_StoreU32(0x8009D018u, 0u);                     /* func_80052F70() = 3 */
    PE_StoreU16(0x800C0E48u, 0x100u);
    PE_StoreU16(0x800C0E4Au, 0x101u);
    PE_StoreU16(0x800C0E4Cu, 0x102u);
    PE_StoreU32(0x8009D0CCu, 0u);                     /* weapons: mask 0x200 */
    PE_StoreU32(0x8009D0D0u, 10u);
    PE_StoreU8(r0 + 6u, 9u); PE_StoreU8(r0 + 7u, 5u); PE_StoreU16(r0 + 0xEu, 10u);
    PE_StoreU8(r1 + 6u, 9u); PE_StoreU8(r1 + 7u, 1u); PE_StoreU16(r1 + 0xEu, 2u);
    PE_StoreU8(r2 + 6u, 9u); PE_StoreU8(r2 + 7u, 20u); PE_StoreU8(r2 + 4u, 0x93u);
    PE_StoreU32(0x8009D05Cu, 0xFFFFFFFFu);
    PE_StoreU32(0x8009D060u, 0xFFFFFFFFu);

    ASSERT(func_8005C688(0, 0) == 1, "5C688 one qualifying record, no equipped");
    ASSERT(PE_LoadU32(0x8009D05Cu) == 1u && PE_LoadU32(0x8009D060u) == 0u,
           "5C688 bitmap cleared then bit 0");
    ASSERT(PE_LoadU32(0x8009D040u) == 3u && PE_LoadU32(0x8009D068u) == 1u,
           "5C688 list of 3, D_8009D068 = 1");
    ASSERT(func_8005C688(1, 0) == 0 && PE_LoadU32(0x8009D068u) == 0u,
           "5C688 min_b excludes it");

    PE_StoreU8(r2 + 0u, 1u); PE_StoreU8(r2 + 5u, 0x10u);   /* equipped weapon */
    ASSERT(func_8005C688(0, 0) == -1, "5C688 negated when a weapon is equipped");
    PASS();
}

/* src/func_8007A360.c / func_80081D74.c / func_80083F44.c */
static void test_PA_r3_small(void)
{
    TEST("PA_r3_small");
    ResetTestState();

    PE_StoreU32(0x8009AFB8u, 1u); PE_StoreU32(0x8009AFC4u, 1u);
    ASSERT(func_8007A360(2) == 1, "7A360 a0 == 2 -> 1");
    ASSERT(PE_LoadU32(0x8009AFB8u) == 0u && PE_LoadU32(0x8009AFC4u) == 0u,
           "7A360 ran func_8007BBB0");

    PE_StoreU32(0x8009B70Cu, 1u);
    PE_StoreU32(0x8009B6F4u, 0x77u);
    ASSERT(func_80081D74(5, 6) == 0 && PE_LoadU32(0x8009B6F4u) == 0x77u, "81D74 busy -> 0");
    PE_StoreU32(0x8009B70Cu, 0u);
    PE_StoreU32(0x800B8AB4u, 0x1111u);
    PE_StoreU32(0x800B8AB8u, 0x2222u);
    ASSERT(func_80081D74(5, 6) == 1, "81D74 starts -> 1");
    ASSERT(PE_LoadU32(0x8009B6ECu) == 0xFFFFFFFFu && PE_LoadU32(0x8009B6F0u) == 0u &&
           PE_LoadU32(0x8009B6F4u) == 5u && PE_LoadU32(0x8009B6F8u) == 0u &&
           PE_LoadU32(0x8009B6FCu) == 6u, "81D74 job words");
    ASSERT(PE_LoadU32(0x8009B700u) == 0x1111u && PE_LoadU32(0x800B8AB4u) == 0x80081E70u &&
           PE_LoadU32(0x8009B704u) == 0x2222u && PE_LoadU32(0x800B8AB8u) == 0x8008214Cu,
           "81D74 exchanges both callbacks");
    ASSERT(PE_LoadU32(0x8009B70Cu) == 1u, "81D74 busy set");

    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();
    const pe_addr_t rec = PA_R3 + 0x2000u;
    PE_StoreU32(0x8009B764u, 3u); PE_StoreU32(0x8009B774u, 3u);
    PE_StoreU32(0x8009B760u, 1u); PE_StoreU32(0x8009B7A4u, 1u);
    PE_StoreU32(0x8009B754u, 0x80099000u); PE_StoreU32(0x8009B750u, 0x80099100u);
    /* An unregistered hook address: the generated TU (round 6) dispatches
     * through PE_GuestCall, which would genuinely run a registered target
     * such as func_80084B78; this case pins the unresolved trace. */
    PE_StoreU32(0x8009B73Cu, 0x80099200u);
    PE_StoreU32(rec + 0xCu, 0x80150000u);
    PE_StoreU8(rec + 0x36u, 0u);
    /* func_800832B4 (sio_port.c) now runs: retail SIO0 / I_STAT pointers and
     * the retail RCNT2 program; no card attached -> RCNT2 timeout -2. */
    PE_Sio0_Reset(); PE_Rcnt2_Reset(); PE_MMIO_Reset();
    PE_StoreU32(0x8009B788u, 0x1F801040u);
    PE_StoreU32(0x8009B784u, 0x1F801070u);
    PE_StoreU16(0x1F80104Au, 0x1003u);
    PE_Rcnt2_WriteTarget(0x44E8u);
    PE_Rcnt2_WriteMode(0x258u);
    PE_StoreU32(rec + 0x3Cu, PA_R3 + 0x3000u);
    PE_StoreU8(PA_R3 + 0x3000u, 0x51u);
    ASSERT(func_80083F44(rec) == -2, "83F44 returns func_800832B4's no-card timeout");
    ASSERT(PE_Decomp_BoundaryCount() == 3, "83F44 two hooks and the sector hook");
    ASSERT(g_bootstrap_arg4_call_count == 2 &&
           g_bootstrap_arg4_calls[0].arg0 == 0x80150000u &&
           g_bootstrap_arg4_calls[1].arg0 == 0x801500F0u,
           "83F44 sector hook on +0xC / +0xF0 (the hooks take no arguments)");
    ASSERT(PE_LoadU16(0x1F80104Eu) == 0x88u, "832B4 ran with mode 0x88 (a0[0x36] 0 -> 0x42)");
    PASS();
}

static void test_PA_r3_all(void)
{
    test_PA_r3_80015964();
    test_PA_r3_80016910();
    test_PA_r3_8003495C();
    test_PA_r3_80042020();
    test_PA_r3_8005C688();
    test_PA_r3_small();
}
