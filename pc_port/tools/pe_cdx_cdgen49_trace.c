/* cdgen49 runtime trace logger (test-only, observes, never changes behaviour): linker-wrap logger for the live
 * symbols of platform/pe_libcd.c.  Link into the full runtime with
 *   -Wl,--wrap=func_8006E6D4,--wrap=pe_plat_ds_system_status,--wrap=pe_plat_cd_queue_len,--wrap=pe_plat_cd_disc_type,
 *   --wrap=PE_Cd_TimeoutRecover,--wrap=PE_Cd_XaServiceAvailable,--wrap=PE_Cd_FilePresent,
 *   --wrap=pe_plat_cd_store_setloc_hint,--wrap=PE_Cd_GetSetlocRaw   pe_cdx_cdgen49_trace.o
 * (--wrap rewrites references between object files only, so a call inside pe_libcd.c itself is not seen; every
 * caller listed in the census is another object.)  Output file: env PE_CDX_TRACE49 (default ./pe_cdx_trace49.log).
 * One record per DISTINCT tuple, each ends with `| V <VBlank counter at first sight> | N <times seen>`:
 *   T 8006E6D4 a0 a1 a2 a3 | M flags=.. hw=.. s574=.. s578=.. b590=.. a3608=.. | G rdy=.. ql=.. dt=.. | R ret=.. flagsafter=..
 *   T TimeoutRecover | M b294=.. b295=.. b296=.. cb=.. ctx=.. tbl=.. | ...
 *   T XaServiceAvailable | R ret   T FilePresent name ptr | R ret   T SetlocHint raw   T GetSetlocRaw | R ret      */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
uint32_t PE_GPU_VSyncQuery(void);
int __real_func_8006E6D4(int, int, pe_addr_t, int);
int __real_pe_plat_ds_system_status(void);
int32_t __real_pe_plat_cd_queue_len(void);
int32_t __real_pe_plat_cd_disc_type(void);
void __real_PE_Cd_TimeoutRecover(void);
int __real_PE_Cd_XaServiceAvailable(void);
int __real_PE_Cd_FilePresent(pe_addr_t);
void __real_pe_plat_cd_store_setloc_hint(uint32_t);
uint32_t __real_PE_Cd_GetSetlocRaw(void);

static FILE *out; static int inited;
#define NREC 4096
static struct { char key[600]; unsigned n; unsigned vb; } rec[NREC]; static unsigned nrec; static unsigned long long total[8];
static int in6d4; static int g_rdy, g_ql, g_dt;

static uint32_t rd32(uint32_t a) { uint32_t v = 0; memcpy(&v, PE_TranslateConst(a, 4), 4); return v; }
static uint16_t rd16(uint32_t a) { uint16_t v = 0; memcpy(&v, PE_TranslateConst(a, 2), 2); return v; }
static uint8_t rd8(uint32_t a) { return *(const uint8_t *)PE_TranslateConst(a, 1); }
static void init(void)
{
    const char *p = getenv("PE_CDX_TRACE49");
    inited = 1;
    if (!p) p = "pe_cdx_trace49.log";
    if (*p) out = fopen(p, "w");
}
static void note(unsigned cls, const char *key)
{
    unsigned i;
    if (!inited) init();
    total[cls]++;
    if (!out) return;
    for (i = 0; i < nrec; i++) if (!strcmp(rec[i].key, key)) { rec[i].n++; return; }
    if (nrec >= NREC) return;
    snprintf(rec[nrec].key, sizeof rec[nrec].key, "%s", key); rec[nrec].n = 1; rec[nrec].vb = PE_GPU_VSyncQuery(); nrec++;
    fprintf(out, "%s | V %u\n", key, rec[nrec - 1].vb); fflush(out);
}
int __wrap_pe_plat_ds_system_status(void) { int r = __real_pe_plat_ds_system_status(); if (in6d4) g_rdy = r; return r; }
int32_t __wrap_pe_plat_cd_queue_len(void) { int32_t r = __real_pe_plat_cd_queue_len(); if (in6d4) g_ql = r; return r; }
int32_t __wrap_pe_plat_cd_disc_type(void) { int32_t r = __real_pe_plat_cd_disc_type(); if (in6d4) g_dt = r; return r; }
int __wrap_func_8006E6D4(int a, int b, pe_addr_t c, int d)
{
    char k[600]; int r; uint32_t flags = rd32(0x800B0CD8u), s574 = rd32(0x8009B574u), s578 = rd32(0x8009B578u), a3608 = rd32(0x800A3608u), b590 = rd32(0x8009B590u);
    unsigned hw = rd16(0x800B0DD4u);
    g_rdy = g_ql = g_dt = 0x7E5A5E5A; in6d4 = 1;
    r = __real_func_8006E6D4(a, b, c, d);
    in6d4 = 0;
    snprintf(k, sizeof k, "T 8006E6D4 %X %X %X %X | M flags=%X hw=%X s574=%X s578=%X b590=%X a3608=%X | G rdy=%X ql=%X dt=%X | R ret=%X flagsafter=%X",
             (unsigned)a, (unsigned)b, (unsigned)c, (unsigned)d, flags, hw, s574, s578, b590, a3608, (unsigned)g_rdy, (unsigned)g_ql, (unsigned)g_dt,
             (unsigned)r, rd32(0x800B0CD8u));
    note(0, k);
    return r;
}
void __wrap_PE_Cd_TimeoutRecover(void)
{
    char k[600]; uint32_t ctx = rd32(0x800A3480u);
    snprintf(k, sizeof k, "T TimeoutRecover | M b294=%X b295=%X b296=%X b2AFD5=%X ctx=%X tbl=%X,%X", rd8(0x8009B294u), rd8(0x8009B295u), rd8(0x8009B296u),
             rd8(0x8009AFD5u), ctx, rd32(0x8009B05Cu), rd32(0x8009AFDCu));
    note(1, k);
    __real_PE_Cd_TimeoutRecover();
}
int __wrap_PE_Cd_XaServiceAvailable(void) { char k[100]; int r = __real_PE_Cd_XaServiceAvailable(); snprintf(k, sizeof k, "T XaServiceAvailable | R ret=%X", (unsigned)r); note(2, k); return r; }
int __wrap_PE_Cd_FilePresent(pe_addr_t n) { char k[100]; int r = __real_PE_Cd_FilePresent(n); snprintf(k, sizeof k, "T FilePresent %X | R ret=%X", (unsigned)n, (unsigned)r); note(3, k); return r; }
void __wrap_pe_plat_cd_store_setloc_hint(uint32_t raw) { char k[100]; snprintf(k, sizeof k, "T SetlocHint %X", (unsigned)raw); note(4, k); __real_pe_plat_cd_store_setloc_hint(raw); }
uint32_t __wrap_PE_Cd_GetSetlocRaw(void) { char k[100]; uint32_t r = __real_PE_Cd_GetSetlocRaw(); snprintf(k, sizeof k, "T GetSetlocRaw | R ret=%X", (unsigned)r); note(5, k); return r; }
__attribute__((destructor)) static void done(void)
{
    if (out) { fprintf(out, "# calls func_8006E6D4=%llu TimeoutRecover=%llu XaServiceAvailable=%llu FilePresent=%llu SetlocHint=%llu GetSetlocRaw=%llu distinct=%u\n",
                       total[0], total[1], total[2], total[3], total[4], total[5], nrec); fclose(out); }
}
