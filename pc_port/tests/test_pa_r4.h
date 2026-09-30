/*
 * port_absent lane round 4 (2026-09-24). Expectations from the matched
 * leaf src/func_XXXXXXXX.c.  Guest scratch 0x80174000..0x80177FFF.
 */
#define PA_R4 0x80174000u

/* src/func_80084B78.c */
static void test_PA_r4_84B78_card_dispatch(void)
{
    TEST("PA_r4_84B78_card_dispatch");
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();
    const pe_addr_t r = PA_R4, b = PA_R4 + 0x200u;
    PE_StoreU32(r + 0x3Cu, b);

    /* *(a0+0x3C) == 0xF3 && a0[0xE8] == 0 -> tag 'C' with 0, return 0 */
    PE_StoreU8(b, 0xF3u);
    PE_StoreU8(r + 0x24u, 0x55u);
    ASSERT(func_80084B78(r) == 0, "84B78 returns 0");
    ASSERT(PE_LoadU8(r + 0x36u) == 0x43u && PE_LoadU8(r + 0x24u) == 0u &&
           PE_LoadU32(r + 0x2Cu) == r + 0x24u && PE_LoadU8(r + 0x35u) == 1u,
           "84B78 0xF3 & E8==0 -> 'C' with 0");
    /* a0[0xE8] != 0: dispatch on a0[0x46] */
    PE_StoreU8(r + 0xE8u, 1u);
    PE_StoreU8(r + 0x46u, 1u);
    (void)func_80084B78(r);
    ASSERT(PE_LoadU8(r + 0x36u) == 0x43u && PE_LoadU8(r + 0x24u) == 1u,
           "84B78 state 1 -> 'C' with 1");
    PE_StoreU8(r + 0x46u, 0xFEu);
    (void)func_80084B78(r);
    ASSERT(PE_LoadU8(r + 0x24u) == 0u, "84B78 state 0xFE -> 'C' with 0");
    /* 0 / 0xFF: nothing */
    PE_StoreU8(r + 0x36u, 0x11u);
    PE_StoreU8(r + 0x46u, 0xFFu);
    (void)func_80084B78(r);
    PE_StoreU8(r + 0x46u, 0u);
    (void)func_80084B78(r);
    ASSERT(PE_LoadU8(r + 0x36u) == 0x11u, "84B78 states 0/0xFF do nothing");
    /* default with a hook: called with a0 (loud boundary, no proven target) */
    PE_StoreU8(r + 0x46u, 5u);
    PE_StoreU32(r + 0x14u, 0x80012340u);
    (void)func_80084B78(r);
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == 0x80012340u &&
           g_bootstrap_arg4_calls[0].arg0 == r, "84B78 hook(a0)");
    /* default without a hook: func_800835C0(a0); state 2 -> func_80083E70
     * ('E' at +0x36) */
    PE_StoreU32(r + 0x14u, 0u);
    PE_StoreU8(r + 0x46u, 2u);
    (void)func_80084B78(r);
    ASSERT(PE_LoadU8(r + 0x36u) == 0x45u, "84B78 no hook -> func_800835C0");
    PASS();
}

/* src/func_80087FA0.c (the 0xBA/0xBC paths call other voice leaves and
 * are not driven here; every other slide is). */
