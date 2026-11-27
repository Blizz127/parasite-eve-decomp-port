/* Vector check for batch cdgen42c / cdgen42ci (pe_cdx_vectors.py cdgen42c | cdgen42ci): the three CD callback-slot targets
 * that have no native in the legacy port:
 *   0x8007F7E8 libds VBlank hook (pump slot 0x800A36A0; no arguments)       -> CDX42C_VB   (void f(void))
 *   0x8007F88C libds status callback (progress slot 0x800A36A8)             -> CDX42C_SCB  (void f(int status, uint32_t response))
 *   0x80080778 default CD ready callback (installed into D_8009AFB8)        -> CDX42C_RDY  (void f(int status, uint32_t response))
 * The provider symbol of each entry is a macro (default: the canonical guest name func_XXXXXXXX that the provider header
 * binds with `#define func_X pe_plat_..`), overridable with -DCDX42C_VB=<sym> etc.  Scaffold: pe_cdx_common.h.
 * Stopped callees are recorded by weak stand-ins (8007FBF0 lane getter, 8007FB44 issue, 8007E704 text_170, 8008080C decode)
 * and by PE_GuestCall / PE_GuestHook_Call / a pooled PE_GuestCode_Resolve (every callback slot target: sentinels
 * 0x80131300/40/80 and the real 0x8007F88C; arguments (status byte, response)).  A provider that calls a stand-in symbol it
 * defines itself (same translation unit) bypasses the weak stand-in: use the -DCDX42C_WRAP build and `--wrap` flags
 * (the checker then defines __wrap_<sym> recorders for the symbols named by CDX42C_LANE / _ISSUE / _TEXT170 / _DECODE).
 * -DCDX42C_RET compares the return register (vector `RET`) too: the three provider functions are then declared `uint32_t f(..)`
 * (default: void, return not compared; PE_GuestHook_Call's `*ret` is its $v0, which the pump slot site ignores).
 * -DCDX42C_STUB links no-op providers (checker self-test: the do-nothing provider passes only the cases in which the original
 * has no observable effect; `flip` corrupts the last guest write).
 * Run: ./check42c vectors.txt [noop|flip]; CDX_CATEGORIES=1 / CDX_MAXPRINT=n / CDX_LOOSE_READS=1 / CDX_DEBUG=1 as check42b.
 * BUILD_STUB:     cc -std=gnu11 -Wall -Wextra -Werror -DCDX42C_STUB -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen42c_check.c -o check42c_stub
 * BUILD:          cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen42c_check.c <provider object(s)> -o check42c
 * BUILD_WRAP:     cc -std=gnu11 -Wall -Wextra -Werror -DCDX42C_WRAP -DCDX42C_LANE=<sym> -DCDX42C_ISSUE=<sym> -DCDX42C_TEXT170=<sym> -DCDX42C_DECODE=<sym> ... <provider object(s)> -Wl,--wrap=<sym> ... -o check42c_w
 */
#include "pe_cdx_common.h"
#include "pe_guestcode.h"
#define CALLEEW(sym, fn, n) \
    __attribute__((weak)) uint32_t sym(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e) \
    { return cdx_callee(fn, n, (uint32_t)a, (uint32_t)b, (uint32_t)c, (uint32_t)d, (uint32_t)e); }
