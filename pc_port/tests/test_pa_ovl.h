/*
 * port_absent lane (2026-09-23) — tests for the ovl group
 * (pc_port/game/decomp_hand/absent_ovl_port.c).  Expectations are computed
 * from the matched leaf src/func_XXXXXXXX.c, never from the port.  Random
 * draws are replayed by re-seeding the BIOS rand (func_80071A64) and calling
 * func_80071A54 in the leaf's evaluation order.
 *
 * The mode-2 draw paths that end in func_800CEE20 / func_800D004C /
 * func_800D0728 / func_800D1DEC need a live GPU packet arena and are not
 * exercised here; their non-draw state (D_800F3374 bracketing, publishes) is.
 */

#define PAO_CTX    0x80160000u   /* D_800F33E0 target; +8 = pool          */
#define PAO_POOL   0x80160100u
#define PAO_ACTOR  0x80160800u   /* D_8009D254 target                     */
#define PAO_MTX    0x80160A00u   /* actor +0x238                          */
#define PAO_TGT    0x80160C00u   /* D_800F32D0 target; +8 = PAO_TOBJ      */
#define PAO_TOBJ   0x80160D00u
#define PAO_REC    0x80161000u   /* the leaf's a1 record                  */
#define PAO_A2     0x80161100u

static void pao_world(void)
{
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    PE_StoreU32(0x800F33E0u, PAO_CTX);
    PE_StoreU32(PAO_CTX + 8u, PAO_POOL);
    PE_StoreU32(0x8009D254u, PAO_ACTOR);
    PE_StoreU32(PAO_ACTOR + 0x238u, PAO_MTX);
    PE_StoreU32(PAO_MTX + 20u, 0x00011111u);          /* mode 0 x (low 16) */
    PE_StoreU32(PAO_MTX + 24u, 0x0002FF00u);          /* y = -0x100        */
    PE_StoreU32(PAO_MTX + 28u, 0x00033333u);          /* z                 */
    PE_StoreU32(PAO_ACTOR + 0x28u, 0x01230000u);      /* mode 1 x (>> 16)  */
    PE_StoreU32(PAO_ACTOR + 0x2Cu, 0xFE000000u);      /* y = -0x200        */
    PE_StoreU32(PAO_ACTOR + 0x30u, 0x04560000u);
    PE_StoreU32(0x800F32D0u, PAO_TGT);
    PE_StoreU32(PAO_TGT + 8u, PAO_TOBJ);
    PE_StoreU16(PAO_TOBJ + 0x268u, 0x0101u);
    PE_StoreU16(PAO_TOBJ + 0x26Au, 0xF000u);
    PE_StoreU16(PAO_TOBJ + 0x26Cu, 0x0303u);
    PE_StoreU16(0x800F34E4u, 1u);   /* func_800CEDA8(1): texture already set */
    PE_StoreU16(0x800E11E6u, 3u);
    PE_StoreU16(0x800E11F6u, 5u);
    PE_StoreU16(0x800E2850u + 6u, 0xAAAAu);
    PE_StoreU16(0x800E2850u + 10u, 0xBBBBu);
}

static int16_t pao_s16(pe_addr_t a) { return (int16_t)PE_LoadU16(a); }

/* The sprite-state block every emitter's mode 2 writes. */
static int pao_state_ok(unsigned av, unsigned e, unsigned bias, int full)
{
    if (PE_LoadU16(0x800F336Cu) != 1u || PE_LoadU16(0x800F3370u) != av ||
        PE_LoadU16(0x800F336Eu) != e || PE_LoadU16(0x800F3374u) != bias)
        return 0;
    if (!full)
        return 1;
    return PE_LoadU16(0x800F3368u) == 0x20u && PE_LoadU16(0x800F336Au) == 2u &&
           PE_LoadU16(0x800F3376u) == 0x20u && PE_LoadU16(0x800F3378u) == 0x20u &&
           PE_LoadU16(0x800F3372u) == 0u;
}

/* func_800CE560 pool header: stride = size + 4, count, callback. */
static int pao_pool_ok(unsigned size, unsigned count, uint32_t fn, int ret)
{
    return ret == (int)((size + 4u) * count + 12u) &&
           PE_LoadU32(PAO_POOL) == size + 4u && PE_LoadU32(PAO_POOL + 4u) == count &&
           PE_LoadU32(PAO_POOL + 8u) == fn;
}

