/* RAM-model vector check for rows whose load ORDER is not part of the contract
 * (PutDispEnv; generator pc_port/tools/pe_gpu_rows_vectors.py `dispenv`).  Build line (run verbatim):
 *   cc -std=c99 -Wall -Wextra -Werror -Dfunc_80076B20=chk_ctl -Dpe_plat_bios_memcpy=chk_memcpy -Ipc_port/tools/chk_inc -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu_ramcheck.c pc_port/platform/pe_plat_gpu_putdispenv.c -Wl,--unresolved-symbols=ignore-all -o checkd
 *   ./checkd vectors.txt
 * Same line format as pe_gpu_rows_check.c.  Memory starts as poison; every byte
 * the original LOADED before it stored to it gets the value it loaded (first
 * access wins).  The code under test may read only those bytes (or bytes it
 * wrote itself).  After the call, its ordered list of stores and callee calls
 * must equal the original's (loads are not compared), and the return value must
 * match.  Callees: the GPU control write (0x80076B20), the BIOS memcpy, the
 * trace hook and the BIOS printf. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_bios_string.h"
#include "pe_plat/gpu_putdispenv.h"

#define MEM 0x200000u
#define MAXEV 4096
typedef struct { char t; uint32_t addr, val; unsigned size; char name[32]; uint32_t arg[4]; } Ev;
static Ev evs[MAXEV], got[MAXEV];
static unsigned nev, ngot;
static uint8_t ram[MEM];
static uint32_t known[MEM], touched[MEM], wrote[MEM], cur;
static int bad_read;
static char why[300];
static struct { char name[32]; uint32_t v[8]; unsigned n, used; } rets[8];
static unsigned nrets;

static uint32_t ld(pe_addr_t a, unsigned n)
{
    uint32_t v = 0; unsigned i;
    for (i = 0; i < n; i++) {
        uint32_t b = (a + i) & (MEM - 1);
        if (wrote[b] != cur && known[b] != cur) bad_read = 1;
        v |= (uint32_t)ram[b] << (8 * i);
    }
    return v;
}
static void st(pe_addr_t a, unsigned n, uint32_t v)
{
    unsigned i;
    for (i = 0; i < n; i++) {
        uint32_t b = (a + i) & (MEM - 1);
        ram[b] = (uint8_t)(v >> (8 * i)); wrote[b] = cur;
    }
    if (ngot < MAXEV) { Ev *e = &got[ngot++]; memset(e, 0, sizeof *e); e->t = 'S'; e->addr = a; e->size = n; e->val = n == 4 ? v : v & ((1u << (8 * n)) - 1); }
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
        if (!strcmp(rets[i].name, name)) { unsigned k = rets[i].used++; return rets[i].v[k < rets[i].n ? k : rets[i].n - 1]; }
    return 0;
}
static uint32_t call(const char *name, unsigned nargs, uint32_t a0, uint32_t a1, uint32_t a2)
{
    if (ngot < MAXEV) {
        Ev *e = &got[ngot++]; memset(e, 0, sizeof *e);
        e->t = 'C'; snprintf(e->name, sizeof e->name, "%s", name);
        e->arg[0] = nargs > 0 ? a0 : 0; e->arg[1] = nargs > 1 ? a1 : 0; e->arg[2] = nargs > 2 ? a2 : 0;
    }
    return scripted(name);
}
void chk_ctl(unsigned cmd) { (void)call("WCM", 1, cmd, 0, 0); }
uint32_t chk_memcpy(uint32_t d, uint32_t s, int32_t n) { (void)call("BIOS_A2A", 3, d, s, (uint32_t)n); return d; }
int32_t PE_Bios_Printf(pe_addr_t fmt, const uint32_t *args, int nargs)
{
    (void)call("BIOS_A3F", 2, fmt, nargs > 0 ? args[0] : 0, 0);
    return 0;
}
unsigned PE_Port_StopEpoch(void) { return 0; }
int PE_Port_ShouldStop(void) { return 0; }
int PE_GuestHook_Call(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0,
                      uintptr_t a1, uintptr_t a2, uintptr_t a3, int *ret)
{
    const char *name = fn == 0x80130000u ? "HOOKA" : fn == 0x80130040u ? "HOOKB" : fn == 0x80130100u ? "LOG" :
                       fn == 0x80130140u ? "LOG2" : "UNKNOWN";
    (void)slot; (void)caller; (void)a3;
    *ret = (int)call(name, arity, (uint32_t)a0, (uint32_t)a1, (uint32_t)a2);
    return 1;
}

int PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    int ret = 0;
    (void)site;
    (void)PE_GuestHook_Call("", "", fn, arity, a0, a1, a2, a3, &ret);
    return ret;
}

/* host-guard stubs (pe_plat/gpu_regs.h): every pointer in the vectors is valid */
/* host-hook stubs (pe_plat/gpu_host.h): host state is not part of the vectors */
#include "pe_plat/gpu_host.h"
void pe_plat_gpu_host_drawsync(int32_t mode) { (void)mode; }
int pe_plat_gpu_host_wait_tick(void) { return 0; }
int pe_plat_gpu_host_dma_preflight(uint32_t packet) { (void)packet; return 1; }
void pe_plat_gpu_host_dma_run(void) {}
void pe_plat_gpu_host_present_dispenv(uint32_t env) { (void)env; }
bool PE_RangeIsRam(pe_addr_t a, size_t n) { (void)a; (void)n; return true; }
int PE_MMIO_IsModelled(pe_addr_t a, size_t n) { (void)a; (void)n; return 1; }
void PE_GuestHook_Refuse(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1,
                         uintptr_t a2, uintptr_t a3, const char *why)
{ (void)slot; (void)caller; (void)fn; (void)arity; (void)a0; (void)a1; (void)a2; (void)a3; (void)why; bad_read = 1; }

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
        } else {
            unsigned a, s, v;
            if (sscanf(t + 2, "%x:%u=%x", &a, &s, &v) != 3) { fprintf(stderr, "bad event %s\n", t); exit(2); }
            e->addr = a; e->size = s; e->val = v;
        }
        nev++;
    }
}
/* significant call arguments per callee name */
static unsigned nsig(const char *n)
{
    if (!strcmp(n, "WCM")) return 1;
    if (!strcmp(n, "BIOS_A2A")) return 3;
    if (!strcmp(n, "BIOS_A3F")) return 2;
    if (!strncmp(n, "LOG", 3)) return 2;
    return 1;
}

