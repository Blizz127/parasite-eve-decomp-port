/* Vector check for batch cdgen49b (pe_cdx_cdgen49b_vectors.py): a DsRead provider (retail 0x80080E34, libcd CdRom_StartRead),
 * linked as a built OBJECT (never read) under a symbol of the author's choice: -DCDX49B_FN=<symbol> (default func_80080E34, the
 * retail name a zz_dispatch / generated caller would use).  Signature checked:
 *     int FN(uint32_t pos, uint32_t sectors, uint32_t dest, uint32_t mode);
 * pos is a guest pointer to the 4 CdlLOC bytes, or 0 (use the last position).  Result compared as a 32-bit value.
 *
 * Stand-ins (positional arguments; the retail callee each stands for; names are the public pe_plat bindings in include/pe_plat headers):
 *   pe_plat_cd_error_active()                       800822AC (0 args)
 *   pe_plat_ds_last_pos(dst)                        80080CDC (1 arg)  returns the scripted pointer
 *   pe_plat_cd_command_sequence_host(a0,a1,a2,a3,a4) 8007F0C8 (5 args + 1: the 32-bit word the second argument points to is read by the
 *                                                    stand-in without logging and compared as an extra field: what the command sequence
 *                                                    would read).  The retail call is (mode|0x20, pointer to the position copy, 6,
 *                                                    0x80080F64, -1): the header's parameter NAMES (mode, location, command, extra,
 *                                                    callback) do not line up with the last two, the comparison is positional.
 *   pe_plat_hostfb_vsync(mode)                      80073A44 (1 arg)
 *   pe_plat_ds_data_callback(cb)                    800824F0 (1 arg)  returns the scripted previous callback
 * Everything else the provider needs from the guest is guest RAM through the PE_Load and PE_Store accessors; reads outside what the retail function read
 * and any write outside what it wrote fail the case (canary seeding of unseeded RAM is the pe_cdx_common.h default).
 *
 * BUILD: cc -std=gnu11 -Wall -Wextra -Werror -DCDX49B_FN=$CDX49B_FN -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen49b_check.c $CDX49B_OBJ -o build/chk/check49b
 * Run: ./check49b dsread.txt [noop|flip|retflip|storeflip|poke0|same]
 */
#include "pe_cdx_common.h"
#ifndef CDX49B_FN
#define CDX49B_FN func_80080E34
#endif
CALLEE(pe_plat_cd_error_active, 0x800822ACu, 0)
uint32_t pe_plat_ds_last_pos(uint32_t dst) { return cdx_callee(0x80080CDCu, 1, dst, 0, 0, 0, 0); }
int pe_plat_cd_command_sequence_host(uint32_t a0, uint32_t a1, int a2, uint32_t a3, uint32_t a4)
{
    uint32_t a[8] = {0}; uint32_t w;
    memcpy(&w, ram + (a1 & (MEM - 1) & ~3u), 4);
    a[0] = a0; a[1] = a1; a[2] = (uint32_t)a2; a[3] = a3; a[4] = a4; a[5] = w;
    return (int)cdx_calleeN(0x8007F0C8u, 6, a);
}
uint32_t pe_plat_hostfb_vsync(int mode) { return cdx_callee(0x80073A44u, 1, (uint32_t)mode, 0, 0, 0, 0); }
uint32_t pe_plat_ds_data_callback(uint32_t cb) { return cdx_callee(0x800824F0u, 1, cb, 0, 0, 0, 0); }
int PE_Port_ShouldStop(void) { return 0; }
int CDX49B_FN(uint32_t pos, uint32_t sectors, uint32_t dest, uint32_t mode);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    if (entry != 0x80080E34u) return -1;
    if (mutant == 1) { *ret = 0; return 1; }
    *ret = (uint32_t)CDX49B_FN(a[0], a[1], a[2], a[3]);
    return 1;
}