static void test_PA_ovl_C65E4(void)
{
    TEST("PA_ovl_C65E4");
    pao_world();
    for (unsigned i = 0; i < 4u; i++)
        PE_StoreU32(0x800C213Cu + i * 4u, 0x11110000u + i);
    PE_StoreU16(PAO_REC + 0u, 10u); PE_StoreU16(PAO_REC + 2u, 0xFFF6u); PE_StoreU16(PAO_REC + 4u, 0);
    PE_StoreU16(PAO_REC + 6u, 3u);  PE_StoreU16(PAO_REC + 8u, 20u);     PE_StoreU16(PAO_REC + 10u, 0x8000u);
    func_800C65E4(PAO_REC, PAO_REC + 6u, PAO_A2);
    /* d = a1 - a0 as ints (-7, 30, -0x8000); m = D_800C213C. */
    ASSERT((int32_t)PE_LoadU32(PE_ABSENT_OVL_C65E4_D + 0u) == -7 &&
           (int32_t)PE_LoadU32(PE_ABSENT_OVL_C65E4_D + 4u) == 30 &&
           (int32_t)PE_LoadU32(PE_ABSENT_OVL_C65E4_D + 8u) == -0x8000, "C65E4: d = a1 - a0");
    ASSERT(PE_LoadU32(PE_ABSENT_OVL_C65E4_M + 12u) == 0x11110003u, "C65E4: m copies D_800C213C");
    /* the registry holds distinct symbols; the argument log holds calls */
    ASSERT(PE_Decomp_BoundaryCount() == 2u &&
           strcmp(PE_Decomp_BoundaryName(0), "func_80079178") == 0 &&
           strcmp(PE_Decomp_BoundaryName(1), "func_80078120") == 0 &&
           strcmp(g_bootstrap_arg4_calls[1].symbol, "func_80079178") == 0 &&
           strcmp(g_bootstrap_arg4_calls[2].symbol, "func_80078120") == 0,
           "C65E4: 2x 79178 then 3x 78120");
    ASSERT(g_bootstrap_arg4_call_count == 5 &&
           g_bootstrap_arg4_calls[0].arg0 == PE_ABSENT_OVL_C65E4_D &&
           g_bootstrap_arg4_calls[0].arg1 == PE_ABSENT_OVL_C65E4_M &&
           g_bootstrap_arg4_calls[0].arg2 == PE_ABSENT_OVL_C65E4_T1 &&
           g_bootstrap_arg4_calls[1].arg1 == PE_ABSENT_OVL_C65E4_T1 &&
           g_bootstrap_arg4_calls[1].arg2 == PE_ABSENT_OVL_C65E4_T2 &&
           g_bootstrap_arg4_calls[2].arg1 == PAO_A2 + 0x14u &&
           g_bootstrap_arg4_calls[3].arg0 == PE_ABSENT_OVL_C65E4_T2 &&
           g_bootstrap_arg4_calls[3].arg1 == PAO_A2 + 0x18u &&
           g_bootstrap_arg4_calls[4].arg0 == PE_ABSENT_OVL_C65E4_D &&
           g_bootstrap_arg4_calls[4].arg1 == PAO_A2 + 0x1Cu, "C65E4: guest argument vectors");
    PASS();
}

static void test_PA_ovl_D5010(void)
{
    TEST("PA_ovl_D5010");
    pao_world();
    /* pos (100,-5,7) vel (-33, 9, 64) end 10 */
    PE_StoreU16(PAO_REC + 0u, 100u); PE_StoreU16(PAO_REC + 2u, 0xFFFBu); PE_StoreU16(PAO_REC + 4u, 7u);
    PE_StoreU16(PAO_REC + 6u, (uint16_t)-33); PE_StoreU16(PAO_REC + 8u, 9u);
    PE_StoreU16(PAO_REC + 10u, 64u); PE_StoreU16(PAO_REC + 12u, 10u);
    PE_StoreU32(0x800E27ECu, 9u);
    ASSERT(func_800D5010(1, PAO_REC) == 0, "D5010: frame 9 < a1[6] keeps going");
    /* y = -5 + 9 = 4 > 0 -> vy = -9 + 3; vx = -33*31/32 = -31 (trunc); vz = 62 */
    ASSERT(pao_s16(PAO_REC + 0u) == 67 && pao_s16(PAO_REC + 2u) == 4 && pao_s16(PAO_REC + 4u) == 71,
           "D5010: position integrates");
    ASSERT(pao_s16(PAO_REC + 6u) == -31 && pao_s16(PAO_REC + 10u) == 62 && pao_s16(PAO_REC + 8u) == -6,
           "D5010: damping, flip, gravity");
    ASSERT(func_800D5010(1, PAO_REC) == 0 && pao_s16(PAO_REC + 8u) == -3,
           "D5010: y <= 0 keeps the sign, adds 3");
    PE_StoreU32(0x800E27ECu, 10u);
    ASSERT(func_800D5010(1, PAO_REC) == 1, "D5010: ends when frame >= a1[6]");
    ASSERT(func_800D5010(0, PAO_REC) == 0 && func_800D5010(3, PAO_REC) == 0, "D5010: other modes");
    PASS();
}

static void test_PA_ovl_D6A1C(void)
{
    TEST("PA_ovl_D6A1C");
    pao_world();
    func_80071A64(0x1234u);
    unsigned r = func_80071A54();
    func_80071A64(0x1234u);
    ASSERT(func_800D6A1C(0, PAO_REC) == 0, "D6A1C: mode 0 returns 0");
    ASSERT(PE_LoadU32(PAO_REC + 16u) == r, "D6A1C: rand into *(int *)(a1 + 8)");
    ASSERT(PE_LoadU16(PAO_REC + 0u) == 0x0101u && PE_LoadU16(PAO_REC + 2u) == 0xF000u &&
           PE_LoadU16(PAO_REC + 4u) == 0x0303u && PE_LoadU16(PAO_REC + 8u) == 0x0101u &&
           PE_LoadU16(PAO_REC + 10u) == (uint16_t)(0xF000u - 0x1B8u) &&
           PE_LoadU16(PAO_REC + 12u) == 0x0303u, "D6A1C: target position, a1[5] -= 0x1B8");
    PE_StoreU32(0x800E27ECu, 0x1Fu);
    ASSERT(func_800D6A1C(1, PAO_REC) == 0, "D6A1C: runs below 0x20");
    PE_StoreU32(0x800E27ECu, 0x20u);
    ASSERT(func_800D6A1C(1, PAO_REC) == 1, "D6A1C: ends at 0x20");
    PASS();
}

