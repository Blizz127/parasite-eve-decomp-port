/*
 * port_absent lane round 2 (2026-09-24) — tests for the lo2 group
 * (pc_port/game/decomp_hand/absent_lo2_port.c).  Expectations are computed
 * from the matched leaf src/func_XXXXXXXX.c named in each block.
 * Guest scratch 0x80164000..0x80167FFF.
 */
#define PA_LO2 0x80164000u

/* Glyph-page data block (the Font record's +0xC `data`), per the leaves:
 *   d[3]         class count      d[4 + k]   class value k
 *   d[28]        slot count       d[29 + i]  class index of slot i
 *   d[129 + i]   column of slot i (func_80039678's p[101 + i], p = d + 28)
 * so cur_attr() = d[d[29 + cur] + 4] is the class value of slot `cur`. */
static void pa_lo2_font(void)
{
    static const unsigned char cls[4] = {5, 2, 3, 24};
    static const unsigned char slot[6] = {0, 1, 2, 2, 3, 1};
    static const unsigned char col[6] = {7, 8, 7, 9, 7, 8};
    const pe_addr_t d = PA_LO2;
    unsigned i;

    PE_StoreU32(0x80091A28u, d);
    PE_StoreU8(d + 3u, 4u);
    for (i = 0; i < 4; i++) PE_StoreU8(d + 4u + i, cls[i]);
    PE_StoreU8(d + 28u, 6u);
    for (i = 0; i < 6; i++) {
        PE_StoreU8(d + 29u + i, slot[i]);
        PE_StoreU8(d + 129u + i, col[i]);
    }
}

static void test_PA_lo2_glyph_cursor(void)
{
    TEST("PA_lo2_glyph_cursor");
    ResetTestState();
    pa_lo2_font();

    /* src/func_80039898.c: class 3 is class index 2; first slot of index 2
     * is slot 2 (attribute 3). */
    PE_StoreU8(0x80091A1Fu, 0x55u);
    ASSERT(func_80039898() == 3u && PE_LoadU8(0x80091A1Fu) == 2u, "39898 class-3 slot");

    /* src/func_80039678.c: 23 = cur + 1, 21 = cur - 1 (byte). */
    ASSERT(func_80039678(23) == 3u && PE_LoadU8(0x80091A1Fu) == 3u, "39678 key 23");
    ASSERT(func_80039678(21) == 3u && PE_LoadU8(0x80091A1Fu) == 2u, "39678 key 21");
    /* 22 = next slot with the same column: from 0 (col 7) -> slot 2. */
    PE_StoreU8(0x80091A1Fu, 0u);
    ASSERT(func_80039678(22) == 3u && PE_LoadU8(0x80091A1Fu) == 2u, "39678 key 22 next col");
    /* 20 = previous slot with the same column, the NEAREST one: from 5
     * (col 8) the walk down stops at slot 1, not 0. */
    PE_StoreU8(0x80091A1Fu, 5u);
    ASSERT(func_80039678(20) == 2u && PE_LoadU8(0x80091A1Fu) == 1u, "39678 key 20 nearest");
    /* no previous match: cursor stays (slot 3, col 9). */
    PE_StoreU8(0x80091A1Fu, 3u);
    ASSERT(func_80039678(20) == 3u && PE_LoadU8(0x80091A1Fu) == 3u, "39678 key 20 no match");
    /* other keys: no move, attribute returned. */
    ASSERT(func_80039678(7) == 3u && PE_LoadU8(0x80091A1Fu) == 3u, "39678 other key");

    /* Mark class (attribute 24, slot 4) + key 20 pages forward via
     * func_80039310, whose key >= 70 guard returns 255 and sets D_80091A20. */
    PE_StoreU8(0x80091A1Fu, 4u);
    PE_StoreU8(0x80091A1Du, 70u);
    PE_StoreU8(0x80091A20u, 0u);
    ASSERT(func_80039678(20) == 255u && PE_LoadU8(0x80091A20u) == 1u &&
           PE_LoadU8(0x80091A1Du) == 70u, "39678 mark+20 -> 39310 guard");
    /* Attribute 2 (slot 1) + key 22 pages back via func_80039184, whose
     * key < 3 guard stores 1 and returns 255. */
    PE_StoreU8(0x80091A1Fu, 1u);
    PE_StoreU8(0x80091A1Du, 2u);
    ASSERT(func_80039678(22) == 255u && PE_LoadU8(0x80091A1Du) == 1u,
           "39678 attr2+22 -> 39184 guard");

    /* src/func_8003944C.c guards: `key - 2 >= 69` is signed, so 71 -> flag,
     * 0 and 1 -> D_80091A1D = 1. */
    PE_StoreU8(0x80091A20u, 0u);
    PE_StoreU8(0x80091A1Du, 9u);
    ASSERT(func_8003944C(71) == 255u && PE_LoadU8(0x80091A20u) == 1u &&
           PE_LoadU8(0x80091A1Du) == 9u, "3944C key 71");
    PE_StoreU8(0x80091A20u, 0u);
    ASSERT(func_8003944C(0) == 255u && PE_LoadU8(0x80091A1Du) == 1u &&
           PE_LoadU8(0x80091A20u) == 0u, "3944C key 0 (signed compare)");
    PE_StoreU8(0x80091A1Du, 9u);
    ASSERT(func_8003944C(1) == 255u && PE_LoadU8(0x80091A1Du) == 1u, "3944C key 1");
    /* func_80039310 / func_80039184 guards directly (both boundaries 69/3). */
    PE_StoreU8(0x80091A1Du, 71u);
    PE_StoreU8(0x80091A20u, 0u);
    ASSERT(func_80039310() == 255u && PE_LoadU8(0x80091A20u) == 1u, "39310 >= 70");
    PE_StoreU8(0x80091A1Du, 0u);
    ASSERT(func_80039184() == 255u && PE_LoadU8(0x80091A1Du) == 1u, "39184 < 3");
    PASS();
}

