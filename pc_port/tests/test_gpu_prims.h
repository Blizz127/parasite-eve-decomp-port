/* GFX1 — one decode check per GP0 primitive class against psx-spx
 * ("GPU Render Polygon/Line/Rectangle Commands", "GPU Memory Transfer
 * Commands", "GPU Display/Rendering Control").  Each case feeds a known
 * packet to the native GP0 decoder and checks the exact pixels it
 * touches (extent, color, blend) and the parser returning to idle.
 * Included once from test_native.c (needs its TEST/ASSERT/B54KR helpers). */

static int GFX1_Words(const uint32_t *w, unsigned n)
{
    unsigned i;
    for (i = 0; i < n; i++) if (!PE_GPU_WriteGP0(w[i])) return 0;
    return 1;
}

static uint16_t GFX1_Pixel(uint32_t x, uint32_t y)
{
    uint16_t p = 0xFFFFu;
    (void)PE_GPU_ReadVRAM(x, y, &p);
    return p;
}

static int GFX1_Idle(void)
{
    PeGpuState s;
    PE_GPU_GetState(&s);
    return s.gp0_state == PE_GPU_GP0_IDLE;
}

/* 4bpp index-1 texture block at (0,0) (u 0..15, v 0..15), 8bpp index-2
 * texel at page 1 (64,0), 15bpp texel at page 2 (128,0); CLUT at (16,10)
 * = id 0x281 with [1]=green, [2]=blue, [3]=white. */
static int GFX1_SeedTextures(void)
{
    uint16_t block[4 * 16], palette[16] = {0u}, eight = 0x0202u, direct = 0x7C00u;
    unsigned i;
    for (i = 0; i < 4u * 16u; i++) block[i] = 0x1111u;
    palette[1] = 0x03E0u; palette[2] = 0x7C00u; palette[3] = 0x7FFFu;
    return B54KR_UploadPixels(0u, 0u, 4u, 16u, block) &&
           B54KR_UploadPixels(16u, 10u, 16u, 1u, palette) &&
           B54KR_UploadPixels(64u, 0u, 1u, 1u, &eight) &&
           B54KR_UploadPixels(128u, 0u, 1u, 1u, &direct);
}

