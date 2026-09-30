/*
 * port_absent lane (2026-09-23) — tests for the handhi group: hand adapters
 * in pc_port/game/decomp_hand/ (seq_ops_hi_port.c, card_tag_hi_port.c,
 * rest_hi_port.c, absent_lo_batch3_port.c) that were written but never
 * tested.  Expectations are computed from the matched leaf
 * src/func_XXXXXXXX.c named in each assertion, never from the port.
 *
 * Guest scratch: 0x80154000..0x80157FFF only.
 */

#define PAH_REC  0x80154000u   /* voice/track record (0x200 bytes)        */
#define PAH_DATA 0x80154800u   /* sequence operand bytes the cursor reads */
#define PAH_MB   0x80155000u   /* D_8009D2C8 master block                 */
#define PAH_BUF  0x80156000u   /* texture / text source bytes            */

/* Zeroed record whose cursor (word 0) points at `bytes`. */
static pe_addr_t pah_seq(const unsigned char *bytes, unsigned n)
{
    PE_Fill(PAH_REC, 0x200u, 0u);
    for (unsigned i = 0; i < n; i++)
        PE_StoreU8(PAH_DATA + i, bytes[i]);
    PE_StoreU32(PAH_REC, PAH_DATA);
    return PAH_REC;
}

static void pah_master(void)
{
    PE_Fill(PAH_MB, 0x80u, 0u);
    PE_StoreU32(0x8009D2C8u, PAH_MB);
}

/* ── flag / constant stores and single-byte readers ───────────────── */
static void test_PA_handhi_seq_simple(void)
{
    pe_addr_t a;
    TEST("PA_handhi_seq_simple");
    ResetTestState();

    a = pah_seq(NULL, 0);
    PE_StoreU16(a + 0x82u, 0xFFFFu);
    func_8008FCB4(a);                       /* src/func_8008FCB4.c: +0x82 = 0 */
    ASSERT(PE_LoadU16(a + 0x82u) == 0u, "8008FCB4: +0x82 cleared");
    func_800904A0(a);                       /* src/func_800904A0.c: +0x84 = 1 */
    ASSERT(PE_LoadU16(a + 0x84u) == 1u, "800904A0: +0x84 = 1");

    PE_StoreU32(a + 0x38u, 0xFFFFFFFFu);
    func_80090A0C(a);                       /* &= ~0x8 */
    ASSERT(PE_LoadU32(a + 0x38u) == 0xFFFFFFF7u, "80090A0C: clears bit 3 only");
    func_80090C4C(a);                       /* &= ~0x10 */
    ASSERT(PE_LoadU32(a + 0x38u) == 0xFFFFFFE7u, "80090C4C: clears bit 4 only");
    func_80090C74(a);                       /* &= ~0x20 */
    ASSERT(PE_LoadU32(a + 0x38u) == 0xFFFFFFC7u, "80090C74: clears bit 5 only");
    PE_StoreU32(a + 0x38u, 0u);
    func_80090C38(a);
    ASSERT(PE_LoadU32(a + 0x38u) == 0x10u, "80090C38: sets 0x10");
    func_80090C60(a);
    ASSERT(PE_LoadU32(a + 0x38u) == 0x30u, "80090C60: sets 0x20");
    func_80090F54(a);
    ASSERT(PE_LoadU32(a + 0x38u) == 0x100030u, "80090F54: sets 0x100000");

    /* src/func_8008F868.c: (v + 1) & 0xF. */
    PE_StoreU16(a + 0x7Cu, 15u);
    func_8008F868(a);
    ASSERT(PE_LoadU16(a + 0x7Cu) == 0u, "8008F868: 15 wraps to 0");
    PE_StoreU16(a + 0x7Cu, 0xFFF7u);
    func_8008F868(a);
    ASSERT(PE_LoadU16(a + 0x7Cu) == 8u, "8008F868: result masked to 4 bits");

    { const unsigned char b[] = {0xAB}; a = pah_seq(b, 1); }
    func_8008F84C(a);                       /* +0x7C = byte */
    ASSERT(PE_LoadU16(a + 0x7Cu) == 0xABu && PE_LoadU32(a) == PAH_DATA + 1u,
           "8008F84C: zero-extended byte, cursor + 1");
    { const unsigned char b[] = {0x81}; a = pah_seq(b, 1); }
    func_8008FFC0(a);                       /* +0xA6 = byte << 8 */
    ASSERT(PE_LoadU16(a + 0xA6u) == 0x8100u && PE_LoadU32(a) == PAH_DATA + 1u,
           "8008FFC0: byte << 8");
    { const unsigned char b[] = {0x81}; a = pah_seq(b, 1); }
    func_800900E4(a);                       /* +0xB4 = byte << 7 */
    ASSERT(PE_LoadU16(a + 0xB4u) == 0x4080u, "800900E4: byte << 7");
    { const unsigned char b[] = {0x80}; a = pah_seq(b, 1); }
    func_8008FCBC(a);                       /* +0xE0 = (signed char) */
    ASSERT(PE_LoadU16(a + 0xE0u) == 0xFF80u, "8008FCBC: sign-extended");

    /* src/func_80090948.c: +0xD2 = 0, +0x58 = +0x56 = +0xD0 = byte. */
    { const unsigned char b[] = {0x9C}; a = pah_seq(b, 1); }
    PE_StoreU16(a + 0xD2u, 5u);
    func_80090948(a);
    ASSERT(PE_LoadU16(a + 0xD2u) == 0u && PE_LoadU16(a + 0x58u) == 0x9Cu &&
           PE_LoadU16(a + 0x56u) == 0x9Cu && PE_LoadU16(a + 0xD0u) == 0x9Cu,
           "80090948: broadcast byte, +0xD2 cleared");

    /* src/func_8008F4E8.c: +0xF4 |= 3, +0x6C = c << 8. */
    { const unsigned char b[] = {0x7F}; a = pah_seq(b, 1); }
    PE_StoreU32(a + 0xF4u, 0x100u);
    func_8008F4E8(a);
    ASSERT(PE_LoadU32(a + 0xF4u) == 0x103u && PE_LoadU16(a + 0x6Cu) == 0x7F00u,
           "8008F4E8: flags | 3, byte << 8");

    /* src/func_8008FCE4.c: u16 += (signed char). */
    { const unsigned char b[] = {0xFD}; a = pah_seq(b, 1); }
    PE_StoreU16(a + 0xE0u, 5u);
    func_8008FCE4(a);
    ASSERT(PE_LoadU16(a + 0xE0u) == 2u, "8008FCE4: 5 + (-3)");
    { const unsigned char b[] = {0xFE}; a = pah_seq(b, 1); }
    PE_StoreU16(a + 0xE0u, 1u);
    func_8008FCE4(a);
    ASSERT(PE_LoadU16(a + 0xE0u) == 0xFFFFu, "8008FCE4: 1 + (-2) wraps u16");
    PASS();
}

