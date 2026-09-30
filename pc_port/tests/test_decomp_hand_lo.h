/*
 * Hand-adapter regression tests for matched leaves below 0x80060000
 * (pc_port/game/decomp_hand/absent_lo_*_port.c, port_absent lane).
 *
 * Each case pins behaviour stated by the matched leaf src/func_XXXXXXXX.c:
 * operand vectors hold 32-bit guest pointers (4-byte stride), exact masks,
 * widths/signedness, the no-write paths, and the guest arguments forwarded to
 * an unported callee's loud boundary.
 */
#include "pe_guest_decomp.h"
#include "pe_bootstrap.h"

#define HLO_VEC  0x80154000u   /* operand vector              */
#define HLO_OP   0x80154100u   /* operands, 0x40 apart        */
#define HLO_ST   0x80155000u   /* stand-in D_8009D2F0 record  */

static void HloSeedVec(unsigned n)
{
    unsigned i;
    for (i = 0; i < n; i++)
        PE_StoreU32(HLO_VEC + 4u * i, HLO_OP + 0x40u * i);
}

/* func_80017E68: *a0[1] = 1 iff all bits of *a0[0] are set in state+0x98. */
static void test_HANDLO_all_bits_set_predicate(void)
{
    TEST("HANDLO_all_bits_set_predicate");
    ResetTestState();
    HloSeedVec(2);
    PE_StoreU32(0x8009D2F0u, HLO_ST);
    PE_StoreU32(HLO_ST + 0x98u, 0x0000F0F0u);

    PE_StoreU32(HLO_OP, 0x000000F0u);
    PE_StoreU32(HLO_OP + 0x40u, 0xFFu);
    ASSERT(func_80017E68(HLO_VEC) == 1, "handler returns 1");
    ASSERT(PE_LoadU32(HLO_OP + 0x40u) == 1u, "subset of set bits -> 1");

    PE_StoreU32(HLO_OP, 0x000001F0u);
    (void)func_80017E68(HLO_VEC);
    ASSERT(PE_LoadU32(HLO_OP + 0x40u) == 0u, "one missing bit -> 0");
    PASS();
}

/* func_80019450: (short(+0x224) << 1) * *a0[0], high half -> (*(+0x1B4))+0x14. */
static void test_HANDLO_scaled_high_half(void)
{
    TEST("HANDLO_scaled_high_half");
    ResetTestState();
    HloSeedVec(1);
    PE_StoreU32(0x8009D2F0u, HLO_ST);
    PE_StoreU16(HLO_ST + 0x224u, (unsigned short)(short)-0x100);  /* n = -0x200 */
    PE_StoreU32(HLO_ST + 0x1B4u, HLO_ST + 0x400u);
    PE_StoreU32(HLO_OP, 0x00030000u);                              /* v */
    PE_StoreU16(HLO_ST + 0x414u, 0xAAAAu);

    ASSERT(func_80019450(HLO_VEC) == 1, "handler returns 1");
    /* -0x200 * 0x30000 = -0x6000000; >> 16 = -0x600 (arithmetic). */
    ASSERT(PE_LoadU16(HLO_ST + 0x414u) == (unsigned short)(short)-0x600,
           "signed product high half must be stored at +0x14");
    PASS();
}

/* func_80019A1C: short/byte copies into the state then +0x250 |= 8. */
static void test_HANDLO_state_field_copy(void)
{
    TEST("HANDLO_state_field_copy");
    ResetTestState();
    HloSeedVec(4);
    PE_StoreU32(0x8009D2F0u, HLO_ST);
    PE_StoreU32(HLO_OP + 0x00u, 0x12345678u);
    PE_StoreU32(HLO_OP + 0x40u, 0x1FFu);
    PE_StoreU32(HLO_OP + 0x80u, 0x2AAu);
    PE_StoreU32(HLO_OP + 0xC0u, 0x3BBu);
    PE_StoreU16(HLO_ST + 0x250u, 0x0101u);

    ASSERT(func_80019A1C(HLO_VEC) == 1, "handler returns 1");
    ASSERT(PE_LoadU16(HLO_ST + 0x24Eu) == 0x5678u, "low half of **a0 at +0x24E");
    ASSERT(PE_LoadU8(HLO_ST + 0x24Bu) == 0xFFu &&
           PE_LoadU8(HLO_ST + 0x24Cu) == 0xAAu &&
           PE_LoadU8(HLO_ST + 0x24Du) == 0xBBu, "low bytes at +0x24B..+0x24D");
    ASSERT(PE_LoadU16(HLO_ST + 0x250u) == 0x0109u, "+0x250 |= 8 keeps other bits");
    PASS();
}

/* func_80038CE4: two-level byte getter through D_80091A28 (index masked 0xFF). */
static void test_HANDLO_two_level_byte_getter(void)
{
    TEST("HANDLO_two_level_byte_getter");
    ResetTestState();
    PE_StoreU32(0x80091A28u, HLO_ST);
    PE_StoreU8(HLO_ST + 0x1Du + 0x05u, 0x10u);
    PE_StoreU8(HLO_ST + 4u + 0x10u, 0x9Cu);
    ASSERT(func_80038CE4(0x305u) == 0x9C, "a0 & 0xFF selects the index byte");
    PASS();
}

/* func_8005184C: dynamic bit set; retail sllv uses the low 5 bits. */
static void test_HANDLO_dynamic_bit_set(void)
{
    TEST("HANDLO_dynamic_bit_set");
    ResetTestState();
    PE_StoreU32(0x800C0E24u, 0x1u);
    func_8005184C(4u);
    ASSERT(PE_LoadU32(0x800C0E24u) == 0x11u, "bit 4 ORed in");
    func_8005184C(33u);
    ASSERT(PE_LoadU32(0x800C0E24u) == 0x13u, "count 33 wraps to bit 1 (sllv)");
    PASS();
}

/* func_8005DE70: D_800A8044 + 0x800A8028. */
static void test_HANDLO_record_base_sum(void)
{
    TEST("HANDLO_record_base_sum");
    ResetTestState();
    PE_StoreU32(0x800A8044u, 0x40u);
    ASSERT((unsigned int)func_8005DE70() == 0x800A8068u,
           "value plus &D_800A8044 - 0x1C");
    PASS();
}

/* D_800BCF88 / D_800A76C4 flag writers. */
static void test_HANDLO_flag_writers(void)
{
    TEST("HANDLO_flag_writers");
    ResetTestState();
    PE_StoreU32(0x800BCF88u, 0xFFFFFF07u);
    ASSERT(func_80017D18() == 1, "returns 1");
    ASSERT(PE_LoadU32(0x800BCF88u) == 0xFFFFFF80u, "low 3 bits cleared, 0x80 set");
    (void)func_800182A0();
    ASSERT(PE_LoadU32(0x800BCF88u) == 0xFFFFFF00u, "&= ~0xC0");
    (void)func_800182C0();
    ASSERT(PE_LoadU32(0x800BCF88u) == 0xFFFFFFC0u, "|= 0xC0");
    PE_StoreU32(0x800A76C4u, 1u);
    (void)func_80018754();
    ASSERT(PE_LoadU32(0x800A76C4u) == 5u, "D_800A76C4 |= 4");
    PE_StoreU32(0x8003E60Cu, 0xFFFFFFFFu);
    func_8003E5F0();
    ASSERT(PE_LoadU32(0x8003E60Cu) == 0u, "D_8003E60C++ wraps");
    PASS();
}

