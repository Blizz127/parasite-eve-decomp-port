/* Host side of the caller-stack payload issue of 0x80076C34 (see pe_plat_gpu_swap.c).  The retail callers
 * that build the two-word payload in their own frame pass its stack address to the worker; the host has no
 * guest address for it, so the workers that own a host model taking the payload words are called through
 * those models, with the same (payload, auxiliary) = (a0, a1) the worker gets.  Mapping (host model
 * signatures in psx_compat.h / the *_worker_host.c files): 0x80076664 upload (xy, wh, source),
 * 0x80076434 clear (words, colour), 0x800768A0 readback (xy, wh, destination). */
#include <stdint.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"
#include "pe_plat/gpu_swap.h"
#include "pe_bootstrap.h"
#include <string.h>

/* Diagnostic-only record (host-only, never written to guest RAM) of the retail indirect call through the
 * library jump-table word jtb[2] that the two-word-payload callers make: symbol func_80076C34, caller
 * func_8007506C (the only such caller), target = jtb[2], a0 = worker, a1 = 0 (the caller-stack payload has no
 * guest address), a2 = 8, a3 = auxiliary, snapshot = the two payload words.  Contract: pc_port/tests
 * (B52_AssertGpuCall, count-of-dispatches assertions). */
void pe_plat_gpu_swap_inline_record(pe_addr_t worker, const uint32_t payload[2], uint32_t auxiliary)
{
    uint32_t jtb = PE_RangeIsRam(0x80095744u, 4u) ? PE_LoadU32(0x80095744u) : 0u;
    uint32_t target = jtb != 0u && PE_RangeIsRam(jtb + 8u, 4u) ? PE_LoadU32(jtb + 8u) : 0u;
    uint8_t snapshot[8];
    memcpy(snapshot, payload, 8u);
    Bootstrap_RecordArg4Indirect("func_80076C34", "func_8007506C", target, worker, 0u, 8u, auxiliary, snapshot, 8u);
}

int pe_plat_gpu_swap_inline_issue(pe_addr_t worker, uint32_t word0, uint32_t word1, uint32_t auxiliary)
{
    uint32_t words[2];
    switch (worker) {
    case 0x80076664u:
        (void)pe_plat_gpu_upload_inline_host(word0, word1, auxiliary);
        return 1;
    case 0x80076434u:
        words[0] = word0;
        words[1] = word1;
        (void)pe_plat_gpu_clear_inline_host(words, auxiliary);
        return 1;
    case 0x800768A0u:
        (void)pe_plat_gpu_readback_inline_host(word0, word1, auxiliary);
        return 1;
    default:
        return 0;
    }
}
