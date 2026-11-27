/* Shared scaffold of the gte60 vector checkers (pe_gte60_a_check.c,
 * pe_gte60_b_check.c, pe_gte60_c_check.c).  Test tooling only; included
 * once per checker, so everything is static.
 *
 * Vector line (pc_port/tools/pe_gte60_vectors.py):
 *   NAME seed pattern SRC A <args|-> RET <ret,...|-> IN <64 hex> OUT <64 hex> F <0|1> L <loads|-> S <stores|->
 *   IN   values written into the GTE in index order (0-31 PE_GTE_WriteData,
 *        32-63 PE_GTE_WriteCtrl) before the call under test
 *   OUT  what PE_GTE_ReadData(0..31) / PE_GTE_ReadCtrl(0..31) must return
 *        after the call; OUT[63] (FLAG) is compared only when F is 1
 *   L    guest bytes the code under test may read (first load wins; a
 *        trailing ^ marks a load that followed the original's own store and
 *        is not pre-seeded); any other guest read fails the case
 *   S    the stores the original made; the set of guest bytes written and
 *        their final values must equal them (no S means no store at all)
 * Guest RAM is a flat 2 MiB array (address & 0x1FFFFF); the GTE is the
 * provider's own (g_pe_gte behind PE_GTE_*).
 *
 * Mutant controls: -DGTE60_MUTANT=1 clears FLAG bit 31 of every
 * ReadCtrl(31); =2 drops the sign extension of ReadCtrl(4); =3 skips the
 * call under test; =4 is a batch-specific wrong call (see the checker).
 * A mutant build must FAIL (exit 1). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include "pe_sdk.h"

#ifndef GTE60_MUTANT
#define GTE60_MUTANT 0
#endif

typedef char gte60_state_size_is_232[(sizeof(PeGteState) == 232) ? 1 : -1];

#define MEM 0x200000u
#define MAXTOK 2048
static uint8_t ram[MEM], want[MEM];
static uint32_t readable[MEM], wrote[MEM], want_set[MEM], cur;
static uint32_t wlog[4096], nwlog, xlog[4096], nxlog;
static int bad_read;

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
        if (wrote[b] != cur) { wrote[b] = cur; if (nwlog < 4096) wlog[nwlog++] = b; }
        readable[b] = cur;
    }
}
uint8_t PE_LoadU8(pe_addr_t a) { rd(a, 1); return ram[a & (MEM - 1)]; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; rd(a, 2); memcpy(&v, ram + (a & (MEM - 1)), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; rd(a, 4); memcpy(&v, ram + (a & (MEM - 1)), 4); return v; }
void PE_StoreU8(pe_addr_t a, uint8_t v) { wr(a, 1); ram[a & (MEM - 1)] = v; }
void PE_StoreU16(pe_addr_t a, uint16_t v) { wr(a, 2); memcpy(ram + (a & (MEM - 1)), &v, 2); }
void PE_StoreU32(pe_addr_t a, uint32_t v) { wr(a, 4); memcpy(ram + (a & (MEM - 1)), &v, 4); }

/* A buffer that ends at a page boundary with a no-access page behind it, so
 * a read past the documented extent of a host array faults. */
static void on_segv(int sig)
{
    static const char msg[] = "READ OVERRUN of a host array (guard page hit)\n";
    (void)sig;
    if (write(2, msg, sizeof msg - 1) < 0) _exit(4);
    _exit(3);
}
static void __attribute__((unused)) *guard_buf(size_t bytes)
{
    static long pg;
    uint8_t *p;
    if (!pg) { pg = sysconf(_SC_PAGESIZE); signal(SIGSEGV, on_segv); }
    p = mmap(NULL, 2 * (size_t)pg, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) { perror("mmap"); exit(2); }
    mprotect(p + pg, (size_t)pg, PROT_NONE);
    return p + pg - bytes;
}