int main(int c, char **v)
{
    FILE *f; static char line[1 << 20]; unsigned n = 0, nbad = 0;
    if (c != 2 || !(f = fopen(v[1], "r"))) return 2;
    while (fgets(line, sizeof line, f)) {
        static char *tok[MAXEV + 64]; static char copy[1 << 20];
        int nt = 0, i, ri, ei; uint32_t a[4] = {0, 0, 0, 0}, gotret, want; unsigned k, w, ok;
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < MAXEV + 64 && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
        if (nt < 10 || strcmp(tok[2], "A") || strcmp(tok[7], "R")) continue;
        for (i = 0; i < 4; i++) a[i] = (uint32_t)strtoul(tok[3 + i], 0, 16);
        nrets = 0;
        if (strcmp(tok[8], "-")) {
            char *s = tok[8], *e;
            while (*s && nrets < 8) {
                char *eq, *p, *q;
                e = strchr(s, ';'); if (e) *e = 0;
                eq = strchr(s, '='); *eq = 0;
                strncpy(rets[nrets].name, s, 31); rets[nrets].n = rets[nrets].used = 0;
                p = eq + 1;
                while (*p && rets[nrets].n < 8) { rets[nrets].v[rets[nrets].n++] = (uint32_t)strtoul(p, &q, 16); p = *q == ',' ? q + 1 : q; }
                nrets++;
                if (!e) break;
                s = e + 1;
            }
        }
        ri = 9; ei = ri + 2;
        if (strcmp(tok[ri], "RET") || strcmp(tok[ei], "EV")) continue;
        want = (uint32_t)strtoul(tok[ri + 1], 0, 16);
        parse_events(tok, nt, ei + 1);
        n++; cur = n; ngot = 0; bad_read = 0; why[0] = 0;
        for (k = 0; k < nev; k++) {
            Ev *e = &evs[k];
            if (e->t != 'L' && e->t != 'S') continue;
            for (w = 0; w < e->size; w++) {
                uint32_t b = (e->addr + w) & (MEM - 1);
                if (touched[b] != cur) {
                    touched[b] = cur;
                    if (e->t == 'L') { known[b] = cur; ram[b] = (uint8_t)(e->val >> (8 * w)); }
                }
            }
        }
        gotret = pe_plat_gpu_put_disp_env(a[0]);
        ok = !bad_read && gotret == want;
        {   /* expected = original's S and C events, in order */
            unsigned j = 0;
            for (k = 0; k < nev && ok; k++) {
                Ev *e = &evs[k], *g;
                unsigned m;
                if (e->t != 'S' && e->t != 'C') continue;
                if (j >= ngot) { ok = 0; snprintf(why, sizeof why, "missing %c %s %X", e->t, e->name, e->addr); break; }
                g = &got[j++];
                if (g->t != e->t) { ok = 0; snprintf(why, sizeof why, "event %u: want %c %s %X got %c %s %X", j - 1, e->t, e->name, e->addr, g->t, g->name, g->addr); break; }
                if (e->t == 'S') { if (g->addr != e->addr || g->size != e->size || g->val != e->val) { ok = 0; snprintf(why, sizeof why, "store want %X/%u=%X got %X/%u=%X", e->addr, e->size, e->val, g->addr, g->size, g->val); } }
                else {
                    if (strcmp(g->name, e->name)) { ok = 0; snprintf(why, sizeof why, "call want %s got %s", e->name, g->name); }
                    for (m = 0; m < nsig(e->name); m++) if (g->arg[m] != e->arg[m]) { ok = 0; snprintf(why, sizeof why, "call %s arg%u want %X got %X", e->name, m, e->arg[m], g->arg[m]); }
                }
            }
            if (ok && j != ngot) { ok = 0; snprintf(why, sizeof why, "extra events %u/%u", j, ngot); }
        }
        if (!ok && nbad++ < 12)
            fprintf(stderr, "MISMATCH line %u:%s ret %X/%X [%s] %.120s\n", n, bad_read ? " [read outside original loads]" : "",
                    gotret, want, why, line);
    }
    printf("checked %u cases, %u mismatches\n", n, nbad);
    return nbad != 0;
}