/* ── byte parameters with +0xF4 dirty flags ───────────────────────── */
static void test_PA_handhi_seq_params(void)
{
    static const struct { void (*fn)(pe_addr_t); uint32_t flags, off, width; } k[] = {
        {func_80090574, 0x900u,  0x10Eu, 2u},   /* src/func_80090574.c */
        {func_800905EC, 0x2200u, 0x114u, 2u},   /* src/func_800905EC.c */
        {func_80090614, 0x4400u, 0x116u, 2u},   /* src/func_80090614.c */
        {func_8009063C, 0x100u,  0x100u, 4u},   /* src/func_8009063C.c */
        {func_80090664, 0x200u,  0x104u, 4u},   /* src/func_80090664.c */
        {func_8009068C, 0x400u,  0x108u, 4u},   /* src/func_8009068C.c */
    };
    pe_addr_t a;
    TEST("PA_handhi_seq_params");
    ResetTestState();

    for (unsigned i = 0; i < sizeof(k) / sizeof(k[0]); i++) {
        const unsigned char b[] = {0xC3};
        a = pah_seq(b, 1);
        PE_StoreU32(a + 0xF4u, 1u);
        PE_StoreU32(a + k[i].off, 0xFFFFFFFFu);
        k[i].fn(a);
        ASSERT(PE_LoadU32(a + 0xF4u) == (1u | k[i].flags), "param: +0xF4 |= flags");
        ASSERT(PE_LoadU32(a) == PAH_DATA + 1u, "param: cursor + 1");
        if (k[i].width == 2u)
            ASSERT(PE_LoadU32(a + k[i].off) == 0xFFFF00C3u, "param: u16 store only");
        else
            ASSERT(PE_LoadU32(a + k[i].off) == 0xC3u, "param: u32 zero-extended store");
    }

    /* src/func_800906B4.c: +0x38 |= 0x200 then as 80090614. */
    { const unsigned char b[] = {0x44}; a = pah_seq(b, 1); }
    PE_StoreU32(a + 0x38u, 1u);
    func_800906B4(a);
    ASSERT(PE_LoadU32(a + 0x38u) == 0x201u && PE_LoadU32(a + 0xF4u) == 0x4400u &&
           PE_LoadU16(a + 0x116u) == 0x44u && PE_LoadU32(a) == PAH_DATA + 1u,
           "800906B4: +0x38 bit 9, +0xF4 0x4400, +0x116 byte");
    PASS();
}

/* ── D_8009D2C8 master-block opcodes ──────────────────────────────── */
static void test_PA_handhi_seq_master(void)
{
    pe_addr_t a;
    TEST("PA_handhi_seq_master");
    ResetTestState();
    pah_master();

    /* src/func_8008F224.c: +0x20 = b0 << 16, +0x52 = 0, +0x20 |= b1 << 24. */
    PE_StoreU16(PAH_MB + 0x52u, 7u);
    { const unsigned char b[] = {0x12, 0x9A}; a = pah_seq(b, 2); }
    func_8008F224(a);
    ASSERT(PE_LoadU32(PAH_MB + 0x20u) == 0x9A120000u && PE_LoadU16(PAH_MB + 0x52u) == 0u &&
           PE_LoadU32(a) == PAH_DATA + 2u, "8008F224: master volume word");

    /* src/func_8008F274.c: steps 0 -> 0x100; old = +0x20 & ~0xFFFF;
     * +0x24 = (v - old) / steps (signed int division). */
    PE_StoreU32(PAH_MB + 0x20u, 0x1234ABCDu);
    { const unsigned char b[] = {0, 0x10, 0x20}; a = pah_seq(b, 3); }
    func_8008F274(a);
    ASSERT(PE_LoadU16(PAH_MB + 0x52u) == 0x100u && PE_LoadU32(PAH_MB + 0x20u) == 0x12340000u &&
           PE_LoadU32(PAH_MB + 0x24u) == (0x20100000u - 0x12340000u) / 0x100u &&
           PE_LoadU32(a) == PAH_DATA + 3u, "8008F274: fade step, 0 steps -> 0x100");
    { const unsigned char b[] = {3, 0x00, 0xF0}; a = pah_seq(b, 3); }
    func_8008F274(a);
    /* (0xF0000000 - 0x12340000) as int = -0x22340000, / 3 truncates toward 0. */
    ASSERT(PE_LoadU16(PAH_MB + 0x52u) == 3u &&
           (int32_t)PE_LoadU32(PAH_MB + 0x24u) == -(0x22340000 / 3),
           "8008F274: signed step truncates toward zero");

    /* src/func_8008F37C.c: the same over +0x58 / +0x40 / +0x44. */
    PE_StoreU32(PAH_MB + 0x40u, 0x0001FFFFu);
    { const unsigned char b[] = {4, 0x00, 0x01}; a = pah_seq(b, 3); }
    func_8008F37C(a);
    ASSERT(PE_LoadU16(PAH_MB + 0x58u) == 4u && PE_LoadU32(PAH_MB + 0x40u) == 0x00010000u &&
           PE_LoadU32(PAH_MB + 0x44u) == (0x01000000u - 0x00010000u) / 4u,
           "8008F37C: second fade channel");

    /* src/func_8008F328.c: +0x40 = b0 << 16 | b1 << 24, +0x58 = 0,
     * D_8009D2C4 |= 0x80. */
    PE_StoreU32(0x8009D2C4u, 1u);
    { const unsigned char b[] = {0x34, 0x56}; a = pah_seq(b, 2); }
    func_8008F328(a);
    ASSERT(PE_LoadU32(PAH_MB + 0x40u) == 0x56340000u && PE_LoadU16(PAH_MB + 0x58u) == 0u &&
           PE_LoadU32(0x8009D2C4u) == 0x81u, "8008F328: immediate level + flag");

    /* src/func_80090A64.c: +0x64 = b0, then |= b1 << 8. */
    PE_StoreU16(PAH_MB + 0x64u, 0xFFFFu);
    { const unsigned char b[] = {0x34, 0x12}; a = pah_seq(b, 2); }
    func_80090A64(a);
    ASSERT(PE_LoadU16(PAH_MB + 0x64u) == 0x1234u && PE_LoadU32(a) == PAH_DATA + 2u,
           "80090A64: little-endian u16 (first store overwrites)");
    PASS();
}