/* func_80018B30: func_800679C4 (batch21_hi_port.c) gets the three
 * sign-extended shorts: x = g[0x28] + dx, y = g[0x2A] + dy, angle
 * g[0x24] + dz, with the clamps of src/func_800679C4.c. */
static void test_HANDLO_boundary_short_operands(void)
{
    TEST("HANDLO_boundary_short_operands");
    const pe_addr_t g = HLO_ST + 0x400u;
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    HloSeedVec(3);
    PE_StoreU16(HLO_OP + 0x00u, 0xFFFEu);
    PE_StoreU16(HLO_OP + 0x40u, 0x0007u);
    PE_StoreU16(HLO_OP + 0x80u, 0x8000u);
    PE_StoreU32(0x800B1624u, g);
    PE_StoreU16(g + 0x28u, 90u); PE_StoreU16(g + 0x2Au, 10u); PE_StoreU16(g + 0x24u, 5u);
    PE_StoreU16(g + 0x30u, (uint16_t)-100); PE_StoreU16(g + 0x32u, 100u);
    PE_StoreU16(g + 0x34u, 0u); PE_StoreU16(g + 0x36u, 200u);
    ASSERT(func_80018B30(HLO_VEC) == 1, "handler returns 1");
    ASSERT(PE_Decomp_BoundaryCount() == 0, "func_800679C4 is a real call");
    ASSERT(PE_LoadU16(g + 0x2Cu) == 88u && PE_LoadU16(g + 0x2Eu) == 17u &&
           PE_LoadU16(g + 0x26u) == 0x8005u,
           "the three operands are forwarded sign-extended");
    PASS();
}

/* func_8001A064: untarget every listed actor aimed at this state. */
static void test_HANDLO_untarget_list_walk(void)
{
    TEST("HANDLO_untarget_list_walk");
    const pe_addr_t a1 = HLO_ST + 0x400u, a2 = HLO_ST + 0x800u;
    ResetTestState();
    PE_StoreU32(0x8009D2F0u, HLO_ST);
    PE_StoreU32(HLO_ST + 0x98u, 0xFFFFFFFFu);
    PE_StoreU32(0x8009D20Cu, a1);
    PE_StoreU32(a1 + 4u, a2);
    PE_StoreU32(a2 + 4u, 0u);
    PE_StoreU32(a1 + 0x18Cu, HLO_ST);      PE_StoreU32(a1 + 0x98u, 0xFFFFFFFFu);
    PE_StoreU32(a2 + 0x18Cu, 0x80123450u); PE_StoreU32(a2 + 0x98u, 0xFFFFFFFFu);

    ASSERT(func_8001A064() == 1, "returns 1");
    ASSERT(PE_LoadU32(HLO_ST + 0x98u) == 0xFFEFFFFFu, "own bit 20 cleared");
    ASSERT(PE_LoadU32(a1 + 0x18Cu) == 0u && PE_LoadU32(a1 + 0x98u) == 0xFF9FFFFFu,
           "matching actor loses its target and bits 21-22");
    ASSERT(PE_LoadU32(a2 + 0x18Cu) == 0x80123450u && PE_LoadU32(a2 + 0x98u) == 0xFFFFFFFFu,
           "non-matching actor untouched");
    PASS();
}

/* func_8003E0FC: keyed 12-byte record search with sign-extended copy-out. */
static void test_HANDLO_keyed_record_search(void)
{
    TEST("HANDLO_keyed_record_search");
    const pe_addr_t hdr = HLO_ST + 0x100u, rec = HLO_ST + 0x200u, out = HLO_ST + 0x300u;
    ResetTestState();
    PE_StoreU32(HLO_ST + 0u, hdr);
    PE_StoreU8(hdr + 3u, 2u);                      /* two records */
    PE_StoreU32(HLO_ST + 0x80u, rec);
    PE_StoreU16(rec + 6u, 5u);
    PE_StoreU16(rec + 12u + 0u, 0xFFFFu);
    PE_StoreU16(rec + 12u + 2u, 0x0010u);
    PE_StoreU16(rec + 12u + 4u, 0x8000u);
    PE_StoreU16(rec + 12u + 6u, 9u);
    PE_StoreU16(rec + 24u + 6u, 7u);               /* beyond the count */

    ASSERT(func_8003E0FC(HLO_ST, 9, out) == 1, "second record matches");
    ASSERT((int)PE_LoadU32(out) == -1 && PE_LoadU32(out + 4u) == 0x10u &&
           (int)PE_LoadU32(out + 8u) == -0x8000, "fields copied sign-extended");
    PE_StoreU32(out, 0x1234u);
    ASSERT(func_8003E0FC(HLO_ST, 7, out) == 0, "record past the count is not searched");
    ASSERT(PE_LoadU32(out) == 0x1234u, "a miss writes nothing");
    PASS();
}

/* func_80018660: (a + b) * 225 * 960 + c * 60, gated by d == 1. */
static void test_HANDLO_timer_seed(void)
{
    TEST("HANDLO_timer_seed");
    ResetTestState();
    HloSeedVec(4);
    PE_StoreU32(HLO_OP + 0x00u, 1u);
    PE_StoreU32(HLO_OP + 0x40u, 2u);
    PE_StoreU32(HLO_OP + 0x80u, 30u);
    PE_StoreU32(HLO_OP + 0xC0u, 1u);
    PE_StoreU32(0x800A76CCu, 0x77u);
    PE_StoreU32(0x800A76C4u, 4u);
    ASSERT(func_80018660(HLO_VEC) == 1, "returns 1");
    ASSERT(PE_LoadU32(0x800A76C8u) == 3u * 225u * 960u + 30u * 60u, "seed value");
    ASSERT(PE_LoadU32(0x800A76CCu) == 0u, "d == 1 clears D_800A76CC");
    ASSERT(PE_LoadU32(0x800A76C4u) == 3u, "|2, |1, &~4");

    PE_StoreU32(HLO_OP + 0xC0u, 0u);
    PE_StoreU32(0x800A76C4u, 0u);
    (void)func_80018660(HLO_VEC);
    ASSERT(PE_LoadU32(0x800A76C8u) == 0u && PE_LoadU32(0x800A76C4u) == 1u,
           "d != 1 zeroes the seed and sets only bit 0");
    PASS();
}

/* func_80020C74: flag updates then func_8001A680(player, 0x12), which binds
 * handler 0x12 of the player's class (D_800B0E98 0xC0-byte rows). */
