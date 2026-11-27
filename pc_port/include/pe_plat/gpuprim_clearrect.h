/* Host seams of the libgpu ClearImage path (guest 0x80074F44, pe_plat2_gpuprim.c) and the OT-clear worker
 * (guest 0x80076354, pe_plat2_gpuprim_otcw.c); provider platform/pe_plat2_gpuprim_clearrect.c, written
 * clean-room (see its header comment).  Symbol names are kept from the retired translated file
 * game/boot/func_80076C34_port.c so that the callers are unchanged. */
#ifndef PE_PLAT_GPUPRIM_CLEARRECT_H
#define PE_PLAT_GPUPRIM_CLEARRECT_H
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/rect.h"

/* ClearImage with a RECT the host holds (a caller's local, no guest address) when guest table slot 2 is the swap
 * 0x80076C34: the retail dispatch with the RECT's two words held by the host (no guest RAM is written for it).  `worker` is slot 3, `color` is
 * red | green << 8 | blue << 16.  Returns what the swap returns (0 issued, the pending count queued, -1 timed out);
 * *rect holds the clamped geometry after an immediate issue and is unchanged after a queued one. */
int PE_DispatchClearRect(pe_addr_t worker, RECT *rect, uint32_t color);

/* Deadline seed (guest 0x800773D0) as a host call; returns the seeded deadline. */
uint32_t pe_plat_gpu_seed_deadline_host(void);
#endif
