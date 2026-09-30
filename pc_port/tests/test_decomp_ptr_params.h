/*
 * Decomp-derived pointer-parameter port regression tests.
 *
 * The TUs under pc_port/game/decomp/ whose leaves take pointer parameters use
 * the guest-address host adapter (see tools/analysis/gen_decomp_ports.py and
 * docs/ai_context/PC_PORT_FROM_DECOMP.md): the host signature takes a
 * `pe_addr_t` and the verbatim body runs against host pointers translated
 * from it.  These tests pin the retail behavior the matched leaves encode, so
 * a regression in the adapter or the leaf surfaces here:
 *
 *   * `func_8008A02C`  strided dual-base delta accumulate (0x40-byte stride)
 *   * `func_8007E594`  0x18-byte node clear with the +9..+0xB gap quirk
 *   * `func_80017A24`  `*dest = *src & (1 << *bit)` through a struct of pointers
 *   * `func_80017C8C`  three-field forwarder to `func_800661EC`
 *   * `func_8008D7C0`  `*out = D_8009B3A0` (pointer-parameter getter)
 *   * `func_80083E70`  display-node tag/backlink clear
 *   * `func_800816F4`  boundary forwards the two guest addresses, not pointers
 *   * `func_80076B58`  `*D_80095854 = 0x4000000` then a count-many push loop
 */
#include "pe_guest_decomp.h"
#include "pe_bootstrap.h"

extern void func_8008A02C(pe_addr_t a0, unsigned int a1, int a2);
extern void func_8007E594(pe_addr_t a0);
extern int func_80017A24(pe_addr_t arg0);
extern int func_80017C8C(pe_addr_t arg0);
extern void func_8008D7C0(pe_addr_t a0);
extern void func_80083E70(pe_addr_t arg0);
extern int func_800816F4(pe_addr_t left, pe_addr_t right);
extern int func_80076B58(pe_addr_t a0, int a1);
extern int func_8006F39C(unsigned int code, pe_addr_t userdata);

/* Guest scratch for the 6F39C overlay-prelude regression.
 *
 * The prelude's CD read goes through the REAL func_8006E6D4 provider against
 * the synthetic in-memory disc fixture the other CD tests use (FxBuild): the
 * D_80093162 pair selects one in-range sector of that fixture, so the prelude
 * runs the genuine issue/poll sequence rather than a stubbed one.  The
 * dispatch tail then reads a zeroed D_800942E0[0x55] handler slot, so the call
 * returns -8 without dereferencing a handler. */
#define F39C_READ_BUF     0x80093000u  /* prelude read destination         */
#define F39C_TABLE_BASE   0x801F3000u  /* D_800942E0 handler table         */
#define F39C_TABLE_ENTRY  0x801F3200u  /* table[0x55] dispatch entry       */
#define F39C_ARENA        0x801F4000u  /* 0xA0C-stride slot arena          */
#define F39C_HANDLER_IDS  0x800E10A0u  /* installed overlay handlers       */
#define F39C_SECTOR_TABLE 0x80093162u  /* offset/size pair for the prelude */

/* Guest pointer-parameter scratch area.  Host pointers are 8 bytes wide, so
 * anything the leaf reaches through a parameter field must live clear of the
 * adapter struct that holds those fields. */
#define PTR_BASE 0x80150000u
#define PTR_DATA (PTR_BASE + 0x1000u)

/* ── 1. strided dual-base delta accumulate ──────────────────────────── */
static void test_DECOMPPTR_strided_delta_accumulate(void)
{
    TEST("DECOMPPTR_strided_delta_accumulate");
    unsigned i;

    /* func_8008A02C(a0, a1, a2): d = a1 - a0[0]; then for a2 iterations
     * a0[i*0x10] += d and a0[i*0x10 + 1] += d (both unsigned words). */
    for (i = 0; i < 3u; i++) {
        PE_StoreU32(PTR_DATA + i * 0x40u, 100u + i);
        PE_StoreU32(PTR_DATA + i * 0x40u + 4u, 1000u + i);
    }
    /* 0xA00 is 2560 and the delta becomes 2560 - 100 = 2460. */
    func_8008A02C(PTR_DATA, 0xA00u, 3);

    ASSERT(PE_LoadU32(PTR_DATA + 0u) == 0xA00u,
           "slot 0 first word must receive the new value");
    ASSERT(PE_LoadU32(PTR_DATA + 4u) == 1000u + 2460u,
           "slot 0 second word must receive the same delta");
    ASSERT(PE_LoadU32(PTR_DATA + 0x40u) == 100u + 1u + 2460u,
           "slot 1 must be advanced by the same delta");
    ASSERT(PE_LoadU32(PTR_DATA + 0x40u + 4u) == 1000u + 1u + 2460u,
           "slot 1 second word must be advanced too");
    ASSERT(PE_LoadU32(PTR_DATA + 0x80u + 4u) == 1000u + 2u + 2460u,
           "slot 2 second word must be advanced too");
    ASSERT(PE_LoadU32(PTR_DATA + 0xC0u) == 0u,
           "the walk must stop after exactly a2 slots");
    PASS();
}