/* ── per-voice fades and envelopes ────────────────────────────────── */
static void test_PA_handhi_seq_fades(void)
{
    pe_addr_t a;
    TEST("PA_handhi_seq_fades");
    ResetTestState();

    /* src/func_8008F59C.c */
    { const unsigned char b[] = {0x81}; a = pah_seq(b, 1); }
    PE_StoreU32(a + 0x38u, 8u);
    func_8008F59C(a);
    ASSERT(PE_LoadU16(a + 0x6Au) == 0x4080u && PE_LoadU32(a + 0xF4u) == 0u,
           "8008F59C: bit 3 -> +0x6A = byte << 7 only");
    { const unsigned char b[] = {0xFF}; a = pah_seq(b, 1); }
    PE_StoreU16(a + 0x72u, 5u);
    func_8008F59C(a);
    ASSERT(PE_LoadU16(a + 0x72u) == 0u && PE_LoadU32(a + 0xF4u) == 3u &&
           PE_LoadU32(a + 0x44u) == 0xFF800000u, "8008F59C: pan (s8)-1 << 23");

    /* src/func_8008F608.c */
    { const unsigned char b[] = {0, 0x40}; a = pah_seq(b, 2); }
    PE_StoreU32(a + 0x44u, 0x0001FFFFu);
    func_8008F608(a);
    ASSERT(PE_LoadU16(a + 0x72u) == 0x100u && PE_LoadU32(a + 0x44u) == 0x00010000u &&
           PE_LoadU32(a + 0x48u) == (0x20000000u - 0x10000u) / 0x100u, "8008F608: pan fade");
    { const unsigned char b[] = {3, 0x80}; a = pah_seq(b, 2); }
    PE_StoreU32(a + 0x44u, 0x00010000u);
    func_8008F608(a);
    ASSERT((int32_t)PE_LoadU32(a + 0x48u) == -(0x40010000 / 3), "8008F608: (s8)0x80 target, trunc");

    /* src/func_8008F6B0.c */
    { const unsigned char b[] = {0x12}; a = pah_seq(b, 1); }
    PE_StoreU16(a + 0x74u, 9u);
    PE_StoreU32(a + 0x38u, 0x100u);
    func_8008F6B0(a);
    ASSERT(PE_LoadU16(a + 0x74u) == 0u && PE_LoadU16(a + 0xD8u) == 0x1200u &&
           PE_LoadU32(a + 0xF4u) == 3u, "8008F6B0: bit 8 sets +0xF4 |= 3");
    { const unsigned char b[] = {0x12}; a = pah_seq(b, 1); }
    func_8008F6B0(a);
    ASSERT(PE_LoadU32(a + 0xF4u) == 0u, "8008F6B0: bit 8 clear leaves +0xF4");

    /* src/func_8008F6F4.c: (short)old — 0x8000 counts as -0x8000. */
    { const unsigned char b[] = {2, 0x10}; a = pah_seq(b, 2); }
    PE_StoreU16(a + 0xD8u, 0x80FFu);
    func_8008F6F4(a);
    ASSERT(PE_LoadU16(a + 0x74u) == 2u && PE_LoadU16(a + 0xD8u) == 0x8000u &&
           PE_LoadU16(a + 0xDAu) == (uint16_t)((0x1000 + 0x8000) / 2), "8008F6F4: signed level view");

    /* src/func_8008F784.c */
    { const unsigned char b[] = {0xC0}; a = pah_seq(b, 1); }
    PE_StoreU16(a + 0x78u, 4u);
    PE_StoreU16(a + 0x76u, 0xFFFFu);
    func_8008F784(a);
    ASSERT(PE_LoadU16(a + 0x78u) == 0u && PE_LoadU32(a + 0xF4u) == 3u &&
           PE_LoadU16(a + 0x76u) == 0u, "8008F784: (0xC0 + 0x40) & 0xFF wraps to 0");
    { const unsigned char b[] = {0x10}; a = pah_seq(b, 1); }
    func_8008F784(a);
    ASSERT(PE_LoadU16(a + 0x76u) == 0x5000u, "8008F784: (0x10 + 0x40) << 8");

    /* src/func_8008F7BC.c: unsigned old — (0 - 0x8000) / 0x100 = -0x80. */
    { const unsigned char b[] = {0, 0xC0}; a = pah_seq(b, 2); }
    PE_StoreU16(a + 0x76u, 0x80FFu);
    func_8008F7BC(a);
    ASSERT(PE_LoadU16(a + 0x78u) == 0x100u && PE_LoadU16(a + 0x76u) == 0x8000u &&
           PE_LoadU16(a + 0xDCu) == 0xFF80u, "8008F7BC: unsigned level view");

    /* src/func_8008FB00.c */
    { const unsigned char b[] = {0xFE, 0xFF}; a = pah_seq(b, 2); }
    PE_StoreU16(a + 0xE2u, 3u);
    func_8008FB00(a);
    ASSERT(PE_LoadU32(a) == PAH_DATA + 2u && PE_LoadU32(a + 0x18u) == PAH_DATA &&
           PE_LoadU16(a + 0x5Au) == 0xFFu && PE_LoadU16(a + 0xE2u) == 0u &&
           PE_LoadU32(a + 0x38u) == 0x1000u, "8008FB00: s16 table offset past operand");

    /* src/func_8008FC28.c */
    { const unsigned char b[] = {0, 0x80}; a = pah_seq(b, 2); }
    func_8008FC28(a);
    ASSERT(PE_LoadU16(a + 0x7Eu) == 0x100u && PE_LoadU16(a + 0xE4u) == 0xFF80u &&
           PE_LoadU32(a) == PAH_DATA + 2u, "8008FC28: 0 -> 0x100, signed second byte");
    { const unsigned char b[] = {5, 0x7F}; a = pah_seq(b, 2); }
    func_8008FC28(a);
    ASSERT(PE_LoadU16(a + 0x7Eu) == 5u && PE_LoadU16(a + 0xE4u) == 0x7Fu, "8008FC28: plain");

    /* src/func_8008FE10.c: int +0x30 width. */
    { const unsigned char b[] = {0x85}; a = pah_seq(b, 1); }
    PE_StoreU32(a + 0x30u, 100u);
    func_8008FE10(a);
    ASSERT(PE_LoadU16(a + 0x94u) == 0x8500u && PE_LoadU16(a + 0x92u) == (5 * 100) >> 7,
           "8008FE10: bit 15 raw width");
    { const unsigned char b[] = {0x05}; a = pah_seq(b, 1); }
    PE_StoreU32(a + 0x30u, 1000u);
    func_8008FE10(a);
    ASSERT(PE_LoadU16(a + 0x92u) == (5 * ((1000 * 15) >> 8)) >> 7, "8008FE10: scaled width");

    /* src/func_8008FE68.c / 8008FFE4.c / 80090108.c */
    { const unsigned char b[] = {0, 0x10}; a = pah_seq(b, 2); }
    PE_StoreU16(a + 0x94u, 0x2000u);
    func_8008FE68(a);
    ASSERT(PE_LoadU16(a + 0x96u) == 0x100u && PE_LoadU16(a + 0x98u) == 0xFFF0u,
           "8008FE68: (0x1000 - 0x2000) / 0x100");
    { const unsigned char b[] = {4, 0x10}; a = pah_seq(b, 2); }
    PE_StoreU16(a + 0xA6u, 0x800u);
    func_8008FFE4(a);
    ASSERT(PE_LoadU16(a + 0xA8u) == 4u && PE_LoadU16(a + 0xAAu) == 0x200u, "8008FFE4");
    { const unsigned char b[] = {0, 0x20}; a = pah_seq(b, 2); }
    func_80090108(a);
    ASSERT(PE_LoadU16(a + 0xB6u) == 0x100u && PE_LoadU16(a + 0xB8u) == 0x10u,
           "80090108: (0x20 << 7) / 0x100");

    /* src/func_80090078.c: +0x24 = D_8009C080[b1]. */
    PE_StoreU32(0x8009C080u + 3u * 4u, 0xDEADBEEFu);
    { const unsigned char b[] = {0, 3}; a = pah_seq(b, 2); }
    func_80090078(a);
    ASSERT(PE_LoadU32(a + 0x38u) == 4u && PE_LoadU16(a + 0xAEu) == 0x100u &&
           PE_LoadU16(a + 0xB2u) == 3u && PE_LoadU16(a + 0xB0u) == 1u &&
           PE_LoadU32(a + 0x24u) == 0xDEADBEEFu, "80090078: waveform table lookup");

    /* src/func_80090B30.c / 80090BA0.c: b + 1, 0 -> 0x101. */
    { const unsigned char b[] = {0}; a = pah_seq(b, 1); }
    func_80090B30(a);
    ASSERT(PE_LoadU16(a + 0xBAu) == 0x101u, "80090B30: 0 -> 0x101");
    { const unsigned char b[] = {0xFF}; a = pah_seq(b, 1); }
    func_80090B30(a);
    ASSERT(PE_LoadU16(a + 0xBAu) == 0x100u, "80090B30: 0xFF + 1");
    { const unsigned char b[] = {7}; a = pah_seq(b, 1); }
    func_80090BA0(a);
    ASSERT(PE_LoadU16(a + 0xBCu) == 8u, "80090BA0: +0xBC = b + 1");
    PASS();
}

