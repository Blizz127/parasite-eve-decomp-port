/* Vector check for batch cdgen42b (pe_cdx_vectors.py cdgen42b): the CD-stream pump 0x8007FE24
 * (pe_plat_cd_status_pump_host), the command-progress handler 0x80080404 (pe_plat_cd_command_progress) and the
 * status decoder 0x8008080C (pe_plat_cd_decode_status), sampled in runtime-observed and product states.
 * Scaffold: pe_cdx_common.h (contract).  The provider is linked as a built object, never read; every symbol it
 * leaves undefined is stubbed below.  New relative to check42: the stand-ins are weak, a guest-code call
 * (PE_GuestCall / PE_GuestHook_Call: the retail `jalr` through the callback slots 0x800A36A0..36AC) is RECORDED as a
 * callee call (arguments compared only where the vectors declare them: the progress handler passes (status, response),
 * the pump passes none), and the optional -DCDX42B_WRAP mode records pe_plat_cd_command_progress / pe_plat_cd_decode_status
 * when a provider calls them from another object (add the --wrap flags of the second BUILD line).
 * Run: ./check42b vectors.txt [noop|flip]; CDX_CATEGORIES=1 prints failure categories, CDX_MAXPRINT=n the first n mismatches.
 * BUILD:  cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen42b_check.c build/vec/CMakeFiles/pe_field_runtime.dir/game/boot/cd_stream_port.c.o -o check42b
 * BUILD_WRAP: cc -std=gnu11 -Wall -Wextra -Werror -DCDX42B_WRAP -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen42b_check.c <provider objects> -Wl,--wrap=pe_plat_cd_command_progress -Wl,--wrap=pe_plat_cd_decode_status -o check42b_w
 */
#include "pe_cdx_common.h"
#include "pe_guestcode.h"
#define CALLEEW(sym, fn, n) \
    __attribute__((weak)) uint32_t sym(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e) \
    { return cdx_callee(fn, n, (uint32_t)a, (uint32_t)b, (uint32_t)c, (uint32_t)d, (uint32_t)e); }
uint8_t PE_CdLoadU8(pe_addr_t a) { return PE_LoadU8(a); }
void PE_CdStoreU8(pe_addr_t a, uint8_t v) { PE_StoreU8(a, v); }
void PE_CdStoreU32(pe_addr_t a, uint32_t v) { PE_StoreU32(a, v); }
/* retail callees (recorded; arguments as the retail callee takes them) */
CALLEEW(pe_plat_bios_enter_critical, 0x80072714u, 0)
CALLEEW(pe_plat_bios_exit_critical, 0x80072724u, 0)
CALLEEW(pe_plat_cd_command_issue_host, 0x8007B558u, 4)
CALLEEW(pe_plat_cd_ds_set_ready_callback, 0x800824C8u, 1)
CALLEEW(pe_plat_cd_flush, 0x8007B9ECu, 0)
CALLEEW(pe_plat_cd_int_to_pos, 0x8007A930u, 2)
CALLEEW(pe_plat_cd_issue_wrapper, 0x8007FCFCu, 2)
CALLEEW(pe_plat_cd_queue_len, 0x8007F778u, 0)
CALLEEW(pe_plat_cd_retry_command, 0x800800F4u, 0)
CALLEEW(pe_plat_cd_set_ready_callback, 0x8007A4BCu, 1)
CALLEEW(pe_plat_cd_status_callback, 0x80080164u, 2)
CALLEEW(pe_plat_cd_status_poll_host, 0x8007B010u, 2)
CALLEEW(pe_plat_cd_stream_handler_host, 0x8007C564u, 0)
CALLEEW(pe_plat_cd_text_13cc, 0x8007F960u, 2)
CALLEEW(pe_plat_cd_try_issue_cmd, 0x8007FB44u, 2)
CALLEEW(pe_plat_dma_callback, 0x80073CF4u, 2)
CALLEEW(pe_plat_ds_data_callback, 0x800824F0u, 1)
CALLEEW(pe_plat_ds_status_lane, 0x8007FBF0u, 1)
CALLEEW(pe_plat_ds_system_status, 0x8007F72Cu, 0)
CALLEEW(pe_plat_ds_text_170, 0x8007E704u, 2)
CALLEEW(pe_plat_ds_text_3d0, 0x8007E964u, 2)
CALLEEW(pe_plat_interrupt_callback, 0x80073CC4u, 2)
CALLEEW(pe_plat_reset_callback, 0x80073C94u, 0)
CALLEEW(pe_plat_vsync_callbacks, 0x80073D58u, 2)
/* host-only symbols: no retail counterpart */
TRAP(Bootstrap_ReturnVoid) TRAP(Bootstrap_ReturnVoid4Indirect) TRAP(PE_Callback_Bind)
TRAP(PE_CdReg_DetachDevice) TRAP(PE_CdReg_EnableDevice) 
IGNORE(PE_StrFeed_Close)   /* host stream-feed close: no retail counterpart, not compared */
uint32_t PE_CdReg_DeviceEnabled(void) { return 1; }   /* the retail device is always present */
int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
static uint32_t cur_entry;
/* arity of a callback call as the vectors record it: the pump reaches its slot / helper targets with leftover registers
 * (none compared; 0x8007FCFC takes two), the progress handler passes (status, response). */
