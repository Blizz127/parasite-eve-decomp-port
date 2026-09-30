/*
 * Decomp-derived port regression tests.
 *
 * These pin the behavior of the generated TUs under pc_port/game/decomp/,
 * which are copied verbatim from the verified matching C leaves in src/ (see
 * tools/analysis/gen_decomp_ports.py and docs/ai_context/PC_PORT_FROM_DECOMP.md).
 *
 * The point is not to re-test the guest-RAM shim; it is to pin the retail
 * quirks the matched leaves encode: exact bit constants, signedness and
 * narrowing, single-write guards, search-and-break semantics, double
 * dereference through a pointer global, mirrored table indexing, and the
 * loud-boundary argument vector.  A regression here means a generated port
 * (or its host adaptation) stopped matching the authority.
 */
#include "pe_guest_decomp.h"
#include "pe_bootstrap.h"

/* ── getter/setter wrappers over one guest word ─────────────────────── */
extern int func_80017FDC(void);            /* D_8009D28C = 5; return 1  */
extern int func_800514F8(void);            /* return D_8009D010         */
extern void func_80051504(void);           /* D_8009D010 = 0            */
extern int func_8007DEB0(void);            /* return D_8009B4AC         */
extern int func_800822AC(void);            /* return D_8009B70C         */
extern int func_80074A28(void);            /* return D_800956EC         */
extern int func_8007FCAC(void);            /* return D_8009B590         */
extern unsigned char func_8007FC54(void);  /* return D_8009B56C         */
extern void func_80080930(void);           /* D_8009B554 = 1            */
extern int func_800438E0(void);            /* return D_8009CEF0         */
extern int func_8004D27C(void);            /* return D_8009CF50         */
extern int func_80042ED0(void);            /* return D_8009CED8 != 0    */
extern unsigned short func_80052514(void); /* return D_800C0E28         */
extern unsigned short func_80052524(void); /* return D_800C0E32         */
extern int func_80019050(void);            /* return 1                  */
extern int func_800190AC(void);            /* return 1                  */
extern int func_80017E9C(void);            /* return 1                  */
extern int func_8003D82C(void);            /* return 1                  */
extern void func_800527C0(void);           /* empty body                */

/* ── flag bit leaves ────────────────────────────────────────────────── */
extern int func_80016DF8(void);            /* D_8009D2F0->f |= 0x20000000  */
extern int func_80016E1C(void);            /* D_8009D2F0->f &= ~0x20000000 */
extern int func_80017EFC(void);            /* *(D_8009D2F0+0x98) |= 0x100   */
extern int func_80017F20(void);            /* *(D_8009D2F0+0x98) &= ~0x100  */
extern int func_80018BC8(void);            /* *(D_8009D2F0+0x98) &= ~0x20   */

/* ── indexed / mirror-table reads ───────────────────────────────────── */
extern unsigned char func_80076B44(int a0);        /* D_800A3348[a0]     */
extern short func_800534CC(int index);             /* *D_8009D048[index] */
extern int func_80077D30(int a0);                  /* mirrored quadrants */
extern int func_8007633C(void);                    /* *D_80095854        */

/* ── search / set-clear / guard / forwarders ────────────────────────── */
extern void func_800363F4(unsigned int needle);    /* 16-entry key clear */
extern int func_800858E8(unsigned int a0);         /* mask |= table[i]   */
extern int func_80085918(unsigned int a0);         /* mask &= ~table[i]  */
extern void func_80085F44(int a0);                 /* guarded setter     */
extern void func_8007DD14(unsigned int value);     /* 73CF4(4, value)    */
extern void func_80085DC4(int value);              /* 73CC4(9, value)    */
extern void func_8007DE78(void);                   /* two boundaries     */
extern void func_8004E94C(void);                   /* boundary with arg  */

/* Retail .data: eight 32-bit DMA callback identities (see pe_libetc.c). */
#define DECOMP_DMA_CB_TABLE 0x800956C0u

