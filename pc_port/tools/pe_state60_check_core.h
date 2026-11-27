/* Shared replay core for the state60 vector checkers (pe_state60_<family>_check.c).
 * A family file defines, before including this header:
 *   static int call(uint32_t entry, const uint32_t *a, uint32_t *ret, int *has_ret, unsigned *ret_width)
 * (runs the candidate provider for one entry), the host/callee recorder
 * symbols the linked provider object needs, and optionally CHECK_EXTRA.
 *
 * Vector line (pc_port/tools/pe_state60_vectors.py):
 *   ENTRY seed pattern A args... RET r L addr:size=value... S addr:size=value... RETS r,... CALL fn,a0,a1,a2,a3@n ...
 *   (a CALL token is fn followed by 1..n argument words and @stores; most callees log four words, a printf callee
 *   logs its format plus only the words it consumes, e.g. fn,fmt,a1@n)
 *
 * Replay rules (stricter than the cd checker; nothing is silently skipped):
 * - RAM starts as poison.  Each byte the original loaded gets the value it
 *   loaded (first load wins); the candidate may read only those bytes.
 * - Register loads (0x1F80xxxx) are replayed in the original's order: same
 *   address and width each, value from the log, and made after exactly the
 *   same number of stores (the token's @k) -- the read/write protocol order.  Extra, missing or reordered
 *   register reads fail the case.
 * - After the call, the set of RAM bytes written and their final values must
 *   equal the original's stores replayed in order; register stores must match
 *   in order (address, width, value).
 * - External callees (CALL list) are recorded by the family file through
 *   state60_note_call(addr, a0..a3); targets, order and arguments must equal
 *   the original's, and the store count at the call (@n) must match.  A call
 *   returns the scripted RETS value.
 * - RET is compared (masked to the return width) when the entry returns a value.
 * - Any malformed line, any buffer overflow and any unknown entry is a
 *   failure; "checked" must equal the number of non-empty lines.
 * Counters: PASS/FAIL per file are printed; exit status 1 on any failure.
 * Environment STATE60_VERBOSE=1 prints the first mismatches in full. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"

#define MEM 0x200000u
#define CAP_M 16384
#define CAP_W 262144
#define CAP_C 256
static uint32_t nst;   /* stores so far in this case (the original's @n counts every logged store) */
static uint8_t ram[MEM], want[MEM];
static uint32_t readable[MEM], wrote[MEM], want_set[MEM], cur;
static uint32_t *wlog, nwlog, *xlog, nxlog;
static int bad_read, overflow;
static uint32_t mlog[CAP_M][3], nmlog;
static uint32_t rq[CAP_M][4], nrq, rq_pos;

