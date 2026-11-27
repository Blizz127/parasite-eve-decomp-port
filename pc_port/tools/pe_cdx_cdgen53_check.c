/* Checker for the cdgen53 set (pe_cdx_cdgen52_61_vectors.py --set cdgen53): libgpu clear worker, retail 0x80076434(rect, color): a guest RECT (x, y, w, h halfwords)
 * and a colour; clamps w / h to the screen-limit halfwords 0x80095750 / 52 (written back into the RECT), builds the clear packet at 0x800A3300.. (the fast
 * 0xE6 / 0xE1 / fill form for a 64-aligned x and w, the general 0xE3 / 0xE4 / 0xE5 + 0x60 form otherwise) and starts it through 0x80076B98.  Scaffold
 * pe_cdx_common_canary.h (canary seeding, store-footprint, controls noop / flip / retflip / storeflip / poke0 / same).  The object is
 * pc_port/platform/pe_plat_gpu_clear_worker_host.c of the int3 tree (read-only) with two entries: pe_plat_gpu_clear_worker_host(rect, color) (guest RECT) and
 * pe_plat_gpu_clear_inline_host(words[2], color) (the RECT as two host words).  Seams (nm -u): pe_plat_gpu_dma_start(packet, aux) = retail 0x80076B98 (the
 * original passes ONE argument, the packet: arity 1, the provider's second argument is not compared); PE_GPU_ReadStatus() = the replayed register read of 0x1F801814;
 * PE_GPU_GetState(&state): the drawing area / offset the ORIGINAL obtains with three GP1 get-info calls (0x80076BE0 with 3 / 4 / 5, stopped callees).
 *   strict (default): GetState reports a zero state, no call recorded;  -DC53_MAP_INFO=1 (mapped): GetState performs the three get-info calls (recorded callees,
 *   scripted results) and reports their values: the most generous mapping.  -DC53_INLINE=1: entry = the inline-words variant (words copied in from the guest RECT
 *   and the clamped width / height copied back into it afterwards).
 * A provider that calls pe_plat_ctl_gpu_query (0x80076BE0) three times itself and reads the status through the pointer word needs no mapping: use BUILD-worker (strict) with its own object.
 * Builds (executed by pe_cdx_cdgen52_61_run.sh; $O = scratch output directory):
 * BUILD-worker: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen53_check.c $O/pe_plat_gpu_clear_worker_host.o -o $O/c53_worker
 * BUILD-worker-mapped: cc -std=gnu11 -Wall -Wextra -Werror -DC53_MAP_INFO=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen53_check.c $O/pe_plat_gpu_clear_worker_host.o -o $O/c53_worker_mapped
 * BUILD-inline-mapped: cc -std=gnu11 -Wall -Wextra -Werror -DC53_MAP_INFO=1 -DC53_INLINE=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen53_check.c $O/pe_plat_gpu_clear_worker_host.o -o $O/c53_inline_mapped
 */
#include "pe_cdx_common_canary.h"
#include "pe_gpu.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
int pe_plat_gpu_dma_start(uint32_t packet, uint32_t auxiliary) { (void)auxiliary; return (int)cdx_callee(0x80076B98u, 1, packet, 0, 0, 0, 0); }
uint32_t PE_GPU_ReadStatus(void) { return PE_LoadU32(0x1F801814u); }
void PE_GPU_GetState(PeGpuState *out)
{
    memset(out, 0, sizeof *out);
#if C53_MAP_INFO
    out->drawing_area_top_left = cdx_callee(0x80076BE0u, 1, 3, 0, 0, 0, 0);
    out->drawing_area_bottom_right = cdx_callee(0x80076BE0u, 1, 4, 0, 0, 0, 0);
    out->drawing_offset = cdx_callee(0x80076BE0u, 1, 5, 0, 0, 0, 0);
#endif
}
/* Seams of a provider that makes the three get-info calls itself and reads the status through the GP1 pointer word (strict build for such an object; defined for every
 * build, an object that does not reference them ignores them):
 *   pe_plat_ctl_gpu_query(query) = retail 0x80076BE0 (arity 1, scripted result): the three calls (3, 4, 5) are recorded callees, each followed by the provider's store;
 *   PE_RangeIsRam(addr, size) / PE_MMIO_IsModelled(addr, size): pure predicates (inline pe_gpu_reg_word() of pe_plat/gpu_regs.h): RAM for the 2 MiB window, modelled for the
 *   0x1F80xxxx register page (the status read through the pointer word is then the replayed register read of 0x1F801814, like PE_GPU_ReadStatus). */
#include <stdbool.h>
unsigned int pe_plat_ctl_gpu_query(unsigned int query) { return cdx_callee(0x80076BE0u, 1, query, 0, 0, 0, 0); }
bool PE_RangeIsRam(pe_addr_t a, size_t n) { return (uint64_t)(a & 0x1FFFFFFFu) + n <= (uint64_t)MEM; }
int PE_MMIO_IsModelled(pe_addr_t a, size_t n) { (void)n; return is_reg(a); }
int pe_plat_gpu_clear_worker_host(uint32_t rect, uint32_t color);
int pe_plat_gpu_clear_inline_host(uint32_t words[2], uint32_t color);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    if (entry != 0x80076434u) return -1;
    if (mutant == 1) { *ret = 0; return 1; }
#if C53_INLINE
    {
        uint32_t w[2], r = (uint32_t)PE_LoadU32(a[0]), r2 = (uint32_t)PE_LoadU32(a[0] + 4);
        w[0] = r; w[1] = r2;
        *ret = (uint32_t)pe_plat_gpu_clear_inline_host(w, a[1]);
        PE_StoreU16(a[0] + 4, (uint16_t)w[1]); PE_StoreU16(a[0] + 6, (uint16_t)(w[1] >> 16));
    }
#else
    *ret = (uint32_t)pe_plat_gpu_clear_worker_host(a[0], a[1]);
#endif
    return 1;
}
