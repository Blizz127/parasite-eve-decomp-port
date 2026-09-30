/*
 * port_absent lane (2026-09-23): regression tests for the formerly `absent`
 * leaves and for hand adapters that were written but never tested.  Each
 * sub-header is owned by one worker; this file only aggregates them.
 */
#include "pe_guest_decomp.h"
#include "pe_rcnt2.h"
#include "pe_mmio.h"
#include "test_pa_handhi.h"
#include "test_pa_sdk.h"
#include "test_pa_cd.h"
#include "test_pa_ovl.h"
#include "test_pa_lo2.h"
#include "test_pa_hi2.h"
#include "test_pa_ovl2.h"
#include "test_pa_r3.h"
#include "test_pa_r4.h"
#include "test_pa_sio.h"

/* PE_HostToGuest: the reverse translation behind the generator's
 * pointer-return adapter (pe_addr_t func_X = PE_HostToGuest(host result)). */
static void test_PORTABSENT_host_to_guest(void)
{
    TEST("PORTABSENT_host_to_guest");
    ResetTestState();
    ASSERT(PE_HostToGuest(NULL) == 0u, "NULL is the guest null pointer");
    ASSERT(PE_HostToGuest(PE_Translate(0x80000000u, 1)) == 0x80000000u, "RAM base");
    ASSERT(PE_HostToGuest(PE_Translate(0x801FFFFFu, 1)) == 0x801FFFFFu, "RAM last byte");
    ASSERT(PE_HostToGuest(PE_Translate(0x8009B582u, 2)) == 0x8009B582u, "RAM interior");
    ASSERT(PE_HostToGuest(PE_Translate(0x1F80001Eu, 2)) == 0x1F80001Eu, "scratchpad");
    PASS();
}

/* e9_guest_ptr_port.c — expectations from the matched src/func_X.c. */
#define PA_E9 0x80150000u   /* guest scratch for these tests */
static void test_PORTABSENT_e9_guest_ptr(void)
{
    TEST("PORTABSENT_e9_guest_ptr");
    ResetTestState();

    /* func_80030640: gate on inner[4] bit 16 (0x10000, not bit 0). */
    {
        const pe_addr_t rec = PA_E9, inner = PA_E9 + 0x100u;
        PE_StoreU32(0x8009D278u, rec);
        PE_StoreU32(rec + 0x68u, inner);
        PE_StoreU32(inner + 0x10u, 0x0000FFFFu);     /* bit 16 clear */
        PE_StoreU16(rec + 0x22u, 100u);              /* always passes */
        PE_StoreU16(rec + 0x10u, 0x1234u);
        func_80030640();
        ASSERT(PE_LoadU16(rec + 0x10u) == 0x1234u, "30640 gate bit16 clear");
        PE_StoreU32(inner + 0x10u, 0x00010000u);
        func_80030640();
        ASSERT(PE_LoadU16(rec + 0x10u) == 9000u, "30640 threshold 100 stores 9000");
        PE_StoreU16(rec + 0x10u, 0x1234u);
        PE_StoreU16(rec + 0x22u, 0u);                /* rnd%100 < 0 never */
        func_80030640();
        ASSERT(PE_LoadU16(rec + 0x10u) == 0x1234u, "30640 threshold 0 no store");
    }

    /* func_80019FE0: list walk via word 1, match on word 0x63. */
    {
        const pe_addr_t s = PA_E9 + 0x400u, a = PA_E9 + 0x800u,
                        b = PA_E9 + 0xC00u, t = PA_E9 + 0x1000u;
        PE_StoreU32(0x8009D2F0u, s);
        PE_StoreU32(s + 0x63u * 4u, 7u);
        PE_StoreU32(s + 0x26u * 4u, 0xFFFFFFFFu);
        PE_StoreU32(0x8009D20Cu, a);
        PE_StoreU32(a + 4u, s);                      /* a -> s -> b -> end */
        PE_StoreU32(a + 0x63u * 4u, 5u);
        PE_StoreU32(s + 4u, b);
        PE_StoreU32(b + 4u, 0u);
        PE_StoreU32(b + 0x63u * 4u, 0u);             /* == s[0x63] after zeroing */
        ASSERT(func_80019FE0() == 1, "19FE0 match returns 1");
        ASSERT(PE_LoadU32(s + 0x63u * 4u) == 0u, "19FE0 zeroes s[0x63]");
        ASSERT(PE_LoadU32(s + 0x26u * 4u) == 0xFF9FFFFFu, "19FE0 s[0x26] mask");
        /* s itself is skipped even though s[0x63] == s[0x63]. */
        PE_StoreU32(0x8009D20Cu, s);
        PE_StoreU32(s + 4u, a);
        PE_StoreU32(a + 4u, b);
        PE_StoreU32(a + 0x63u * 4u, 1u);
        PE_StoreU32(b + 4u, 0u);
        PE_StoreU32(b + 0x63u * 4u, 0u);
        ASSERT(func_80019FE0() == 1, "19FE0 skips self, matches b");
        /* no match: the fall-through re-reads the zeroed s[0x63] and RMWs
         * KUSEG 0x98 = main RAM 0x80000098 (bit 20 cleared). */
        PE_StoreU32(b + 0x63u * 4u, 5u);
        PE_StoreU32(0x80000098u, 0xFFFFFFFFu);
        ASSERT(func_80019FE0() == 1, "19FE0 fall-through returns 1");
        ASSERT(PE_LoadU32(0x80000098u) == 0xFFEFFFFFu, "19FE0 KUSEG 0x98 RMW");
        (void)t;
    }

    /* func_80018D50 / 80018DD4: base = **a0 * 22|28 + D_8009D1FC[7]. */
    {
        const pe_addr_t vec = PA_E9 + 0x1400u, val = PA_E9 + 0x1410u,
                        blk = PA_E9 + 0x1420u, tbl = PA_E9 + 0x1500u;
        PE_StoreU32(vec, val);
        PE_StoreU32(val, 3u);
        PE_StoreU32(0x8009D1FCu, blk);
        PE_StoreU32(blk + 7u * 4u, tbl);
        PE_StoreU32(0x8009D1D8u, 0u);
        PE_StoreU8(tbl + 66u, 0x01u);
        ASSERT(func_80018D50(vec) == 1, "18D50 returns 1");
        ASSERT(PE_LoadU8(tbl + 66u) == 0x81u, "18D50 sets bit 7 at 3*22");
        PE_StoreU32(0x8009D1D8u, 1u);
        PE_StoreU8(tbl + 84u, 0xFFu);
        ASSERT(func_80018DD4(vec) == 1, "18DD4 returns 1");
        ASSERT(PE_LoadU8(tbl + 84u) == 0x7Fu, "18DD4 clears bit 7 at 3*28");
        ASSERT(PE_LoadU8(tbl + 66u) == 0x81u, "18DD4 stride 28 not 22");
    }

    /* func_8004006C: %d two SJIS digits, %D one, %s guest string, NUL. */
    {
        const pe_addr_t fmt = PA_E9 + 0x1800u, dst = PA_E9 + 0x1900u,
                        str = PA_E9 + 0x1A00u;
        const char f[] = "a%d%D%sz%s";
        static const unsigned char want[] = {
            'a', 0x82, 0x53, 0x82, 0x51,   /* 42 -> '4','2' = 0x824F+4, +2 */
            0x82, 0x56,                    /* 7  -> 0x824F+7 */
            'h', 'i', 'z', 0 };
        for (unsigned i = 0; i < sizeof f; i++)
            PE_StoreU8(fmt + i, (uint8_t)f[i]);
        PE_StoreU8(str, 'h'); PE_StoreU8(str + 1u, 'i'); PE_StoreU8(str + 2u, 0);
        PE_StoreU32(0x800A1708u, 42u);
        PE_StoreU32(0x800A170Cu, 7u);
        PE_StoreU32(0x800A1710u, str);
        PE_StoreU32(0x800A1714u, 0u);                /* %s NULL: nothing */
        func_8004006C(dst, fmt);
        for (unsigned i = 0; i < sizeof want; i++)
            ASSERT(PE_LoadU8(dst + i) == want[i], "4006C output byte");
    }

    /* func_80088F6C: record flags 0 -> first func_800878F0 is a no-op; copy
     * the pair voice's volumes (D_800B8AC0 + a1 * 0x11C, +0x118/+0x11A), set
     * flags 0x1FF93, reprogram voice a1: the volume pair lands in voice 2's
     * registers 0x20/0x22 (masked 0x7FFF) and the flags end cleared. */
    {
        const pe_addr_t a0 = PA_E9 + 0x2000u;
        const pe_addr_t pair = 0x800B8AC0u + 2u * 0x11Cu;
        PE_StoreU32(a0 + 0xF4u, 0u);
        PE_StoreU16(pair + 0x118u, 0x9111u);
        PE_StoreU16(pair + 0x11Au, 0x2222u);
        func_80088F6C(a0, 2);
        ASSERT(PE_LoadU16(a0 + 0x118u) == 0x9111u &&
               PE_LoadU16(a0 + 0x11Au) == 0x2222u, "88F6C copies the pair volumes");
        ASSERT(PE_SpuRegister_LoadU16(0x20u) == 0x1111u &&
               PE_SpuRegister_LoadU16(0x22u) == 0x2222u, "88F6C programs voice a1");
        ASSERT(PE_LoadU32(a0 + 0xF4u) == 0u, "88F6C flags cleared by 878F0");
    }
    PASS();
}

