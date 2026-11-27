/* libgpu rectangle-clear worker (guest 0x80076434 func_80076434(rect, color)), written clean-room from the
 * black-box vector set cdgen53 (docs/evidence/native-cdx-20261126/cdgen53: 1500 retail-derived cases, load / store /
 * callee logs), the public headers (pe_plat/gpu_regs.h, gpu_dma_start.h, ctl.h, psx_compat.h), the house-style
 * providers pe_plat2_gpuimg_work.c and pe_plat2_gpuprim_clearrect.c, the suite tests of the clear path
 * (test_clear_image.h) and psx-spx (GP0 E1h / E3h / E4h / E5h / E6h / 60h, GP1 10h get-GPU-info, GPUSTAT).
 * No SDK source, decompiled source, hand port or disassembly was read.
 *
 * Contract (every line traced to the vectors):
 *  - RECT = x, y, w, h halfwords.  w (rect+4) and h (rect+6) are clamped and written back, w first: a negative
 *    value becomes 0 WITHOUT reading the limit; otherwise the limit halfword (0x80095750 for w, 0x80095752 for h)
 *    is read as a signed value and a value >= limit becomes limit - 1 (stored as a halfword: limit 0 gives 0xFFFF,
 *    limit 0x8000 reads as -32768 and gives 0x7FFF).  Both halfwords are stored back even when unchanged.
 *  - x (rect+0, halfword) is read next.  If (x | w) is a multiple of 64 (w after the clamp) the clear is the fast
 *    form, otherwise the general form.  Then the GPU status is read through the pointer word 0x80095854 (which
 *    holds the GP1/GPUSTAT register address), whatever the form.
 *  - The packet lives at 0x800A3300.  Fast form (5 words after the header, list ends there), stores in this order:
 *      +00 0x05FFFFFF, +04 0xE6000000, +0C 0x02000000 | (color & 0xFFFFFF), +08 E1, +10 position word, +14 size word.
 *    General form: two linked lists, stores in this order:
 *      +00 0x080A3328 (8 words, next node 0x800A3328), +10 0xE6000000, +04 0xE3000000, +08 0xE4FFFFFF,
 *      +0C 0xE5000000, +18 0x60000000 | (color & 0xFFFFFF), +14 E1, +1C position word, +28 0x03FFFFFF (3 words, end),
 *      +20 size word; then three GP1 get-info queries (0x80076BE0 with 3, 4, 5: drawing-area top-left, bottom-right,
 *      drawing offset), the result of each OR-ed WITHOUT a mask into 0xE3000000 / 0xE4000000 / 0xE5000000 and stored
 *      right after its query at +2C / +30 / +34 (the restore list that undoes the 0xE3 / 0xE4 / 0xE5 words above).
 *    E1 = 0xE1000000 | (status & 0x7FF) | ((color >> 21) & 0x400): the low 11 bits of GPUSTAT (texture page,
 *    semi-transparency, dither, draw-to-display) with bit 10 taken from colour bit 31 (the vectors show both).
 *    The position and size words are LOADED from the RECT at the moment of their store (so a RECT that overlaps the
 *    packet sees the words stored before it: the suite test clear_image_packet_rect_alias).
 *  - The packet is started with pe_plat_gpu_dma_start(0x800A3300, 0) (retail 0x80076B98 with one argument); the
 *    worker returns 0.
 *  - Host: the status pointer word is used raw when it is in the hardware register page (0x1F80xxxx, even
 *    unaligned: the explorer cases) or guest RAM; otherwise (library state never set up, as in unit tests) the status
 *    register address is read and the three restore words take the host GPU model's drawing area / offset (masked to
 *    20 / 20 / 22 bits) instead of a query through the pointers.  The inline entry takes the RECT as two host words and writes the clamped size
 *    halfwords back into words[1] (no guest memory is touched for the RECT). */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/gpu_dma_start.h"
#include "pe_plat/gpu_hw.h"
#include "pe_plat/ctl.h"
#include "pe_plat/gpuclear.h"

#define LIM_W    0x80095750u
#define LIM_H    0x80095752u
#define P_GP1    0x80095854u
#define PACKET   0x800A3300u
#define HW_GP1   0x1F801814u

/* GPUSTAT through the pointer word: the pointer is used as it is (the vectors with a perturbed, even unaligned,
 * register address read exactly that address; there is no alignment or range check in the original).  Host: a
 * pointer word that is neither in the hardware register page nor guest RAM (library state never set up, as in unit
 * tests) reads the status register itself. */
static uint32_t read_status(int *pointer_ok)
{
    uint32_t p = PE_LoadU32(P_GP1);
    *pointer_ok = (p >> 16) == 0x1F80u || PE_RangeIsRam(p, 4u);
    return PE_LoadU32(*pointer_ok ? p : HW_GP1);
}

/* A RECT either in guest memory (words == NULL) or held as two host words. */
typedef struct { pe_addr_t guest; uint32_t *words; } ClearRect;

