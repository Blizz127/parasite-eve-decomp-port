/* cdgen47 runtime capture: linker-wrap logger for the six CD host providers (retail 0x8007AAB4 ack,
 * 0x8007B010 status poll, 0x8007B558 command issue, 0x8007BF44 sector DMA, 0x8007C564 stream handler,
 * 0x8007CEAC DMA issue).  Test-only tooling: it observes calls, it does not change any behaviour.
 *
 * Link into the full runtime with (no provider or CMake edit; see the REPORT for the exact configure line):
 *   -Wl,--wrap=pe_plat_cd_acknowledge_host,--wrap=pe_plat_cd_status_poll_host,--wrap=pe_plat_cd_command_issue_host,
 *   --wrap=pe_plat_cd_sector_dma_host,--wrap=pe_plat_cd_stream_handler_host,--wrap=pe_plat_cd_dma_issue_host,
 *   --wrap=PE_CdLoadU8,--wrap=PE_CdLoadU32,--wrap=PE_GPU_ReadDPCR,--wrap=PE_GPU_ReadDICR  pe_cdx_cdgen47_capture.o
 * Output: file named by env PE_CDX_CAPTURE (default ./pe_cdx_capture.log; empty string = no output); every record ends with `| V <VBlank counter at first sight>`.  One record per DISTINCT call tuple:
 *   T <retail entry> <args> | R <region>:<hex>.. | M <kind>:<addr>:<size>:<value>;.. | C <callee tokens>
 * where the RAM regions are snapshotted at ENTRY, and M lists the register reads the CURRENT provider made during
 * the call (native-observed, a model input for the generator, never an expected output).  Nested calls (ack from
 * the status poll) attribute reads to the innermost active function.  The key is (entry, args, snapshot, M, C), so a
 * tuple that repeats is counted, not repeated. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
uint32_t PE_GPU_VSyncQuery(void);   /* VBlank counter: first-seen "frame" of a tuple (read-only query) */

uint8_t __real_PE_CdLoadU8(pe_addr_t a);
uint32_t __real_PE_CdLoadU32(pe_addr_t a);
uint32_t __real_PE_GPU_ReadDPCR(void);
uint32_t __real_PE_GPU_ReadDICR(void);
int __real_pe_plat_cd_acknowledge_host(void);
int __real_pe_plat_cd_status_poll_host(uint32_t, uint32_t);
int __real_pe_plat_cd_command_issue_host(uint32_t, uint32_t, uint32_t, uint32_t);
int __real_pe_plat_cd_sector_dma_host(uint32_t, uint32_t);
void __real_pe_plat_cd_stream_handler_host(void);
void __real_pe_plat_cd_dma_issue_host(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
int __real_PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity, uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d);
int __real_PE_Cd_DispatchDataCallback(pe_addr_t target, uint32_t status, pe_addr_t response);

static const struct { uint32_t lo, n; } REG[] = {
    {0x8009AF00u, 0x900u}, {0x800A3400u, 0x400u}, {0x800A8000u, 0x80u}, {0x800B0C00u, 0x100u}, {0x800B8A80u, 0x80u},
    {0x800BE980u, 0x78u}, {0x800C0D00u, 0x100u}, {0x800C20B8u, 0x18u},
};
#define NREG (sizeof REG / sizeof REG[0])
#define MAXD 8
#define LOGCAP 6000
typedef struct {
    uint32_t entry, args[7]; unsigned nargs;
    unsigned char *snap; size_t snaplen; uint32_t vb;
    char m[LOGCAP]; size_t mlen; char c[LOGCAP]; size_t clen;
} Frame;
static Frame st[MAXD]; static int depth = -1;
static FILE *out; static int inited;
static uint64_t *seen; static size_t seen_cap = 1u << 20, seen_n;
static unsigned long long counts[8];
static const uint32_t ENTRIES[6] = {0x8007AAB4u, 0x8007B010u, 0x8007B558u, 0x8007BF44u, 0x8007C564u, 0x8007CEACu};