/* func_8004A6CC (stat >= 3 path): the retail stack buf[3] handed to
 * func_8005B91C / func_8005BA78 must hold exactly what those callees produce
 * for (D_8009CFD0, D_8009CFDC). */
static void test_PORTABSENT_status_panel_buf(void)
{
    TEST_RETAIL_DISC1("PORTABSENT_4A6CC_status_panel_buf"); TEST_RETAIL_FIXUPS(RETAILFIX_menu_text);
    ResetTestState();
    /* Retail menu-text state (glyph packet arena, pen, font) captured by the
     * NAM5 menu-text cases (test_menu_text.h), so the glyph draws run. */
    for (unsigned i = 0; i < sizeof(NAM5_menu_text_common) / sizeof(NAM5_menu_text_common[0]); i++)
        PE_StoreU32(0x80000000u + NAM5_menu_text_common[i][0], NAM5_menu_text_common[i][1]);
    const pe_addr_t ref = PA_E9 + 0x3000u;
    PE_StoreU32(0x8009CFD0u, 3u);
    PE_StoreU32(0x8009CFDCu, 57u);
    func_8005B91C(3, 57, ref + 0u, 0u);
    func_8005BA78(3, 57, ref + 4u, ref + 8u);
    PE_StoreU32(0x801FF700u, 0xDEADBEEFu);
    PE_StoreU32(0x801FF704u, 0xDEADBEEFu);
    PE_StoreU32(0x801FF708u, 0xDEADBEEFu);
    func_8004A6CC();
    ASSERT(PE_LoadU32(0x801FF700u) == PE_LoadU32(ref + 0u), "4A6CC buf[0] from 8005B91C");
    ASSERT(PE_LoadU32(0x801FF704u) == PE_LoadU32(ref + 4u) &&
           PE_LoadU32(0x801FF708u) == PE_LoadU32(ref + 8u), "4A6CC buf[1..2] from 8005BA78");
    PASS();
}

