/* func_800E051C (camera-facing glow sprite): one POLY_FT4 from the scratchpad
 * block its callers publish, checked field by field against the matched C
 * (src/func_800E051C.c).  The RTPS results (sx, sy, sz) are read back from the
 * scratchpad the function itself writes; the packet must then follow the C:
 *   h = ((size << 5) * *D_800BCFA8) / ((sz << 4) + *D_800BCFA8) >> 1
 *   corners (sx -/+ h, sy -/+ h), uv (u,v) (u+16,v) (u,v+16) (u+16,v+16)
 *   addPrim at ordering[bank] + ((sz >> 2) - 8) words, cursor += 0x28. */
void func_800E051C(void);

static void test_GLOW1_func_800E051C_packet(void)
{
    const pe_addr_t s = 0x1F800000u, mat = 0x80160000u, scale_p = 0x80160100u;
    const pe_addr_t ot = 0x80161000u, pk = 0x80162000u;
    int32_t sx, sy, sz, size = 100, scale = 256, h;   /* h = 25: distinct corners */
    pe_addr_t slot;
    unsigned i;

    TEST("GLOW1_func_800E051C_packet");
    ResetTestState();
    /* matrix: identity rotation (1.0 = 0x1000), translation (0, 0, 1000) */
    for (i = 0; i < 32u; i += 4u) PE_StoreU32(mat + i, 0u);
    PE_StoreU16(mat + 0u, 0x1000u); PE_StoreU16(mat + 8u, 0x1000u); PE_StoreU16(mat + 16u, 0x1000u);
    PE_StoreU32(mat + 28u, 1000u);
    PE_StoreU32(scale_p, (uint32_t)scale);
    /* scratchpad block (Scratch layout in src/func_800E051C.c) */
    for (i = 0; i < 0x40u; i += 4u) PE_StoreU32(s + i, 0u);
    PE_StoreU8(s + 0x18u, 0x11u); PE_StoreU8(s + 0x19u, 0x22u); PE_StoreU8(s + 0x1Au, 0x33u);
    PE_StoreU8(s + 0x1Cu, 0x40u); PE_StoreU8(s + 0x1Du, 0x50u);
    PE_StoreU16(s + 0x1Eu, 0x7713u); PE_StoreU16(s + 0x22u, 0x34u);
    PE_StoreU32(s + 0x2Cu, (uint32_t)size); PE_StoreU32(s + 0x34u, mat);
    /* render buffers: bank 0, ordering[0] = ot, packets[0] = pk; cursor 0 */
    PE_StoreU32(0x8009CDD8u, 0u); PE_StoreU32(0x8009CDDCu, 0u);
    PE_StoreU32(0x800B0E38u, ot); PE_StoreU32(0x800B0E58u, pk);
    PE_StoreU32(0x800BCFA8u, scale_p);
    for (i = 0; i < 0x400u; i += 4u) PE_StoreU32(ot + i, 0x00ABCDEFu);
    for (i = 0; i < 40u; i += 4u) PE_StoreU32(pk + i, 0u);

    func_800E051C();

    sx = (int16_t)PE_LoadU16(s + 0u); sy = (int16_t)PE_LoadU16(s + 2u);
    sz = (int32_t)PE_LoadU32(s + 4u);
    ASSERT(sz == 1000, "RTPS depth: translation z with identity rotation");
    ASSERT(PE_LoadU32(0x8009CDD8u) == 0x28u, "packet cursor advances by 0x28");
    ASSERT(PE_LoadU8(pk + 3u) == 9u && PE_LoadU8(pk + 7u) == 0x2Eu, "setlen 9, code 0x2E");
    ASSERT(PE_LoadU8(pk + 4u) == 0x11u && PE_LoadU8(pk + 5u) == 0x22u && PE_LoadU8(pk + 6u) == 0x33u, "rgb");
    ASSERT(PE_LoadU16(pk + 14u) == 0x7713u && PE_LoadU16(pk + 22u) == 0x34u, "clut, tpage");
    ASSERT(PE_LoadU8(pk + 12u) == 0x40u && PE_LoadU8(pk + 13u) == 0x50u, "uv0 = (u, v)");
    ASSERT(PE_LoadU8(pk + 20u) == 0x50u && PE_LoadU8(pk + 21u) == 0x50u, "uv1 = (u+16, v)");
    ASSERT(PE_LoadU8(pk + 28u) == 0x40u && PE_LoadU8(pk + 29u) == 0x60u, "uv2 = (u, v+16)");
    ASSERT(PE_LoadU8(pk + 36u) == 0x50u && PE_LoadU8(pk + 37u) == 0x60u, "uv3 = (u+16, v+16)");
    h = ((size << 5) * scale) / ((sz << 4) + scale) >> 1;
    ASSERT(h == 25, "size formula: (100<<5)*256 / ((1000<<4)+256) >> 1");
    ASSERT((int16_t)PE_LoadU16(pk + 8u) == (int16_t)(sx - h) && (int16_t)PE_LoadU16(pk + 10u) == (int16_t)(sy - h), "xy0");
    ASSERT((int16_t)PE_LoadU16(pk + 16u) == (int16_t)(sx + h) && (int16_t)PE_LoadU16(pk + 18u) == (int16_t)(sy - h), "xy1");
    ASSERT((int16_t)PE_LoadU16(pk + 24u) == (int16_t)(sx - h) && (int16_t)PE_LoadU16(pk + 26u) == (int16_t)(sy + h), "xy2");
    ASSERT((int16_t)PE_LoadU16(pk + 32u) == (int16_t)(sx + h) && (int16_t)PE_LoadU16(pk + 34u) == (int16_t)(sy + h), "xy3");
    slot = ot + (uint32_t)((sz >> 2) - 8) * 4u;
    ASSERT((PE_LoadU32(slot) & 0xFFFFFFu) == (pk & 0xFFFFFFu), "OT[(sz>>2)-8] links the packet");
    ASSERT((PE_LoadU32(pk) & 0xFFFFFFu) == 0xABCDEFu, "packet tag keeps the old OT link");
    ASSERT(PE_LoadU32(slot - 4u) == 0x00ABCDEFu && PE_LoadU32(slot + 4u) == 0x00ABCDEFu, "neighbour OT slots untouched");
    PASS();
}