static void init(void)
{
    const char *p;
    inited = 1;
    p = getenv("PE_CDX_CAPTURE");
    if (!p) p = "pe_cdx_capture.log";   /* harness scripts run under env -i: default to a file in the working directory */
    if (p && *p) out = fopen(p, "w");
    seen = calloc(seen_cap, sizeof *seen);
}
static uint64_t fnv(uint64_t h, const void *d, size_t n)
{
    const unsigned char *s = d; size_t i;
    for (i = 0; i < n; i++) { h ^= s[i]; h *= 1099511628211ull; }
    return h;
}
static int in_ram(uint32_t a) { return a >= 0x80000000u && a < 0x801FFF00u; }
static void enter(uint32_t entry, unsigned nargs, const uint32_t *a)
{
    size_t off = 0, i; Frame *f;
    if (!inited) init();
    if (!out || depth + 1 >= MAXD) { depth++; return; }
    f = &st[++depth];
    f->entry = entry; f->nargs = nargs; memset(f->args, 0, sizeof f->args);
    for (i = 0; i < nargs; i++) f->args[i] = a[i];
    f->mlen = f->clen = 0; f->m[0] = f->c[0] = 0; f->vb = PE_GPU_VSyncQuery();
    free(f->snap);
    f->snaplen = 0; for (i = 0; i < NREG; i++) f->snaplen += REG[i].n;
    f->snaplen += 7 * 64;
    f->snap = malloc(f->snaplen);
    for (i = 0; i < NREG; i++) { memcpy(f->snap + off, PE_TranslateConst(REG[i].lo, REG[i].n), REG[i].n); off += REG[i].n; }
    for (i = 0; i < 7; i++) {
        if (i < nargs && in_ram(a[i]) && in_ram(a[i] + 64u)) memcpy(f->snap + off, PE_TranslateConst(a[i], 64), 64);
        else memset(f->snap + off, 0, 64);
        off += 64;
    }
}
static void leave(void)
{
    Frame *f; uint64_t h = 1469598103934665603ull, k; size_t i, pos;
    if (depth < 0) return;
    if (depth >= MAXD || !out) { depth--; return; }
    f = &st[depth--];
    h = fnv(h, &f->entry, 4); h = fnv(h, f->args, sizeof f->args); h = fnv(h, f->snap, f->snaplen);
    h = fnv(h, f->m, f->mlen); h = fnv(h, f->c, f->clen);
    for (i = 0; i < 6; i++) if (ENTRIES[i] == f->entry) counts[i]++;
    k = h | 1u; pos = (size_t)(k % seen_cap);
    while (seen[pos] && seen[pos] != k) pos = (pos + 1) % seen_cap;
    if (seen[pos] == k) return;
    seen[pos] = k; seen_n++;
    fprintf(out, "T %08X", f->entry);
    for (i = 0; i < f->nargs; i++) fprintf(out, " %X", f->args[i]);
    fprintf(out, " | R");
    {   size_t off = 0, j;
        for (i = 0; i < NREG + 7; i++) {
            size_t n = i < NREG ? REG[i].n : 64;
            fprintf(out, " %s%08X:", i < NREG ? "" : "A", i < NREG ? REG[i].lo : (uint32_t)(i - NREG));
            for (j = 0; j < n; j++) fprintf(out, "%02X", f->snap[off + j]);
            off += n;
        }
    }
    fprintf(out, " | M %s | C %s | V %u\n", f->mlen ? f->m : "-", f->clen ? f->c : "-", f->vb);
    fflush(out);
}
/* the retail entry's stats are written at exit */
static void note_m(const char *kind, uint32_t a, unsigned size, uint32_t v)
{
    Frame *f;
    if (depth < 0 || depth >= MAXD || !out) return;
    f = &st[depth];
    if (f->mlen + 48 < LOGCAP) f->mlen += (size_t)snprintf(f->m + f->mlen, LOGCAP - f->mlen, "%s:%X:%u:%X;", kind, a, size, v);
}
static void note_c(const char *what, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e)
{
    Frame *f;
    if (depth < 0 || depth >= MAXD || !out) return;
    f = &st[depth];
    if (f->clen + 96 < LOGCAP) f->clen += (size_t)snprintf(f->c + f->clen, LOGCAP - f->clen, "%s,%X,%X,%X,%X,%X;", what, a, b, c, d, e);
}
uint8_t __wrap_PE_CdLoadU8(pe_addr_t a) { uint8_t v = __real_PE_CdLoadU8(a); note_m("b", a, 1, v); return v; }
uint32_t __wrap_PE_CdLoadU32(pe_addr_t a) { uint32_t v = __real_PE_CdLoadU32(a); note_m("w", a, 4, v); return v; }
uint32_t __wrap_PE_GPU_ReadDPCR(void) { uint32_t v = __real_PE_GPU_ReadDPCR(); note_m("w", 0x1F8010F0u, 4, v); return v; }
uint32_t __wrap_PE_GPU_ReadDICR(void) { uint32_t v = __real_PE_GPU_ReadDICR(); note_m("w", 0x1F8010F4u, 4, v); return v; }
int __wrap_PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity, uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d)
{
    note_c("G", fn, a, b, c, d); (void)site; (void)arity;
    return __real_PE_GuestCall(site, fn, arity, a, b, c, d);
}
int __wrap_PE_Cd_DispatchDataCallback(pe_addr_t target, uint32_t status, pe_addr_t response)
{
    int r; note_c("D", target, status, response, 0, 0);
    r = __real_PE_Cd_DispatchDataCallback(target, status, response);
    return r;
}
int __wrap_pe_plat_cd_acknowledge_host(void) { int r; enter(0x8007AAB4u, 0, NULL); r = __real_pe_plat_cd_acknowledge_host(); note_c("ret", (uint32_t)r, 0, 0, 0, 0); leave(); return r; }
int __wrap_pe_plat_cd_status_poll_host(uint32_t a, uint32_t b)
{
    uint32_t v[2]; int r; v[0] = a; v[1] = b; enter(0x8007B010u, 2, v);
    r = __real_pe_plat_cd_status_poll_host(a, b); note_c("ret", (uint32_t)r, 0, 0, 0, 0); leave(); return r;
}
int __wrap_pe_plat_cd_command_issue_host(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    uint32_t v[4]; int r; v[0] = a; v[1] = b; v[2] = c; v[3] = d; enter(0x8007B558u, 4, v);
    r = __real_pe_plat_cd_command_issue_host(a, b, c, d); note_c("ret", (uint32_t)r, 0, 0, 0, 0); leave(); return r;
}
int __wrap_pe_plat_cd_sector_dma_host(uint32_t a, uint32_t b)
{
    uint32_t v[2]; int r; v[0] = a; v[1] = b; enter(0x8007BF44u, 2, v);
    r = __real_pe_plat_cd_sector_dma_host(a, b); note_c("ret", (uint32_t)r, 0, 0, 0, 0); leave(); return r;
}
void __wrap_pe_plat_cd_stream_handler_host(void) { enter(0x8007C564u, 0, NULL); __real_pe_plat_cd_stream_handler_host(); leave(); }
void __wrap_pe_plat_cd_dma_issue_host(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g)
{
    uint32_t v[7]; v[0] = a; v[1] = b; v[2] = c; v[3] = d; v[4] = e; v[5] = f; v[6] = g; enter(0x8007CEACu, 7, v);
    __real_pe_plat_cd_dma_issue_host(a, b, c, d, e, f, g); leave();
}
__attribute__((destructor)) static void cap_done(void)
{
    if (out) {
        fprintf(out, "# calls ack=%llu poll=%llu cmd=%llu sector=%llu stream=%llu dma=%llu distinct=%zu\n",
                counts[0], counts[1], counts[2], counts[3], counts[4], counts[5], seen_n);
        fflush(out);
    }
}
