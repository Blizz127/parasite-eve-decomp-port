/* #16: M34 phase-2 room effects (room_m0348i func_80190E98 / 80191A98 /
 * 80191FE0) and the ring renderer func_800C4FC4.
 *
 * (1) The two COP2 command words the inline gte_CompMatrix macro encodes
 *     (0x4A49E012 rtir, 0x4A480012 rt) decode on pe_gte to the same
 *     sf/mx/v/cv/lm as the short forms the port passes, and both match an
 *     independent integer reference (MVMVA sf=1 lm=0, IR saturated).
 * (2) PE_M0348iCompMatrix (the macro, op-for-op) matches a pure-C reference
 *     of the macro's semantics, including r2 == r3 aliasing and IR
 *     saturation of the stlvnl translation words, and leaves RT/TR = r1.
 * (3) func_800C4FC4 on analytic inputs (identity camera, H=256, z=1024 so
 *     the projection ratio is exactly 1/4): ring vertex order per segment
 *     (i, i+1 mod n, n+i, n+(i+1 mod n)), screen xy, OTZ = SZ3 >> 2 of the
 *     4th vertex (RotTransPers4 return), POLY_G4 colours/code, the
 *     draw-mode word, OT linking and the 0x2C packet-cursor advance. */
void PE_M0348iCompMatrix(pe_addr_t r1, pe_addr_t r2, pe_addr_t r3);
void func_800C4FC4(pe_addr_t r, pe_addr_t matrix, unsigned billboard);

static int32_t m348g_sat16(int64_t v) { return v < -32768 ? -32768 : v > 32767 ? 32767 : (int32_t)v; }

static void m348g_put_matrix(pe_addr_t a, const int16_t r[9], const int32_t t[3])
{
    for (unsigned i = 0; i < 9u; i++) PE_StoreU16(a + i * 2u, (uint16_t)r[i]);
    PE_StoreU16(a + 18u, 0x5A5Au);   /* pad: never read */
    for (unsigned i = 0; i < 3u; i++) PE_StoreU32(a + 20u + i * 4u, (uint32_t)t[i]);
}

static void test_M348G_mvmva_words(void)
{
    static const int16_t R[9] = { 0x1000, -0x200, 0x0F00, 0x0123, 0x0E00, -0x7FF, -0x1000, 0x0444, 0x0B00 };
    static const int16_t V[3] = { 1234, -32000, 777 };
    static const int32_t T[3] = { 50000, -3, 9000 };
    TEST("M348G_mvmva_words");
    ResetTestState();
    for (unsigned w = 0; w < 2u; w++) {
        const uint32_t full = w ? 0x4A480012u : 0x4A49E012u, shrt = w ? 0x80012u : 0x9E012u;
        int32_t ir_full[3], mac_full[3];
        for (unsigned k = 0; k < 9u; k++) g_pe_gte.rt[k / 3u][k % 3u] = R[k];
        for (unsigned k = 0; k < 3u; k++) g_pe_gte.tr[k] = T[k];
        PE_GTE_SetIR(V[0], V[1], V[2]); PE_GTE_SetV0(V[0], V[1], V[2]);
        PE_GTE_MVMVA(full);
        memcpy(ir_full, g_pe_gte.ir, sizeof ir_full); memcpy(mac_full, g_pe_gte.mac, sizeof mac_full);
        PE_GTE_SetIR(V[0], V[1], V[2]); PE_GTE_SetV0(V[0], V[1], V[2]);
        PE_GTE_MVMVA(shrt);
        ASSERT(memcmp(ir_full, g_pe_gte.ir, sizeof ir_full) == 0 &&
               memcmp(mac_full, g_pe_gte.mac, sizeof mac_full) == 0, "full vs short command word differ");
        for (unsigned row = 0; row < 3u; row++) {
            int64_t s = w ? (int64_t)T[row] * 4096 : 0;
            for (unsigned c = 0; c < 3u; c++) s += (int64_t)R[row * 3u + c] * V[c];
            s >>= 12;
            ASSERT(mac_full[row] == (int32_t)s, w ? "rt MAC differs from reference" : "rtir MAC differs from reference");
            ASSERT(ir_full[row] == m348g_sat16(s), w ? "rt IR differs from reference" : "rtir IR differs from reference");
        }
    }
    PASS();
}