/* ── control flow: loop breaks, subroutine target, note block ─────── */
static void test_PA_handhi_seq_flow(void)
{
    pe_addr_t a;
    TEST("PA_handhi_seq_flow");
    ResetTestState();

    /* src/func_800907DC.c: last pass (counter + 1 == n) takes the jump. */
    { const unsigned char b[] = {3, 0x10, 0x00}; a = pah_seq(b, 3); }
    PE_StoreU16(a + 0xCEu, 1u);
    PE_StoreU16(a + 0x64u, 2u);             /* +0x62 + 1*2 */
    func_800907DC(a);
    ASSERT(PE_LoadU32(a) == PAH_DATA + 3u + 0x10u && PE_LoadU16(a + 0xCEu) == 1u,
           "800907DC: jump from after the operand, no pop");
    { const unsigned char b[] = {3, 0x10, 0x00}; a = pah_seq(b, 3); }
    PE_StoreU16(a + 0xCEu, 1u);
    func_800907DC(a);
    ASSERT(PE_LoadU32(a) == PAH_DATA + 3u, "800907DC: not last pass skips 3 bytes");

    /* src/func_8009086C.c: jump + pop; n = 0 means 0x100. */
    { const unsigned char b[] = {0, 0xFC, 0xFF}; a = pah_seq(b, 3); }
    PE_StoreU16(a + 0xCEu, 0u);
    PE_StoreU16(a + 0x62u, 0xFFu);
    func_8009086C(a);
    ASSERT(PE_LoadU32(a) == PAH_DATA + 3u - 4u && PE_LoadU16(a + 0xCEu) == 3u,
           "8009086C: 0xFF + 1 == 0x100 jumps back, slot (0 - 1) & 3");

    /* src/func_800909C0.c */
    { const unsigned char b[] = {0xFC, 0xFF}; a = pah_seq(b, 2); }
    PE_StoreU16(a + 0x46u, 0x8001u);
    func_800909C0(a);
    ASSERT(PE_LoadU32(a) == PAH_DATA + 2u && PE_LoadU32(a + 0x14u) == PAH_DATA + 2u - 4u &&
           PE_LoadU16(a + 0x6Au) == 0x8001u && PE_LoadU32(a + 0x38u) == 8u,
           "800909C0: target past operand, +0x6A = +0x46");

    /* src/func_80090C88.c: offsets 4 and 0 (NULL); note block; boundary
     * func_8008A92C(D_800B89D0, r1, r2); cursor += 4. */
    { const unsigned char b[] = {0x04, 0x00, 0x00, 0x00}; a = pah_seq(b, 4); }
    PE_StoreU16(a + 0x76u, 0xAB00u);
    PE_StoreU32(a + 0x44u, 0xFF800000u);
    PE_StoreU32(0x800B89D4u, 0x11u);
    PE_StoreU32(0x800B89D8u, 0x22u);
    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();
    func_80090C88(a);
    ASSERT(PE_LoadU32(0x800B89D4u) == 0u && PE_LoadU32(0x800B89D8u) == 0u &&
           PE_LoadU32(0x800B89DCu) == 0xABu && PE_LoadU32(0x800B89E0u) == 0xFFFFFFFFu &&
           PE_LoadU32(a) == PAH_DATA + 4u, "80090C88: note block, arithmetic >> 23");
    ASSERT(PE_Decomp_BoundaryCount() == 1 &&
           strcmp(PE_Decomp_BoundaryName(0), "func_8008A92C") == 0 &&
           g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].arg0 == 0x800B89D0u &&
           g_bootstrap_arg4_calls[0].arg1 == PAH_DATA + 4u + 2u &&
           g_bootstrap_arg4_calls[0].arg2 == 0u, "80090C88: r1 = p + o + 2, r2 NULL");
    PASS();
}

