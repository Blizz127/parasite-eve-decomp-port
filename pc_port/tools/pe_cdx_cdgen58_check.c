/* Checker for the cdgen58 set (pe_cdx_cdgen52_61_vectors.py --set cdgen58): libcd issue wrapper 0x8007FCFC(command, data).
 * Scaffold pe_cdx_common_canary.h (canary seeding, store-footprint category, controls noop / flip / retflip / storeflip / poke0 / same).  One provider object per
 * build, linked as a black box (pc_port/platform/pe_plat_cd_issue_wrapper.c of the int3 tree, read-only).  Seams (from `nm -u` of the object; stand-ins here): pe_plat_cd_flush() = 0x8007B9EC, pe_plat_cd_copy4(dst, src) = 0x80080950 (arity 2), pe_plat_cdhost_cmd(cmd, input, dst, mode) = 0x8007B558 (arity 4).
 * Build line (executed by pe_cdx_cdgen52_61_run.sh):
 * BUILD-issue_wrapper: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen58_check.c $O/pe_plat_cd_issue_wrapper.o -o $O/c58_issue_wrapper
 * Run: $O/c58_issue_wrapper <set>.txt [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
CALLEE(pe_plat_cd_flush, 0x8007B9ECu, 0)
CALLEE(pe_plat_cd_copy4, 0x80080950u, 2)
CALLEE(pe_plat_cdhost_cmd, 0x8007B558u, 4)
int pe_plat_cd_issue_wrapper(uint32_t command, uint32_t data);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
    if (entry != 0x8007FCFCu) return -1;
    if (mutant == 1) { *ret = 0; return 1; }
    *ret = (uint32_t)pe_plat_cd_issue_wrapper(a[0], a[1]); return 1;
}