/* ── 2. 0x18-byte node clear with the retail gap ────────────────────── */
static void test_DECOMPPTR_display_node_zero(void)
{
    TEST("DECOMPPTR_display_node_zero");
    unsigned i;

    /* Fill 0x20 bytes with 0xAA, then clear the node.  Retail clears the word
     * at +0, the byte at +4, then walks bytes +8..+5 with the down-counting
     * `q = a0 + 3` pointer loop, and finally stores three words at +0xC, +0x10
     * and +0x14.  That deliberately leaves a three-byte gap at +9..+0xB — the
     * leaf's quirk is that this is not a contiguous 0x18-byte clear. */
    for (i = 0; i < 0x20u; i++)
        PE_StoreU8(PTR_DATA + i, 0xAAu);

    func_8007E594(PTR_DATA);

    for (i = 0; i <= 0x8u; i++)
        ASSERT(PE_LoadU8(PTR_DATA + i) == 0u,
               "bytes 0..8 must be cleared by the byte walk");
    for (i = 0x9u; i <= 0xBu; i++)
        ASSERT(PE_LoadU8(PTR_DATA + i) == 0xAAu,
               "bytes +9..+0xB are the gap the leaf leaves untouched");
    for (i = 0xCu; i <= 0x17u; i++)
        ASSERT(PE_LoadU8(PTR_DATA + i) == 0u,
               "the three trailing words must clear +0xC..+0x17");
    ASSERT(PE_LoadU8(PTR_DATA + 0x18u) == 0xAAu,
           "bytes past the node must be left alone");
    PASS();
}

/* ── 3. struct-of-pointers bit test ─────────────────────────────────── */
static void test_DECOMPPTR_struct_pointer_bittest(void)
{
    TEST("DECOMPPTR_struct_pointer_bittest");
    /* Retail Arguments { u32 *source, *bit, *destination; } is three 32-bit
     * guest pointers (hand adapter since generator rule E18). */
    PE_StoreU32(PTR_BASE + 0x0u, PTR_DATA + 0x0u);
    PE_StoreU32(PTR_BASE + 0x4u, PTR_DATA + 0x4u);
    PE_StoreU32(PTR_BASE + 0x8u, PTR_DATA + 0x8u);

    PE_StoreU32(PTR_DATA + 0x0u, 0x00000104u); /* bit 2 set, bit 8 set */
    PE_StoreU32(PTR_DATA + 0x4u, 2u);
    PE_StoreU32(PTR_DATA + 0x8u, 0xDEADBEEFu);
    ASSERT(func_80017A24(PTR_BASE) == 1,
           "func_80017A24 must report success");
    ASSERT(PE_LoadU32(PTR_DATA + 0x8u) == (1u << 2),
           "*dest must be exactly *src & (1 << *bit), not a full copy");

    PE_StoreU32(PTR_DATA + 0x0u, 0xFFFFFFFBu); /* bit 2 clear */
    PE_StoreU32(PTR_DATA + 0x8u, 0xFFFFFFFFu);
    func_80017A24(PTR_BASE);
    ASSERT(PE_LoadU32(PTR_DATA + 0x8u) == 0u,
           "a clear source bit must store zero");
    PASS();
}

