/*
 * port_absent lane round 2 (2026-09-24) — tests for the ovl2 group
 * (pc_port/game/decomp_hand/absent_ovl2_port.c).  Expectations are computed
 * from the matched leaf src/func_XXXXXXXX.c, never from the port.  Random
 * draws are replayed by re-seeding the BIOS rand (func_80071A64) and calling
 * func_80071A54 in the leaf's evaluation order; sin/cos come from the
 * matched func_80077DC4 / func_80077CF4.
 *
 * Mode-2 paths that reach func_800CEE20 / func_800D004C / func_800D0728 need
 * a live GPU packet arena and are only exercised on frames where the matched
 * C skips the draw (its non-draw state is asserted).
 */

#define PO2_CTX    0x8016C000u   /* D_800F33E0 target; +8 = pool          */
#define PO2_POOL   0x8016C100u
#define PO2_POOL2  0x8016C800u   /* a separate pool (D_800E21F4 / 2208)   */
#define PO2_ACTOR  0x8016D000u   /* D_8009D254 target                     */
#define PO2_MTX    0x8016D400u   /* actor +0x238                          */
#define PO2_TGT    0x8016D800u   /* D_800F32D0 target; +8 = PO2_TOBJ      */
#define PO2_TOBJ   0x8016D900u
#define PO2_OBJ    0x8016DC00u   /* D_800E2368 target (+0x1E kind)        */
#define PO2_REC    0x8016E000u   /* the leaf's a1 record                  */

static void po2_world(void)
{
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();
    PE_StoreU32(0x800F33E0u, PO2_CTX);
    PE_StoreU32(PO2_CTX + 8u, PO2_POOL);
    PE_StoreU32(0x8009D254u, PO2_ACTOR);
    PE_StoreU32(PO2_ACTOR + 0x238u, PO2_MTX);
    PE_StoreU32(PO2_MTX + 20u, 0x00011111u);          /* mode 0 x          */
    PE_StoreU32(PO2_MTX + 24u, 0x0002FF00u);          /* y = -0x100        */
    PE_StoreU32(PO2_MTX + 28u, 0x00033333u);
    PE_StoreU32(PO2_ACTOR + 0x28u, 0x01230000u);      /* mode 1 x (>> 16)  */
    PE_StoreU32(PO2_ACTOR + 0x2Cu, 0xFE000000u);
    PE_StoreU32(PO2_ACTOR + 0x30u, 0x04560000u);
    PE_StoreU32(0x800F32D0u, PO2_TGT);
    PE_StoreU32(PO2_TGT + 8u, PO2_TOBJ);
    PE_StoreU16(PO2_TOBJ + 0x268u, 0x0101u);
    PE_StoreU16(PO2_TOBJ + 0x26Au, 0xF000u);
    PE_StoreU16(PO2_TOBJ + 0x26Cu, 0x0303u);
    PE_StoreU32(0x800E2368u, PO2_OBJ);
    PE_StoreU16(0x800F34E4u, 1u);   /* func_800CEDA8(1): texture already set */
    PE_StoreU16(0x800E11E6u, 3u);
    PE_StoreU16(0x800E11F6u, 5u);
    PE_StoreU16(0x800E2850u + 6u, 0xAAAAu);
    PE_StoreU16(0x800E2850u + 10u, 0xBBBBu);
}

static int16_t po2_s16(pe_addr_t a) { return (int16_t)PE_LoadU16(a); }
static void po2_set(pe_addr_t a, int v) { PE_StoreU16(a, (uint16_t)v); }
#define R2(k) (PO2_REC + 2u * (k))
#define PO2_P1 (PO2_POOL + 16u)    /* first particle body of a fresh pool */

static int po2_pool_ok(pe_addr_t pool, unsigned size, unsigned count, uint32_t fn, int ret)
{
    return ret == (int)((size + 4u) * count + 12u) &&
           PE_LoadU32(pool) == size + 4u && PE_LoadU32(pool + 4u) == count &&
           PE_LoadU32(pool + 8u) == fn;
}

