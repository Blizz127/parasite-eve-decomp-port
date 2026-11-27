/* State60 vector checker, batch state60a rows 14-26 (clean-room provider pe_plat2_spu_b.c).
 * Same replay core and host recorders as pe_state60_spu_check.c, restricted to the 13 functions of
 * func_80085B34 80085BB4 80085C44 80085D84 80085DC4 80085EB4 80085F14 80085F44 80085F74 800862F4
 * 80089EB8 80089F08 80089F28 and linked against the new provider object only.
 * Build (run from the repository root; provider object is linked as a black box):
 *   cc -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -O1 -fno-stack-protector -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/tools pc_port/tools/pe_state60_spub_check.c build/state60/prov/pe_plat_spu_b.c.o -Wl,--unresolved-symbols=ignore-all -o build/state60/check_spub
 * Provider object: cc -std=c99 -O2 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -c pc_port/platform/pe_plat2_spu_b.c -o build/state60/prov/pe_plat_spu_b.c.o
 * Run:  ./build/state60/check_spub <vector files>   (pe_state60_run_spub.py runs the 13 files and the mutants) */
#define _POSIX_C_SOURCE 200809L
#include "pe_state60_check_core.h"
#include "pe_plat/spu_b.h"

uint32_t pe_plat_interrupt_callback(uint32_t s, uint32_t h) { return state60_note_call(0x80073CC4u, s, h, 0, 0); }
static unsigned family_arity(uint32_t fn)
{
    switch (fn) {
    case 0x80073CC4u: return 2;
    }
    return 0;
}
/* Host services the provider reaches that the original never called: any use is reported as an
 * unexpected host call (the case then fails, with the reason under STATE60_VERBOSE=1). */
void pe_plat_sio_poll_tick(void) { host_calls_unexpected++; }
void PE_Port_RequestStop(int reason) { (void)reason; host_calls_unexpected++; }
int PE_Port_ShouldStop(void) { return 0; }
void __stack_chk_fail(void) { abort(); }
static int family_call(uint32_t entry, const uint32_t *a, uint32_t *ret, int *has_ret, unsigned *rw)
{
    *rw = 4; *has_ret = 0;
    switch (entry) {
    case 0x80085B34u: *ret = (uint32_t)func_80085B34(a[0]); *has_ret = 1; return 1;
    case 0x80085BB4u: *ret = (uint32_t)func_80085BB4(a[0]); *has_ret = 1; return 1;
    case 0x80085C44u: *ret = (uint32_t)func_80085C44((int)a[0]); *has_ret = 1; return 1;
    case 0x80085D84u: *ret = (uint32_t)func_80085D84((int)a[0]); *has_ret = 1; return 1;
    case 0x80085DC4u: func_80085DC4((int)a[0]); return 1;
    case 0x80085EB4u: *ret = func_80085EB4(a[0]); *has_ret = 1; return 1;
    case 0x80085F14u: *ret = (uint32_t)func_80085F14((int)a[0]); *has_ret = 1; return 1;
    case 0x80085F44u: func_80085F44((int)a[0]); return 1;
    case 0x80085F74u: func_80085F74(a[0]); return 1;
    case 0x800862F4u: func_800862F4((int)a[0], (uint16_t)a[1], (uint16_t)a[2], (int16_t)a[3], (int16_t)a[4]); return 1;
    case 0x80089EB8u: *ret = (uint32_t)func_80089EB8((int)a[0]); *has_ret = 1; return 1;
    case 0x80089F08u: func_80089F08(a[0], a[1]); return 1;
    case 0x80089F28u: func_80089F28((uint16_t)a[0], (uint16_t)a[1]); return 1;
    }
    return -1;
}