/* func_8001A680 with a zero handler slot: retail reads handler[2] at
 * address 2 = main RAM 0x80000002 through the KUSEG mirror (the live route
 * test_DAY2_m0374i_delivery reaches this via func_80017AE8). */
static void test_PORTABSENT_1A680_null_handler(void)
{
    TEST("PORTABSENT_1A680_null_handler");
    ResetTestState();
    const pe_addr_t body = PA_E9 + 0x4000u;
    PE_StoreU8(body + 0x0Cu, 1u);                       /* classId 1 */
    PE_StoreU32(0x800B0E98u + 0xC0u + 3u * 4u, 0u);     /* entry 3 = 0 */
    PE_StoreU32(body + 0x98u, 0x200u);
    PE_StoreU8(0x80000002u, 5u);
    func_8001A680(body, 0x10003u);                       /* (u16)id = 3 */
    ASSERT(PE_LoadU32(body + 0x1B0u) == 0u, "1A680 handler = 0");
    ASSERT(PE_LoadU8(body + 0x0Eu) == 3u, "1A680 handlerId = (u8)id");
    ASSERT(PE_LoadU8(body + 0x0Fu) == 4u, "1A680 count = low RAM byte 2 - 1");
    ASSERT(PE_LoadU32(body + 0x98u) == 0u, "1A680 clears flag 0x200");
    PASS();
}

/* absent_save_port.c: func_8003F800 / func_8003FBD8 round trip through the
 * D_800A0ED0 cursor, and func_80040B80's block layout + CRC (computed here
 * independently from src/func_80040B80.c). */
static void test_PORTABSENT_save_block(void)
{
    TEST("PORTABSENT_save_block");
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    const pe_addr_t buf = PA_E9 + 0x6000u;
    unsigned i;

    for (i = 0; i < 0x800u; i++) PE_StoreU8(0x800A77F0u + i, (uint8_t)(i * 7u + 1u));
    PE_StoreU32(0x8009D2E8u, 0xFFFFFFFFu);
    PE_StoreU32(0x8009D280u, 0x11223344u);
    PE_StoreU32(0x8009D1A0u, 0xFFFFFFFFu);
    PE_StoreU32(0x800B0CDCu, 0x55667788u);
    PE_StoreU16(0x800B0CE0u, 0xA1A2u); PE_StoreU16(0x800B0CE2u, 0xB1B2u);
    PE_StoreU16(0x800B0CE4u, 0xC1C2u);
    PE_StoreU8(0x800B0CE6u, 0xD1u); PE_StoreU8(0x800BCFEEu, 0xE1u);
    for (i = 0; i < 0x70u; i++) PE_StoreU8(0x800B8A20u + i, (uint8_t)(0x40u + i));
    for (i = 0; i < 0x18u; i++) PE_StoreU8(0x800B0CB0u + i, (uint8_t)(0x90u + i));
    for (i = 0; i < 8u; i++) PE_StoreU8(0x8009D1B0u + i, (uint8_t)(0xF0u + i));

    PE_StoreU32(0x800A0ED0u, buf);
    func_8003F800();
    ASSERT(PE_LoadU32(0x800A0ED0u) == buf + 0x800u + 16u + 6u + 2u + 0x70u + 0x18u + 8u,
           "3F800 advances the cursor by the record size");
    /* layout: +0 D_800A77F0[0x800], +0x800 D2E8, +0x804 D280, +0x808 D1A0,
     * +0x80C 0CDC, +0x810 CE0, +0x812 CE2, +0x814 CE4, +0x816 CE6,
     * +0x817 CFEE, +0x818 8A20[0x70], +0x888 0CB0[0x18], +0x8A0 D1B0[8]. */
    ASSERT(PE_LoadU8(buf + 5u) == 5u * 7u + 1u && PE_LoadU32(buf + 0x804u) == 0x11223344u &&
           PE_LoadU16(buf + 0x812u) == 0xB1B2u && PE_LoadU8(buf + 0x816u) == 0xD1u &&
           PE_LoadU8(buf + 0x817u) == 0xE1u && PE_LoadU8(buf + 0x818u) == 0x40u &&
           PE_LoadU8(buf + 0x8A7u) == 0xF7u, "3F800 field order");

    for (i = 0; i < 0x800u; i++) PE_StoreU8(0x800A77F0u + i, 0u);
    PE_StoreU32(0x8009D280u, 0u); PE_StoreU8(0x800BCFEEu, 0u);
    PE_StoreU8(0x8009D1B0u + 7u, 0u);
    PE_StoreU32(0x800A0ED0u, buf);
    func_8003FBD8();
    ASSERT(PE_LoadU8(0x800A77F0u + 0x7FFu) == (uint8_t)(0x7FFu * 7u + 1u) &&
           PE_LoadU32(0x8009D280u) == 0x11223344u && PE_LoadU8(0x800BCFEEu) == 0xE1u &&
           PE_LoadU8(0x8009D1B0u + 7u) == 0xF7u, "3FBD8 restores every field");
    ASSERT(PE_LoadU32(0x8009D1A0u) == 0xFFFF2679u && PE_LoadU32(0x8009D2E8u) == 0xFFFFFFF6u,
           "3FBD8 masks D_8009D1A0 / D_8009D2E8");

    /* func_80040B80 */
    {
        const pe_addr_t o = PA_E9 + 0x5000u, rec = PA_E9 + 0x5100u;
        unsigned crc = 0xFFFFu, j;
        PE_StoreU8(o + 0x18u, 0x42u);
        PE_StoreU32(0x800A8044u, rec - (0x800A8044u - 0x1Cu));   /* func_8005DE70 -> rec */
        for (i = 0; i < 0xC0u; i++) PE_StoreU8(rec + i, (uint8_t)(i ^ 0x5Au));
        PE_StoreU16(0x80010F48u, 0x4353u);
        for (i = 0; i < 0x12E4u; i++) PE_StoreU8(0x800C0DE0u + i, (uint8_t)(i * 3u));
        ASSERT(func_80040B80(o) == 0, "40B80 returns 0");
        ASSERT(PE_LoadU32(0x800A5D50u) == 0x2000u, "40B80 D_800A5D50 = 0x2000");
        ASSERT(PE_LoadU16(0x8009EED0u) == 0x4353u && PE_LoadU8(0x8009EED2u) == 0x11u &&
               PE_LoadU8(0x8009EED3u) == 1u, "40B80 header id/0x11/1");
        ASSERT(PE_LoadU8(0x8009EED0u + 0x60u) == (0x14u ^ 0x5Au) &&
               PE_LoadU8(0x8009EED0u + 0xFFu) == (0xBFu ^ 0x5Au), "40B80 header copies src");
        ASSERT(PE_LoadU8(0x8009EED0u + 0x100u + 0x12E3u) == (uint8_t)(0x12E3u * 3u),
               "40B80 D_800C0DE0 follows the header");
        ASSERT(func_80072A54(0x8009EED0u + 4u, 0x8009EE8Cu) == 0,
               "40B80 strcpy (A(19h)) copies the func_80040210 title to header +4");
        pe_addr_t tail = 0x8009EED0u + 0x100u + 0x12E4u + 0x800u + 16u + 6u + 2u + 0x70u + 0x18u + 8u;
        /* the CRC word lands inside the zeroed 0x2000 block, after the CRC
         * was computed over it: recompute with those 4 bytes as zero. */
        for (i = 0; i < 0x2000u; i++) {
            pe_addr_t a = 0x8009EED0u + i;
            crc ^= (a >= tail && a < tail + 4u) ? 0u : (unsigned)PE_LoadU8(a) << 8;
            for (j = 0; j < 8u; j++)
                crc = (crc & 0x8000u) ? ((crc << 1) ^ 0x1021u) : (crc << 1);
        }
        ASSERT(PE_LoadU32(0x800A0ED0u) == tail + 4u, "40B80 cursor past the CRC word");
        ASSERT(PE_LoadU32(tail) == (~crc & 0xFFFFu), "40B80 CRC-16/CCITT of the block");
    }
    PASS();
}