static void test_PA_r4_87FA0_voice_slides(void)
{
    TEST("PA_r4_87FA0_voice_slides");
    ResetTestState();
    const pe_addr_t v = PA_R4 + 0x400u;
    const pe_addr_t t1 = PA_R4 + 0x800u, t2 = PA_R4 + 0x840u, t3 = PA_R4 + 0x880u;

    /* 0x72: +0x44 += +0x48; high-bit boundary crossing -> F4 |= 3.  I32
     * +0x44 overlaps S16 +0x46 (read later by the 0xA8 path, after this
     * update): +0x44 = 0x7FFFFFFF -> 0x80000000, so +0x46 becomes 0x8000. */
    PE_StoreU16(v + 0x72u, 1u);
    PE_StoreU32(v + 0x48u, 1u);
    /* 0x96: x = 0x7F00 + 0x200 = 0x8100 (bit 15 set): d = 1, m = d * +0x30,
     * +0x92 = m >> 7; table +0x1C redirected by t[2] = 3 halfwords */
    PE_StoreU16(v + 0x96u, 1u);
    PE_StoreU16(v + 0x94u, 0x7F00u);
    PE_StoreU16(v + 0x98u, 0x0200u);
    PE_StoreU32(v + 0x30u, 0x8000u);
    PE_StoreU32(v + 0x1Cu, t1);
    PE_StoreU16(t1 + 0u, 0u); PE_StoreU16(t1 + 2u, 0u); PE_StoreU16(t1 + 4u, 3u);
    PE_StoreU16(t1 + 6u, 0x4000u);
    /* 0xA8: +0xA6 += +0xAA; a = (S16(0x46) * (U16(0x6C) >> 8)) >> 7;
     * n = ((a * (A6 >> 8)) << 9) >> 16; n = (n * t[0]) >> 15 */
    PE_StoreU16(v + 0xA8u, 1u);
    PE_StoreU16(v + 0xA6u, 0x0100u);
    PE_StoreU16(v + 0xAAu, 0x0F00u);
    PE_StoreU32(v + 0x44u, 0x7FFFFFFFu);          /* sets S16 +0x46 = 0x7FFF */
    PE_StoreU16(v + 0x6Cu, 0xFF00u);
    PE_StoreU32(v + 0x20u, t2);
    PE_StoreU16(t2, 0x7FFFu);
    /* 0xB6: +0xB4 += +0xB8; n = ((B4 >> 8) * t[0]) >> 15 (signed) */
    PE_StoreU16(v + 0xB6u, 1u);
    PE_StoreU16(v + 0xB4u, 0x0100u);
    PE_StoreU16(v + 0xB8u, 0x0300u);
    PE_StoreU32(v + 0x24u, t3);
    PE_StoreU16(t3, 0xFF00u);                      /* -256 */
    /* 0x7A: +0x34 += +0x4C; bit-16 boundary crossing -> F4 |= 0x10 */
    PE_StoreU16(v + 0x7Au, 1u);
    PE_StoreU32(v + 0x34u, 0x0000FFFFu);
    PE_StoreU32(v + 0x4Cu, 1u);

    func_80087FA0(v, 4u);

    ASSERT(PE_LoadU16(v + 0x72u) == 0u && PE_LoadU32(v + 0x44u) == 0x80000000u,
           "87FA0 0x72 slide");
    ASSERT(PE_LoadU16(v + 0x94u) == 0x8100u && PE_LoadU16(v + 0x92u) == 0x100u,
           "87FA0 0x96: +0x94 wraps to 16 bits, +0x92 = (1 * 0x8000) >> 7");
    /* n = (0x100 * 0x4000) >> 16 = 0x40 != 0 -> E8 = 0x40, then n >= 0 -> 0x80 */
    ASSERT(PE_LoadU16(v + 0xE8u) == 0x80u, "87FA0 0x96: E8 = n * 2 for n >= 0");
    {
        int a = ((int16_t)0x8000 * (0xFF00 >> 8)) >> 7;   /* +0x46 after the 0x72 slide */
        int n = (int32_t)(((uint32_t)a * (uint32_t)(0x1000u >> 8)) << 9) >> 16;
        n = (n * 0x7FFF) >> 15;
        ASSERT(PE_LoadU16(v + 0xA6u) == 0x1000u &&
               (int16_t)PE_LoadU16(v + 0xEAu) == (int16_t)n, "87FA0 0xA8 volume");
    }
    ASSERT((int16_t)PE_LoadU16(v + 0xECu) == ((4 * -256) >> 15),
           "87FA0 0xB6: signed (4 * -256) >> 15 = -1");
    ASSERT(PE_LoadU32(v + 0x34u) == 0x10000u, "87FA0 0x7A slide");
    ASSERT(PE_LoadU32(v + 0xF4u) == (3u | 0x10u), "87FA0 flags 3 | 0x10");
    PASS();
}

