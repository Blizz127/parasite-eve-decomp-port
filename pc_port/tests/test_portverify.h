/*
 * portverify regressions: hand ports that disagreed with the matching decomp.
 *
 * Each case below was found by the differential harness (generated TU vs the
 * pre-existing hand port, forked per trial on identical seeded guest RAM) and
 * confirmed against the retail disassembly (tools/analysis/pe_dis.sh).  The
 * matched C in src/ is the authority; these pin its behaviour so a future hand
 * edit cannot silently reintroduce the old divergence.  Record:
 * docs/evidence/portverify/REPORT.md.
 */

#include "pe_guest_decomp.h"   /* PE_Decomp_* boundary log, hand-adapter protos */

static void pv_seed_vm_task(pe_addr_t task, pe_addr_t actor)
{
    PE_StoreU32(0x8009D300u, task);
    PE_StoreU32(0x8009D2F0u, actor);
    PE_StoreU32(0x8009CE00u, 0x8012300Cu);
}

static void test_PV_661EC_mask_and_mode(void)
{
    TEST("PV_661EC_mask_and_mode");
    /* Retail 0x80066220 `li v0,-16` (mask ~0xF keeps bits 31..4) and
     * 0x8006624C `bne a3,8` -> `ori 1`, fall-through `ori 9`. */
    ResetTestState();
    PE_StoreU32(0x800BCF88u, 0xABCD0047u);
    ASSERT(func_800661EC(1, 2, 3, 8) == 0, "accepted");
    ASSERT(PE_LoadU32(0x800BCF88u) == 0xABCD0049u, "a3 == 8 -> low nibble 9, high bits kept");
    PE_StoreU32(0x800BCF88u, 0xABCD0047u);
    ASSERT(func_800661EC(1, 2, 3, 3) == 0, "accepted");
    ASSERT(PE_LoadU32(0x800BCF88u) == 0xABCD0041u, "a3 != 8 -> low nibble 1, high bits kept");
    PASS();
}

static void test_PV_19B08_bit8_polarity(void)
{
    const pe_addr_t task = 0x80108700u, actor = 0x80108800u;
    TEST("PV_19B08_bit8_polarity");
    /* Retail 0x80019B30..0x80019B4C: armed (task+8 & 0x20) and actor+0x98
     * bit 8 SET -> clear 0x20 and return 1; bit 8 clear -> yield 8. */
    ResetTestState();
    pv_seed_vm_task(task, actor);
    PE_StoreU16(task + 8u, 0x20u);
    PE_StoreU32(actor + 0x98u, 8u);
    ASSERT(func_80019B08() == 1, "bit 8 set completes");
    ASSERT((PE_LoadU16(task + 8u) & 0x20u) == 0u, "armed bit cleared");
    ASSERT(PE_LoadU32(0x8009CE00u) == 0x8012300Cu, "no rewind on completion");
    PE_StoreU16(task + 8u, 0x20u);
    PE_StoreU32(actor + 0x98u, 0u);
    ASSERT(func_80019B08() == 0, "bit 8 clear waits");
    ASSERT(PE_LoadU32(0x8009CE00u) == 0x80123004u, "wait rewinds 8");
    PASS();
}

static void test_PV_144FC_out_of_table_completes(void)
{
    static const uint8_t states[] = {0x3Cu, 0x80u, 0xFFu};
    const pe_addr_t task = 0x80108700u;
    unsigned i;
    TEST("PV_144FC_out_of_table_completes");
    /* Retail 0x80014520 `sltiu v0,v1,60` / `beqz -> 0x80014658`
     * (`j 0x8001467C`, `li v0,1`). */
    for (i = 0; i < sizeof(states); i++) {
        ResetTestState();
        pv_seed_vm_task(task, 0x80108800u);
        PE_StoreU8(0x800B0CD8u + 0xF4u, states[i]);
        ASSERT(func_800144FC(0x80120F80u) == 1, "out-of-table state returns 1");
        ASSERT(PE_LoadU32(0x8009CE00u) == 0x8012300Cu, "no yield rewind");
        ASSERT(PE_LoadU32(task + 0x10u) == 0u, "task not parked");
    }
    PASS();
}

