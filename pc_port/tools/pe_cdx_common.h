/* Shared scaffold for pe_cdx_cdgen4{2,3,4,5}_check.c (vectors from
 * pe_cdx_vectors.py / pe_cdx_irq_vectors.py).  Header only; each checker
 * includes it, defines the entry dispatcher call() and the stand-ins for the
 * symbols its linked provider objects leave undefined.
 *
 * Black-box contract (what a case checks):
 *   - the return value (when the entry returns one);
 *   - the final guest RAM image: every byte the original's stores wrote must
 *     hold the original's final value, and no other byte of the 2 MiB RAM
 *     may differ from the seeded image (a full diff, so writes through
 *     PE_Translate pointers are caught as well as PE_StoreU*), except the
 *     native's own guest-frame arena;
 *   - register (0x1F80xxxx) stores, in the original's order, width and value;
 *   - register reads: replayed in the original's order (same address and
 *     width) -- hardware inputs; reading more, fewer or other registers fails;
 *   - callee calls recorded by the stand-ins: same callees, same order, same
 *     arguments (only those the stand-in declares), same number of distinct
 *     bytes written before each call (the "@n" suffix); callee return values
 *     are the scripted ones (RETS fn=val,...);
 *   - plain RAM reads must hit bytes the original also read (L list) or
 *     bytes the code under test wrote earlier (bad_read otherwise).
 * Runtime modes (argv[2]): none = check the native; "noop" = replace the
 * native by a stub that does nothing and returns 0 (control: must FAIL);
 * "flip" = run the native, then corrupt its return value / its last
 * guest write by one bit (control: must FAIL).  Both controls print the same
 * per-entry table.
 * Pointers into the caller's own stack frame are compared as offsets from the
 * case's first such pointer ("F+n"): the original's lie in 0x801FE000..FFF,
 * the native's in the guest-frame arena. */