/* Pure-C gte_CompMatrix(r1, r2, r3) semantics (register-level, no pe_gte). */
static void m348g_ref_comp(const int16_t r1[9], const int32_t t1[3], const int16_t r2[9],
                           const int32_t t2[3], int16_t r3[9], int32_t t3[3])
{
    for (unsigned c = 0; c < 3u; c++)
        for (unsigned row = 0; row < 3u; row++) {
            int64_t s = 0;
            for (unsigned k = 0; k < 3u; k++) s += (int64_t)r1[row * 3u + k] * r2[k * 3u + c];
            r3[row * 3u + c] = (int16_t)m348g_sat16(s >> 12);
        }
    for (unsigned row = 0; row < 3u; row++) {
        int64_t s = (int64_t)t1[row] * 4096;
        for (unsigned k = 0; k < 3u; k++) s += (int64_t)r1[row * 3u + k] * (int16_t)t2[k];
        t3[row] = m348g_sat16(s >> 12);   /* stlvnl stores IR1-3 (swc2 $9-$11) */
    }
}

static void test_M348G_comp_matrix(void)
{
    static const int16_t A[9] = { 0x0B50, 0, -0x0B50, 0, 0x1000, 0, 0x0B50, 0, 0x0B50 };
    static const int16_t B[9] = { 0x1000, 0, 0, 0, 0x0800, 0x0DDB, 0, -0x0DDB, 0x0800 };
    static const int32_t cases_t1[3][3] = { { 100, -200, 3000 }, { 0, 0, 0 }, { 40000, -1, 30000 } };
    static const int32_t cases_t2[3][3] = { { -300, 1389, 120 }, { 0, 100, -100 }, { 0x7FFF, 0, 0x7FFF } };
    const pe_addr_t a = 0x80140000u, b = 0x80140020u, o = 0x80140040u;
    TEST("M348G_comp_matrix");
    ResetTestState();
    for (unsigned k = 0; k < 3u; k++)
        for (unsigned alias = 0; alias < 2u; alias++) {
            int16_t rr[9]; int32_t rt[3];
            const pe_addr_t out = alias ? b : o;
            m348g_put_matrix(a, A, cases_t1[k]);
            m348g_put_matrix(b, B, cases_t2[k]);
            m348g_ref_comp(A, cases_t1[k], B, cases_t2[k], rr, rt);
            PE_M0348iCompMatrix(a, b, out);
            for (unsigned i = 0; i < 9u; i++)
                ASSERT((int16_t)PE_LoadU16(out + i * 2u) == rr[i], "CompMatrix rotation differs from reference");
            for (unsigned i = 0; i < 3u; i++)
                ASSERT((int32_t)PE_LoadU32(out + 20u + i * 4u) == rt[i], "CompMatrix translation differs from reference");
            for (unsigned i = 0; i < 9u; i++)
                ASSERT(g_pe_gte.rt[i / 3u][i % 3u] == A[i], "CompMatrix must leave RT = r1");
            for (unsigned i = 0; i < 3u; i++)
                ASSERT(g_pe_gte.tr[i] == cases_t1[k][i], "CompMatrix must leave TR = r1.t");
        }
    PASS();
}

