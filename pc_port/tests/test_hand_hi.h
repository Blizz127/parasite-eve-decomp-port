/*
 * Hand-adapter regression tests — pc_port/game/decomp_hand/ leaves with
 * VRAM >= 0x80060000 (port_stubs lane).  Every expectation is computed from
 * the matched leaf src/func_XXXXXXXX.c named in the assertion, not from the
 * port: masks, field positions, access widths, signedness and loop shape.
 */
#include "pe_guest_decomp.h"
#include "pe_spu_dma.h"

extern int32_t func_80077CF4(int32_t angle);
extern int32_t func_80077DC4(int32_t angle);

#define HH_SCR 0x80153000u   /* guest scratch for these tests */

/* Sequence-opcode fixture: a voice record at HH_SCR whose cursor (word 0)
 * points at the operand bytes placed at HH_SCR + 0x800. */
static pe_addr_t seq_fixture(const unsigned char *bytes, unsigned n)
{
    const pe_addr_t rec = HH_SCR, data = HH_SCR + 0x800u;

    for (unsigned i = 0; i < n; i++)
        PE_StoreU8(data + i, bytes[i]);
    PE_StoreU32(rec, data);
    return rec;
}

static void test_HANDHI_seq_ops(void)
{
    TEST("HANDHI_seq_ops");
    ResetTestState();
    pe_addr_t a0;

    /* src/func_8008FBD4.c: +0xDE = (s8) byte, cursor + 1. */
    { const unsigned char b[] = {0xFE}; a0 = seq_fixture(b, 1); }
    func_8008FBD4(a0);
    ASSERT((int16_t)PE_LoadU16(a0 + 0xDEu) == -2 && PE_LoadU32(a0) == HH_SCR + 0x801u,
           "FBD4: sign-extended byte, cursor advanced");
    /* src/func_8008FBFC.c: += (s8) byte. */
    { const unsigned char b[] = {0x05}; a0 = seq_fixture(b, 1); }
    func_8008FBFC(a0);
    ASSERT(PE_LoadU16(a0 + 0xDEu) == 3u, "FBFC: -2 + 5");

    /* src/func_8008F880.c: (v - 1) & 0xF wraps 0 -> 15. */
    PE_StoreU16(HH_SCR + 0x7Cu, 0u);
    func_8008F880(HH_SCR);
    ASSERT(PE_LoadU16(HH_SCR + 0x7Cu) == 15u, "F880: 4-bit wrap");

    /* src/func_8008F430.c: relative jump by s16 -4 from p + 2. */
    { const unsigned char b[] = {0xFC, 0xFF}; a0 = seq_fixture(b, 2); }
    func_8008F430(a0);
    ASSERT(PE_LoadU32(a0) == HH_SCR + 0x800u + 2u - 4u, "F430: cursor = p + 2 + (s16)off");

    /* src/func_8008F470.c: condition false skips 3 bytes. */
    PE_StoreU32(0x8009D2C8u, HH_SCR + 0x400u);
    PE_StoreU16(HH_SCR + 0x456u, 1u);
    { const unsigned char b[] = {5, 0x10, 0x00}; a0 = seq_fixture(b, 3); }
    func_8008F470(a0);
    ASSERT(PE_LoadU32(a0) == HH_SCR + 0x803u, "F470: +0x56 < b0 skips the operand");
    { const unsigned char b[] = {1, 0x10, 0x00}; a0 = seq_fixture(b, 3); }
    func_8008F470(a0);
    ASSERT(PE_LoadU32(a0) == HH_SCR + 0x803u + 0x10u, "F470: taken jump from after the operand");

    /* src/func_8008F514.c: level 0x4000 -> target 0x50 << 8 over 0 (-> 0x100) steps. */
    PE_StoreU16(HH_SCR + 0x6Cu, 0x40FFu);
    { const unsigned char b[] = {0, 0x50}; a0 = seq_fixture(b, 2); }
    func_8008F514(a0);
    ASSERT(PE_LoadU16(a0 + 0x6Eu) == 0x100u && PE_LoadU16(a0 + 0x6Cu) == 0x4000u &&
           PE_LoadU16(a0 + 0xD4u) == (uint16_t)((0x5000 - 0x4000) / 0x100),
           "F514: steps 0 -> 0x100, level masked, step");

    /* src/func_8008FC78.c: returns 1, +0x82 = 0x100 for 0. */
    { const unsigned char b[] = {0}; a0 = seq_fixture(b, 1); }
    ASSERT(func_8008FC78(a0) == 1 && PE_LoadU16(a0 + 0x82u) == 0x100u && PE_LoadU16(a0 + 0x84u) == 1u,
           "FC78: 0 -> 0x100, +0x84 = 1");

    /* src/func_80090970.c: clamp into [1, 0xFF]. */
    PE_StoreU16(HH_SCR + 0xD0u, 0xF0u);
    { const unsigned char b[] = {0x20}; a0 = seq_fixture(b, 1); }
    func_80090970(a0);
    ASSERT(PE_LoadU16(a0 + 0xD2u) == 0xFFu, "90970: clamped high");
    { const unsigned char b[] = {0x00}; a0 = seq_fixture(b, 1); }
    func_80090970(a0);
    ASSERT(PE_LoadU16(a0 + 0xD2u) == 0u, "90970: 0 stays 0");

    /* src/func_8009071C.c / 8009090C.c / 80090754.c: loop push, repeat, end. */
    PE_StoreU16(HH_SCR + 0xCEu, 3u);
    PE_StoreU32(HH_SCR, HH_SCR + 0x900u);
    func_8009071C(HH_SCR);
    ASSERT(PE_LoadU16(HH_SCR + 0xCEu) == 0u && PE_LoadU32(HH_SCR + 4u) == HH_SCR + 0x900u &&
           PE_LoadU16(HH_SCR + 0x62u) == 0u, "9071C: slot (3 + 1) & 3 = 0 saves the cursor");
    PE_StoreU32(HH_SCR, HH_SCR + 0x950u);
    ASSERT(func_8009090C(HH_SCR) == (int)(HH_SCR + 0x900u) && PE_LoadU16(HH_SCR + 0x62u) == 1u,
           "9090C: counter bumped, cursor rewound");
    { const unsigned char b[] = {2}; seq_fixture(b, 1); }
    PE_StoreU32(HH_SCR + 4u, HH_SCR + 0x900u);
    func_80090754(HH_SCR);                           /* counter 1 -> 2 == n */
    ASSERT(PE_LoadU16(HH_SCR + 0xCEu) == 3u, "90754: loop done pops the slot");

    /* src/func_80090A20.c: D_8009D2C8 fields. */
    { const unsigned char b[] = {7, 9}; a0 = seq_fixture(b, 2); }
    func_80090A20(a0);
    ASSERT(PE_LoadU16(HH_SCR + 0x460u) == 7u && PE_LoadU16(HH_SCR + 0x45Cu) == 9u, "90A20: +0x60/+0x5C");

    /* src/func_800904C4.c: relative transpose on the main track. */
    PE_StoreU16(HH_SCR + 0x54u, 0u);
    PE_StoreU16(HH_SCR + 0x45Au, 0x3Eu);
    { const unsigned char b[] = {0x43}; a0 = seq_fixture(b, 1); }
    func_800904C4(a0);
    ASSERT(PE_LoadU16(HH_SCR + 0x45Au) == ((0x3Eu + 3u) & 0x3Fu), "904C4: (cur + 3) & 0x3F");

    /* src/func_80083E84.c / 800835C0.c: tag 'L' with a0[0xE4]. */
    PE_StoreU8(HH_SCR + 0x46u, 3u);
    PE_StoreU8(HH_SCR + 0xE4u, 0x5Au);
    func_800835C0(HH_SCR);
    ASSERT(PE_LoadU8(HH_SCR + 0x36u) == 0x4Cu && PE_LoadU8(HH_SCR + 0x24u) == 0x5Au &&
           PE_LoadU32(HH_SCR + 0x2Cu) == HH_SCR + 0x24u && PE_LoadU8(HH_SCR + 0x35u) == 1u,
           "835C0: state 3 -> 'L' with a0[0xE4]");

    /* src/func_80085D84.c: exchange. */
    PE_StoreU32(0x8009B438u, 4u);
    ASSERT(func_80085D84(4) == 4, "85D84: unchanged returns old, no call");
    PASS();
}

static void test_HANDHI_spu_voice(void)
{
    TEST("HANDHI_spu_voice");
    ResetTestState();

    /* func_80087798: voice 3 -> offsets 0x30/0x32, both masked to 0x7FFF. */
    func_80087798(3, 0xFFFF, 0x8001);
    ASSERT(PE_SpuRegister_LoadU16(0x30u) == 0x7FFFu, "87798: p[0] = a1 & 0x7FFF");
    ASSERT(PE_SpuRegister_LoadU16(0x32u) == 0x0001u, "87798: p[1] = a2 & 0x7FFF");

    /* func_800877BC: 0x1F801C04 + (a0 << 4). */
    func_800877BC(2, 0x1234u);
    ASSERT(PE_SpuRegister_LoadU16(0x24u) == 0x1234u, "877BC: +4 of voice 2");

    /* func_8008783C: (v & 0xFF0F) | (a1 << 4) at +8. */
    PE_SpuRegister_StoreU16(0x18u, 0xABCDu);
    func_8008783C(1, 0x7);
    ASSERT(PE_SpuRegister_LoadU16(0x18u) == 0xAB7Du, "8783C: nibble 1 replaced");

    /* func_80087864: (v & 0xFFF0) | a1 at +8. */
    PE_SpuRegister_StoreU16(0x18u, 0xABCDu);
    func_80087864(1u, 0x3u);
    ASSERT(PE_SpuRegister_LoadU16(0x18u) == 0xABC3u, "87864: low nibble replaced");

    /* func_8008780C: keeps only the LOW BYTE (lbu), then
     * low | ((a2 >> 2) << 15) | (a1 << 8), truncated to 16 bits. */
    PE_SpuRegister_StoreU16(0x08u, 0xFF5Au);
    func_8008780C(0, 0x12, 5u);           /* (5>>2)<<15 = 0x8000 */
    ASSERT(PE_SpuRegister_LoadU16(0x08u) == (uint16_t)(0x5Au | 0x8000u | 0x1200u),
           "8780C: high byte rebuilt from a1/a2, low byte kept");

    /* func_8008788C: (*reg & 0x3F) | ((mode >> 1) << 14) | (field << 6). */
    PE_SpuRegister_StoreU16(0x0Au, 0xFFFFu);
    func_8008788C(0, 0x7Fu, 3u);
    ASSERT(PE_SpuRegister_LoadU16(0x0Au) == (uint16_t)(0x3Fu | 0x4000u | (0x7Fu << 6)),
           "8788C: field/mode packed over the low 6 bits");

    /* func_80087728: halfwords at 0x1F801D8C (low) / 0x1F801D8E (high). */
    func_80087728(0x00ABCDEFu);
    ASSERT(PE_SpuRegister_LoadU16(0x18Cu) == 0xCDEFu, "87728: low half at 0x1D8C");
    ASSERT(PE_SpuRegister_LoadU16(0x18Eu) == 0x00ABu, "87728: high half at 0x1D8E");
    PASS();
}

