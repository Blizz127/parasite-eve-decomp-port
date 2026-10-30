/* GPU packet builders for SetTexWindow (0x80075B4C) and SetDrawArea
 * (0x80075B84), written clean-room by an author who saw neither SDK source nor
 * this port's earlier code: hardware documentation plus 15902 black-box
 * vectors of the original code (pc_port/tools/pe_gpu_area_vectors.py, checked
 * by pe_gpu_area_vectors_check.c); see docs/evidence/native-gpu-area-20261030/.
 * Bound in pe_plat/gpu_area.h.  psx-spx GPU page:
 *   "GP0(E2h) - Texture Window setting": bits 0-4 mask X, 5-9 mask Y,
 *     10-14 offset X, 15-19 offset Y (all in 8-pixel steps).
 *   "GP0(E3h) - Set Drawing Area top left (X1,Y1)" and
 *   "GP0(E4h) - Set Drawing Area bottom right (X2,Y2)": bits 0-9 X, Y at
 *     bit 10 -- 9 bits on the v0 GPU, 10 bits (10-19) on the v2 GPU.  The
 *     vectors (VRAM size 0 gives Y field 0xFFC00) show the 10-bit form.
 * The packet is a 3-word ordering-table entry: word 0 keeps its 24-bit link
 * and gets word count 2 in the top byte; words 1 and 2 hold the commands. */
#include "pe_guest_ram.h"
#include "pe_plat/gpu_area.h"

#define GPU_VRAM_W_ADDR 0x80095750u /* VRAM width halfword (draw area limit) */
#define GPU_VRAM_H_ADDR 0x80095752u /* VRAM height halfword (draw area limit) */

/* Write the OT header (count 2, link preserved) and both command words. */
static void put_packet(uint32_t packet, uint32_t cmd1, uint32_t cmd2)
{
    PE_StoreU32(packet, (PE_LoadU32(packet) & 0x00FFFFFFu) | 0x02000000u);
    PE_StoreU32(packet + 4u, cmd1);
    PE_StoreU32(packet + 8u, cmd2);
}

/* Narrow to int16 with two's-complement wrap (the RECT sum is 16-bit). */
static int wrap16(int v)
{
    v &= 0xFFFF;
    return v >= 0x8000 ? v - 0x10000 : v;
}

/* Clamp to [0, limit]; the low bound wins first, so limit -1 yields -1. */
static int clamp_area(int v, int limit)
{
    if (v < 0) return 0;
    if (v > limit) return limit;
    return v;
}

/* E3/E4 coordinate pair: 10-bit X, then Y at bit 10 (10 bits). */
static uint32_t area_cmd(uint32_t op, int x, int y)
{
    return (op << 24) | ((uint32_t)x & 0x3FFu) | (((uint32_t)y & 0x3FFu) << 10);
}

void pe_plat_gpu_tex_window_values(uint32_t packet, const int16_t rect[4])
{
    uint32_t cmd = 0;

    if (rect) {
        /* Mask = ~((size - 1) >> 3), offset = pos >> 3, each 5 bits. */
        uint32_t mx = (uint32_t)~((rect[2] - 1) >> 3) & 0x1Fu;
        uint32_t my = (uint32_t)~((rect[3] - 1) >> 3) & 0x1Fu;
        uint32_t ox = (uint32_t)(rect[0] >> 3) & 0x1Fu;
        uint32_t oy = (uint32_t)(rect[1] >> 3) & 0x1Fu;

        cmd = 0xE2000000u | mx | (my << 5) | (ox << 10) | (oy << 15);
    }
    put_packet(packet, cmd, 0);
}

void pe_plat_gpu_draw_area_values(uint32_t packet, const int16_t rect[4])
{
    int xlim = (int16_t)PE_LoadU16(GPU_VRAM_W_ADDR) - 1;
    int ylim = (int16_t)PE_LoadU16(GPU_VRAM_H_ADDR) - 1;
    int x1, y1, x2, y2;

    x1 = clamp_area(rect[0], xlim);
    y1 = clamp_area(rect[1], ylim);
    x2 = clamp_area(wrap16(rect[0] + rect[2] - 1), xlim);
    y2 = clamp_area(wrap16(rect[1] + rect[3] - 1), ylim);
    put_packet(packet, area_cmd(0xE3u, x1, y1), area_cmd(0xE4u, x2, y2));
}

/* Fetch a guest RECT {x, y, w, h} as host int16s. */
static void load_rect(uint32_t rect, int16_t out[4])
{
    int i;

    for (i = 0; i < 4; i++) out[i] = (int16_t)PE_LoadU16(rect + 2u * (uint32_t)i);
}

void pe_plat_gpu_set_tex_window(uint32_t packet, uint32_t rect)
{
    int16_t r[4];

    if (!rect) {
        pe_plat_gpu_tex_window_values(packet, 0);
        return;
    }
    load_rect(rect, r);
    pe_plat_gpu_tex_window_values(packet, r);
}

void pe_plat_gpu_set_draw_area(uint32_t packet, uint32_t rect)
{
    int16_t r[4];

    /* Retail has no rect == 0 case here: guest address 0 is read like any other. */
    load_rect(rect, r);
    pe_plat_gpu_draw_area_values(packet, r);
}
