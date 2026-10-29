/* func_800D1384 (gradient trail ribbon, hand port of the EXE GTE wrapper) on
 * analytic inputs: identity camera, TR z=1024, H=256 (RTPS/RTPT ratio 1/4).
 * Checks the trail shift and new head (flags 1), packet space n * 36 taken
 * up front, POLY_G4 vertices (RTPT a[i], b[i], a[i+1]; RTPS b[i+1]), the
 * 128 - i*128/n colour fade, OT linking at (SZ3 >> 2) - D_800F3374, the
 * early return when MAC0 after RTPT is 0 (DQA = DQB = 0), and the
 * n >= 1000 flag reset. */
void func_800D1384(pe_addr_t head0, pe_addr_t head1, uint32_t n, pe_addr_t col0,
                   pe_addr_t col1, int32_t scale, pe_addr_t trail, int32_t abr);

static void d1384_put(pe_addr_t a, int16_t x, int16_t y, int16_t z, uint16_t flag)
{
    PE_StoreU16(a, (uint16_t)x); PE_StoreU16(a + 2u, (uint16_t)y);
    PE_StoreU16(a + 4u, (uint16_t)z); PE_StoreU16(a + 6u, flag);
}

static uint32_t d1384_xy(int x, int y) { return (uint32_t)(uint16_t)x | ((uint32_t)(uint16_t)y << 16); }

static void d1384_setup(pe_addr_t cam, pe_addr_t trail, pe_addr_t arena, pe_addr_t ot, int32_t dqb)
{
    ResetTestState();
    for (unsigned k = 0; k < 9u; k++) PE_StoreU16(cam + k * 2u, (k % 4u == 0u) ? 0x1000u : 0u);
    PE_StoreU32(cam + 20u, 0u); PE_StoreU32(cam + 24u, 0u); PE_StoreU32(cam + 28u, 1024u);
    PE_StoreU32(0x800BCFA4u, cam);
    g_pe_gte.h = 256; g_pe_gte.ofx = 160 << 16; g_pe_gte.ofy = 120 << 16;
    g_pe_gte.dqa = 0; g_pe_gte.dqb = dqb;
    PE_StoreU32(0x8009CDDCu, 0u); PE_StoreU32(0x8009CDD8u, 0u);
    PE_StoreU32(0x800B0E58u, arena); PE_StoreU32(0x800B0E38u, ot);
    PE_StoreU16(0x800F3374u, 0u);
    d1384_put(trail + 0u, 0, -40, 0, 1u);  d1384_put(trail + 8u, 40, -40, 0, 1u);
    d1384_put(trail + 16u, 0, -80, 0, 1u); d1384_put(trail + 24u, 40, -80, 0, 1u);
    d1384_put(trail + 32u, 9, 9, 9, 0u);   d1384_put(trail + 40u, 9, 9, 9, 0u);
    PE_StoreU32(ot + 256u * 4u, 0x00ABCDEFu);
}

static void test_D1384_trail(void)
{
    const pe_addr_t cam = 0x80140000u, h0 = 0x80140040u, h1 = 0x80140048u, c0 = 0x80140050u,
                    c1 = 0x80140054u, trail = 0x80140100u, arena = 0x80150000u, ot = 0x80160000u;
    const pe_addr_t p0 = arena, p1 = arena + 36u;
    TEST("D1384_trail");
    d1384_setup(cam, trail, arena, ot, 0x01000000);
    d1384_put(h0, 0, 0, 0, 0u); d1384_put(h1, 40, 0, 0, 0u);
    PE_StoreU8(c0, 200u); PE_StoreU8(c0 + 1u, 100u); PE_StoreU8(c0 + 2u, 50u);
    PE_StoreU8(c1, 10u); PE_StoreU8(c1 + 1u, 20u); PE_StoreU8(c1 + 2u, 30u);
    func_800D1384(h0, h1, 2u, c0, c1, 128, trail, 255);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 72u, "packet space n * 36");
    ASSERT(PE_LoadU16(trail + 6u) == 1u && PE_LoadU16(trail + 14u) == 1u, "new head flags");
    ASSERT((int16_t)PE_LoadU16(trail + 18u) == -40 && (int16_t)PE_LoadU16(trail + 34u) == -80, "trail shifted");
    ASSERT(PE_LoadU32(p0 + 8u) == d1384_xy(160, 120) && PE_LoadU32(p0 + 16u) == d1384_xy(170, 120) &&
           PE_LoadU32(p0 + 24u) == d1384_xy(160, 110) && PE_LoadU32(p0 + 32u) == d1384_xy(170, 110),
           "segment 0 vertices");
    ASSERT(PE_LoadU32(p1 + 8u) == d1384_xy(160, 110) && PE_LoadU32(p1 + 32u) == d1384_xy(170, 100),
           "segment 1 vertices");
    ASSERT(PE_LoadU8(p0 + 3u) == 8u && PE_LoadU8(p0 + 7u) == 0x38u, "POLY_G4 tag");
    ASSERT(PE_LoadU8(p0 + 4u) == 200u && PE_LoadU8(p0 + 14u) == 30u, "segment 0 near colours (t = 128)");
    ASSERT(PE_LoadU8(p0 + 20u) == 100u && PE_LoadU8(p0 + 30u) == 15u, "segment 0 far colours (t = 64)");
    ASSERT(PE_LoadU8(p1 + 4u) == 100u && PE_LoadU8(p1 + 20u) == 0u, "segment 1 fades to 0");
    ASSERT((PE_LoadU32(ot + 256u * 4u) & 0xFFFFFFu) == (p1 & 0xFFFFFFu) &&
           (PE_LoadU32(p1) & 0xFFFFFFu) == (p0 & 0xFFFFFFu) &&
           (PE_LoadU32(p0) & 0xFFFFFFu) == 0xABCDEFu, "OT chain slot 256 -> seg1 -> seg0 -> old");
    /* MAC0 == 0 after RTPT ends the call: nothing linked, space still taken. */
    d1384_setup(cam, trail, arena, ot, 0);
    func_800D1384(h0, h1, 2u, c0, c1, 128, trail, 255);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 72u, "space taken even when nothing is drawn");
    ASSERT(PE_LoadU32(ot + 256u * 4u) == 0x00ABCDEFu, "MAC0 == 0 must not link");
    /* n >= 1000: clear the flags of the first n - 999 entries only. */
    d1384_setup(cam, trail, arena, ot, 0x01000000);
    func_800D1384(0, 0, 1001u, 0, 0, 128, trail, 255);
    ASSERT(PE_LoadU16(trail + 6u) == 0u && PE_LoadU16(trail + 14u) == 0u &&
           PE_LoadU16(trail + 22u) == 0u && PE_LoadU16(trail + 30u) == 0u, "flags cleared");
    ASSERT((int16_t)PE_LoadU16(trail + 2u) == -40 && PE_LoadU32(0x8009CDD8u) == 0u, "reset draws nothing");
    PASS();
}