static void test_PA_ovl_D8B6C(void)
{
    TEST("PA_ovl_D8B6C");
    pao_world();
    ASSERT(pao_pool_ok(0xCu, 0xEu, 0x800D8978u, func_800D8B6C(0, PAO_REC)), "D8B6C: pool 0xE x 0xC");
    /* odd frame 3: spawn p = pool + 12 + 4, then fall through into mode 2 */
    PE_StoreU32(0x800E27ECu, 3u);
    func_80071A64(77u);
    unsigned r = func_80071A54();
    func_80071A64(77u);
    ASSERT(func_800D8B6C(1, PAO_REC) == 0, "D8B6C: mode 1 below 0x32 returns 0");
    pe_addr_t p = PAO_POOL + 16u;
    ASSERT(PE_LoadU16(PAO_POOL + 12u) == 1u && PE_LoadU16(p + 2u) == (uint16_t)r &&
           PE_LoadU16(p + 0u) == 0u && PE_LoadU16(p + 4u) == 0u &&
           pao_s16(p + 8u) == -0x200 - 0x200, "D8B6C: spawn, p[4] = player y - 0x200");
    ASSERT(PE_LoadU16(0x800E21F8u) == 0x123u && PE_LoadU16(0x800E21FAu) == 0xFE00u &&
           PE_LoadU16(0x800E21FCu) == 0x456u, "D8B6C: fall-through refreshes D_800E21F8");
    ASSERT(pao_state_ok(0xBBBBu, 1u, 8u, 1), "D8B6C: sprite state (ix D_800E11F6, 336E = 1)");
    /* even frame 0x32: no spawn, returns 1 before mode 2 */
    PE_StoreU16(0x800F3374u, 0x99u);
    PE_StoreU32(0x800E27ECu, 0x32u);
    ASSERT(func_800D8B6C(1, PAO_REC) == 1 && PE_LoadU16(0x800F3374u) == 0x99u,
           "D8B6C: returns 1 at 0x32 without the mode-2 body");
    ASSERT(func_800D8B6C(5, PAO_REC) == 0, "D8B6C: default");
    PASS();
}

static void test_PA_ovl_D9554(void)
{
    TEST("PA_ovl_D9554");
    pao_world();
    PE_StoreU16(PAO_REC + 0u, 0u); PE_StoreU16(PAO_REC + 2u, 50u); PE_StoreU16(PAO_REC + 4u, 5u);
    PE_StoreU16(PAO_REC + 6u, 40u); PE_StoreU16(PAO_REC + 8u, (uint16_t)-2); PE_StoreU16(PAO_REC + 10u, (uint16_t)-40);
    PE_StoreU32(0x800E27ECu, 0x12u);
    func_80071A64(5u);
    unsigned r1 = func_80071A54(), r2 = func_80071A54();
    func_80071A64(5u);
    ASSERT(func_800D9554(1, PAO_REC) == 0, "D9554: mode 1 below 0x28");
    ASSERT(pao_s16(PAO_REC + 0u) == (int16_t)(0xFFFF + 40 + (int)(r1 & 3)) &&
           pao_s16(PAO_REC + 2u) == (int16_t)(49 - 2 + (int)(r2 & 3)) &&
           pao_s16(PAO_REC + 4u) == -35, "D9554: jittered integration");
    ASSERT(pao_s16(PAO_REC + 6u) == 38 && pao_s16(PAO_REC + 10u) == -38 &&
           pao_s16(PAO_REC + 8u) == -3, "D9554: damping and vy-- below 0x13");
    PE_StoreU32(0x800E27ECu, 0x13u);
    ASSERT(func_800D9554(1, PAO_REC) == 0 && pao_s16(PAO_REC + 8u) == -2, "D9554: vy++ from 0x13");
    PE_StoreU32(0x800E27ECu, 0x28u);
    ASSERT(func_800D9554(1, PAO_REC) == 1 && func_800D9554(0, PAO_REC) == 0, "D9554: ends at 0x28");
    PASS();
}