static void test_HANDHI_stream_cursor(void)
{
    TEST("HANDHI_stream_cursor");
    ResetTestState();
    const pe_addr_t rec = HH_SCR, data = HH_SCR + 0x200u;

    PE_StoreU8(data + 0u, 0x9Au);
    PE_StoreU8(data + 1u, 0xF0u);
    PE_StoreU32(rec, data);
    PE_StoreU32(rec + 0xF4u, 0x1u);

    func_8009059C(rec);                   /* byte -> +0x110, |= 0x1000 */
    ASSERT(PE_LoadU32(rec) == data + 1u, "9059C: cursor post-incremented");
    ASSERT(PE_LoadU16(rec + 0x110u) == 0x009Au, "9059C: byte zero-extended to +0x110");
    ASSERT(PE_LoadU32(rec + 0xF4u) == 0x1001u, "9059C: flag 0x1000 ORed");

    func_80090AAC((int)rec, 0x55);        /* 9059C(a0) then 905C4(a0) */
    ASSERT(PE_LoadU32(rec) == data + 3u, "90AAC: both readers advance the cursor");
    ASSERT(PE_LoadU16(rec + 0x110u) == 0x00F0u, "90AAC: first byte via 9059C");
    ASSERT(PE_LoadU32(rec + 0xF4u) == 0x9001u, "90AAC: 905C4 ORs 0x8000");
    PASS();
}

static void test_HANDHI_misc(void)
{
    TEST("HANDHI_misc");
    ResetTestState();

    /* func_8008D820: arg2 >> 2 words, forward copy. */
    for (unsigned i = 0; i < 4u; i++) PE_StoreU32(HH_SCR + 4u * i, 0x11110000u + i);
    PE_StoreU32(HH_SCR + 0x100u + 12u, 0xDEADBEEFu);
    func_8008D820(HH_SCR, HH_SCR + 0x100u, 12u);
    ASSERT(PE_LoadU32(HH_SCR + 0x100u) == 0x11110000u, "8D820: word 0");
    ASSERT(PE_LoadU32(HH_SCR + 0x108u) == 0x11110002u, "8D820: word 2");
    ASSERT(PE_LoadU32(HH_SCR + 0x10Cu) == 0xDEADBEEFu, "8D820: 12 bytes = 3 words only");

    /* func_800824DC: swap D_800B8AB8. */
    PE_StoreU32(0x800B8AB8u, 7u);
    ASSERT(func_800824DC(9) == 7, "824DC: returns the old value");
    ASSERT(PE_LoadU32(0x800B8AB8u) == 9u, "824DC: stores the new value");

    /* func_80080B44: lba 0 -> 00:02:00; 4500*10+75*59+74-150 -> 10:59:74 BCD. */
    ASSERT(func_80080B44(0, HH_SCR) == HH_SCR, "80B44: returns arg1");
    ASSERT(PE_LoadU8(HH_SCR) == 0x00 && PE_LoadU8(HH_SCR + 1u) == 0x02 &&
           PE_LoadU8(HH_SCR + 2u) == 0x00, "80B44: lba 0 is 00:02:00");
    func_80080B44(4500 * 10 + 75 * 59 + 74 - 150, HH_SCR);
    ASSERT(PE_LoadU8(HH_SCR) == 0x10 && PE_LoadU8(HH_SCR + 1u) == 0x59 &&
           PE_LoadU8(HH_SCR + 2u) == 0x74, "80B44: BCD minutes/seconds/frames");

    /* func_800850C0: D_8009D24C = 1 before the callback registration. */
    PE_StoreU32(0x8009D24Cu, 0u);
    func_800850C0();
    ASSERT(PE_LoadU32(0x8009D24Cu) == 1u, "850C0: D_8009D24C = 1");

    /* func_8007FCBC: D_8009B598 > 0 short-circuits with 0 and no store. */
    PE_StoreU32(0x8009B598u, 1u);
    PE_StoreU32(0x8009B570u, 0u);
    ASSERT(func_8007FCBC(0x1FF, 0) == 0, "7FCBC: busy returns 0");
    ASSERT(PE_LoadU32(0x8009B570u) == 0u, "7FCBC: busy path stores nothing");

    /* func_8008AF08: gate on D_8009D2C8->+4; bit 0x100 enables the 0x18-entry
     * walk (stride 0x11C from D_800B6B80 + 0x5A), lowering >= 0x50 by 0x30. */
    PE_StoreU32(0x8009D2C8u, HH_SCR + 0x400u);
    PE_StoreU32(HH_SCR + 0x400u, 0x100u);
    PE_StoreU32(HH_SCR + 0x404u, 1u);
    PE_StoreU16(0x800B8AC0u + 0x5Au, 0x50u);            /* copied to B6B80 */
    PE_StoreU16(0x800B8AC0u + 0x5Au + 0x11Cu, 0x4Fu);
    PE_StoreU16(0x800B8AC0u + 0x5Au + 0x11Cu * 0x17u, 0x90u);
    func_8008AF08();
    ASSERT(PE_LoadU32(0x800B8968u) == 0x100u, "8AF08: record copied to D_800B8968");
    ASSERT(PE_LoadU16(0x800B6B80u + 0x5Au) == 0x20u, "8AF08: 0x50 lowered by 0x30");
    ASSERT(PE_LoadU16(0x800B6B80u + 0x5Au + 0x11Cu) == 0x4Fu, "8AF08: < 0x50 untouched");
    ASSERT(PE_LoadU16(0x800B6B80u + 0x5Au + 0x11Cu * 0x17u) == 0x60u,
           "8AF08: last (24th) entry walked");
    PASS();
}

static void test_HANDHI_small(void)
{
    TEST("HANDHI_small");
    ResetTestState();

    /* SPU control pairs (src/func_8008770C/87744/87760/8777C.c). */
    func_8008770C(0x11112222u);
    func_80087744(0x33334444u);
    func_80087760(0x55556666u);
    func_8008777C(0x77778888u);
    ASSERT(PE_SpuRegister_LoadU16(0x188u) == 0x2222u &&
           PE_SpuRegister_LoadU16(0x18Au) == 0x1111u, "8770C: [0xEC4]/[0xEC5]");
    ASSERT(PE_SpuRegister_LoadU16(0x198u) == 0x4444u &&
           PE_SpuRegister_LoadU16(0x19Au) == 0x3333u, "87744: [0xECC]/[0xECD]");
    ASSERT(PE_SpuRegister_LoadU16(0x194u) == 0x6666u &&
           PE_SpuRegister_LoadU16(0x196u) == 0x5555u, "87760: [0xECA]/[0xECB]");
    ASSERT(PE_SpuRegister_LoadU16(0x190u) == 0x8888u &&
           PE_SpuRegister_LoadU16(0x192u) == 0x7777u, "8777C: [0xEC8]/[0xEC9]");

    /* src/func_800877D4.c / 800877F0.c: a1 >> 3 at +6 / +0xE. */
    func_800877D4(1, 0x80008u);
    func_800877F0(1, 0x18u);
    ASSERT(PE_SpuRegister_LoadU16(0x16u) == 0x0001u, "877D4: (a1 >> 3) truncated to u16");
    ASSERT(PE_SpuRegister_LoadU16(0x1Eu) == 0x0003u, "877F0: a1 >> 3 at +0xE");

    /* src/func_80089F08.c through a guest-RAM D_8009B3FC base. */
    PE_StoreU32(0x8009B3FCu, HH_SCR);
    PE_StoreU16(HH_SCR + 0x2Cu, 0xBEEFu);
    func_80089F08(2u, HH_SCR + 0x100u);
    ASSERT(PE_LoadU16(HH_SCR + 0x100u) == 0xBEEFu, "89F08: base + (a0 << 4) + 0xC");

    ASSERT(func_8007A354() == 0x8009AFD0u, "7A354: &D_8009AFD0");
    ASSERT(func_8007FC28() == 0x8009B582u, "7FC28: &D_8009B582");
    func_8007FBC0(0x80012345u);
    ASSERT(PE_LoadU32(0x800A36A0u) == 0x80012345u, "7FBC0: D_800A36A0 = cb");

    PE_StoreU32(0x8009AFB4u, 0x80011111u);
    ASSERT(func_8007A4A8(0x80022222u) == 0x80011111u &&
           PE_LoadU32(0x8009AFB4u) == 0x80022222u, "7A4A8: callback exchange");
    PE_StoreU32(0x8009B6D0u, 0x80033333u);
    ASSERT(func_80081254(0u) == 0x80033333u && PE_LoadU32(0x8009B6D0u) == 0u,
           "81254: callback exchange");
    PE_StoreU32(0x8009B708u, 5u);
    ASSERT(func_80081E5C(6) == 5 && PE_LoadU32(0x8009B708u) == 6u, "81E5C: swap");
    PE_StoreU32(0x800B8AB0u, 5u);
    ASSERT(func_800824B4(-1) == 5 && PE_LoadU32(0x800B8AB0u) == 0xFFFFFFFFu, "824B4: swap");

    PE_StoreU32(0x800E2248u, 0x80160000u);
    ASSERT(func_800C2B10(3) == 0x80160014u, "C2B10: base + 4*i + 8");
    ASSERT(func_800C2B28(3) == 0x80160054u, "C2B28: base + 4*i + 0x48");

    func_80085728(0x80170000u);
    ASSERT(PE_LoadU32(0x8009D240u) == 0x80170000u &&
           PE_LoadU32(0x8009D260u) == 0x80170800u, "85728: base and base + 0x800");

    PE_StoreU32(HH_SCR + 4u, 8u);
    ASSERT(func_80071944(HH_SCR) == HH_SCR + 0xCu, "71944: flag 8 -> &value (+0xC)");
    ASSERT(func_800719C4(HH_SCR) == HH_SCR + 0x14u, "719C4: flag 8 -> &value (+0x14)");
    PE_StoreU32(HH_SCR + 4u, 7u);
    ASSERT(func_80071944(HH_SCR) == 0u && func_800719C4(HH_SCR) == 0u,
           "71944/719C4: flag clear -> NULL");
    PASS();
}