/* src/func_8008D844.c */
static void test_PA_r4_8D844_fade_tick(void)
{
    TEST("PA_r4_8D844_fade_tick");
    ResetTestState();
    const pe_addr_t s = PA_R4 + 0x1000u;
    const pe_addr_t v0 = 0x800BC03Cu, v1 = 0x800BC03Cu + 0x11Cu;
    unsigned i;

    /* not every 4th call: nothing but the counter */
    PE_StoreU16(0x8009CDECu, 0u);
    PE_StoreU16(0x8009D220u, 2u);
    func_8008D844();
    ASSERT(PE_LoadU16(0x8009CDECu) == 1u && PE_LoadU16(0x8009D220u) == 2u,
           "8D844 skips unless (++counter & 3) == 0");

    PE_StoreU16(0x8009CDECu, 3u);
    PE_StoreU16(0x8009D2A2u, 0u);                /* no func_8008D7D0 */
    PE_StoreU32(0x8009D2D0u, 5u); PE_StoreU32(0x8009D214u, 7u);
    PE_StoreU16(0x8009D21Eu, 1u);
    PE_StoreU32(0x8009D2CCu, 0x0000FFFFu); PE_StoreU32(0x8009D210u, 1u);
    PE_StoreU32(0x8009D2C8u, s);
    PE_StoreU32(s + 4u, 1u); PE_StoreU16(s + 0x50u, 1u);
    PE_StoreU32(s + 0x48u, 0x10000u); PE_StoreU32(s + 0x4Cu, 0x100u);
    PE_StoreU32(s + 0x68u + 4u, 0u);             /* second Seq off */
    PE_StoreU32(0x800BCD50u, 0x3000u);           /* voices 0 and 1 */
    PE_StoreU16(v0 + 0x38u, 1u); PE_StoreU16(v0 + 0x9Cu, 0x00F0u);
    PE_StoreU16(v0 + 0x9Eu, 0x0020u);
    PE_StoreU16(v1 + 0x3Cu, 1u); PE_StoreU16(v1 + 0x3Au, 0x0100u);
    PE_StoreU16(v1 + 0xA0u, 0xFFFFu);            /* -1 */
    PE_StoreU16(v1 + 0x34u, 1u); PE_StoreU32(v1 + 0u, 0x1FFu);
    PE_StoreU32(v1 + 4u, 1u);

    func_8008D844();

    ASSERT(PE_LoadU16(0x8009D220u) == 1u && PE_LoadU32(0x8009D2D0u) == 12u,
           "8D844 D_8009D2D0 += D_8009D214");
    ASSERT(PE_LoadU32(0x8009D2CCu) == 0x10000u, "8D844 D_8009D2CC += D_8009D210");
    for (i = 0; i < 24u; i++)
        ASSERT(PE_LoadU32(0x800B8BB4u + i * 0x11Cu) == 0x10u,
               "8D844 bit-16 crossing flags all 24 voices");
    ASSERT(PE_LoadU16(s + 0x50u) == 0u && PE_LoadU32(s + 0x48u) == 0x10100u,
           "8D844 Seq fade (no 0x7F0000 crossing)");
    ASSERT(PE_LoadU32(0x8009D2C8u) == s, "8D844 restores D_8009D2C8");
    ASSERT(PE_LoadU16(v0 + 0x9Cu) == 0x0110u && PE_LoadU32(v0 + 0xB8u) == 3u,
           "8D844 voice 0 pan crosses 0xFF00 -> flags 3");
    ASSERT(PE_LoadU16(v1 + 0x3Au) == 0x00FFu && PE_LoadU32(v1 + 0u) == 0x200u &&
           PE_LoadU32(v1 + 0xB8u) == (3u | 0x10u),
           "8D844 voice 1 vol and pitch crossings");

    /* a mask bit below 0x1000 never clears: retail spins -> stop request */
    PE_StoreU16(0x8009CDECu, 3u);
    PE_StoreU16(0x8009D21Eu, 0u);
    PE_StoreU32(0x800BCD50u, 0x1u);
    func_8008D844();
    ASSERT(PE_Port_ShouldStop(), "8D844 stuck mask -> run-control stop");
    ResetTestState();
    PASS();
}