static uint16_t rect_half(const ClearRect *r, unsigned index)   /* 0 x, 1 y, 2 w, 3 h */
{
    if (r->words) return (uint16_t)(r->words[index >> 1] >> ((index & 1u) * 16u));
    return PE_LoadU16(r->guest + 2u * index);
}

static void rect_set_half(ClearRect *r, unsigned index, uint16_t value)
{
    if (r->words) {
        unsigned shift = (index & 1u) * 16u;
        r->words[index >> 1] = (r->words[index >> 1] & ~(0xFFFFu << shift)) | ((uint32_t)value << shift);
    } else {
        PE_StoreU16(r->guest + 2u * index, value);
    }
}

static uint32_t rect_word(const ClearRect *r, unsigned index)   /* 0 position word, 1 size word */
{
    return r->words ? r->words[index] : PE_LoadU32(r->guest + 4u * index);
}

/* Clamp the halfword at `index` against the limit halfword at `limit` and store it back; returns the clamped value
 * as the signed integer the comparison used (limit - 1 is not truncated here). */
static int32_t clamp_half(ClearRect *r, unsigned index, pe_addr_t limit)
{
    int32_t v = (int16_t)rect_half(r, index);
    if (v < 0) v = 0;
    else {
        int32_t lim = (int16_t)PE_LoadU16(limit);
        if (v >= lim) v = lim - 1;
    }
    rect_set_half(r, index, (uint16_t)v);
    return v;
}

/* The three restore words 0xE3 / 0xE4 / 0xE5 | get-info result, stored right after each query (see the contract
 * above): the results are not masked.
 * Host only: when the GP1 pointer word (the one the status was read through) was never set up (library state of a
 * unit test, where retail would fault) the host model's own drawing area / offset are used instead of a query
 * through the pointers, masked to the field widths of the GP0 commands (20 / 20 / 22 bits). */
static void store_restore_words(int pointer_ok)
{
    if (pointer_ok) {
        PE_StoreU32(PACKET + 0x2Cu, 0xE3000000u | pe_plat_ctl_gpu_query(3u));
        PE_StoreU32(PACKET + 0x30u, 0xE4000000u | pe_plat_ctl_gpu_query(4u));
        PE_StoreU32(PACKET + 0x34u, 0xE5000000u | pe_plat_ctl_gpu_query(5u));
    } else {
        PeGpuState gpu;
        PE_GPU_GetState(&gpu);
        PE_StoreU32(PACKET + 0x2Cu, 0xE3000000u | (gpu.drawing_area_top_left & 0xFFFFFu));
        PE_StoreU32(PACKET + 0x30u, 0xE4000000u | (gpu.drawing_area_bottom_right & 0xFFFFFu));
        PE_StoreU32(PACKET + 0x34u, 0xE5000000u | (gpu.drawing_offset & 0x3FFFFFu));
    }
}

static int run_clear(ClearRect *r, uint32_t color)
{
    int32_t w, x;
    int pointer_ok;
    uint32_t status, mode, fill;
    w = clamp_half(r, 2, LIM_W);
    (void)clamp_half(r, 3, LIM_H);
    x = (int16_t)rect_half(r, 0);
    status = read_status(&pointer_ok);
    mode = 0xE1000000u | (status & 0x7FFu) | ((color >> 21) & 0x400u);
    fill = color & 0xFFFFFFu;
    if (((x | w) & 63) == 0) {
        PE_StoreU32(PACKET + 0x00u, 0x05FFFFFFu);
        PE_StoreU32(PACKET + 0x04u, 0xE6000000u);
        PE_StoreU32(PACKET + 0x0Cu, 0x02000000u | fill);
        PE_StoreU32(PACKET + 0x08u, mode);
        PE_StoreU32(PACKET + 0x10u, rect_word(r, 0));
        PE_StoreU32(PACKET + 0x14u, rect_word(r, 1));
    } else {
        PE_StoreU32(PACKET + 0x00u, 0x080A3328u);
        PE_StoreU32(PACKET + 0x10u, 0xE6000000u);
        PE_StoreU32(PACKET + 0x04u, 0xE3000000u);
        PE_StoreU32(PACKET + 0x08u, 0xE4FFFFFFu);
        PE_StoreU32(PACKET + 0x0Cu, 0xE5000000u);
        PE_StoreU32(PACKET + 0x18u, 0x60000000u | fill);
        PE_StoreU32(PACKET + 0x14u, mode);
        PE_StoreU32(PACKET + 0x1Cu, rect_word(r, 0));
        PE_StoreU32(PACKET + 0x28u, 0x03FFFFFFu);
        PE_StoreU32(PACKET + 0x20u, rect_word(r, 1));
        store_restore_words(pointer_ok);
    }
    (void)pe_plat_gpu_dma_start(PACKET, 0u);
    return 0;
}

int pe_plat_gpu_clear_worker_host(pe_addr_t rect, uint32_t color)
{
    ClearRect r = { rect, 0 };
    return run_clear(&r, color);
}

int pe_plat_gpu_clear_inline_host(uint32_t words[2], uint32_t color)
{
    ClearRect r = { 0, words };
    return run_clear(&r, color);
}