static void test_HANDHI_batch3(void)
{
    TEST("HANDHI_batch3");
    ResetTestState();

    func_8007FBCC(0x80010001u); func_8007FBD8(0x80010002u); func_8007FBE4(0x80010003u);
    ASSERT(PE_LoadU32(0x800A36A4u) == 0x80010001u && PE_LoadU32(0x800A36A8u) == 0x80010002u &&
           PE_LoadU32(0x800A36ACu) == 0x80010003u, "7FBCC/D8/E4: callback slots");

    /* src/func_8008C16C.c / 8008C270.c: signed byte at +4, << 16. */
    PE_StoreU8(HH_SCR + 4u, 0xFEu);                  /* -2 */
    PE_StoreU16(0x8009D220u, 0x1234u); PE_StoreU16(0x8009D21Eu, 0x1234u);
    func_8008C16C(HH_SCR);
    func_8008C270(HH_SCR);
    ASSERT(PE_LoadU16(0x8009D220u) == 0u && PE_LoadU32(0x8009D2D0u) == 0xFFFE0000u,
           "8C16C: sign-extended byte << 16, D_8009D220 cleared");
    ASSERT(PE_LoadU16(0x8009D21Eu) == 0u && PE_LoadU32(0x8009D2CCu) == 0xFFFE0000u,
           "8C270: sign-extended byte << 16, D_8009D21E cleared");

    /* src/func_80074330.c: arg1 words zeroed, arg1 == 0 is a no-op. */
    for (unsigned i = 0; i < 4u; i++) PE_StoreU32(HH_SCR + 4u * i, 0xFFFFFFFFu);
    func_80074330(HH_SCR, 3);
    func_8007474C(HH_SCR + 12u, 0);
    ASSERT(PE_LoadU32(HH_SCR) == 0u && PE_LoadU32(HH_SCR + 8u) == 0u &&
           PE_LoadU32(HH_SCR + 12u) == 0xFFFFFFFFu, "74330: exactly arg1 words; 0 is no-op");

    /* src/func_80077A28.c: byte fill. */
    func_80077A28(HH_SCR + 0x20u, 0xA5u, 5);
    ASSERT(PE_LoadU8(HH_SCR + 0x24u) == 0xA5u && PE_LoadU8(HH_SCR + 0x25u) == 0u,
           "77A28: fills exactly arg2 bytes");

    /* src/func_80078C94.c: a0[5..7] = a1[0..2]. */
    PE_StoreU32(HH_SCR + 0x40u, 1u); PE_StoreU32(HH_SCR + 0x44u, 2u); PE_StoreU32(HH_SCR + 0x48u, 3u);
    ASSERT(func_80078C94(HH_SCR + 0x80u, HH_SCR + 0x40u) == HH_SCR + 0x80u, "78C94: returns a0");
    ASSERT(PE_LoadU32(HH_SCR + 0x94u) == 1u && PE_LoadU32(HH_SCR + 0x9Cu) == 3u,
           "78C94: words land at +0x14..+0x1C");

    /* src/func_80082ADC.c: two handler words, the words either side cleared. */
    PE_StoreU32(0x800A5AB0u, 9u); PE_StoreU32(0x800A5ABCu, 9u);
    func_80082ADC();
    ASSERT(PE_LoadU32(0x800A5AB4u) == 0x80082B70u && PE_LoadU32(0x800A5AB8u) == 0x80082B08u &&
           PE_LoadU32(0x800A5AB0u) == 0u && PE_LoadU32(0x800A5ABCu) == 0u,
           "82ADC: p[0]/p[1] handlers, p[-1]/p[2] zero");

    /* src/func_800C7D00.c -> src/func_800C2AF0.c: D_800E2248 = base + 12,
     * (base + 12)[index + 18] = value, return 0. */
    ASSERT(func_800CE118(HH_SCR + 0x200u, 0, 2, 0xCAFEu, 7, 8) == 0, "CE118: returns 0");
    ASSERT(PE_LoadU32(0x800E2248u) == HH_SCR + 0x20Cu, "CE118: D_800E2248 = base + 3 words");
    ASSERT(PE_LoadU32(HH_SCR + 0x20Cu + 4u * 20u) == 0xCAFEu, "CE118: slot index + 18");

    /* src/func_80071964.c / 80071994.c. */
    PE_StoreU32(HH_SCR + 0x304u, 8u); PE_StoreU32(HH_SCR + 0x308u, 0x40u);
    ASSERT(func_80071964(HH_SCR + 0x300u) == HH_SCR + 0x300u + 8u + 0x40u + 4u,
           "71964: +8 + skip + 4 when flag 8");
    PE_StoreU32(HH_SCR + 0x304u, 0u);
    ASSERT(func_80071994(HH_SCR + 0x300u) == HH_SCR + 0x300u + 8u + 12u,
           "71994: +8 + 12 without flag");

    /* src/func_8008D7D0.c: the D_800C0D90 attribute seed (the apply step,
     * func_80085F74, is covered by its generated TU). */
    PE_StoreU16(0x8009D2B6u, 0x3FFFu);
    PE_StoreU32(0x8009B3FCu, HH_SCR + 0x400u);        /* guest-RAM register mirror */
    func_8008D7D0();
    ASSERT(PE_LoadU32(0x800C0D90u) == 0x1C0u && PE_LoadU32(0x800C0DA4u) == 0u &&
           PE_LoadU16(0x800C0DA0u) == 0x3FFFu && PE_LoadU16(0x800C0DA2u) == 0x3FFFu,
           "8D7D0: mask 0x1C0 and both volumes from D_8009D2B6");
    PASS();
}

static void test_HANDHI_batch4(void)
{
    TEST("HANDHI_batch4");
    ResetTestState();

    /* src/func_800878C0.c: (v & 0xFFC0) | ((mode >> 2) << 5) | low at +0xA. */
    PE_SpuRegister_StoreU16(0x1Au, 0xFFFFu);
    func_800878C0(1, 0x5u, 8u);
    ASSERT(PE_SpuRegister_LoadU16(0x1Au) == (uint16_t)(0xFFC0u | (2u << 5) | 5u),
           "878C0: low 6 bits rebuilt");

    /* src/func_8007A400.c / 8007A434.c: bounded table getters. */
    PE_StoreU32(0x8009AFDCu + 4u * 0x1Bu, 0x11u);
    PE_StoreU32(0x8009B05Cu + 4u * 6u, 0x22u);
    ASSERT(func_8007A400(0x11Bu) == 0x11u, "7A400: index masked to 8 bits");
    ASSERT(func_8007A400(0x1Cu) == 0x800119CCu, "7A400: >= 0x1C -> &D_800119CC");
    ASSERT(func_8007A434(6u) == 0x22u && func_8007A434(7u) == 0x800119CCu,
           "7A434: bound 7");

    /* src/func_8007C444.c: word 0 of records a0..a0+a1-1 (stride 32). */
    PE_StoreU32(0x800C0DC8u, HH_SCR);
    for (unsigned i = 0; i < 5u; i++) PE_StoreU32(HH_SCR + 32u * i, 0xFFu);
    func_8007C444(1, 3u);
    ASSERT(PE_LoadU32(HH_SCR) == 0xFFu && PE_LoadU32(HH_SCR + 32u) == 0u &&
           PE_LoadU32(HH_SCR + 96u) == 0u && PE_LoadU32(HH_SCR + 128u) == 0xFFu,
           "7C444: records 1..3 cleared");

    /* src/func_8008594C.c: index masked to 16 bits, bound 3, stride 16. */
    PE_StoreU32(0x8009B7D0u, HH_SCR + 0x100u);
    PE_StoreU16(HH_SCR + 0x120u, 0xFFFFu);
    ASSERT(func_8008594C(0x10002) == 1 && PE_LoadU16(HH_SCR + 0x120u) == 0u,
           "8594C: (a0 & 0xFFFF) == 2 clears entry 2");
    ASSERT(func_8008594C(3) == 0, "8594C: >= 3 rejected");

    /* src/func_80083790.c. */
    PE_StoreU8(HH_SCR + 0xE3u, 4u);          /* ((4+1)>>1)<<2 = 8 */
    PE_StoreU8(HH_SCR + 0xE9u, 3u);          /* (15+3)&0xFFC = 16, +4 = 20 */
    PE_StoreU32(HH_SCR + 0xECu, 0x80100000u);
    ASSERT(func_80083790(HH_SCR) == (int)(0x80100000u + 8u + 20u), "83790: offset formula");

    /* src/func_800C811C.c: a2[2..6]. */
    PE_StoreU16(0x800E2348u, 0x11u); PE_StoreU16(0x800E234Cu, 0x33u);
    PE_StoreU32(0x8009D254u, HH_SCR + 0x200u); PE_StoreU16(HH_SCR + 0x22Eu, 0x22u);
    func_800C811C(0, 0, HH_SCR + 0x300u);
    ASSERT(PE_LoadU16(HH_SCR + 0x304u) == 0x7Fu && PE_LoadU16(HH_SCR + 0x306u) == 0x224u &&
           PE_LoadU16(HH_SCR + 0x308u) == 0x11u && PE_LoadU16(HH_SCR + 0x30Au) == 0x22u &&
           PE_LoadU16(HH_SCR + 0x30Cu) == 0x33u, "C811C: a2[2..6]");

    /* src/func_800CC284.c: y - 0x64, a2[2] = 0, byte +3 = 0x7F (written after a2[2]). */
    PE_StoreU16(0x800E2290u, 1u); PE_StoreU16(0x800E2292u, 0x100u); PE_StoreU16(0x800E2294u, 3u);
    func_800CC284(0, 0, HH_SCR + 0x400u);
    ASSERT(PE_LoadU16(HH_SCR + 0x406u) == 1u && PE_LoadU16(HH_SCR + 0x408u) == 0x9Cu &&
           PE_LoadU16(HH_SCR + 0x404u) == 0u && PE_LoadU8(HH_SCR + 0x403u) == 0x7Fu &&
           PE_LoadU16(HH_SCR + 0x40Au) == 3u, "CC284: fields incl. byte +3");

    /* src/func_80082400.c: a0 != 2 -> D_800B28F8 = 2. */
    PE_StoreU32(0x800B28F8u, 0u);
    func_80082400(1);
    ASSERT(PE_LoadU32(0x800B28F8u) == 2u, "82400: non-2 path stores 2");

    /* src/func_8008AB1C.c: table lookups with the 0xFFFF NULL sentinel. */
    PE_StoreU32(0x8009D240u, HH_SCR + 0x500u);
    PE_StoreU32(0x8009D260u, 0x80180000u);
    PE_StoreU16(HH_SCR + 0x500u + 4u * 0x3u, 0x40u);     /* entry 6 */
    PE_StoreU16(HH_SCR + 0x500u + 4u * 0x3u + 2u, 0xFFFFu);
    func_8008AB1C(HH_SCR + 0x600u, HH_SCR + 0x604u, 0x403u); /* & 0x3FF -> 3 */
    ASSERT(PE_LoadU32(HH_SCR + 0x600u) == 0x80180040u, "8AB1C: base + offset");
    ASSERT(PE_LoadU32(HH_SCR + 0x604u) == 0u, "8AB1C: 0xFFFF -> NULL");

    /* src/func_8008B084.c: parameter reset. */
    func_8008B084(HH_SCR + 0x700u);
    ASSERT(PE_LoadU32(HH_SCR + 0x704u) == 0x400u && PE_LoadU32(HH_SCR + 0x708u) == 0x1000000u &&
           PE_LoadU32(HH_SCR + 0x70Cu) == 0x80u && PE_LoadU32(HH_SCR + 0x710u) == 0x7Fu,
           "8B084: arg0[1..4]");
    PASS();
}

