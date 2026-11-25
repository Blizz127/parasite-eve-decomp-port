/* Lockstep vector check for the clean-room libgpu/libgte rows (generator:
 * pc_port/tools/pe_gpu_rows_vectors.py).  Build line (run verbatim):
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_SET_A -Dfunc_80073A44=chk_vsync -Ipc_port/tools/chk_inc -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu_rows_check.c pc_port/platform/pe_plat_gpu_timeout.c pc_port/platform/pe_plat_gpu_dma_start.c -Wl,--unresolved-symbols=ignore-all -o check
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_SET_B -Dpe_plat_gpu_timeout=chk_timeout -Dfunc_800773D0=chk_seed -Ipc_port/tools/chk_inc -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu_rows_check.c pc_port/platform/pe_plat_gpu_dma_queue.c -Wl,--unresolved-symbols=ignore-all -o checkb
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_SET_C -Dpe_plat_gpu_dma_queue_run=chk_dmaq -Ipc_port/tools/chk_inc -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu_rows_check.c pc_port/platform/pe_plat_gpu_drawsync.c -Wl,--unresolved-symbols=ignore-all -o checkc
 *   cc -std=c99 -Wall -Wextra -Werror -DCHK_SET_E -Ipc_port/tools/chk_inc -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu_rows_check.c pc_port/platform/pe_plat_gpu_resetgraph.c pc_port/platform/pe_plat_resetgraph_light_host.c -Wl,--unresolved-symbols=ignore-all -o checke
 *   ./checke vectors.txt   (set E: resetgraph; the light-path modes also exercise the light provider, whose host-only
 *                           guard reads of the GPU register pointer words 0x80095860/70/54 consume no event)
 *   ./check vectors.txt    (set A: timeout, dmastart)   ./checkc vectors.txt   (set C: drawsync)   ./checkb vectors.txt    (set B: dmaq)
 * Line: ENTRY seed A a0..a3 R name=v,v;name=v RET r EV event...
 * Events (the original's observable behaviour in order):
 *   L:addr:size=val   data load      S:addr:size=val   data store
 *   C:name:a0,a1,a2,a3[:stk=x,y]   call into a stubbed callee
 * The code under test must perform exactly the same events in the same order:
 * a load must name the same address and size (it receives the value the
 * original loaded), a store must match address, size and value, a callee call
 * must match the name and the arguments the row declares significant; the
 * callee returns the scripted value.  After the call every event must have
 * been consumed and the return value must match (when the row has one). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#if defined(CHK_SET_A) || defined(CHK_SET_C) || defined(CHK_SET_E)
#include "pe_bios_string.h"
#endif
#ifdef CHK_SET_A
#include "pe_plat/gpu_timeout.h"
#include "pe_plat/gpu_dma_start.h"
#endif
#ifdef CHK_SET_B
#include "game_port.h"
#include "pe_plat/gpu_dma_queue.h"
int32_t chk_timeout(void);
#endif
#ifdef CHK_SET_C
#include "pe_plat/gpu_drawsync.h"
#endif
#ifdef CHK_SET_E
#include "game_port.h"
#include "pe_plat/libc.h"
#include "pe_plat/irq_callback.h"
#include "pe_plat/gpu_primitive.h"
#include "pe_plat/gpu_resetgraph.h"
#endif

#define MAXEV 8192
typedef struct { char t; uint32_t addr, val; unsigned size; char name[32]; uint32_t arg[4], stk; } Ev;
static Ev evs[MAXEV];
static unsigned nev, pos;
#ifdef CHK_SET_E
static int e_light;   /* the case's mode is a light-path mode (low three bits not 0, 3, 5) */
#endif
static int fail;
static char failmsg[256];
static struct { char name[32]; uint32_t v[64]; unsigned n, used; } rets[8];
static unsigned nrets;

