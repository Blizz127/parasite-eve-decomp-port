/* State60 vector checker, family gpuenv (batch state60d): draw environment / draw mode / draw offset words (pe_plat2_gpuenv.c, clean-room provider state60d).
 * Build (run from the repository root; provider object is linked as a black box):
 *   cc -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -O1 -fno-stack-protector -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/tools pc_port/tools/pe_state60_gpuenv_check.c build/state60/prov/pe_plat2_gpuenv.c.o -Wl,--unresolved-symbols=ignore-all -o build/state60/check_gpuenv
 * Run:  ./build/state60/check_gpuenv <vector files of state60d>
 * Provider object under test: build/state60/prov/pe_plat2_gpuenv.c.o (compiled from pc_port/platform/pe_plat2_gpuenv.c, see docs/evidence/native-state60-20261126/state60d/AUTHOR-REPORT.md).
 * Replay rules are in pe_state60_check_core.h; vectors come from pc_port/tools/pe_state60_vectors.py state60d. */
#define _POSIX_C_SOURCE 200809L
#include "pe_state60_check_core.h"
#include "pe_plat/gpuenv.h"
static unsigned family_arity(uint32_t fn) { (void)fn; return 0; }
static int family_call(uint32_t entry, const uint32_t *a, uint32_t *ret, int *has_ret, unsigned *rw)
{
    *rw = 4; *has_ret = 0;
    switch (entry) {
    case 0x800749D8u: *ret = pe_plat_gpuenv_display_env(a[0], (int)a[1], (int)a[2], (int)a[3], (int)a[4]); *has_ret = 1; return 1;
    case 0x80076150u: *ret = pe_plat_gpuenv_draw_mode((int)a[0], (int)a[1], a[2]); *has_ret = 1; return 1;
    case 0x800762A0u: *ret = (uint32_t)pe_plat_gpuenv_draw_offset((int)a[0], (int)a[1]); *has_ret = 1; return 1;
    case 0x80075C04u: pe_plat_gpuenv_draw_offset_packet(a[0], a[1]); return 1;
    }
    return -1;
}
