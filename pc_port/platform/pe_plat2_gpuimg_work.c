/* Image-transfer workers (guest 0x80076664 LoadImage worker, 0x800768A0 StoreImage worker), written clean-room from
 * the black-box vector sets gpu2b (docs/evidence/native-gpu2-20261126), the port's host GPU/DMA model API and
 * psx-spx (GP0 A0h/C0h image transfers, GP1(04h) DMA direction, DMA channel 2 MADR/BCR/CHCR, GPUSTAT bits 26/27);
 * no SDK source, decompiled source, tables or disassembly were read.
 *
 * Contract (every line traced to the vectors):
 *  - both first call the deadline seeder 0x800773D0.  The rectangle's width and height halfwords (rect+4, rect+6)
 *    are clamped and written back: a negative value becomes 0, otherwise a value above the halfword limit
 *    0x80095750 (width) / 0x80095752 (height) becomes the limit (the limit itself may be negative).
 *    The word count is words = (w * h + 1) / 2 of the clamped values; words <= 0 returns -1 with no hardware access.
 *  - the hardware registers are reached through pointer words: GP0 0x80095850, GP1/GPUSTAT 0x80095854, DMA2 MADR
 *    0x80095858, BCR 0x8009585C, CHCR 0x80095860; every access reloads its pointer word.
 *  - LoadImage: wait until GPUSTAT bit 26 is set (each failed poll calls the timeout check 0x80077404; non-zero returns
 *    -1), GP1 0x04000000, GP0 0x01000000, 0xA0000000, the rectangle's position word, its size word (read back after
 *    the clamp); then the first (words & 15) source words are written to GP0 one by one; when words >= 16 the rest
 *    goes by DMA: GP1 0x04000002, MADR = source + 4 * (words & 15), BCR = (words >> 4) << 16 | 16, CHCR 0x01000201.
 *  - StoreImage: the same wait and set-up with 0xC0000000 and no data, then wait until GPUSTAT bit 27 is set (same
 *    timeout rule); the first (words & 15) words are read from GP0 and stored to the destination; the rest by DMA: GP1
 *    0x04000003, MADR = destination + 4 * (words & 15), BCR as above, CHCR 0x01000200.  Both return 0 when done.
 *  - Host-only (not observable in the vectors, which only hold aligned in-RAM buffers and a model that is always
 *    ready): (1) a data buffer that is NULL, unaligned or whose whole span is not guest RAM returns -1 before any
 *    hardware access (the retained translation behaviour); (2) after the first wait and before the first register
 *    write the host GPU model must be able to take an image (parser idle, DMA2 idle, DMA2 enabled in DPCR for a
 *    transfer that needs it: PE_GPU_CanBeginImageLoad), otherwise -1 with no register access; (3) each failed
 *    poll lets the host DMA transfer progress (pe_plat_gpu_host_wait_tick) and a stop request ends the wait; a wait
 *    that the timeout never ends (WAIT_BUDGET polls) requests a stop; (4) a register pointer word that is neither
 *    RAM nor a modelled register falls back to the register's address; (5) the final CHCR start goes through the host
 *    model's DMA2 issue (PE_GPU_DMA2Issue, which reads the source at issue as the hardware does) when the CHCR pointer
 *    word is the real register address, and is a plain register store otherwise (or when the model declines). */
#include <stdint.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "game_port.h"
#include "pe_plat/gpu_timeout.h"
#include "pe_plat/gpu_hw.h"
#include "pe_plat/gpu_regs.h"
#include "pe_plat/gpu_host.h"
#include "pe_plat/gpuimg.h"

#define LIM_W      0x80095750u
#define LIM_H      0x80095752u
#define P_GP0      0x80095850u
#define P_GP1      0x80095854u
#define P_MADR     0x80095858u
#define P_BCR      0x8009585Cu
#define P_CHCR     0x80095860u
#define HW_GP0     0x1F801810u
#define HW_GP1     0x1F801814u
#define HW_MADR    0x1F8010A0u
#define HW_BCR     0x1F8010A4u
#define HW_CHCR    0x1F8010A8u
#define STAT_READY_CMD  0x04000000u
#define STAT_READY_READ 0x08000000u
#define CHCR_LOAD  0x01000201u   /* start, memory to GPU, sync mode 1 (psx-spx DMA2 CHCR) */
#define CHCR_STORE 0x01000200u   /* start, GPU to memory, sync mode 1 */
#define WAIT_BUDGET 4000000u

static void gp0(uint32_t value) { PE_StoreU32(pe_gpu_reg_word(P_GP0, HW_GP0, 4), value); }
static void gp1(uint32_t value) { PE_StoreU32(pe_gpu_reg_word(P_GP1, HW_GP1, 4), value); }

/* MADR, BCR, then the CHCR start (see host-only item 5 above). */
static void start_dma(uint32_t madr, uint32_t bcr, uint32_t chcr)
{
    uint32_t chcr_reg;
    PE_StoreU32(pe_gpu_reg_word(P_MADR, HW_MADR, 4), madr);
    PE_StoreU32(pe_gpu_reg_word(P_BCR, HW_BCR, 4), bcr);
    chcr_reg = pe_gpu_reg_word(P_CHCR, HW_CHCR, 4);
    if (chcr_reg == HW_CHCR && PE_GPU_DMA2Issue(madr, bcr, chcr)) return;
    PE_StoreU32(chcr_reg, chcr);
}