/* ── 1. scalar getters/setters (retail widening + write/no-write) ───── */
static void test_DECOMP_scalar_getters_and_setters(void)
{
    TEST("DECOMP_scalar_getters_and_setters");
    ResetTestState();

    /* func_80017FDC writes 5 to D_8009D28C and returns 1. */
    PE_StoreU32(0x8009D28Cu, 0xDEADBEEFu);
    ASSERT(func_80017FDC() == 1 && PE_LoadU32(0x8009D28Cu) == 5u,
           "func_80017FDC must store 5 and return 1");

    /* func_800514F8 reads D_8009D010 (int), func_80051504 clears it. */
    PE_StoreU32(0x8009D010u, 0xFFFFFFFEu);
    ASSERT(func_800514F8() == -2, "func_800514F8 must return the signed word");
    func_80051504();
    ASSERT(PE_LoadU32(0x8009D010u) == 0u, "func_80051504 must clear the word");

    /* Unsigned-word getters must not sign-extend. */
    PE_StoreU32(0x8009B4ACu, 0x80000001u);
    ASSERT(func_8007DEB0() == (int)0x80000001u,
           "func_8007DEB0 must return the full 32-bit word");
    PE_StoreU32(0x8009B70Cu, 0xFFFFFFFFu);
    ASSERT(func_800822AC() == -1, "func_800822AC must return 0xFFFFFFFF as int");
    PE_StoreU32(0x800956ECu, 0x7FFFFFFFu);
    ASSERT(func_80074A28() == 0x7FFFFFFF, "func_80074A28 must return the word");
    PE_StoreU32(0x8009B590u, 0x00010001u);
    ASSERT(func_8007FCAC() == 0x00010001, "func_8007FCAC must return the word");

    /* Byte getter: only the low byte is read. */
    PE_StoreU32(0x8009B56Cu, 0x000000ABu);
    ASSERT(func_8007FC54() == 0xABu, "func_8007FC54 must read one unsigned byte");

    /* func_80080930 writes 1 to an opaque word. */
    PE_StoreU32(0x8009B554u, 0u);
    func_80080930();
    ASSERT(PE_LoadU32(0x8009B554u) == 1u, "func_80080930 must store 1");

    /* func_80042ED0 is a != 0 boolean over a signed int. */
    PE_StoreU32(0x8009CED8u, 0u);
    ASSERT(func_80042ED0() == 0, "func_80042ED0 must be 0 when the word is 0");
    PE_StoreU32(0x8009CED8u, 0x80000000u);
    ASSERT(func_80042ED0() == 1, "func_80042ED0 must be 1 for a negative word");

    /* Halfword getters: upper bits of the word must not leak. */
    PE_StoreU32(0x800C0E28u, 0xFFFF1234u);
    ASSERT(func_80052514() == 0x1234u, "func_80052514 must read one halfword");
    PE_StoreU32(0x800C0E32u, 0xFFFFABCDu);
    ASSERT(func_80052524() == 0xABCDu, "func_80052524 must read one halfword");

    /* Return-1 leaves. */
    ASSERT(func_80019050() == 1 && func_800190AC() == 1 &&
           func_80017E9C() == 1 && func_8003D82C() == 1,
           "constant-1 leaves must return 1");

    /* Empty leaf writes nothing. */
    PE_StoreU32(0x800527C0u, 0x5A5A5A5Au);
    func_800527C0();
    ASSERT(PE_LoadU32(0x800527C0u) == 0x5A5A5A5Au,
           "func_800527C0 must be a no-op");
    PASS();
}