static void test_GFX1_gp0_polygons(void)
{
    static const uint32_t flat_tri[] = {0x200000F8u, (100u<<16)|100u, (100u<<16)|110u, (110u<<16)|100u};
    static const uint32_t gouraud[] = {0x300000F8u, (300u<<16)|0u, 0x0000F800u, (300u<<16)|40u,
                                       0x00F80000u, (340u<<16)|0u};
    uint32_t ft4[] = {0x2C808080u, (0u<<16)|400u, 0x02810000u, (0u<<16)|404u, 0x00000003u,
                      (4u<<16)|400u, 0x00000300u, (4u<<16)|404u, 0x00000303u};
    static const unsigned abr_expect[4] = {23u, 31u, 0u, 23u};
    PeGpuState state;
    unsigned abr;
    TEST("GFX1_gp0_polygons");
    ResetTestState(); PE_GPU_Init();
    ASSERT(PE_GPU_GP0_PacketWords(0x20000000u) == 4u && PE_GPU_GP0_PacketWords(0x28000000u) == 5u &&
           PE_GPU_GP0_PacketWords(0x2C000000u) == 9u && PE_GPU_GP0_PacketWords(0x30000000u) == 6u &&
           PE_GPU_GP0_PacketWords(0x38000000u) == 8u && PE_GPU_GP0_PacketWords(0x3C000000u) == 12u &&
           PE_GPU_GP0_PacketWords(0x34000000u) == 9u, "polygon packet lengths");
    ASSERT(GFX1_Words(flat_tri, 4u) && GFX1_Idle(), "F3 accepted");
    ASSERT(GFX1_Pixel(101u, 101u) == 0x001Fu && GFX1_Pixel(108u, 101u) == 0x001Fu &&
           GFX1_Pixel(109u, 109u) == 0u && GFX1_Pixel(111u, 100u) == 0u, "F3 coverage");
    /* Semi-transparent F4 over a (16,0,0) fill, every ABR mode. */
    for (abr = 0; abr < 4u; abr++) {
        uint32_t x = 208u + abr * 16u;
        uint32_t fill[] = {0x02000080u, x, (8u<<16)|16u};
        uint32_t quad[] = {0x2A0000F8u, x, x + 8u, (8u<<16)|x, (8u<<16)|(x + 8u)};
        ASSERT(GFX1_Words(fill, 3u) && PE_GPU_WriteGP0(0xE1000000u | (abr << 5)) &&
               GFX1_Words(quad, 5u), "F4 semi packet");
        ASSERT(GFX1_Pixel(x + 3u, 3u) == abr_expect[abr] && GFX1_Pixel(x + 9u, 3u) == 16u,
               "ABR 0-3: B/2+F/2, B+F, B-F, B+F/4 (and only inside the quad)");
    }
    ASSERT(PE_GPU_WriteGP0(0xE1000000u) && GFX1_Words(gouraud, 6u) && GFX1_Idle(), "G3 accepted");
    {
        uint16_t p = GFX1_Pixel(1u, 301u);
        ASSERT((p & 31u) >= 28u && ((p >> 5) & 31u) <= 3u && ((p >> 10) & 31u) <= 3u,
               "G3 interpolates from the v0 color");
    }
    ASSERT(GFX1_SeedTextures(), "seed textures");
    ASSERT(GFX1_Words(ft4, 9u) && GFX1_Pixel(401u, 1u) == 0x03E0u && GFX1_Pixel(404u, 1u) == 0u,
           "FT4 4bpp modulation 0x80 is identity; right edge excluded");
    PE_GPU_GetState(&state);
    ASSERT((state.draw_mode & 0x1FFu) == 0u, "FT4 uv1 word loads the texpage");
    ft4[0] = 0x2D000000u; ft4[1] = (0u<<16)|410u; ft4[3] = (0u<<16)|414u;
    ft4[5] = (4u<<16)|410u; ft4[7] = (4u<<16)|414u;
    ASSERT(GFX1_Words(ft4, 9u) && GFX1_Pixel(411u, 1u) == 0x03E0u, "raw FT4 ignores the color");
    ft4[0] = 0x2C808080u; ft4[1] = (0u<<16)|420u; ft4[3] = (0u<<16)|422u;
    ft4[5] = (2u<<16)|420u; ft4[7] = (2u<<16)|422u;
    ft4[4] = 0x00810000u;           /* page 1, 8bpp */
    ft4[2] = 0x02810000u; ft4[6] = 0u; ft4[8] = 0u;
    ASSERT(GFX1_Words(ft4, 9u) && GFX1_Pixel(420u, 0u) == 0x7C00u, "FT4 8bpp CLUT lookup");
    ft4[4] = 0x01020000u;           /* page 2, 15bpp */
    ft4[1] = (0u<<16)|430u; ft4[3] = (0u<<16)|432u; ft4[5] = (2u<<16)|430u; ft4[7] = (2u<<16)|432u;
    ASSERT(GFX1_Words(ft4, 9u) && GFX1_Pixel(430u, 0u) == 0x7C00u, "FT4 15bpp direct texel");
    PE_GPU_GetState(&state);
    ASSERT((state.draw_mode & 0x1FFu) == 0x102u && GFX1_Idle(), "texpage from the last polygon");
    PASS();
}

static void test_GFX1_gp0_lines_and_polylines(void)
{
    static const uint32_t poly[] = {0x480000F8u, (300u<<16)|10u, (300u<<16)|14u, (304u<<16)|14u, 0x55555555u};
    static const uint32_t gpoly[] = {0x580000F8u, (310u<<16)|10u, 0x0000F800u, (310u<<16)|14u,
                                     0x00F80000u, (314u<<16)|14u, 0x50005000u};
    static const uint32_t after[] = {0x6000F800u, (320u<<16)|10u, (1u<<16)|1u};
    uint32_t stream[8];
    TEST("GFX1_gp0_lines_and_polylines");
    ResetTestState(); PE_GPU_Init();
    memcpy(stream, poly, sizeof poly);
    ASSERT(PE_GPU_GP0_StreamPacketWords(stream, 5u) == 5u &&
           PE_GPU_GP0_StreamPacketWords(stream, 4u) == 0u, "flat polyline runs to its terminator");
    memcpy(stream, gpoly, sizeof gpoly);
    ASSERT(PE_GPU_GP0_StreamPacketWords(stream, 7u) == 7u, "shaded polyline length");
    ASSERT(GFX1_Words(poly, 5u) && GFX1_Idle(), "flat polyline accepted, parser idle after 55555555h");
    ASSERT(GFX1_Pixel(10u, 300u) == 0x1Fu && GFX1_Pixel(12u, 300u) == 0x1Fu &&
           GFX1_Pixel(14u, 302u) == 0x1Fu && GFX1_Pixel(14u, 304u) == 0x1Fu &&
           GFX1_Pixel(12u, 302u) == 0u, "both polyline segments drawn, nothing else");
    ASSERT(GFX1_Words(gpoly, 7u) && GFX1_Idle(), "shaded polyline, 50005000h terminator");
    ASSERT(GFX1_Pixel(10u, 310u) == 0x1Fu && GFX1_Pixel(14u, 310u) == 0x03E0u &&
           GFX1_Pixel(14u, 314u) == 0x7C00u, "per-vertex colors");
    ASSERT(GFX1_Words(after, 3u) && GFX1_Pixel(10u, 320u) == 0x03E0u, "next command after a polyline");
    PASS();
}

