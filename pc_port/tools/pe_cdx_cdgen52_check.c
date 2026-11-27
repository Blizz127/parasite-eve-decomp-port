/* Checker for the cdgen52 sets (pe_cdx_cdgen52_61_vectors.py --set cdgen52): libcd CD DMA data sync, retail 0x8007BDDC and its forwarding ABIs 0x8007A910 and
 * 0x80080B24 (three retail entries, ONE bound symbol pe_plat_cd_data_sync(mode)).  Scaffold pe_cdx_common_canary.h (canary seeding, store-footprint category,
 * controls noop / flip / retflip / storeflip / poke0 / same).  One provider object per build, linked as a black box (pc_port/platform/pe_plat_cd_sync.c of the int3
 * tree, read-only).  Seams (from `nm -u` of the object): HostFB_VSync(mode) = retail VSync 0x80073A44 (arity 1, scripted; the clock the original asks twice
 * and then once per pass), PE_GuestCall(site, fn, arity, ..) = the retail callee `fn` (the puts 0x80073C5C), PE_Printf_Guest = retail printf 0x80071A74 (format
 * pointer + the conversions the format consumes, up to 3), PE_CdLoadU32 = the register read of the DMA3 channel control word (replayed in order).  The CD flush
 * 0x8007B9EC is a stopped callee of the original with NO bound symbol in the provider.
 * Build line (executed by pe_cdx_cdgen52_61_run.sh):
 * BUILD-sync: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen52_check.c $O/pe_plat_cd_sync.o -o $O/c52_sync
 * Run: $O/c52_sync <set>.txt [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
uint32_t PE_CdLoadU32(pe_addr_t a) { return PE_LoadU32(a); }
uint32_t HostFB_VSync(int mode) { return cdx_callee(0x80073A44u, 1, (uint32_t)mode, 0, 0, 0, 0); }
int PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity, uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d)
{ (void)site; return (int)cdx_callee(fn, arity, (uint32_t)a, (uint32_t)b, (uint32_t)c, (uint32_t)d, 0); }
static unsigned fmt_args(pe_addr_t fmt)
{
    const uint8_t *p = ram + (fmt & (MEM - 1)); unsigned n = 0;
    while (*p) {
        if (*p == '%') {
            p++;
            if (*p == '%') { p++; continue; }
            while (*p && strchr("-+ #0123456789.hlL", *p)) p++;
            if (!*p) break;
            n++;
        }
        p++;
    }
    return n > 3 ? 3 : n;
}
int32_t PE_Printf_Guest(pe_addr_t fmt, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7)
{ (void)a4; (void)a5; (void)a6; (void)a7; return (int32_t)cdx_callee(0x80071A74u, 1 + fmt_args(fmt), fmt, a1, a2, a3, 0); }
int pe_plat_cd_data_sync(int mode);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    if (entry != 0x8007BDDCu && entry != 0x8007A910u && entry != 0x80080B24u) return -1;
    if (mutant == 1) { *ret = 0; return 1; }
    *ret = (uint32_t)pe_plat_cd_data_sync((int)a[0]);
    return 1;
}