/* GTE register access through the provider (with the mutant controls). */
static uint32_t g_rd(unsigned i)
{
    uint32_t v = PE_GTE_ReadData(i);
    return v;
}
static uint32_t g_rc(unsigned i)
{
    uint32_t v = PE_GTE_ReadCtrl(i);
#if GTE60_MUTANT == 1
    if (i == 31) v &= 0x7FFFFFFFu;
#elif GTE60_MUTANT == 2
    if (i == 4) v &= 0xFFFFu;
#endif
    return v;
}
static void snapshot(uint32_t *s)
{
    unsigned i;
    for (i = 0; i < 32; i++) s[i] = g_rd(i);
    for (i = 0; i < 32; i++) s[32 + i] = g_rc(i);
}
static void load_in(const uint32_t *in)
{
    unsigned i;
    for (i = 0; i < 32; i++) PE_GTE_WriteData(i, in[i]);
    for (i = 0; i < 32; i++) PE_GTE_WriteCtrl(i, in[32 + i]);
}

/* ---- parsed line ----------------------------------------------------- */
typedef struct {
    char name[64];
    unsigned seed, pat;
    char src[8];
    uint32_t a[16]; int na;
    uint32_t ret[8]; int nret;
    uint32_t in[64], out[64];
    int fv;
    char *loads[256]; int nloads;
    char *stores[256]; int nstores;
} Case;

static int find_tok(char **tok, int nt, int from, const char *s)
{
    while (from < nt && strcmp(tok[from], s)) from++;
    return from;
}
static int parse_case(char *copy, Case *c)
{
    static char *tok[MAXTOK];
    int nt = 0, i, ri, ii, oi, fi, li, si;
    char *rest;
    char *t;
    while (nt < MAXTOK && (t = strtok(nt ? NULL : copy, " \n"))) tok[nt++] = t;
    if (nt < 12 || strcmp(tok[4], "A")) return 0;
    ri = find_tok(tok, nt, 5, "RET"); ii = find_tok(tok, nt, ri, "IN"); oi = find_tok(tok, nt, ii, "OUT");
    fi = oi + 65;       /* "F" is also a hex value: locate it by position */
    if (fi + 2 >= nt || strcmp(tok[fi], "F") || strcmp(tok[fi + 2], "L")) return 0;
    li = fi + 2; si = find_tok(tok, nt, li, "S");
    if (si >= nt || oi - ii != 65) return 0;
    snprintf(c->name, sizeof c->name, "%s", tok[0]);
    c->seed = (unsigned)strtoul(tok[1], 0, 10); c->pat = (unsigned)strtoul(tok[2], 0, 10);
    snprintf(c->src, sizeof c->src, "%s", tok[3]);
    c->na = 0;
    for (i = 5; i < ri && c->na < 16; i++) if (strcmp(tok[i], "-")) c->a[c->na++] = (uint32_t)strtoul(tok[i], 0, 16);
    c->nret = 0;
    if (strcmp(tok[ri + 1], "-")) {
        for (rest = tok[ri + 1]; *rest && c->nret < 8; ) {
            c->ret[c->nret++] = (uint32_t)strtoul(rest, &rest, 16);
            if (*rest == ',') rest++;
        }
    }
    for (i = 0; i < 64; i++) { c->in[i] = (uint32_t)strtoul(tok[ii + 1 + i], 0, 16); c->out[i] = (uint32_t)strtoul(tok[oi + 1 + i], 0, 16); }
    c->fv = atoi(tok[fi + 1]);
    c->nloads = c->nstores = 0;
    for (i = li + 1; i < si && c->nloads < 256; i++) c->loads[c->nloads++] = tok[i];
    for (i = si + 1; i < nt && c->nstores < 256; i++) c->stores[c->nstores++] = tok[i];
    return 1;
}
static int apply_mem(const char *t, uint8_t *dst, uint32_t *set, int first_only)
{
    unsigned addr, size, val, i;
    if (!strcmp(t, "-")) return 1;
    if (sscanf(t, "%x:%u=%x", &addr, &size, &val) != 3 || size == 0 || size > 4) return 0;
    for (i = 0; i < size; i++) {
        uint32_t b = (addr + i) & (MEM - 1);
        if (set[b] == cur) { if (first_only) continue; }
        else if (!first_only && nxlog < 4096) xlog[nxlog++] = b;
        dst[b] = (uint8_t)(val >> (8 * i));
        set[b] = cur;
    }
    return 1;
}
static void mem_begin(const Case *c)
{
    int k;
    nwlog = nxlog = 0; bad_read = 0;
    for (k = 0; k < c->nloads; k++) if (!strchr(c->loads[k], '^')) apply_mem(c->loads[k], ram, readable, 1);
    for (k = 0; k < c->nstores; k++) apply_mem(c->stores[k], want, want_set, 0);
}
static int mem_ok(void)
{
    unsigned k;
    if (bad_read) return 0;
    if (nwlog != nxlog) return 0;
    for (k = 0; k < nwlog; k++)
        if (want_set[wlog[k]] != cur || ram[wlog[k]] != want[wlog[k]]) return 0;
    return 1;
}