static void test_HANDHI_batch5(void)
{
    TEST("HANDHI_batch5");
    ResetTestState();

    /* src/func_8008CB08.c: D_800B8628 + n * 0x24, then n++. */
    PE_StoreU32(0x8009D2F4u, 2u);
    func_8008CB08(HH_SCR);
    ASSERT(PE_LoadU32(HH_SCR) == 0x800B8628u + 0x48u && PE_LoadU32(0x8009D2F4u) == 3u,
           "8CB08: cursor = base + n*0x24, n incremented");

    /* src/func_8008AB9C.c: bits of D_8009D2C8[1] mark records from a0 + 0xF4. */
    PE_StoreU32(0x8009D2C8u, HH_SCR + 0x100u);
    PE_StoreU32(HH_SCR + 0x104u, 0x5u);                  /* records 0 and 2 */
    func_8008AB9C(HH_SCR + 0x1000u);
    ASSERT(PE_LoadU32(HH_SCR + 0x1000u + 0xF4u) == 3u &&
           PE_LoadU32(HH_SCR + 0x1000u + 0xF4u + 0x11Cu) == 0u &&
           PE_LoadU32(HH_SCR + 0x1000u + 0xF4u + 0x238u) == 3u,
           "8AB9C: set bits OR 3 into +0xF4 of their 0x11C records");

    /* src/func_80080CDC.c: NULL -> func_8007FC28(); else copy the quad. */
    PE_StoreU32(0x8009B582u - 2u, 0u);
    PE_StoreU8(0x8009B582u, 0x12u); PE_StoreU8(0x8009B583u, 0x34u);
    PE_StoreU8(0x8009B584u, 0x56u); PE_StoreU8(0x8009B585u, 0x78u);
    ASSERT(func_80080CDC(0u) == 0x8009B582u, "80CDC: NULL returns the source quad");
    ASSERT(func_80080CDC(HH_SCR + 0x200u) == HH_SCR + 0x200u &&
           PE_LoadU32(HH_SCR + 0x200u) == 0x78563412u, "80CDC: quad copied");

    /* src/func_80083D9C.c: case 3. */
    PE_StoreU8(HH_SCR + 0x346u, 3u);
    func_80083D9C(HH_SCR + 0x300u);
    ASSERT(PE_LoadU8(HH_SCR + 0x336u) == 0x4Du && PE_LoadU8(HH_SCR + 0x335u) == 6u &&
           PE_LoadU32(HH_SCR + 0x32Cu) == HH_SCR + 0x35Du, "83D9C: case 3 fields");

    /* src/func_80085A04.c. */
    PE_StoreU32(0x8009B424u, 2u);
    ASSERT(func_80085A04(0, HH_SCR + 0x400u) == 0, "85A04: a0 <= 0 returns 0");
    ASSERT(func_80085A04(9, HH_SCR + 0x400u) == 9, "85A04: returns a0");
    ASSERT(PE_LoadU32(HH_SCR + 0x400u) == 0x40001010u &&
           PE_LoadU32(HH_SCR + 0x404u) == (0x40000u - 0x1010u) &&
           PE_LoadU32(0x8009B464u) == HH_SCR + 0x400u && PE_LoadU32(0x8009B45Cu) == 9u,
           "85A04: header words and globals");

    /* src/func_800DFB20.c: countdown only when +0x1A >= +0xF; -1 is sticky. */
    PE_StoreU32(HH_SCR + 0x508u, HH_SCR + 0x600u);
    PE_StoreU16(HH_SCR + 0x514u, 2u);
    PE_StoreU8(HH_SCR + 0x518u, 0xFBu);                  /* -5 */
    PE_StoreU8(HH_SCR + 0x60Fu, 4u);
    PE_StoreU16(HH_SCR + 0x61Au, 4u);
    func_800DFB20(HH_SCR + 0x500u);
    ASSERT(PE_LoadU32(HH_SCR + 0x614u) == 0xFFFFFFFBu && PE_LoadU16(HH_SCR + 0x514u) == 1u,
           "DFB20: b->f14 = (s8)a0->f18, counter decremented");
    PE_StoreU16(HH_SCR + 0x514u, 0xFFFFu);
    func_800DFB20(HH_SCR + 0x500u);
    ASSERT(PE_LoadU16(HH_SCR + 0x514u) == 0xFFFFu, "DFB20: -1 is not decremented");

    /* src/func_800C6584.c: strict < on the squared XZ distance. */
    PE_StoreU16(HH_SCR + 0x700u, 3u); PE_StoreU16(HH_SCR + 0x704u, 4u);
    PE_StoreU16(HH_SCR + 0x708u, 0u); PE_StoreU16(HH_SCR + 0x70Cu, 0u);
    ASSERT(func_800C6584(HH_SCR + 0x700u, 2, HH_SCR + 0x708u, 3) == 0, "C6584: 25 < 25 false");
    ASSERT(func_800C6584(HH_SCR + 0x700u, 3, HH_SCR + 0x708u, 3) == 1, "C6584: 25 < 36 true");

    /* src/func_800629BC.c: second node's word [5] matches. */
    PE_StoreU32(0x8009D154u, HH_SCR + 0x800u);
    PE_StoreU32(HH_SCR + 0x800u, HH_SCR + 0x900u);
    PE_StoreU32(HH_SCR + 0x900u, 0u);
    PE_StoreU32(HH_SCR + 0x900u + 20u, 77u);
    ASSERT(func_800629BC(77) == HH_SCR + 0x900u, "629BC: finds word [5] in node 2");
    ASSERT(func_800629BC(78) == 0u, "629BC: miss -> NULL");

    /* src/func_80084644.c: reset only when +0x49 != 0. */
    PE_StoreU8(HH_SCR + 0xA49u, 1u); PE_StoreU32(HH_SCR + 0xA14u, 5u);
    func_80084644(HH_SCR + 0xA00u);
    ASSERT(PE_LoadU8(HH_SCR + 0xA49u) == 0u && PE_LoadU32(HH_SCR + 0xA14u) == 0u &&
           PE_LoadU8(HH_SCR + 0xA5Du) == 0xFFu && PE_LoadU8(HH_SCR + 0xA62u) == 0xFFu &&
           PE_LoadU8(HH_SCR + 0xA63u) == 0u, "84644: fields cleared, 6 bytes 0xFF");

    /* src/func_80089250.c with a guest-RAM register mirror: voice 0 masked,
     * voice 1 reads its +0xC register. */
    PE_StoreU32(0x8009D2C8u, HH_SCR + 0xB00u);
    PE_StoreU32(HH_SCR + 0xB04u, 1u); PE_StoreU32(HH_SCR + 0xB0Cu, 1u);
    PE_StoreU32(0x8009B3FCu, HH_SCR + 0xC00u);
    for (unsigned v = 0; v < 24u; v++) PE_StoreU16(HH_SCR + 0xC00u + v * 16u + 0xCu, 0x100u + v);
    func_80089250(0u);
    ASSERT(PE_LoadU16(0x800B002Cu) == 0x7FFFu, "89250: masked voice -> 0x7FFF");
    ASSERT(PE_LoadU16(0x800B002Cu + 8u) == 0x101u, "89250: voice 1 reads +0xC register");
    PASS();
}

static void test_HANDHI_batch6(void)
{
    TEST("HANDHI_batch6");
    ResetTestState();

    /* src/func_800686A0.c: entry i and mirror entry i + count. */
    PE_StoreU16(HH_SCR + 0x26u, 2u);
    PE_StoreU32(HH_SCR + 0x30u, HH_SCR + 0x100u);
    func_800686A0(HH_SCR, 0x11, 0x22, 0x133);
    ASSERT(PE_LoadU8(HH_SCR + 0x114u) == 0x11u && PE_LoadU8(HH_SCR + 0x115u) == 0x22u &&
           PE_LoadU8(HH_SCR + 0x116u) == 0x33u, "686A0: entry 1 bytes (truncated a3)");
    ASSERT(PE_LoadU8(HH_SCR + 0x134u) == 0x11u && PE_LoadU8(HH_SCR + 0x144u) == 0u,
           "686A0: mirror entry 3 written, entry 4 untouched");

    /* src/func_8008A354.c: a0 != 0 must equal +0x54; then 24 voice records. */
    PE_StoreU32(0x8009D2C8u, HH_SCR + 0x200u);
    PE_StoreU16(HH_SCR + 0x254u, 7u);
    func_8008A354(6, HH_SCR + 0x1000u);
    ASSERT(PE_LoadU32(HH_SCR + 0x218u) == 0u, "8A354: id mismatch does nothing");
    func_8008A354(7, HH_SCR + 0x1000u);
    ASSERT(PE_LoadU32(HH_SCR + 0x218u) == 0xFFFFFFu, "8A354: +0x18 = 0xFFFFFF");
    ASSERT(PE_LoadU32(HH_SCR + 0x1000u + 0x11Cu * 23u) == 0x8009B8F4u &&
           PE_LoadU32(HH_SCR + 0x1000u + 0x11Cu * 23u + 0xF4u) == 0x4400u &&
           PE_LoadU16(HH_SCR + 0x1000u + 0x116u) == 5u, "8A354: 24th record reset");

    /* src/func_8008C55C.c: mode 1, D_8009D2C8 restored after the bank swap. */
    PE_StoreU32(0x8009D2C8u, HH_SCR + 0x200u);
    PE_StoreU32(0x800BCD50u, 0u);
    func_8008C55C();
    ASSERT(PE_LoadU32(0x8009D2C0u) == 1u && PE_LoadU32(0x8009D2C8u) == HH_SCR + 0x200u,
           "8C55C: mode stored, record pointer restored");

    /* src/func_8006E590.c: 2 entries, second matches. */
    PE_StoreU32(HH_SCR + 0x404u, 0x10u);
    PE_StoreU32(HH_SCR + 0x400u + 0x10u + 0x30u, (2u << 22) | 0x80u);
    PE_StoreU16(HH_SCR + 0x480u + 0xAu, 1u);
    PE_StoreU16(HH_SCR + 0x48Cu + 0xAu, 9u);
    PE_StoreU32(HH_SCR + 0x48Cu + 4u, 0xFF000040u);
    ASSERT(func_8006E590(HH_SCR + 0x400u, 9) == HH_SCR + 0x440u, "6E590: offset & 0xFFFFFF");
    ASSERT(func_8006E590(HH_SCR + 0x400u, 5) == 0u, "6E590: miss -> NULL");

    /* src/func_800C8BC0.c: integrate, gravity, bounce, countdown. */
    PE_StoreU16(HH_SCR + 0x50Au, (uint16_t)-2);
    PE_StoreU16(HH_SCR + 0x512u, 5u);
    PE_StoreU8(HH_SCR + 0x502u, 0u);
    func_800C8BC0(0, HH_SCR + 0x600u, HH_SCR + 0x500u);
    ASSERT(PE_LoadU16(HH_SCR + 0x50Au) == 3u, "C8BC0: y += vy");
    ASSERT((int16_t)PE_LoadU16(HH_SCR + 0x512u) == -8, "C8BC0: vy + 3 then bounced");
    ASSERT(PE_LoadU8(HH_SCR + 0x502u) == 0xFFu && PE_LoadU8(HH_SCR + 0x601u) == 2u,
           "C8BC0: countdown wrapped from 0 -> a1[1] = 2");

    /* src/func_800CD5EC.c: frame byte reaching 8 flags a1[1]. */
    PE_StoreU8(HH_SCR + 0x703u, 7u);
    PE_StoreU16(HH_SCR + 0x718u, 1u); PE_StoreU16(HH_SCR + 0x728u, 4u);
    func_800CD5EC(0, HH_SCR + 0x800u, HH_SCR + 0x700u);
    ASSERT(PE_LoadU16(HH_SCR + 0x718u) == 5u, "CD5EC: second SVEC advanced");
    ASSERT(PE_LoadU8(HH_SCR + 0x801u) == 2u, "CD5EC: c >= 8 -> a1[1] = 2");
    PASS();
}