/* ── reverb slot claim + func_800903A0 ────────────────────────────── */
static void test_PA_handhi_seq_reverb(void)
{
    pe_addr_t a;
    TEST("PA_handhi_seq_reverb");
    ResetTestState();
    pah_master();

    /* src/func_80090D54.c: lowest clear bit of +4 | +0x30 is claimed. */
    PE_StoreU32(PAH_MB + 4u, 0x5u);
    PE_StoreU32(PAH_MB + 0x30u, 0x2u);
    PE_StoreU32(0x8009D2C4u, 0u);
    { const unsigned char b[] = {0x7F}; a = pah_seq(b, 1); }
    PE_StoreU16(a + 0x60u, 5u);
    func_80090D54(a, 0x40);
    ASSERT(PE_LoadU16(a + 0x5Eu) == 0x7F00u && PE_LoadU16(a + 0x60u) == 0u &&
           PE_LoadU32(PAH_MB + 0x30u) == 0xAu && PE_LoadU16(a + 0x5Cu) == 3u &&
           PE_LoadU32(a + 0x38u) == 0x800u, "80090D54: claims bit 3");
    /* func_800903A0: main track -> D_8009D2C8[14] |= a1; func_80089B28. */
    ASSERT(PE_LoadU32(PAH_MB + 0x38u) == 0x40u && PE_LoadU32(0x8009D2C4u) == 0x100u,
           "80090D54: func_800903A0 main-track mask");

    /* Already claimed (+0x38 bit 11): nothing taken. */
    { const unsigned char b[] = {0x01}; a = pah_seq(b, 1); }
    PE_StoreU32(a + 0x38u, 0x800u);
    PE_StoreU16(a + 0x5Cu, 9u);
    func_80090D54(a, 0);
    ASSERT(PE_LoadU32(PAH_MB + 0x30u) == 0xAu && PE_LoadU16(a + 0x5Cu) == 9u,
           "80090D54: bit 11 set skips the claim");

    /* All 24 bits busy: loop exits with bit 0x1000000, nothing claimed;
     * sub-track (+0x54 != 0) -> D_800BCD70 |= a1. */
    PE_StoreU32(PAH_MB + 4u, 0xFFFFFFu);
    PE_StoreU32(0x800BCD70u, 1u);
    { const unsigned char b[] = {0x01}; a = pah_seq(b, 1); }
    PE_StoreU16(a + 0x54u, 1u);
    func_80090D54(a, 0x80);
    ASSERT(PE_LoadU32(PAH_MB + 0x30u) == 0xAu && PE_LoadU32(a + 0x38u) == 0u &&
           PE_LoadU32(0x800BCD70u) == 0x81u, "80090D54: no free slot, sub-track mask");

    /* src/func_80090E20.c: ((short)(c << 8) - hi) / steps. */
    PE_StoreU32(PAH_MB + 4u, 0u);
    PE_StoreU32(PAH_MB + 0x30u, 0u);
    { const unsigned char b[] = {0, 0x80}; a = pah_seq(b, 2); }
    PE_StoreU16(a + 0x5Eu, 0x12FFu);
    func_80090E20(a, 2);
    ASSERT(PE_LoadU16(a + 0x60u) == 0x100u && PE_LoadU16(a + 0x5Eu) == 0x1200u &&
           PE_LoadU16(a + 0xD6u) == (uint16_t)((-0x8000 - 0x1200) / 0x100) &&
           PE_LoadU32(PAH_MB + 0x30u) == 1u && PE_LoadU16(a + 0x5Cu) == 0u &&
           PE_LoadU32(PAH_MB + 0x38u) == (0x40u | 2u), "80090E20: fade + claim bit 0");
    PASS();
}

/* ── program change / vibrato / tremolo ───────────────────────────── */
static void pah_program_setup(unsigned old, uint32_t pitch, uint32_t rate, uint32_t den)
{
    pah_master();
    PE_StoreU32(PAH_MB, 0x100u);            /* bank-B mode */
    PE_StoreU32(PAH_MB + 0x14u, 1u);
    PE_StoreU32(0x800BCD50u, 0u);
    PE_StoreU16(PAH_REC + 0x5Au, (uint16_t)old);
    PE_StoreU32(PAH_REC + 0x30u, pitch);
    PE_StoreU32(PAH_REC + 0x38u, 0x1001u);
    PE_StoreU32(0x800B2900u + 0x51u * 0x40u + 0x00u, 0x11223344u);
    PE_StoreU32(0x800B2900u + 0x51u * 0x40u + 0x04u, 0x55667788u);
    PE_StoreU32(0x800B2900u + 0x51u * 0x40u + 0x10u, rate);
    PE_StoreU32(0x800B2900u + 0x21u * 0x40u + 0x10u, rate);
    PE_StoreU32(0x800B2910u + old * 0x40u, den);   /* D_800B2910[old * 0x10] */
}