static void test_PA_lo2_small(void)
{
    TEST("PA_lo2_small");
    ResetTestState();

    /* src/func_8003E0A4.c: (*a0)[2] == 2 -> 3, else 1; int +0x24, shorts
     * +0x28 / +0x2A. */
    {
        const pe_addr_t a0 = PA_LO2 + 0x400u, x = PA_LO2 + 0x500u;
        PE_StoreU32(a0, x);
        PE_StoreU8(x + 2u, 2u);
        func_8003E0A4(a0, -5, 0x12345);
        ASSERT(PE_LoadU32(a0 + 0x24u) == (uint32_t)-5 && PE_LoadU16(a0 + 0x28u) == 3u &&
               PE_LoadU16(a0 + 0x2Au) == 0x2345u, "3E0A4 selector 3 + stores");
        PE_StoreU8(x + 2u, 3u);
        func_8003E0A4(a0, 1, 2);
        ASSERT(PE_LoadU16(a0 + 0x28u) == 1u && PE_LoadU16(a0 + 0x2Au) == 2u, "3E0A4 selector 1");
    }

    /* src/func_8005DAFC.c: base = D_800A8028 + D_800A802C; entry a0 of the
     * u16-counted table = base + (short)base[2 + 2*a0]; a0 == count -> 0. */
    {
        const pe_addr_t base = 0x800A8028u + 0x2000u;
        PE_StoreU32(0x800A802Cu, 0x2000u);
        PE_StoreU16(base, 3u);
        PE_StoreU16(base + 2u, 0x10u);
        PE_StoreU16(base + 4u, 0xFFFCu);             /* signed -4 */
        PE_StoreU16(base + 6u, 0x20u);
        ASSERT((uint32_t)func_8005DAFC(0) == base + 0x10u, "5DAFC entry 0");
        ASSERT((uint32_t)func_8005DAFC(1) == base - 4u, "5DAFC signed offset");
        ASSERT(func_8005DAFC(3) == 0 && func_8005DAFC(0xFFFFFFFFu) == 0, "5DAFC unsigned bound");
    }
    PASS();
}

/* src/func_8004ECB4.c: with neither the confirm (0x10000) nor the cancel
 * (0x40) bit it only reads child 0 of `self` and returns 1.  (func_8004E704 /
 * func_8004E97C / func_80045FA4 need the disc-loaded window templates —
 * func_800647D0 divides by the template's column count — so they are not
 * driven here.) */
static void test_PA_lo2_equip_handler_idle(void)
{
    pe_addr_t win;

    TEST("PA_lo2_equip_handler_idle");
    ResetTestState(); HostFB_Init(); PE_GPU_Init();
    func_80062F9C();
    win = func_80062D2C(0x25u, 0u, 0u, 0u);
    (void)func_8006322C(0x25u, win, win);
    PE_StoreU32(0x8009CF1Cu, 0x77u);
    PE_StoreU32(0x8009CF30u, 0x77u);
    ASSERT(func_8004ECB4(win, 0x20) == 1, "ECB4 returns 1");
    ASSERT(PE_LoadU32(0x8009CF1Cu) == 0x77u && PE_LoadU32(0x8009CF30u) == 0x77u,
           "ECB4 idle key: no case taken");
    ASSERT(!PE_Port_ShouldStop(), "no unresolved stop");
    PASS();
}

static void test_PA_lo2_all(void)
{
    test_PA_lo2_glyph_cursor();
    test_PA_lo2_small();
    test_PA_lo2_equip_handler_idle();
}
