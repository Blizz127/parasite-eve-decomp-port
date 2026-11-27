/* State60 vector checker, family ctl (batch state60b): RCNT counters, GPU control/query words, pad-init flag, video mode, callback state (pe_plat2_ctl.c, clean-room provider).
 * Build (run from the repository root; provider object is linked as a black box):
 *   cc -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -O1 -fno-stack-protector -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/tools pc_port/tools/pe_state60_ctl_check.c build/state60/prov/pe_plat2_ctl.c.o -Wl,--unresolved-symbols=ignore-all -o build/state60/check_ctl
 * Run:  ./build/state60/check_ctl <vector files of state60b>
 * Provider object under test: build/state60/prov/pe_plat2_ctl.c.o (the pe_plat2_ctl.c object of the build tree; hash in the author report).
 * Replay rules are in pe_state60_check_core.h; vectors come from pc_port/tools/pe_state60_vectors.py state60b. */
#define _POSIX_C_SOURCE 200809L
#include "pe_state60_check_core.h"
#include "pe_plat/ctl.h"

/* The ctl leaves have no callees (the vector CALL columns are all "-"), so no callee recorders are
 * defined: any call the provider made would be an unresolved symbol (-Wl,--unresolved-symbols=ignore-all). */
static unsigned family_arity(uint32_t fn) { (void)fn; return 0; }
/* Host services the provider reaches that the original never called: any use is reported as an
 * unexpected host call (the case then fails, with the reason under STATE60_VERBOSE=1). */
void PE_Decomp_GuestTrapDivZero(const char *fn) { (void)fn; host_calls_unexpected++; }
void PE_Decomp_NoteStopDetail(const char *t) { (void)t; host_calls_unexpected++; }
void PE_Port_RequestStop(int reason) { (void)reason; host_calls_unexpected++; }
int PE_Port_ShouldStop(void) { return 0; }
bool PE_RangeIsRam(pe_addr_t a, size_t n) { return (a & 0x1FFFFFFFu) < MEM && (a & 0x1FFFFFFFu) + n <= MEM; }
pe_addr_t PE_RamCanonical(pe_addr_t a, size_t n) { (void)n; return 0x80000000u | (a & (MEM - 1)); }
int PE_SpuDma_Begin(uint32_t a, uint32_t b, uint32_t c) { (void)a; (void)b; (void)c; host_calls_unexpected++; return 0; }
int PE_SpuDma_BeginRead(uint32_t a, uint32_t b, uint32_t c) { (void)a; (void)b; (void)c; host_calls_unexpected++; return 0; }
int PE_SpuDma_GetState(void) { host_calls_unexpected++; return 0; }
uint32_t PE_Spu_TransferCursor(void) { host_calls_unexpected++; return 0; }
uint16_t PE_SpuRegister_LoadU16(uint32_t off)
{ uint32_t v; if (mmio_read(0x1F801C00u + off, 2, &v)) return (uint16_t)v; return 0; }
void __stack_chk_fail(void) { abort(); }
static int family_call(uint32_t entry, const uint32_t *a, uint32_t *ret, int *has_ret, unsigned *rw)
{
    *rw = 4; *has_ret = 0;
    switch (entry) {
    case 0x80076B58u: *ret = (uint32_t)func_80076B58(a[0], (int)a[1]); *has_ret = 1; return 1;
    case 0x80076BE0u: *ret = func_80076BE0(a[0]); *has_ret = 1; return 1;
    case 0x8007633Cu: *ret = (uint32_t)func_8007633C(); *has_ret = 1; return 1;
    case 0x80076B20u: func_80076B20(a[0]); return 1;
    case 0x80076B44u: *ret = func_80076B44((int)a[0]); *rw = 1; *has_ret = 1; return 1;
    case 0x80073DF8u: *ret = func_80073DF8(); *rw = 2; *has_ret = 1; return 1;
    case 0x80085814u: *ret = (uint32_t)func_80085814((unsigned short)a[0], (short)a[1], (int)a[2]); *has_ret = 1; return 1;
    case 0x800858E8u: *ret = (uint32_t)func_800858E8(a[0]); *has_ret = 1; return 1;
    case 0x80085918u: *ret = (uint32_t)func_80085918(a[0]); *has_ret = 1; return 1;
    case 0x800858B0u: *ret = (uint32_t)func_800858B0((int)a[0]); *has_ret = 1; return 1;
    case 0x8008594Cu: *ret = (uint32_t)func_8008594C((int)a[0]); *has_ret = 1; return 1;
    case 0x80074CB8u: *ret = func_80074CB8(); *rw = 1; *has_ret = 1; return 1;
    case 0x8007DEA4u: func_8007DEA4(a[0]); return 1;
    case 0x80074A14u: *ret = func_80074A14(a[0]); *has_ret = 1; return 1;
    case 0x80073DE8u: *ret = func_80073DE8(); *rw = 2; *has_ret = 1; return 1;
    case 0x80074A28u: *ret = (uint32_t)func_80074A28(); *has_ret = 1; return 1;
    case 0x8007DEB0u: *ret = (uint32_t)func_8007DEB0(); *has_ret = 1; return 1;
    }
    return -1;
}
