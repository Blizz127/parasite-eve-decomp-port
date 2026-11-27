/* Checker for the cdgen64 sets (pe_gpu2_vectors.py cdgen64s / cdgen64i): the ResetGraph light path, libgpu 0x80074A44 (mode & 7 in 1, 2, 4, 6, 7),
 * provider pc_port/platform/pe_plat_resetgraph_light_host.c (pe_plat_resetgraph_light_host(mode)).  The reset modes (mode & 7 = 0, 3, 5) belong to
 * the sibling provider pe_plat_gpu_resetgraph.c and are not in the vectors.  Contract: pe_gpu2_check_core.h.  Controls: GPU2_MUTANT=1 no-op entry,
 * =2 return bit flip, =3 store bit flip (cdx noop / retflip / storeflip), reversed order and the missing-provider link check by the run script.
 * Seams of the provider (nm -u): PE_Bios_Printf (the debug print the original makes through the log vector 0x80071A74), PE_GuestHook_Call / Refuse
 * (the guest-function hooks: the log vector when it is not the printf stub, and the table slot 13 target), pe_plat_gpu_2700 (= guest 0x80077144, the GPU
 * reset the light path jumps to), PE_Port_StopEpoch, PE_RangeIsRam, PE_MMIO_IsModelled (host stubs).  Two builds:
 *   BUILD-stopped (vectors cdgen64s): pe_plat_gpu_2700 is a stopped callee (g2_call 0x80077144): measures the light host alone;
 *   BUILD-inline (vectors cdgen64i): the current provider of the GPU reset, pe_plat_gpu_gen9.o, is linked (its seams: SetIntrMask = host stub, the byte fill
 *       0x80077A28 and the text entry 0x80077548 are stopped callees): the DMA2 CHCR / DPCR / GP1 register writes and the state stores are compared.
 * Refusal arms of the provider (invalid root table, unaligned / out-of-RAM register pointer words) have no counterpart in the original, which just jumps
 * through the words: no faithful vector reaches them.
 * Build lines (executed verbatim by pe_gpu2_cdgen63_64_run.sh; $O holds pe_plat_resetgraph_light_host.o and pe_plat_gpu_gen9.o):
 * BUILD-stopped: cc -std=gnu11 -Wall -Wextra -Werror -DC64_STOPPED=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_cdgen64_check.c $O/pe_plat_resetgraph_light_host.o -o $O/c64_stopped
 * BUILD-inline: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_cdgen64_check.c $O/pe_plat_resetgraph_light_host.o $O/pe_plat_gpu_gen9.o -o $O/c64_inline
 * Run: $O/c64_stopped <cdgen64s vectors>;  $O/c64_inline <cdgen64i vectors>  */
#include "pe_gpu2_check_core.h"
#include "pe_guest_ram.h"
#include "pe_gpu2_host_stubs.h"
#include "pe_bios_string.h"

int pe_plat_resetgraph_light_host(int mode);
int32_t PE_Bios_Printf(pe_addr_t fmt, const uint32_t *args, int nargs)
{ return (int32_t)g2_call(0x80071A74u, fmt, nargs > 0 ? args[0] : 0, nargs > 1 ? args[1] : 0, nargs > 2 ? args[2] : 0); }
#if C64_STOPPED
int pe_plat_gpu_2700(int mode) { return (int)g2_call(0x80077144u, (uint32_t)mode, 0, 0, 0); }
#else
void pe_plat_fill_bytes(uint32_t dst, uint8_t value, int32_t count) { (void)g2_call(0x80077A28u, dst, value, (uint32_t)count, 0); }
int pe_plat_gpu_2b04(int a0) { return (int)g2_call(0x80077548u, (uint32_t)a0, 0, 0, 0); }
#endif
static unsigned g2_selected_list(uint32_t *out) { out[0] = 0x80074A44u; return 1; }
static int g2_entry(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
#if defined(GPU2_MUTANT) && GPU2_MUTANT == 1
    { (void)a; (void)ret; return entry == 0x80074A44u ? 1 : -2; }
#else
    if (entry != 0x80074A44u) return -2;
    *ret = (uint32_t)pe_plat_resetgraph_light_host((int)a[0]);
    return 1;
#endif
}
int main(int c, char **v) { return g2_main(c, v); }