/* pe_rcnt2: register semantics (psx-spx timers) and the retail program
 * SetRCnt(2, 0x44E8, 0x1000) -> mode 0x258 (src/func_80085814.c). */
static void test_PORTABSENT_rcnt2_model(void)
{
    TEST("PORTABSENT_rcnt2_model");
    ResetTestState();
    PE_Rcnt2_Reset();
    PE_Rcnt2_WriteCounter(0x1234u);
    PE_Rcnt2_WriteTarget(0x44E8u);
    PE_Rcnt2_WriteMode(0x258u);
    ASSERT(PE_Rcnt2_ReadCounter() == 0u, "mode write resets the counter");
    ASSERT(PE_Rcnt2_PeekMode() == 0x658u, "mode write sets bit 10 (no IRQ request)");
    PE_Rcnt2_Advance(7u);
    ASSERT(PE_Rcnt2_ReadCounter() == 0u, "sysclk/8: 7 cycles is no tick");
    PE_Rcnt2_Advance(1u);
    ASSERT(PE_Rcnt2_ReadCounter() == 1u, "8 cycles = 1 tick (prescale kept)");
    PE_Rcnt2_Advance(8u * 0x44E6u);
    ASSERT(PE_Rcnt2_ReadCounter() == 0x44E7u && !(PE_Rcnt2_PeekMode() & 0x800u),
           "one short of the target");
    PE_Rcnt2_Advance(8u);
    ASSERT(PE_Rcnt2_ReadCounter() == 0u && (PE_Rcnt2_PeekMode() & 0x800u),
           "reset at target, reached flag set");
    ASSERT((PE_Rcnt2_ReadMode() & 0x800u) && !(PE_Rcnt2_PeekMode() & 0x800u),
           "reading the mode clears the reached flag");
    PE_Rcnt2_Advance(8u * (0x44E8u * 3u + 5u));
    ASSERT(PE_Rcnt2_ReadCounter() == 5u, "periodic modulo the target");
    /* free-running system clock, 16-bit wrap and the 0xFFFF flag */
    PE_Rcnt2_WriteMode(0x0000u);
    PE_Rcnt2_Advance(0x10005u);
    ASSERT(PE_Rcnt2_ReadCounter() == 5u && (PE_Rcnt2_PeekMode() & 0x1000u),
           "free run wraps at 16 bits");
    /* sync mode 0 with sync enabled stops counter 2 */
    PE_Rcnt2_WriteMode(0x0001u);
    PE_Rcnt2_Advance(100u);
    ASSERT(PE_Rcnt2_ReadCounter() == 0u, "sync mode 0 stops counter 2");
    PASS();
}

