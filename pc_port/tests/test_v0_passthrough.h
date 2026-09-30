/* Batch O: retail $v0 of func_80062A7C, returned through func_80044E14.
 *
 * PE_V0_func_80062A7C (decomp_hand/func_80044E14_port.c) is a line-by-line
 * port of src/func_80062A7C.c plus the value retail leaves in $v0 at its
 * single `jr ra`: the final pop's `lw v0,-8(v1)` (the popped word, which is
 * also the new D_8009D124), or 0 on underflow (func_800527C0 is empty; $v0
 * still holds the `sltu` result).  func_80062A7C is the same port with the
 * value discarded.  VPASS1 checks the draw-state stack against a model of the
 * matched C over the stack-pointer grid x {glyph string, empty string,
 * out-of-range id (str == 0)}; VPASS2 pins each $v0 path; VPASS3 checks
 * func_80044E14's return. */
uint32_t PE_V0_func_80062A7C(uint32_t a0);
int func_8005F1A0(pe_addr_t pe_p);

#define VP_HDR   0x80170000u   /* message header: ptr = mem32[0x800A802C] + 0x800A8028 */
#define VP_TBL   0x80170100u   /* ptr + mem32[ptr + 4] */
#define VP_FONT  0x80170800u   /* D_800A8028 + D_800A8050: glyph metrics */

static void vp_env(uint32_t sp0)
{
    static const uint8_t glyphs[] = {0x01u, 0x0Fu, 0x21u, 0xFAu, 0x05u, 0x33u, 0xFFu};
    uint32_t i;
    ResetTestState();
    PE_StoreU32(0x800A802Cu, VP_HDR - 0x800A8028u);
    PE_StoreU32(VP_HDR + 4u, VP_TBL - VP_HDR);
    PE_StoreU16(VP_TBL, 2u);                          /* ids 0, 1 valid */
    PE_StoreU16(VP_TBL + 2u, 0x20u);                  /* id 0: glyph string */
    PE_StoreU16(VP_TBL + 4u, 0x40u);                  /* id 1: empty string */
    for (i = 0; i < sizeof glyphs; i++) PE_StoreU8(VP_TBL + 0x20u + i, glyphs[i]);
    PE_StoreU8(VP_TBL + 0x40u, 0xFFu);
    PE_StoreU8(0x80000000u, 0xFFu);                   /* str == 0 reads guest 0 */
    PE_StoreU32(0x800A8050u, VP_FONT - 0x800A8028u);
    for (i = 0; i < 0x200u; i++) PE_StoreU8(VP_FONT + i, (uint8_t)(0x31u + i * 7u));
    PE_StoreU32(0x8009D100u, 0x80180000u); PE_StoreU32(0x8009D104u, 0x80180000u);
    PE_StoreU32(0x8009D11Cu, 0x80190000u); PE_StoreU32(0x80190000u, 0x00FFFFFFu);
    PE_StoreU32(0x8009D124u, 40u); PE_StoreU32(0x8009D128u, 60u);
    PE_StoreU32(0x8009D138u, 200u); PE_StoreU32(0x8009D0D8u, 0u);
    for (i = 0x800A2250u; i < 0x800A22C8u; i += 4u) PE_StoreU32(i, 0x5A000000u | i);  /* stale words */
    PE_StoreU32(0x8009D12Cu, sp0);
}

/* VPASS1 model of src/func_80062A7C.c's draw-state stack: push (x, y);
 * centre x by (D_8009D138 - (w + 4)) >> 1 with w from the matched measure
 * func_8005F1A0; if str != 0: push (x, y), draw, pop; then pop.  A push needs
 * sp < D_800A22B0, a pop sp > D_800A2270; a failed one only calls the empty
 * func_800527C0. */
