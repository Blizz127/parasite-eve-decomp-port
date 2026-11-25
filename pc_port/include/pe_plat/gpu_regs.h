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
/* Register pointer word with the architectural default for library state that was
 * never set up (zero, or any value that is neither guest RAM nor a modelled MMIO
 * register, as with the canary-filled RAM of unit tests; retail would fault).
 * Production boot sets every word to its register address, so the default is never
 * used there, and the vectors (all pointers valid) never see it. */
static inline uint32_t pe_gpu_reg_word(uint32_t word_addr, uint32_t architectural, unsigned size)
{
    uint32_t p = PE_LoadU32(word_addr);
    return pe_gpu_reg_ok(p, size) ? p : architectural;
}
#define PE_GPU_REG_GPU1  0x1F801814u   /* GP1 */
#define PE_GPU_REG_MADR2 0x1F8010A0u
#define PE_GPU_REG_BCR2  0x1F8010A4u
#define PE_GPU_REG_CHCR2 0x1F8010A8u
#define PE_GPU_REG_IMASK 0x1F801074u

static inline int pe_gpu_reg_refuse(const char *site, const char *caller, uint32_t p)
{
    PE_GuestHook_Refuse(site, caller, p, 0, 0, 0, 0, 0, "invalid GPU register pointer");
    return 0;
}
#endif
