/* Host seams PE_DispatchClearRect and pe_plat_gpu_seed_deadline_host, written clean-room from their callers'
 * contracts: pe_plat2_gpuprim.c (ClearImage, which documents the seam: "same argument packing, in-place clamp"
 * when slot 2 is the swap 0x80076C34), pe_plat2_gpuprim_otcw.c (the deadline seed call), the clean-room swap
 * pe_plat_gpu_swap.c / gpu_swap.h and the queue provider pe_plat2_gpuq.c / gpuq.h, the tests in pc_port/tests
 * (test_clear_image.h: DAY1_clear_sdk_lifetime, DAY1_clear_sdk_original_arguments, DAY1_clear_cases /
 * retail_clear_image_cases.h for the clamp rule) and the gpu2c vectors that define ClearImage's guest-RECT behaviour.
 * No SDK source, decompiled source, hand port or disassembly was read.
 *
 *  - PE_DispatchClearRect(worker, rect, color): the retail call is swap(worker, &rect, 8, color) with the RECT in
 *    the caller's stack frame.  The host RECT has no guest address and the port's tests require that no guest RAM
 *    outside the retail writes changes, so it is not staged in guest memory: the two RECT words go through the
 *    clean-room swap's caller-held-payload entry pe_plat_gpu_swap_inline8, which records the dispatch as the retail
 *    indirect call (target = slot 2, a0 = worker, a1 = 0, a2 = 8, a3 = color, snapshot = the RECT words), then
 *    either issues the worker immediately on those words or copies them into the ring entry (queued).
 *  - In-place clamp: the clear worker clamps width and height of the RECT it is given against the limits
 *    int16 at 0x80095750 (width) / 0x80095752 (height) to [0, limit - 1] (retail_clear_image_cases.h rect_out;
 *    DAY1_clear_sdk_lifetime: 1024 -> 1023 with limit 1024).  The caller's RECT sees that clamp after an
 *    immediate issue (swap result 0, ring write index 0x80095874 unchanged, no stop) of the clear worker 0x80076434;
 *    after a queued issue (the ring index advanced; the clamp happens later on the ring copy) the RECT is untouched.
 *  - pe_plat_gpu_seed_deadline_host(): calls the clean-room seed function 0x800773D0 and returns its result. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_plat/gpu_swap.h"
#include "pe_plat/gpuq.h"
#include "pe_plat/gpuprim_clearrect.h"

#define W_RING_WRITE   0x80095874u   /* ring producer index */
#define H_LIMIT_W      0x80095750u   /* int16 width limit */
#define H_LIMIT_H      0x80095752u   /* int16 height limit */
#define CLEAR_WORKER   0x80076434u

static int16_t clamp_extent(int16_t value, int16_t limit)
{
    if (value < 0) return 0;
    if (value >= limit) return (int16_t)(limit - 1);
    return value;
}

int PE_DispatchClearRect(pe_addr_t worker, RECT *rect, uint32_t color)
{
    uint32_t w0 = (uint32_t)(uint16_t)rect->x | ((uint32_t)(uint16_t)rect->y << 16);
    uint32_t w1 = (uint32_t)(uint16_t)rect->w | ((uint32_t)(uint16_t)rect->h << 16);
    uint32_t producer = PE_LoadU32(W_RING_WRITE);
    int32_t result = pe_plat_gpu_swap_inline8(worker, w0, w1, color);
    if (result == 0 && worker == CLEAR_WORKER && !PE_Port_ShouldStop() && PE_LoadU32(W_RING_WRITE) == producer) {
        rect->w = clamp_extent(rect->w, (int16_t)PE_LoadU16(H_LIMIT_W));
        rect->h = clamp_extent(rect->h, (int16_t)PE_LoadU16(H_LIMIT_H));
    }
    return (int)result;
}

uint32_t pe_plat_gpu_seed_deadline_host(void)
{
    return func_800773D0();
}
