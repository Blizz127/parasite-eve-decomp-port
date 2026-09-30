/*
 * Phase 6E-A batch 3 — Native adaptation of func_8006E834 (post-mount
 * image loader), rewired to the real disc providers (pe_libcd.c).
 *
 * On a real image, B54K-P adopts retail rodata D_80093164={0x03D2,0x0457}
 * and D_80011614=0x8018EFF0 after the boot EXE is authenticated.  This call
 * therefore loads the 0x42800-byte overlay containing func_801909B4.
 * Bootstrap fixtures retain an explicit zero range and exercise the same
 * retry/poll topology through a trivial-length read.
 *
 * All bootstrap stubs now use centralized Bootstrap_ReturnInt/Void.
 * D_800BCE80 is guest-RAM-backed (defined in psx_compat.h), no local override.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_bootstrap.h"
#include <string.h>

/* ── Globals ───────────────────────────────────────────────────────── */
/* D_800B0DB2-B7, D_800B0CD8, D_800B0DD8 are guest-RAM lvalue macros
 * (psx_compat.h).  D_80011614 is a pe_addr_t guest address. */
extern unsigned short D_80093164[4];

extern void func_80086FF8(void);
extern void func_800726C4(void);
extern uint32_t func_80073A44(int a);
extern void func_80074D28(int a);
extern pe_addr_t func_800755F0(pe_addr_t env);
/* func_80072714/func_80072724 (pe_libetc.c), func_800749D8 (pe_libgpu.c)
 * and func_8006E6D4/func_800811E4 (pe_libcd.c) are real implementations
 * (Phase 6E-A) via pe_sdk.h. */

/* Retail builds the DISPENV on the guest stack (sp+0x18).  The host does
 * not track a guest stack pointer, so the env lives at a fixed scratch
 * address inside guest RAM; it is local to this call and dead after
 * PutDispEnv.  All accesses stay bounds-checked pe_addr_t. */
#define PE_6E834_ENV_ADDR 0x801FFF00u

/* Retail passes a stack pointer (sp+0x30) to func_800811E4, which forwards
 * it to the collapsed DsDataSync query.  The host uses a fixed guest
 * scratch address instead of a host stack pointer. */
#define PE_6E834_SYNC_ADDR 0x801FFEE0u

/* func_80086FF8 is real (pe_stream.c, Phase 6E-A batch 2), and
 * func_800726C4 is the host-safe BIOS A0(44h) FlushCache adapter in
 * pe_libetc.c. */

/* ── Adapted function ──────────────────────────────────────────────── */
int func_8006E834(void)
{
    unsigned short *tbl;
    int r;

    D_800B0DB5 = -1;
    D_800B0DB4 = -1;
    D_800B0DB7 = -1;
    D_800B0DB6 = -1;
    D_800B0DB3 = -1;
    D_800B0DB2 = -1;
    D_800B0CD8 &= ~0xF0;
    func_80086FF8();

retry:
    tbl = D_80093164;
    do {
        r = func_8006E6D4(D_800B0DD8 + tbl[0], 0, D_80011614, tbl[1] - tbl[0]);
    } while (r == -1);

    for (;;) {
        r = func_800811E4(PE_6E834_SYNC_ADDR);
        if ((unsigned)(r + 1) < 2) {
            D_800B0CD8 &= 0xFEFFBFFF;
        }
        if (r == 0) break;
        if (r == -1) goto retry;
    }

    func_80072714();
    func_800726C4();
    func_80072724();
    func_80073A44(0);
    func_80074D28(0);
    func_800749D8(PE_6E834_ENV_ADDR, 0, 0, 0x140, 0xF0);
    PE_StoreU8(PE_6E834_ENV_ADDR + 0x11u, 1);   /* isrgb24 = 1 */
    func_800755F0(PE_6E834_ENV_ADDR);
    return 0;
}