static void test_HANDHI_batch7(void)
{
    TEST("HANDHI_batch7");
    ResetTestState();

    /* src/func_8008E840.c: note 12*8+5 -> octave 8 (<< 2), entry 5; tune. */
    PE_StoreU32(0x800B2910u + 0x40u + 5u * 4u, 0x1000u);
    ASSERT(func_8008E840(1, 0x100u + 12u * 8u + 5u, 0) == 0x4000u, "8E840: octave 8 shifts left 2");
    ASSERT(func_8008E840(1, 12u * 4u + 5u, 0) == 0x400u, "8E840: octave 4 shifts right 2");
    ASSERT(func_8008E840(1, 12u * 6u + 5u, 0x40) == 0x1800u, "8E840: fine tune v += v*a2 >> 7");

    /* src/func_8007F608.c: D_800A3530 has priority; its +4 byte returned. */
    PE_StoreU32(0x800A3530u, 1u); PE_StoreU8(0x800A3534u, 0x5Au);
    PE_StoreU32(0x800A3520u, 1u);
    ASSERT(func_8007F608((int)HH_SCR) == 0x5A, "7F608: D_800A3530 slot returns its +4 byte");
    ASSERT(PE_LoadU32(0x800A3530u) == 0u && PE_LoadU32(0x800A3520u) == 1u,
           "7F608: only the chosen slot is cleared");
    PE_StoreU32(0x800A3520u, 0u);
    ASSERT(func_8007F608((int)HH_SCR) == 0, "7F608: no ready slot -> 0");

    /* src/func_800CDF4C.c. */
    PE_StoreU8(HH_SCR + 0x101u, 0x1Fu);
    PE_StoreU16(HH_SCR + 0x100u + 0x1Eu, 10u); PE_StoreU16(HH_SCR + 0x100u + 0x4Eu, 5u);
    func_800CDF4C(0, HH_SCR + 0x200u, HH_SCR + 0x100u);
    ASSERT(PE_LoadU16(HH_SCR + 0x11Eu) == 15u, "CDF4C: x[7] += dx[7]");
    ASSERT(PE_LoadU8(HH_SCR + 0x101u) == 0x0Fu && PE_LoadU8(HH_SCR + 0x201u) == 2u,
           "CDF4C: f1 faded below 0x10 -> a1[1] = 2");

    /* src/func_800C4E50.c: first ring vertex at angle 0 is (sin 0, cos 0) * f10. */
    PE_StoreU32(HH_SCR + 0x300u, HH_SCR + 0x400u);
    PE_StoreU16(HH_SCR + 0x30Cu, 4u);
    PE_StoreU16(HH_SCR + 0x30Eu, 0x100u);
    PE_StoreU16(HH_SCR + 0x310u, 0x200u);
    func_800C4E50(HH_SCR + 0x300u);
    ASSERT((int16_t)PE_LoadU16(HH_SCR + 0x400u) == (int16_t)((func_80077CF4(0) * 0x200) >> 12) &&
           (int16_t)PE_LoadU16(HH_SCR + 0x402u) == (int16_t)((func_80077DC4(0) * 0x200) >> 12),
           "C4E50: vertex 0 of ring 1 uses f10");
    ASSERT((int16_t)PE_LoadU16(HH_SCR + 0x400u + 4u * 8u + 2u) ==
           (int16_t)((func_80077DC4(0) * 0x100) >> 12), "C4E50: ring 2 starts at 0 with fE");
    ASSERT((int16_t)PE_LoadU16(HH_SCR + 0x408u) ==
           (int16_t)((func_80077CF4(0x400) * 0x200) >> 12), "C4E50: step = 0x1000 / fC");
    PASS();
}

static void test_HANDHI_batch8(void)
{
    TEST("HANDHI_batch8");
    ResetTestState();

    /* func_80061B80 / func_800CC7AC end in draw-packet callees
     * (func_80061878, func_800C2EAC..func_800C3B04) that need a live menu
     * packet pool; they are exercised through their callers, not here. */

    /* src/func_80083C3C.c: group 0 has 2 slots, limit 1. */
    PE_StoreU8(HH_SCR + 0x1E9u, 1u);
    PE_StoreU32(HH_SCR + 0x120u, HH_SCR + 0x300u);
    PE_StoreU32(HH_SCR + 0x104u, HH_SCR + 0x340u);
    for (unsigned j = 0; j < 6u; j++) PE_StoreU8(HH_SCR + 0x300u + j, (j == 1 || j == 4) ? 0u : 9u);
    PE_StoreU8(HH_SCR + 0x342u, 1u);
    for (unsigned j = 0; j < 6u; j++) PE_StoreU8(HH_SCR + 0x15Du + j, 0x55u);
    ASSERT(func_80083C3C(HH_SCR + 0x100u) == 0, "83C3C: returns 0");
    /* cnt = 2, lim = 1: slot 1 -> k (cnt !< lim), slot 4 -> k. */
    ASSERT(PE_LoadU8(HH_SCR + 0x15Eu) == 0u && PE_LoadU8(HH_SCR + 0x161u) == 0u &&
           PE_LoadU8(HH_SCR + 0x15Du) == 0x55u, "83C3C: over-limit group keeps k");
    ASSERT(PE_LoadU8(HH_SCR + 0x146u) == 0xFEu, "83C3C: +0x46 = 0xFE");
    PE_StoreU8(HH_SCR + 0x342u, 3u);
    func_80083C3C(HH_SCR + 0x100u);
    /* cnt = 2 < lim = 3: slot 1 -> 0xFF (cnt 1), slot 4 -> 0xFF. */
    ASSERT(PE_LoadU8(HH_SCR + 0x15Eu) == 0xFFu && PE_LoadU8(HH_SCR + 0x161u) == 0xFFu,
           "83C3C: under-limit group cleared to 0xFF");

    PASS();
}

static void test_HANDHI_batch9(void)
{
    TEST("HANDHI_batch9");
    ResetTestState();

    /* src/func_800DFB78.c: sub[0xD] bit 1 consumes obj->+0x98 bit 19. */
    PE_StoreU32(HH_SCR + 8u, HH_SCR + 0x100u);
    PE_StoreU8(HH_SCR + 0xCu + 0xDu, 2u);
    PE_StoreU32(HH_SCR + 0x198u, 0x80001u);
    ASSERT(func_800DFB78(HH_SCR) == 1 && PE_LoadU32(HH_SCR + 0x198u) == 1u,
           "DFB78: bit 19 consumed -> 1");
    PE_StoreU32(HH_SCR + 0x100u, 0u);
    ASSERT(func_800DFB78(HH_SCR) == 0, "DFB78: nothing pending, no record -> 0");
    PE_StoreU32(HH_SCR + 0x100u, HH_SCR + 0x200u);
    PE_StoreU32(HH_SCR + 0x200u, 2u);               /* (2 >> 1) & 7 = 1 */
    ASSERT(func_800DFB78(HH_SCR) == 1, "DFB78: record count bits -> 1");

    /* src/func_8008B1FC.c: id 0 -> bank 0 volume word. */
    PE_StoreU32(0x8009D2C8u, HH_SCR + 0x300u);
    PE_StoreU32(0x800BCD50u, 0u);
    PE_StoreU32(HH_SCR + 0x404u, 0x1FFu);
    PE_StoreU32(HH_SCR + 0x410u, 0u);
    func_8008B1FC(HH_SCR + 0x400u);
    ASSERT(PE_LoadU32(HH_SCR + 0x348u) == 0x7F0000u, "8B1FC: (v & 0x7F) << 16 in bank 0");
    /* id matching +0xBC -> bank 1, pointer restored. */
    PE_StoreU16(HH_SCR + 0x354u, 1u); PE_StoreU16(HH_SCR + 0x3BCu, 9u);
    PE_StoreU32(HH_SCR + 0x410u, 9u);
    PE_StoreU32(HH_SCR + 0x404u, 0x05u);
    func_8008B1FC(HH_SCR + 0x400u);
    ASSERT(PE_LoadU32(HH_SCR + 0x3B0u) == 0x50000u && PE_LoadU32(0x8009D2C8u) == HH_SCR + 0x300u,
           "8B1FC: bank 1 word, D_8009D2C8 restored");

    /* src/func_800625B8.c: pop from the free list, link into a1's slot 1. */
    PE_StoreU32(0x8009D158u, HH_SCR + 0x500u);
    PE_StoreU32(HH_SCR + 0x500u, HH_SCR + 0x600u);
    PE_StoreU32(0x8009D154u, 0x80123450u);
    PE_StoreU32(HH_SCR + 0x708u, 1u);               /* a1 slot 0 used */
    PE_StoreU32(HH_SCR + 0x70Cu, 0u);
    ASSERT(func_800625B8(42, HH_SCR + 0x700u) == HH_SCR + 0x500u, "625B8: returns the node");
    ASSERT(PE_LoadU32(0x8009D158u) == HH_SCR + 0x600u && PE_LoadU32(0x8009D154u) == HH_SCR + 0x500u &&
           PE_LoadU32(HH_SCR + 0x500u) == 0x80123450u && PE_LoadU32(HH_SCR + 0x504u) == 42u,
           "625B8: lists relinked, +4 = a0");
    ASSERT(PE_LoadU32(HH_SCR + 0x70Cu) == HH_SCR + 0x500u, "625B8: stored in a1's first free slot");
    PASS();
}