uint8_t PE_CdLoadU8(pe_addr_t a) { return PE_LoadU8(a); }
void PE_CdStoreU8(pe_addr_t a, uint8_t v) { PE_StoreU8(a, v); }
void PE_CdStoreU32(pe_addr_t a, uint32_t v) { PE_StoreU32(a, v); }
IGNORE(PE_StrFeed_Close)
int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
#ifndef CDX42C_LANE
#define CDX42C_LANE pe_plat_ds_status_lane
#define CDX42C_ISSUE pe_plat_cd_try_issue_cmd
#define CDX42C_TEXT170 pe_plat_ds_text_170
#define CDX42C_DECODE pe_plat_cd_decode_status
#endif
#ifdef CDX42C_WRAP
#define WRAPNAME2(x) __wrap_##x
#define WRAPNAME(x) WRAPNAME2(x)
static uint32_t wr_lane(uintptr_t a) { return cdx_callee(0x8007FBF0u, 1, (uint32_t)a, 0, 0, 0, 0); }
int WRAPNAME(CDX42C_LANE)(int a) { return (int)wr_lane((uintptr_t)a); }
int WRAPNAME(CDX42C_ISSUE)(uint32_t a, uint32_t b) { return (int)cdx_callee(0x8007FB44u, 2, a, b, 0, 0, 0); }
void WRAPNAME(CDX42C_TEXT170)(uint32_t a, uint32_t b) { (void)cdx_callee(0x8007E704u, 2, a, b, 0, 0, 0); }
void WRAPNAME(CDX42C_DECODE)(int a, uint32_t b) { (void)cdx_callee(0x8008080Cu, 2, (uint32_t)a, b, 0, 0, 0); }
#else
CALLEEW(CDX42C_LANE, 0x8007FBF0u, 1)
CALLEEW(CDX42C_ISSUE, 0x8007FB44u, 2)
CALLEEW(CDX42C_TEXT170, 0x8007E704u, 2)
CALLEEW(CDX42C_DECODE, 0x8008080Cu, 2)
#endif
static uint32_t cur_entry;
/* arity of a callback call as the vectors record it: the pump reaches its slot / helper targets with leftover registers
 * (none compared; 0x8007FCFC takes two), the progress handler passes (status, response). */
static unsigned cb_arity(uint32_t fn) { (void)cur_entry; return (fn == 0x8007FBF0u) ? 1u : 2u; }  /* every recorded call of this batch: lane getter 1, the rest 2 */
/* ICall labels (the `site` / `slot` / `caller` strings): retail has no labels, so vectors cannot fix them and the checker does not
 * compare them; CDX_LABELS=1 prints each distinct (label, target) the provider passes once, for the unit-test strcmp contract. */
static void label_note(const char *kind, const char *a, const char *b, uint32_t fn)
{
    static char seen[64][200]; static unsigned n; char l[200]; unsigned i;
    if (!getenv("CDX_LABELS")) return;
    snprintf(l, sizeof l, "%s slot/site=%s caller=%s fn=%08X", kind, a ? a : "(null)", b ? b : "(null)", fn);
    for (i = 0; i < n; i++) if (!strcmp(seen[i], l)) return;
    if (n < 64) { strcpy(seen[n++], l); fprintf(stderr, "LABEL %s\n", l); }
}
int PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    (void)arity; label_note("call", site, NULL, fn);
    return (int)cdx_callee(fn, cb_arity(fn), (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, 0);
}
int PE_GuestHook_Call(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1,
                      uintptr_t a2, uintptr_t a3, int *ret)
{
    uint32_t r = cdx_callee(fn, cb_arity(fn), (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, 0);
    (void)arity; label_note("hook", slot, caller, fn);
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

#ifndef CDX42C_VB
#define CDX42C_VB func_8007F7E8
#define CDX42C_SCB func_8007F88C
#define CDX42C_RDY func_80080778
#endif
#ifdef CDX42C_RET
#define CDX42C_RT uint32_t
#else
#define CDX42C_RT void
#endif
#ifdef CDX42C_STUB
CDX42C_RT CDX42C_VB(void) { return (CDX42C_RT)0; }
CDX42C_RT CDX42C_SCB(int s, uint32_t r) { (void)s; (void)r; return (CDX42C_RT)0; }
CDX42C_RT CDX42C_RDY(int s, uint32_t r) { (void)s; (void)r; return (CDX42C_RT)0; }
#else
CDX42C_RT CDX42C_VB(void); CDX42C_RT CDX42C_SCB(int, uint32_t); CDX42C_RT CDX42C_RDY(int, uint32_t);
#endif
#ifdef CDX42C_RET
#define INVOKE(expr) do { *ret = (uint32_t)(expr); return 1; } while (0)
#else
#define INVOKE(expr) do { (void)(expr); return 0; } while (0)
#endif
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)ret;
    cur_entry = entry;
    if (mutant == 1) return 0;
    switch (entry) {
    case 0x8007F7E8u: INVOKE(CDX42C_VB());
    case 0x8007F88Cu: INVOKE(CDX42C_SCB((int)a[0], a[1]));
    case 0x80080778u: INVOKE(CDX42C_RDY((int)a[0], a[1]));
    }
    return -1;
}