static void test_M348G_c4fc4_ring(void)
{
    static const int16_t I[9] = { 0x1000, 0, 0, 0, 0x1000, 0, 0, 0, 0x1000 };
    static const int32_t T0[3] = { 0, 0, 0 }, TZ[3] = { 0, 0, 1024 };
    static const int16_t verts[4][3] = { { 0, 0, 0 }, { 40, 0, 0 }, { 0, -40, 0 }, { 40, -40, 0 } };
    const pe_addr_t cam = 0x80140000u, mat = 0x80140020u, r = 0x80140040u, vtx = 0x80140080u;
    const pe_addr_t arena = 0x80150000u, ot = 0x80160000u;
    const uint32_t idx = (1024u >> 2) + 4u;   /* OTZ + bias */
    TEST("M348G_c4fc4_ring");
    for (unsigned bb = 0; bb < 2u; bb++) {
        ResetTestState();
        g_pe_gte.h = 256; g_pe_gte.ofx = 160 << 16; g_pe_gte.ofy = 120 << 16;
        m348g_put_matrix(cam, I, T0);
        m348g_put_matrix(mat, I, TZ);
        PE_StoreU32(0x800BCFA4u, cam);
        PE_StoreU32(0x8009CDDCu, 0u); PE_StoreU32(0x8009CDD8u, 0u);
        PE_StoreU32(0x800B0E58u, arena); PE_StoreU32(0x800B0E38u, ot);
        PE_StoreU8(0x800E224Cu, 2u); PE_StoreU8(0x800F337Au, 0u);
        for (unsigned i = 0; i < 4u; i++) {
            PE_StoreU16(vtx + i * 8u + 0u, (uint16_t)verts[i][0]);
            PE_StoreU16(vtx + i * 8u + 2u, (uint16_t)verts[i][1]);
            PE_StoreU16(vtx + i * 8u + 4u, (uint16_t)verts[i][2]);
        }
        PE_StoreU32(r + 0u, vtx);
        PE_StoreU32(r + 4u, 0x00112233u);   /* ramp -> c1 (sp+0x30) */
        PE_StoreU32(r + 8u, 0x00445566u);   /* ramp -> c0 (sp+0x28) */
        PE_StoreU16(r + 0xCu, 2u);
        PE_StoreU16(r + 0x12u, 4u);
        PE_StoreU16(r + 0x14u, 0x80u);      /* func_800C608C: straight copy */
        PE_StoreU32(ot + idx * 4u, 0x00ABCDEFu);
        func_800C4FC4(r, mat, bb);
        ASSERT(PE_LoadU32(0x8009CDD8u) == 0x58u, "packet cursor must advance 0x2C per segment");
        for (unsigned s = 0; s < 2u; s++) {
            const pe_addr_t pk = arena + s * 0x2Cu;
            const unsigned k1 = (s + 1u) % 2u, order[4] = { s, k1, 2u + s, 2u + k1 };
            for (unsigned q = 0; q < 4u; q++) {
                const uint32_t sx = (uint32_t)(160 + verts[order[q]][0] / 4);
                const uint32_t sy = (uint32_t)(120 + verts[order[q]][1] / 4);
                ASSERT(PE_LoadU32(pk + 8u + q * 8u) == ((sx & 0xFFFFu) | (sy << 16)), "ring vertex xy / order differs");
            }
            ASSERT(PE_LoadU8(pk + 3u) == 8u && PE_LoadU8(pk + 7u) == 0x38u, "POLY_G4 length/code differs");
            ASSERT((PE_LoadU32(pk + 4u) & 0xFFFFFFu) == 0x445566u && (PE_LoadU32(pk + 0xCu) & 0xFFFFFFu) == 0x445566u,
                   "ring-0 colours must come from r+8");
            ASSERT((PE_LoadU32(pk + 0x14u) & 0xFFFFFFu) == 0x112233u && (PE_LoadU32(pk + 0x1Cu) & 0xFFFFFFu) == 0x112233u,
                   "ring-1 colours must come from r+4");
            ASSERT(PE_LoadU8(pk + 0x27u) == 1u && PE_LoadU32(pk + 0x28u) == (0xE1000000u | (2u << 5)),
                   "draw-mode word differs");
        }
        ASSERT(PE_LoadU32(0x1F80000Cu) == 256u, "OTZ must be SZ3 >> 2");
        /* OT[idx] -> dm1 -> pk1 -> dm0 -> pk0 -> old head */
        ASSERT((PE_LoadU32(ot + idx * 4u) & 0xFFFFFFu) == ((arena + 0x2Cu + 0x24u) & 0xFFFFFFu), "OT head differs");
        ASSERT((PE_LoadU32(arena + 0x2Cu + 0x24u) & 0xFFFFFFu) == ((arena + 0x2Cu) & 0xFFFFFFu), "dm1 link differs");
        ASSERT((PE_LoadU32(arena + 0x2Cu) & 0xFFFFFFu) == ((arena + 0x24u) & 0xFFFFFFu), "pk1 link differs");
        ASSERT((PE_LoadU32(arena + 0x24u) & 0xFFFFFFu) == (arena & 0xFFFFFFu), "dm0 link differs");
        ASSERT((PE_LoadU32(arena) & 0xFFFFFFu) == 0xABCDEFu, "pk0 must link the old OT head");
        if (bb)
            ASSERT((int32_t)PE_LoadU32(mat + 28u) == 1024 && PE_LoadU32(mat + 20u) == 0u,
                   "billboard path must RotTrans the translation in place");
    }
    PASS();
}