/* ix / size / mode / av part of the sprite state. */
static int po2_prefix_ok(unsigned size, unsigned mode, unsigned av)
{
    return PE_LoadU16(0x800F3368u) == size && PE_LoadU16(0x800F336Au) == mode &&
           PE_LoadU16(0x800F3376u) == size && PE_LoadU16(0x800F3378u) == size &&
           PE_LoadU16(0x800F336Cu) == 1u && PE_LoadU16(0x800F3370u) == av;
}

static void test_PA_ovl2_D7764(void)
{
    TEST("PA_ovl2_D7764");
    po2_world();
    po2_set(R2(4), 9); po2_set(R2(5), 9);
    ASSERT(func_800D7764(0, PO2_REC) == 0, "D7764: mode 0 returns 0");
    ASSERT(po2_s16(R2(4)) == 0 && po2_s16(R2(5)) == 0, "D7764: a1[4] = a1[5] = 0");
    ASSERT(PE_LoadU16(R2(0)) == 0x1111u && PE_LoadU16(R2(1)) == 0xFF00u &&
           PE_LoadU16(R2(2)) == 0x3333u, "D7764: player position, func_800CE870 mode 0");
    PE_StoreU32(0x800E27ECu, 0x3Bu);
    ASSERT(func_800D7764(1, PO2_REC) == 0, "D7764: runs below 0x3C");
    PE_StoreU32(0x800E27ECu, 0x3Cu);
    ASSERT(func_800D7764(1, PO2_REC) == 1 && func_800D7764(7, PO2_REC) == 0, "D7764: ends at 0x3C");
    PASS();
}

static void test_PA_ovl2_D7B70(void)
{
    TEST("PA_ovl2_D7B70");
    po2_world();
    func_80071A64(0x55u);
    unsigned r = func_80071A54();
    func_80071A64(0x55u);
    PE_StoreU32(R2(6), 0xDEADu);
    ASSERT(po2_pool_ok(PO2_POOL, 8u, 0x18u, 0x800D7A1Cu, func_800D7B70(0, PO2_REC)),
           "D7B70: pool 0x18 x 8 bytes, callback func_800D7A1C");
    ASSERT(PE_LoadU32(R2(4)) == r && PE_LoadU32(R2(6)) == 0u, "D7B70: angle = rand, radius = 0");
    ASSERT(PE_LoadU16(R2(0)) == 0x1111u, "D7B70: player position mode 0");
    /* mode 1 at frame 5: angle 0x300, radius 1000 */
    PE_StoreU32(R2(4), 0x300u);
    PE_StoreU32(R2(6), 1000u);
    po2_set(R2(0), 10); po2_set(R2(1), -20); po2_set(R2(2), 30);
    PE_StoreU32(0x800E27ECu, 5u);
    func_80071A64(9u);
    unsigned r1 = func_80071A54(), r2 = func_80071A54();
    func_80071A64(9u);
    ASSERT(func_800D7B70(1, PO2_REC) == 0, "D7B70: mode 1 below 0x4A");
    ASSERT(po2_s16(PO2_P1 + 0u) == (int16_t)(10 + func_80077DC4(0x300) * 1000 / 4096) &&
           po2_s16(PO2_P1 + 4u) == (int16_t)(30 + func_80077CF4(0x300) * 1000 / 4096) &&
           po2_s16(PO2_P1 + 2u) == -20 && po2_s16(PO2_P1 + 6u) == (int16_t)(r1 & 3),
           "D7B70: p = a1 + (sin, cos)(angle) * radius / 4096, p[3] = rand & 3");
    ASSERT(PE_LoadU32(R2(4)) == 0x300u + (r2 & 0x1Fu) + 0x8AAu, "D7B70: angle advance");
    PE_StoreU32(0x800E27ECu, 0x4Au);
    ASSERT(func_800D7B70(1, PO2_REC) == 1, "D7B70: ends at 0x4A");
    /* mode 2 at 0x40: no ring (>= 0x33), sprite state 0x10/1, bias 4 */
    PE_StoreU32(0x800E27ECu, 0x40u);
    ASSERT(func_800D7B70(2, PO2_REC) == 0, "D7B70: mode 2");
    ASSERT(po2_prefix_ok(0x10u, 1u, 0xAAAAu) && PE_LoadU16(0x800F336Eu) == 0u &&
           PE_LoadU16(0x800F3372u) == 0u && PE_LoadU16(0x800F3374u) == 4u,
           "D7B70: sprite state ix D_800E11E6, 0x10/1, bias 4");
    PASS();
}