static void test_PA_ovl_D9FD4(void)
{
    TEST("PA_ovl_D9FD4");
    pao_world();
    ASSERT(pao_pool_ok(8u, 0xCu, 0x800D9E5Cu, func_800D9FD4(0, PAO_REC)), "D9FD4: pool 0xC x 8");
    ASSERT(PE_LoadU16(PAO_REC + 0u) == 0x123u && PE_LoadU16(PAO_REC + 2u) == 0xFE00u,
           "D9FD4: player position (mode 1)");
    PE_StoreU32(0x800E27ECu, 1u);
    func_80071A64(9u);
    int r0 = (int)func_80071A54(), r1 = (int)func_80071A54(), r2 = (int)func_80071A54();
    func_80071A64(9u);
    ASSERT(func_800D9FD4(1, PAO_REC) == 0, "D9FD4: mode 1");
    pe_addr_t p = PAO_POOL + 16u;
    ASSERT(pao_s16(p + 0u) == (int16_t)(0x123 + r0 % 400 - 200) &&
           PE_LoadU16(p + 2u) == 0xFE00u &&
           pao_s16(p + 4u) == (int16_t)(0x456 + r1 % 400 - 200) &&
           pao_s16(p + 6u) == (int16_t)(-(r2 & 7) - 16), "D9FD4: spawn");
    PE_StoreU32(0x800E27ECu, 2u);
    ASSERT(func_800D9FD4(1, PAO_REC) == 0 && PE_LoadU16(PAO_POOL + 12u + 12u) == 0u,
           "D9FD4: even frame does not spawn");
    PE_StoreU32(0x800E27ECu, 0x46u);
    ASSERT(func_800D9FD4(1, PAO_REC) == 1, "D9FD4: ends at 0x46");
    ASSERT(func_800D9FD4(2, PAO_REC) == 0 && pao_state_ok(0xAAAAu, 0u, 8u, 1), "D9FD4: sprite state");
    PASS();
}

static void test_PA_ovl_DA5D4(void)
{
    TEST("PA_ovl_DA5D4");
    pao_world();
    PE_StoreU32(0x800F32D8u, 0x80161800u);          /* zero mesh: UVs untouched */
    PE_StoreU16(0x800F3420u, 7u);
    func_80071A64(11u);
    unsigned r = func_80071A54();
    func_80071A64(11u);
    ASSERT(pao_pool_ok(0x10u, 0x18u, 0x800DA1FCu, func_800DA5D4(0, PAO_REC)), "DA5D4: pool 0x18 x 0x10");
    ASSERT(PE_LoadU32(PAO_REC) == r && PE_LoadU16(0x800F3420u) == 0u, "DA5D4: seed, mesh reset");
    PE_StoreU32(PAO_REC, 0x12345u);
    PE_StoreU32(0x800E27ECu, 12u);
    func_80071A64(3u);
    unsigned t = func_80071A54();
    func_80071A64(3u);
    ASSERT(func_800DA5D4(1, PAO_REC) == 0, "DA5D4: mode 1");
    pe_addr_t p = PAO_POOL + 16u;
    ASSERT(PE_LoadU16(p + 10u) == 0x2345u && PE_LoadU16(p + 6u) == 0u && PE_LoadU16(p + 12u) == 0u &&
           PE_LoadU32(PAO_REC) == 0x12345u - (0x555u + (t & 0xFFu)), "DA5D4: spawn on frame % 6 == 0");
    PE_StoreU32(0x800E27ECu, 13u);
    ASSERT(func_800DA5D4(1, PAO_REC) == 0 && PE_LoadU16(PAO_POOL + 12u + 0x14u) == 0u,
           "DA5D4: no spawn off the 6-frame beat");
    PE_StoreU32(0x800E27ECu, 0x46u);
    ASSERT(func_800DA5D4(1, PAO_REC) == 1, "DA5D4: ends at 0x46");
    PE_StoreU16(0x800942ECu, 0x4444u);
    ASSERT(func_800DA5D4(2, PAO_REC) == 0 && PE_LoadU16(0x800E2214u) == 0x0101u &&
           PE_LoadU16(0x800E2216u) == 0x4444u && PE_LoadU16(0x800E2218u) == 0x0303u,
           "DA5D4: publishes target, y from D_800942EC");
    PASS();
}

static void test_PA_ovl_DA934(void)
{
    TEST("PA_ovl_DA934");
    pao_world();
    PE_StoreU16(0x800942ECu, 0x0555u);
    ASSERT(pao_pool_ok(8u, 0xCu, 0x800DA780u, func_800DA934(0, PAO_REC)), "DA934: pool 0xC x 8");
    ASSERT(PE_LoadU16(PAO_REC + 0u) == 0x0101u && PE_LoadU16(PAO_REC + 2u) == 0x0555u &&
           PE_LoadU16(PAO_REC + 4u) == 0x0303u, "DA934: target x/z, y = D_800942EC");
    PE_StoreU32(0x800E27ECu, 0x27u);
    func_80071A64(21u);
    int r0 = (int)func_80071A54(), r1 = (int)func_80071A54(), r2 = (int)func_80071A54();
    func_80071A64(21u);
    ASSERT(func_800DA934(1, PAO_REC) == 0, "DA934: mode 1");
    pe_addr_t p = PAO_POOL + 16u;
    ASSERT(pao_s16(p + 0u) == (int16_t)(0x101 + r0 % 400 - 200) &&
           pao_s16(p + 2u) == (int16_t)(0x555 - 0x320) &&
           pao_s16(p + 4u) == (int16_t)(0x303 + r1 % 400 - 200) &&
           pao_s16(p + 6u) == (int16_t)((r2 & 7) + 0x2A), "DA934: spawn");
    PE_StoreU32(0x800E27ECu, 0x46u);
    ASSERT(func_800DA934(1, PAO_REC) == 1, "DA934: ends at 0x46");
    ASSERT(func_800DA934(2, PAO_REC) == 0 && pao_state_ok(0xAAAAu, 0u, 0x20u, 1), "DA934: sprite state");
    PASS();
}