static void test_HANDHI_batch10(void)
{
    TEST("HANDHI_batch10");
    ResetTestState();

    /* src/func_800C7F60.c: template n - 1 through an identity matrix. */
    const pe_addr_t owner = HH_SCR, inner = HH_SCR + 0x100u, mat = inner + 0x260u;
    PE_StoreU32(0x800E279Cu, owner);
    PE_StoreU32(owner + 0x238u, inner);
    for (unsigned i = 0; i < 9u; i++) PE_StoreU16(mat + i * 2u, (i % 4u == 0u) ? 0x1000u : 0u);
    for (unsigned i = 0; i < 3u; i++) PE_StoreU32(mat + 0x14u + i * 4u, 0u);
    PE_StoreU32(0x8009D254u, HH_SCR + 0x400u);
    PE_StoreU32(HH_SCR + 0x400u, HH_SCR + 0x410u);
    PE_StoreU32(HH_SCR + 0x410u + 0x68u, HH_SCR + 0x480u);
    PE_StoreU16(HH_SCR + 0x486u, 2u);                       /* n = 2 -> entry 1 */
    PE_StoreU16(0x800E08A8u + 8u, 10u); PE_StoreU16(0x800E08A8u + 10u, 20u);
    PE_StoreU16(0x800E08A8u + 12u, 30u);
    PE_StoreU16(0x800E2348u, 100u); PE_StoreU16(0x800E234Au, 200u); PE_StoreU16(0x800E234Cu, 300u);
    func_800C7F60(0, 0, HH_SCR + 0x600u);
    ASSERT(PE_LoadU16(HH_SCR + 0x608u) == 110u && PE_LoadU16(HH_SCR + 0x60Au) == 220u &&
           PE_LoadU16(HH_SCR + 0x60Cu) == 330u, "C7F60: base + transformed template entry n-1");
    ASSERT(PE_LoadU32(HH_SCR + 0x610u) == PE_LoadU32(mat) && PE_LoadU16(HH_SCR + 0x604u) == 0x7Fu,
           "C7F60: matrix words copied to +0x10, +4 = 0x7F");

    /* src/func_800C7BA0.c: parameter blocks. */
    PE_StoreU32(0x800E2248u, 0u);
    ASSERT(func_800C7BA0(HH_SCR + 0x1000u) == 0, "C7BA0: returns 0");
    /* src/func_800C22F8.c returns D_800E2248 + 0x6C = slot + 0xC + 0x6C. */
    ASSERT(PE_LoadU32(HH_SCR + 0x1000u + 0x78u) == 0x800E0928u,
           "C7BA0: script stored at func_800C22F8's returned word");
    ASSERT(PE_LoadU8(0x800F3498u + 4u) == 0xAEu && (int16_t)PE_LoadU16(0x800F3498u + 8u) == -0x32 &&
           PE_LoadU8(0x800F3498u + 1u) == 0x50u, "C7BA0: second block");
    ASSERT(PE_LoadU8(0x800F34D8u + 5u) == 0x20u && PE_LoadU16(0x800F34D8u + 8u) == 0x32u,
           "C7BA0: fourth block");
    PASS();
}

static void test_HANDHI_batch11(void)
{
    TEST("HANDHI_batch11");
    ResetTestState();

    /* src/func_800C8F94.c: base position from D_800E2350/52/54, +2 = 0x14
     * frames, +1 = 0; the offset goes through the D_800E27A0 matrix. */
    const pe_addr_t owner = HH_SCR, mat = HH_SCR + 0x100u;
    PE_StoreU32(0x800E27A0u, owner);
    PE_StoreU32(owner + 0x238u, mat);
    for (unsigned i = 0; i < 9u; i++) PE_StoreU16(mat + i * 2u, (i % 4u == 0u) ? 0x1000u : 0u);
    PE_StoreU16(0x800E2350u, 1u); PE_StoreU16(0x800E2352u, 2u); PE_StoreU16(0x800E2354u, 3u);
    PE_StoreU8(HH_SCR + 0x201u, 9u);
    func_800C8F94(0, 0, HH_SCR + 0x200u);
    ASSERT(PE_LoadU16(HH_SCR + 0x208u) == 1u && PE_LoadU16(HH_SCR + 0x20Au) == 2u &&
           PE_LoadU16(HH_SCR + 0x20Cu) == 3u, "C8F94: base position");
    ASSERT(PE_LoadU8(HH_SCR + 0x202u) == 0x14u && PE_LoadU8(HH_SCR + 0x201u) == 0u,
           "C8F94: frame count 0x14, +1 cleared");
    /* vy = -(r % 3 + 9) lies in [-11, -9] through an identity matrix. */
    int16_t vy = (int16_t)PE_LoadU16(HH_SCR + 0x212u);
    ASSERT(vy >= -11 && vy <= -9, "C8F94: vy = -(r % 3 + 9)");
    PASS();
}

static void test_HANDHI_batch12(void)
{
    TEST("HANDHI_batch12");
    ResetTestState();
    const pe_addr_t s = HH_SCR + 0x100u, cmd = HH_SCR;

    /* src/func_8008B410.c: bank 0 ramp 0x10 -> 0x30 over 4 steps. */
    PE_StoreU32(0x8009D2C8u, s);
    PE_StoreU32(0x800BCD50u, 0u);
    PE_StoreU32(cmd + 4u, 4u); PE_StoreU32(cmd + 8u, 0x10u);
    PE_StoreU32(cmd + 0xCu, 0x30u); PE_StoreU32(cmd + 0x10u, 0u);
    func_8008B410(cmd);
    ASSERT(PE_LoadU32(s + 0x48u) == 0x100000u && PE_LoadU32(s + 0x4Cu) == 0x80000u &&
           PE_LoadU16(s + 0x50u) == 4u, "8B410: base, step, count");

    /* src/func_8008B2CC.c: steps 0 -> 1; fade from +0x48 to 0x20 << 16. */
    PE_StoreU32(cmd + 4u, 0u); PE_StoreU32(cmd + 8u, 0x20u);
    func_8008B2CC(cmd);
    ASSERT(PE_LoadU32(s + 0x4Cu) == 0x100000u && PE_LoadU16(s + 0x50u) == 1u,
           "8B2CC: n = 1, step = target - current");

    /* src/func_80088E64.c: pan split. */
    PE_StoreU16(HH_SCR + 0x400u + 0xD8u, 0x4000u);         /* pan byte 0x40 */
    PE_StoreU16(HH_SCR + 0x400u + 0x118u, 0x100u);
    PE_StoreU16(HH_SCR + 0x400u + 0x11Au, 0x200u);
    PE_StoreU32(HH_SCR + 0x400u + 0xF4u, 0x3u);
    func_80088E64(HH_SCR + 0x400u, 1);
    ASSERT(PE_LoadU16(HH_SCR + 0x518u) == (uint16_t)((0x100u * 0x3Fu) >> 8) &&
           PE_LoadU16(0x800B8AC0u + 0x11Cu + 0x118u) == (uint16_t)((0x100 * 0x4000) >> 16),
           "88E64: L split by pan");
    /* +0xF4 (flags 3) is ORed into the pair, then the real func_800878F0
     * (src/func_800878F0.c) programs voice 1's volume pair from the record
     * at +0xF0 (+0x28/+0x2A = +0x118/+0x11A) and clears the flags. */
    ASSERT(PE_SpuRegister_LoadU16(0x10u) == 0x40u && PE_SpuRegister_LoadU16(0x12u) == 0x80u,
           "88E64: pair voice 1 volumes programmed");
    ASSERT(PE_LoadU32(0x800B8AC0u + 0x11Cu + 0xF4u) == 0u, "88E64: pair flags consumed");

    /* src/func_800CD0BC.c: seed and random velocities in [-25, 24]. */
    PE_StoreU16(0x800E27F8u, 7u); PE_StoreU16(0x800E27FAu, 8u); PE_StoreU16(0x800E27FCu, 9u);
    func_800CD0BC(0, 0, HH_SCR + 0x600u);
    ASSERT(PE_LoadU16(HH_SCR + 0x606u) == 0x3B4u && PE_LoadU16(HH_SCR + 0x61Cu) == 9u,
           "CD0BC: header and satellite 1 z");
    int16_t vx = (int16_t)PE_LoadU16(HH_SCR + 0x628u);
    ASSERT(vx >= -25 && vx <= 24 && PE_LoadU16(HH_SCR + 0x62Cu) == 0u, "CD0BC: r % 50 - 25");
    PASS();
}

static void test_HANDHI_batch13(void)
{
    TEST("HANDHI_batch13");
    ResetTestState();

    /* src/func_800CC0E0.c: stage 3 -> 8 points at angle i << 9. */
    PE_StoreU32(0x800E2248u, HH_SCR);                 /* func_800C2B10 base */
    PE_StoreU32(HH_SCR + 8u + 12u, 0x11u);            /* slot 3 */
    PE_StoreU32(HH_SCR + 8u + 16u, 0x22u);
    PE_StoreU32(HH_SCR + 8u + 20u, 0x33u);
    PE_StoreU32(0x8009D254u, HH_SCR + 0x100u);
    PE_StoreU32(HH_SCR + 0x100u, HH_SCR + 0x110u);
    PE_StoreU32(HH_SCR + 0x110u + 0x68u, HH_SCR + 0x200u);
    PE_StoreU16(HH_SCR + 0x206u, 3u);
    PE_StoreU16(0x800E2290u, 5u); PE_StoreU16(0x800E2292u, 6u); PE_StoreU16(0x800E2294u, 7u);
    func_800CC0E0(0, 0, HH_SCR + 0x400u);
    ASSERT(PE_LoadU8(0x800E2280u) == 0x11u && PE_LoadU8(0x800E2282u) == 0x33u, "CC0E0: colour slots");
    ASSERT(PE_LoadU8(HH_SCR + 0x402u) == 8u && PE_LoadU8(HH_SCR + 0x401u) == 0x7Fu, "CC0E0: 8 points");
    ASSERT(PE_LoadU16(HH_SCR + 0x400u + 7u * 8u + 0x28u) == 6u &&
           (int16_t)PE_LoadU16(HH_SCR + 0x400u + 7u * 8u + 0xA8u) == -0x1400 &&
           (int16_t)PE_LoadU16(HH_SCR + 0x400u + 7u * 8u + 0xA6u) == (int16_t)func_80077CF4(7 << 9) &&
           PE_LoadU16(HH_SCR + 0x400u + 7u * 2u + 6u) == 0x258u, "CC0E0: point 7");

    /* src/func_800CAA38.c: identity matrix -> base + template. */
    const pe_addr_t inner = HH_SCR + 0x800u, mat = inner + 0x260u;
    PE_StoreU32(0x800E27A8u, HH_SCR + 0x700u);
    PE_StoreU32(HH_SCR + 0x700u + 0x238u, inner);
    for (unsigned i = 0; i < 9u; i++) PE_StoreU16(mat + i * 2u, (i % 4u == 0u) ? 0x1000u : 0u);
    for (unsigned i = 0; i < 3u; i++) PE_StoreU32(mat + 0x14u + i * 4u, 0u);
    PE_StoreU16(0x800C21C4u, 1u); PE_StoreU16(0x800C21CCu + 4u, 9u);
    PE_StoreU16(0x800E2360u, 100u); PE_StoreU16(0x800E2364u, 300u);
    func_800CAA38(0, 0, HH_SCR + 0xA00u);
    ASSERT(PE_LoadU16(HH_SCR + 0xA08u) == 101u && PE_LoadU16(HH_SCR + 0xA14u) == 309u &&
           PE_LoadU16(HH_SCR + 0xA04u) == 0x7Fu, "CAA38: both endpoints");
    PASS();
}

