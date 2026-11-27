/* Draw environment, draw mode and draw offset words (guest 0x800749D8,
 * 0x80076150, 0x800762A0, 0x80075C04), written clean-room from the black-box
 * vector sets state60d (docs/evidence/native-state60-20261126) and psx-spx;
 * no SDK source, decompiled source, tables or disassembly were read. */
#include "pe_plat/gpuenv.h"
#include "pe_guest_ram.h"

/* Display-environment record (guest layout): four halfwords for the display
 * rectangle, then a zero halfword each for the screen rectangle (x,y,w,h)
 * and four zero bytes of flags (interlace, 24-bit, pads). */
#define DENV_DISP_X        0x00u
#define DENV_DISP_Y        0x02u
#define DENV_DISP_W        0x04u
#define DENV_DISP_H        0x06u
#define DENV_SCREEN_FIRST  0x08u
#define DENV_SCREEN_LAST   0x0Eu
#define DENV_FLAGS_FIRST   0x10u
#define DENV_FLAGS_LAST    0x13u

/* GP0 command words (psx-spx "GP0 Rendering Attributes"). */
#define GP0_DRAW_MODE      0xE1000000u
#define GP0_DRAW_OFFSET    0xE5000000u
/* E1h: bits 0-8 texpage x/y/semi-transparency/colour depth, bit 9 dither,
 * bit 10 drawing to the display area, bit 11 texture disable.  The vectors
 * show the page argument contributes bits 0-8 and 11; 9 and 10 come only
 * from the two flag arguments (page bits 9/10 never reach the word). */
#define DRAW_MODE_PAGE_MASK 0x09FFu
#define DRAW_MODE_DITHER    0x0200u
#define DRAW_MODE_DISPLAY   0x0400u
/* E5h: 11-bit signed X in bits 0-10, 11-bit signed Y in bits 11-21. */
#define OFFSET_AXIS_MASK    0x7FFu
#define OFFSET_Y_SHIFT      11u

uint32_t pe_plat_gpuenv_display_env(uint32_t env, int x, int y, int width, int height)
{
    uint32_t off;

    PE_StoreU16(env + DENV_DISP_X, (uint16_t)x);
    PE_StoreU16(env + DENV_DISP_Y, (uint16_t)y);
    PE_StoreU16(env + DENV_DISP_W, (uint16_t)width);
    PE_StoreU16(env + DENV_DISP_H, (uint16_t)height);
    for (off = DENV_SCREEN_FIRST; off <= DENV_SCREEN_LAST; off += 2u)
        PE_StoreU16(env + off, 0u);
    for (off = DENV_FLAGS_FIRST; off <= DENV_FLAGS_LAST; off++)
        PE_StoreU8(env + off, 0u);
    return env;
}

uint32_t pe_plat_gpuenv_draw_mode(int draw_to_display, int dither, uint32_t page)
{
    uint32_t word = GP0_DRAW_MODE | (page & DRAW_MODE_PAGE_MASK);

    if (dither != 0)
        word |= DRAW_MODE_DITHER;
    if (draw_to_display != 0)
        word |= DRAW_MODE_DISPLAY;
    return word;
}

int pe_plat_gpuenv_draw_offset(int x, int y)
{
    uint32_t word = GP0_DRAW_OFFSET | ((uint32_t)x & OFFSET_AXIS_MASK) |
                    (((uint32_t)y & OFFSET_AXIS_MASK) << OFFSET_Y_SHIFT);

    return (int)word;
}

/* The packet form takes its x/y from two sign-extended guest halfwords.
 * The vectors show the packet's size byte (packet+3) set to 2, the command
 * word at packet+4 and a zero word at packet+8. */
#define PKT_SIZE_BYTE   3u
#define PKT_WORD        4u
#define PKT_TAIL        8u
#define PKT_SIZE_WORDS  2u

void pe_plat_gpuenv_draw_offset_packet(uint32_t packet, uint32_t coords)
{
    int x = (int16_t)PE_LoadU16(coords);
    int y = (int16_t)PE_LoadU16(coords + 2u);

    PE_StoreU8(packet + PKT_SIZE_BYTE, PKT_SIZE_WORDS);
    PE_StoreU32(packet + PKT_WORD, (uint32_t)pe_plat_gpuenv_draw_offset(x, y));
    PE_StoreU32(packet + PKT_TAIL, 0u);
}