/* src/func_8008E2DC.c */
static void test_PA_r4_8E2DC_peek_event(void)
{
    TEST("PA_r4_8E2DC_peek_event");
    ResetTestState();
    const pe_addr_t a = PA_R4 + 0x2000u, p = PA_R4 + 0x2400u;
    PE_StoreU32(a, p);

#define R4_STREAM(...) do { static const unsigned char s_[] = {__VA_ARGS__}; \
        for (unsigned i_ = 0; i_ < sizeof s_; i_++) PE_StoreU8(p + i_, s_[i_]); } while (0)

    R4_STREAM(0x40);
    PE_StoreU16(a + 0x82u, 0x1234u); PE_StoreU16(a + 0x84u, 0xFFFFu);
    ASSERT(func_8008E2DC(a) == 0x40 && PE_LoadU16(a + 0x82u) == 0x1234u,
           "8E2DC note < 0x8F returned, fields kept");
    R4_STREAM(0x90);
    ASSERT(func_8008E2DC(a) == 0x90 && PE_LoadU16(a + 0x82u) == 0u &&
           PE_LoadU16(a + 0x84u) == 0xFFFAu, "8E2DC 0x8F..0x99 clear +0x82, +0x84 bits 0/2");
    R4_STREAM(0x9B);
    ASSERT(func_8008E2DC(a) == 0xA0, "8E2DC 0x9A..0x9F -> 0xA0");
    /* D_8009B7BC skip table */
    PE_StoreU8(0x8009B7BCu + 0xA5u, 2u);
    R4_STREAM(0xA5, 0xEE, 0x31);
    ASSERT(func_8008E2DC(a) == 0x31, "8E2DC D_8009B7BC[c] skip");
    /* 0xFC 6: signed 16-bit relative skip from after the operand */
    R4_STREAM(0xFC, 6, 0x02, 0x00, 0x77, 0x77, 0x33);
    ASSERT(func_8008E2DC(a) == 0x33, "8E2DC 0xFC 6 rel16");
    /* 0xFC 7: skip2 when D_8009D2C8->+0x56 < operand, else rel16 */
    PE_StoreU32(0x8009D2C8u, PA_R4 + 0x3000u);
    PE_StoreU16(PA_R4 + 0x3000u + 0x56u, 3u);
    R4_STREAM(0xFC, 7, 5, 0x10, 0x00, 0x35);
    /* +0x56 (3) < 5 -> skip2: p = after operand + 2 -> 0x35 */
    ASSERT(func_8008E2DC(a) == 0x35, "8E2DC 0xFC 7 skip2");
    PE_StoreU16(PA_R4 + 0x3000u + 0x56u, 9u);
    R4_STREAM(0xFC, 7, 5, 0x03, 0x00, 0x11, 0x11, 0x11, 0x36);
    ASSERT(func_8008E2DC(a) == 0x36, "8E2DC 0xFC 7 rel16 when +0x56 >= operand");
    /* 0xC9: loop-count match pops (k-- & 3), else jump to the k slot */
    PE_StoreU16(a + 0xCEu, 1u);
    PE_StoreU16(a + 0x62u + 2u, 4u);
    R4_STREAM(0xC9, 5, 0x21);
    ASSERT(func_8008E2DC(a) == 0x21, "8E2DC 0xC9 count match continues");
    PE_StoreU32(a + 4u + 4u, PA_R4 + 0x2800u);
    PE_StoreU8(PA_R4 + 0x2800u, 0x22u);
    R4_STREAM(0xC9, 9);
    ASSERT(func_8008E2DC(a) == 0x22, "8E2DC 0xC9 no match -> jump to slot k");
    /* 0xCC: clear +0x84 bits, 0xA0 */
    PE_StoreU16(a + 0x84u, 0xFFFFu); PE_StoreU16(a + 0x82u, 0x5555u);
    R4_STREAM(0xCC);
    ASSERT(func_8008E2DC(a) == 0xA0 && PE_LoadU16(a + 0x84u) == 0xFFFAu &&
           PE_LoadU16(a + 0x82u) == 0x5555u, "8E2DC 0xCC");
    /* default opcode: clear both, 0xA0 */
    R4_STREAM(0xE0);
    ASSERT(func_8008E2DC(a) == 0xA0 && PE_LoadU16(a + 0x82u) == 0u, "8E2DC default");
    ASSERT(PE_LoadU32(a) == p, "8E2DC never writes the cursor back");
#undef R4_STREAM
    PASS();
}

static void test_PA_r4_all(void)
{
    test_PA_r4_84B78_card_dispatch();
    test_PA_r4_87FA0_voice_slides();
    test_PA_r4_8D844_fade_tick();
    test_PA_r4_8E2DC_peek_event();
}