/* ── 4. three-field forwarder into the camera request leaf ──────────── */
static void test_DECOMPPTR_field_forwarder(void)
{
    TEST("DECOMPPTR_field_forwarder");
    /* Retail Arguments { short *first, *second; u16 *third; } is three 32-bit
     * guest pointers (hand adapter since generator rule E18). */
    PE_StoreU32(PTR_BASE + 0x0u, PTR_DATA + 0x0u);
    PE_StoreU32(PTR_BASE + 0x4u, PTR_DATA + 0x2u);
    PE_StoreU32(PTR_BASE + 0x8u, PTR_DATA + 0x4u);
    PE_StoreU16(PTR_DATA + 0x0u, 0xFFF1u); /* -15 as a short */
    PE_StoreU16(PTR_DATA + 0x2u, 0x0007u);
    PE_StoreU16(PTR_DATA + 0x4u, 0x8001u);

    PE_Decomp_ResetBoundaries();
    /* func_800661EC is a real pc_port leaf here (not a boundary): drive its
     * guard so the forwarder's dereferenced fields reach the camera request. */
    PE_StoreU32(0x800BCF88u, 0x00000040u);
    PE_StoreU32(0x800BCF8Cu, 0x12345678u);
    ASSERT(func_80017C8C(PTR_BASE) == 1,
           "func_80017C8C must always report success");
    ASSERT(PE_LoadU16(0x800BCF9Cu) == 0xFFF1u,
           "the first field must sign-extend into the short request");
    ASSERT(PE_LoadU16(0x800BCF9Eu) == 0x0007u,
           "the second field must forward verbatim");
    ASSERT(PE_LoadU16(0x800BCFA2u) == 0x8001u,
           "the third field must forward as an unsigned short");
    ASSERT(PE_LoadU16(0x800BCFA0u) == 1u,
           "the camera request must be committed");
    /* Retail 0x8006624C: `bne a3,8` with `ori 1` in the delay slot, then
     * `ori 9` on the a3 == 8 fall-through (src/func_800661EC.c); the old
     * hand port had this inverted (portverify). */
    ASSERT((PE_LoadU32(0x800BCF88u) & 0xFu) == 9u,
           "mode 8 must select the pan flag low nibble 9");
    PASS();
}

/* ── 5. pointer-parameter getter over a guest word ──────────────────── */
static void test_DECOMPPTR_word_getter(void)
{
    TEST("DECOMPPTR_word_getter");
    unsigned char *out = PE_Translate(PTR_DATA, 4);

    out[0] = 0xEE; out[1] = 0xEE; out[2] = 0xEE; out[3] = 0xEE;
    PE_StoreU32(0x8009B3A0u, 0x0BADF00Du);
    func_8008D7C0(PTR_DATA);
    ASSERT(PE_LoadU32(PTR_DATA) == 0x0BADF00Du,
           "func_8008D7C0 must copy D_8009B3A0 into the pointed-to word");
    PASS();
}

/* ── 6. display-node tag/backlink clear ─────────────────────────────── */
static void test_DECOMPPTR_display_node_clear(void)
{
    TEST("DECOMPPTR_display_node_clear");
    unsigned i;

    for (i = 0; i < 0x40u; i++)
        PE_StoreU8(PTR_DATA + i, 0xAAu);

    func_80083E70(PTR_DATA);

    ASSERT(PE_LoadU8(PTR_DATA + 0x36u) == 0x45u,
           "the node tag must become 0x45");
    ASSERT(PE_LoadU32(PTR_DATA + 0x2Cu) == 0u,
           "the node backlink word must be cleared");
    ASSERT(PE_LoadU8(PTR_DATA + 0x35u) == 0u,
           "the node count/flag byte must be cleared");
    /* No other byte may change. */
    for (i = 0; i < 0x40u; i++) {
        if (i == 0x36u || (i >= 0x2Cu && i <= 0x2Fu) || i == 0x35u)
            continue;
        ASSERT(PE_LoadU8(PTR_DATA + i) == 0xAAu,
               "func_80083E70 must not touch unrelated bytes");
    }
    PASS();
}

