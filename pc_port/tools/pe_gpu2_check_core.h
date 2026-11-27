/* Shared core of the sw-vec-gpu2 replay checkers (batches gpu2a..gpu2d).
 * Included by pe_gpu2_<batch>_check.c; generator: pe_gpu2_vectors.py.
 *
 * Vector line:  ENTRY seed A a0 a1 a2 a3 R name=r,r;... RET r|_ EV ev...
 *   ev  L:addr:size=val    data load       S:addr:size=val    data store
 *       C:func_XXXXXXXX:a0,a1,a2,a3        call of a guest function the original made (arg `_` = leftover
 *                                          register, not significant); the stub returns the scripted value
 * Contract (what a clean-room provider must reproduce):
 *   - hardware window 0x1F801000..0x1F801FFF: loads/stores and guest calls form ONE ordered stream; every
 *     access must match the next event (address, size, store value; load returns the recorded value);
 *   - guest RAM: loads are answered from the bytes the original read (first read wins, a byte the original
 *     wrote earlier reads back what it wrote); reading any other byte fails; the SET of RAM bytes written
 *     and their final values must equal the original's (store order and width are not constrained);
 *   - the return value must equal RET unless RET is `_` (the original returned a leftover register).
 * GPU2_MUTANT=1 replaces the entry by a no-op stub, =2 flips bit 0 of the return value, =3 flips bit 0 of every
 * store: each must fail wherever the case set has a return value / a store respectively. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>

#define G2_MAXEV 16384
#define G2_MEM 0x200000u
typedef struct { char t; uint32_t addr, val; unsigned size; char name[24]; uint32_t arg[4]; unsigned mask; } G2Ev;
static G2Ev g2_ev[G2_MAXEV];
static unsigned g2_nev, g2_pos, g2_fail;
static char g2_msg[300];
static uint8_t g2_ram[G2_MEM];
static uint32_t g2_known[G2_MEM];        /* stamp = case number: byte readable */
static uint32_t g2_wrote[G2_MEM];        /* stamp: byte written by provider */
static uint32_t g2_exp[G2_MEM];          /* stamp: byte written by original */
static uint8_t g2_expval[G2_MEM];
static uint32_t g2_wlog[1 << 17], g2_nw, g2_xlog[1 << 17], g2_nx;
static uint32_t g2_cur;
static struct { char name[24]; uint32_t v[64]; unsigned n, used; } g2_rets[16];
static unsigned g2_nrets;

static int g2_lenient_mode, g2_extra_read;
static int g2_ishw(uint32_t a) { return (a & 0x1FFFF000u) == 0x1F801000u; }
static void g2_bad(const char *what, uint32_t a, unsigned n)
{
    if (!g2_fail)
        snprintf(g2_msg, sizeof g2_msg, "event %u: unexpected %s %08X/%u (expected %c %s %08X/%u=%X)", g2_pos, what, a, n,
                 g2_pos < g2_nev ? g2_ev[g2_pos].t : '-', g2_pos < g2_nev ? g2_ev[g2_pos].name : "",
                 g2_pos < g2_nev ? g2_ev[g2_pos].addr : 0, g2_pos < g2_nev ? g2_ev[g2_pos].size : 0,
                 g2_pos < g2_nev ? g2_ev[g2_pos].val : 0);
    g2_fail = 1;
}
static int g2_ordered(const G2Ev *e) { return e->t == 'C' || g2_ishw(e->addr); }
static void g2_skip(void) { while (g2_pos < g2_nev && !g2_ordered(&g2_ev[g2_pos])) g2_pos++; }
static uint32_t g2_ld(uint32_t a, unsigned n)
{
    unsigned i; uint32_t v = 0;
    if (g2_fail) return 0;
    if (g2_ishw(a)) {
        g2_skip();
        if (g2_pos >= g2_nev || g2_ev[g2_pos].t != 'L' || g2_ev[g2_pos].addr != a || g2_ev[g2_pos].size != n) {
            g2_bad("hw load", a, n);
            return 0;
        }
        return g2_ev[g2_pos++].val;
    }
    for (i = 0; i < n; i++) {
        uint32_t b = (a + i) & (G2_MEM - 1);
        if (g2_known[b] != g2_cur) {
            if (g2_lenient_mode) { g2_extra_read = 1; continue; }    /* informational smoke mode: read as zero */
            g2_bad("RAM load outside original reads", a, n); return 0;
        }
        v |= (uint32_t)g2_ram[b] << (8 * i);
    }
    return v;
}
static void g2_st(uint32_t a, unsigned n, uint32_t v)
{
    unsigned i;
    if (g2_fail) return;
#if defined(GPU2_MUTANT) && GPU2_MUTANT == 3
    v ^= 1u;
#endif
    if (g2_ishw(a)) {
        g2_skip();
        if (g2_pos >= g2_nev || g2_ev[g2_pos].t != 'S' || g2_ev[g2_pos].addr != a || g2_ev[g2_pos].size != n ||
            g2_ev[g2_pos].val != (n == 4 ? v : v & ((1u << (8 * n)) - 1))) { g2_bad("hw store", a, n); return; }
        g2_pos++;
        return;
    }
    for (i = 0; i < n; i++) {
        uint32_t b = (a + i) & (G2_MEM - 1);
        if (g2_wrote[b] != g2_cur) { g2_wrote[b] = g2_cur; if (g2_nw < (1u << 17)) g2_wlog[g2_nw++] = b; }
        g2_ram[b] = (uint8_t)(v >> (8 * i));
        g2_known[b] = g2_cur;
    }
}
/* Hooks the providers link against (guest memory accessors). */
uint8_t PE_LoadU8(uint32_t a) { return (uint8_t)g2_ld(a, 1); }
uint16_t PE_LoadU16(uint32_t a) { return (uint16_t)g2_ld(a, 2); }
uint32_t PE_LoadU32(uint32_t a) { return g2_ld(a, 4); }
void PE_StoreU8(uint32_t a, uint8_t v) { g2_st(a, 1, v); }
void PE_StoreU16(uint32_t a, uint16_t v) { g2_st(a, 2, v); }
void PE_StoreU32(uint32_t a, uint32_t v) { g2_st(a, 4, v); }