/* ── 2. bit constants: exact bit, and untouched neighbouring bits ───── */
static void test_DECOMP_flag_bits_exact(void)
{
    TEST("DECOMP_flag_bits_exact");
    ResetTestState();

    /* D_8009D2F0 is a pointer global to a state record; flags at +0x98. */
    PE_StoreU32(0x8009D2F0u, 0x80110000u);
    PE_StoreU32(0x80110098u, 0x00000000u);
    ASSERT(func_80016DF8() == 1 && PE_LoadU32(0x80110098u) == 0x20000000u,
           "func_80016DF8 must set bit 29 (0x20000000), not 0x80000000");
    /* Set a neighbour bit, then clear only the 0x20000000 bit. */
    PE_StoreU32(0x80110098u, 0x2000000Au);
    ASSERT(func_80016E1C() == 1 && PE_LoadU32(0x80110098u) == 0xAu,
           "func_80016E1C must clear only bit 29");

    /* The byte-addressed twins use 0x100 and 0x20. */
    PE_StoreU32(0x80110098u, 0xFFFFFFFFu);
    ASSERT(func_80017EFC() == 1,
           "func_80017EFC must return 1");
    PE_StoreU32(0x80110098u, 0x00000000u);
    func_80017EFC();
    ASSERT(PE_LoadU32(0x80110098u) == 0x100u,
           "func_80017EFC must set exactly 0x100");
    PE_StoreU32(0x80110098u, 0x1FFu);
    func_80017F20();
    ASSERT(PE_LoadU32(0x80110098u) == 0xFFu,
           "func_80017F20 must clear exactly 0x100");
    PE_StoreU32(0x80110098u, 0x3Fu);
    func_80018BC8();
    ASSERT(PE_LoadU32(0x80110098u) == 0x1Fu,
           "func_80018BC8 must clear exactly 0x20");
    PASS();
}

/* ── 3. indexed reads, pointer-global base, mirrored tables ─────────── */
static void test_DECOMP_indexed_and_mirrored_reads(void)
{
    TEST("DECOMP_indexed_and_mirrored_reads");
    ResetTestState();

    /* func_80076B44: byte table at D_800A3348. */
    PE_StoreU8(0x800A3348u, 0x11u);
    PE_StoreU8(0x800A3349u, 0x22u);
    PE_StoreU8(0x800A334Au, 0x33u);
    ASSERT(func_80076B44(0) == 0x11u && func_80076B44(1) == 0x22u &&
           func_80076B44(2) == 0x33u,
           "func_80076B44 must read the indexed byte");

    /* func_800534CC: pointer global D_8009D048, indexed halfwords. */
    PE_StoreU32(0x8009D048u, 0x80120000u);
    PE_StoreU16(0x80120000u, 0x1357u);
    PE_StoreU16(0x80120002u, 0x2468u);
    ASSERT(func_800534CC(0) == 0x1357 && func_800534CC(1) == 0x2468,
           "func_800534CC must read through the pointer global");

    /* func_80077D30: four mirrored quadrants over D_8009489C/D_8009589C. */
    PE_StoreU16(0x8009589Cu, 0x1111u);
    PE_StoreU16(0x8009589Cu + 0x7FEu, 0x1234u); /* [0x3FF] */
    PE_StoreU16(0x8009489Cu + 0x801u * 2u, 0x0777u); /* [0x801] */
    ASSERT(func_80077D30(0) == 0x1111, "low quadrant must use D_8009589C[a0]");
    ASSERT(func_80077D30(0x800) == 0x1111,
           "the 0x800 mirror must land on D_8009589C[0]");
    ASSERT(func_80077D30(0x401) == 0x1234,
           "the 0x401..0x800 mirror must use D_8009589C[0x800-a0]");
    ASSERT(func_80077D30(0x801) == -0x0777,
           "the 0x801..0xC00 band must negate D_8009489C[a0]");
    ASSERT(func_80077D30(0xC01) == -0x1234,
           "the top mirror must negate D_8009589C[0x1000-a0]");

    /* func_8007633C: double dereference through a pointer global. */
    PE_StoreU32(0x80095854u, 0x80130000u);
    PE_StoreU32(0x80130000u, 0x0BADF00Du);
    ASSERT(func_8007633C() == 0x0BADF00D,
           "func_8007633C must double-dereference D_80095854");
    PASS();
}