static void test_PA_ovl2_D7FBC(void)
{
    TEST("PA_ovl2_D7FBC");
    po2_world();
    (void)func_800CE560(PO2_POOL2, 8u, 4, 0u);
    PE_StoreU32(0x800E21F4u, PO2_POOL2);
    PE_StoreU16(0x800E21ECu, 1000u);
    PE_StoreU16(0x800E21F0u, 2000u);
    PE_StoreU16(PO2_OBJ + 0x1Eu, 0xBu);          /* v = 1: spawn every frame */
    po2_set(R2(1), 0x200); po2_set(R2(2), 4); po2_set(R2(4), -50); po2_set(R2(6), 600);
    PE_StoreU32(0x800E27ECu, 7u);
    int x = 1000 + func_80077DC4(0x200) * 600 / 4096;
    int z = 2000 + func_80077CF4(0x200) * 600 / 4096;
    int rad = func_80077DC4((7 << 10) / 36) * 700 / 4096;
    func_80071A64(33u);
    unsigned r1 = func_80071A54(), r2 = func_80071A54(), r3 = func_80071A54();
    func_80071A64(33u);
    ASSERT(func_800D7FBC(1, PO2_REC) == 0, "D7FBC: mode 1 below 0x24");
    ASSERT(po2_s16(R2(3)) == (int16_t)x && po2_s16(R2(5)) == (int16_t)z,
           "D7FBC: orbit about D_800E21EC/D_800E21F0");
    ASSERT(po2_s16(R2(1)) == 0x260 && po2_s16(R2(6)) == (int16_t)rad && po2_s16(R2(2)) == 5,
           "D7FBC: angle += 0x60, radius sweep, a1[2]++");
    pe_addr_t p = PO2_POOL2 + 16u;
    ASSERT(po2_s16(p + 0u) == (int16_t)(x + (int)(r1 & 7) - 3) &&
           po2_s16(p + 2u) == (int16_t)(-50 + (int)(r2 & 7) - 3) &&
           po2_s16(p + 4u) == (int16_t)(z + (int)(r3 & 7) - 3), "D7FBC: jittered particle");
    /* v = 3 (kind != 0xB): frame 7 % 3 != 0 -> no second particle */
    PE_StoreU16(PO2_OBJ + 0x1Eu, 0u);
    ASSERT(func_800D7FBC(1, PO2_REC) == 0 && PE_LoadU16(PO2_POOL2 + 12u + 12u) == 0u,
           "D7FBC: every third frame otherwise");
    PE_StoreU32(0x800E27ECu, 0x24u);
    ASSERT(func_800D7FBC(1, PO2_REC) == 1 && func_800D7FBC(0, PO2_REC) == 0, "D7FBC: ends at 0x24");
    PASS();
}

