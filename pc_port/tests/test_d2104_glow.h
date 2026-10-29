/* func_800D2104 (glow point sprite, hand port of the EXE GTE wrapper) on
 * analytic inputs: identity camera, TR z=1024, H=256, so the RTPS ratio is
 * exactly 1/4.  Checks the TILE_1 packet (SetTile1 tag, colour scaled by
 * scale/128 with R3000 truncation, SXY2 at +8), otz = (SZ3>>2) - D_800F3374,
 * OT linking with and without the draw-mode word (abr 255 = opaque), the
 * semi-transparency bit, the packet-cursor advance (12, +8 with a draw-mode
 * word), and that an out-of-range otz links nothing. */
void func_800D2104(pe_addr_t pos, pe_addr_t color, int32_t scale, int32_t abr);

static void d2104_setup(pe_addr_t arena, pe_addr_t ot, uint16_t bias)
{
    ResetTestState();
    for (unsigned k = 0; k < 9u; k++) g_pe_gte.rt[k / 3u][k % 3u] = (k % 4u == 0u) ? 0x1000 : 0;
    g_pe_gte.tr[0] = 0; g_pe_gte.tr[1] = 0; g_pe_gte.tr[2] = 1024;
    g_pe_gte.h = 256; g_pe_gte.ofx = 160 << 16; g_pe_gte.ofy = 120 << 16;
    PE_StoreU32(0x8009CDDCu, 1u); PE_StoreU32(0x8009CDD8u, 0x40u);
    PE_StoreU32(0x800B0E5Cu, arena); PE_StoreU32(0x800B0E3Cu, ot);
    PE_StoreU16(0x800F3374u, bias);
}

static void test_D2104_glow_point(void)
{
    const pe_addr_t pos = 0x80140000u, col = 0x80140010u, arena = 0x80150000u, ot = 0x80160000u;
    const uint32_t slot = (1024u >> 2) - 6u;
    TEST("D2104_glow_point");
    for (unsigned semi = 0; semi < 2u; semi++) {
        const pe_addr_t pk = arena + 0x40u, dm = arena + 0x4Cu;
        d2104_setup(arena, ot, 6u);
        PE_StoreU16(pos, 40u); PE_StoreU16(pos + 2u, (uint16_t)-40); PE_StoreU16(pos + 4u, 0u);
        PE_StoreU8(col, 200u); PE_StoreU8(col + 1u, 100u); PE_StoreU8(col + 2u, 3u);
        PE_StoreU32(ot + slot * 4u, 0x00ABCDEFu);
        func_800D2104(pos, col, semi ? 64 : -1, semi ? 1 : 255);
        ASSERT(PE_LoadU8(pk + 3u) == 2u, "TILE_1 length");
        ASSERT(PE_LoadU8(pk + 7u) == (semi ? 0x6Au : 0x68u), "TILE_1 code / semi-transparency bit");
        if (semi) {
            ASSERT(PE_LoadU8(pk + 4u) == 100u && PE_LoadU8(pk + 5u) == 50u && PE_LoadU8(pk + 6u) == 1u,
                   "colour * 64 / 128");
        } else {   /* -200/128 = -1, -100/128 = 0, -3/128 = 0 (truncation) */
            ASSERT(PE_LoadU8(pk + 4u) == 0xFFu && PE_LoadU8(pk + 5u) == 0u && PE_LoadU8(pk + 6u) == 0u,
                   "colour * -1 / 128 truncates toward zero");
        }
        ASSERT(PE_LoadU32(pk + 8u) == ((uint32_t)(uint16_t)170 | ((uint32_t)(uint16_t)110 << 16)), "SXY2");
        ASSERT(g_pe_gte.sz[3] == 1024u, "SZ3");
        ASSERT((PE_LoadU32(pk) & 0xFFFFFFu) == 0xABCDEFu, "sprite links to the old OT head");
        if (semi) {
            ASSERT(PE_LoadU32(0x8009CDD8u) == 0x40u + 20u, "cursor +12 +8");
            ASSERT((PE_LoadU32(dm) & 0xFFFFFFu) == (pk & 0xFFFFFFu), "draw-mode word links to the sprite");
            ASSERT((PE_LoadU32(ot + slot * 4u) & 0xFFFFFFu) == (dm & 0xFFFFFFu), "OT head = draw-mode word");
        } else {
            ASSERT(PE_LoadU32(0x8009CDD8u) == 0x40u + 12u, "cursor +12");
            ASSERT((PE_LoadU32(ot + slot * 4u) & 0xFFFFFFu) == (pk & 0xFFFFFFu), "OT head = sprite");
        }
    }
    /* otz = 256 - 300 < 0 -> unsigned >= 0x1000: nothing linked, space kept. */
    d2104_setup(arena, ot, 300u);
    PE_StoreU16(pos, 0u); PE_StoreU16(pos + 2u, 0u); PE_StoreU16(pos + 4u, 0u);
    for (unsigned i = 0; i < 0x1000u; i += 0x100u) PE_StoreU32(ot + i * 4u, 0x00FEDCBAu);
    func_800D2104(pos, col, 128, 1);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 0x40u + 12u, "cursor +12 even when culled");
    for (unsigned i = 0; i < 0x1000u; i += 0x100u)
        ASSERT(PE_LoadU32(ot + i * 4u) == 0x00FEDCBAu, "culled point must not touch the OT");
    PASS();
}
