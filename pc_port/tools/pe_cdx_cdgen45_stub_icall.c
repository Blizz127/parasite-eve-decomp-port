/* Link stand-in for the cdgen45 checker against the clean-room provider
 * pc_port/platform/pe_plat2_cdx_save.c.  It also stands in for the two kernel
 * interrupt-chain services the provider now calls (pe_plat_bios_sys_deq_int_rp /
 * pe_plat_bios_sys_enq_int_rp, retail C(03h) 0x8007E1F4 / C(02h) 0x8007E1E4): the
 * oracle silenced those calls (bios_silent C(02h)/C(03h)), so the stand-ins keep
 * a private call record, outside the checker's logs, and the checker prints
 * the record at exit; each of the 150 cdgen45 start vectors must have made the
 * two calls once, dequeue first, with (2, 0x800A5AB0).  The provider's installed-hook-slot tail of
 * 0x80082534 calls PE_Card_ICall; the cdgen45 vectors never reach it (every vector
 * seeds the slot empty), so reaching it here is a failure.  Deliberately does not
 * include pe_cdx_common.h (that header defines main and the PE_* accessors).
 * Build line (deviation from pe_cdx_run_all.sh: provider object, the two renames,
 * this stand-in):
 *   cc -std=gnu11 -Wall -Wextra -Werror -Dpe_plat_pad_initialize_host=pe_plat_cdxsave_pad_comm_start \
 *      -Dpe_plat_pad_records_initialize_host=pe_plat_cdxsave_pad_init_direct -Ipc_port/include \
 *      -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen45_check.c \
 *      pc_port/tools/pe_cdx_cdgen45_stub_icall.c <build>/CMakeFiles/pe_field_runtime.dir/platform/pe_plat2_cdx_save.c.o -o check45 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
static unsigned g_calls, g_bad, g_order[2];
int pe_plat_bios_sys_deq_int_rp(uint32_t priority, uint32_t record);
int pe_plat_bios_sys_enq_int_rp(uint32_t priority, uint32_t record);
static void record_call(unsigned which, uint32_t priority, uint32_t record)
{
    /* which: 0 = dequeue (C(03h)), 1 = enqueue (C(02h)); dequeue must come first of each pair */
    if (priority != 2u || record != 0x800A5AB0u || (which == 0u) != (g_calls % 2u == 0u)) g_bad++;
    g_order[which]++;
    g_calls++;
}
int pe_plat_bios_sys_deq_int_rp(uint32_t priority, uint32_t record) { record_call(0u, priority, record); return 0; }
int pe_plat_bios_sys_enq_int_rp(uint32_t priority, uint32_t record) { record_call(1u, priority, record); return 0; }
static void report_calls(void)
{
    fprintf(stderr, "cdgen45 stand-in: interrupt-chain calls dequeue=%u enqueue=%u bad=%u\n", g_order[0], g_order[1], g_bad);
}
__attribute__((constructor)) static void arm_report(void) { atexit(report_calls); }
uint32_t PE_Card_ICall(const char *slot, uint32_t fn, unsigned arity, uint32_t a0, uint32_t a1, uint32_t a2);
uint32_t PE_Card_ICall(const char *slot, uint32_t fn, unsigned arity, uint32_t a0, uint32_t a1, uint32_t a2)
{
    (void)fn; (void)arity; (void)a0; (void)a1; (void)a2;
    fprintf(stderr, "cdgen45 stand-in: PE_Card_ICall(%s) reached, the vectors never install a slot\n", slot);
    abort();
}
