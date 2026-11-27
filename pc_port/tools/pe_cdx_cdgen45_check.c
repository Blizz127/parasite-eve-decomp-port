/* Vector check for batch cdgen45 (pe_cdx_vectors.py cdgen45): the two retail entries bound in platform/pe_save.c,
 * 0x80082534 (wrapper around the libpad communication start 0x80082CF0) and 0x800844E4 (pad record initialisation).
 * Audit conflict, not resolved here: the provider's header calls them game logic over guest RAM, the project
 * classification TSV labels them libpad PadStartCom / PadInitDirect.  Provider linked as a built object, never
 * read.  The BIOS C(02h) / C(03h) interrupt-chain services the retail path reaches are not observable on the
 * native (no host symbol) and are dropped in the vectors; the libcard hook slots 0x8009B724.. are empty.
 * Scaffold: pe_cdx_common.h.
 * Build (object: ninja -C build/vec CMakeFiles/pe_field_runtime.dir/platform/pe_save.c.o):
 * BUILD: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen45_check.c build/vec/CMakeFiles/pe_field_runtime.dir/platform/pe_save.c.o -o check45
 * Run: ./check45 vectors.txt [noop|flip]
 */
#include "pe_cdx_common.h"
CALLEE(pe_plat_bios_enter_critical, 0x80072714u, 0)
CALLEE(pe_plat_bios_exit_critical, 0x80072724u, 0)
CALLEE(pe_plat_bios_change_clear_rcnt, 0x80073C84u, 2)
void pe_plat_pad_initialize_host(void); void pe_plat_pad_records_initialize_host(uint32_t, uint32_t);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)ret;
    if (mutant == 1) return entry == 0x80082534u || entry == 0x800844E4u ? 0 : -1;
    switch (entry) {
    case 0x80082534u: pe_plat_pad_initialize_host(); return 0;
    case 0x800844E4u: pe_plat_pad_records_initialize_host(a[0], a[1]); return 0;
    }
    return -1;
}
