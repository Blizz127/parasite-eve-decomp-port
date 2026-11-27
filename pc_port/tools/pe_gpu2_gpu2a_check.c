/* sw-vec-gpu2 batch gpu2a checker: libgpu queue pump func_80076EE4 and the seed-deadline remnant func_800773D0
 * (the dispatcher func_80076C34 itself is already clean-room bound: pe_plat_gpu_swap.c).
 * Generator: pc_port/tools/pe_gpu2_vectors.py gpu2a; contract: pe_gpu2_check_core.h.
 * Groups (select with -D): CHK_PUMP (func_80076EE4 -> pe_plat_gpuq_pump), CHK_SEED (func_800773D0 ->
 * pe_plat_gpuq_seed_deadline).  The build line selects both.
 * Build line (run from the repo root, verbatim; links the clean-room provider pe_plat2_gpuq.c, which binds both entries):
 *   cc -std=gnu11 -O1 -w -DCHK_PUMP -DCHK_SEED -DG2_BATCH_NAME=\"gpu2a\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2a_check.c pc_port/platform/pe_plat2_gpuq.c -o check_gpu2a
 *   ./check_gpu2a vectors/gpu2a/gpu2a.txt
 * Clean-room shapes (no -DG2_SMOKE; a missing provider fails at link):
 *   TEMPLATE cc -std=gnu11 -Wall -Wextra -Werror -DCHK_PUMP -DG2_BATCH_NAME=\"gpu2a-pump\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2a_check.c <provider.c> -o check_gpu2a_pump
 *   TEMPLATE cc -std=gnu11 -Wall -Wextra -Werror -DCHK_SEED -DG2_BATCH_NAME=\"gpu2a-seed\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2a_check.c <provider.c> -o check_gpu2a_seed
 * Callees are checker stubs: the pump calls pe_plat_dma_callback, pe_plat_irq_set_mask and the entry function through
 * PE_GuestHook_Call; the seed reads the VBlank counter through the host hook pe_plat_gpu_host_vsync_query (checker stub: the guest
 * VSync(-1), func_80073A44 = HostFB_VSync).
 * Mutants: -DGPU2_MUTANT=1 (no-op stub), =2 (return bit flip), =3 (store bit flip). */
#if defined(CHK_SEED) || defined(CHK_PUMP)
#define G2_REAL_SEED     /* one provider TU (pe_plat2_gpuq.c) binds both entries: no seed stub next to it */
#endif
#include "pe_gpu2_check_core.h"
#include "pe_guest_ram.h"
#include "pe_gpu2_host_stubs.h"
#ifdef G2_SMOKE
#include "pe_gpu2_smoke_host.h"
#endif
#ifdef CHK_PUMP
int pe_plat_gpuq_pump(void);
#endif
#ifdef CHK_SEED
uint32_t pe_plat_gpuq_seed_deadline(void);
#endif
static unsigned g2_selected_list(uint32_t *out)
{
    unsigned n = 0;
#ifdef CHK_PUMP
    out[n++] = 0x80076EE4u;
#endif
#ifdef CHK_SEED
    out[n++] = 0x800773D0u;
#endif
    (void)out;
    return n;
}
static int g2_entry(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
#if defined(GPU2_MUTANT) && GPU2_MUTANT == 1
    {
        uint32_t sel[8]; unsigned i, n = g2_selected_list(sel);
        for (i = 0; i < n; i++) if (sel[i] == entry) return 1;
        return -2;
    }
#else
    switch (entry) {
#ifdef CHK_PUMP
    case 0x80076EE4u: *ret = (uint32_t)pe_plat_gpuq_pump(); return 1;
#endif
#ifdef CHK_SEED
    case 0x800773D0u: *ret = pe_plat_gpuq_seed_deadline(); return 1;
#endif
    }
    return -2;
#endif
}
int main(int c, char **v) { return g2_main(c, v); }