/* ── 4. search-and-clear over the 16-entry table ────────────────────── */
static void test_DECOMP_search_and_clear(void)
{
    TEST("DECOMP_search_and_clear");
    unsigned i;
    for (i = 0; i < 16u; i++) {
        PE_StoreU32(0x800A7624u + i * 8u, 0x100u + i);      /* key   */
        PE_StoreU32(0x800A7624u + i * 8u + 4u, 0x900u + i); /* pad4  */
    }

    /* A miss scans all 16 entries and clears nothing. */
    func_800363F4(0x999u);
    for (i = 0; i < 16u; i++) {
        ASSERT(PE_LoadU32(0x800A7624u + i * 8u) == 0x100u + i,
               "a miss must not clear any key");
        ASSERT(PE_LoadU32(0x800A7624u + i * 8u + 4u) == 0x900u + i,
               "a miss must not touch the partner word");
    }

    /* A hit clears only the matching key and breaks immediately. */
    PE_StoreU32(0x800A7624u + 5u * 8u, 0x100u + 9u); /* duplicate key at 9 */
    func_800363F4(0x109u);
    ASSERT(PE_LoadU32(0x800A7624u + 5u * 8u) == 0u,
           "the first matching key must be cleared");
    ASSERT(PE_LoadU32(0x800A7624u + 9u * 8u) == 0x109u,
           "the search must break before the later duplicate");
    ASSERT(PE_LoadU32(0x800A7624u + 5u * 8u + 4u) == 0x905u,
           "the partner word must be untouched");
    PASS();
}

/* ── 5. mask set/clear through a pointer-global accumulator ─────────── */
static void test_DECOMP_mask_set_clear(void)
{
    TEST("DECOMP_mask_set_clear");
    ResetTestState();

    PE_StoreU32(0x8009B7CCu, 0x80140000u);
    PE_StoreU32(0x80140004u, 0x000000F0u);

    PE_StoreU32(0x8009B7D4u + 2u * 4u, 0x00000F00u);
    ASSERT(func_800858E8(2u) == 1,
           "func_800858E8 must report in-range for i < 3");
    ASSERT(PE_LoadU32(0x80140004u) == 0x00000FF0u,
           "func_800858E8 must OR the table word into accumulator word 1");

    PE_StoreU32(0x8009B7D4u + 5u * 4u, 0xFFFFFFFFu);
    ASSERT(func_800858E8(5u) == 0,
           "func_800858E8 must report out-of-range for i >= 3");
    ASSERT(PE_LoadU32(0x80140004u) == 0xFFFFFFFFu,
           "the OR must still be applied when the predicate is false");

    ASSERT(func_80085918(2u) == 1,
           "func_80085918 must always report 1");
    ASSERT(PE_LoadU32(0x80140004u) == 0xFFFFF0FFu,
           "func_80085918 must AND the complement of the table word "
           "(the earlier OR left 0xFFFFFFFF)");

    /* The index is masked to 16 bits before use. */
    PE_StoreU32(0x8009B7D4u, 0x0000FFFFu);
    (void)func_800858E8(0x00010000u);
    ASSERT(PE_LoadU32(0x80140004u) == 0xFFFFFFFFu,
           "func_800858E8 must mask the index with 0xFFFF");
    PASS();
}

/* ── 6. guarded setter: writes only on change ───────────────────────── */
static void test_DECOMP_guarded_setter(void)
{
    TEST("DECOMP_guarded_setter");
    ResetTestState();

    PE_StoreU32(0x8009B434u, 0x12345678u);
    func_80085F44(0x12345678);
    ASSERT(PE_LoadU32(0x8009B434u) == 0x12345678u,
           "an unchanged value must leave the word alone");
    func_80085F44(0x0BADF00D);
    ASSERT(PE_LoadU32(0x8009B434u) == 0x0BADF00Du,
           "a changed value must be stored verbatim");
    PASS();
}