static void test_GFX1_gp0_rectangles(void)
{
    static const struct { uint32_t op, x, w, h; } mono[] = {
        {0x60u, 500u, 3u, 2u}, {0x68u, 510u, 1u, 1u}, {0x70u, 520u, 8u, 8u}, {0x78u, 540u, 16u, 16u}};
    unsigned i;
    TEST("GFX1_gp0_rectangles");
    ResetTestState(); PE_GPU_Init();
    ASSERT(PE_GPU_GP0_PacketWords(0x60000000u) == 3u && PE_GPU_GP0_PacketWords(0x68000000u) == 2u &&
           PE_GPU_GP0_PacketWords(0x70000000u) == 2u && PE_GPU_GP0_PacketWords(0x78000000u) == 2u &&
           PE_GPU_GP0_PacketWords(0x64000000u) == 4u && PE_GPU_GP0_PacketWords(0x6C000000u) == 3u &&
           PE_GPU_GP0_PacketWords(0x74000000u) == 3u && PE_GPU_GP0_PacketWords(0x7C000000u) == 3u,
           "rectangle packet lengths for every size class");
    for (i = 0; i < 4u; i++) {
        uint32_t w[3] = {(mono[i].op << 24) | 0x0000F8u, mono[i].x, (mono[i].h << 16) | mono[i].w};
        ASSERT(GFX1_Words(w, mono[i].op == 0x60u ? 3u : 2u) && GFX1_Idle(), "TILE accepted");
        ASSERT(GFX1_Pixel(mono[i].x + mono[i].w - 1u, mono[i].h - 1u) == 0x1Fu &&
               GFX1_Pixel(mono[i].x + mono[i].w, mono[i].h - 1u) == 0u &&
               GFX1_Pixel(mono[i].x + mono[i].w - 1u, mono[i].h) == 0u, "TILE exact extent");
    }
    ASSERT(GFX1_SeedTextures(), "seed textures");
    for (i = 0; i < 4u; i++) {
        uint32_t op = 0x64u + i * 8u, x = 600u + i * 20u;
        uint32_t w = op == 0x64u ? 3u : op == 0x6Cu ? 1u : op == 0x74u ? 8u : 16u;
        uint32_t h = op == 0x64u ? 2u : w;
        uint32_t pk[4] = {(op << 24) | 0x808080u, x, 0x02810000u, (h << 16) | w};
        ASSERT(PE_GPU_WriteGP0(0xE1000000u) && GFX1_Words(pk, op == 0x64u ? 4u : 3u) && GFX1_Idle(),
               "SPRT accepted (variable, 1x1, 8x8, 16x16)");
        ASSERT(GFX1_Pixel(x + w - 1u, h - 1u) == 0x03E0u && GFX1_Pixel(x + w, h - 1u) == 0u &&
               GFX1_Pixel(x + w - 1u, h) == 0u, "SPRT exact extent and texel");
    }
    {
        uint32_t pk[4] = {0x65000000u, 700u, 0u, (1u << 16) | 1u};
        ASSERT(PE_GPU_WriteGP0(0xE1000102u) && GFX1_Words(pk, 4u) && GFX1_Pixel(700u, 0u) == 0x7C00u,
               "15bpp raw SPRT");
    }
    {
        /* Texture window: mask 8px, offset 8 -> u 0..7 reads u 8..15. */
        uint16_t three = 0x3333u;
        uint32_t pk[3] = {0x75808080u, 720u, 0x02810000u};
        ASSERT(B54KR_UploadPixels(2u, 0u, 1u, 1u, &three) && PE_GPU_WriteGP0(0xE1000000u) &&
               PE_GPU_WriteGP0(0xE2000000u | (1u << 10) | 1u) && GFX1_Words(pk, 3u) &&
               GFX1_Pixel(720u, 0u) == 0x7FFFu && GFX1_Pixel(720u, 1u) == 0x03E0u,
               "E2 texture window remaps u (v unmasked)");
        ASSERT(PE_GPU_WriteGP0(0xE2000000u), "clear window");
    }
    PASS();
}