static unsigned cb_arity(uint32_t fn) { return cur_entry == 0x8007FE24u ? (fn == 0x8007FCFCu ? 2u : 0u) : 2u; }
int PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    (void)site; (void)arity;
    return (int)cdx_callee(fn, cb_arity(fn), (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, 0);
}
int PE_GuestHook_Call(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1,
                      uintptr_t a2, uintptr_t a3, int *ret)
{
    uint32_t r = cdx_callee(fn, cb_arity(fn), (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, 0);
    (void)slot; (void)caller; (void)arity;
    if (ret) *ret = (int)r;
    return 1;
}
/* PE_GuestCode_Resolve stand-in: a provider that resolves a callback address to a host thunk (the legacy binding does)
 * gets a thunk that RECORDS the call as a callee call of that address; the pool is keyed by address. */
static uint32_t pool_fn[32]; static unsigned pool_n;
static PeGuestCodeEntry pool_entry[32];
#define POOLTHUNK(i) static int pool_thunk_##i(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d) \
    { return (int)cdx_callee(pool_fn[i], cb_arity(pool_fn[i]), (uint32_t)a, (uint32_t)b, (uint32_t)c, (uint32_t)d, 0); }
POOLTHUNK(0) POOLTHUNK(1) POOLTHUNK(2) POOLTHUNK(3) POOLTHUNK(4) POOLTHUNK(5) POOLTHUNK(6) POOLTHUNK(7)
POOLTHUNK(8) POOLTHUNK(9) POOLTHUNK(10) POOLTHUNK(11) POOLTHUNK(12) POOLTHUNK(13) POOLTHUNK(14) POOLTHUNK(15)
static const PeGuestThunk pool_thunk[16] = { pool_thunk_0, pool_thunk_1, pool_thunk_2, pool_thunk_3, pool_thunk_4, pool_thunk_5,
    pool_thunk_6, pool_thunk_7, pool_thunk_8, pool_thunk_9, pool_thunk_10, pool_thunk_11, pool_thunk_12, pool_thunk_13,
    pool_thunk_14, pool_thunk_15 };
const PeGuestCodeEntry *PE_GuestCode_Resolve(pe_addr_t addr)
{
    unsigned i;
    for (i = 0; i < pool_n; i++) if (pool_fn[i] == addr) return &pool_entry[i];
    if (pool_n >= 16) { bad_call = 1; trap_name = "PE_GuestCode_Resolve(pool)"; return NULL; }
    pool_fn[pool_n] = addr; pool_entry[pool_n].vma = addr; pool_entry[pool_n].thunk = pool_thunk[pool_n];
    pool_entry[pool_n].symbol = "cdx-standin";
    return &pool_entry[pool_n++];
}
void pe_plat_cd_status_pump_host(void);
#ifdef CDX42B_WRAP
void __real_pe_plat_cd_command_progress(unsigned char, int); void __real_pe_plat_cd_decode_status(int, uint32_t);
void __wrap_pe_plat_cd_command_progress(unsigned char s, int r) { (void)cdx_callee(0x80080404u, cb_arity(0x80080404u), s, (uint32_t)r, 0, 0, 0); }
void __wrap_pe_plat_cd_decode_status(int s, uint32_t r) { (void)cdx_callee(0x8008080Cu, cb_arity(0x8008080Cu), (uint32_t)s, r, 0, 0, 0); }
#define PROGRESS __real_pe_plat_cd_command_progress
#define DECODE __real_pe_plat_cd_decode_status
#else
void pe_plat_cd_command_progress(unsigned char, int); void pe_plat_cd_decode_status(int, uint32_t);
#define PROGRESS pe_plat_cd_command_progress
#define DECODE pe_plat_cd_decode_status
#endif
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)ret;
    cur_entry = entry;
    if (mutant == 1) return 0;
    switch (entry) {
    case 0x8007FE24u: pe_plat_cd_status_pump_host(); return 0;
    case 0x80080404u: PROGRESS((unsigned char)a[0], (int)a[1]); return 0;
    case 0x8008080Cu: DECODE((int)a[0], a[1]); return 0;
    }
    return -1;
}