/* rcnt2_port.c: func_80084FC4 / func_80084FE4 from their matched C. */
static void test_PORTABSENT_rcnt2_timeout(void)
{
    TEST("PORTABSENT_84FC4_84FE4_timeout");
    ResetTestState();
    PE_Rcnt2_Reset();
    PE_Rcnt2_WriteTarget(0x44E8u);
    PE_Rcnt2_WriteMode(0x258u);                 /* retail program, /8 clock */
    PE_Rcnt2_Advance(8u * 0x4000u);             /* counter 0x4000 */
    func_80084FC4(0x3Cu);
    ASSERT(PE_LoadU32(0x800BD02Cu) == 0x3Cu && PE_LoadU32(0x800A76D0u) == 0x4000u,
           "84FC4 stores the limit and the zero-extended count");
    ASSERT(func_80084FE4() == 0, "84FE4 not elapsed at +0");
    PE_Rcnt2_Advance(8u * 0x3Bu);
    ASSERT(func_80084FE4() == 0, "84FE4 not elapsed at +0x3B (bit 9 set: no /8)");
    PE_Rcnt2_Advance(8u);
    ASSERT(func_80084FE4() == 1, "84FE4 elapsed at +0x3C");
    /* wrap below the base adds the target (0x44E8) */
    PE_Rcnt2_Advance(8u * (0x44E8u - 0x4000u - 0x3Cu + 0x10u));   /* counter 0x10 */
    func_80084FC4(0x1000u);
    PE_StoreU32(0x800A76D0u, 0x4400u);          /* base above the counter */
    ASSERT(func_80084FE4() == 0, "84FE4 wrapped: 0x10 + 0x44E8 - 0x4400 < 0x1000");
    PE_StoreU32(0x800BD02Cu, 0xF8u);
    ASSERT(func_80084FE4() == 1, "84FE4 wrapped: 0xF8 elapsed");
    /* bit 9 clear (system clock): the difference is scaled by 1/8 */
    PE_Rcnt2_WriteTarget(0u);
    PE_Rcnt2_WriteMode(0x0000u);
    PE_Rcnt2_Advance(0x100u);
    func_80084FC4(0x20u);
    PE_Rcnt2_Advance(0xFFu);
    ASSERT(func_80084FE4() == 0, "84FE4 sysclk: 0xFF >> 3 = 0x1F < 0x20");
    PE_Rcnt2_Advance(1u);
    ASSERT(func_80084FE4() == 1, "84FE4 sysclk: 0x100 >> 3 = 0x20");
    /* target 0 and a wrap: + 0x10000 */
    PE_StoreU32(0x800A76D0u, 0xFFF0u);
    PE_StoreU32(0x800BD02Cu, (0x10000u + PE_Rcnt2_ReadCounter() - 0xFFF0u) >> 3);
    ASSERT(func_80084FE4() == 1, "84FE4 target 0 wraps by 0x10000");
    PASS();
}

/* pe_bios_string.c: BIOS A-function string/memory semantics as quoted from
 * psx-spx "BIOS String Functions" / "BIOS Memory Fill/Copy/Compare". */
static void test_PORTABSENT_bios_string(void)
{
    TEST("PORTABSENT_bios_string");
    ResetTestState();
    const pe_addr_t a = PA_E9 + 0x8000u, b = PA_E9 + 0x8100u;
    const char *hello = "HELLO";
    for (unsigned i = 0; i < 6u; i++) PE_StoreU8(a + i, (uint8_t)hello[i]);

    /* A(19h) strcpy: refuses NULL dst/src and returns 0; otherwise copies
     * through the 00h byte and returns dst. */
    PE_StoreU8(b, 0x77u);
    ASSERT(func_80071A14(0u, a) == 0u, "strcpy dst 0 -> 0");
    ASSERT(func_80071A14(b, 0u) == 0u && PE_LoadU8(b) == 0x77u,
           "strcpy src 0 -> 0, nothing copied");
    PE_StoreU8(b + 6u, 0x77u);
    ASSERT(func_80071A14(b, a) == b, "strcpy returns dst");
    ASSERT(PE_LoadU8(b + 4u) == 'O' && PE_LoadU8(b + 5u) == 0u &&
           PE_LoadU8(b + 6u) == 0x77u, "strcpy copies through the NUL only");
    /* boundary-crossing copy: KUSEG alias source, KSEG0 destination
     * straddling a 4 KiB page boundary (0x80158FFE..0x80159003). */
    {
        const pe_addr_t d = 0x80158FFEu;
        pe_addr_t kuseg = a & 0x1FFFFFFFu;
        ASSERT(func_80071A14(d, kuseg) == d, "strcpy across page, KUSEG src");
        for (unsigned i = 0; i < 6u; i++)
            ASSERT(PE_LoadU8(d + i) == (uint8_t)hello[i], "strcpy page-crossing bytes");
    }
    /* A(17h) strcmp / A(18h) strncmp: NULL rejection and byte difference. */
    ASSERT(func_80072A54(0u, 0u) == 0 && func_80072A54(0u, a) == -1 &&
           func_80072A54(a, 0u) == 1, "strcmp NULL rules");
    ASSERT(func_80072A54(a, b) == 0, "strcmp equal");
    PE_StoreU8(b + 1u, 'A');
    ASSERT(func_80072A54(a, b) == 'E' - 'A', "strcmp [str1+N]-[str2+N]");
    ASSERT(func_80071A04(a, b, 1) == 0 && func_80071A04(a, b, 2) == 'E' - 'A',
           "strncmp stops after maxlen");
    ASSERT(func_80071A04(0u, b, 3) == -1, "strncmp NULL rule");
    /* A(1Bh) strlen */
    ASSERT(func_80072314(a) == 5 && func_80072314(0u) == 0, "strlen");
    /* A(2Ah) memcpy: dst 0 or len > 7FFFFFFFh refuse; always returns dst. */
    PE_StoreU8(b, 0x11u);
    ASSERT(func_80071A34(b, a, -1) == b && PE_LoadU8(b) == 0x11u, "memcpy len<0 refuses");
    ASSERT(func_80071A34(0u, a, 4) == 0u, "memcpy dst 0 returns 0 (dst)");
    ASSERT(func_80071A34(b, a, 3) == b && PE_LoadU8(b + 2u) == 'L', "memcpy copies len");
    /* A(2Bh) memset: returns 0 for len 0 / len > 7FFFFFFFh, else dst. */
    ASSERT(func_80071A44(b, 0x5A, 0) == 0u && PE_LoadU8(b) == 'H', "memset len 0 -> 0");
    ASSERT(func_80071A44(b, 0x5A, 2) == b && PE_LoadU8(b + 1u) == 0x5Au &&
           PE_LoadU8(b + 2u) == 'L', "memset fills len bytes");
    /* A(28h) bzero: same as memset with 00h. */
    ASSERT(func_80071A24(b, 0u) == 0u && PE_LoadU8(b) == 0x5Au, "bzero len 0 -> 0");
    ASSERT(func_80071A24(b, 1u) == b && PE_LoadU8(b) == 0u, "bzero zeroes");
    PASS();
}

