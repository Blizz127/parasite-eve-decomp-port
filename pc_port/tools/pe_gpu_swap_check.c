/* Lockstep vector check for the clean-room 0x80076C34 provider (generator:
 * pc_port/tools/pe_gpu_swap_vectors.py).  Build line (run verbatim):
 *   cc -std=c99 -Wall -Wextra -Werror -Dpe_plat_gpu_timeout=chk_timeout -Dfunc_800773D0=chk_seed -Dfunc_80073E10=chk_sim -Dfunc_80073CF4=chk_dmacb -Dfunc_80076EE4=chk_kick -Ipc_port/tools/chk_inc -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu_swap_check.c pc_port/platform/pe_plat_gpu_swap.c -Wl,--unresolved-symbols=ignore-all -o checkswap
 *   ./checkswap vectors.txt
 * Line format and lockstep rules as in pe_gpu_rows_check.c: the code under test
 * must perform exactly the recorded loads (address, size; they receive the
 * original's values), stores (address, size, value) and callee calls (name and
 * the significant arguments) in the same order, return the same value, and
 * consume every event.  Callee names: G298C (0x800773D0, 0 args), SIM
 * (0x80073E10, 1), DMACB (0x80073CF4, 2), TO (0x80077404, 0), KICK
 * (0x80076EE4, 0), PC_<addr> (the worker, 2 args: argument, auxiliary).
 * `./checkswap vectors.txt inline8`: only the queued cases with copy_bytes == 8 are run, through
 * pe_plat_gpu_swap_inline8(worker, word0, word1, auxiliary) with the two payload loads
 * (argument + 0, + 4) removed from the event list and their values passed as the words. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_plat/gpu_timeout.h"
#include "pe_plat/gpu_swap.h"

#define MAXEV 8192
typedef struct { char t; uint32_t addr, val; unsigned size; char name[32]; uint32_t arg[4], stk; } Ev;
static Ev evs[MAXEV];
static unsigned nev, pos;
static int fail;
static char failmsg[256];
static struct { char name[32]; uint32_t v[64]; unsigned n, used; } rets[16];
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
int32_t chk_timeout(void) { return (int32_t)callee("TO", 0, 0, 0, 0, 0); }
uint32_t chk_seed(void) { return callee("G298C", 0, 0, 0, 0, 0); }
uint16_t chk_sim(uint16_t m) { return (uint16_t)callee("SIM", 1, m, 0, 0, 0); }
uint32_t chk_dmacb(uint32_t ch, uint32_t h) { return callee("DMACB", 2, ch, h, 0, 0); }
int chk_kick(void) { return (int)callee("KICK", 0, 0, 0, 0, 0); }
void pe_plat_gpu_swap_inline_record(pe_addr_t w, const uint32_t p[2], uint32_t x) { (void)w; (void)p; (void)x; }
int pe_plat_gpu_swap_inline_issue(pe_addr_t w, uint32_t a, uint32_t b, uint32_t x) { (void)w; (void)a; (void)b; (void)x; fail = 1; return 0; }
int PE_Port_EscapeArmed(void) { return 0; }
unsigned PE_Port_StopEpoch(void) { return 0; }
int PE_Port_ShouldStop(void) { return fail; }   /* a failed case ends every wait */
void PE_Port_RequestStop(PEPortStopReason reason) { (void)reason; fail = 1; }
int PE_GuestHook_Call(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0,
                      uintptr_t a1, uintptr_t a2, uintptr_t a3, int *ret)
{
    char name[32];
    snprintf(name, sizeof name, "PC_%08X", fn);
    (void)slot; (void)caller;
    *ret = (int)callee(name, arity, (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
    return !fail;
}
bool PE_RangeIsRam(pe_addr_t a, size_t n) { (void)a; (void)n; return true; }
int PE_MMIO_IsModelled(pe_addr_t a, size_t n) { (void)a; (void)n; return 1; }
void PE_GuestHook_Refuse(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1,
                         uintptr_t a2, uintptr_t a3, const char *why)
{ (void)slot; (void)caller; (void)fn; (void)arity; (void)a0; (void)a1; (void)a2; (void)a3; (void)why; fail = 1; }

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

int main(int c, char **v)
{
    FILE *f; static char line[1 << 20]; unsigned n = 0, nbad = 0;
    int inl = c == 3 && !strcmp(v[2], "inline8");
    if ((c != 2 && !inl) || !(f = fopen(v[1], "r"))) return 2;
    while (fgets(line, sizeof line, f)) {
        static char *tok[MAXEV + 64]; static char copy[1 << 20];
        int nt = 0, i; uint32_t a[4] = {0, 0, 0, 0}, got = 0, want = 0, ok;
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < MAXEV + 64 && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
        if (nt < 10 || strcmp(tok[2], "A") || strcmp(tok[7], "R")) continue;
        for (i = 0; i < 4; i++) a[i] = (uint32_t)strtoul(tok[3 + i], 0, 16);
        nrets = 0;
        if (strcmp(tok[8], "-")) {
            char *s = tok[8], *e;
            while (*s && nrets < 16) {
                char *eq;
                e = strchr(s, ';'); if (e) *e = 0;
                eq = strchr(s, '=');
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
        if (strcmp(tok[9], "RET") || strcmp(tok[11], "EV")) continue;
        parse_events(tok, nt, 12);
        if (inl) {
            unsigned k, d = 0, j; uint32_t w[2] = {0, 0}; int found = 0;
            if (a[2] != 8u) continue;
            for (k = 0; k < nev; k++) if (evs[k].t == 'C' && !strcmp(evs[k].name, "DMACB")) found = 1;
            if (!found) continue;                       /* immediate issue: refused for inline8 callers */
            for (k = 0, j = 0; k < nev; k++) {
                if (evs[k].t == 'L' && (evs[k].addr == a[1] || evs[k].addr == a[1] + 4u) && d < 2 && evs[k].size == 4 &&
                    k + 1 < nev && evs[k + 1].t == 'L' && evs[k + 1].addr == 0x80095874u) {
                    w[d++] = evs[k].val;
                    continue;
                }
                evs[j++] = evs[k];
            }
            if (d != 2) continue;
            nev = j;
            n++; pos = 0; fail = 0; failmsg[0] = 0;
            got = (uint32_t)pe_plat_gpu_swap_inline8(a[0], w[0], w[1], a[3]);
        } else {
        n++; pos = 0; fail = 0; failmsg[0] = 0;
        got = (uint32_t)pe_plat_gpu_swap(a[0], a[1], (int32_t)a[2], a[3]);
        }
        want = (uint32_t)strtoul(tok[10], 0, 16);
        ok = !fail && pos == nev && got == want;
        if (!ok && nbad++ < 12)
            fprintf(stderr, "MISMATCH line %u: %s%s%s ret %X/%X consumed %u/%u: %.160s\n", n, failmsg,
                    pos != nev ? " [events left]" : "", got != want ? " [ret]" : "", got, want, pos, nev, line);
    }
    printf("checked %u cases, %u mismatches\n", n, nbad);
    return nbad != 0;
}