static void test_PA_ovl_DBA9C(void)
{
    TEST("PA_ovl_DBA9C");
    pao_world();
    PE_StoreU32(0x800E1D60u, 0x400u);
    ASSERT(func_800DBA9C(0, PAO_REC) == 0, "DBA9C: mode 0");
    ASSERT(PE_LoadU32(PAO_REC + 8u) == 0x400u && PE_LoadU32(0x800E1D60u) == 0x955u,
           "DBA9C: angle stored and advanced by 0x555");
    int sx = func_80077DC4(0x400) * 300 / 4096, cz = func_80077CF4(0x400) * 300 / 4096;
    ASSERT(pao_s16(PAO_REC + 0u) == (int16_t)(0x1111 + sx) &&
           pao_s16(PAO_REC + 2u) == -0x100 - 100 &&
           pao_s16(PAO_REC + 4u) == (int16_t)(0x3333 + cz), "DBA9C: orbit offset");
    PE_StoreU32(0x800E27ECu, 7u);
    ASSERT(func_800DBA9C(1, PAO_REC) == 0, "DBA9C: runs below 8");
    PE_StoreU32(0x800E27ECu, 8u);
    ASSERT(func_800DBA9C(1, PAO_REC) == 1 && func_800DBA9C(9, PAO_REC) == 0, "DBA9C: ends at 8");
    PASS();
}

static void test_PA_ovl_DBE6C_DC750(void)
{
    TEST("PA_ovl_DBE6C_DC750");
    pao_world();
    ASSERT(pao_pool_ok(8u, 0x14u, 0x800DBCD8u, func_800DBE6C(0, PAO_REC)), "DBE6C: pool 0x14 x 8");
    ASSERT(PE_LoadU16(PAO_REC + 2u) == 0xF000u, "DBE6C: target position");
    PE_StoreU32(0x800E27ECu, 0x14u);
    func_80071A64(31u);
    int r0 = (int)func_80071A54(), r1 = (int)func_80071A54(), r2 = (int)func_80071A54();
    func_80071A64(31u);
    ASSERT(func_800DBE6C(1, PAO_REC) == 0, "DBE6C: mode 1");
    pe_addr_t p = PAO_POOL + 16u;
    ASSERT(pao_s16(p + 0u) == (int16_t)(0x101 + (r0 & 0x1FF) - 0x100) &&
           pao_s16(p + 2u) == (int16_t)(0xF000 + (r1 & 0x1FF) - 0x100) &&
           pao_s16(p + 4u) == (int16_t)(0x303 + (r2 & 0x1FF) - 0x100), "DBE6C: box spawn");
    PE_StoreU32(0x800E27ECu, 0x15u);
    ASSERT(func_800DBE6C(1, PAO_REC) == 0 && PE_LoadU16(PAO_POOL + 12u + 12u) == 0u,
           "DBE6C: no spawn from 0x15");
    PE_StoreU32(0x800E27ECu, 0x35u);
    ASSERT(func_800DBE6C(1, PAO_REC) == 1, "DBE6C: ends at 0x35");
    ASSERT(func_800DBE6C(2, PAO_REC) == 0 && pao_state_ok(0xAAAAu, 0u, 0x10u, 1), "DBE6C: sprite state");

    pao_world();
    ASSERT(pao_pool_ok(8u, 0x14u, 0x800DC5BCu, func_800DC750(0, PAO_REC)), "DC750: pool 0x14 x 8");
    ASSERT(PE_LoadU16(PAO_REC + 0u) == 0x1111u && PE_LoadU16(PAO_REC + 2u) == 0xFF00u,
           "DC750: player position (mode 0)");
    PE_StoreU32(0x800E27ECu, 0x27u);
    ASSERT(func_800DC750(1, PAO_REC) == 0 && PE_LoadU16(PAO_POOL + 12u) == 1u, "DC750: spawns below 0x28");
    PE_StoreU32(0x800E27ECu, 0x28u);
    ASSERT(func_800DC750(1, PAO_REC) == 0 && PE_LoadU16(PAO_POOL + 12u + 12u) == 0u,
           "DC750: no spawn at 0x28, still running");
    PE_StoreU32(0x800E27ECu, 0x35u);
    ASSERT(func_800DC750(1, PAO_REC) == 1, "DC750: ends at 0x35");
    ASSERT(func_800DC750(2, PAO_REC) == 0 && pao_state_ok(0xBBBBu, 1u, 0x10u, 1), "DC750: sprite state");
    PASS();
}

