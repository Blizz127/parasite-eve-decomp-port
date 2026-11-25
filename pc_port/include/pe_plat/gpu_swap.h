/* GPU command enqueue / immediate issue (guest 0x80076C34, "text_21F0" in the
 * classification); host model in platform/pe_plat_gpu_swap.c. */
#ifndef PE_PLAT_GPU_SWAP_H
#define PE_PLAT_GPU_SWAP_H
#include <stdint.h>
#include "pe_guest_ram.h"
int32_t pe_plat_gpu_swap(pe_addr_t worker, pe_addr_t argument, int32_t copy_bytes, uint32_t auxiliary);
#define func_80076C34 pe_plat_gpu_swap
/* Canonical guest prototype (expands to the pe_plat declaration above). */
int32_t func_80076C34(pe_addr_t worker, pe_addr_t argument, int32_t copy_bytes, uint32_t auxiliary);
int32_t pe_plat_gpu_swap_inline8(pe_addr_t worker, uint32_t word0, uint32_t word1, uint32_t auxiliary);
/* Immediate issue of a worker whose payload is two caller-held words: 1 = issued by a host model, 0 = no model. */
int pe_plat_gpu_swap_inline_issue(pe_addr_t worker, uint32_t word0, uint32_t word1, uint32_t auxiliary);
/* Diagnostic record of the dispatch (host-only, see pe_plat_gpu_swap_host.c). */
void pe_plat_gpu_swap_inline_record(pe_addr_t worker, const uint32_t payload[2], uint32_t auxiliary);
#define PE_func_80076C34_Inline8 pe_plat_gpu_swap_inline8
int32_t PE_func_80076C34_Inline8(pe_addr_t worker, uint32_t word0, uint32_t word1, uint32_t auxiliary);
#endif
