/* GPU packet setters (guest 0x80077B04..0x80077CB4), written clean-room from the
 * black-box vector sets state60c (docs/evidence/native-state60-20261126) and
 * psx-spx; no SDK source, decompiled source, tables or disassembly were read.
 *
 * Packet layout (psx-spx, GPU linked-list DMA): word 0 is the node header,
 * bits 0..23 next address and bits 24..31 the number of command words that
 * follow, so the length lives in the byte at packet+3.  Word 1 is the first
 * GP0 word: colour in bits 0..23 and the command byte in bits 24..31, i.e. the
 * byte at packet+7. */
#include "pe_plat/gpupkt.h"
#include "pe_guest_ram.h"

#define PKT_LEN_BYTE     3u   /* node header: word count */
#define PKT_CMD_BYTE     7u   /* first GP0 word: command */
#define PKT_WORD1        4u

/* GP0 commands / word counts (psx-spx "GPU Render Polygon/Line/Rectangle Commands"):
 * the count is the command word plus vertices (and UV/clut words, colours). */
#define GP0_POLY_F3      0x20u  /* mono triangle: cmd + 3 vertices            = 4  */
#define GP0_POLY_G3      0x30u  /* shaded triangle: 3 * (colour + vertex)      = 6  */
#define GP0_POLY_FT4     0x2Cu  /* mono textured quad: cmd + 4 * (vertex + uv) = 9  */
#define GP0_POLY_G4      0x38u  /* shaded quad: 4 * (colour + vertex)          = 8  */
#define GP0_POLY_GT4     0x3Cu  /* shaded textured quad: 4 * 3                 = 12 */
#define GP0_SPRITE       0x64u  /* textured variable rectangle: cmd, vertex, uv, size = 4 */
#define GP0_TILE1        0x68u  /* 1x1 rectangle: cmd + vertex                 = 2  */
#define GP0_TILE         0x60u  /* variable rectangle: cmd, vertex, size       = 3  */
#define GP0_LINE_F2      0x40u  /* mono line: cmd + 2 vertices                 = 3  */

#define GP0_SEMITRANS_BIT 0x02u /* command bit 1: semi-transparent */
#define GP0_RAW_TEX_BIT   0x01u /* command bit 0: raw texture (no shading/modulation) */

#define GP0_DRAW_MODE     0xE1000000u /* "Draw Mode setting" */
#define DM_TPAGE_MASK     0x000009FFu /* tpage bits 0..8 and 11 pass through */
#define DM_DITHER         0x00000200u /* bit 9 */
#define DM_DRAW_TO_DISP   0x00000400u /* bit 10 */

#define LINK_MAX_WORDS    15u   /* head words + tail words that may be joined */

/* Length byte first, then the command byte (the vectors record that order). */
static void init_packet(uint32_t packet, uint8_t words, uint8_t cmd)
{
    PE_StoreU8(packet + PKT_LEN_BYTE, words);
    PE_StoreU8(packet + PKT_CMD_BYTE, cmd);
}

void pe_plat_gpupkt_poly_f3(uint32_t packet)  { init_packet(packet, 4,  GP0_POLY_F3); }
void pe_plat_gpupkt_poly_g3(uint32_t packet)  { init_packet(packet, 6,  GP0_POLY_G3); }
void pe_plat_gpupkt_poly_ft4(uint32_t packet) { init_packet(packet, 9,  GP0_POLY_FT4); }
void pe_plat_gpupkt_poly_g4(uint32_t packet)  { init_packet(packet, 8,  GP0_POLY_G4); }
void pe_plat_gpupkt_poly_gt4(uint32_t packet) { init_packet(packet, 12, GP0_POLY_GT4); }
void pe_plat_gpupkt_sprite(uint32_t packet)   { init_packet(packet, 4,  GP0_SPRITE); }
void pe_plat_gpupkt_tile1(uint32_t packet)    { init_packet(packet, 2,  GP0_TILE1); }
void pe_plat_gpupkt_tile(uint32_t packet)     { init_packet(packet, 3,  GP0_TILE); }
void pe_plat_gpupkt_line_f2(uint32_t packet)  { init_packet(packet, 3,  GP0_LINE_F2); }

/* Read-modify-write of the command byte; the byte is stored back even when it
 * does not change (the vectors show the store in every case). */
static void update_cmd_bit(uint32_t packet, uint8_t bit, int on)
{
    uint8_t cmd = PE_LoadU8(packet + PKT_CMD_BYTE);
    cmd = (uint8_t)(on ? (cmd | bit) : (cmd & (uint8_t)~bit));
    PE_StoreU8(packet + PKT_CMD_BYTE, cmd);
}

void pe_plat_gpupkt_set_semitrans(uint32_t packet, int enabled)
{
    update_cmd_bit(packet, GP0_SEMITRANS_BIT, enabled != 0);
}

void pe_plat_gpupkt_set_shade_texture(uint32_t packet, int enabled)
{
    update_cmd_bit(packet, GP0_RAW_TEX_BIT, enabled != 0);
}

/* A one-word GP0(E1h) packet: length 1, then the Draw Mode word.  Both flags
 * are tested as full 32-bit values; the texture-page word contributes bits 0..8
 * and 11 only.  Returns the word. */
uint32_t pe_plat_gpupkt_draw_page(uint32_t packet, uint32_t draw_to_display,
                                   uint32_t dither, uint32_t page)
{
    uint32_t word = GP0_DRAW_MODE | (page & DM_TPAGE_MASK);
    if (draw_to_display)
        word |= DM_DRAW_TO_DISP;
    if (dither)
        word |= DM_DITHER;
    PE_StoreU8(packet + PKT_LEN_BYTE, 1u);
    PE_StoreU32(packet + PKT_WORD1, word);
    return word;
}

/* Join the tail packet onto the head: allowed only when both bodies plus the
 * tail's header word fit in 16 words (the GP0 FIFO depth, psx-spx); the head
 * length becomes head + tail + 1 and the tail header word is cleared.  Returns
 * 0 on success and -1, with no store, when it does not fit. */
int pe_plat_gpupkt_append_packet(uint32_t head, uint32_t tail)
{
    uint32_t hw = PE_LoadU8(head + PKT_LEN_BYTE);
    uint32_t tw = PE_LoadU8(tail + PKT_LEN_BYTE);
    if (hw + tw > LINK_MAX_WORDS)
        return -1;
    PE_StoreU8(head + PKT_LEN_BYTE, (uint8_t)(hw + tw + 1u));
    PE_StoreU32(tail, 0u);
    return 0;
}