static void test_PV_4F464_clears_flag(void)
{
    TEST("PV_4F464_clears_flag");
    /* src/func_8004F464.c: call func_8004E97C then D_8009D008 = 0. */
    ResetTestState();
    PE_StoreU32(0x8009D008u, 5u);
    func_8004F464();
    ASSERT(PE_LoadU32(0x8009D008u) == 0u, "pending word cleared after the call");
    PASS();
}

static void test_PV_6B35C_plus6_halfwords(void)
{
    unsigned i;
    TEST("PV_6B35C_plus6_halfwords");
    /* src/func_8006B35C.c: for 4 records of 8 bytes, zero +6 (from
     * D_80094488) and +4 (D_8009448C); +0/+2 are untouched. */
    ResetTestState();
    for (i = 0; i < 0x20u; i += 2u)
        PE_StoreU16(0x80094488u + i, 0xFFFFu);
    func_8006B35C();
    for (i = 0; i < 4u; i++) {
        pe_addr_t r = 0x80094488u + i * 8u;
        ASSERT(PE_LoadU16(r + 6u) == 0u, "+6 halfword cleared");
        ASSERT(PE_LoadU16(r + 4u) == 0u, "+4 halfword cleared");
        ASSERT(PE_LoadU16(r + 0u) == 0xFFFFu && PE_LoadU16(r + 2u) == 0xFFFFu,
               "+0/+2 untouched");
    }
    PASS();
}

static void test_PV_6BE4C_previous_half(void)
{
    TEST("PV_6BE4C_previous_half");
    /* Retail 0x8006BE94: right = (CE3 - 10) / 2 (the old port dropped -10). */
    ResetTestState();
    PE_StoreU8(0x800B0CE2u, 12u);
    PE_StoreU8(0x800B0CE3u, 13u);            /* (12-10)>>1 == (13-10)/2 == 1 */
    ASSERT(func_8006BE4C() == 0, "v0");
    ASSERT((PE_LoadU8(0x800B0CE6u) & 4u) == 0u, "equal halves leave CE6");
    PE_StoreU8(0x800B0CE3u, 0u);             /* 1 vs -5 */
    ASSERT(func_8006BE4C() == 0, "v0");
    ASSERT((PE_LoadU8(0x800B0CE6u) & 4u) == 4u, "different halves set CE6 bit 2");
    PASS();
}

static void pv_seed_colour_container(void)
{
    const pe_addr_t c = 0x80140000u, e = 0x80140100u, q = 0x80141000u;
    PE_StoreU32(0x800B1624u, c);
    PE_StoreU16(c + 6u, 1u);                 /* one record */
    PE_StoreU32(c + 0x14u, 0x100u);          /* records at c + 0x100 */
    PE_StoreU32(e + 0x30u, q);
    PE_StoreU16(e + 0x26u, 2u);              /* two 16-byte entries */
}

static void test_PV_67B74_fade_step(void)
{
    const pe_addr_t q = 0x80141000u;
    TEST("PV_67B74_fade_step");
    /* src/func_80067B74.c; the old hand port returned without doing anything. */
    ResetTestState();
    pv_seed_colour_container();
    PE_StoreU32(0x800BCF88u, 0x400u);
    PE_StoreU8(0x800BCFFBu, 2u);
    PE_StoreU8(0x800BCFFAu, 5u);             /* d = (2 << 7) / 4 = 64, col 0x40 */
    ASSERT(func_80067B74() == 0, "v0");
    ASSERT(PE_LoadU8(q + 4u) == 0x40u && PE_LoadU8(q + 5u) == 0x40u &&
           PE_LoadU8(q + 6u) == 0x40u && PE_LoadU8(q + 0x16u) == 0x40u,
           "both entries coloured");
    ASSERT(PE_LoadU8(0x800BCFFBu) == 3u, "step counter advanced");
    ASSERT(PE_LoadU32(0x800BCF88u) == 0x400u, "not finished yet");
    PE_StoreU8(0x800BCFFBu, 4u);             /* t = 5 >= 5 finishes */
    PE_StoreU32(0x8009CDDCu, 1u);            /* entries offset by c << 4 */
    ASSERT(func_80067B74() == 0, "v0");
    ASSERT(PE_LoadU8(q + 0x20u + 4u) == (uint8_t)(0x80 - (4 << 7) / 4), "offset entry coloured");
    ASSERT(PE_LoadU32(0x800BCF88u) == 0x800u, "0x400 phase -> 0x800");
    ASSERT(PE_LoadU16(0x80140026u) == 0x1FF0u, "container +0x26 = 0x1FF0");
    PASS();
}