static void test_HANDLO_player_release(void)
{
    TEST("HANDLO_player_release");
    const pe_addr_t handler = HLO_ST + 0x900u;
    ResetTestState();
    PE_StoreU32(0x8009D254u, HLO_ST);
    PE_StoreU8(HLO_ST + 0x0Cu, 1u);                        /* class 1 */
    PE_StoreU32(HLO_ST + 0x98u, 0xFFEFFFFFu);              /* no 0x100000 cascade */
    PE_StoreU32(HLO_ST + 0x14u, 0x77u);
    PE_StoreU32(0x800B0E98u + 0xC0u + 0x12u * 4u, handler);
    PE_StoreU8(handler + 2u, 5u);
    PE_StoreU32(0x8009D278u, HLO_ST + 0x400u);
    PE_StoreU16(0x8009D298u, 0x55u);
    func_80020C74();
    ASSERT(PE_LoadU32(0x8009D2E8u) & 1u, "D_8009D2E8 |= 1");
    ASSERT(PE_LoadU32(HLO_ST + 0x44Cu) == 0x10000u, "(*D_8009D278)+0x4C |= 0x10000");
    ASSERT(PE_LoadU16(0x8009D298u) == 0u, "D_8009D298 cleared");
    ASSERT(PE_LoadU8(HLO_ST + 0x0Eu) == 0x12u && PE_LoadU32(HLO_ST + 0x1B0u) == handler &&
           PE_LoadU8(HLO_ST + 0x0Fu) == 4u && PE_LoadU32(HLO_ST + 0x14u) == 0u,
           "handler 0x12 bound: id, address, count = byte[2] - 1, value reset");
    ASSERT(PE_LoadU32(HLO_ST + 0x98u) == (0xFFEFFFFFu & ~0x100u & ~0x200u),
           "flags lose bit 8 (release) and bit 9 (activation)");
    PASS();
}

/* func_8001A680: cascade to 0x200000-linked children of a 0x100000 body. */
static void test_HANDLO_handler_cascade(void)
{
    TEST("HANDLO_handler_cascade");
    const pe_addr_t child = HLO_ST + 0x400u, other = HLO_ST + 0x800u;
    const pe_addr_t h0 = HLO_ST + 0xC00u, h1 = HLO_ST + 0xC10u;
    ResetTestState();
    PE_StoreU8(HLO_ST + 0x0Cu, 0u);
    PE_StoreU8(child + 0x0Cu, 1u);
    PE_StoreU32(HLO_ST + 0x98u, 0x100000u);
    PE_StoreU32(0x8009D20Cu, other);
    PE_StoreU32(other + 4u, child);
    PE_StoreU32(other + 0x18Cu, HLO_ST);                   /* linked, no 0x200000 */
    PE_StoreU32(child + 0x18Cu, HLO_ST);
    PE_StoreU32(child + 0x98u, 0x200000u);
    PE_StoreU32(0x800B0E98u + 3u * 4u, h0);
    PE_StoreU32(0x800B0E98u + 0xC0u + 3u * 4u, h1);
    PE_StoreU8(h0 + 2u, 2u);
    PE_StoreU8(h1 + 2u, 9u);
    func_8001A680(HLO_ST, 0x10003u);                        /* (u16)id = 3 */
    ASSERT(PE_LoadU32(HLO_ST + 0x1B0u) == h0 && PE_LoadU8(HLO_ST + 0x0Fu) == 1u,
           "body bound to class 0 handler 3");
    ASSERT(PE_LoadU32(child + 0x1B0u) == h1 && PE_LoadU8(child + 0x0Fu) == 8u &&
           PE_LoadU8(child + 0x0Eu) == 3u, "0x200000 child bound to its class handler 3");
    ASSERT(PE_LoadU32(other + 0x1B0u) == 0u, "linked body without 0x200000 is skipped");
    PASS();
}

/* func_80033430: entry f12 -= 2, packet pushed to the matching OT + 0x1C. */
static void test_HANDLO_entry_decrement(void)
{
    TEST("HANDLO_entry_decrement");
    ResetTestState();
    PE_StoreU32(0x8009CDDCu, 1u);
    PE_StoreU16(0x8009EC38u + 0x1Cu + 0x12u, 1u);
    PE_StoreU8(0x8009D235u, 0u);
    PE_StoreU32(0x800B0E38u + 4u, HLO_ST);
    func_80033430();
    ASSERT(PE_LoadU16(0x8009EC38u + 0x1Cu + 0x12u) == 0xFFFFu, "f12 -= 2 wraps");
    ASSERT(PE_LoadU8(0x8009D235u) == 0xFFu, "D_8009D235-- wraps");
    PASS();
}

/* func_800424B4: slot/record bounds and the in-use byte. */
static void test_HANDLO_card_record_getter(void)
{
    TEST("HANDLO_card_record_getter");
    ResetTestState();
    PE_StoreU8(0x800A0ED6u + 1048u, 3u);                 /* slot 1: 3 records */
    PE_StoreU8(0x800A0EF1u + 1048u + 2u * 68u, 1u);      /* record 2 in use   */
    ASSERT(func_800424B4(1, 2) == 0x800A0EF0u + 1048u + 136u, "in-use record address");
    ASSERT(func_800424B4(1, 1) == 0u, "unused record -> 0");
    ASSERT(func_800424B4(1, 3) == 0u, "index == count -> 0");
    ASSERT(func_800424B4(1, -1) == 0u, "negative index -> 0");
    ASSERT(func_800424B4(2, 0) == 0u && func_800424B4(-1, 0) == 0u,
           "slot must be 0 or 1 (unsigned compare)");
    PASS();
}

/* func_8005DE08: skip `count` NUL-terminated strings. */
static void test_HANDLO_string_skip(void)
{
    TEST("HANDLO_string_skip");
    const pe_addr_t base = 0x800A8028u, text = 0x800A8200u;
    ResetTestState();
    PE_StoreU32(0x800A8054u, text - base);
    PE_StoreU32(0x800A804Cu, 0x100u);
    PE_StoreU8(base + 0x100u + 3u, 2u);                   /* entry 3 -> skip 2 */
    PE_StoreU8(text + 0u, 'a'); PE_StoreU8(text + 1u, 0u);
    PE_StoreU8(text + 2u, 'b'); PE_StoreU8(text + 3u, 'c'); PE_StoreU8(text + 4u, 0u);
    ASSERT(func_8005DE08(3) == text + 5u, "p ends just past the second NUL");
    ASSERT(func_8005DE08(4) == 0u, "a zero count returns 0");
    PASS();
}