/* Host guard: a data buffer of `words` words that the hardware could not reach (NULL, unaligned, outside RAM). */
static int buffer_ok(pe_addr_t buffer, int32_t words)
{
    size_t bytes = (size_t)words * 4u;
    return buffer != 0u && (buffer & 3u) == 0u && (uint32_t)words <= 0x3FFFFFFFu &&
           PE_RangeIsRam(PE_RamCanonical(buffer, bytes), bytes);
}

/* Poll GPUSTAT until `mask` is set.  1 = ready, 0 = give up with *ret (timeout: -1, stop: 0). */
static int wait_status(uint32_t mask, int *ret)
{
    uint32_t polls = 0;
    for (;;) {
        if (PE_LoadU32(pe_gpu_reg_word(P_GP1, HW_GP1, 4)) & mask) return 1;
        if (pe_plat_gpu_timeout() != 0) { *ret = -1; return 0; }
        if (PE_Port_ShouldStop()) { *ret = 0; return 0; }
        if (++polls > WAIT_BUDGET) { PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY); *ret = 0; return 0; }
        if (pe_plat_gpu_host_wait_tick() < 0) { *ret = 0; return 0; }
    }
}

/* Clamp the size halfwords (written back for a guest rectangle) and return the word count; words <= 0 means nothing
 * to do. */
static int32_t clamp(uint32_t rect, uint32_t in_wh, int32_t *cw, int32_t *ch)
{
    int32_t w, h;
    w = (int16_t)(rect ? PE_LoadU16(rect + 4u) : (uint16_t)in_wh);
    if (w < 0) w = 0;
    else if (w > (int16_t)PE_LoadU16(LIM_W)) w = (int16_t)PE_LoadU16(LIM_W);
    h = (int16_t)(rect ? PE_LoadU16(rect + 6u) : (uint16_t)(in_wh >> 16));
    if (rect) PE_StoreU16(rect + 4u, (uint16_t)w);
    if (h < 0) h = 0;
    else if (h > (int16_t)PE_LoadU16(LIM_H)) h = (int16_t)PE_LoadU16(LIM_H);
    if (rect) PE_StoreU16(rect + 6u, (uint16_t)h);
    *cw = w; *ch = h;
    return (w * h + 1) / 2;
}

/* The position and size words given to GP0 (a guest rectangle is read back after the clamp). */
static void rect_words(uint32_t rect, uint32_t in_xy, int32_t w, int32_t h, uint32_t *xy, uint32_t *wh)
{
    if (rect) { *xy = PE_LoadU32(rect); *wh = PE_LoadU32(rect + 4u); }
    else { *xy = in_xy; *wh = ((uint32_t)(uint16_t)h << 16) | (uint16_t)w; }
}

static int upload(uint32_t rect, uint32_t in_xy, uint32_t in_wh, pe_addr_t source)
{
    uint32_t xy, wh, count, blocks, i;
    int32_t words, w, h;
    int ret = 0;
    (void)func_800773D0();
    words = clamp(rect, in_wh, &w, &h);
    if (words <= 0) return -1;
    count = (uint32_t)words & 15u; blocks = (uint32_t)words >> 4;
    if (!buffer_ok(source, words)) return -1;
    if (!wait_status(STAT_READY_CMD, &ret)) return ret;
    if (!PE_GPU_CanBeginImageLoad(blocks != 0u)) return -1;
    rect_words(rect, in_xy, w, h, &xy, &wh);
    gp1(0x04000000u);
    gp0(0x01000000u);
    gp0(0xA0000000u);
    gp0(xy);
    gp0(wh);
    for (i = 0; i < count; i++) gp0(PE_LoadU32(source + 4u * i));
    if (blocks != 0u) {
        gp1(0x04000002u);
        start_dma(source + 4u * count, (blocks << 16) | 16u, CHCR_LOAD);
    }
    return 0;
}

static int readback(uint32_t rect, uint32_t in_xy, uint32_t in_wh, pe_addr_t destination)
{
    uint32_t xy, wh, count, blocks, i;
    int32_t words, w, h;
    int ret = 0;
    (void)func_800773D0();
    words = clamp(rect, in_wh, &w, &h);
    if (words <= 0) return -1;
    count = (uint32_t)words & 15u; blocks = (uint32_t)words >> 4;
    if (!buffer_ok(destination, words)) return -1;
    if (!wait_status(STAT_READY_CMD, &ret)) return ret;
    if (!PE_GPU_CanBeginImageLoad(blocks != 0u)) return -1;
    rect_words(rect, in_xy, w, h, &xy, &wh);
    gp1(0x04000000u);
    gp0(0x01000000u);
    gp0(0xC0000000u);
    gp0(xy);
    gp0(wh);
    if (!wait_status(STAT_READY_READ, &ret)) return ret;
    for (i = 0; i < count; i++) PE_StoreU32(destination + 4u * i, PE_LoadU32(pe_gpu_reg_word(P_GP0, HW_GP0, 4)));
    if (blocks != 0u) {
        gp1(0x04000003u);
        start_dma(destination + 4u * count, (blocks << 16) | 16u, CHCR_STORE);
    }
    return 0;
}

int pe_plat_gpuimg_upload_worker(pe_addr_t rect, pe_addr_t source) { return upload(rect, 0u, 0u, source); }
int pe_plat_gpuimg_readback_worker(pe_addr_t rect, pe_addr_t destination) { return readback(rect, 0u, 0u, destination); }
int pe_plat_gpuimg_upload_inline(uint32_t position, uint32_t size, pe_addr_t source) { return upload(0u, position, size, source); }
int pe_plat_gpuimg_readback_inline(uint32_t position, uint32_t size, pe_addr_t destination) { return readback(0u, position, size, destination); }