static void rd(pe_addr_t a, unsigned n)
{
    unsigned i;
    for (i = 0; i < n; i++)
        if (readable[(a + i) & (MEM - 1)] != cur) bad_read = 1;
}
static void wr(pe_addr_t a, unsigned n)
{
    unsigned i;
    for (i = 0; i < n; i++) {
        uint32_t b = (a + i) & (MEM - 1);
        if (wrote[b] != cur) {
            wrote[b] = cur;
            if (nwlog < CAP_W) wlog[nwlog++] = b; else overflow = 1;
        }
    }
}
static void mmio(pe_addr_t a, unsigned n, uint32_t v)
{
    if ((a >> 16) == 0x1F80u) {
        if (nmlog < CAP_M) { mlog[nmlog][0] = a; mlog[nmlog][1] = n; mlog[nmlog][2] = v; nmlog++; }
        else overflow = 1;
    }
}
static int mmio_read(pe_addr_t a, unsigned n, uint32_t *v)
{
    if ((a >> 16) != 0x1F80u) return 0;
    if (rq_pos >= nrq || rq[rq_pos][0] != a || rq[rq_pos][1] != n || rq[rq_pos][3] != nst) { bad_read = 1; *v = 0; return 1; }
    *v = rq[rq_pos++][2];
    return 1;
}
uint8_t PE_LoadU8(pe_addr_t a) { uint32_t m; if (mmio_read(a, 1, &m)) return (uint8_t)m; rd(a, 1); return ram[a & (MEM - 1)]; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; uint32_t m; if (mmio_read(a, 2, &m)) return (uint16_t)m; rd(a, 2); memcpy(&v, ram + (a & (MEM - 1)), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; if (mmio_read(a, 4, &v)) return v; rd(a, 4); memcpy(&v, ram + (a & (MEM - 1)), 4); return v; }
#ifndef STATE60_MUTANT
#define STATE60_MUTANT 0       /* 1: flip the return's low bit; 2: drop each case's first store; (3 = no-op provider object, link-time) */
#endif
#define DROP_FIRST (STATE60_MUTANT == 2 && nst++ == 0)
void PE_StoreU8(pe_addr_t a, uint8_t v) { if (DROP_FIRST) return; if (STATE60_MUTANT != 2) nst++; mmio(a, 1, v); wr(a, 1); ram[a & (MEM - 1)] = v; }
void PE_StoreU16(pe_addr_t a, uint16_t v) { if (DROP_FIRST) return; if (STATE60_MUTANT != 2) nst++; mmio(a, 2, v); wr(a, 2); memcpy(ram + (a & (MEM - 1)), &v, 2); }
void PE_StoreU32(pe_addr_t a, uint32_t v) { if (DROP_FIRST) return; if (STATE60_MUTANT != 2) nst++; mmio(a, 4, v); wr(a, 4); memcpy(ram + (a & (MEM - 1)), &v, 4); }

/* Callee recorder (called by the family file's stubs for each external callee). */
static uint32_t cg_fn[CAP_C], cg_a[CAP_C][4], cg_stores[CAP_C], ncg;
static uint32_t scripted_ret(uint32_t fn);
static __attribute__((unused)) uint32_t state60_note_call(uint32_t fn, uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    if (ncg < CAP_C) {
        cg_fn[ncg] = fn; cg_a[ncg][0] = a0; cg_a[ncg][1] = a1; cg_a[ncg][2] = a2; cg_a[ncg][3] = a3;
        cg_stores[ncg] = nst;
    } else overflow = 1;
    ncg++;
    return scripted_ret(fn);
}

static uint32_t rets_by_fn[64][2], nrets;            /* (fn, value) from the RETS list */
static uint32_t scripted_ret(uint32_t fn)
{
    unsigned i;
    for (i = 0; i < nrets; i++) if (rets_by_fn[i][0] == fn) return rets_by_fn[i][1];
    return 0;
}

static int apply(const char *t, uint8_t *dst, uint32_t *set, int first_only)
{
    unsigned addr, size, val, i;
    if (sscanf(t, "%x:%u=%x", &addr, &size, &val) != 3 || size == 0 || size > 4) return 0;
    for (i = 0; i < size; i++) {
        uint32_t b = (addr + i) & (MEM - 1);
        if (set[b] == cur) { if (first_only) continue; }
        else if (!first_only) { if (nxlog < CAP_W) xlog[nxlog++] = b; else overflow = 1; }
        dst[b] = (uint8_t)(val >> (8 * i));
        set[b] = cur;
    }
    return 1;
}

static unsigned family_arity(uint32_t fn);   /* register arguments the callee really takes (a1..a3 beyond that are leftovers) */
static int family_call(uint32_t entry, const uint32_t *a, uint32_t *ret, int *has_ret, unsigned *ret_width);
/* Hook for families whose provider reaches hardware through host calls the
 * original did not make: it may flag them (set *bad). */
static unsigned host_calls_unexpected;

/* Parse "fn[,arg...]@n" (hex fields, decimal n).  Returns 1 on success; w4[] gets the first four argument
 * words (zero when absent), *nf the number of argument fields present (any beyond four are accepted and ignored). */
static int parse_call_token(const char *t, unsigned *fn, unsigned *w4, unsigned *nf, unsigned *at)
{
    const char *p = t, *end;
    char *e;
    unsigned long v;
    unsigned n = 0;
    v = strtoul(p, &e, 16);
    if (e == p || *e != ',') return 0;       /* a callee record always carries at least one argument field */
    *fn = (unsigned)v;
    p = e;
    while (*p == ',') {
        p++;
        v = strtoul(p, &e, 16);
        if (e == p) return 0;
        if (n < 4) w4[n] = (unsigned)v;
        n++;
        p = e;
    }
    if (*p != '@') return 0;
    end = p + 1;
    v = strtoul(end, &e, 10);
    if (e == end || *e != '\0') return 0;
    *at = (unsigned)v;
    *nf = n;
    return 1;
}

static int run_file(const char *path, unsigned *n_out, unsigned *bad_out)
{
    FILE *f;
    char *line = NULL; size_t cap = 0; ssize_t len;
    unsigned n = 0, bad = 0, lines = 0;
    int verbose = getenv("STATE60_VERBOSE") != NULL;
    if (!(f = fopen(path, "r"))) return 2;
    wlog = malloc(sizeof(uint32_t) * CAP_W); xlog = malloc(sizeof(uint32_t) * CAP_W);
    if (!wlog || !xlog) return 2;
    while ((len = getline(&line, &cap, f)) > 0) {
        char *tok[65536]; int nt = 0, i, ri, li, si, ci, ok = 1, na, has_ret = 0;
        uint32_t entry, a[8] = {0}, wret, got = 0; unsigned k, rw = 4;
        char *copy = strdup(line), *p;
        int r_i = -1;
        for (p = strtok(copy, " \n"); p && nt < 65536; p = strtok(NULL, " \n")) tok[nt++] = p;
        if (nt == 0) { free(copy); continue; }
        lines++;
        cur = ++n; nst = 0; nwlog = nxlog = 0; bad_read = 0; nmlog = 0; nrq = rq_pos = 0; ncg = 0; overflow = 0;
        nrets = 0; host_calls_unexpected = 0;
        ri = li = si = ci = -1;
        for (i = 4; i < nt; i++) {
            if (ri < 0 && !strcmp(tok[i], "RET")) ri = i;
            else if (ri >= 0 && li < 0 && !strcmp(tok[i], "L")) li = i;
            else if (li >= 0 && si < 0 && !strcmp(tok[i], "S")) si = i;
            else if (si >= 0 && r_i < 0 && !strcmp(tok[i], "RETS")) r_i = i;
            else if (r_i >= 0 && ci < 0 && !strcmp(tok[i], "CALL")) ci = i;
        }
        if (nt < 9 || strcmp(tok[3], "A") || ri < 0 || li != ri + 2 || si < 0 || r_i < 0 || ci < 0) {
            fprintf(stderr, "MALFORMED line %u\n", lines); bad++; free(copy); continue;
        }
        entry = (uint32_t)strtoul(tok[0], 0, 16);
        na = ri - 4;
        for (i = 0; i < na && i < 8; i++) if (strcmp(tok[4 + i], "-")) a[i] = (uint32_t)strtoul(tok[4 + i], 0, 16);
        wret = (uint32_t)strtoul(tok[ri + 1], 0, 16);
        for (k = (unsigned)li + 1; k < (unsigned)si; k++) {
            unsigned addr, size, val, kst = 0;
            if (!strcmp(tok[k], "-")) continue;
            if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) == 3 && (addr >> 16) == 0x1F80u) {
                if (sscanf(tok[k], "%x:%u=%x@%u", &addr, &size, &val, &kst) != 4) { fprintf(stderr, "register load without @k, line %u\n", lines); ok = 0; }
                if (nrq < CAP_M) { rq[nrq][0] = addr; rq[nrq][1] = size; rq[nrq][2] = val; rq[nrq][3] = kst; nrq++; } else overflow = 1;
                continue;
            }
            apply(tok[k], ram, readable, 1);
        }
        for (k = (unsigned)si + 1; k < (unsigned)r_i; k++) if (strcmp(tok[k], "-")) apply(tok[k], want, want_set, 0);
        for (k = (unsigned)r_i + 1; k < (unsigned)ci; k++) {   /* RETS fn=value,fn=value */
            char *c2 = strdup(tok[k]), *q;
            for (q = strtok(c2, ","); q; q = strtok(NULL, ",")) {
                unsigned fn, val;
                if (sscanf(q, "%x=%x", &fn, &val) == 2 && nrets < 64) { rets_by_fn[nrets][0] = fn; rets_by_fn[nrets][1] = val; nrets++; }
            }
            free(c2);
        }
        if (family_call(entry, a, &got, &has_ret, &rw) < 0) {
            fprintf(stderr, "unknown entry %08X\n", entry); free(copy); fclose(f); return 2;
        }
        if (has_ret && STATE60_MUTANT == 1) got ^= 1u;
        if (has_ret) {
            uint32_t m = rw == 4 ? 0xFFFFFFFFu : (1u << (8 * rw)) - 1u;
            if ((got & m) != (wret & m)) { ok = 0; if (verbose) fprintf(stderr, "  ret got %X want %X\n", got, wret); }
        }
        if (bad_read) { ok = 0; if (verbose) fprintf(stderr, "  read outside the original's loads / register read order\n"); }
        if (overflow) { ok = 0; fprintf(stderr, "  OVERFLOW (raise CAP_*)\n"); }
        if (nwlog != nxlog) { ok = 0; if (verbose) fprintf(stderr, "  ram bytes written %u want %u\n", nwlog, nxlog); }
        if (rq_pos != nrq) { ok = 0; if (verbose) fprintf(stderr, "  register reads %u want %u\n", rq_pos, nrq); }
        if (host_calls_unexpected) { ok = 0; if (verbose) fprintf(stderr, "  %u host calls with no original counterpart\n", host_calls_unexpected); }
        {   /* callee calls */
            unsigned wc = 0;
            for (k = (unsigned)ci + 1; k < (unsigned)nt; k++) {
                unsigned fn, at, w4[4] = {0, 0, 0, 0}, nf = 0;
                if (!strcmp(tok[k], "-")) continue;
                /* fn,arg,arg...@n: the field count varies (a printf callee records its format and only the
                 * words it consumes, so 2..4 fields; a callee with stack arguments records more) */
                if (!parse_call_token(tok[k], &fn, w4, &nf, &at)) { ok = 0; continue; }
                if (wc >= ncg || wc >= CAP_C || cg_fn[wc] != fn || cg_stores[wc] != at) ok = 0;
                else {
                    unsigned ar = family_arity(fn);
                    if (ar > nf) ok = 0;      /* the log must carry every argument the checker compares */
                    for (i = 0; i < (int)ar && i < 4; i++) if (cg_a[wc][i] != w4[i]) ok = 0;
                }
                wc++;
            }
            if (wc != ncg) { ok = 0; if (verbose) fprintf(stderr, "  callee calls %u want %u\n", ncg, wc); }
        }
        {   /* register stores in order */
            unsigned want_m = 0, addr, size, val;
            for (k = (unsigned)si + 1; k < (unsigned)r_i; k++) {
                if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) != 3 || (addr >> 16) != 0x1F80u) continue;
                if (want_m >= nmlog || mlog[want_m][0] != addr || mlog[want_m][1] != size ||
                    mlog[want_m][2] != (val & (size == 4 ? 0xFFFFFFFFu : (1u << (8 * size)) - 1u))) {
                    ok = 0;
                    if (verbose && want_m < nmlog) fprintf(stderr, "  reg store %u: got %08X:%u=%X want %08X:%u=%X\n", want_m,
                                                             mlog[want_m][0], mlog[want_m][1], mlog[want_m][2], addr, size, val);
                }
                want_m++;
            }
            if (want_m != nmlog) { ok = 0; if (verbose) fprintf(stderr, "  register stores %u want %u\n", nmlog, want_m); }
        }
        for (k = 0; k < nwlog && ok; k++)
            if (want_set[wlog[k]] != cur || ram[wlog[k]] != want[wlog[k]]) ok = 0;
        if (!ok) { if (bad++ < (verbose ? 12u : 3u)) fprintf(stderr, "MISMATCH line %u: %.220s\n", lines, line); }
        free(copy);
        (void)ri;
    }
    free(line); fclose(f); free(wlog); free(xlog);
    *n_out = n; *bad_out = bad;
    if (n != lines) return 2;
    return 0;
}

int main(int argc, char **argv)
{
    int i, rc = 0; unsigned tot = 0, totbad = 0;
    if (argc < 2) { fprintf(stderr, "usage: %s vectors.txt...\n", argv[0]); return 2; }
    for (i = 1; i < argc; i++) {
        unsigned n = 0, bad = 0;
        int r = run_file(argv[i], &n, &bad);
        if (r == 2) { fprintf(stderr, "ERROR reading/parsing %s\n", argv[i]); rc = 2; }
        printf("%s: checked %u cases, PASS %u, FAIL %u\n", argv[i], n, n - bad, bad);
        tot += n; totbad += bad;
    }
    printf("total: checked %u, PASS %u, FAIL %u\n", tot, tot - totbad, totbad);
    return rc ? rc : (totbad != 0);
}