static void test_VPASS1_v0_variant_stack_model(void)
{
    static const uint32_t sps[] = {0x800A2260u, 0x800A2268u, 0x800A2270u, 0x800A2278u,
                                   0x800A2290u, 0x800A22A0u, 0x800A22A8u, 0x800A22B0u, 0x800A22B8u};
    static const uint32_t ids[] = {0u, 1u, 7u};       /* glyphs, empty, out of range */
    uint32_t mem[(0x800A22C8u - 0x800A2250u) / 4u];
    unsigned si, k, n_pop = 0, n_under = 0;
    TEST("VPASS1_v0_variant_stack_model");
    for (si = 0; si < sizeof sps / sizeof sps[0]; si++) for (k = 0; k < 3u; k++) {
        uint32_t s = sps[si], a, v0, want_v0 = 0u, x, y = 60u, str;
        int32_t w;
        int pop3_failed = 0;
        vp_env(s);
        str = func_8005DC4C(ids[k]);
        w = func_8005F1A0(str);                        /* matched measure (generated TU) */
        for (a = 0x800A2250u; a < 0x800A22C8u; a += 4u) mem[(a - 0x800A2250u) / 4u] = 0x5A000000u | a;
#define VM(addr) mem[((addr) - 0x800A2250u) / 4u]
        if (s < 0x800A22B0u) { VM(s) = 40u; VM(s + 4u) = 60u; s += 8u; }
        x = (uint32_t)(40 + ((200 - (w + 4)) >> 1));
        if (str != 0u) {
            if (s < 0x800A22B0u) { VM(s) = x; VM(s + 4u) = y; s += 8u; }
            if (s > 0x800A2270u) { s -= 8u; x = VM(s); y = VM(s + 4u); } else pop3_failed = 1;
        }
        if (s > 0x800A2270u) { s -= 8u; x = VM(s); y = VM(s + 4u); want_v0 = x; }
        else if (pop3_failed && ids[k] == 0u) x = y = 0xFFFFFFFFu;   /* post-draw pen: not modelled */
#undef VM
        vp_env(sps[si]);
        v0 = PE_V0_func_80062A7C(ids[k]);
        if (v0 != want_v0 || PE_LoadU32(0x8009D12Cu) != s
            || (x != 0xFFFFFFFFu && (PE_LoadU32(0x8009D124u) != x || PE_LoadU32(0x8009D128u) != y)))
            fprintf(stderr, "VPASS1 sp0=%08X id=%u: v0 %08X/%08X sp %08X/%08X xy %08X,%08X/%08X,%08X\n",
                    sps[si], ids[k], v0, want_v0, PE_LoadU32(0x8009D12Cu), s,
                    PE_LoadU32(0x8009D124u), PE_LoadU32(0x8009D128u), x, y);
        ASSERT(v0 == want_v0, "$v0 = popped word, or 0 on underflow");
        ASSERT(PE_LoadU32(0x8009D12Cu) == s, "D_8009D12C follows the push/pop model");
        ASSERT(x == 0xFFFFFFFFu || (PE_LoadU32(0x8009D124u) == x && PE_LoadU32(0x8009D128u) == y),
               "D_8009D124/128 follow the push/pop model");
        for (a = 0x800A2250u; a < 0x800A22C8u; a += 4u)
            ASSERT(PE_LoadU32(a) == mem[(a - 0x800A2250u) / 4u], "stack words follow the push model");
        if (want_v0) n_pop++; else n_under++;
    }
    ASSERT(n_pop > 0 && n_under > 0, "grid covers both $v0 paths");
    PASS();
}

static void test_VPASS2_v0_paths(void)
{
    uint32_t v0;
    TEST("VPASS2_v0_paths");
    vp_env(0x800A2270u);                              /* pop, pop: returns pushed x0 */
    v0 = PE_V0_func_80062A7C(0u);
    ASSERT(v0 == 40u && PE_LoadU32(0x8009D124u) == 40u && PE_LoadU32(0x8009D128u) == 60u
           && PE_LoadU32(0x8009D12Cu) == 0x800A2270u, "non-empty: $v0 = popped word = new D_8009D124");
    vp_env(0x800A2268u);                              /* final pop finds sp == D_800A2270 */
    v0 = PE_V0_func_80062A7C(0u);
    ASSERT(v0 == 0u && PE_LoadU32(0x8009D12Cu) == 0x800A2270u, "underflow: $v0 = 0");
    vp_env(0x800A22B0u);                              /* full: both pushes skipped, pops stale */
    v0 = PE_V0_func_80062A7C(1u);
    ASSERT(v0 == (0x5A000000u | 0x800A22A0u) && PE_LoadU32(0x8009D124u) == v0
           && PE_LoadU32(0x8009D12Cu) == 0x800A22A0u, "overflow: $v0 = stale popped word");
    PASS();
}

static void test_VPASS3_func_80044E14_returns_v0(void)
{
    const pe_addr_t win = 0x80171000u;
    TEST("VPASS3_func_80044E14_returns_v0");
    vp_env(0x800A2270u);
    PE_StoreU32(win + 0x24u, 0x29u + 1u);             /* idx 1 */
    PE_StoreU8(0x800A1980u + 64u, 0xFFu);             /* empty item label */
    PE_StoreU32(0x8009CFA0u + 4u, 1u);                /* D_8009CFA0[1] = message 1 */
    ASSERT(func_80044E14(win) == 40, "entry set: returns func_80062A7C's popped word");
    /* The item-label draw (func_8005F594 -> func_8005F354, balanced push/pop)
     * always leaves D_8009D12C >= D_800A2270, so func_80062A7C's final pop
     * cannot underflow through func_80044E14: from a full stack both label
     * pushes are skipped and its pops leave sp = 0x800A22A0. */
    vp_env(0x800A22B0u);
    PE_StoreU32(win + 0x24u, 0x29u + 1u); PE_StoreU8(0x800A1980u + 64u, 0xFFu);
    PE_StoreU32(0x8009CFA0u + 4u, 1u);
    ASSERT((uint32_t)func_80044E14(win) == (0x5A000000u | 0x800A22A0u)
           && PE_LoadU32(0x8009D124u) == (0x5A000000u | 0x800A22A0u)
           && PE_LoadU32(0x8009D12Cu) == 0x800A22A0u, "entry set, full stack: returns the stale popped word");
    vp_env(0x800A2270u);
    PE_StoreU32(win + 0x24u, 0x29u + 1u); PE_StoreU8(0x800A1980u + 64u, 0xFFu);
    PE_StoreU32(0x8009CFA0u + 4u, 0u);
    ASSERT(func_80044E14(win) == 0 && PE_LoadU32(0x8009D124u) == 40u, "entry empty: returns 0, no draw");
    PASS();
}

static void test_VPASS_all(void)
{
    test_VPASS1_v0_variant_stack_model();
    test_VPASS2_v0_paths();
    test_VPASS3_func_80044E14_returns_v0();
}