static void test_PV_67D18_hold_phase(void)
{
    const pe_addr_t q = 0x80141000u;
    TEST("PV_67D18_hold_phase");
    /* src/func_80067D18.c; the old hand port returned without doing anything. */
    ResetTestState();
    pv_seed_colour_container();
    PE_StoreU32(0x800BCF88u, 0x1000u | 0x2000u | 0x4000u);
    PE_StoreU8(0x800BCFFCu, 0x33u);
    ASSERT(func_80067D18() == 0, "v0");
    ASSERT(PE_LoadU8(q + 4u) == 0x33u && PE_LoadU8(q + 0x16u) == 0x33u, "held colour");
    ASSERT(PE_LoadU32(0x800BCF88u) == 0xB000u, "0x4000 -> clear 0xC000, set 0x8000");
    PE_StoreU32(0x800BCF88u, 0x1000u | 0x8000u);
    ASSERT(func_80067D18() == 0, "v0");
    ASSERT(PE_LoadU8(q + 4u) == 0x80u, "default colour 0x80");
    ASSERT(PE_LoadU32(0x800BCF88u) == 0u, "0x8000 -> clear 0xC000 and 0x1000");
    PASS();
}

static void test_PV_52BCC_unsigned_char_head(void)
{
    const pe_addr_t dst = 0x80140000u, src = 0x80140100u;
    TEST("PV_52BCC_unsigned_char_head");
    /* src/func_80052BCC.c keeps the head byte in plain `char`, which is
     * unsigned in the era compiler (retail `lbu`/`andi 0xFF`); decomp TUs
     * are built with -funsigned-char.  A signed host char missed the 0xFF
     * head terminator (retail oracle INV8 case 80). */
    ResetTestState();
    PE_StoreU8(src, 0xFFu);
    PE_StoreU8(src + 1u, 0x11u);
    PE_StoreU8(src + 2u, 0xFFu);
    PE_StoreU8(dst + 1u, 0xAAu);
    func_80052BCC(dst, src);
    ASSERT(PE_LoadU8(dst) == 0xFFu, "terminator copied");
    ASSERT(PE_LoadU8(dst + 1u) == 0xAAu, "copy stops at the head terminator");
    PASS();
}


static int pv_load_aborts(pe_addr_t a)
{
    int status = 0;
    pid_t pid;
    fflush(NULL);
    pid = fork();
    if (pid == 0) {
        (void)freopen("/dev/null", "w", stderr);
        (void)PE_LoadU32(a);
        _exit(0);
    }
    if (pid < 0 || waitpid(pid, &status, 0) != pid) return -1;
    return WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT;
}