/* func_8005DD8C: signed-offset table entry address with the +0x7F bias. */
static void test_HANDLO_message_entry(void)
{
    TEST("HANDLO_message_entry");
    const pe_addr_t base = 0x800A8028u, rec = 0x800A8300u, tbl = 0x800A8400u;
    ResetTestState();
    PE_StoreU32(0x800A804Cu, 0x10u);
    PE_StoreU32(0x800A802Cu, rec - base);
    PE_StoreU32(rec + 0x10u, tbl - rec);
    PE_StoreU8(base + 0x10u + 1u, 1u);          /* a0 = 1 -> idx 1 -> 0x80 */
    PE_StoreU16(tbl, 0x81u);                    /* count */
    PE_StoreU16(tbl + (0x80u + 1u) * 2u, (unsigned short)(short)-8);
    ASSERT((unsigned int)func_8005DD8C(1) == tbl - 8u, "p + (short)p[idx + 1]");
    PE_StoreU16(tbl, 0x80u);
    ASSERT(func_8005DD8C(1) == 0, "biased index >= count -> 0");
    ASSERT(func_8005DD8C(2) == 0, "zero id -> 0");
    PASS();
}

/* func_800183E8: first matching node across three lists gets bit 6. */
static void test_HANDLO_node_mark(void)
{
    TEST("HANDLO_node_mark");
    const pe_addr_t n1 = HLO_ST + 0x400u, n2 = HLO_ST + 0x500u;
    ResetTestState();
    HloSeedVec(1);
    PE_StoreU32(HLO_OP, 0x22u);
    PE_StoreU32(0x8009D2F0u, HLO_ST);
    PE_StoreU32(HLO_ST + 0xA4u, n1);            /* second list */
    PE_StoreU32(n1 + 0x24u, n2);
    PE_StoreU16(n1 + 10u, 0x21u);
    PE_StoreU16(n2 + 10u, 0x22u);
    PE_StoreU16(n2 + 8u, 0x0001u);
    ASSERT(func_800183E8(HLO_VEC) == 1, "returns 1");
    ASSERT(PE_LoadU16(n2 + 8u) == 0x41u && PE_LoadU16(n1 + 8u) == 0u,
           "only the matching node gets bit 6");
    PASS();
}

/* func_8003DD08: record copied twice (second copy at +0x20 / +0x34). */
static void test_HANDLO_double_record_copy(void)
{
    TEST("HANDLO_double_record_copy");
    const pe_addr_t a0 = HLO_ST, a1 = HLO_ST + 0x100u;
    const pe_addr_t dst = HLO_ST + 0x200u, tab = HLO_ST + 0x400u;
    unsigned k;
    ResetTestState();
    PE_StoreU32(a0 + 0x84u, dst);
    PE_StoreU32(a1 + 0x84u, tab);
    for (k = 0; k < 32u; k += 2u)
        PE_StoreU16(tab + 2u * 32u + k, (unsigned short)(0x100u + k));
    func_8003DD08(a0, a1, 2);
    ASSERT(PE_LoadU16(dst + 0x10u) == 0x110u && PE_LoadU16(dst + 0x30u) == 0x110u,
           "halfword 8 lands at +0x10 and +0x30");
    ASSERT(PE_LoadU16(dst + 0x12u) == 0u, "the source pad halfword +0x12 is not copied");
    ASSERT(PE_LoadU32(dst + 0x1Cu) == PE_LoadU32(tab + 64u + 0x1Cu) &&
           PE_LoadU32(dst + 0x3Cu) == PE_LoadU32(tab + 64u + 0x1Cu),
           "word 2 lands at +0x1C and +0x3C");
    PASS();
}

/* func_8003DF50: +0x30 = rec.04 + rec.06 (via dst +0x70). */
static void test_HANDLO_record_bind(void)
{
    TEST("HANDLO_record_bind");
    const pe_addr_t dst = HLO_ST, src = HLO_ST + 0x100u, recs = HLO_ST + 0x200u;
    ResetTestState();
    PE_StoreU32(src + 0x18u, recs);
    PE_StoreU16(recs + 16u + 0u, 0x11u);
    PE_StoreU16(recs + 16u + 2u, 0x22u);
    PE_StoreU16(recs + 16u + 4u, 0x30u);
    PE_StoreU16(recs + 16u + 6u, 0x04u);
    PE_StoreU32(dst, 0xFFFFFFFFu);
    func_8003DF50(dst, src, 1);
    ASSERT(PE_LoadU32(dst) == 0u && PE_LoadU32(dst + 0x24u) == src &&
           PE_LoadU16(dst + 0x32u) == 1u, "header: 0, source, index");
    ASSERT(PE_LoadU16(dst + 0x2Cu) == 0x11u && PE_LoadU16(dst + 0x2Eu) == 0x22u &&
           PE_LoadU16(dst + 0x70u) == 0x04u && PE_LoadU16(dst + 0x30u) == 0x34u,
           "fields, with +0x30 = rec.04 + rec.06");
    PASS();
}

/* func_8005DEE4: free-list pop + queue append, first and second node. */
static void test_HANDLO_queue_append(void)
{
    TEST("HANDLO_queue_append");
    const pe_addr_t n1 = HLO_ST, n2 = HLO_ST + 0x10u;
    ResetTestState();
    PE_StoreU32(0x8009D0DCu, n1);
    PE_StoreU32(n1, n2);
    PE_StoreU32(n2, 0u);
    func_8005DEE4(7, 8);
    ASSERT(PE_LoadU32(0x8009D0E0u) == n1 && PE_LoadU32(0x8009D0E4u) == n1 &&
           PE_LoadU32(0x8009D0DCu) == n2, "first node becomes head and tail");
    ASSERT(PE_LoadU32(n1) == 0u && PE_LoadU32(n1 + 4u) == 7u && PE_LoadU32(n1 + 8u) == 8u,
           "payload stored, link cleared");
    func_8005DEE4(9, 10);
    ASSERT(PE_LoadU32(n1) == n2 && PE_LoadU32(0x8009D0E4u) == n2 &&
           PE_LoadU32(0x8009D0E0u) == n1 && PE_LoadU32(0x8009D0DCu) == 0u,
           "second node linked after the tail");
    func_8005DEE4(1, 2);
    ASSERT(PE_LoadU32(0x8009D0E4u) == n2, "empty free list: no-op");
    PASS();
}

/* func_8005BBE4: bracket search over a 100-entry ascending table. */
static void test_HANDLO_bracket_search(void)
{
    TEST("HANDLO_bracket_search");
    pe_addr_t tbl;
    unsigned k;
    ResetTestState();
    PE_StoreU32(0x800A8038u, 0x1000u);          /* table offset for func_8005DB8C */
    tbl = func_8005DB8C(0);
    for (k = 0; k < 100u; k++)
        PE_StoreU32(tbl + k * 4u, k * 100u);
    ASSERT(func_8005BBE4(0, 1234) == 134,
           "bracket [1200,1300) ends at idx 13; r = 12 returns key - tbl[r - 1]");
    ASSERT(func_8005BBE4(0, 50) == 0, "bracket 0 returns 0");
    ASSERT(func_8005BBE4(0, 99999) == 99999 - 9700, "clamped to r = 0x62: key - tbl[0x61]");
    PASS();
}