/* ── 7. boundary receives the guest addresses, not host pointers ────── */
static void test_DECOMPPTR_boundary_guest_addresses(void)
{
    TEST("DECOMPPTR_boundary_guest_addresses");
    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();

    /* func_800816F4 forwards the two guest addresses and 12 to BIOS A(18h)
     * strncmp (func_80071A04, pc_port/platform/pe_bios_string.c): the
     * guest strings at those addresses decide the result, so the guest
     * addresses (not host pointers) must have been forwarded. */
    {
        static const char k[] = "BESLUS-00662";
        unsigned i;
        for (i = 0; i < 12u; i++) {
            PE_StoreU8(0x80160000u + i, (uint8_t)k[i]);
            PE_StoreU8(0x80170000u + i, (uint8_t)k[i]);
        }
        PE_StoreU8(0x80170000u + 12u, 'X');         /* beyond maxlen 12 */
        ASSERT(func_800816F4(0x80160000u, 0x80170000u) == 1,
               "equal in the first 12 bytes -> 1");
        PE_StoreU8(0x80170000u + 11u, '3');
        ASSERT(func_800816F4(0x80160000u, 0x80170000u) == 0,
               "byte 11 differs -> 0 (size 12 forwarded)");
        ASSERT(PE_Decomp_BoundaryCount() == 0 && g_bootstrap_arg4_call_count == 0,
               "strncmp is a real call, not a boundary");
    }
    PASS();
}

/* ── 8. count-many push loop with the 0x04000000 flag store ─────────── */
static void test_DECOMPPTR_push_loop(void)
{
    TEST("DECOMPPTR_push_loop");
    PE_StoreU32(0x80095854u, 0x80180000u); /* push-port address slot */
    PE_StoreU32(0x80095850u, 0x80190000u); /* data-port address slot */
    PE_StoreU32(0x80180000u, 0xDEADBEEFu);
    PE_StoreU32(0x80190000u, 0xDEADBEEFu);

    PE_StoreU32(PTR_DATA + 0u, 0x11111111u);
    PE_StoreU32(PTR_DATA + 4u, 0x22222222u);
    PE_StoreU32(PTR_DATA + 8u, 0x33333333u);

    func_80076B58(PTR_DATA, 3);
    ASSERT(PE_LoadU32(0x80180000u) == 0x04000000u,
           "the push port must first see the 0x04000000 flag word");
    ASSERT(PE_LoadU32(0x80190000u) == 0x33333333u,
           "the last pushed word must be the final source word");
    PASS();
}

/* a2 == 0 must write the flag and no data word. */
static void test_DECOMPPTR_push_loop_zero_count(void)
{
    TEST("DECOMPPTR_push_loop_zero_count");
    PE_StoreU32(0x80095854u, 0x80180000u);
    PE_StoreU32(0x80095850u, 0x80190000u);
    PE_StoreU32(0x80180000u, 0u);
    PE_StoreU32(0x80190000u, 0xCAFEBABEu);

    func_80076B58(PTR_DATA, 0);
    ASSERT(PE_LoadU32(0x80180000u) == 0x04000000u,
           "a zero count must still write the flag word");
    ASSERT(PE_LoadU32(0x80190000u) == 0xCAFEBABEu,
           "a zero count must not push any data word");
    PASS();
}

/* ── 9. 6F39C overlay-handler prelude for ids 0x6C..0x72 ─────────────── */
/* Drive the shared 0x6C..0x72 prelude and observe the dispatch tail.  The
 * prelude's CD read resolves the D_80093162 pair against D_800B0DD8 and
 * forwards the difference as the sector count to the real func_8006E6D4.  A
 * one-sector transfer of the synthetic FxBuild image exercises the genuine
 * issue/poll path without any retail disc.  D_800942E0[0x55] points at a null
 * handler entry with a free arena slot, so the dispatch path also returns the
 * allocated slot index. */
static int F39C_RunPrelude(unsigned int code)
{
    return func_8006F39C(code, 0u);
}

