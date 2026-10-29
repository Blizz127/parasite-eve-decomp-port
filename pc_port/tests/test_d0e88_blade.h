/* func_800D0E88 (shaded blade, native port of the matched C) on analytic
 * inputs: identity camera, TR z=1024, H=256, zero angles.  Checks the
 * POLY_G3 taken up front (28 bytes) with its header, the fan vertices
 * (0,0,0), (-w/2,0,len), (w/2,0,len) on screen, colours scaled by
 * bright/128 with D_800C22A0 standing in for a null colour, the draw-mode
 * word plus semi-transparency bit for abr != 255 and the OT chain at
 * (SZ3 >> 2) - D_800F3374, and the two early returns (MAC0 == 0 after the
 * RTPT; depth out of range). */
static void INIT1_SeedTrig(void);

static void d0e88_setup(pe_addr_t cam, pe_addr_t arena, pe_addr_t ot, int32_t dqb, uint16_t bias)
{
    ResetTestState();
    INIT1_SeedTrig();                 /* rsin/rcos tables in guest RAM */
    for (unsigned k = 0; k < 9u; k++) PE_StoreU16(cam + k * 2u, (k % 4u == 0u) ? 0x1000u : 0u);
    PE_StoreU32(cam + 20u, 0u); PE_StoreU32(cam + 24u, 0u); PE_StoreU32(cam + 28u, 1024u);
    PE_StoreU32(0x800BCFA4u, cam);
    g_pe_gte.h = 256; g_pe_gte.ofx = 160 << 16; g_pe_gte.ofy = 120 << 16;
    g_pe_gte.dqa = 0; g_pe_gte.dqb = dqb;
    PE_StoreU32(0x8009CDDCu, 0u); PE_StoreU32(0x8009CDD8u, 0u);
    PE_StoreU32(0x800B0E58u, arena); PE_StoreU32(0x800B0E38u, ot);
    PE_StoreU16(0x800F3374u, bias);
    PE_StoreU8(0x800C22A0u, 40u); PE_StoreU8(0x800C22A1u, 60u); PE_StoreU8(0x800C22A2u, 80u);
    PE_StoreU32(ot + 256u * 4u, 0x00ABCDEFu);
}

static void test_D0E88_blade(void)
{
    const pe_addr_t cam = 0x80140000u, pos = 0x80140040u, ang = 0x80140048u, col = 0x80140050u,
                    arena = 0x80150000u, ot = 0x80160000u;
    const pe_addr_t p = arena, dm = arena + 28u;
    TEST("D0E88_blade");
    d0e88_setup(cam, arena, ot, 0x01000000, 0u);
    for (unsigned k = 0; k < 8u; k += 2u) { PE_StoreU16(pos + k, 0u); PE_StoreU16(ang + k, 0u); }
    PE_StoreU8(col, 200u); PE_StoreU8(col + 1u, 100u); PE_StoreU8(col + 2u, 50u);
    func_800D0E88(pos, ang, 0, 80, col, 0, col, 64, 1);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 36u, "POLY_G3 (28) + DR_MODE (8) taken");
    ASSERT(PE_LoadU8(p + 3u) == 6u && (PE_LoadU8(p + 7u) & ~2u) == 0x30u, "POLY_G3 header");
    ASSERT(PE_LoadU32(p + 8u) == d1384_xy(160, 120) && PE_LoadU32(p + 16u) == d1384_xy(150, 120) &&
           PE_LoadU32(p + 24u) == d1384_xy(170, 120), "fan vertices");
    ASSERT(PE_LoadU8(p + 4u) == 100u && PE_LoadU8(p + 5u) == 50u && PE_LoadU8(p + 6u) == 25u,
           "colour 0 scaled by 64/128");
    ASSERT(PE_LoadU8(p + 12u) == 20u && PE_LoadU8(p + 13u) == 30u && PE_LoadU8(p + 14u) == 40u,
           "null colour 1 uses D_800C22A0");
    ASSERT(PE_LoadU8(p + 20u) == 100u, "colour 2");
    ASSERT((PE_LoadU8(p + 7u) & 2u) && PE_LoadU8(dm + 3u) == 1u, "semi-transparent + draw mode");
    ASSERT((PE_LoadU32(ot + 256u * 4u) & 0xFFFFFFu) == (dm & 0xFFFFFFu) &&
           (PE_LoadU32(dm) & 0xFFFFFFu) == (p & 0xFFFFFFu) &&
           (PE_LoadU32(p) & 0xFFFFFFu) == 0xABCDEFu, "OT chain slot 256 -> mode -> blade -> old");
    /* abr 255: opaque, no draw mode. */
    d0e88_setup(cam, arena, ot, 0x01000000, 0u);
    func_800D0E88(pos, ang, 0, 80, col, col, col, 128, 255);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 28u && !(PE_LoadU8(p + 7u) & 2u) &&
           (PE_LoadU32(ot + 256u * 4u) & 0xFFFFFFu) == (p & 0xFFFFFFu), "opaque blade linked");
    /* MAC0 == 0 after the RTPT (DQA = DQB = 0): nothing linked. */
    d0e88_setup(cam, arena, ot, 0, 0u);
    func_800D0E88(pos, ang, 0, 80, col, col, col, 128, 255);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 28u && PE_LoadU32(ot + 256u * 4u) == 0x00ABCDEFu,
           "MAC0 == 0 must not link");
    /* depth 256 - bias 300 wraps: out of range, nothing linked. */
    d0e88_setup(cam, arena, ot, 0x01000000, 300u);
    func_800D0E88(pos, ang, 0, 80, col, col, col, 128, 1);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 28u && PE_LoadU32(ot + 256u * 4u) == 0x00ABCDEFu,
           "out-of-range depth must not link or take draw-mode space");
    PASS();
}