static uint32_t g2_scripted(const char *name)
{
    unsigned i;
    for (i = 0; i < g2_nrets; i++)
        if (!strcmp(g2_rets[i].name, name)) {
            unsigned k = g2_rets[i].used++;
            return g2_rets[i].v[k < g2_rets[i].n ? k : g2_rets[i].n - 1];
        }
    return 0;
}
/* A guest function the original called; the provider must call it with the significant arguments. */
static __attribute__((unused)) uint32_t g2_call(uint32_t addr, uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    char name[24]; uint32_t a[4]; unsigned i;
    a[0] = a0; a[1] = a1; a[2] = a2; a[3] = a3;
    snprintf(name, sizeof name, "func_%08X", addr);
    if (g2_fail) return 0;
    g2_skip();
    if (g2_pos >= g2_nev || g2_ev[g2_pos].t != 'C' || strcmp(g2_ev[g2_pos].name, name)) { g2_bad(name, a0, 0); return 0; }
    for (i = 0; i < 4; i++)
        if ((g2_ev[g2_pos].mask >> i & 1u) && g2_ev[g2_pos].arg[i] != a[i]) { g2_bad(name, a[i], i); return 0; }
    g2_pos++;
    return g2_scripted(name);
}

static void g2_parse_events(char **tok, int nt, int from)
{
    int i;
    g2_nev = 0;
    for (i = from; i < nt && g2_nev < G2_MAXEV; i++) {
        G2Ev *e = &g2_ev[g2_nev]; char *t = tok[i];
        memset(e, 0, sizeof *e);
        if (!strcmp(t, "-")) continue;
        e->t = t[0];
        if (t[0] == 'C') {
            char *p = t + 2, *q = strchr(p, ':'), *s; unsigned n = (unsigned)(q - p), k = 0;
            if (n >= sizeof e->name) n = sizeof e->name - 1;
            memcpy(e->name, p, n);
            s = q + 1;
            while (k < 4) {
                if (*s == '_') { s++; }
                else { char *end; e->arg[k] = (uint32_t)strtoul(s, &end, 16); e->mask |= 1u << k; s = end; }
                k++;
                if (*s == ',') s++;
            }
        } else {
            unsigned a, s, v;
            if (sscanf(t + 2, "%x:%u=%x", &a, &s, &v) != 3) { fprintf(stderr, "bad event %s\n", t); exit(2); }
            e->addr = a; e->size = s; e->val = v;
        }
        g2_nev++;
    }
}
/* Rebuild the case's initial RAM image and the original's final write set from the event order. */
static void g2_prepare(void)
{
    unsigned k, i;
    g2_nx = 0;
    for (k = 0; k < g2_nev; k++) {
        G2Ev *e = &g2_ev[k];
        if (e->t == 'C' || g2_ishw(e->addr)) continue;
        for (i = 0; i < e->size; i++) {
            uint32_t b = (e->addr + i) & (G2_MEM - 1);
            uint8_t byte = (uint8_t)(e->val >> (8 * i));
            if (e->t == 'L') {
                if (g2_known[b] != g2_cur && g2_exp[b] != g2_cur) { g2_ram[b] = byte; g2_known[b] = g2_cur; }
            } else {
                if (g2_exp[b] != g2_cur) { g2_exp[b] = g2_cur; g2_xlog[g2_nx++] = b; }
                g2_expval[b] = byte;
            }
        }
    }
}
static int g2_finish(int has, uint32_t got, int ret_any, uint32_t want)
{
    unsigned k;
    if (!g2_fail) g2_skip();
    if (g2_fail || g2_pos != g2_nev) return 0;
    if (has && !ret_any && got != want) return 0;
    if (g2_nw != g2_nx) return 0;
    for (k = 0; k < g2_nw; k++)
        if (g2_exp[g2_wlog[k]] != g2_cur || g2_ram[g2_wlog[k]] != g2_expval[g2_wlog[k]]) return 0;
    return 1;
}
#ifndef G2_BATCH_NAME
#define G2_BATCH_NAME "gpu2"
#endif
/* entry dispatcher supplied by the batch file: returns 1 with *ret set, 0 when the entry has no return value,
 * -1 for an unknown entry */