static void test_GFX1_gp0_fill_transfer_and_environment(void)
{
    static const uint32_t fill[] = {0x020000F8u, (20u<<16)|803u, (1u<<16)|5u};
    uint16_t src[2] = {0x1234u, 0x5678u};
    uint32_t word = 0u;
    TEST("GFX1_gp0_fill_transfer_and_environment");
    ResetTestState(); PE_GPU_Init();
    ASSERT(PE_GPU_WriteGP0(0xE6000003u) && PE_GPU_WriteGP0(0xE5000000u | (5u << 11) | 5u) &&
           GFX1_Words(fill, 3u), "fill accepted");
    ASSERT(GFX1_Pixel(800u, 20u) == 0x1Fu && GFX1_Pixel(815u, 20u) == 0x1Fu &&
           GFX1_Pixel(816u, 20u) == 0u && GFX1_Pixel(800u, 21u) == 0u,
           "fill: x rounded to 16, width to 16, ignores offset and mask");
    ASSERT(PE_GPU_WriteGP0(0xE6000000u) && PE_GPU_WriteGP0(0xE5000000u), "reset env");
    ASSERT(B54KR_UploadPixels(900u, 0u, 2u, 1u, src) && GFX1_Pixel(901u, 0u) == 0x5678u, "A0 upload");
    ASSERT(PE_GPU_WriteGP0(0xC0000000u) && PE_GPU_WriteGP0(900u) && PE_GPU_WriteGP0((1u<<16)|2u) &&
           PE_GPU_ReadGP0(&word) && word == 0x56781234u && GFX1_Idle(), "C0 readback");
    ASSERT(PE_GPU_WriteGP1(0x04000002u) && PE_GPU_MoveImage(900u, (5u<<16)|900u, (1u<<16)|2u) &&
           GFX1_Pixel(900u, 5u) == 0x1234u && GFX1_Pixel(901u, 5u) == 0x5678u, "80h VRAM copy");
    ASSERT(PE_GPU_WriteGP1(0x04000000u) && PE_GPU_WriteGP0(0x01000000u), "parser reset");
    /* Offset + drawing area + mask set/check on a flat tile. */
    ASSERT(PE_GPU_WriteGP0(0xE3000000u | (40u << 10) | 40u) &&
           PE_GPU_WriteGP0(0xE4000000u | (41u << 10) | 41u) &&
           PE_GPU_WriteGP0(0xE5000000u | (40u << 11) | 40u) &&
           PE_GPU_WriteGP0(0xE6000001u) && PE_GPU_WriteGP0(0x780000F8u) && PE_GPU_WriteGP0(0u),
           "tile with offset/area/mask-set");
    ASSERT(GFX1_Pixel(40u, 40u) == 0x801Fu && GFX1_Pixel(41u, 41u) == 0x801Fu &&
           GFX1_Pixel(42u, 41u) == 0u && GFX1_Pixel(39u, 40u) == 0u, "offset, inclusive area, mask bit");
    ASSERT(PE_GPU_WriteGP0(0xE6000002u) && PE_GPU_WriteGP0(0x7800F800u) && PE_GPU_WriteGP0(0u) &&
           GFX1_Pixel(40u, 40u) == 0x801Fu, "mask check preserves masked pixels");
    /* Dither applies to shaded/textured polygons only, not flat ones. */
    ASSERT(PE_GPU_WriteGP0(0xE6000000u) && PE_GPU_WriteGP0(0xE3000000u) &&
           PE_GPU_WriteGP0(0xE4000000u | (511u << 10) | 1023u) && PE_GPU_WriteGP0(0xE5000000u) &&
           PE_GPU_WriteGP0(0xE1000200u) && PE_GPU_WriteGP0(0x20848484u) &&
           PE_GPU_WriteGP0((60u<<16)|60u) && PE_GPU_WriteGP0((60u<<16)|68u) && PE_GPU_WriteGP0((68u<<16)|60u),
           "dithered flat F3");
    ASSERT(GFX1_Pixel(61u, 61u) == 0x4210u && GFX1_Pixel(62u, 61u) == 0x4210u &&
           GFX1_Pixel(61u, 62u) == 0x4210u && GFX1_Pixel(62u, 62u) == 0x4210u,
           "flat polygons are never dithered");
    PASS();
}

static void test_GFX1_gp0_primitives(void)
{
    test_GFX1_gp0_polygons();
    test_GFX1_gp0_lines_and_polylines();
    test_GFX1_gp0_rectangles();
    test_GFX1_gp0_fill_transfer_and_environment();
}