static void test_PV_ram_mirror(void)
{
    TEST("PV_ram_mirror");
    /* PS1 main RAM is visible at KUSEG 0x00000000, KSEG0 0x80000000 and
     * KSEG1 0xA0000000 (psx-spx memory map); retail data carries KUSEG
     * aliases, e.g. func_80064C20(list = 0) writes 0x00000070. */
    ResetTestState();
    PE_StoreU32(0x00001234u, 0xCAFEF00Du);
    ASSERT(PE_LoadU32(0x80001234u) == 0xCAFEF00Du, "KUSEG store lands in KSEG0");
    ASSERT(PE_LoadU32(0xA0001234u) == 0xCAFEF00Du, "KSEG1 alias reads the same RAM");
    PE_StoreU16(0xA01FFFFEu, 0xBEEFu);
    ASSERT(PE_LoadU16(0x801FFFFEu) == 0xBEEFu, "last KSEG1 halfword");
    ASSERT(PE_RamCanonical(0x001FFFFCu, 4u) == 0x801FFFFCu, "last KUSEG word maps");
    ASSERT(PE_RamCanonical(0x001FFFFEu, 4u) == 0x001FFFFEu, "range crossing 2 MiB is not folded");
    ASSERT(PE_RamCanonical(0x00200000u, 1u) == 0x00200000u, "beyond 2 MiB is not RAM");
    ASSERT(PE_RamCanonical(0x1F800000u, 4u) == 0x1F800000u, "scratchpad is not a RAM alias");
    ASSERT(!PE_AddressIsRam(0x00001234u), "PE_AddressIsRam stays KSEG0-only");
#if !defined(__SANITIZE_ADDRESS__)
    ASSERT(pv_load_aborts(0x00200000u) == 1, "0x00200000 still aborts");
    ASSERT(pv_load_aborts(0x001FFFFEu) == 1, "a word crossing the mirror end aborts");
    ASSERT(pv_load_aborts(0xA0200000u) == 1, "KSEG1 beyond 2 MiB aborts");
#endif
    PASS();
}


static void test_PV_52F70_signed_cap(void)
{
    TEST("PV_52F70_signed_cap");
    /* Retail 0x80052F94 `slti v1,v1,51`: the cap test is signed. */
    ResetTestState();
    PE_StoreU8(0x800C0E0Cu, 3u);
    D_8009D018 = (uint32_t)-10;
    ASSERT(func_80052F70() == (unsigned int)-7, "negative sum takes the add path, not the cap");
    D_8009D018 = 60u;
    ASSERT(func_80052F70() == 50u, "sum >= 51 caps at 50");
    D_8009D018 = 0u;
    PASS();
}

static void test_PV_1F814_null_actor_latch(void)
{
    TEST("PV_1F814_null_actor_latch");
    /* src/func_8001F814.c (retail 0x8001F814): no null test on D_8009D254
     * or D_8009D278.  A zero Aya pointer reads her command byte (+0x0E)
     * and frame byte (+0x0F) from KUSEG low RAM and latches them.  The old
     * hand port returned 0 before the latch (and on a zero actor).
     * Round 7: hand port retired for the generated TU.  The 0x12000 state
     * bit makes the leaf return right after the latch. */
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    PE_StoreU32(0x8009D254u, 0u);
    PE_StoreU32(0x8009D278u, 0x80160000u);
    PE_StoreU32(0x8016004Cu, 0x2000u);
    PE_StoreU8(0x8000000Eu, 7u);
    PE_StoreU8(0x8000000Fu, 3u);
    PE_StoreU16(0x8009D298u, 0u);
    (void)func_8001F814(0u);
    ASSERT(PE_LoadU8(0x8009D29Au) == 7u && PE_LoadU8(0x8009D29Bu) == 3u,
           "command/frame latched from low RAM");
    ASSERT(PE_LoadU16(0x8009D298u) == 1u && PE_LoadU32(0x8009D29Cu) == (3u << 16),
           "kind 7: D_8009D298 = 1, D_8009D29C = frame << 16");
    ASSERT(PE_Decomp_BoundaryCount() == 0, "no boundary");
    PASS();
}