/* Generator finding (7): pointer returns keep the root's KUSEG segment;
 * finding (6): PE_DECOMP_PTRGLOBAL translates a stored 0 through the mirror,
 * the _NULLABLE form keeps it NULL. */
static void test_PORTABSENT_segments(void)
{
    TEST("PORTABSENT_segments");
    ResetTestState();
    ASSERT(PE_DecompReturnSegment(0x8000006Du, 0x00000065u) == 0x0000006Du,
           "KUSEG root -> KUSEG result (func_800C2B10: 0x6D)");
    ASSERT(PE_DecompReturnSegment(0x8000006Du, 0xA0000065u) == 0xA000006Du,
           "KSEG1 root -> KSEG1 result");
    ASSERT(PE_DecompReturnSegment(0x8000006Du, 0x80000065u) == 0x8000006Du,
           "KSEG0 root unchanged");
    ASSERT(PE_DecompReturnSegment(0u, 0x00000065u) == 0u, "NULL result stays 0");
    ASSERT(PE_DecompReturnSegment(0x1F800010u, 0x00000065u) == 0x1F800010u,
           "scratchpad result unchanged");
    PE_StoreU32(0x800E2248u, 0u);
    PE_StoreU8(0x80000003u, 0x5Cu);
    ASSERT(PE_DECOMP_PTRGLOBAL(0x800E2248u, unsigned char)[3] == 0x5Cu,
           "PTRGLOBAL 0 reads low RAM through the mirror, like retail");
    ASSERT(PE_DECOMP_PTRGLOBAL_NULLABLE(0x800E2248u, unsigned char) == NULL,
           "PTRGLOBAL_NULLABLE keeps NULL for null-tested slots");
    PASS();
}

int func_80085814(unsigned short a0, short a1, int a2);   /* generated TU */

/* Expect `body` to die with SIGABRT in a forked child. */
#define PA_EXPECT_ABORT(body, msg) do { \
    pid_t pa_pid = fork(); int pa_st = 0; \
    ASSERT(pa_pid >= 0, "fork failed"); \
    if (pa_pid == 0) { \
        freopen("/dev/null", "w", stderr); \
        body; _exit(99); \
    } \
    ASSERT(waitpid(pa_pid, &pa_st, 0) == pa_pid && WIFSIGNALED(pa_st) && \
           WTERMSIG(pa_st) == SIGABRT, msg); \
} while (0)

/* pe_mmio: modelled registers through the value path and through a
 * generated-style pointer-global host pointer; unmodelled I/O aborts. */
static void test_PORTABSENT_mmio(void)
{
    TEST("PORTABSENT_mmio");
    ResetTestState();
    PE_Rcnt2_Reset();
    PE_MMIO_Reset();
    /* value path */
    PE_StoreU16(0x1F801128u, 0x1234u);
    PE_StoreU16(0x1F801124u, 0x0258u);
    ASSERT(PE_Rcnt2_ReadTarget() == 0x1234u && PE_Rcnt2_PeekMode() == 0x0658u,
           "PE_StoreU16 reaches RCNT2 target/mode");
    PE_Rcnt2_Advance(8u * 5u);
    ASSERT(PE_LoadU32(0x1F801120u) == 5u, "PE_LoadU32 of the 16-bit count");
    ASSERT(PE_LoadU16(0xBF801120u) == 5u, "KSEG1 alias of the count");
    /* generated-style pointer-global store: the generated SetRCnt
     * (src/func_80085814.c) through D_8009B7D0 = 0x1F801100 */
    PE_Rcnt2_Reset();
    PE_StoreU32(0x8009B7D0u, 0x1F801100u);
    ASSERT(func_80085814(2, 0x44E8, 0x1000) == 1, "SetRCnt(2) returns 1");
    PE_MMIO_Commit();
    ASSERT(PE_Rcnt2_ReadTarget() == 0x44E8u && PE_Rcnt2_PeekMode() == 0x0658u,
           "generated SetRCnt reaches pe_rcnt2 (target 0x44E8, mode 0x258)");
    /* a PE_DECOMP_PTRGLOBAL store to the mode register */
    PE_DECOMP_PTRGLOBAL(0x8009B7D0u, unsigned short)[0x24 / 2] = 0x0208u;
    ASSERT(PE_LoadU16(0x1F801124u) == 0x0608u,
           "host-pointer mode write committed before the next value access");
    /* I_MASK through D_8009B7CC-style pointer: [1] |= bit */
    (void)PE_IRQ_ExchangeMask(0x0001u);
    PE_MMIO_Reset();
    PE_StoreU32(0x8009B7CCu, 0x1F801070u);
    PE_DECOMP_PTRGLOBAL(0x8009B7CCu, unsigned int)[1] |= 0x40u;
    PE_MMIO_Commit();
    ASSERT(PE_IRQ_GetMask() == 0x0041u, "I_MASK |= through the shadow");
    /* unmodelled I/O stays a loud abort on every path */
    PA_EXPECT_ABORT(PE_StoreU32(0x1F801100u, 1u), "RCNT0 count store aborts");
    PA_EXPECT_ABORT((void)PE_LoadU16(0x1F801040u), "SIO0 load aborts");
    PA_EXPECT_ABORT(PE_StoreU16(0x1F801114u, 0x0100u),
                    "an unmodelled RCNT1 mode value aborts");
    PA_EXPECT_ABORT((void)PE_Translate(0x1F801800u, 1u), "CD register translate aborts");
    PA_EXPECT_ABORT({ PE_DECOMP_PTRGLOBAL(0x8009B7D0u, unsigned int)[0] = 7u;
                      PE_MMIO_Commit(); },
                    "a host-pointer write to unmodelled RCNT0 aborts at commit");
    PASS();
}

