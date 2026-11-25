/* GPU packet-word leaves (guest 0x80076170 / 0x80076208 / 0x800762BC /
 * 0x80077AC4), written clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_gpu_leaf_vectors.py), with no SDK source.
 * Evidence: docs/evidence/native-gpu-words-20261030/. */
#include "pe_plat/gpu_primitive.h"
#include "pe_guest_ram.h"

/* Screen limits: the vectors show a signed halfword per axis. */
#define GPU_LIMIT_X         0x80095750u
#define GPU_LIMIT_Y         0x80095752u

/* GP0 command bytes (already shifted into bits 24..31). */
#define GP0_DRAW_AREA_TL    0xE3000000u
#define GP0_DRAW_AREA_BR    0xE4000000u
#define GP0_TEX_WINDOW      0xE2000000u

#define GPU_COORD_MASK      0x3FFu /* 10 bits per draw-area coordinate */
#define GPU_Y_SHIFT         10

/* Texture window fields: five bits each, in units of 8 texels. */
#define GPU_TW_MASK         0x1Fu
#define GPU_TW_UNIT_SHIFT   3
#define GPU_TW_OFF_X_SHIFT  10
#define GPU_TW_OFF_Y_SHIFT  15
#define GPU_TW_MASK_Y_SHIFT 5

/* Rectangle record fields read by tex_window_word. */
#define GPU_RECT_X          0x0u /* byte */
#define GPU_RECT_Y          0x2u /* byte */
#define GPU_RECT_W          0x4u /* halfword */
#define GPU_RECT_H          0x6u /* halfword */

/* Packet link word: 8-bit length in the top byte, 24-bit address below. */
#define GPU_LINK_ADDR_MASK  0x00FFFFFFu

/* Negative inputs become 0 (the limit is not read); an input at or past the
 * limit becomes limit - 1.  Result keeps the low 10 bits. */
static uint32_t clamp_coord(short v, uint32_t limit_addr)
{
    int n = v, limit;
    if (n < 0)
        return 0u;
    limit = (short)PE_LoadU16(limit_addr);
    if (n >= limit)
        n = limit - 1;
    return (uint32_t)n & GPU_COORD_MASK;
}

static uint32_t draw_area_word(uint32_t cmd, short x, short y)
{
    uint32_t cx = clamp_coord(x, GPU_LIMIT_X);
    uint32_t cy = clamp_coord(y, GPU_LIMIT_Y);
    return cmd | (cy << GPU_Y_SHIFT) | cx;
}

uint32_t pe_plat_gpu_draw_area_tl_word(short x, short y)
{
    return draw_area_word(GP0_DRAW_AREA_TL, x, y);
}

uint32_t pe_plat_gpu_draw_area_br_word(short x, short y)
{
    return draw_area_word(GP0_DRAW_AREA_BR, x, y);
}

/* Window size is stored as (-size) in 8-texel units, offset as origin / 8. */
uint32_t pe_plat_gpu_tex_window_word(uint32_t rect)
{
    uint32_t x, y, w, h;
    if (rect == 0u)
        return 0u;
    x = PE_LoadU8(rect + GPU_RECT_X);
    w = PE_LoadU16(rect + GPU_RECT_W);
    y = PE_LoadU8(rect + GPU_RECT_Y);
    h = PE_LoadU16(rect + GPU_RECT_H);
    return GP0_TEX_WINDOW
         | (((0u - w) >> GPU_TW_UNIT_SHIFT) & GPU_TW_MASK)
         | ((((0u - h) >> GPU_TW_UNIT_SHIFT) & GPU_TW_MASK) << GPU_TW_MASK_Y_SHIFT)
         | (((x >> GPU_TW_UNIT_SHIFT) & GPU_TW_MASK) << GPU_TW_OFF_X_SHIFT)
         | (((y >> GPU_TW_UNIT_SHIFT) & GPU_TW_MASK) << GPU_TW_OFF_Y_SHIFT);
}

/* Insert prim after the ordering-table head: prim takes over the head's
 * link address, the head now points at prim; length bytes stay in place. */
void pe_plat_gpu_add_prim(uint32_t ot, uint32_t prim)
{
    uint32_t head = PE_LoadU32(ot);
    uint32_t pw = PE_LoadU32(prim);
    PE_StoreU32(prim, (pw & ~GPU_LINK_ADDR_MASK) | (head & GPU_LINK_ADDR_MASK));
    PE_StoreU32(ot, (head & ~GPU_LINK_ADDR_MASK) | (prim & GPU_LINK_ADDR_MASK));
}