static void test_PA_handhi_seq_program(void)
{
    pe_addr_t a;
    const unsigned char b[] = {0x21};
    TEST("PA_handhi_seq_program");
    ResetTestState();

    /* src/func_8008F898.c: main track + bank B remaps 0x21 -> 0x51;
     * pitch = pitch * rate / D_800B2910[old * 0x10] (unsigned). */
    a = pah_seq(b, 1);
    pah_program_setup(2u, 1000u, 3u, 7u);
    func_8008F898(a, 1u);
    ASSERT(PE_LoadU16(a + 0x5Au) == 0x51u && PE_LoadU32(a + 0x30u) == 3000u / 7u,
           "8008F898: bank-B remap, rescaled pitch");
    /* func_8008F0D0(a0, rec, rec word 0); +0x38 bit 9 clear -> +0xF4 |= 0x1FF80. */
    ASSERT(PE_LoadU32(a + 0xF8u) == 0x11223344u && PE_LoadU32(a + 0xFCu) == 0x55667788u &&
           PE_LoadU32(a + 0xF4u) == (0x10u | 0x1FF80u) && PE_LoadU32(a + 0x38u) == 1u,
           "8008F898: attributes from rec[0], +0x38 bit 12 cleared");

    /* Unsigned multiply/divide: 0x80000000 * 1 / 2 = 0x40000000. */
    a = pah_seq(b, 1);
    pah_program_setup(2u, 0x80000000u, 1u, 2u);
    func_8008F898(a, 1u);
    ASSERT(PE_LoadU32(a + 0x30u) == 0x40000000u, "8008F898: unsigned division");

    /* Held voice ((+0x14 & a1) & D_800BCD50 != 0 on the main track): no rescale. */
    a = pah_seq(b, 1);
    pah_program_setup(2u, 1000u, 3u, 7u);
    PE_StoreU32(0x800BCD50u, 1u);
    func_8008F898(a, 1u);
    ASSERT(PE_LoadU32(a + 0x30u) == 1000u && PE_LoadU32(a + 0xF4u) == 0x1FF80u,
           "8008F898: held voice keeps pitch, no 0x10 flag");

    /* old == 0xFF: no previous program, no rescale. */
    a = pah_seq(b, 1);
    pah_program_setup(0xFFu, 1000u, 3u, 7u);
    func_8008F898(a, 1u);
    ASSERT(PE_LoadU32(a + 0x30u) == 1000u && PE_LoadU16(a + 0x5Au) == 0x51u,
           "8008F898: old 0xFF skips the rescale");

    /* src/func_8008F9CC.c: fixed attributes 0x1010; a sub-track does not
     * remap and rescales regardless of the hold mask. */
    a = pah_seq(b, 1);
    pah_program_setup(2u, 1000u, 3u, 7u);
    PE_StoreU32(0x800BCD50u, 1u);
    PE_StoreU16(a + 0x54u, 1u);
    func_8008F9CC(a, 1u);
    ASSERT(PE_LoadU16(a + 0x5Au) == 0x21u && PE_LoadU32(a + 0x30u) == 3000u / 7u &&
           PE_LoadU32(a + 0xF8u) == 0x1010u, "8008F9CC: sub-track, attributes 0x1010");
    PASS();
}

static void test_PA_handhi_seq_lfo(void)
{
    pe_addr_t a;
    TEST("PA_handhi_seq_lfo");
    ResetTestState();
    PE_StoreU32(0x8009C080u + 1u * 4u, 0xA1u);
    PE_StoreU32(0x8009C080u + 2u * 4u, 0xA2u);

    /* src/func_8008FD10.c sub-track: delay 0, depth b0 << 8, width u16 +0x30. */
    { const unsigned char b[] = {0x05, 0x00, 0x02}; a = pah_seq(b, 3); }
    PE_StoreU16(a + 0x54u, 1u);
    PE_StoreU16(a + 0x88u, 9u);
    PE_StoreU32(a + 0x30u, 1000u);
    func_8008FD10(a);
    ASSERT(PE_LoadU32(a + 0x38u) == 1u && PE_LoadU16(a + 0x88u) == 0u &&
           PE_LoadU16(a + 0x94u) == 0x500u && PE_LoadU16(a + 0x8Cu) == 0x100u &&
           PE_LoadU16(a + 0x90u) == 2u &&
           PE_LoadU16(a + 0x92u) == (5u * (unsigned)((1000 * 15) >> 8)) >> 7 &&
           PE_LoadU16(a + 0x8Au) == 0u && PE_LoadU16(a + 0x8Eu) == 1u &&
           PE_LoadU32(a + 0x1Cu) == 0xA2u && PE_LoadU32(a) == PAH_DATA + 3u,
           "8008FD10: sub-track vibrato");
    /* Main track: b0 is the delay; depth +0x94 kept; +0x30 read as u16. */
    { const unsigned char b[] = {9, 7, 1}; a = pah_seq(b, 3); }
    PE_StoreU16(a + 0x94u, 0x8300u);
    PE_StoreU32(a + 0x30u, 0x12345678u);
    func_8008FD10(a);
    ASSERT(PE_LoadU16(a + 0x88u) == 9u && PE_LoadU16(a + 0x8Au) == 9u &&
           PE_LoadU16(a + 0x8Cu) == 7u && PE_LoadU16(a + 0x94u) == 0x8300u &&
           PE_LoadU16(a + 0x92u) == (3u * 0x5678u) >> 7 && PE_LoadU32(a + 0x1Cu) == 0xA1u,
           "8008FD10: main-track vibrato, u16 width");
    /* Sub-track with depth byte 0 keeps +0x94. */
    { const unsigned char b[] = {0, 1, 1}; a = pah_seq(b, 3); }
    PE_StoreU16(a + 0x54u, 1u);
    PE_StoreU16(a + 0x94u, 0x0200u);
    func_8008FD10(a);
    ASSERT(PE_LoadU16(a + 0x94u) == 0x0200u, "8008FD10: depth 0 keeps +0x94");

    /* src/func_8008FEFC.c */
    { const unsigned char b[] = {0, 3, 2}; a = pah_seq(b, 3); }
    PE_StoreU16(a + 0x54u, 1u);
    PE_StoreU16(a + 0x9Cu, 4u);
    PE_StoreU16(a + 0xA6u, 0x1234u);
    func_8008FEFC(a);
    ASSERT(PE_LoadU32(a + 0x38u) == 2u && PE_LoadU16(a + 0x9Cu) == 0u &&
           PE_LoadU16(a + 0xA6u) == 0x1234u && PE_LoadU16(a + 0xA0u) == 3u &&
           PE_LoadU16(a + 0xA4u) == 2u && PE_LoadU16(a + 0x9Eu) == 0u &&
           PE_LoadU16(a + 0xA2u) == 1u && PE_LoadU32(a + 0x20u) == 0xA2u,
           "8008FEFC: sub-track tremolo");
    { const unsigned char b[] = {6, 0, 1}; a = pah_seq(b, 3); }
    func_8008FEFC(a);
    ASSERT(PE_LoadU16(a + 0x9Cu) == 6u && PE_LoadU16(a + 0x9Eu) == 6u &&
           PE_LoadU16(a + 0xA0u) == 0x100u && PE_LoadU32(a + 0x20u) == 0xA1u,
           "8008FEFC: main-track tremolo");
    { const unsigned char b[] = {0x7, 1, 1}; a = pah_seq(b, 3); }
    PE_StoreU16(a + 0x54u, 1u);
    func_8008FEFC(a);
    ASSERT(PE_LoadU16(a + 0xA6u) == 0x700u, "8008FEFC: sub-track depth b0 << 8");
    PASS();
}