#ifndef PE_CDX_COMMON_H
#define PE_CDX_COMMON_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include "pe_guest_ram.h"
#define MEM 0x200000u
#define MAXTOK 16384
static uint8_t ram[MEM], snap[MEM], expect[MEM];
static uint32_t readable[MEM], wrote[MEM], cur;
static uint32_t wlog[8192], nwlog;
static int bad_read, bad_call; static const char *trap_name;
static unsigned long runaway;
static jmp_buf case_escape; static int escape_armed;
static int in_frame(pe_addr_t a) { return a >= PE_GUEST_FRAME_LO && a < PE_GUEST_FRAME_HI; }
static uint32_t mlog[1024][3]; static unsigned nmlog;
static uint32_t rq[16384][3]; static unsigned nrq, rq_pos;
static uint32_t last_w_addr; static unsigned last_w_size;
static int is_reg(pe_addr_t a) { return (a >> 16) == 0x1F80u; }
static int mmio_read(pe_addr_t a, unsigned n, uint32_t *v)
{
    if (!is_reg(a)) return 0;
    if (rq_pos >= nrq || rq[rq_pos][0] != a || rq[rq_pos][1] != n) {
        bad_read = 1; *v = 0;
        if (escape_armed && ++runaway > 1000000ul) longjmp(case_escape, 1);
        return 1;
    }
    *v = rq[rq_pos++][2];
    return 1;
}
static void rd(pe_addr_t a, unsigned n)
{
    unsigned j;
    if (in_frame(a)) return;
    for (j = 0; j < n; j++) if (readable[(a + j) & (MEM - 1)] != cur) bad_read = 1;
}
static void wr(pe_addr_t a, unsigned n)
{
    unsigned i;
    if (in_frame(a)) { for (i = 0; i < n; i++) readable[(a + i) & (MEM - 1)] = cur; return; }
    last_w_addr = a; last_w_size = n;
    for (i = 0; i < n; i++) {
        uint32_t b = (a + i) & (MEM - 1);
        readable[b] = cur;
        if (wrote[b] != cur) { wrote[b] = cur; if (nwlog < 8192) wlog[nwlog++] = b; }
    }
}
static void mmio_store(pe_addr_t a, unsigned n, uint32_t v)
{
    if (is_reg(a) && nmlog < 1024) { mlog[nmlog][0] = a; mlog[nmlog][1] = n; mlog[nmlog][2] = v; nmlog++; }
}
uint8_t PE_LoadU8(pe_addr_t a) { uint32_t m; if (mmio_read(a, 1, &m)) return (uint8_t)m; rd(a, 1); return ram[a & (MEM - 1)]; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; uint32_t m; if (mmio_read(a, 2, &m)) return (uint16_t)m; rd(a, 2); memcpy(&v, ram + (a & (MEM - 1)), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; if (mmio_read(a, 4, &v)) return v; rd(a, 4); memcpy(&v, ram + (a & (MEM - 1)), 4); return v; }
void PE_StoreU8(pe_addr_t a, uint8_t v) { mmio_store(a, 1, v); wr(a, 1); ram[a & (MEM - 1)] = v; }
void PE_StoreU16(pe_addr_t a, uint16_t v) { mmio_store(a, 2, v); wr(a, 2); memcpy(ram + (a & (MEM - 1)), &v, 2); }
void PE_StoreU32(pe_addr_t a, uint32_t v) { mmio_store(a, 4, v); wr(a, 4); memcpy(ram + (a & (MEM - 1)), &v, 4); }
/* Translated pointers bypass the logs; the full-RAM diff covers their writes. */
void *PE_Translate(pe_addr_t a, size_t size) { (void)size; return ram + (a & (MEM - 1)); }
static pe_addr_t frame_top = PE_GUEST_FRAME_HI;
pe_addr_t PE_GuestFrame_Enter(uint32_t size)
{
    frame_top = (frame_top - size) & ~7u;
    if (frame_top < PE_GUEST_FRAME_LO) bad_read = 1;
    return frame_top;
}
void PE_GuestFrame_Leave(pe_addr_t base, uint32_t size) { (void)size; frame_top = base; }

/* ---- callee stand-ins ------------------------------------------------ */
#define MAX_CALLS 2048
static char got_calls[MAX_CALLS][256]; static unsigned ngot;
static uint32_t rets_fn[64], rets_val[64]; static unsigned nrets_t;
static uint32_t want_fbase, got_fbase; static int want_fset, got_fset;
static uint32_t ret_of(uint32_t fn)
{
    unsigned i;
    for (i = 0; i < nrets_t; i++) if (rets_fn[i] == fn) return rets_val[i];
    return 0;
}
static void fmt_got(char *o, size_t n, uint32_t v)
{
    if (v >= PE_GUEST_FRAME_LO && v < PE_GUEST_FRAME_HI) {
        if (!got_fset) { got_fbase = v; got_fset = 1; }
        snprintf(o, n, "F%+d", (int)(v - got_fbase));
    } else snprintf(o, n, "%08X", v);
}
static void fmt_want(char *o, size_t n, uint32_t v)
{
    if (v >= 0x801FE000u && v < 0x801FF000u) {
        if (!want_fset) { want_fbase = v; want_fset = 1; }
        snprintf(o, n, "F%+d", (int)(v - want_fbase));
    } else snprintf(o, n, "%08X", v);
}
/* Record a call to callee `fn` taking `n` guest arguments; return its scripted result. */
static uint32_t cdx_callee(uint32_t fn, unsigned n, uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4)
{
    char t[200], p[24]; uint32_t a[5]; unsigned i; char sfx[24];
    a[0] = a0; a[1] = a1; a[2] = a2; a[3] = a3; a[4] = a4;
    snprintf(t, sizeof t, "%08X", fn);
    for (i = 0; i < n && i < 5; i++) {
        fmt_got(p, sizeof p, a[i]);
        strncat(t, ",", sizeof t - strlen(t) - 1);
        strncat(t, p, sizeof t - strlen(t) - 1);
    }
    snprintf(sfx, sizeof sfx, "@%u", nwlog);
    if (ngot < MAX_CALLS) { memcpy(got_calls[ngot], t, strlen(t) + 1); strncat(got_calls[ngot], sfx, sizeof got_calls[ngot] - strlen(t) - 1); }
    ngot++;
    return ret_of(fn);
}
#define CALLEE(sym, fn, n) \
    uint32_t sym(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e) \
    { return cdx_callee(fn, n, (uint32_t)a, (uint32_t)b, (uint32_t)c, (uint32_t)d, (uint32_t)e); }
/* A symbol the code under test must not reach: a call fails the case. */
#define TRAP(sym) \
    uint32_t sym(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e) \
    { (void)a; (void)b; (void)c; (void)d; (void)e; bad_call = 1; trap_name = #sym; return 0; }

/* A host-only effect with no retail counterpart (a host stream feed closing, ...): counted, not compared. */
static unsigned host_only_calls;
#define IGNORE(sym) \
    uint32_t sym(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e) \
    { (void)a; (void)b; (void)c; (void)d; (void)e; host_only_calls++; return 0; }

static int call(uint32_t entry, const uint32_t *a, uint32_t *ret);   /* batch dispatcher: 1 = returns a value, 0 = void, -1 unknown */
static int mutant;   /* 0 native, 1 noop, 2 flip */

static int find(char **tok, int nt, int from, const char *s)
{
    while (from < nt && strcmp(tok[from], s)) from++;
    return from;
}
static void apply_seed(const char *t)
{
    unsigned addr, size, val, i;
    if (sscanf(t, "%x:%u=%x", &addr, &size, &val) != 3 || size == 0 || size > 4) return;
    for (i = 0; i < size; i++) {
        uint32_t b = (addr + i) & (MEM - 1);
        if (readable[b] == cur) continue;   /* first value read wins */
        ram[b] = (uint8_t)(val >> (8 * i)); readable[b] = cur;
    }
}
static unsigned entries_seen[256]; static uint32_t entry_id[256]; static unsigned entry_bad[256]; static unsigned nentry;
static unsigned entry_slot(uint32_t e)
{
    unsigned i;
    for (i = 0; i < nentry; i++) if (entry_id[i] == e) return i;
    entry_id[nentry] = e; return nentry++;
}
int main(int argc, char **argv)
{
    FILE *f; static char line[1 << 20], copy[1 << 20]; static char *tok[MAXTOK];
    unsigned n = 0, bad = 0, i;
    if (argc < 2 || !(f = fopen(argv[1], "r"))) { fprintf(stderr, "usage: check vectors.txt [noop|flip]\n"); return 2; }
    if (argc > 2) mutant = !strcmp(argv[2], "noop") ? 1 : !strcmp(argv[2], "flip") ? 2 : 0;
    while (fgets(line, sizeof line, f)) {
        int nt = 0, ri, li, si, ti, ci, ok = 1, na, has; uint32_t entry, a[8] = {0}, wret, got = 0; unsigned k, slot;
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < MAXTOK && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
        if (nt < 8 || strcmp(tok[3], "A")) continue;
        ri = find(tok, nt, 4, "RET"); li = ri + 2; si = find(tok, nt, li, "S"); ti = find(tok, nt, si, "RETS"); ci = find(tok, nt, ti, "CALL");
        if (si >= nt || ti >= nt || ci >= nt || strcmp(tok[li], "L")) continue;
        entry = (uint32_t)strtoul(tok[0], 0, 16);
        na = ri - 4;
        for (i = 0; (int)i < na && i < 8; i++) a[i] = (uint32_t)strtoul(tok[4 + i], 0, 16);
        wret = (uint32_t)strtoul(tok[ri + 1], 0, 16);
        slot = entry_slot(entry); entries_seen[slot]++;
        cur = ++n; nwlog = 0; bad_read = 0; bad_call = 0; nmlog = 0; nrq = rq_pos = 0; ngot = 0; frame_top = PE_GUEST_FRAME_HI;
        want_fset = got_fset = 0;
        memset(ram, 0, sizeof ram);
        for (k = (unsigned)li + 1; k < (unsigned)si; k++) {
            unsigned addr, size, val;
            if (!strcmp(tok[k], "-")) continue;
            if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) == 3 && is_reg(addr)) {
                if (nrq < 16384) { rq[nrq][0] = addr; rq[nrq][1] = size; rq[nrq][2] = val; nrq++; }
                continue;
            }
            apply_seed(tok[k]);
        }
        memcpy(snap, ram, sizeof ram); memcpy(expect, ram, sizeof ram);
        nrets_t = 0;
        if (strcmp(tok[ti + 1], "-"))
            for (k = (unsigned)ti + 1; k < (unsigned)ci; k++) {
                char *e = tok[k], *eq = strchr(e, '=');
                if (eq && nrets_t < 64) { rets_fn[nrets_t] = (uint32_t)strtoul(e, 0, 16); rets_val[nrets_t++] = (uint32_t)strtoul(eq + 1, 0, 16); }
            }
        for (k = (unsigned)si + 1; k < (unsigned)ti; k++) {          /* the original's final stores */
            unsigned addr, size, val, b;
            if (!strcmp(tok[k], "-") || sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) != 3 || size > 4) continue;
            for (b = 0; b < size; b++) expect[(addr + b) & (MEM - 1)] = (uint8_t)(val >> (8 * b));
        }
        runaway = 0;
        if (setjmp(case_escape)) { escape_armed = 0; bad_read = 1; has = 0; got = 0; }
        else { escape_armed = 1; has = call(entry, a, &got); escape_armed = 0; }
        if (has < 0) { fprintf(stderr, "unknown entry %08X\n", entry); return 2; }
        if (mutant == 2) { if (has) got ^= 1u; if (last_w_size) ram[last_w_addr & (MEM - 1)] ^= 1u; else ram[0x1000] ^= 1; }
        if (has && got != wret) ok = 0;
        if (bad_read || bad_call) ok = 0;
        if (rq_pos != nrq) ok = 0;
        {   /* final RAM image (the native's frame arena and the original's stack are out) */
            uint32_t lo = PE_GUEST_FRAME_LO & (MEM - 1), hi = PE_GUEST_FRAME_HI & (MEM - 1);
            for (k = 0; k < MEM && ok; k += 4) {
                if (memcmp(ram + k, expect + k, 4)) {
                    unsigned j;
                    for (j = 0; j < 4; j++)
                        if (ram[k + j] != expect[k + j] && !((k + j) >= lo && (k + j) < hi) &&
                            !((k + j) >= 0x1FE000u && (k + j) < 0x1FF000u)) ok = 0;
                }
            }
        }
        {   /* register stores in order */
            unsigned want_m = 0, addr, size, val;
            for (k = (unsigned)si + 1; k < (unsigned)ti; k++) {
                if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) != 3 || !is_reg(addr)) continue;
                if (want_m >= nmlog || mlog[want_m][0] != addr || mlog[want_m][1] != size ||
                    mlog[want_m][2] != (val & (size == 4 ? 0xFFFFFFFFu : (1u << (8 * size)) - 1u))) ok = 0;
                want_m++;
            }
            if (want_m != nmlog) ok = 0;
        }
        {   /* callee calls */
            unsigned want_c = 0;
            for (k = (unsigned)ci + 1; k < (unsigned)nt; k++) {
                char w[300], tk[260], *at, *p; unsigned cnt = 0, q, b, seen = 0, nst;
                static uint32_t mark[MEM]; static uint32_t gen;
                if (!strcmp(tok[k], "-")) continue;
                strncpy(tk, tok[k], sizeof tk - 1); tk[sizeof tk - 1] = 0;
                at = strchr(tk, '@');
                if (at) {
                    nst = (unsigned)strtoul(at + 1, 0, 10); gen++;
                    for (q = (unsigned)si + 1; q < (unsigned)ti && nst; q++) {
                        unsigned ad, sz, vv;
                        if (sscanf(tok[q], "%x:%u=%x", &ad, &sz, &vv) != 3) continue;
                        if ((ad >= 0x801FE000u && ad < 0x801FF000u)) { nst--; continue; }
                        for (b = 0; b < sz; b++) { uint32_t m = (ad + b) & (MEM - 1); if (mark[m] != gen) { mark[m] = gen; seen++; } }
                        nst--;
                    }
                    cnt = seen; *at = 0;
                }
                w[0] = 0;
                for (p = strtok(tk, ","); p; p = strtok(NULL, ",")) {
                    char o[24];
                    if (w[0]) strncat(w, ",", sizeof w - strlen(w) - 1);
                    if (!w[0] && p == tk) snprintf(o, sizeof o, "%08X", (unsigned)strtoul(p, 0, 16));
                    else fmt_want(o, sizeof o, (uint32_t)strtoul(p, 0, 16));
                    strncat(w, o, sizeof w - strlen(w) - 1);
                }
                { char sfx[24]; snprintf(sfx, sizeof sfx, "@%u", cnt); strncat(w, sfx, sizeof w - strlen(w) - 1); }
                if (want_c >= ngot || (want_c < MAX_CALLS && strcmp(got_calls[want_c], w))) ok = 0;
                want_c++;
            }
            if (want_c != ngot) ok = 0;
        }
        if (!ok) {
            entry_bad[slot]++;
            if (bad++ < 8 && !mutant) {
                unsigned q;
                fprintf(stderr, "MISMATCH line %u entry %08X (%s%s%s) %.160s\n", n, entry, bad_read ? "read outside original loads; " : "",
                        bad_call ? "unexpected call; " : "", "stores/return/calls/regs", line);
                if (bad_call) fprintf(stderr, "  trap: %s\n", trap_name ? trap_name : "?");
                { const char *cp = strstr(line, " CALL "); if (cp) fprintf(stderr, "  want%.300s\n", cp + 5); }
                if (getenv("CDX_DEBUG")) {
                    unsigned q;
                    fprintf(stderr, "  native wrote bytes:");
                    for (q = 0; q < nwlog && q < 48; q++) fprintf(stderr, " %05X=%02X", wlog[q], ram[wlog[q]]);
                    fprintf(stderr, "\n  original S: ");
                    for (q = (unsigned)si + 1; q < (unsigned)ti && q < (unsigned)si + 40; q++) fprintf(stderr, "%s ", tok[q]);
                    fprintf(stderr, "\n");
                }
                for (q = 0; q < ngot && q < 6 && q < MAX_CALLS; q++) fprintf(stderr, "  got %s\n", got_calls[q]);
            }
        }
    }
    printf("mode=%s checked %u cases, %u mismatches (host-only calls ignored: %u)\n", mutant == 1 ? "noop" : mutant == 2 ? "flip" : "native", n, bad, host_only_calls);
    for (i = 0; i < nentry; i++) printf("  %08X cases=%u mismatches=%u\n", entry_id[i], entries_seen[i], entry_bad[i]);
    return mutant ? 0 : bad != 0;
}
#endif