/* gen_overlay_ports.py step 1: overlay leaves are linked under their
 * namespaced symbols `<ovl>__func_X` (src/overlays/<ovl>/func_X.c). */
pe_addr_t ovl_03C9__func_801F1B00(void);
int ovl_03C9__func_801F1B60(int a0);
void ovl_0700__func_80190D08(pe_addr_t pe_a0);
static void test_PORTABSENT_overlay_namespaced(void)
{
    TEST("PORTABSENT_overlay_namespaced");
    ResetTestState();
    /* ovl_03C9 func_801F1B00: `return &D_801F1F28;` (a guest address). */
    ASSERT(ovl_03C9__func_801F1B00() == 0x801F1F28u,
           "ovl_03C9 accessor returns &D_801F1F28");
    /* ovl_03C9 func_801F1B60: a0 == 1 -> 1 if D_801F1F3A == 0, else clear
     * it and 0; any other a0 -> 0. */
    PE_StoreU16(0x801F1F3Au, 0u);
    ASSERT(ovl_03C9__func_801F1B60(1) == 1, "801F1B60 flag 0 -> 1");
    PE_StoreU16(0x801F1F3Au, 5u);
    ASSERT(ovl_03C9__func_801F1B60(2) == 0 && PE_LoadU16(0x801F1F3Au) == 5u,
           "801F1B60 a0 != 1 leaves the flag");
    ASSERT(ovl_03C9__func_801F1B60(1) == 0 && PE_LoadU16(0x801F1F3Au) == 0u,
           "801F1B60 clears the flag");
    /* ovl_0700 func_80190D08: func_80191834(a0) — ovl_0700-local, no port:
     * a boundary labelled "ovl_0700:" (never a plain-named pc_port
     * definition) — then func_80191678(*a0), the namespaced generated
     * ovl_0700 leaf: unlink node n from the D_801E4A88 {prev,next} list,
     * push it on the D_8019CBC0 free chain (D_8019C830[n] = old head). */
    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();
    {
        const pe_addr_t L = 0x801E4A88u;
        PE_StoreU16(L + 2u * 4u + 2u, 3u);             /* node2.next = 3 */
        PE_StoreU16(L + 3u * 4u + 0u, 2u);             /* node3.prev = 2 */
        PE_StoreU16(L + 3u * 4u + 2u, 4u);             /* node3.next = 4 */
        PE_StoreU16(L + 4u * 4u + 0u, 3u);             /* node4.prev = 3 */
        PE_StoreU16(0x8019C9D0u, 7u);
        PE_StoreU16(0x8019CBC0u, 9u);
        PE_StoreU16(PA_E9 + 0x7000u, 3u);
        ovl_0700__func_80190D08(PA_E9 + 0x7000u);
        ASSERT(PE_Decomp_BoundaryCount() == 1 &&
               strcmp(PE_Decomp_BoundaryName(0), "ovl_0700:func_80191834") == 0 &&
               g_bootstrap_arg4_calls[0].arg0 == PA_E9 + 0x7000u,
               "ovl_0700-local unported callee: overlay-labelled boundary");
        ASSERT(PE_LoadU16(L + 2u * 4u + 2u) == 4u && PE_LoadU16(L + 4u * 4u) == 2u,
               "node 3 unlinked (2 <-> 4)");
        ASSERT(PE_LoadU16(L + 3u * 4u) == 0xFFFFu && PE_LoadU16(L + 3u * 4u + 2u) == 0xFFFFu,
               "node 3 links cleared to -1");
        ASSERT(PE_LoadU16(0x8019CBC0u) == 3u && PE_LoadU16(0x8019C830u + 6u) == 9u,
               "node 3 pushed on the free chain");
    }
    PASS();
}