static unsigned g_cases, g_bad, g_shown;
static unsigned by_name_n[64], by_name_bad[64];
static char by_name[64][64];
static void count_case(const char *name, int ok)
{
    int i;
    for (i = 0; i < 64 && by_name[i][0]; i++) if (!strcmp(by_name[i], name)) break;
    if (i == 64) return;
    if (!by_name[i][0]) snprintf(by_name[i], 64, "%s", name);
    by_name_n[i]++; if (!ok) by_name_bad[i]++;
    g_cases++; if (!ok) g_bad++;
}
static int compare_regs(const Case *c, const uint32_t *got, char *why, size_t wn)
{
    int i, ok = 1;
    for (i = 0; i < 64; i++) {
        if (i == 63 && !c->fv) continue;
        if (got[i] != c->out[i]) {
            if (ok) snprintf(why, wn, "%c%d got %X want %X", i < 32 ? 'd' : 'c', i & 31, got[i], c->out[i]);
            ok = 0;
        }
    }
    return ok;
}
static void report_fail(const Case *c, const char *why)
{
    if (g_shown++ < 10)
        fprintf(stderr, "MISMATCH %s seed %u pat %u src %s: %s\n", c->name, c->seed, c->pat, c->src, why);
}
static int finish(const char *label)
{
    int i;
    for (i = 0; i < 64 && by_name[i][0]; i++)
        printf("  %-34s %6u cases %6u mismatches\n", by_name[i], by_name_n[i], by_name_bad[i]);
    printf("%s: checked %u cases, %u mismatches\n", label, g_cases, g_bad);
    return g_bad != 0;
}