static int g2_entry(uint32_t entry, const uint32_t *a, uint32_t *ret);
/* Entries this build selects (compiled in).  Lines of other entries are not checked ("not selected"); every selected
 * entry must have at least one case, and a build that selects nothing fails. */
static unsigned g2_selected_list(uint32_t *out);
static int g2_argc;
static char **g2_argv;
#ifdef G2_HAVE_EXTRA
/* batch-specific line format (type token `X`): returns 1 = case ok, 0 = mismatch; `case_no` is the stamp */
static int g2_extra(char **tok, int nt, uint32_t case_no, const char **why);
#endif

static struct { uint32_t entry; unsigned cases, bad, lenient; } g2_stat[64];
static unsigned g2_nstat, g2_skipped;
static sigjmp_buf g2_hang;
static void g2_on_alarm(int sig) { (void)sig; siglongjmp(g2_hang, 1); }
static int g2_main(int c, char **v)
{
    FILE *f; static char line[1 << 23]; unsigned n = 0, nbad = 0;
    g2_argc = c; g2_argv = v;
    g2_lenient_mode = getenv("G2_LENIENT") != NULL;
    if (c < 2 || !(f = fopen(v[1], "r"))) { fprintf(stderr, "usage: %s vectors.txt [extra files]\n", v[0]); return 2; }
    while (fgets(line, sizeof line, f)) {
        static char *tok[G2_MAXEV + 4096]; static char copy[1 << 23];
        int nt = 0, ret_any; uint32_t a[4], entry, got = 0, want; int has, ok; unsigned i;
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < G2_MAXEV + 4096 && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
#ifdef G2_HAVE_EXTRA
        if (nt >= 4 && !strcmp(tok[2], "X")) {
            const char *why = "";
            int xok;
            {
                uint32_t xs[64], xe0 = (uint32_t)strtoul(tok[0], 0, 16); unsigned xi, xn = g2_selected_list(xs), xsel = 0;
                for (xi = 0; xi < xn; xi++) if (xs[xi] == xe0) xsel = 1;
                n++;
                if (!xsel) { g2_skipped++; continue; }
            }
            xok = g2_extra(tok, nt, n, &why);
            {
                unsigned si; uint32_t xe = (uint32_t)strtoul(tok[0], 0, 16);
                for (si = 0; si < g2_nstat && g2_stat[si].entry != xe; si++) ;
                if (si == g2_nstat && g2_nstat < 64) { g2_stat[si].entry = xe; g2_nstat++; }
                g2_stat[si].cases++;
                if (!xok) g2_stat[si].bad++;
            }
            if (!xok && nbad++ < (getenv("G2_VERBOSE") ? 100000u : 12u)) fprintf(stderr, "MISMATCH line %u: %s: %.120s\n", n, why, line);
            continue;
        }
#endif
        if (nt < 11 || strcmp(tok[2], "A") || strcmp(tok[7], "R") || strcmp(tok[9], "RET") || strcmp(tok[11], "EV")) continue;
        entry = (uint32_t)strtoul(tok[0], 0, 16);
        for (i = 0; i < 4; i++) a[i] = (uint32_t)strtoul(tok[3 + i], 0, 16);
        g2_nrets = 0;
        if (strcmp(tok[8], "-")) {
            char *s = tok[8], *e, *eq;
            while (*s && g2_nrets < 16) {
                char *p, *q;
                e = strchr(s, ';'); if (e) *e = 0;
                eq = strchr(s, '='); *eq = 0;
                strncpy(g2_rets[g2_nrets].name, s, 23); g2_rets[g2_nrets].n = g2_rets[g2_nrets].used = 0;
                p = eq + 1;
                while (*p && g2_rets[g2_nrets].n < 64) {
                    g2_rets[g2_nrets].v[g2_rets[g2_nrets].n++] = (uint32_t)strtoul(p, &q, 16);
                    p = *q == ',' ? q + 1 : q;
                }
                g2_nrets++;
                if (!e) break;
                s = e + 1;
            }
        }
        ret_any = tok[10][0] == '_';
        want = ret_any ? 0 : (uint32_t)strtoul(tok[10], 0, 16);
        g2_parse_events(tok, nt, 12);
        n++; g2_cur = n; g2_pos = 0; g2_fail = 0; g2_msg[0] = 0; g2_nw = 0; g2_extra_read = 0;
        g2_prepare();
        {
            struct itimerval it; volatile int hung = 0;
            memset(&it, 0, sizeof it);
            signal(SIGALRM, g2_on_alarm);
            if (sigsetjmp(g2_hang, 1)) { hung = 1; has = 1; }
            else {
                it.it_value.tv_usec = 60000;
                setitimer(ITIMER_REAL, &it, NULL);
                has = g2_entry(entry, a, &got);
            }
            memset(&it, 0, sizeof it);
            setitimer(ITIMER_REAL, &it, NULL);
            if (hung) { g2_bad("HANG (provider did not return within 60 ms)", 0, 0); got = want ^ 1u; }
        }
#if defined(GPU2_MUTANT) && GPU2_MUTANT == 2
        got ^= 1u;
#endif
        if (has < 0 && has != -2) { fprintf(stderr, "unknown entry %08X\n", entry); return 2; }
        if (has == -2) { g2_skipped++; continue; }
        ok = g2_finish(has, got, ret_any, want);
        {
            unsigned si;
            for (si = 0; si < g2_nstat && g2_stat[si].entry != entry; si++) ;
            if (si == g2_nstat && g2_nstat < 64) { g2_stat[si].entry = entry; g2_nstat++; }
            g2_stat[si].cases++;
            if (ok && g2_extra_read) g2_stat[si].lenient++;   /* G2_LENIENT: passes only with tolerated extra reads */
            if (!ok) g2_stat[si].bad++;
        }
        if (!ok && nbad++ < (getenv("G2_VERBOSE") ? 100000u : 12u))
            fprintf(stderr, "MISMATCH line %u: %s%s ret %X/%X consumed %u/%u written %u/%u: %.160s\n", n, g2_msg,
                    g2_pos != g2_nev ? " [events left]" : "", got, want, g2_pos, g2_nev, g2_nw, g2_nx, line);
    }
    {
        unsigned si, k, nsel; uint32_t sel[64]; int missing = 0;
        for (si = 0; si < g2_nstat; si++)
            if (g2_lenient_mode)
                printf("  %08X: %u cases, %u mismatches (other than reads outside the original's) + %u cases passing only because those reads are tolerated\n",
                       g2_stat[si].entry, g2_stat[si].cases, g2_stat[si].bad, g2_stat[si].lenient);
            else
                printf("  %08X: %u cases, %u mismatches\n", g2_stat[si].entry, g2_stat[si].cases, g2_stat[si].bad);
        nsel = g2_selected_list(sel);
        if (getenv("G2_ENTRIES") && *getenv("G2_ENTRIES")) {                      /* restrict the required entries (e.g. split vector files) */
            unsigned kk, m = 0;
            for (kk = 0; kk < nsel; kk++) {
                char hex[16]; snprintf(hex, sizeof hex, "%08X", sel[kk]);
                if (strstr(getenv("G2_ENTRIES"), hex)) sel[m++] = sel[kk];
            }
            nsel = m;
        }
        for (k = 0; k < nsel; k++) {
            for (si = 0; si < g2_nstat && g2_stat[si].entry != sel[k]; si++) ;
            if (si == g2_nstat || !g2_stat[si].cases) { printf("  %08X: SELECTED BUT NO CASE IN THE VECTOR FILE\n", sel[k]); missing = 1; }
        }
        printf("%s checked %u cases, %u mismatches%s, %u lines of non-selected entries ignored, %u selected entries\n", G2_BATCH_NAME,
               n - g2_skipped, nbad, g2_lenient_mode ? " (LENIENT: extra RAM reads tolerated, informational only)" : "", g2_skipped, nsel);
        if (nsel == 0 || n - g2_skipped == 0) { printf("FAIL: no entry selected / no case checked (build selects nothing)\n"); return 1; }
        if (missing) return 1;
    }
    return nbad != 0;
}