static void test_HANDHI_batch14(void)
{
    TEST("HANDHI_batch14");
    ResetTestState();
    const pe_addr_t v = HH_SCR, z = HH_SCR + 0x200u;

    /* Three zones: programs 1, 2, 0x25 with key tops 10, 20, 30 (p[2])
     * and velocity floors 0, 40, 80 (p[1]); terminator 0x80. */
    PE_StoreU32(0x8009D2C8u, HH_SCR + 0x300u);
    PE_StoreU32(HH_SCR + 0x300u, 0x100u);                   /* adj = 0x30 */
    const unsigned char prog[3] = {1u, 2u, 0x25u}, top[3] = {10u, 20u, 30u}, lo[3] = {0u, 40u, 80u};
    for (unsigned i = 0; i < 3u; i++) {
        PE_StoreU8(z + i * 8u + 0u, prog[i]);
        PE_StoreU8(z + i * 8u + 1u, lo[i]);
        PE_StoreU8(z + i * 8u + 2u, top[i]);
        PE_StoreU8(z + i * 8u + 3u, (uint8_t)(0x10u + i));
        PE_StoreU8(z + i * 8u + 6u, (uint8_t)(0x20u + i));
    }
    PE_StoreU8(z + 24u, 0x80u);
    /* func_8008E664 counts the terminator entry itself (it tests each
     * entry's own first byte), so its floor must exceed the probe. */
    PE_StoreU8(z + 25u, 0xFFu);
    PE_StoreU32(v + 0x18u, z);

    /* src/func_8008E4E8.c: key 15 -> zone 1 (program 2). */
    PE_StoreU16(v + 0x5Au, 0x7777u);
    func_8008E4E8(v, 15u);
    ASSERT(PE_LoadU16(v + 0x5Au) == 2u && PE_LoadU16(v + 0x10Eu) == 0x11u &&
           PE_LoadU16(v + 0x116u) == 0x21u && (PE_LoadU32(v + 0xF4u) & 0x4000u),
           "8E4E8: first zone with top >= key selected");
    /* key 25 -> zone 2, program 0x25 + adj. */
    func_8008E4E8(v, 25u);
    ASSERT(PE_LoadU16(v + 0x5Au) == 0x55u, "8E4E8: programs >= 0x20 get +0x30");

    /* src/func_8008E664.c: velocity 50 -> backward scan stops at zone 1. */
    PE_StoreU16(v + 0x5Au, 0x7777u);
    func_8008E664(v, 50u);
    ASSERT(PE_LoadU16(v + 0x5Au) == 2u, "8E664: last zone with floor <= velocity");
    /* Already on program 1's neighbour (predecessor current) -> no change. */
    PE_StoreU16(v + 0x5Au, 1u);
    func_8008E664(v, 50u);
    ASSERT(PE_LoadU16(v + 0x5Au) == 1u, "8E664: predecessor current -> no reselect");
    PASS();
}

static void test_HANDHI_batch15(void)
{
    TEST("HANDHI_batch15");
    ResetTestState();
    const pe_addr_t ot = HH_SCR, prim = HH_SCR + 0x100u, arena = HH_SCR + 0x200u;

    /* src/func_800CF6F8.c, a2 == 0xFF: link a1 only. */
    PE_StoreU32(ot, 0xAA123456u);
    PE_StoreU32(prim, 0x05000000u);
    func_800CF6F8(ot, prim, 0xFF);
    ASSERT(PE_LoadU32(prim) == 0x05123456u, "CF6F8: prim takes the old OT link");
    ASSERT(PE_LoadU32(ot) == (0xAA000000u | (prim & 0xFFFFFFu)), "CF6F8: OT points at prim");

    /* a2 != 0xFF: packet from the arena linked in front of prim. */
    PE_StoreU32(0x8009CDDCu, 0u);
    PE_StoreU32(0x800B0E58u, arena);
    PE_StoreU32(0x8009CDD8u, 0x10u);
    PE_StoreU32(ot, 0xAA123456u);
    PE_StoreU32(prim, 0x05000000u);
    PE_StoreU8(prim + 7u, 0u);
    func_800CF6F8(ot, prim, 1);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 0x18u, "CF6F8: arena cursor += 8");
    ASSERT((PE_LoadU8(prim + 7u) & 2u) && (PE_LoadU32(prim) & 0xFFFFFFu) == 0x123456u,
           "CF6F8: prim flagged and linked to the old head");
    ASSERT((PE_LoadU32(arena + 0x10u) & 0xFFFFFFu) == (prim & 0xFFFFFFu) &&
           (PE_LoadU32(ot) & 0xFFFFFFu) == ((arena + 0x10u) & 0xFFFFFFu),
           "CF6F8: packet between OT and prim");
    PASS();
}

static void test_HANDHI_batch16(void)
{
    TEST("HANDHI_batch16");
    ResetTestState();
    const pe_addr_t a1 = HH_SCR;

    /* src/func_800D6C58.c mode 1: integrate, damp 31/32, bounce, gravity. */
    PE_StoreU32(0x800E27ECu, 3u);
    PE_StoreU16(a1 + 0u, 10u); PE_StoreU16(a1 + 2u, (uint16_t)-1); PE_StoreU16(a1 + 4u, 0u);
    PE_StoreU16(a1 + 6u, 64u); PE_StoreU16(a1 + 8u, 5u); PE_StoreU16(a1 + 0xAu, (uint16_t)-64);
    PE_StoreU16(a1 + 0xCu, 10u);
    ASSERT(func_800D6C58(1, a1) == 0, "D6C58: alive while frame < life");
    ASSERT(PE_LoadU16(a1 + 0u) == 74u && PE_LoadU16(a1 + 2u) == 4u, "D6C58: position += velocity");
    ASSERT(PE_LoadU16(a1 + 6u) == 62u && (int16_t)PE_LoadU16(a1 + 0xAu) == -62,
           "D6C58: vx/vz * 31 / 32 (signed)");
    ASSERT((int16_t)PE_LoadU16(a1 + 8u) == -2, "D6C58: y > 0 bounces vy, then + 3");
    PE_StoreU32(0x800E27ECu, 10u);
    ASSERT(func_800D6C58(1, a1) == 1, "D6C58: finished at frame >= life");

    /* src/func_800D9E5C.c mode 1: vy decays by 2 below frame 0x13. */
    PE_StoreU32(0x800E27ECu, 0x12u);
    PE_StoreU16(a1 + 2u, 100u); PE_StoreU16(a1 + 6u, 10u);
    ASSERT(func_800D9E5C(1, a1) == 0 && PE_LoadU16(a1 + 2u) == 110u && PE_LoadU16(a1 + 6u) == 8u,
           "D9E5C: y += v, v -= 2");
    PE_StoreU32(0x800E27ECu, 0x18u);
    ASSERT(func_800D9E5C(1, a1) == 1 && PE_LoadU16(a1 + 6u) == 8u, "D9E5C: no decay late, done at 0x18");

    /* src/func_800D7A1C.c mode 1: y -= v (old), then v -= 1. */
    PE_StoreU32(0x800E27ECu, 1u);
    PE_StoreU16(a1 + 2u, 50u); PE_StoreU16(a1 + 6u, 7u);
    ASSERT(func_800D7A1C(1, a1) == 0 && PE_LoadU16(a1 + 2u) == 43u && PE_LoadU16(a1 + 6u) == 6u,
           "D7A1C: y -= v then v--");

    /* src/func_800D7E78.c / 800D8D14.c mode 1 lifetimes. */
    PE_StoreU32(0x800E27ECu, 5u);
    ASSERT(func_800D7E78(1, a1) == 0 && func_800D8D14(1, a1) == 0, "D7E78/D8D14: alive");
    ASSERT(PE_LoadU16(a1 + 2u) == 43u - 1u - 3u, "D7E78 -1 / D8D14 -3 on y");
    PE_StoreU32(0x800E27ECu, 6u);
    ASSERT(func_800D7E78(1, a1) == 1 && func_800D8D14(1, a1) == 0, "D7E78 ends at 6, D8D14 at 12");
    PASS();
}

static void test_HANDHI_batch17(void)
{
    TEST("HANDHI_batch17");
    ResetTestState();

    /* src/func_800D1AE0.c, a2 == 0xFF: tint TILE linked into OT slot 2. */
    const pe_addr_t arena = HH_SCR, otbase = HH_SCR + 0x400u, rgb = HH_SCR + 0x500u;
    PE_StoreU32(0x8009CDDCu, 9u);
    PE_StoreU32(0x800B0E58u + 9u * 4u, arena);
    PE_StoreU32(0x800B0E58u + 1u * 4u, otbase);      /* index 9 - 8 */
    PE_StoreU32(0x8009CDD8u, 0x20u);
    PE_StoreU8(rgb + 0u, 0x80u); PE_StoreU8(rgb + 1u, 0x40u); PE_StoreU8(rgb + 2u, 0x20u);
    PE_StoreU32(otbase + 8u, 0x00ABCDEFu);
    func_800D1AE0(rgb, 64, 0xFF, 2);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 0x30u, "D1AE0: arena += 0x10");
    ASSERT(PE_LoadU8(arena + 0x24u) == 0x40u && PE_LoadU8(arena + 0x25u) == 0x20u &&
           PE_LoadU8(arena + 0x26u) == 0x10u, "D1AE0: colour * a1 / 128");
    ASSERT(PE_LoadU16(arena + 0x2Cu) == 0x140u && PE_LoadU16(arena + 0x2Eu) == 0xF0u,
           "D1AE0: 320x240");
    ASSERT((PE_LoadU32(arena + 0x20u) & 0xFFFFFFu) == 0xABCDEFu &&
           (PE_LoadU32(otbase + 8u) & 0xFFFFFFu) == ((arena + 0x20u) & 0xFFFFFFu),
           "D1AE0: linked into OT slot a3");
    /* a3 >= 0x1000: allocated and coloured, never linked or sized. */
    func_800D1AE0(rgb, 128, 0xFF, 0x1000);
    ASSERT(PE_LoadU32(0x8009CDD8u) == 0x40u && PE_LoadU16(arena + 0x3Cu) != 0x140u,
           "D1AE0: out-of-range slot not linked");

    /* src/func_800D629C.c mode 1: circle step. */
    PE_StoreU32(0x800E27ECu, 1u);
    PE_StoreU16(0x800E21D4u, 0u);
    PE_StoreU16(0x800E21D8u, 50u); PE_StoreU16(0x800E21DCu, 60u);
    PE_StoreU16(HH_SCR + 0x608u, 10u); PE_StoreU16(HH_SCR + 0x60Au, 100u);
    ASSERT(func_800D629C(1, HH_SCR + 0x600u) == 0, "D629C: alive");
    ASSERT(PE_LoadU16(HH_SCR + 0x602u) == 100u && PE_LoadU16(HH_SCR + 0x60Au) == 93u &&
           PE_LoadU16(HH_SCR + 0x608u) == 12u && PE_LoadU16(HH_SCR + 0x600u) == 50u &&
           PE_LoadU16(HH_SCR + 0x604u) == 60u, "D629C: y = old a1[5]; a1[5] -= 7; angle += 2");

    /* src/func_800D5898.c mode 1: a1[0] = a1[1] * cos(...) / 0x1000, life a1[3]. */
    PE_StoreU32(0x800E27ECu, 0u);
    PE_StoreU16(HH_SCR + 0x702u, 0x100u); PE_StoreU16(HH_SCR + 0x706u, 4u);
    ASSERT(func_800D5898(1, HH_SCR + 0x700u) == 0 &&
           (int16_t)PE_LoadU16(HH_SCR + 0x700u) == (int16_t)(0x100 * func_80077DC4(0) / 0x1000),
           "D5898: amplitude * cos(0)");
    PE_StoreU32(0x800E27ECu, 4u);
    ASSERT(func_800D5898(1, HH_SCR + 0x700u) == 1, "D5898: finished at frame >= a1[3]");
    PASS();
}