static void test_PA_ovl2_D8E74(void)
{
    TEST("PA_ovl2_D8E74");
    po2_world();
    (void)func_800CE560(PO2_POOL2, 8u, 4, 0u);
    PE_StoreU32(0x800E2208u, PO2_POOL2);
    PE_StoreU16(0x800E2200u, 100u);
    PE_StoreU16(0x800E2202u, 3000u);
    PE_StoreU16(0x800E2204u, 200u);
    po2_set(R2(1), 0); po2_set(R2(6), 0); po2_set(R2(7), 1);
    PE_StoreU32(0x800E27ECu, 9u);                 /* odd: no spawn */
    ASSERT(func_800D8E74(1, PO2_REC) == 0, "D8E74: mode 1");
    ASSERT(po2_s16(R2(4)) == 3000 - 9 * 500 / 36, "D8E74: a1[7] != 0 sweeps down");
    ASSERT(PE_LoadU16(PO2_POOL2 + 12u) == 0u, "D8E74: odd frame spawns nothing");
    po2_set(R2(7), 0);
    PE_StoreU32(0x800E27ECu, 10u);
    ASSERT(func_800D8E74(1, PO2_REC) == 0, "D8E74: mode 1 even frame");
    ASSERT(po2_s16(R2(4)) == 3000 + (10 * 500 / 36 - 1000), "D8E74: a1[7] == 0 sweeps up");
    ASSERT(PE_LoadU16(PO2_POOL2 + 12u) == 1u, "D8E74: even frame spawns");
    PE_StoreU32(0x800E27ECu, 0x24u);
    ASSERT(func_800D8E74(1, PO2_REC) == 1, "D8E74: ends at 0x24");
    PASS();
}

static void test_PA_ovl2_DACA4(void)
{
    TEST("PA_ovl2_DACA4");
    po2_world();
    func_80071A64(0x77u);
    unsigned r = func_80071A54();
    func_80071A64(0x77u);
    ASSERT(po2_pool_ok(PO2_POOL, 8u, 0x18u, 0x800DAB98u, func_800DACA4(0, PO2_REC)),
           "DACA4: pool 0x18 x 8 bytes, callback D_800DAB98");
    ASSERT(PE_LoadU32(R2(4)) == r && PE_LoadU16(R2(0)) == 0x1111u, "DACA4: angle = rand, position");
    PE_StoreU32(R2(4), 0x12345u);
    PE_StoreU32(0x800E27ECu, 0x10u);
    func_80071A64(3u);
    unsigned r1 = func_80071A54(), r2 = func_80071A54(), r3 = func_80071A54();
    func_80071A64(3u);
    ASSERT(func_800DACA4(1, PO2_REC) == 0, "DACA4: mode 1 below 0x50");
    ASSERT(po2_s16(PO2_P1 + 2u) == (int16_t)((r1 & 0xFF) + 500) &&
           po2_s16(PO2_P1 + 6u) == (int16_t)((r2 & 3) + 0x16) &&
           PE_LoadU16(PO2_P1 + 4u) == 0x2345u, "DACA4: p[1], p[3], p[2] = (short)angle");
    ASSERT(PE_LoadU32(R2(4)) == 0x12345u + (r3 & 0x1Fu) + 0x8AAu, "DACA4: angle advance");
    PE_StoreU32(0x800E27ECu, 0x50u);
    ASSERT(func_800DACA4(1, PO2_REC) == 1, "DACA4: ends at 0x50");
    /* mode 2 at 0x60: no bursts; D_800F3374 = 0x40, func_800CF5B0(a1, 0) */
    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();
    PE_StoreU32(0x800E27ECu, 0x60u);
    ASSERT(func_800DACA4(2, PO2_REC) == 0, "DACA4: mode 2");
    ASSERT(PE_LoadU16(0x800F3374u) == 0x40u, "DACA4: D_800F3374 = 0x40");
    ASSERT(PE_Decomp_BoundaryCount() == 1u &&
           strcmp(PE_Decomp_BoundaryName(0), "func_800CF5B0") == 0 &&
           g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].arg0 == PO2_REC && g_bootstrap_arg4_calls[0].arg1 == 0u,
           "DACA4: func_800CF5B0(a1, 0) boundary");
    PASS();
}

