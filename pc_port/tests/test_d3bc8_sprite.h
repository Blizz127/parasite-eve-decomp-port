/* func_800D3BC8 (camera-facing POLY_FT4 sprite, native port) on analytic
 * inputs: identity RT with TR z=1024, H=256, so (0,0,0) -> (160,120) and
 * (64,0,0) -> (176,120), SZ3>>2 = 256.  Checks the 0x28-byte prim taken up
 * front, header/colour (scaled and NULL), the 16x16 cell UVs (plain and the
 * D_800F336E fold), half extents w*sx/8192, tpage/clut, the OT link at
 * SZ3>>2 - D_800F3374, the lerp variant (depth pinned to 14, centre pulled
 * towards (160,120)) and the out-of-range return. */
static void d3bc8_setup(pe_addr_t cam, pe_addr_t arena, pe_addr_t ot, uint16_t bias, uint16_t fold)
{
    d0e88_setup(cam, arena, ot, 0x01000000, bias);
    PE_StoreU16(0x800F336Eu, fold);
    PE_StoreU16(0x800F3370u, 0x0123u);
    PE_StoreU16(0x800F3376u, 32u);
    PE_StoreU16(0x800F3378u, 16u);
    PE_StoreU32(ot + 14u * 4u, 0x00ABCDEFu);
}

static void test_D3BC8_sprite(void)
{
    const pe_addr_t cam = 0x80140000u, pos = 0x80140040u, col = 0x80140050u,
                    arena = 0x80150000u, ot = 0x80160000u;
    const pe_addr_t p = arena;
    TEST("D3BC8_sprite");
    d3bc8_setup(cam, arena, ot, 0u, 0u);
    PE_StoreU16(pos, 0u); PE_StoreU16(pos + 2u, 0u); PE_StoreU16(pos + 4u, 0u);
    PE_StoreU8(col, 200u); PE_StoreU8(col + 1u, 100u); PE_StoreU8(col + 2u, 50u);
    func_800D3BC8(pos, 4096, 4096, 0x23, 0x7777, 1, 64, col, 0);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 0x28u, "POLY_FT4 (0x28) taken");
    ASSERT(PE_LoadU8(p + 3u) == 9u && PE_LoadU8(p + 7u) == 0x2Eu, "semi POLY_FT4 header");
    ASSERT(PE_LoadU8(p + 4u) == 100u && PE_LoadU8(p + 5u) == 50u && PE_LoadU8(p + 6u) == 25u,
           "colour scaled by 64/128");
    ASSERT(PE_LoadU16(p + 0x16u) == (uint16_t)(0x0123u | func_80077A64(0, 1, 0, 0)) &&
           PE_LoadU16(p + 0x0Eu) == 0x7777u, "tpage | GetTPage(abr) + clut");
    ASSERT(PE_LoadU8(p + 0x0Cu) == 48u && PE_LoadU8(p + 0x0Du) == 32u &&
           PE_LoadU8(p + 0x14u) == 79u && PE_LoadU8(p + 0x15u) == 32u &&
           PE_LoadU8(p + 0x1Cu) == 48u && PE_LoadU8(p + 0x1Du) == 47u &&
           PE_LoadU8(p + 0x24u) == 79u && PE_LoadU8(p + 0x25u) == 47u, "cell 0x23 UVs, 32x16");
    ASSERT(PE_LoadU32(p + 0x08u) == d1384_xy(144, 112) && PE_LoadU32(p + 0x10u) == d1384_xy(176, 112) &&
           PE_LoadU32(p + 0x18u) == d1384_xy(144, 128) && PE_LoadU32(p + 0x20u) == d1384_xy(176, 128),
           "corners: half extents 16 x 8 around (160,120)");
    ASSERT((PE_LoadU32(ot + 256u * 4u) & 0xFFFFFFu) == (p & 0xFFFFFFu) &&
           (PE_LoadU32(p) & 0xFFFFFFu) == 0xABCDEFu, "linked at SZ3>>2");
    /* fold: cell 0x2B -> u (11-8)<<4, v 32 + 2*16; opaque; NULL colour; lerp. */
    d3bc8_setup(cam, arena, ot, 0u, 1u);
    PE_StoreU16(pos, 64u);
    func_800D3BC8(pos, 8192, 4096, 0x2B, 0x1111, 255, 77, 0u, 0x400);
    ASSERT(PE_LoadU8(p + 7u) == 0x2Cu && PE_LoadU16(p + 0x16u) == 0x0123u, "opaque header + tpage");
    ASSERT(PE_LoadU8(p + 4u) == 77u && PE_LoadU8(p + 5u) == 77u && PE_LoadU8(p + 6u) == 77u,
           "NULL colour = brightness");
    ASSERT(PE_LoadU8(p + 0x0Cu) == 48u && PE_LoadU8(p + 0x0Du) == 64u, "folded cell UV");
    ASSERT(PE_LoadU32(p + 0x08u) == d1384_xy(168 - 32, 112) &&
           PE_LoadU32(p + 0x20u) == d1384_xy(168 + 32, 128), "lerp half-way to (160,120), sx 2.0");
    ASSERT((PE_LoadU32(ot + 14u * 4u) & 0xFFFFFFu) == (p & 0xFFFFFFu) &&
           (PE_LoadU32(ot + 256u * 4u) & 0xFFFFFFu) == 0xABCDEFu, "lerp pins depth 14");
    /* depth 256 - bias 300 wraps: out of range, prim still taken, nothing linked. */
    d3bc8_setup(cam, arena, ot, 300u, 0u);
    PE_StoreU16(pos, 0u);
    func_800D3BC8(pos, 4096, 4096, 0, 0, 255, 128, 0u, 0);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 0x28u && PE_LoadU32(ot + 256u * 4u) == 0x00ABCDEFu,
           "out-of-range depth must not link");
    PASS();
}