static void test_HANDHI_batch18(void)
{
    TEST("HANDHI_batch18");
    ResetTestState();

    /* src/func_800DF6AC.c mode 1 without a spawn (frame not a multiple of 4). */
    PE_StoreU32(0x800E27ECu, 0x21u);
    PE_StoreU16(HH_SCR + 0u, 8u); PE_StoreU16(HH_SCR + 2u, 3u);
    ASSERT(func_800DF6AC(1, HH_SCR) == 0 && PE_LoadU16(HH_SCR + 2u) == 1u, "DF6AC: fade by 2");
    ASSERT(func_800DF6AC(1, HH_SCR) == 1 && PE_LoadU16(HH_SCR + 2u) == 0u, "DF6AC: done, clamped to 0");

    /* src/func_800DB0D0.c mode 2 publishes the position. */
    PE_StoreU16(HH_SCR + 0x100u, 1u); PE_StoreU16(HH_SCR + 0x102u, 2u); PE_StoreU16(HH_SCR + 0x104u, 3u);
    ASSERT(func_800DB0D0(2, HH_SCR + 0x100u) == 0, "DB0D0: mode 2 returns 0");
    ASSERT(PE_LoadU16(0x800E221Cu) == 1u && PE_LoadU16(0x800E2220u) == 3u &&
           PE_LoadU16(0x800F3374u) == 8u, "DB0D0: position published, D_800F3374 = 8");

    /* src/func_800C5EB0.c with count 1: only the activation and the tick. */
    PE_StoreU32(HH_SCR + 0x200u, HH_SCR + 0x300u);
    PE_StoreU16(HH_SCR + 0x204u, 1u);
    PE_StoreU16(HH_SCR + 0x20Eu, 0x64u);
    PE_StoreU32(HH_SCR + 0x280u, 7u);
    ASSERT(func_800C5EB0(HH_SCR + 0x200u, 0, HH_SCR + 0x280u) == 1, "C5EB0: tick 0x64 -> 1");
    ASSERT(PE_LoadU16(HH_SCR + 0x20Eu) == 0x65u && PE_LoadU32(HH_SCR + 0x280u) == 0u,
           "C5EB0: tick incremented, *a2 cleared");
    PE_StoreU16(HH_SCR + 0x20Eu, 0u);
    ASSERT(func_800C5EB0(HH_SCR + 0x200u, 0, HH_SCR + 0x280u) == 0 &&
           PE_LoadU8(HH_SCR + 0x300u) == 2u, "C5EB0: segment 0 activated");
    PASS();
}

static void test_HANDHI_batch19(void)
{
    TEST("HANDHI_batch19");
    ResetTestState();

    /* src/func_80065C38.c: clamp into room 1's rect [-10, 10] x [0, 50]. */
    const pe_addr_t base = HH_SCR, h = HH_SCR + 0x40u + 52u;
    PE_StoreU32(0x800B1624u, base);
    PE_StoreU32(base + 0x1Cu, 0x40u);
    PE_StoreU8(0x800BCFFDu, 1u);
    PE_StoreU16(h + 0x2Cu, (uint16_t)-10); PE_StoreU16(h + 0x2Eu, 10u);
    PE_StoreU16(h + 0x30u, 0u); PE_StoreU16(h + 0x32u, 50u);
    PE_StoreU32(0x800BCF88u, 0x40u);
    func_80065C38(-20, 60);
    ASSERT((int16_t)PE_LoadU16(0x800BCF8Cu) == -10 && PE_LoadU16(0x800BCF8Eu) == 50u,
           "65C38: clamped to min x / max y");
    func_80065C38(5, 7);
    ASSERT(PE_LoadU16(0x800BCF8Cu) == 5u && PE_LoadU16(0x800BCF8Eu) == 7u, "65C38: inside passes through");
    PE_StoreU32(0x800BCF88u, 0u);
    func_80065C38(99, 99);
    ASSERT(PE_LoadU16(0x800BCF8Cu) == 5u, "65C38: gate bit 6 clear -> untouched");

    /* src/func_800CBCA4.c: a0[2] position + records. */
    PE_StoreU32(0x800E2248u, 0u);
    /* func_800C22F8 clears slot + 0xC .. + 0xA0C, so the owner record
     * lives outside the slot. */
    PE_StoreU32(HH_SCR + 0x208u, HH_SCR + 0x1000u);
    PE_StoreU16(HH_SCR + 0x102Au, 11u); PE_StoreU16(HH_SCR + 0x102Eu, 22u); PE_StoreU16(HH_SCR + 0x1032u, 33u);
    ASSERT(func_800CBCA4(HH_SCR + 0x200u) == 0, "CBCA4: returns 0");
    ASSERT(PE_LoadU32(0x800F3470u) == HH_SCR + 0x1000u && PE_LoadU16(0x800E2268u + 0x2Cu) == 33u,
           "CBCA4: owner and position published");
    ASSERT(PE_LoadU32(0x800E2820u + 0x10u) == 0x1A8u && PE_LoadU8(0x800F3388u + 0x1Du) == 0x30u &&
           (int16_t)PE_LoadU16(0x800F3388u + 0x1Eu) == -0x50, "CBCA4: record fields");
    PASS();
}

static void test_HANDHI_batch20(void)
{
    TEST("HANDHI_batch20");
    ResetTestState();

    /* src/func_80085A64.c through a guest-RAM register mirror. */
    PE_StoreU32(0x8009B3FCu, HH_SCR);
    PE_StoreU16(HH_SCR + 0x1AAu, 0x0001u);
    PE_StoreU32(0x8009B394u, 1u);                 /* configured -> enable */
    ASSERT(func_80085A64(1) == 1 && PE_LoadU16(HH_SCR + 0x1AAu) == 0x0081u,
           "85A64: enable sets SPUCNT bit 7");
    ASSERT(func_80085A64(0) == 0 && PE_LoadU16(HH_SCR + 0x1AAu) == 0x0001u,
           "85A64: disable clears bit 7");
    ASSERT(func_80085A64(5) == 0, "85A64: other modes only report the state");

    /* src/func_8007BBB0.c: the four callback words are cleared. */
    PE_StoreU32(0x8009AFB4u, 1u); PE_StoreU32(0x8009AFB8u, 1u);
    PE_StoreU32(0x8009AFC4u, 1u); PE_StoreU32(0x8009AFC8u, 1u);
    func_8007BBB0();
    ASSERT(PE_LoadU32(0x8009AFB4u) == 0u && PE_LoadU32(0x8009AFB8u) == 0u &&
           PE_LoadU32(0x8009AFC4u) == 0u && PE_LoadU32(0x8009AFC8u) == 0u,
           "7BBB0: CD callback words cleared");

    /* src/func_800D6E3C.c mode 2: draw state for the fountain. */
    PE_StoreU16(0x800E11E4u, 1u);
    PE_StoreU16(0x800E2850u + 2u, 0x1234u);
    ASSERT(func_800D6E3C(2, HH_SCR + 0x400u) == 0, "D6E3C: mode 2 returns 0");
    ASSERT(PE_LoadU16(0x800F3370u) == 0x1234u && PE_LoadU16(0x800F3368u) == 0x10u &&
           PE_LoadU16(0x800F336Au) == 1u, "D6E3C: texpage for stage D_800E11E4");
    PE_StoreU32(0x800E27ECu, 0x28u);
    ASSERT(func_800D6E3C(1, HH_SCR + 0x400u) == 1, "D6E3C: ends at 0x28");
    PASS();
}

static void test_HANDHI_batch21(void)
{
    TEST("HANDHI_batch21");
    ResetTestState();
    const pe_addr_t g = HH_SCR;

    /* src/func_800679C4.c: clamp x to [-100, 100], y to [0, 200]. */
    PE_StoreU32(0x800B1624u, g);
    PE_StoreU16(g + 0x28u, 90u); PE_StoreU16(g + 0x2Au, 10u); PE_StoreU16(g + 0x24u, 5u);
    PE_StoreU16(g + 0x30u, (uint16_t)-100); PE_StoreU16(g + 0x32u, 100u);
    PE_StoreU16(g + 0x34u, 0u); PE_StoreU16(g + 0x36u, 200u);
    ASSERT(func_800679C4(20, -30, 3) == 0, "679C4: returns 0");
    ASSERT(PE_LoadU16(g + 0x2Cu) == 100u && PE_LoadU16(g + 0x2Eu) == 0u && PE_LoadU16(g + 0x26u) == 8u,
           "679C4: x clamped to max, y to min, angle summed");
    func_800679C4(-5, 5, 0);
    ASSERT(PE_LoadU16(g + 0x2Cu) == 85u && PE_LoadU16(g + 0x2Eu) == 15u, "679C4: in range passes");

    /* src/func_800DAF8C.c mode 1 at frame 32. */
    PE_StoreU32(0x800E27ECu, 32u);
    PE_StoreU16(HH_SCR + 0x10Cu, 64u);   /* A */
    PE_StoreU16(HH_SCR + 0x10Eu, 3u);    /* vy */
    PE_StoreU16(HH_SCR + 0x102u, 10u);
    ASSERT(func_800DAF8C(1, HH_SCR + 0x100u) == 0, "DAF8C: alive below 0x40");
    ASSERT(PE_LoadU16(HH_SCR + 0x100u) == 32u && PE_LoadU16(HH_SCR + 0x102u) == 13u &&
           PE_LoadU16(HH_SCR + 0x10Au) == (uint16_t)(0x640 - 32 * 1000 / 64),
           "DAF8C: A - A*f/64, y += vy, a1[5]");

    /* src/func_800DAB98.c mode 1 life check. */
    PE_StoreU16(HH_SCR + 0x206u, 32u);
    ASSERT(func_800DAB98(1, HH_SCR + 0x200u) == 1 && PE_LoadU16(HH_SCR + 0x204u) == 0x10u,
           "DAB98: z += 0x10, done at f >= a1[3]");
    PASS();
}

static void test_HANDHI_all(void)
{
    test_HANDHI_seq_ops();
    test_HANDHI_batch21();
    test_HANDHI_batch20();
    test_HANDHI_batch19();
    test_HANDHI_batch18();
    test_HANDHI_batch17();
    test_HANDHI_batch16();
    test_HANDHI_batch15();
    test_HANDHI_batch14();
    test_HANDHI_batch13();
    test_HANDHI_batch12();
    test_HANDHI_batch11();
    test_HANDHI_batch10();
    test_HANDHI_batch9();
    test_HANDHI_batch8();
    test_HANDHI_batch7();
    test_HANDHI_batch6();
    test_HANDHI_batch5();
    test_HANDHI_batch4();
    test_HANDHI_batch3();
    test_HANDHI_small();
    test_HANDHI_spu_voice();
    test_HANDHI_stream_cursor();
    test_HANDHI_misc();
}