static void test_DECOMPPTR_overlay_handler_prelude(void)
{
    DiscFixture fx;

    TEST("DECOMPPTR_overlay_handler_prelude");
    ResetTestState();

    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    /* func_8007ED58 is the CD reset/state-clear every other CD test runs;
     * without it D_8009B574 is 0 and the read issue returns -1 forever. */
    func_8007ED58();
    PE_Disc_SetActive(fx.disc);

    PE_StoreU32(0x800B0DD8u, FX_PEIMG_LBA);   /* mount base (PE.IMG LBA)   */
    PE_StoreU32(0x80011618u, F39C_READ_BUF);  /* prelude read dest         */
    PE_StoreU32(0x800B0CD8u, 0u);             /* flag guard clear          */
    PE_StoreU16(F39C_SECTOR_TABLE, 0x0000u);  /* tbl[0] = 0, sector 0 here */
    PE_StoreU16(F39C_SECTOR_TABLE + 2u, 0x0001u); /* tbl[1] - tbl[0] = 1   */
    PE_StoreU32(0x8009D1A0u, 0x82u);          /* match the 6914C call: ready */
    PE_StoreU32(0x800942E0u, F39C_TABLE_BASE);/* dispatch handler table    */
    PE_StoreU32(F39C_TABLE_BASE + 0x55u * 4u, F39C_TABLE_ENTRY);
    PE_StoreU32(F39C_TABLE_ENTRY + 4u, 0x800C9A70u); /* real ported handler */
    PE_StoreU32(0x800942E4u, F39C_ARENA);     /* 0xA0C-stride arena        */

    /* A code in 0x6C..0x72 must run the prelude, raise bit 0x10000, and
     * install the seven overlay handler descriptors in order.  The call must
     * still return the allocated slot index (0: the arena's first slot). */
    ASSERT(F39C_RunPrelude(0x6Cu) == 0,
           "func_8006F39C must still return the allocated slot index");
    ASSERT((PE_LoadU32(0x800B0CD8u) & 0x10000u) != 0u,
           "the 0x6C..0x72 prelude must raise D_800B0CD8 bit 0x10000");
    ASSERT(PE_LoadU32(F39C_HANDLER_IDS + 0u) == 0x801F1BD8u &&
           PE_LoadU32(F39C_HANDLER_IDS + 4u) == 0x801F1C58u &&
           PE_LoadU32(F39C_HANDLER_IDS + 8u) == 0x801F1D00u &&
           PE_LoadU32(F39C_HANDLER_IDS + 12u) == 0x801F1D8Cu &&
           PE_LoadU32(F39C_HANDLER_IDS + 16u) == 0x801F1E18u &&
           PE_LoadU32(F39C_HANDLER_IDS + 20u) == 0x801F1EA4u &&
           PE_LoadU32(F39C_HANDLER_IDS + 24u) == 0x801F1EF0u,
           "the seven D_800E10A0 overlay handlers must be installed in order");
    ASSERT(PE_LoadU8(F39C_ARENA) == 1u &&
           PE_LoadU8(F39C_ARENA + 1u) == 0x6Cu,
           "the dispatch tail must format the allocated slot for code 0x6C");

    /* The flag is already set, so a second call must not re-stream or
     * reinstall: the whole prelude is skipped and the flag stays set.  Note
     * D_800E1044[0x17] aliases D_800E10A0[0], so slot 0 must hold a valid
     * guest pointer for the 0x55 dispatch; a RAM sentinel proves it was not
     * reinstalled to 0x801F1BD8. */
    PE_StoreU32(F39C_HANDLER_IDS + 0u, F39C_ARENA);
    PE_StoreU32(F39C_HANDLER_IDS + 24u, 0x5A5A5A5Au);
    (void)F39C_RunPrelude(0x6Cu);
    ASSERT(PE_LoadU32(F39C_HANDLER_IDS + 0u) == F39C_ARENA &&
           PE_LoadU32(F39C_HANDLER_IDS + 24u) == 0x5A5A5A5Au &&
           (PE_LoadU32(0x800B0CD8u) & 0x10000u) != 0u,
           "a set 0x10000 flag must make the prelude a no-op");

    /* Clear the flag again; a code below 0x6C must still not run the
     * prelude (an out-of-range install would overwrite the sentinel). */
    PE_StoreU32(0x800B0CD8u, 0u);
    PE_StoreU32(F39C_HANDLER_IDS + 0u, F39C_ARENA);
    (void)F39C_RunPrelude(0x10u);
    ASSERT(PE_LoadU32(F39C_HANDLER_IDS + 0u) == F39C_ARENA &&
           (PE_LoadU32(0x800B0CD8u) & 0x10000u) == 0u,
           "codes below 0x6C must not run the overlay prelude");

    PE_Disc_SetActive(NULL);
    FxFree(&fx);
    PASS();
}

static void test_DECOMPPTR_all(void)
{
    test_DECOMPPTR_strided_delta_accumulate();
    test_DECOMPPTR_display_node_zero();
    test_DECOMPPTR_struct_pointer_bittest();
    test_DECOMPPTR_field_forwarder();
    test_DECOMPPTR_word_getter();
    test_DECOMPPTR_display_node_clear();
    test_DECOMPPTR_boundary_guest_addresses();
    test_DECOMPPTR_push_loop();
    test_DECOMPPTR_push_loop_zero_count();
    test_DECOMPPTR_overlay_handler_prelude();
}