static void test_PV_67A78_null_header_low_ram(void)
{
    TEST("PV_67A78_null_header_low_ram");
    /* src/func_80067A78.c (retail 0x80067A78): no null test on D_800B1624.
     * A zero header pointer rebases through KUSEG low RAM (0x00000038/3A ==
     * 0x80000038/3A); the old hand port returned early and wrote nothing.
     * Round 6: hand port retired for the generated TU. */
    ResetTestState();
    PE_StoreU32(0x800B1624u, 0u);
    PE_StoreU16(0x8000002Cu, 0x0120u);
    PE_StoreU16(0x8000002Eu, 0x0090u);
    PE_StoreU16(0x800BCF8Cu, 0x00B0u);       /* -> (short)(0xB0 - 0xA0) = 0x10 */
    PE_StoreU16(0x800BCF8Eu, 0x0070u);       /* -> 0 */
    PE_StoreU16(0x80000006u, 0u);            /* entry count 0: no walk */
    PE_StoreU32(0x80000014u, 0u);
    func_80067A78();
    ASSERT(PE_LoadU16(0x80000038u) == 0x0110u, "+0x38 = +0x2C - (CF8C - 0xA0)");
    ASSERT(PE_LoadU16(0x8000003Au) == 0x0090u, "+0x3A = +0x2E - (CF8E - 0x70)");
    PASS();
}

/* Immediate libgpu entry points (src/func_800775E8.c, func_800777C0.c):
 * previously absent from the port.  Same GPU authority as the queued
 * wrappers (B54KR/B54KS); the rectangle is a guest address. */