/* Step 3 runtime: pe_guestcode (PE_GuestCall / residency / resolve). */
static void test_PORTABSENT_guestcode(void)
{
    TEST("PORTABSENT_guestcode");
    ResetTestState();
    PE_Overlay_ResetResidency();
    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();

    /* EXE: a guest call to 0x800824DC reaches the generated leaf
     * (src/func_800824DC.c: exchange D_800B8AB8 with value, return old). */
    PE_StoreU32(0x800B8AB8u, 0x11u);
    ASSERT(PE_GuestCall("t:exe", 0x800824DCu, 1u, 0x22u, 0, 0, 0) == 0x11 &&
           PE_LoadU32(0x800B8AB8u) == 0x22u, "EXE guest call dispatches");
    ASSERT(PE_Decomp_BoundaryCount() == 0, "no boundary for a resolved call");

    /* Unresolved address: loud boundary with the site, address and args. */
    ASSERT(PE_GuestCall("t:none", 0x80000004u, 2u, 7u, 8u, 0, 0) == 0 &&
           PE_Decomp_BoundaryCount() == 1 &&
           strcmp(PE_Decomp_BoundaryName(0), "t:none") == 0 &&
           g_bootstrap_arg4_calls[0].target == 0x80000004u &&
           g_bootstrap_arg4_calls[0].arg0 == 7u && g_bootstrap_arg4_calls[0].arg1 == 8u,
           "unresolved guest call is a loud boundary carrying the address");

    /* Overlay: without residency (and RAM not holding the retail bytes) the
     * ovl_03C9 function at 0x801F1B60 is unresolved; once ovl_03C9 is
     * resident it dispatches (src/overlays/ovl_03C9/func_801F1B60.c). */
    PE_Decomp_ResetBoundaries();
    ASSERT(PE_GuestCode_Resolve(0x801F1B60u) == NULL, "overlay not resident: unresolved");
    ASSERT(PE_Overlay_SetResident("ovl_03C9") == 0 && PE_Overlay_IsResident("ovl_03C9"),
           "residency set");
    PE_StoreU16(0x801F1F3Au, 0u);
    ASSERT(PE_GuestCall("t:ovl", 0x801F1B60u, 1u, 1u, 0, 0, 0) == 1 &&
           PE_Decomp_BoundaryCount() == 0, "resident overlay dispatches");
    /* Shared arena: ovl_03D2 and ovl_0700 both load at 0x8018EFF0. */
    ASSERT(PE_Overlay_SetResident("ovl_0700") == 0, "ovl_0700 resident");
    ASSERT(PE_Overlay_SetResident("ovl_03D2") == 0 && !PE_Overlay_IsResident("ovl_0700") &&
           PE_Overlay_IsResident("ovl_03D2") && PE_Overlay_IsResident("ovl_03C9"),
           "an overlapping load evicts the other arena tenant only");
    ASSERT(PE_GuestCode_Resolve(0x80191678u) == NULL,
           "ovl_0700's function is not dispatched while ovl_03D2 is resident");
    ASSERT(PE_Overlay_SetResident("no_such_overlay") == -1, "unknown overlay id");
    /* Verified residency: guest RAM does not hold the blob -> SHA-1 mismatch. */
    PE_Overlay_ResetResidency();
    ASSERT(PE_Overlay_SetResidentVerified("ovl_03C9") == -2 &&
           !PE_Overlay_IsResident("ovl_03C9"), "SHA-1 mismatch: not resident");
    /* SHA-1 (FIPS 180-4 test vector "abc"). */
    {
        char hex[41];
        PE_StoreU8(PA_E9 + 0x7800u, 'a'); PE_StoreU8(PA_E9 + 0x7801u, 'b');
        PE_StoreU8(PA_E9 + 0x7802u, 'c');
        PE_GuestCode_Sha1Hex(PA_E9 + 0x7800u, 3u, hex);
        ASSERT(strcmp(hex, "a9993e364706816aba3e25717850c26c9cd0d89d") == 0, "SHA-1 abc");
    }
    PE_Overlay_ResetResidency();
    PASS();
}

/* Step 4: a deduplicated room leaf, dispatched per resident room.
 * src/overlays/room_m0022i/func_80190CD4.c (identical in room_m0428i):
 * `o->fn(o); return 0;` with fn the word at o+0xC — itself a guest call. */
static void test_PORTABSENT_room_dedupe_dispatch(void)
{
    TEST("PORTABSENT_room_dedupe_dispatch");
    ResetTestState();
    PE_Overlay_ResetResidency();
    PE_Decomp_ResetBoundaries();
    const pe_addr_t o = PA_E9 + 0x7A00u;
    PE_StoreU32(o + 0xCu, 0x800824DCu);          /* fn = EXE exchange leaf */
    PE_StoreU32(0x800B8AB8u, 0x55u);
    ASSERT(PE_GuestCode_Resolve(0x80190CD4u) == NULL, "no room resident: unresolved");
    ASSERT(PE_Overlay_SetResident("room_m0428i") == 0, "room_m0428i resident");
    ASSERT(PE_GuestCall("t:room", 0x80190CD4u, 1u, o, 0, 0, 0) == 0 &&
           PE_LoadU32(0x800B8AB8u) == o && PE_Decomp_BoundaryCount() == 0,
           "room leaf dispatches and its o->fn(o) reaches the EXE leaf");
    /* the other member room maps to the same group symbol */
    ASSERT(PE_Overlay_SetResident("room_m0022i") == 0 &&
           !PE_Overlay_IsResident("room_m0428i"), "rooms share one arena");
    {
        const PeGuestCodeEntry *e = PE_GuestCode_Resolve(0x80190CD4u);
        ASSERT(e && strcmp(e->symbol, "room_efeac995__func_80190CD4") == 0,
               "both member rooms dispatch to the deduplicated TU");
    }
    PE_Overlay_ResetResidency();
    PASS();
}

static void test_PORTABSENT_all(void)
{
    test_PORTABSENT_host_to_guest();
    test_PORTABSENT_e9_guest_ptr();
    test_PORTABSENT_status_panel_buf();
    test_PORTABSENT_1A680_null_handler();
    test_PORTABSENT_save_block();
    test_PORTABSENT_rcnt2_model();
    test_PORTABSENT_rcnt2_timeout();
    test_PORTABSENT_bios_string();
    test_PORTABSENT_segments();
    test_PORTABSENT_mmio();
    test_PORTABSENT_overlay_namespaced();
    test_PORTABSENT_guestcode();
    test_PORTABSENT_room_dedupe_dispatch();
    test_PA_handhi_all();
    test_PA_sdk_all();
    test_PA_cd_all();
    test_PA_ovl_all();
    test_PA_lo2_all();
    test_PA_hi2_all();
    test_PA_ovl2_all();
    test_PA_r3_all();
    test_PA_r4_all();
    test_PA_sio_all();
}