static void test_PA_ovl2_DC5BC(void)
{
    TEST("PA_ovl2_DC5BC");
    po2_world();
    po2_set(R2(0), 0); po2_set(R2(1), 1); po2_set(R2(2), 100);
    PE_StoreU32(0x800E27ECu, 0xDu);
    func_80071A64(11u);
    unsigned r1 = func_80071A54(), r2 = func_80071A54(), r3 = func_80071A54();
    func_80071A64(11u);
    ASSERT(func_800DC5BC(1, PO2_REC) == 0, "DC5BC: mode 1 below 0xE");
    ASSERT(PE_LoadU16(R2(0)) == (uint16_t)(0xFFFDu + (r1 & 7u)) &&
           PE_LoadU16(R2(2)) == (uint16_t)(97u + (r2 & 7u)) &&
           PE_LoadU16(R2(1)) == (uint16_t)(0xFFFFu - (r3 & 3u)),
           "DC5BC: x/z - 3 + (rand & 7), y - 2 - (rand & 3) as u16");
    PE_StoreU32(0x800E27ECu, 0xEu);
    ASSERT(func_800DC5BC(1, PO2_REC) == 1 && func_800DC5BC(0, PO2_REC) == 0, "DC5BC: ends at 0xE");
    PASS();
}

static void test_PA_ovl2_DD380(void)
{
    TEST("PA_ovl2_DD380");
    po2_world();
    po2_set(R2(4), 1); po2_set(R2(5), 7);
    ASSERT(func_800DD380(1, PO2_REC) == 0 && po2_s16(R2(5)) == 8, "DD380: a1[5]++, stage < 2");
    po2_set(R2(4), 2);
    ASSERT(func_800DD380(1, PO2_REC) == 1 && po2_s16(R2(5)) == 9, "DD380: stage 2 ends");
    ASSERT(func_800DD380(0, PO2_REC) == 0, "DD380: default");
    PASS();
}

static void test_PA_ovl2_DDD70(void)
{
    TEST("PA_ovl2_DDD70");
    po2_world();
    ASSERT(po2_pool_ok(PO2_POOL, 0x10u, 0x20u, 0x800DD9E4u, func_800DDD70(0, PO2_REC)),
           "DDD70: pool 0x20 x 0x10, callback func_800DD9E4");
    ASSERT(PE_LoadU16(R2(0)) == 0x123u && PE_LoadU16(R2(1)) == 0xFE00u &&
           PE_LoadU16(R2(2)) == 0x456u, "DDD70: player position mode 1");
    PE_StoreU32(0x800E27ECu, 0x20u);
    func_80071A64(21u);
    unsigned r1 = func_80071A54(), r2 = func_80071A54(), r3 = func_80071A54(), r4 = func_80071A54();
    func_80071A64(21u);
    ASSERT(func_800DDD70(1, PO2_REC) == 0, "DDD70: mode 1 below 0x56");
    ASSERT(po2_s16(PO2_P1 + 0u) == (int16_t)(0x123 + (int)(r1 & 0x1FF) - 0x100) &&
           po2_s16(PO2_P1 + 2u) == (int16_t)(-0x200 - (int)r2 % 600) &&
           po2_s16(PO2_P1 + 4u) == (int16_t)(0x456 + (int)(r3 & 0x1FF) - 0x100) &&
           po2_s16(PO2_P1 + 8u) == 0 && po2_s16(PO2_P1 + 10u) == 0 &&
           PE_LoadU16(PO2_P1 + 12u) == (uint16_t)r4, "DDD70: spawned spark");
    PE_StoreU32(0x800E27ECu, 0x56u);
    ASSERT(func_800DDD70(1, PO2_REC) == 1, "DDD70: ends at 0x56");
    /* mode 2 at 0x60: anchor words (int -> short) published, no draw,
     * sprite state ix D_800E11E6 with D_800F3374 left alone */
    PE_StoreU32(PO2_MTX + 0x274u, 0x00010005u);
    PE_StoreU32(PO2_MTX + 0x278u, 0xFFFFFFF0u);
    PE_StoreU32(PO2_MTX + 0x27Cu, 0x7777u);
    PE_StoreU16(0x800F3374u, 0x99u);
    PE_StoreU32(0x800E27ECu, 0x60u);
    ASSERT(func_800DDD70(2, PO2_REC) == 0, "DDD70: mode 2");
    ASSERT(PE_LoadU16(0x800E223Cu) == 5u && PE_LoadU16(0x800E223Eu) == 0xFFF0u &&
           PE_LoadU16(0x800E2240u) == 0x7777u, "DDD70: anchor published to D_800E223C..40");
    ASSERT(po2_prefix_ok(0x20u, 2u, 0xAAAAu) && PE_LoadU16(0x800F336Eu) == 0u &&
           PE_LoadU16(0x800F3372u) == 0u && PE_LoadU16(0x800F3374u) == 0x99u,
           "DDD70: sprite state, D_800F3374 untouched");
    PASS();
}