static void test_PV_777C0_immediate_moveimage(void)
{
    static const uint16_t pixels[] = {
        0x101u, 0x102u, 0x103u, 0x104u,
        0x201u, 0x202u, 0x203u, 0x204u
    };
    const pe_addr_t rect = 0x80170100u;
    PeGpuState state;
    TEST_RETAIL_DISC1("PV_777C0_immediate_moveimage"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    ResetTestState();
    B54KR_SeedGpuStatic();
    ASSERT(B54KR_UploadPixels(10u, 20u, 4u, 2u, pixels), "source upload failed");
    PE_StoreU32(rect, 0x0014000Au);           /* x=10, y=20 */
    PE_StoreU32(rect + 4u, 0x00020004u);      /* w=4, h=2 */
    ASSERT(func_800777C0(rect, 100, 30) == 0 && !PE_Port_ShouldStop(),
           "immediate MoveImage did not return 0");
    ASSERT(PE_LoadU32(0x800957ECu) == 0x0014000Au &&
           PE_LoadU32(0x800957F0u) == 0x001E0064u &&
           PE_LoadU32(0x800957F4u) == 0x00020004u,
           "packet xy/col/wh differ");
    ASSERT(B54KR_PixelIs(100u, 30u, 0x101u) && B54KR_PixelIs(103u, 31u, 0x204u),
           "moved pixels differ");
    ASSERT(PE_LoadU32(PE_DMA_CallbackSlotAddress(2u)) == 0x80077A00u,
           "DMA2 callback slot not func_80077A00");
    ASSERT(PE_LoadU32(0x8009588Cu) == 0u &&
           PE_LoadU32(0x80095888u) == PE_GPU_VSyncQuery() + 0xF0u,
           "deadline preamble differs");
    PE_GPU_GetState(&state);
    ASSERT(state.move_count == 1u, "not exactly one move");

    /* Zero height: -1 after the wait + callback install, packet untouched. */
    PE_StoreU32(0x800957ECu, 0xA5A5A5A5u);
    PE_StoreU32(rect + 4u, 0x00000004u);
    ASSERT(func_800777C0(rect, 1, 2) == -1 &&
           PE_LoadU32(0x800957ECu) == 0xA5A5A5A5u && !PE_Port_ShouldStop(),
           "zero-size immediate MoveImage submitted");
    PASS();
}

static void test_PV_775E8_immediate_loadimage(void)
{
    const pe_addr_t rect = 0x80170100u;
    uint32_t i;
    TEST_RETAIL_DISC1("PV_775E8_immediate_loadimage"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    ResetTestState();
    HostFB_Init();
    B54KR_SeedGpuStatic();
    for (i = 0u; i < 16u; i++)
        PE_StoreU32(0x80170000u + i * 4u,
                    (0x3000u + i * 2u) | ((0x3001u + i * 2u) << 16));
    PE_StoreU32(rect, 0x001E0014u);           /* x=20, y=30 */
    PE_StoreU32(rect + 4u, 0x00020010u);      /* w=16, h=2 */
    ASSERT(func_800775E8(rect, 0x80170000u) == 0 && !PE_Port_ShouldStop(),
           "immediate LoadImage did not return 0");
    ASSERT(PE_LoadU32(PE_DMA_CallbackSlotAddress(2u)) == 0x80077A00u,
           "DMA2 callback slot not func_80077A00");
    ASSERT(func_80074DC0(0) == 0 && !PE_GPU_DMA2Pending() && !PE_Port_ShouldStop(),
           "DrawSync did not complete the direct transfer");
    ASSERT(B54KR_PixelIs(20u, 30u, 0x3000u) && B54KR_PixelIs(35u, 31u, 0x301Fu),
           "uploaded pixels differ");
    PASS();
}

/* src/func_80048918.c (item/equipment window constructor): absent before
 * round 6 — its hand callers recorded a loud boundary instead.  Default mode
 * (no frame D_8009CF0C, no equip-slot group D_8009CF30, no D_8009CF1C). */
static void test_PV_48918_item_window_constructor(void)
{
    pe_addr_t window, list, w13, w15;
    TEST("PV_48918_item_window_constructor");
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    menu4ae1c_build(&window, &list);
    /* Window specs 11 and 13 (D_80092888 + id*32): ResetTestState zeroes the
     * EXE table, and a zero column count is the retail DIV break trap in
     * func_800647D0.  Seed one column / four visible rows / 12px rows. */
    {
        static const uint32_t ids[2] = {11u, 13u};
        unsigned n;
        for (n = 0u; n < 2u; n++) {
            PE_StoreU32(0x80092888u + ids[n] * 32u + 8u, 1u);
            PE_StoreU32(0x80092888u + ids[n] * 32u + 12u, 4u);
            PE_StoreU32(0x80092888u + ids[n] * 32u + 24u, 12u);
        }
    }
    PE_StoreU32(0x8009CF0Cu, 0u);
    PE_StoreU32(0x8009CF30u, 0u);
    PE_StoreU32(0x8009CF1Cu, 0u);
    func_80048918(0x33E, -3, -1);
    ASSERT(!PE_Port_ShouldStop() && PE_Decomp_BoundaryCount() == 0,
           "constructor reached a boundary");
    ASSERT(PE_LoadU32(0x8009CFC4u) == 0xFFFFFFFDu &&
           PE_LoadU32(0x8009CF34u) == 0u && PE_LoadU32(0x8009CF38u) == 1u &&
           PE_LoadU32(0x8009CF18u) == 1u, "a1 == -3 flags / a0 != 0x200");
    w13 = func_80062A34(1u, 13u);
    w15 = func_80062A34(1u, 15u);
    ASSERT(w13 != 0u && PE_LoadU32(w13 + 0x2Cu) == 0x800494ACu &&
           PE_LoadU32(w13 + 0x40u) == 1u, "0x0D list window + update 0x800494AC");
    ASSERT(w15 != 0u && PE_LoadU32(w15 + 0x30u) == 0x8004A0C8u,
           "0x0F detail window draw 0x8004A0C8");
    ASSERT(func_80062A34(1u, 11u) != 0u, "0x0B window created");
    PASS();
}

static void test_PV_all(void)
{
    test_PV_48918_item_window_constructor();
    test_PV_777C0_immediate_moveimage();
    test_PV_775E8_immediate_loadimage();
    test_PV_1F814_null_actor_latch();
    test_PV_67A78_null_header_low_ram();
    test_PV_52F70_signed_cap();
    test_PV_ram_mirror();
    test_PV_661EC_mask_and_mode();
    test_PV_19B08_bit8_polarity();
    test_PV_144FC_out_of_table_completes();
    test_PV_4F464_clears_flag();
    test_PV_6B35C_plus6_halfwords();
    test_PV_6BE4C_previous_half();
    test_PV_67B74_fade_step();
    test_PV_67D18_hold_phase();
    test_PV_52BCC_unsigned_char_head();
}