/* func_80042170: card record re-armed only in state 1. */
static void test_HANDLO_card_load_arm(void)
{
    TEST("HANDLO_card_load_arm");
    const pe_addr_t s0 = 0x800A0ED4u + 0x418u;
    ResetTestState();
    PE_StoreU8(s0, 2u);
    ASSERT(func_80042170(1, 3) == 0 && PE_LoadU32(0x800A1854u) == 0u,
           "state != 1 leaves the record alone");
    PASS();
}

/* func_80019260 -> func_8003E0D0(state + 0x1B4): anim header +2 == 2 picks
 * mode 2 at +0x28, clears +0x24; then state+0x18C = 0. */
static void test_HANDLO_anim_reset(void)
{
    TEST("HANDLO_anim_reset");
    const pe_addr_t anim = HLO_ST + 0x1B4u, hdr = HLO_ST + 0x600u;
    ResetTestState();
    PE_StoreU32(0x8009D2F0u, HLO_ST);
    PE_StoreU32(anim, hdr);
    PE_StoreU8(hdr + 2u, 2u);
    PE_StoreU32(anim + 0x24u, 0x99u);
    PE_StoreU32(HLO_ST + 0x18Cu, 0x80100000u);
    ASSERT(func_80019260() == 1, "returns 1");
    ASSERT(PE_LoadU32(anim + 0x24u) == 0u && PE_LoadU16(anim + 0x28u) == 2u,
           "type 2 header -> mode 2");
    ASSERT(PE_LoadU32(HLO_ST + 0x18Cu) == 0u, "state+0x18C cleared");
    PE_StoreU8(hdr + 2u, 3u);
    func_8003E0D0(anim);
    ASSERT(PE_LoadU16(anim + 0x28u) == 0u, "other header types -> mode 0");
    PASS();
}

/* func_8005DF6C: masked dequeue back onto the free list, 12-byte copy-out. */
static void test_HANDLO_masked_dequeue(void)
{
    TEST("HANDLO_masked_dequeue");
    const pe_addr_t n1 = HLO_ST, n2 = HLO_ST + 0x10u, fr = HLO_ST + 0x20u;
    const pe_addr_t out = HLO_ST + 0x40u;
    ResetTestState();
    PE_StoreU32(0x8009D0E0u, n1);
    PE_StoreU32(0x8009D0E4u, n2);
    PE_StoreU32(0x8009D0DCu, fr);
    PE_StoreU32(n1, n2);  PE_StoreU32(n1 + 4u, 0x1u); PE_StoreU32(n1 + 8u, 0x11u);
    PE_StoreU32(n2, 0u);  PE_StoreU32(n2 + 4u, 0x6u); PE_StoreU32(n2 + 8u, 0x22u);

    func_8005DF6C(0x4, out);
    ASSERT(PE_LoadU32(out + 4u) == 6u && PE_LoadU32(out + 8u) == 0x22u,
           "the tail node matches mask 4 and is copied out");
    ASSERT(PE_LoadU32(out) == fr, "copied next is the old free head (retail order)");
    ASSERT(PE_LoadU32(n1) == 0u && PE_LoadU32(0x8009D0E4u) == n1 &&
           PE_LoadU32(0x8009D0DCu) == n2, "unlinked, tail moved back, freed");

    func_8005DF6C(0x8, out);
    ASSERT(PE_LoadU32(out + 4u) == 0u && PE_LoadU32(out + 8u) == 0u,
           "no match clears f4/f8");
    func_8005DF6C(0x1, 0u);   /* null out: no effect */
    ASSERT(PE_LoadU32(0x8009D0E0u) == n1, "null out leaves the queue alone");
    PASS();
}

/* func_80019F04: actor search (key, sub-key, skip bit 4) then targeting. */
static void test_HANDLO_actor_target(void)
{
    TEST("HANDLO_actor_target");
    const pe_addr_t a1 = HLO_ST + 0x400u, a2 = HLO_ST + 0x800u, a3 = HLO_ST + 0xC00u;
    ResetTestState();
    HloSeedVec(2);
    PE_StoreU32(HLO_OP + 0x00u, 5u);         /* key     */
    PE_StoreU32(HLO_OP + 0x40u, 2u);         /* sub-key */
    PE_StoreU32(0x8009D2F0u, HLO_ST);
    PE_StoreU32(0x8009D20Cu, a1);
    PE_StoreU32(a1 + 4u, a2); PE_StoreU8(a1 + 0xCu, 5u); PE_StoreU8(a1 + 0xDu, 2u);
    PE_StoreU32(a1 + 0x98u, 0x10u);          /* bit 4: skipped */
    PE_StoreU32(a2 + 4u, a3); PE_StoreU8(a2 + 0xCu, 5u); PE_StoreU8(a2 + 0xDu, 1u);
    PE_StoreU32(a3 + 4u, 0u); PE_StoreU8(a3 + 0xCu, 5u); PE_StoreU8(a3 + 0xDu, 2u);

    ASSERT(func_80019F04(HLO_VEC) == 1, "returns 1");
    ASSERT(PE_LoadU32(HLO_ST + 0x18Cu) == a3, "third actor is the first full match");
    ASSERT(PE_LoadU32(a3 + 0x98u) == 0x100000u && PE_LoadU32(HLO_ST + 0x98u) == 0x600000u,
           "target and state flags set");

    PE_StoreU32(HLO_OP + 0x40u, 9u);
    PE_StoreU32(HLO_ST + 0x18Cu, 0x1234u);
    (void)func_80019F04(HLO_VEC);
    ASSERT(PE_LoadU32(HLO_ST + 0x18Cu) == 0x1234u, "no match leaves the target alone");

    PE_StoreU32(HLO_OP + 0x00u, 0u);         /* key 0: the player */
    PE_StoreU32(0x8009D254u, a1);
    (void)func_80019F04(HLO_VEC);
    ASSERT(PE_LoadU32(HLO_ST + 0x18Cu) == a1, "key 0 targets D_8009D254");
    PASS();
}

/* func_80040210: h/m/s split, fixed-title path, formatter boundary args. */
static void test_HANDLO_save_header(void)
{
    TEST("HANDLO_save_header");
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    PE_StoreU32(0x8009EE8Cu, 0xFFFFFFFFu);
    PE_StoreU32(0x8009EE90u, 0xFFFFFFFFu);
    PE_StoreU32(0x800A1704u, 1u);
    PE_StoreU32(0x8009222Cu, 0x80012340u);
    PE_StoreU8(0x80012340u, 'A'); PE_StoreU8(0x80012341u, 'B');
    PE_StoreU8(0x80012342u, 0u);
    ASSERT(func_80040210(2, 3725) == 0x8009EE8Cu, "returns the header address");
    ASSERT(PE_LoadU32(0x8009EE90u) == 0u, "header zeroed first");
    ASSERT(PE_LoadU8(0x8009EE8Cu) == 'A' && PE_LoadU8(0x8009EE8Du) == 'B' &&
           PE_LoadU8(0x8009EE8Eu) == 0u,
           "func_8004006C formats D_8009222C's string into the header");
    ASSERT(PE_LoadU32(0x800A1708u) == 2u && PE_LoadU32(0x800A170Cu) == 1u &&
           PE_LoadU32(0x800A1710u) == 2u && PE_LoadU32(0x800A1714u) == 5u,
           "slot and 1:02:05");
    ASSERT(PE_LoadU32(0x800A1718u) == 0x800A8028u + 15u,
           "fixed title: func_8005DE08(-1) skips 15 strings");
    ASSERT(PE_Decomp_BoundaryCount() == 0, "formatter is a real call now");
    PASS();
}

