/* libgpu rectangle-clear worker (guest 0x80076434 ``func_80076434(rect, color)``); provider
 * platform/pe_plat2_gpuclear.c, written clean-room from the black-box vector set cdgen53
 * (docs/evidence/native-cdx-20261126/cdgen53) and psx-spx (see the provider's header comment).
 * Symbol names are kept from the retired host file so that the callers (the queue pump pe_plat2_gpuq.c,
 * the swap host pe_plat_gpu_swap_host.c, the tests) are unchanged. */
#ifndef PE_PLAT_GPUCLEAR_H
#define PE_PLAT_GPUCLEAR_H
#include <stdint.h>
#include "pe_guest_ram.h"

/* The worker on a guest RECT (x, y, w, h halfwords at `rect`): clamps w and h in place against the screen-limit
 * halfwords, builds and starts the clear packet.  Always returns 0. */
int pe_plat_gpu_clear_worker_host(pe_addr_t rect, uint32_t color);
#define func_80076434 pe_plat_gpu_clear_worker_host
/* Canonical guest prototype (expands to the pe_plat declaration above). */
int func_80076434(pe_addr_t rect, uint32_t color);

/* The same worker for a caller that holds the RECT as two host words (position word, size word); the clamped size
 * halfwords are written back into words[1]. */
int pe_plat_gpu_clear_inline_host(uint32_t words[2], uint32_t color);
#define PE_ClearImageInline8 pe_plat_gpu_clear_inline_host
int PE_ClearImageInline8(uint32_t words[2], uint32_t color);
#endif