static void bad(const char *what, uint32_t a, unsigned n)
{
    if (!fail)
        snprintf(failmsg, sizeof failmsg, "event %u: unexpected %s %08X/%u (expected %c %s %08X/%u=%X)", pos, what, a, n,
                 pos < nev ? evs[pos].t : '-', pos < nev ? evs[pos].name : "", pos < nev ? evs[pos].addr : 0,
                 pos < nev ? evs[pos].size : 0, pos < nev ? evs[pos].val : 0);
    fail = 1;
}
static uint32_t ld(pe_addr_t a, unsigned n)
{
    if (fail) return 0;
#ifdef CHK_SET_E
    /* host-only guard reads of the light provider (GPU register pointer words): not events of the original;
     * tolerated for the light modes only, a reset mode must not read these words at all */
    if (e_light && (a == 0x80095860u || a == 0x80095870u || a == 0x80095854u) &&
        (pos >= nev || evs[pos].t != 'L' || evs[pos].addr != a)) return 0x1F801814u;
#endif
    if (pos >= nev || evs[pos].t != 'L' || evs[pos].addr != a || evs[pos].size != n) { bad("load", a, n); return 0; }
    return evs[pos++].val;
}
static void st(pe_addr_t a, unsigned n, uint32_t v)
{
    if (fail) return;
    if (pos >= nev || evs[pos].t != 'S' || evs[pos].addr != a || evs[pos].size != n ||
        evs[pos].val != (n == 4 ? v : v & ((1u << (8 * n)) - 1))) { bad("store", a, n); return; }
    pos++;
}
uint8_t PE_LoadU8(pe_addr_t a) { return (uint8_t)ld(a, 1); }
uint16_t PE_LoadU16(pe_addr_t a) { return (uint16_t)ld(a, 2); }
uint32_t PE_LoadU32(pe_addr_t a) { return ld(a, 4); }
void PE_StoreU8(pe_addr_t a, uint8_t v) { st(a, 1, v); }
void PE_StoreU16(pe_addr_t a, uint16_t v) { st(a, 2, v); }
void PE_StoreU32(pe_addr_t a, uint32_t v) { st(a, 4, v); }