/* func_80059FD0: override item list selected; both snapshots written back to
 * the id-resolved records (0x100.. -> D_800BEEAC[id], 0x200.. -> D_8009DE64). */
static void test_HANDLO_item_snapshot_restore(void)
{
    TEST("HANDLO_item_snapshot_restore");
    const pe_addr_t list = HLO_ST;
    const pe_addr_t r0 = 0x800BEEACu + (0x105u << 5), r1 = 0x8009DE64u + (0x202u << 5);
    unsigned k;
    ResetTestState();
    PE_StoreU32(0x8009D098u, 1u);
    PE_StoreU32(0x8009D09Cu, 1u);
    PE_StoreU32(0x8009D04Cu, list);
    PE_StoreU32(0x8009D054u, 3u);
    PE_StoreU16(list + 2u, 0x105u);
    PE_StoreU16(list + 4u, 0x202u);
    PE_StoreU32(0x8009D090u, 1u);
    PE_StoreU32(0x8009D094u, 2u);
    for (k = 0; k < 32u; k += 4u) {
        PE_StoreU32(0x800A204Cu + k, 0xA0000000u + k);
        PE_StoreU32(0x800A206Cu + k, 0xB0000000u + k);
    }
    func_80059FD0();
    ASSERT(PE_LoadU32(0x8009D048u) == list && PE_LoadU32(0x8009D058u) == 0x800A1F84u &&
           PE_LoadU32(0x8009D064u) == 4u && PE_LoadU32(0x8009D050u) == 3u,
           "override list configuration");
    ASSERT(PE_LoadU32(r0) == 0xA0000000u && PE_LoadU32(r0 + 28u) == 0xA000001Cu,
           "slot 0 snapshot restored into the 0x100-range record");
    ASSERT(PE_LoadU32(r1) == 0xB0000000u && PE_LoadU32(r1 + 28u) == 0xB000001Cu,
           "slot 1 snapshot restored into the 0x200-range record");
    PASS();
}

/* func_8003E474: UV/CLUT scroll over GT4 pairs then GT3 pairs; GT4 u3 is
 * deliberately left alone (leaf quirk). */
static void test_HANDLO_mesh_uv_scroll(void)
{
    TEST("HANDLO_mesh_uv_scroll");
    const pe_addr_t m = HLO_ST, hdr = HLO_ST + 0x100u, prims = HLO_ST + 0x200u;
    const pe_addr_t gt3 = prims + 2u * 0x34u;
    ResetTestState();
    PE_StoreU32(m, hdr);
    PE_StoreU32(m + 0x54u, prims);
    PE_StoreU16(hdr + 8u, 1u);              /* one GT4 pair */
    PE_StoreU16(hdr + 0xAu, 1u);            /* one GT3 pair */
    PE_StoreU8(prims + 0x0Cu, 0xFEu);       /* u0 wraps */
    PE_StoreU16(prims + 0x0Eu, 0x7F00u);
    PE_StoreU8(prims + 0x34u + 0x31u, 3u);  /* second GT4: v3 */
    PE_StoreU8(prims + 0x34u + 0x30u, 9u);  /* second GT4: u3 */
    PE_StoreU8(gt3 + 0x28u + 0x25u, 1u);    /* second GT3: v2 */

    func_8003E474(m, 4, 2, (signed char)-1);
    ASSERT(PE_LoadU8(prims + 0x0Cu) == 0x02u && PE_LoadU8(prims + 0x0Du) == 2u,
           "u0 += du (byte wrap), v0 += dv");
    ASSERT(PE_LoadU16(prims + 0x0Eu) == 0x7EFFu, "clut += (signed char)dc");
    ASSERT(PE_LoadU8(prims + 0x34u + 0x31u) == 5u && PE_LoadU8(prims + 0x34u + 0x30u) == 9u,
           "GT4 v3 moves, u3 does not");
    ASSERT(PE_LoadU8(gt3 + 0x28u + 0x25u) == 3u && PE_LoadU8(gt3 + 0x28u + 0x24u) == 4u,
           "GT3 pair uses the 0x28 stride after the GT4s");
    PASS();
}

/* The ten draw-callback VMAs the lo hand adapters store into menu nodes now
 * dispatch through func_800638D8's switch instead of hitting its
 * "unported drawing callback" stop.  State is seeded so each real callee
 * takes a short, memory-safe path. */
extern void PE_Menu_DispatchDrawCallback(pe_addr_t fn, uint32_t value);

/* Minimal message archive for func_8005DC4C: HDR+4 -> sub-chunk at
 * 0x800A8128, whose +4 word (0x10) puts the table at 0x800A8138; all 0x200
 * entries point at one empty (0xFF-terminated) string. */
static void HloSeedMessages(void)
{
    const pe_addr_t ptr = 0x800A8128u, tbl = ptr + 0x10u;
    unsigned i;
    PE_StoreU32(0x800A802Cu, ptr - 0x800A8028u);
    PE_StoreU32(ptr + 4u, tbl - ptr);
    PE_StoreU16(tbl, 0x200u);
    for (i = 0; i < 0x200u; i++)
        PE_StoreU16(tbl + 2u + i * 2u, 0x600u);
    PE_StoreU8(tbl + 0x600u, 0xFFu);
}