static void test_PA_ovl_DCA80(void)
{
    TEST("PA_ovl_DCA80");
    pao_world();
    ASSERT(pao_pool_ok(8u, 0x14u, 0x800DC910u, func_800DCA80(0, PAO_REC)), "DCA80: pool 0x14 x 8");
    PE_StoreU32(0x800E27ECu, 0x21u);
    ASSERT(func_800DCA80(1, PAO_REC) == 0 && PE_LoadU16(PAO_POOL + 12u) == 0u, "DCA80: no spawn at 0x21");
    PE_StoreU32(0x800E27ECu, 0x1Fu);
    func_80071A64(41u);
    int r0 = (int)func_80071A54(), r1 = (int)func_80071A54(), r2 = (int)func_80071A54();
    func_80071A64(41u);
    ASSERT(func_800DCA80(1, PAO_REC) == 0, "DCA80: mode 1");
    pe_addr_t p = PAO_POOL + 16u;
    ASSERT(pao_s16(p + 0u) == (int16_t)(0x123 + r0 % 800 - 400) &&
           pao_s16(p + 2u) == (int16_t)(0xFE00 + r1 % 400 - 700) &&
           pao_s16(p + 4u) == (int16_t)(0x456 + r2 % 800 - 400), "DCA80: spawn");
    PE_StoreU32(0x800E27ECu, 0x3Cu);
    ASSERT(func_800DCA80(1, PAO_REC) == 1, "DCA80: ends at 0x3C");
    ASSERT(func_800DCA80(2, PAO_REC) == 0 && pao_state_ok(0xAAAAu, 0u, 0x10u, 1), "DCA80: sprite state");
    PASS();
}

static void test_PA_ovl_DCE94(void)
{
    TEST("PA_ovl_DCE94");
    pao_world();
    func_80071A64(51u);
    unsigned r = func_80071A54();
    func_80071A64(51u);
    ASSERT(pao_pool_ok(0x10u, 0x10u, 0x800DCCCCu, func_800DCE94(0, PAO_REC)), "DCE94: pool 0x10 x 0x10");
    ASSERT(PE_LoadU32(PAO_REC + 8u) == r && PE_LoadU16(PAO_REC + 0u) == 0x1111u, "DCE94: seed, player");
    PE_StoreU32(PAO_REC + 8u, 0x10000u);
    PE_StoreU32(0x800E27ECu, 0x10u);
    func_80071A64(52u);
    int r0 = (int)func_80071A54(), r1 = (int)func_80071A54(), r2 = (int)func_80071A54(),
        r3 = (int)func_80071A54();
    func_80071A64(52u);
    ASSERT(func_800DCE94(1, PAO_REC) == 0, "DCE94: mode 1");
    pe_addr_t p = PAO_POOL + 16u;
    ASSERT(pao_s16(p + 12u) == (int16_t)((r0 & 0x1FF) - 0x100) && PE_LoadU16(p + 2u) == 0u &&
           PE_LoadU16(p + 4u) == (uint16_t)r1 && pao_s16(p + 14u) == (int16_t)((r2 & 0x1F) - 0x10) &&
           PE_LoadU32(PAO_REC + 8u) == 0x10000u + 0x8AAu + (unsigned)(r3 & 0x1F), "DCE94: spawn");
    PE_StoreU32(0x800E27ECu, 0x49u);
    ASSERT(func_800DCE94(1, PAO_REC) == 1, "DCE94: ends at 0x49");
    /* mode 2 from 0x41: no draw, publish + reduced sprite state */
    PE_StoreU16(0x800F3368u, 0x77u); PE_StoreU16(0x800F3372u, 0x77u);
    PE_StoreU32(0x800E27ECu, 0x41u);
    ASSERT(func_800DCE94(2, PAO_REC) == 0, "DCE94: mode 2");
    ASSERT(PE_LoadU16(0x800E222Cu) == 0x1111u && PE_LoadU16(0x800E222Eu) == 0xFF00u &&
           PE_LoadU16(0x800E2230u) == 0x3333u, "DCE94: publishes a1");
    ASSERT(pao_state_ok(0xAAAAu, 0u, 8u, 0) && PE_LoadU16(0x800F3368u) == 0x77u &&
           PE_LoadU16(0x800F3372u) == 0x77u, "DCE94: only 336C/3370/336E/3374 written");
    PASS();
}

