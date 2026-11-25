/* Host guard for the GPU/DMA/IRQ register pointer words the clean-room GPU
 * providers dereference (0x80095854 ... ): a pointer that is neither guest RAM
 * nor a modelled MMIO register (uninitialised library state, as in some unit
 * tests) is a named refusal, like pe_plat_resetgraph_light_host.c, instead of a
 * fatal bad guest access.  Not observable in the vectors (their pointers are
 * valid). */
#ifndef PE_PLAT_GPU_REGS_H
#define PE_PLAT_GPU_REGS_H
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_mmio.h"
#include "pe_guestcode.h"

static inline int pe_gpu_reg_ok(uint32_t p, unsigned n)
{
    return (p & (n - 1u)) == 0u && (PE_RangeIsRam(p, n) || PE_MMIO_IsModelled(p, n));
}
static inline int pe_gpu_reg_refuse(const char *site, const char *caller, uint32_t p)
{
    PE_GuestHook_Refuse(site, caller, p, 0, 0, 0, 0, 0, "invalid GPU register pointer");
    return 0;
}
#endif