static void test_HANDLO_menu_draw_dispatch(void)
{
    TEST("HANDLO_menu_draw_dispatch");
    static const struct { pe_addr_t fn; uint32_t value; } cases[] = {
        {0x80050690u, 0u}, {0x800506E8u, 0u}, {0x80050708u, 0u},
        {0x80050728u, 0u}, {0x80050DC0u, 0u}, {0x80050E70u, 0u},
        {0x80050618u, 0u}, {0x800434C0u, 0u},
        {0x8004F2E4u, HLO_ST + 0x800u}, {0x8004FEECu, HLO_ST + 0x800u},
    };
    unsigned k;
    for (k = 0; k < sizeof(cases) / sizeof(cases[0]); k++) {
        ResetTestState();
        PE_StoreU32(0x8009CEF4u, HLO_ST);   /* zeroed list node            */
        PE_StoreU32(0x8009D07Cu, HLO_ST);   /* item list: id 0 -> no draw  */
        PE_StoreU32(0x8009D048u, HLO_ST);   /* inventory list (zero ids)   */
        PE_StoreU32(0x8009D058u, HLO_ST);   /* owned-flag bitmap           */
        HloSeedMessages();
        PE_StoreU32(0x8009D100u, HLO_ST + 0x2000u);  /* packet arena (D_8009D104 limit) */
        PE_StoreU32(0x8009D104u, HLO_ST + 0x2000u);
        PE_StoreU32(0x8009D11Cu, HLO_ST + 0x1F00u);  /* ordering-table entry */
        PE_Menu_DispatchDrawCallback(cases[k].fn, cases[k].value);
        ASSERT(!PE_Port_ShouldStop(), "callback must not request the unresolved stop");
        ASSERT(CountOrderLog("PE_MenuDrawCallback") == 0,
               "callback must not reach the unported-callback provider");
    }
    /* A list draw really ran: func_8004FEEC latches its node. */
    ASSERT(PE_LoadU32(0x8009CEF4u) == HLO_ST + 0x800u, "8004FEEC set D_8009CEF4");
    PASS();
}

/* func_80042264: CRC-16/CCITT (init 0xFFFF, poly 0x1021) over the 0x2000
 * bytes at D_8009EED0, computed after the stored checksum word (just past the
 * 0x12E4-byte save block) is consumed and zeroed; ~crc & 0xFFFF must equal
 * it.  The menu pool (func_80062F9C) and a minimal message archive let the
 * real result dialogs run. */
static unsigned HloCrc16(pe_addr_t base, unsigned len)
{
    unsigned crc = 0xFFFFu, i, b;
    for (i = 0; i < len; i++) {
        crc ^= (unsigned)PE_LoadU8(base + i) << 8;
        for (b = 0; b < 8u; b++)
            crc = (crc & 0x8000u) ? ((crc << 1) ^ 0x1021u) : (crc << 1);
        crc &= 0xFFFFu;
    }
    return crc;
}

static void HloSeedCardVerify(unsigned stored_delta)
{
    /* func_8003FBD8 (absent_save_port.c) consumes the 0x8A8-byte field
     * state after the 0x12E4 snapshot, so the checksum word follows it. */
    const pe_addr_t tab = 0x8009EED0u, word = 0x8009EFD0u + 0x12E4u + 0x8A8u;
    unsigned i;
    ResetTestState();HostFB_Init();PE_GPU_Init();
    func_80062F9C();
    HloSeedMessages();
    PE_StoreU32(0x8009D100u, 0x80160000u);
    PE_StoreU32(0x8009D104u, 0x80160000u);
    PE_StoreU32(0x8009D11Cu, 0x80185000u);
    PE_Decomp_ResetBoundaries();
    for (i = 0; i < 0x2000u; i += 4u)
        PE_StoreU32(tab + i, i * 2654435761u);
    PE_StoreU32(0x8009EFD0u + 0x10u, 0xABCDEF01u);  /* inside the CRC range */
    PE_StoreU32(word, 0u);
    PE_StoreU32(word, (~HloCrc16(tab, 0x2000u) & 0xFFFFu) + stored_delta);
}

static void test_HANDLO_card_crc_verify(void)
{
    TEST("HANDLO_card_crc_verify");
    const pe_addr_t word = 0x8009EFD0u + 0x12E4u + 0x8A8u;

    HloSeedCardVerify(0u);
    func_80042264();
    ASSERT(PE_LoadU32(0x800C0DE0u + 0x10u) == 0xABCDEF01u, "save block snapshot copied");
    ASSERT(PE_LoadU32(word) == 0u, "stored checksum word consumed (zeroed)");
    ASSERT(PE_LoadU32(0x800A0ED0u) == word + 4u, "cursor advanced past block + word");
    ASSERT(PE_LoadU32(0x8009CFFCu) == 0x80042228u, "success dialog callback armed");
    ASSERT(PE_LoadU32(0x800A185Cu) == 1u, "matching checksum marks the load valid");
    ASSERT(PE_LoadU32(0x8009D1A0u) == (PE_LoadU32(0x8009EFD0u + 0x12E4u + 0x808u) & 0xFFFF2679u),
           "func_8003FBD8 restored D_8009D1A0 (masked) before the checksum");

    HloSeedCardVerify(1u);                         /* off by one: mismatch */
    func_80042264();
    ASSERT(PE_LoadU32(0x800A185Cu) == 0u, "mismatch leaves D_800A185C clear");
    ASSERT(PE_LoadU32(0x8009CFFCu) == 0u, "mismatch arms no success callback");
    PASS();
}

/* func_800588EC: item-list view selection and key-item 0x204 insertion. */
static void test_HANDLO_item_view_select(void)
{
    TEST("HANDLO_item_view_select");
    const pe_addr_t kl = 0x800C1F80u, inv = HLO_ST;
    ResetTestState();
    func_800588EC(0);
    ASSERT(PE_LoadU32(0x8009D07Cu) == 0x800C1EB8u && PE_LoadU32(0x8009D080u) == 0x64u &&
           PE_LoadU32(0x8009D04Cu) == 0x800C1EB8u && PE_LoadU32(0x8009D054u) == 0x64u,
           "a0 == 0: the 100-entry list, mirrored into D_8009D04C/54");

    ResetTestState();
    PE_StoreU16(kl + 7u * 2u, 0x204u);
    func_800588EC(1);
    ASSERT(PE_LoadU32(0x8009D080u) == 0x51u, "0x204 already present: 0x51 entries");

    ResetTestState();
    PE_StoreU16(kl + 0u, 0x11u);                  /* slot 0 used, slot 1 free */
    PE_StoreU32(0x8009D048u, inv);
    PE_StoreU32(0x8009D050u, 2u);
    PE_StoreU16(inv + 2u, 0x101u);                /* inventory[1] = id 0x101 */
    PE_StoreU8(0x800BEEACu + (0x101u << 5) + 6u, 6u);  /* type 6 */
    func_800588EC(1);
    ASSERT(PE_LoadU16(kl + 2u) == 0x204u && PE_LoadU32(0x8009D080u) == 0x51u,
           "type-6 item owned: 0x204 inserted in the first free slot");

    ResetTestState();
    PE_StoreU32(0x8009D048u, inv);
    PE_StoreU32(0x8009D050u, 1u);
    PE_StoreU16(inv, 0x101u);
    PE_StoreU8(0x800BEEACu + (0x101u << 5) + 6u, 5u);  /* not type 6 */
    func_800588EC(1);
    ASSERT(PE_LoadU16(kl) == 0u && PE_LoadU32(0x8009D080u) == 0x50u,
           "no type-6 item: nothing inserted, 0x50 entries");
    PASS();
}

/* func_8005D020: remove the first 0x61 one-shot item, re-point D_800C0E20 at
 * the first type-7 item.  (The type scan dereferences the record of every
 * slot it passes, as retail does, so the type-7 item sits first.) */