/* ---- g_pe_gte field view (layout documented in pe_sdk.h) -------------- */
static void __attribute__((unused)) fields_from_reads(const uint32_t *o, PeGteState *g)
{
    int i, n;
    memset(g, 0, sizeof *g);
    for (i = 0; i < 3; i++) {
        int16_t *v = i == 0 ? g->v0 : i == 1 ? g->v1 : g->v2;
        v[0] = (int16_t)o[2 * i]; v[1] = (int16_t)(o[2 * i] >> 16); v[2] = (int16_t)o[2 * i + 1];
    }
    g->rgbc = o[6]; g->otz = o[7]; g->ir0 = (int16_t)o[8];
    for (i = 0; i < 3; i++) {
        g->ir[i] = (int16_t)o[9 + i]; g->sxy[i] = o[12 + i]; g->rgb_fifo[i] = o[20 + i];
        g->mac[i] = (int32_t)o[25 + i];
        g->tr[i] = (int32_t)o[32 + 5 + i]; g->bk[i] = (int32_t)o[32 + 13 + i]; g->fc[i] = (int32_t)o[32 + 21 + i];
    }
    for (i = 0; i < 4; i++) g->sz[i] = o[16 + i];
    g->mac0 = (int32_t)o[24]; g->lzcs = o[30]; g->lzcr = o[31];
    for (n = 0; n < 9; n++) {
        g->rt[n / 3][n % 3] = (int16_t)(o[32 + n / 2] >> (16 * (n % 2)));
        g->llm[n / 3][n % 3] = (int16_t)(o[32 + 8 + n / 2] >> (16 * (n % 2)));
        g->lcm[n / 3][n % 3] = (int16_t)(o[32 + 16 + n / 2] >> (16 * (n % 2)));
    }
    g->ofx = (int32_t)o[32 + 24]; g->ofy = (int32_t)o[32 + 25];
    g->h = (int16_t)o[32 + 26]; g->dqa = (int16_t)o[32 + 27]; g->dqb = (int32_t)o[32 + 28];
    g->zsf3 = (int16_t)o[32 + 29]; g->zsf4 = (int16_t)o[32 + 30];
}
/* Field-by-field check against a canonical read vector: 16-bit-backed
 * fields compare their low 16 bits only (the container width is the
 * provider's choice), 32-bit fields compare exactly; lzcr and
 * projection_flags are not compared. */
static int __attribute__((unused)) fields_match(const uint32_t *o, const PeGteState *g, char *why, size_t wn)
{
    PeGteState e;
    int i, n, ok = 1;
#define F32(f, ef, name) if ((uint32_t)(f) != (uint32_t)(ef)) { ok = 0; snprintf(why, wn, "field %s", name); }
#define H16(f, ef, name) if (((uint32_t)(f) & 0xFFFFu) != ((uint32_t)(ef) & 0xFFFFu)) { ok = 0; snprintf(why, wn, "field %s", name); }
    fields_from_reads(o, &e);
    for (i = 0; i < 3; i++) {
        const int16_t *a = i == 0 ? g->v0 : i == 1 ? g->v1 : g->v2, *b = i == 0 ? e.v0 : i == 1 ? e.v1 : e.v2;
        for (n = 0; n < 3; n++) H16(a[n], b[n], "v");
        H16(g->ir[i], e.ir[i], "ir"); F32(g->sxy[i], e.sxy[i], "sxy"); F32(g->rgb_fifo[i], e.rgb_fifo[i], "rgb_fifo");
        F32(g->mac[i], e.mac[i], "mac"); F32(g->tr[i], e.tr[i], "tr"); F32(g->bk[i], e.bk[i], "bk"); F32(g->fc[i], e.fc[i], "fc");
    }
    for (i = 0; i < 4; i++) H16(g->sz[i], e.sz[i], "sz");
    F32(g->rgbc, e.rgbc, "rgbc"); H16(g->otz, e.otz, "otz"); H16(g->ir0, e.ir0, "ir0"); F32(g->mac0, e.mac0, "mac0");
    F32(g->lzcs, e.lzcs, "lzcs");
    for (n = 0; n < 9; n++) {
        H16(g->rt[n / 3][n % 3], e.rt[n / 3][n % 3], "rt"); H16(g->llm[n / 3][n % 3], e.llm[n / 3][n % 3], "llm");
        H16(g->lcm[n / 3][n % 3], e.lcm[n / 3][n % 3], "lcm");
    }
    F32(g->ofx, e.ofx, "ofx"); F32(g->ofy, e.ofy, "ofy"); H16(g->h, e.h, "h"); H16(g->dqa, e.dqa, "dqa");
    F32(g->dqb, e.dqb, "dqb"); H16(g->zsf3, e.zsf3, "zsf3"); H16(g->zsf4, e.zsf4, "zsf4");
#undef F32
#undef H16
    return ok;
}
