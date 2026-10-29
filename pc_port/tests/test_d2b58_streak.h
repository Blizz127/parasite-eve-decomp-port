/* func_800D2B58 (streak, native port of the matched C) on analytic inputs:
 * identity RT with TR z=1024, H=256, a horizontal pair (-64,0,0)/(64,0,0)
 * -> screen (144,120)/(176,120), SZ3>>2 = 256 each.  Checks the LINE_G2 +
 * POLY_GT4 (0x48 bytes) taken up front, line colours (point 1 blue scaled
 * by b0: the retail quirk), the quad corners from ratan2/CF4/DC4, its fixed
 * UVs/clut/tpage, the OT chain (quad -> mode -> line -> old) at the mean
 * depth minus D_800F3374, the opaque variant, the out-of-range return and
 * the NULL c1 read (kuseg 0 = RAM, as on hardware). */
static void d2b58_setup(pe_addr_t cam, pe_addr_t arena, pe_addr_t ot, uint16_t bias)
{
    d0e88_setup(cam, arena, ot, 0x01000000, bias);
    PE_GTE_LoadRT(cam);
    PE_StoreU16(0x8009589Cu + 0x400u * 2u, 0x1000u);   /* quarter-wave entry: 1.0 at 90 deg */
    PE_StoreU16(0x800E2852u, 0x0123u);
}

static void test_D2B58_streak(void)
{
    const pe_addr_t cam = 0x80140000u, v0 = 0x80140040u, v1 = 0x80140048u,
                    c0 = 0x80140050u, c1 = 0x80140054u, arena = 0x80150000u, ot = 0x80160000u;
    const pe_addr_t line = arena, quad = arena + 0x14u, dm = arena + 0x48u;
    int32_t a, cl, sl, sn, cs;
    TEST("D2B58_streak");
    d2b58_setup(cam, arena, ot, 0u);
    a = func_80079FB4(0, 32);
    cl = func_80077CF4(-a) / 512; sl = func_80077DC4(-a) / 512;
    sn = func_80077DC4(a) / 256; cs = func_80077CF4(a) / 256;
    PE_StoreU16(v0, (uint16_t)-64); PE_StoreU16(v0 + 2u, 0u); PE_StoreU16(v0 + 4u, 0u);
    PE_StoreU16(v1, 64u); PE_StoreU16(v1 + 2u, 0u); PE_StoreU16(v1 + 4u, 0u);
    PE_StoreU8(c0, 200u); PE_StoreU8(c0 + 1u, 100u); PE_StoreU8(c0 + 2u, 50u);
    PE_StoreU8(c1, 80u); PE_StoreU8(c1 + 1u, 40u); PE_StoreU8(c1 + 2u, 20u);
    func_800D2B58(v0, v1, c0, c1, 64, 128, 1);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 0x50u, "LINE_G2 + POLY_GT4 (0x48) + DR_MODE (8) taken");
    ASSERT(PE_LoadU8(line + 3u) == 4u && PE_LoadU8(line + 7u) == 0x52u, "semi LINE_G2 header");
    ASSERT(PE_LoadU32(line + 8u) == d1384_xy(144, 120) &&
           PE_LoadU32(line + 0x10u) == d1384_xy(176, 120), "line end points");
    ASSERT(PE_LoadU8(line + 4u) == 100u && PE_LoadU8(line + 5u) == 50u &&
           PE_LoadU8(line + 6u) == 25u, "colour 0 scaled by b0");
    ASSERT(PE_LoadU8(line + 0xCu) == 80u && PE_LoadU8(line + 0xDu) == 40u &&
           PE_LoadU8(line + 0xEu) == 10u, "colour 1: r,g by b1, blue by b0 (retail)");
    ASSERT(PE_LoadU8(quad + 3u) == 0xCu && PE_LoadU8(quad + 7u) == 0x3Eu, "semi POLY_GT4 header");
    ASSERT(PE_LoadU16(quad + 0x1Au) == (uint16_t)(0x0123u | func_80077A64(0, 1, 0, 0)) &&
           PE_LoadU16(quad + 0xEu) == (uint16_t)func_80077AA4(0x20, 0x1D6), "tpage + clut");
    ASSERT((int16_t)PE_LoadU16(quad + 0x08u) == 144 - sn + cl &&
           (int16_t)PE_LoadU16(quad + 0x0Au) == 120 - cs + sl &&
           (int16_t)PE_LoadU16(quad + 0x14u) == 144 - sn - cl &&
           (int16_t)PE_LoadU16(quad + 0x16u) == 120 - cs - sl &&
           (int16_t)PE_LoadU16(quad + 0x20u) == 176 + sn + cl &&
           (int16_t)PE_LoadU16(quad + 0x22u) == 120 + cs + sl &&
           (int16_t)PE_LoadU16(quad + 0x2Cu) == 176 + sn - cl &&
           (int16_t)PE_LoadU16(quad + 0x2Eu) == 120 + cs - sl, "quad corners");
    ASSERT(sn == 16 && sl == 8 && cl == 0 && cs == 0, "horizontal: 16 along, 8 across");
    ASSERT(PE_LoadU16(quad + 0x0Cu) == 0xD090u && PE_LoadU16(quad + 0x18u) == 0xD09Fu &&
           PE_LoadU16(quad + 0x24u) == 0xDF90u && PE_LoadU16(quad + 0x30u) == 0xDF9Fu, "UVs");
    ASSERT(PE_LoadU8(quad + 4u) == 100u && PE_LoadU8(quad + 0x10u) == 100u &&
           PE_LoadU8(quad + 0x1Cu) == 80u && PE_LoadU8(quad + 0x2Au) == 10u, "quad colours");
    ASSERT((PE_LoadU32(ot + 256u * 4u) & 0xFFFFFFu) == (quad & 0xFFFFFFu) &&
           (PE_LoadU32(quad) & 0xFFFFFFu) == (dm & 0xFFFFFFu) &&
           (PE_LoadU32(dm) & 0xFFFFFFu) == (line & 0xFFFFFFu) &&
           (PE_LoadU32(line) & 0xFFFFFFu) == 0xABCDEFu, "OT chain quad -> mode -> line -> old");
    /* abr 255: opaque, no draw mode; c1 NULL reads RAM 0x80000000..2. */
    d2b58_setup(cam, arena, ot, 0u);
    PE_StoreU8(0x80000000u, 6u); PE_StoreU8(0x80000001u, 8u); PE_StoreU8(0x80000002u, 10u);
    func_800D2B58(v0, v1, c0, 0, 128, 64, 255);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 0x48u && PE_LoadU8(line + 7u) == 0x50u &&
           PE_LoadU8(quad + 7u) == 0x3Cu && PE_LoadU16(quad + 0x1Au) == 0x0123u, "opaque streak");
    ASSERT(PE_LoadU8(line + 0xCu) == 3u && PE_LoadU8(line + 0xDu) == 4u &&
           PE_LoadU8(line + 0xEu) == 10u, "NULL c1 reads guest RAM 0x80000000");
    ASSERT((PE_LoadU32(ot + 256u * 4u) & 0xFFFFFFu) == (quad & 0xFFFFFFu) &&
           (PE_LoadU32(quad) & 0xFFFFFFu) == (line & 0xFFFFFFu), "opaque chain quad -> line");
    /* depth 256 - bias 300 wraps: out of range, nothing linked. */
    d2b58_setup(cam, arena, ot, 300u);
    func_800D2B58(v0, v1, c0, c1, 128, 128, 1);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 0x48u && PE_LoadU32(ot + 256u * 4u) == 0x00ABCDEFu,
           "out-of-range depth must not link");
    PASS();
}