static void test_PA_ovl_DD19C_DD76C(void)
{
    TEST("PA_ovl_DD19C_DD76C");
    pao_world();
    func_80071A64(61u);
    unsigned r = func_80071A54();
    func_80071A64(61u);
    ASSERT(func_800DD19C(0, PAO_REC) == 0, "DD19C: mode 0");
    ASSERT(PE_LoadU32(PAO_REC + 8u) == r && PE_LoadU16(PAO_REC + 0u) == 0x123u &&
           pao_s16(PAO_REC + 2u) == -0x200 - 0x1FE, "DD19C: seed, player mode 1, y - 0x1FE");
    PE_StoreU32(0x800E27ECu, 0x3Fu);
    ASSERT(func_800DD19C(1, PAO_REC) == 0, "DD19C: runs below 0x40");
    PE_StoreU32(0x800E27ECu, 0x40u);
    ASSERT(func_800DD19C(1, PAO_REC) == 1, "DD19C: ends at 0x40");

    pao_world();
    ASSERT(pao_pool_ok(0x10u, 0x18u, 0x800DD380u, func_800DD76C(0, PAO_REC)), "DD76C: pool 0x18 x 0x10");
    ASSERT(PE_LoadU16(PAO_REC + 8u) == 0x123u && pao_s16(PAO_REC + 10u) == -0x200 - 0x226 &&
           PE_LoadU16(PAO_REC + 12u) == 0x456u, "DD76C: mirror, a1[5] -= 0x226");
    PE_StoreU32(0x800E27ECu, 0x2Fu);
    func_80071A64(62u);
    int r0 = (int)func_80071A54(), r1 = (int)func_80071A54(), r2 = (int)func_80071A54();
    func_80071A64(62u);
    ASSERT(func_800DD76C(1, PAO_REC) == 0, "DD76C: mode 1");
    pe_addr_t p = PAO_POOL + 16u;
    ASSERT(pao_s16(p + 0u) == (int16_t)(0x123 + r0 % 700 - 350) && pao_s16(p + 2u) == -0x200 &&
           pao_s16(p + 4u) == (int16_t)(0x456 + r1 % 700 - 350) && PE_LoadU16(p + 8u) == 0u &&
           PE_LoadU16(p + 10u) == 0u && PE_LoadU16(p + 14u) == (uint16_t)r2, "DD76C: spawn");
    PE_StoreU32(0x800E27ECu, 0x8Cu);
    ASSERT(func_800DD76C(1, PAO_REC) == 1, "DD76C: ends at 0x8C");
    ASSERT(func_800DD76C(2, PAO_REC) == 0 && PE_LoadU16(0x800E2234u) == 0x123u &&
           pao_s16(0x800E2236u) == -0x200 - 0x226 && pao_state_ok(0xAAAAu, 0u, 0x10u, 1),
           "DD76C: publishes a1[4..6], sprite state");
    PASS();
}

static void pao_glow(pe_addr_t g, unsigned st, unsigned speed, unsigned delay, unsigned alpha)
{
    PE_StoreU8(g + 0u, (uint8_t)st); PE_StoreU8(g + 1u, (uint8_t)speed);
    PE_StoreU8(g + 2u, (uint8_t)delay); PE_StoreU8(g + 3u, (uint8_t)alpha);
    PE_StoreU16(g + 4u, 0x1234u); PE_StoreU16(g + 6u, 0x5678u); PE_StoreU16(g + 8u, 0x9ABCu);
    PE_StoreU16(g + 0xEu, 0xFFF0u);
}

/* func_800E051C (now matched + native) draws for real: a valid MATRIX behind
 * D_800BCFA4 (it is published to scratchpad +0x34), a scale behind
 * D_800BCFA8, and packet / ordering-table buffers.  The translation z
 * compensates pao_glow's vector z (0x9ABC = -25924) so sz = 1000 and the
 * ordering-table slot (sz >> 2) - 8 = 242 lies inside PAO_GLOW_OT.  Each draw
 * advances the packet cursor D_8009CDD8 by 0x28. */
#define PAO_GLOW_MAT 0x80162000u
#define PAO_GLOW_SCL 0x80162100u
#define PAO_GLOW_OT  0x80163000u
#define PAO_GLOW_PKT 0x80164000u
static void pao_glow_draw_env(void)
{
    unsigned i;
    for (i = 0; i < 32u; i += 4u) PE_StoreU32(PAO_GLOW_MAT + i, 0u);
    PE_StoreU16(PAO_GLOW_MAT + 0u, 0x1000u); PE_StoreU16(PAO_GLOW_MAT + 8u, 0x1000u);
    PE_StoreU16(PAO_GLOW_MAT + 16u, 0x1000u);
    PE_StoreU32(PAO_GLOW_MAT + 28u, 25924u + 1000u);
    PE_StoreU32(PAO_GLOW_SCL, 256u);
    PE_StoreU32(0x800BCFA4u, PAO_GLOW_MAT);
    PE_StoreU32(0x800BCFA8u, PAO_GLOW_SCL);
    PE_StoreU32(0x8009CDD8u, 0u); PE_StoreU32(0x8009CDDCu, 0u);
    PE_StoreU32(0x800B0E38u, PAO_GLOW_OT); PE_StoreU32(0x800B0E58u, PAO_GLOW_PKT);
    for (i = 0; i < 0x400u; i += 4u) PE_StoreU32(PAO_GLOW_OT + i, 0x00FFFFFFu);
}