/* ── 7. channel forwarders reach the exact DMA callback slot ────────── */
static void test_DECOMP_channel_forwarders(void)
{
    TEST("DECOMP_channel_forwarders");
    ResetTestState();

    /* func_8007DD14(value) == func_80073CF4(4, value). */
    PE_StoreU32(DECOMP_DMA_CB_TABLE + 4u * 4u, 0u);
    func_8007DD14(0x80001234u);
    ASSERT(PE_LoadU32(DECOMP_DMA_CB_TABLE + 4u * 4u) == 0x80001234u,
           "func_8007DD14 must register on DMA channel 4");
    /* Same-handler reinstall is a no-op (func_800746A0 equality path). */
    func_8007DD14(0x80001234u);
    ASSERT(PE_LoadU32(DECOMP_DMA_CB_TABLE + 4u * 4u) == 0x80001234u,
           "a same-handler reinstall must not disturb the slot");

    /* func_80085DC4(value) == func_80073CC4(9, value).  Source 9 is outside
     * func_800740D0's translated 0/2/3 set, so the forward is observed at the
     * loud boundary with the exact source and handler arguments. */
    Bootstrap_ResetArg4CallLog();
    func_80085DC4(0x80005678);
    ASSERT(g_bootstrap_arg4_call_count == 1,
           "func_80085DC4 must reach the func_80073CC4 backend exactly once");
    ASSERT(g_bootstrap_arg4_calls[0].arg0 == 9u &&
           g_bootstrap_arg4_calls[0].arg1 == 0x80005678u,
           "func_80085DC4 must pass source 9 and the value unchanged");
    PASS();
}

/* ── 8. loud boundary records the symbol and the guest argument ─────── */
static void test_DECOMP_boundary_argument_vector(void)
{
    TEST("DECOMP_boundary_argument_vector");
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();

    /* The generated shim a TU emits for an unresolved 1-argument callee,
     * exactly as func_8004E94C (`func_8004E704(D_8009CF0C - 1)`) spelled it
     * before func_8004E704 got a hand adapter (absent_lo2_port.c, 2026-09-24);
     * func_8004E94C itself now calls the real window builder. */
    PE_StoreU32(0x8009CF0Cu, 7u);
    (void)PE_D_COMP_BOUNDARY1("func_8004E704", 0x8004E704u,
                              (int)PE_LoadU32(0x8009CF0Cu) - 1);
    ASSERT(PE_Decomp_BoundaryCount() == 1,
           "the unresolved callee must register exactly one boundary");
    ASSERT(strcmp(PE_Decomp_BoundaryName(0), "func_8004E704") == 0,
           "the boundary must name the retail callee");
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].arg0 == 6u,
           "the boundary must forward the computed guest argument (7 - 1)");

    /* Zero-argument boundaries forward no registers. */
    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();
    /* (func_800506E8 used to be the second example; it forwards its live
     * $a0 to func_80058AA8 and is now a hand adapter with that argument.) */
    func_8007DE78();
    ASSERT(PE_Decomp_BoundaryCount() == 2,
           "a zero-arg leaf must register its two distinct callees");
    ASSERT(g_bootstrap_arg4_call_count == 0,
           "zero-argument boundaries must not fabricate an argument vector");
    PASS();
}

static void test_DECOMP_all(void)
{
    test_DECOMP_scalar_getters_and_setters();
    test_DECOMP_flag_bits_exact();
    test_DECOMP_indexed_and_mirrored_reads();
    test_DECOMP_search_and_clear();
    test_DECOMP_mask_set_clear();
    test_DECOMP_guarded_setter();
    test_DECOMP_channel_forwarders();
    test_DECOMP_boundary_argument_vector();
}