static void test_PA_ovl2_DEA30(void)
{
    TEST("PA_ovl2_DEA30");
    po2_world();
    func_80071A64(0x99u);
    unsigned r = func_80071A54();
    func_80071A64(0x99u);
    ASSERT(po2_pool_ok(PO2_POOL, 0x10u, 0x20u, 0x800DE7A8u, func_800DEA30(0, PO2_REC)),
           "DEA30: pool 0x20 x 0x10, callback func_800DE7A8");
    ASSERT(PE_LoadU32(R2(4)) == r && PE_LoadU16(R2(0)) == 0x0101u &&
           PE_LoadU16(R2(1)) == 0xF000u && PE_LoadU16(R2(2)) == 0x0303u,
           "DEA30: angle = rand, target position");
    /* mode 1 at frame 2: one particle filled twice; the second fill wins */
    PE_StoreU32(R2(4), 0u);
    PE_StoreU32(0x800E27ECu, 2u);
    func_80071A64(4u);
    unsigned rr[16];
    for (unsigned i = 0; i < 16u; i++) rr[i] = func_80071A54();
    func_80071A64(4u);
    ASSERT(func_800DEA30(1, PO2_REC) == 0, "DEA30: mode 1 below 0x30");
    const unsigned *s = rr + 8;   /* second pass: 8 draws per pass */
    ASSERT(po2_s16(PO2_P1 + 6u) == (int16_t)((int)s[0] % 48 - 24) &&
           po2_s16(PO2_P1 + 10u) == (int16_t)(-((int)s[1] % 80)) &&
           po2_s16(PO2_P1 + 8u) == (int16_t)((int)s[2] % 48 - 32),
           "DEA30: p[3..5] = v (x, y, z drawn in x, z, y order)");
    ASSERT(po2_s16(PO2_P1 + 0u) == (int16_t)(0x0101 + (int)(s[3] & 0xFF) - 0x80) &&
           po2_s16(PO2_P1 + 2u) == (int16_t)(-0x1000 + (int)(s[4] & 0xFF) - 0x80) &&
           po2_s16(PO2_P1 + 4u) == (int16_t)(0x0303 + (int)(s[5] & 0xFF) - 0x80) &&
           po2_s16(PO2_P1 + 14u) == (int16_t)((s[6] & 0x3F) + 0x50) &&
           PE_LoadU16(PO2_P1 + 12u) == (uint16_t)s[7], "DEA30: position, p[7], p[6]");
    ASSERT(PE_LoadU32(R2(4)) == 2u * 0x955u, "DEA30: angle += 0x955 per pass");
    ASSERT(PE_LoadU16(PO2_POOL + 12u + 0x14u) == 0u, "DEA30: only one particle allocated");
    ASSERT(g_bootstrap_arg4_call_count == 2 &&
           g_bootstrap_arg4_calls[0].arg0 == PO2_MTX &&
           g_bootstrap_arg4_calls[0].arg1 == PE_OVL2_V &&
           g_bootstrap_arg4_calls[0].arg2 == PE_OVL2_V, "DEA30: func_800CEAE8(stats, v, v) x2");
    /* mode 2 at 0x20: no sprite/ring; final state from the second block */
    po2_set(R2(1), 50);
    PE_StoreU32(0x800E27ECu, 0x20u);
    ASSERT(func_800DEA30(2, PO2_REC) == 0, "DEA30: mode 2");
    ASSERT(po2_s16(R2(1)) == 50, "DEA30: a1[1] only lowered below 0x19");
    ASSERT(po2_prefix_ok(0x20u, 2u, 0xAAAAu) && PE_LoadU16(0x800F336Eu) == 0u &&
           PE_LoadU16(0x800F3372u) == 0u && PE_LoadU16(0x800F3374u) == 0x18u,
           "DEA30: second sprite state ix D_800E11E6, D_800F3374 = 0x18");
    PASS();
}