static void test_PA_ovl_E026C(void)
{
    TEST("PA_ovl_E026C");
    const pe_addr_t g = PAO_REC;
    pao_world();
    pao_glow(g, 0u, 2u, 0u, 0x20u);
    PE_StoreU16(0x1F80001Eu, 0xEEEEu);
    func_800E026C(g);
    ASSERT(PE_LoadU16(0x1F80001Eu) == 0xEEEEu && PE_Decomp_BoundaryCount() == 0u, "E026C: inactive no-op");
    pao_glow_draw_env();
    PE_StoreU16(0x800E21A4u, 5u);
    pao_glow(g, 1u, 2u, 0u, 0x20u);
    func_800E026C(g);
    ASSERT(PE_LoadU16(0x1F80001Eu) == 0x77D3u && PE_LoadU16(0x1F800022u) == 0x34u &&
           PE_LoadU8(0x1F800018u) == 0x80u && PE_LoadU8(0x1F80001Au) == 0x80u &&
           PE_LoadU8(0x1F80001Cu) == 0x20u && PE_LoadU8(0x1F80001Du) == 0xD0u &&
           PE_LoadU16(0x1F800026u) == 0x5678u && PE_LoadU32(0x1F80002Cu) == 0xFFFFFFF0u &&
           PE_LoadU32(0x1F800034u) == PAO_GLOW_MAT, "E026C: scratchpad (alpha before the step)");
    ASSERT(PE_LoadU8(g + 3u) == 0x30u && PE_LoadU8(g + 2u) == 2u && PE_LoadU8(g) == 1u,
           "E026C: alpha += 0x10, delay reloads");
    ASSERT(PE_LoadU32(0x8009CDD8u) == 0x28u && PE_LoadU8(PAO_GLOW_PKT + 7u) == 0x2Eu,
           "E026C: func_800E051C drew one glow packet (cursor +0x28)");
    func_800E026C(g);
    ASSERT(PE_LoadU8(g + 2u) == 1u && PE_LoadU8(g + 3u) == 0x30u &&
           PE_LoadU32(0x8009CDD8u) == 0x50u, "E026C: delay counts down (still drawn)");
    PE_StoreU8(g + 2u, 0u);
    func_800E026C(g);
    ASSERT(PE_LoadU8(g) == 0u && PE_LoadU8(g + 3u) == 0u && PE_LoadU16(0x800E21A4u) == 4u &&
           PE_LoadU32(0x8009CDD8u) == 0x50u, "E026C: past 0x30 frees without the draw");
    pao_glow(g, 2u, 2u, 0u, 0x20u);
    func_800E026C(g);
    ASSERT(PE_LoadU8(g + 3u) == 0x20u && PE_LoadU32(0x8009CDD8u) == 0x50u, "E026C: state 2 only publishes");
    PASS();
}

static void test_PA_ovl_E03A0(void)
{
    TEST("PA_ovl_E03A0");
    const pe_addr_t g = PAO_REC;
    pao_world();
    pao_glow(g, 1u, 0x40u, 0u, 0u);
    PE_StoreU16(g + 0xCu, 0x50u);
    PE_StoreU8(g + 0x10u, 0x60u); PE_StoreU8(g + 0x11u, 0x20u); PE_StoreU8(g + 0x12u, 0xFFu);
    pao_glow_draw_env();
    func_800E03A0(g);
    ASSERT(PE_LoadU16(0x1F80001Eu) == 0x7713u && PE_LoadU8(0x1F80001Cu) == 0xC0u &&
           PE_LoadU8(0x1F80001Du) == 0xCAu && PE_LoadU16(0x1F800028u) == 0x9ABCu,
           "E03A0: scratchpad parameters");
    ASSERT(PE_LoadU8(0x1F800018u) == 0x10u && PE_LoadU8(0x1F800019u) == 0u &&
           PE_LoadU8(0x1F80001Au) == 0xAFu, "E03A0: colour minus level, clamped at 0");
    ASSERT(PE_LoadU16(g + 0xCu) == 0x90u && PE_LoadU8(g) == 1u, "E03A0: fade up by a0[1]");
    /* src/func_800E051C.c is void(void): it reads the scratchpad block only. */
    ASSERT(PE_LoadU32(0x8009CDD8u) == 0x28u && PE_LoadU8(PAO_GLOW_PKT + 7u) == 0x2Eu &&
           PE_LoadU8(PAO_GLOW_PKT + 4u) == 0x10u && PE_LoadU8(PAO_GLOW_PKT + 6u) == 0xAFu,
           "E03A0: func_800E051C drew the glow with the clamped colour");
    PE_StoreU16(g + 0xCu, 0xF0u);
    func_800E03A0(g);
    ASSERT(PE_LoadU16(g + 0xCu) == 0xFFu && PE_LoadU8(g) == 2u, "E03A0: clamp at 0xFF, state 2");
    PE_StoreU16(0x800E21A4u, 3u);
    PE_StoreU16(g + 0xCu, 0x30u);
    func_800E03A0(g);
    ASSERT(PE_LoadU8(g) == 0u && PE_LoadU16(g + 0xCu) == 0u && PE_LoadU16(0x800E21A4u) == 2u &&
           PE_LoadU32(0x8009CDD8u) == 0x50u, "E03A0: fade-down below 0 frees, no draw");
    PASS();
}

static void test_PA_ovl_all(void)
{
    test_PA_ovl_C65E4();
    test_PA_ovl_D5010();
    test_PA_ovl_D6A1C();
    test_PA_ovl_D8B6C();
    test_PA_ovl_D9554();
    test_PA_ovl_D9FD4();
    test_PA_ovl_DA5D4();
    test_PA_ovl_DA934();
    test_PA_ovl_DBA9C();
    test_PA_ovl_DBE6C_DC750();
    test_PA_ovl_DCA80();
    test_PA_ovl_DCE94();
    test_PA_ovl_DD19C_DD76C();
    test_PA_ovl_E026C();
    test_PA_ovl_E03A0();
}