static void test_HANDLO_consume_one_shot(void)
{
    TEST("HANDLO_consume_one_shot");
    const pe_addr_t inv = 0x800C0E48u;
    ResetTestState();
    HloSeedMessages();
    PE_StoreU32(0x8009D100u, 0x80160000u);
    PE_StoreU32(0x8009D104u, 0x80160000u);
    PE_StoreU32(0x8009D11Cu, 0x80185000u);
    D_8009D018 = 0u;
    PE_StoreU32(0x8009D018u, 0u);
    PE_StoreU8(0x800C0E0Cu, 2u);                  /* func_80052F70() -> 2 */
    PE_StoreU16(inv + 0u, 0x106u);                /* type-7 weapon      */
    PE_StoreU16(inv + 2u, 0x105u);                /* 0x61 one-shot item */
    PE_StoreU8(0x800BEEACu + (0x106u << 5) + 6u, 7u);
    PE_StoreU8(0x800BEEB0u + 0x105u * 0x20u, 0x61u);
    PE_StoreU8(0x800C0EACu + (5u << 5), 0x33u);
    PE_StoreU8(0x800C0E22u, 0xFFu);               /* not equipped */
    PE_StoreU32(0x8009D028u, 9u);

    func_8005D020();
    ASSERT(PE_LoadU32(0x8009D050u) == 2u && PE_LoadU32(0x8009D048u) == inv,
           "inventory list selected");
    ASSERT(PE_LoadU16(inv + 2u) == 0u && PE_LoadU8(0x800C0EACu + (5u << 5)) == 0u,
           "one-shot removed and its ownership byte cleared");
    ASSERT(PE_LoadU16(inv) == 0x106u, "other entries untouched");
    ASSERT(PE_LoadU8(0x800C0E20u) == 0u && PE_LoadU32(0x8009D028u) == 0u,
           "D_800C0E20 re-pointed at the type-7 slot, D_8009D028 cleared");
    PASS();
}

/* func_8002F300: palette bytes, scene flags, player velocity reset; the
 * D_8009D1A0 & 0x1800 gate skips the handler/effect spawn. */
static void test_HANDLO_sewer_exit_setup(void)
{
    TEST("HANDLO_sewer_exit_setup");
    ResetTestState();
    PE_StoreU32(0x8009D254u, HLO_ST);
    PE_StoreU32(0x8009D278u, HLO_ST + 0x400u);
    PE_StoreU32(HLO_ST + 0x68u, 5u);
    PE_StoreU32(HLO_ST + 0x98u, 0x101u);
    PE_StoreU32(0x8009D1A0u, 0x800u);
    D_8009D1A0 = 0x800u;
    func_8002F300();
    ASSERT(PE_LoadU8(0x800B00EDu) == 0x46u && PE_LoadU8(0x800B692Eu) == 0xF9u &&
           PE_LoadU8(0x800B0162u) == 0x01u && PE_LoadU8(0x800B01B8u) == 0x83u,
           "gradient bytes written");
    ASSERT(PE_LoadU32(0x8009D28Cu) == 2u && (PE_LoadU32(0x8009D2E8u) & 1u),
           "scene state 2 and D_8009D2E8 bit 0");
    ASSERT(PE_LoadU32(HLO_ST + 0x68u) == 0u && PE_LoadU32(HLO_ST + 0x98u) == 0x001u,
           "velocity cleared, +0x98 bit 8 cleared");
    ASSERT(PE_LoadU32(0x800B0CD8u) & 0x8000u, "D_800B0CD8 bit 15 set");
    ASSERT(PE_LoadU32(HLO_ST + 0x1B0u) == 0u, "gated: handler 0x14 not bound");
    PASS();
}

/* func_8004D6D4: confirm with no card record (or no cell) is refused
 * (func_800526C4) with the cell latched in D_8009CF48 and no dialog built;
 * cancel closes without touching D_8009CF48. */
static void test_HANDLO_card_slot_update(void)
{
    TEST("HANDLO_card_slot_update");
    pe_addr_t win;
    ResetTestState();HostFB_Init();PE_GPU_Init();
    func_80062F9C();
    HloSeedMessages();
    win = func_80062D2C(0x25u, 0u, 0u, 0u);
    (void)func_8006322C(0x25u, win, win);
    PE_StoreU32(0x8009CF44u, 0u);                 /* slot 0: no records */
    PE_StoreU32(0x8009CF48u, 0x77u);
    PE_StoreU32(0x8009CF14u, 0u);
    ASSERT(func_8004D6D4((int)win, 0x10000) == 1, "confirm returns 1");
    ASSERT(PE_LoadU32(0x8009CF48u) == (uint32_t)func_8006346C(func_80062A20(win, 0u)),
           "func_8006346C's cell result is latched in D_8009CF48");
    ASSERT(PE_LoadU32(0x8009CF14u) == 0u, "no overwrite dialog without a record");

    PE_StoreU32(0x8009CF48u, 0x55u);
    ASSERT(func_8004D6D4((int)win, 0x40) == 1, "cancel returns 1");
    ASSERT(PE_LoadU32(0x8009CF48u) == 0x55u, "cancel leaves D_8009CF48 alone");
    ASSERT(!PE_Port_ShouldStop(), "no unresolved stop on either path");
    PASS();
}

static void test_HANDLO_all(void)
{
    test_HANDLO_card_slot_update();
    test_HANDLO_item_view_select();
    test_HANDLO_consume_one_shot();
    test_HANDLO_sewer_exit_setup();
    test_HANDLO_card_crc_verify();
    test_HANDLO_menu_draw_dispatch();
    test_HANDLO_mesh_uv_scroll();
    test_HANDLO_item_snapshot_restore();
    test_HANDLO_save_header();
    test_HANDLO_actor_target();
    test_HANDLO_masked_dequeue();
    test_HANDLO_anim_reset();
    test_HANDLO_double_record_copy();
    test_HANDLO_record_bind();
    test_HANDLO_queue_append();
    test_HANDLO_bracket_search();
    test_HANDLO_card_load_arm();
    test_HANDLO_card_record_getter();
    test_HANDLO_string_skip();
    test_HANDLO_message_entry();
    test_HANDLO_node_mark();
    test_HANDLO_untarget_list_walk();
    test_HANDLO_keyed_record_search();
    test_HANDLO_timer_seed();
    test_HANDLO_player_release();
    test_HANDLO_handler_cascade();
    test_HANDLO_entry_decrement();
    test_HANDLO_all_bits_set_predicate();
    test_HANDLO_scaled_high_half();
    test_HANDLO_state_field_copy();
    test_HANDLO_two_level_byte_getter();
    test_HANDLO_dynamic_bit_set();
    test_HANDLO_record_base_sum();
    test_HANDLO_flag_writers();
    test_HANDLO_boundary_short_operands();
}