static uint32_t scripted(const char *name)
{
    unsigned i;
    for (i = 0; i < nrets; i++)
        if (!strcmp(rets[i].name, name)) {
            unsigned k = rets[i].used++;
            return rets[i].v[k < rets[i].n ? k : rets[i].n - 1];
        }
    return 0;
}
/* Callee stub: nargs significant arguments are compared. */
static uint32_t callee(const char *name, unsigned nargs, uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    uint32_t a[4]; unsigned i;
    a[0] = a0; a[1] = a1; a[2] = a2; a[3] = a3;
    if (fail) return 0;
    if (pos >= nev || evs[pos].t != 'C' || strcmp(evs[pos].name, name)) { bad(name, a0, 0); return 0; }
    for (i = 0; i < nargs; i++)
        if (evs[pos].arg[i] != a[i]) { bad(name, a[i], i); return 0; }
    pos++;
    return scripted(name);
}
#ifdef CHK_SET_A
uint32_t chk_vsync(int mode) { return callee("VSync", 1, (uint32_t)mode, 0, 0, 0); }
#endif
#ifdef CHK_SET_C
int32_t chk_dmaq(int32_t mode) { return (int32_t)callee("DMAQ", 1, (uint32_t)mode, 0, 0, 0); }
#endif
#if defined(CHK_SET_C) || defined(CHK_SET_E)
unsigned PE_Port_StopEpoch(void) { return 0; }
int PE_Port_ShouldStop(void) { return fail; }
int PE_GuestHook_Call(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0,
                      uintptr_t a1, uintptr_t a2, uintptr_t a3, int *ret)
{
    const char *name = fn == 0x80130000u ? "HOOKA" : fn == 0x80130040u ? "HOOKB" : fn == 0x80130100u ? "LOG" :
                       fn == 0x80130140u ? "LOG2" : "UNKNOWN";
    (void)slot; (void)caller;
    *ret = (int)callee(name, arity, (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
    return !fail;
}
int PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    int ret = 0;
    (void)site;
    (void)PE_GuestHook_Call("", "", fn, arity, a0, a1, a2, a3, &ret);
    return ret;
}
#endif
#ifdef CHK_SET_B
int32_t chk_timeout(void) { return (int32_t)callee("TO", 0, 0, 0, 0, 0); }
uint32_t chk_seed(void) { return callee("G298C", 0, 0, 0, 0, 0); }
int PE_Port_ShouldStop(void) { return fail; }   /* a failed case ends every wait */
void PE_Port_RequestStop(PEPortStopReason reason) { (void)reason; fail = 1; }
int PE_GuestHook_Call(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0,
                      uintptr_t a1, uintptr_t a2, uintptr_t a3, int *ret)
{
    const char *name = fn == 0x80130000u ? "FN" : fn == 0x80130040u ? "FN2" : fn == 0x80130100u ? "CB" :
                       fn == 0x80130140u ? "CB2" : "UNKNOWN";
    (void)slot; (void)caller;
    *ret = (int)callee(name, arity, (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
    return !fail;
}
#endif
#if defined(CHK_SET_A) || defined(CHK_SET_C) || defined(CHK_SET_E)
int32_t PE_Bios_Printf(pe_addr_t fmt, const uint32_t *args, int nargs)
{
    uint32_t a[4] = {0, 0, 0, 0}; int i;
    if (fail) return 0;
    for (i = 0; i < nargs && i < 4; i++) a[i] = args[i];
    if (pos >= nev || evs[pos].t != 'C' || strcmp(evs[pos].name, "BIOS_A3F") || evs[pos].arg[0] != fmt ||
        nargs < 1 || nargs > 4 || (nargs > 0 && evs[pos].arg[1] != a[0]) || (nargs > 1 && evs[pos].arg[2] != a[1]) ||
        (nargs > 2 && evs[pos].arg[3] != a[2]) || (nargs > 3 && evs[pos].stk != a[3])) {
        bad("printf", fmt, (unsigned)nargs);
        return 0;
    }
    pos++;
    return 0;
}
#endif

#ifdef CHK_SET_E
/* ResetGraph callees.  The fill value is compared sign-extended from its byte: retail passes -1 (0xFF) or 0. */
void pe_plat_fill_bytes(uint32_t dst, uint8_t value, int32_t count)
{ (void)callee("FILL", 3, dst, (uint32_t)(int32_t)(int8_t)value, (uint32_t)count, 0); }
uint32_t pe_plat_reset_callback(void) { return callee("RESETCB", 0, 0, 0, 0, 0); }
void pe_plat_gpu_cw(uint32_t word) { (void)callee("GPUCW", 1, word, 0, 0, 0); }
int pe_plat_gpu_2700(int mode) { return (int)callee("G2700", 1, (uint32_t)mode, 0, 0, 0); }
#endif

/* host-hook stubs (pe_plat/gpu_host.h): host state is not part of the vectors */
#include "pe_plat/gpu_host.h"
void pe_plat_gpu_host_drawsync(int32_t mode) { (void)mode; }
int32_t pe_plat_gpu_host_deadline_clock(int32_t now) { return now; }
int pe_plat_gpu_host_wait_tick(void) { return 0; }
int pe_plat_gpu_host_dma_preflight(uint32_t packet) { (void)packet; return 1; }
void pe_plat_gpu_host_dma_run(void) {}
void pe_plat_gpu_host_present_dispenv(uint32_t env) { (void)env; }
void pe_plat_gpu_host_reset_prepare(void) {}

/* host-guard stubs (pe_plat/gpu_regs.h): every pointer in the vectors is valid */
bool PE_RangeIsRam(pe_addr_t a, size_t n) { (void)a; (void)n; return true; }
int PE_MMIO_IsModelled(pe_addr_t a, size_t n) { (void)a; (void)n; return 1; }
void PE_GuestHook_Refuse(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1,
                         uintptr_t a2, uintptr_t a3, const char *why)
{ (void)slot; (void)caller; (void)fn; (void)arity; (void)a0; (void)a1; (void)a2; (void)a3; (void)why; fail = 1; }

static int run_entry(uint32_t entry, const uint32_t *a, uint32_t *ret, int *has)
{
    (void)a; *has = 1;
    switch (entry) {
#ifdef CHK_SET_C
    case 0x80074DC0u: *ret = (uint32_t)pe_plat_gpu_drawsync((int32_t)a[0]); return 0;
#endif
#ifdef CHK_SET_A
    case 0x80077404u: *ret = (uint32_t)pe_plat_gpu_timeout(); return 0;
    case 0x80076B98u: *ret = (uint32_t)pe_plat_gpu_dma_start(a[0], a[1]); return 0;
#endif
#ifdef CHK_SET_E
    case 0x80074A44u: *ret = (uint32_t)pe_plat_gpu_resetgraph((int32_t)a[0]); return 0;
#endif
#ifdef CHK_SET_B
    case 0x80077294u: *ret = (uint32_t)pe_plat_gpu_dma_queue_run((int32_t)a[0]); return 0;
#endif
    }
    return -1;
}

static void parse_events(char **tok, int nt, int from)
{
    int i;
    nev = 0;
    for (i = from; i < nt && nev < MAXEV; i++) {
        Ev *e = &evs[nev]; char *t = tok[i];
        memset(e, 0, sizeof *e);
        if (!strcmp(t, "-")) continue;
        e->t = t[0];
        if (t[0] == 'C') {
            char *p = t + 2, *q = strchr(p, ':'); unsigned n = (unsigned)(q - p);
            if (n >= sizeof e->name) n = sizeof e->name - 1;
            memcpy(e->name, p, n);
            sscanf(q + 1, "%x,%x,%x,%x", &e->arg[0], &e->arg[1], &e->arg[2], &e->arg[3]);
            q = strstr(q, ":stk=");
            if (q) sscanf(q + 5, "%x", &e->stk);
        } else {
            unsigned a, s, v;
            if (sscanf(t + 2, "%x:%u=%x", &a, &s, &v) != 3) { fprintf(stderr, "bad event %s\n", t); exit(2); }
            e->addr = a; e->size = s; e->val = v;
        }
        nev++;
    }
}

int main(int c, char **v)
{
    FILE *f; static char line[1 << 20]; unsigned n = 0, nbad = 0;
    if (c != 2 || !(f = fopen(v[1], "r"))) return 2;
    while (fgets(line, sizeof line, f)) {
        static char *tok[MAXEV + 64]; static char copy[1 << 20];
        int nt = 0, i, ri, ei; uint32_t a[4] = {0, 0, 0, 0}, entry, got = 0, want = 0; int has = 0, ok;
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < MAXEV + 64 && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
        if (nt < 10 || strcmp(tok[2], "A")) continue;
        entry = (uint32_t)strtoul(tok[0], 0, 16);
        for (i = 0; i < 4; i++) a[i] = (uint32_t)strtoul(tok[3 + i], 0, 16);
        if (strcmp(tok[7], "R")) continue;
        /* R list */
        nrets = 0;
        if (strcmp(tok[8], "-")) {
            char *s = tok[8], *e;
            while (*s && nrets < 8) {
                e = strchr(s, ';'); if (e) *e = 0;
                char *eq = strchr(s, '=');
                *eq = 0;
                strncpy(rets[nrets].name, s, 31); rets[nrets].n = rets[nrets].used = 0;
                {
                    char *p = eq + 1, *q;
                    while (*p && rets[nrets].n < 64) {
                        rets[nrets].v[rets[nrets].n++] = (uint32_t)strtoul(p, &q, 16);
                        p = *q == ',' ? q + 1 : q;
                    }
                }
                nrets++;
                if (!e) break;
                s = e + 1;
            }
        }
        ri = 9;
        if (strcmp(tok[ri], "RET")) continue;
        ei = ri + 2;
        if (strcmp(tok[ei], "EV")) continue;
        parse_events(tok, nt, ei + 1);
        n++; pos = 0; fail = 0; failmsg[0] = 0;
#ifdef CHK_SET_E
        e_light = entry == 0x80074A44u && (a[0] & 7u) != 0u && (a[0] & 7u) != 3u && (a[0] & 7u) != 5u;
#endif
        if (run_entry(entry, a, &got, &has) < 0) { fprintf(stderr, "unknown entry %08X\n", entry); return 2; }
        want = (uint32_t)strtoul(tok[ri + 1], 0, 16);
        ok = !fail && pos == nev && (!has || got == want);
        if (!ok && nbad++ < 12)
            fprintf(stderr, "MISMATCH line %u: %s%s%s ret %X/%X consumed %u/%u: %.160s\n", n, failmsg,
                    pos != nev ? " [events left]" : "", got != want ? " [ret]" : "", got, want, pos, nev, line);
    }
    printf("checked %u cases, %u mismatches\n", n, nbad);
    return nbad != 0;
}