/* ── memory-card tags + small helpers ─────────────────────────────── */
static void test_PA_handhi_card_rest(void)
{
    const pe_addr_t c = PAH_REC;
    TEST("PA_handhi_card_rest");
    ResetTestState();

    /* src/func_80083E50.c / 80083EA4.c / 80083EC4.c: tag + one-byte arg. */
    PE_Fill(c, 0x100u, 0u);
    func_80083E50(c, 0x9Au);
    ASSERT(PE_LoadU8(c + 0x36u) == 0x43u && PE_LoadU32(c + 0x2Cu) == c + 0x24u &&
           PE_LoadU8(c + 0x24u) == 0x9Au && PE_LoadU8(c + 0x35u) == 1u, "80083E50: 'C'");
    func_80083EA4(c, 1u);
    ASSERT(PE_LoadU8(c + 0x36u) == 0x46u && PE_LoadU8(c + 0x24u) == 1u, "80083EA4: 'F'");
    func_80083EC4(c, 2u);
    ASSERT(PE_LoadU8(c + 0x36u) == 0x47u && PE_LoadU8(c + 0x24u) == 2u, "80083EC4: 'G'");

    /* src/func_800835C0.c state 2 -> func_80083E70(a0): 'E', +0x2C = 0, +0x35 = 0. */
    PE_StoreU8(c + 0x46u, 2u);
    func_800835C0(c);
    ASSERT(PE_LoadU8(c + 0x36u) == 0x45u && PE_LoadU32(c + 0x2Cu) == 0u &&
           PE_LoadU8(c + 0x35u) == 0u, "800835C0: state 2 -> 80083E70 on a0");
    /* state 4 -> 'G' with a0[0x47]; other states do nothing. */
    PE_StoreU8(c + 0x46u, 4u);
    PE_StoreU8(c + 0x47u, 0x33u);
    func_800835C0(c);
    ASSERT(PE_LoadU8(c + 0x36u) == 0x47u && PE_LoadU8(c + 0x24u) == 0x33u, "800835C0: state 4");
    PE_StoreU8(c + 0x36u, 0u);
    PE_StoreU8(c + 0x46u, 5u);
    func_800835C0(c);
    ASSERT(PE_LoadU8(c + 0x36u) == 0u, "800835C0: state 5 untouched");

    /* src/func_8008389C.c: 2 'L', 3 'F', 4 'G' (a0[0x48] == 0) else 80083EE4 'K'. */
    PE_StoreU8(c + 0x47u, 0x5u);
    PE_StoreU8(c + 0x46u, 2u);
    func_8008389C(c);
    ASSERT(PE_LoadU8(c + 0x36u) == 0x4Cu && PE_LoadU8(c + 0x24u) == 5u, "8008389C: state 2 'L'");
    PE_StoreU8(c + 0x46u, 3u);
    func_8008389C(c);
    ASSERT(PE_LoadU8(c + 0x36u) == 0x46u, "8008389C: state 3 'F'");
    PE_StoreU8(c + 0x46u, 4u);
    PE_StoreU8(c + 0x48u, 0u);
    func_8008389C(c);
    ASSERT(PE_LoadU8(c + 0x36u) == 0x47u && PE_LoadU8(c + 0x35u) == 1u, "8008389C: state 4 'G'");
    PE_StoreU8(c + 0x48u, 1u);
    func_8008389C(c);
    ASSERT(PE_LoadU8(c + 0x36u) == 0x4Bu && PE_LoadU32(c + 0x2Cu) == 0u &&
           PE_LoadU8(c + 0x35u) == 0u, "8008389C: state 4 held -> 80083EE4 on a0");

    /* src/func_8008F178.c: +0x5A = a1; func_8008F0D0(a0, D_800B2900 + a1*64, word 0). */
    PE_Fill(c, 0x200u, 0u);
    PE_StoreU32(0x800B2900u + 3u * 0x40u, 0xCAFEF00Du);
    PE_StoreU32(0x800B2900u + 3u * 0x40u + 4u, 0x0BADBEEFu);
    PE_StoreU8(0x800B2900u + 3u * 0x40u + 8u, 0x42u);
    func_8008F178(c, 3);
    ASSERT(PE_LoadU16(c + 0x5Au) == 3u && PE_LoadU32(c + 0xF8u) == 0xCAFEF00Du &&
           PE_LoadU32(c + 0xFCu) == 0x0BADBEEFu && PE_LoadU16(c + 0x10Eu) == 0x42u,
           "8008F178: program record a1 << 6, attributes = its word 0");

    /* src/func_80085D84.c: unchanged -> return old, no call. */
    PE_StoreU32(0x8009B438u, 4u);
    g_bootstrap_arg4_call_count = 0;
    ASSERT(func_80085D84(4) == 4 && g_bootstrap_arg4_call_count == 0,
           "80085D84: unchanged value makes no call");
    /* Changed: store, then func_80085DC4 -> func_80073CC4(9, a0); a0 is the
     * live $a0 (pe_dis.sh 0x80085D84 0x3C).  Source 9 reaches pc_port's
     * func_800740D0 source cut, which logs (source, handler). */
    ASSERT(func_80085D84(7) == 4 && PE_LoadU32(0x8009B438u) == 7u,
           "80085D84: returns old, stores new");
    ASSERT(g_bootstrap_arg4_call_count == 1 && g_bootstrap_arg4_calls[0].arg0 == 9u &&
           g_bootstrap_arg4_calls[0].arg1 == 7u, "80085D84: forwards a0 as the handler");
    PASS();
}