static void test_PA_ovl2_DEFFC(void)
{
    TEST("PA_ovl2_DEFFC");
    po2_world();
    /* kind 1: integrate + jitter; ends when a1[9] reaches 0x10 */
    po2_set(R2(8), 1); po2_set(R2(9), 5);
    po2_set(R2(0), 10); po2_set(R2(1), 20); po2_set(R2(2), 30);
    po2_set(R2(4), 1); po2_set(R2(5), -2); po2_set(R2(6), 3);
    func_80071A64(8u);
    unsigned r1 = func_80071A54(), r2 = func_80071A54(), r3 = func_80071A54();
    func_80071A64(8u);
    ASSERT(func_800DEFFC(1, PO2_REC) == 0, "DEFFC: kind 1 continues");
    ASSERT(po2_s16(R2(9)) == 6 && po2_s16(R2(0)) == (int16_t)(11 + (int)(r1 & 7) - 3) &&
           po2_s16(R2(1)) == (int16_t)(18 + (int)(r2 & 7) - 3) &&
           po2_s16(R2(2)) == (int16_t)(33 + (int)(r3 & 7) - 3), "DEFFC: kind 1 integration");
    po2_set(R2(9), 0xF);
    ASSERT(func_800DEFFC(1, PO2_REC) == 1, "DEFFC: kind 1 ends at 0x10");
    /* kind 2: integrate only; ends at 0x18 */
    po2_set(R2(8), 2); po2_set(R2(9), 0x16);
    po2_set(R2(0), 0); po2_set(R2(1), 0); po2_set(R2(2), 0);
    ASSERT(func_800DEFFC(1, PO2_REC) == 0 && po2_s16(R2(0)) == 1 && po2_s16(R2(1)) == -2 &&
           po2_s16(R2(2)) == 3 && po2_s16(R2(9)) == 0x17, "DEFFC: kind 2 integration");
    ASSERT(func_800DEFFC(1, PO2_REC) == 1, "DEFFC: kind 2 ends at 0x18");
    po2_set(R2(8), 3);
    ASSERT(func_800DEFFC(1, PO2_REC) == 0 && func_800DEFFC(2, PO2_REC) == 0 &&
           func_800DEFFC(0, PO2_REC) == 0, "DEFFC: other kinds / modes");
    /* kind 0 on frame & 3 != 0: joint position into a1, no spawn */
    po2_set(R2(8), 0); po2_set(R2(9), 0); po2_set(R2(3), 0);
    for (unsigned i = 0; i < 8u; i++) PE_StoreU8(0x800C22F0u + i, 0);
    PE_StoreU32(0x800E27ECu, 5u);
    ASSERT(func_800DEFFC(1, PO2_REC) == 0 && po2_s16(R2(9)) == 1, "DEFFC: kind 0 counts");
    ASSERT(PE_LoadU16(R2(0)) == 0x1111u && PE_LoadU16(R2(1)) == 0xFF00u &&
           PE_LoadU16(R2(2)) == 0x3333u, "DEFFC: func_800CE8F0 joint 0 position");
    ASSERT(PE_LoadU16(PO2_POOL + 12u) == 0u, "DEFFC: no spawn off-beat");
    PASS();
}

static void test_PA_ovl2_all(void)
{
    test_PA_ovl2_D7764();
    test_PA_ovl2_D7B70();
    test_PA_ovl2_D7FBC();
    test_PA_ovl2_D8E74();
    test_PA_ovl2_DACA4();
    test_PA_ovl2_DC5BC();
    test_PA_ovl2_DD380();
    test_PA_ovl2_DDD70();
    test_PA_ovl2_DEA30();
    test_PA_ovl2_DEFFC();
}
