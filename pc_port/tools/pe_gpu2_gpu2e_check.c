/* sw-vec-gpu2 batch gpu2e checker (added by the gpu2c author): the environment packet builder func_80075EE0, the callee
 * of PutDrawEnv / DrawOTagEnv.  Vector file: gpu2-vectors/gpu2e/gpu2e.txt (generator: pe_gpu2_vectors.py OUT func_80075EE0).
 * Group CHK_SETENV: 80075EE0 against pe_plat2_gpuprim_setenv.c; every callee (80076170, 80076208, 800762A0, 80076150,
 * 800762BC) is reached by guest address and is a checker callee.
 * Build line (repo root, verbatim):
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_SETENV -DG2_BATCH_NAME=\"gpu2e-setenv\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2e_check.c pc_port/platform/pe_plat2_gpuprim_setenv.c -o check_gpu2e_setenv
 *   ./check_gpu2e_setenv vectors/gpu2e/gpu2e.txt
 * Mutants: -DGPU2_MUTANT=1 (no-op stub), =2 (return bit flip), =3 (store bit flip). */
#include "pe_gpu2_check_core.h"
#include "pe_guest_ram.h"
#include "pe_gpu2_host_stubs.h"
int pe_plat_gpuprim_set_draw_env(uint32_t packet, uint32_t env);
static unsigned g2_selected_list(uint32_t *out)
{
    unsigned n = 0;
#ifdef CHK_SETENV
    out[n++] = 0x80075EE0u;
#endif
    (void)out;
    return n;
}
static int g2_entry(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
#if defined(GPU2_MUTANT) && GPU2_MUTANT == 1
    {
        uint32_t sel[32]; unsigned i, n = g2_selected_list(sel);
        for (i = 0; i < n; i++) if (sel[i] == entry) return 1;
        return -2;
    }
#else
    switch (entry) {
#ifdef CHK_SETENV
    case 0x80075EE0u: *ret = (uint32_t)pe_plat_gpuprim_set_draw_env(a[0], a[1]); return 1;
#endif
    }
    return -2;
#endif
}
int main(int c, char **v) { return g2_main(c, v); }