/* ── texture uploads (func_80074774 / func_800CED3C) ──────────────── */
static void test_PA_handhi_texture(void)
{
    uint16_t px;
    unsigned i;
    TEST_RETAIL_DISC1("PA_handhi_texture"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    ResetTestState();HostFB_Init();PE_GPU_Init();B54KR_SeedGpuStatic();

    /* src/func_80074774.c mode 1: RECT {a3, a4, a5 / 2, a6}; returns
     * func_80077A64(a1, a2, a3, a4) (src/func_80077A64.c). */
    for (i = 0; i < 8u; i++)
        PE_StoreU16(PAH_BUF + i * 2u, (uint16_t)(0x1000u + i));
    {
        unsigned short tp = func_80074774((int)PAH_BUF, 1, 2, 0x140, 0x100, 8, 2);
        unsigned expect = ((1u & 3u) << 7) | ((2u & 3u) << 5) | ((0x100u & 0x100u) >> 4) |
                          ((0x140u & 0x3FFu) >> 6) | ((0x100u & 0x200u) << 2);
        ASSERT(tp == expect, "80074774: texpage id from func_80077A64");
    }
    ASSERT(func_80074DC0(0) == 0, "80074774: upload drained");
    for (i = 0; i < 8u; i++) {
        ASSERT(PE_GPU_ReadVRAM(0x140u + (i & 3u), 0x100u + (i >> 2), &px) && px == 0x1000u + i,
               "80074774: 4x2 rectangle (width a5 / 2)");
    }

    /* src/func_800CED3C.c: 64x256 at (0x380, 0x100) from D_800B0E18 (a0 == 0)
     * else D_800B0E1C; D_800F34E4 = a0. */
    PE_StoreU32(0x800B0E18u, PAH_BUF);
    PE_StoreU32(0x800B0E1Cu, PAH_BUF + 0x80u);
    PE_Fill(PAH_BUF, 0x100u, 0u);
    PE_StoreU16(PAH_BUF + 0x00u, 0x1111u);
    PE_StoreU16(PAH_BUF + 0x7Eu, 0x2222u);   /* pixel 63 of row 0 */
    PE_StoreU16(PAH_BUF + 0x80u, 0x3333u);   /* D_800B0E1C source pixel 0 */
    func_800CED3C(0);
    ASSERT(func_80074DC0(0) == 0 && PE_LoadU16(0x800F34E4u) == 0u, "800CED3C: a0 0");
    ASSERT(PE_GPU_ReadVRAM(0x380u, 0x100u, &px) && px == 0x1111u &&
           PE_GPU_ReadVRAM(0x3BFu, 0x100u, &px) && px == 0x2222u &&
           PE_GPU_ReadVRAM(0x380u, 0x101u, &px) && px == 0x3333u,
           "800CED3C: 64-wide rows from D_800B0E18");
    func_800CED3C(1);
    ASSERT(func_80074DC0(0) == 0 && PE_LoadU16(0x800F34E4u) == 1u &&
           PE_GPU_ReadVRAM(0x380u, 0x100u, &px) && px == 0x3333u,
           "800CED3C: a0 != 0 uses D_800B0E1C");
    PASS();
}

/* ── wrapped text (func_8005F698) ─────────────────────────────────── */
static void test_PA_handhi_text(void)
{
    const unsigned char text[] = {0x20, 0x21, 0x22, 0xFF};
    TEST("PA_handhi_text");
    ResetTestState();

    /* Menu packet arena + OT for func_8005EED4 (the per-glyph emitter). */
    PE_StoreU32(0x8009D104u, PAH_BUF + 0x400u);
    PE_StoreU32(0x8009D100u, PAH_BUF + 0x400u);
    PE_StoreU32(0x8009D11Cu, PAH_BUF + 0x3F0u);
    /* Width nibble 5 for codes 0x20..0x22 (func_8005DC28: D_800A8028 +
     * D_800A8050 + code). */
    PE_StoreU32(0x800A8050u, 0u);
    PE_StoreU8(0x800A8028u + 0x20u, 0x50u);
    PE_StoreU8(0x800A8028u + 0x21u, 0x50u);
    PE_StoreU8(0x800A8028u + 0x22u, 0x50u);
    PE_StoreU32(0x8009D12Cu, 0x800A2270u);   /* empty pen stack */
    PE_StoreU32(0x8009D124u, 10u);
    PE_StoreU32(0x8009D128u, 20u);
    for (unsigned i = 0; i < sizeof(text); i++)
        PE_StoreU8(PAH_BUF + i, text[i]);

    /* src/func_8005F698.c: each glyph adds width 5 + spacing 1 (code
     * >= 0xA, != 0xF), so a width limit of 12 fits two glyphs per line:
     * line 1 = 0x20 0x21, line 2 = 0x22.  Each line pushes/pops the pen and
     * then moves it down 0xE. */
    func_8005F698(PAH_BUF, 12);
    ASSERT(PE_LoadU32(0x8009D12Cu) == 0x800A2270u, "8005F698: pen stack balanced");
    ASSERT(PE_LoadU32(0x8009D124u) == 10u && PE_LoadU32(0x8009D128u) == 20u + 2u * 0xEu,
           "8005F698: x restored, y down 0xE per line");
    ASSERT(PE_LoadU32(0x8009D100u) == PAH_BUF + 0x400u + 3u * 40u,
           "8005F698: one glyph packet per character");
    ASSERT(PE_LoadU32(0x8009CDB0u) == 1u, "8005F698: spacing 1 for code 0x22");
    PASS();
}

static void test_PA_handhi_all(void)
{
    test_PA_handhi_seq_simple();
    test_PA_handhi_seq_params();
    test_PA_handhi_seq_master();
    test_PA_handhi_seq_fades();
    test_PA_handhi_seq_flow();
    test_PA_handhi_seq_reverb();
    test_PA_handhi_seq_program();
    test_PA_handhi_seq_lfo();
    test_PA_handhi_card_rest();
    test_PA_handhi_texture();
    test_PA_handhi_text();
}
