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
 * Canary seeding (the default; -DCDX_SEED_ZERO restores the old zero seeding, bit-identical output):
 *   unseeded guest RAM (a byte the case's L list does not seed) starts as the deterministic non-zero canary
 *   canary_byte(b) = 0xC5 ^ ((b ^ (b>>8) ^ (b>>16)) & 0x7F)  (always 0x80..0xC5, varies by address, never 0 / 0xFF),
 *   except the two exempt windows that are never compared (the native's guest-frame arena
 *   PE_GUEST_FRAME_LO..HI and the original's stack 0x801FE000..0x801FEFFF), which stay 0.  Consequences:
 *   - a provider store (of 0 or anything else) to a word the original neither read nor stored changes the
 *     canary and fails the final-image diff (before: a store of 0 over the zero seed was invisible);
 *   - an original store (of 0 or anything else) the provider omits leaves the canary behind and fails the diff;
 *   - category 8 "store-footprint": every byte the provider stored through PE_Store* (outside the exempt windows) must
 *     be a byte the original's S list stored, whatever its value (a same-value re-store is a footprint difference).
 *     Stores through PE_Translate pointers bypass the footprint log; they are covered by the image diff only.
 *   Both register stores (0x1F80xxxx) and RAM stores alias into ram[a & (MEM-1)] the same way on both sides.
 *   Extra controls (argv[2]): "poke0" = run the native, then PE_StoreU32(<a word the case neither reads nor stores>, 0)
 *   (must FAIL with canary, PASSES with -DCDX_SEED_ZERO); "same" = run the native, then re-store a seeded, unstored
 *   word with its own value (FAILS only through category 8).
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
static int case_terminated;   /* cdgen49: a stand-in sets it before longjmp(case_escape) when the callee never returns on hardware (BIOS exit): the case ends there, not a failure */
/* The candidate's fixed host scratch window (0x801FFE80 .. 0x801FFEA0: the 0xE command parameter byte of the stream pump at 0x801FFE90 and the
 * CD stream handler's own frame, result bytes at +0x10 and FIFO bytes at +8 from 0x801FFE80 (cdhost author, extends the same mapping), the slot the
 * project fixtures reach at the standard sp 0x801FFF00) is the same slot the vectors record as an own-frame pointer F+n
 * (oracle entered in isolation at a lower sp).  It is treated as own-frame: reads/writes are unlogged, pointers into it
 * compare as F+offset, and it is out of the final RAM diff.  -DCDX_STRICT_FRAME_PTR restores the raw comparison. */
#define CDX_SCRATCH_LO 0x801FFE80u
#define CDX_SCRATCH_HI 0x801FFEA0u
#ifdef CDX_STRICT_FRAME_PTR
static int in_scratch(pe_addr_t a) { (void)a; return 0; }
#else
static int in_scratch(pe_addr_t a) { return a >= CDX_SCRATCH_LO && a < CDX_SCRATCH_HI; }
#endif
static int in_frame(pe_addr_t a) { return (a >= PE_GUEST_FRAME_LO && a < PE_GUEST_FRAME_HI) || in_scratch(a); }
static uint32_t mlog[8192][3]; static unsigned nmlog;   /* register-store log: 8192 entries (was 1024; a 100-chunk SPU clear writes more, cdgen51) */
#ifndef CDX_SEED_ZERO
static uint8_t canary_img[MEM];                 /* the seed image of unseeded RAM */
static uint32_t ostore[MEM];                    /* generation mark: byte stored by the original's S list in this case */
static uint8_t canary_byte(uint32_t b) { return (uint8_t)(0xC5u ^ ((b ^ (b >> 8) ^ (b >> 16)) & 0x7Fu)); }
static int cdx_exempt(uint32_t b) { return (b >= (PE_GUEST_FRAME_LO & (MEM - 1)) && b < (PE_GUEST_FRAME_HI & (MEM - 1))) || (b >= 0x1FE000u && b < 0x1FF000u) || in_scratch(0x80000000u + b); }
#endif
static uint32_t rq[16384][3]; static unsigned nrq, rq_pos;
static uint32_t last_w_addr; static unsigned last_w_size;
static int is_reg(pe_addr_t a) { return (a >> 16) == 0x1F80u; }
/* Optional (a checker sets it before the first case): one non-register word whose loads are replayed in order, like register
 * reads, from the vector's L list (cdgen48: the DMA-completion flag 0x8009D24C, whose clearing is a harness hook).  0 = off. */
static pe_addr_t replay_extra;
/* Optional (cdgen48): the "@n" write-timing of a callee record counts logged stores (PE_StoreU*).  A provider that writes through
 * PE_Translate pointers has no logged store, so its "@0" cannot be told from a reordering; with this flag set a timing-only
 * mismatch (same callee, order and arguments) is counted as `timing_unobservable` instead of failing when the provider made
 * no logged store at all in the case.  Default 0 = the strict behaviour of the older checkers. */
static int timing_needs_logged_stores; static unsigned timing_unobservable;
static int is_replay(pe_addr_t a) { return is_reg(a) || (replay_extra && a == replay_extra); }
static int loose_reads;   /* CDX_LOOSE_READS=1: an extra read of PLAIN RAM the original never made is not a mismatch (no observable effect); register reads stay replayed in order */
static int mmio_read(pe_addr_t a, unsigned n, uint32_t *v)
{
    if (!is_replay(a)) return 0;
    if (rq_pos >= nrq || rq[rq_pos][0] != a || rq[rq_pos][1] != n) {
        bad_read = 1; *v = 0;
        if (getenv("CDX_SERVE_ANY")) {   /* diagnostic (default off): serve the read from the first unconsumed recorded read of the same address and width, any order; the case still fails */
            unsigned j;
            for (j = rq_pos; j < nrq; j++)
                if (rq[j][0] == a && rq[j][1] == n) { *v = rq[j][2]; fprintf(stderr, "  native R %08X/%u -> %X (SERVED OUT OF ORDER from recorded read #%u)\n", (unsigned)a, n, *v, j); return 1; }
        }
        if (getenv("CDX_TRACE")) fprintf(stderr, "  native R %08X/%u -> NOT REPLAYABLE (next recorded read: %s)\n", (unsigned)a, n, rq_pos < nrq ? "see L" : "none");
        if (escape_armed && ++runaway > 1000000ul) longjmp(case_escape, 1);
        return 1;
    }
    *v = rq[rq_pos++][2];
    if (getenv("CDX_TRACE")) fprintf(stderr, "  native R %08X/%u -> %X\n", (unsigned)a, n, *v);
    return 1;
}
static void rd(pe_addr_t a, unsigned n)
{
    unsigned j;
    if (in_frame(a) || loose_reads) return;
    for (j = 0; j < n; j++) if (readable[(a + j) & (MEM - 1)] != cur) {
        if (!bad_read && getenv("CDX_DEBUG")) fprintf(stderr, "  first read outside original loads: %08X size %u\n", (unsigned)a, n);
        bad_read = 1;
    }
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
    if (is_reg(a) && nmlog < 8192) { mlog[nmlog][0] = a; mlog[nmlog][1] = n; mlog[nmlog][2] = v; nmlog++; }
    if (is_reg(a) && getenv("CDX_TRACE")) fprintf(stderr, "  native W %08X/%u <- %X\n", (unsigned)a, n, v);
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
static uint32_t rets_fn[64], rets_val[64][8]; static unsigned rets_len[64], rets_ctr[64]; static unsigned nrets_t;
static uint32_t want_fbase, got_fbase; static int want_fset, got_fset;
static uint32_t ret_of(uint32_t fn)
{
    unsigned i;
    for (i = 0; i < nrets_t; i++) if (rets_fn[i] == fn) {      /* a per-call sequence `a/b/c`; the last value repeats */
        unsigned k = rets_ctr[i]++;
        return rets_val[i][k < rets_len[i] ? k : rets_len[i] - 1];
    }
    return 0;
}
static void fmt_got(char *o, size_t n, uint32_t v)
{
    if ((v >= PE_GUEST_FRAME_LO && v < 0x801FF000u) || in_scratch(v)) {   /* sw-cdx-stream2 de6617fc9: the whole 0x801FE000..FFF own-frame window; sw-cdx-stream3: also the fixed scratch slot */
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
/* Record a call to callee `fn` taking `n` guest arguments (up to 8: cdgen47 records the 5th..7th, the stack words of the
 * retail 7-argument 0x8007CEAC, as ordinary values); return its scripted result. */
static __attribute__((unused)) uint32_t cdx_calleeN(uint32_t fn, unsigned n, const uint32_t *a)
{
    char t[200], p[24]; unsigned i; char sfx[24];
    snprintf(t, sizeof t, "%08X", fn);
    for (i = 0; i < n && i < 8; i++) {
        fmt_got(p, sizeof p, a[i]);
        strncat(t, ",", sizeof t - strlen(t) - 1);
        strncat(t, p, sizeof t - strlen(t) - 1);
    }
    snprintf(sfx, sizeof sfx, "@%u", nwlog);
    if (ngot < MAX_CALLS) { memcpy(got_calls[ngot], t, strlen(t) + 1); strncat(got_calls[ngot], sfx, sizeof got_calls[ngot] - strlen(t) - 1); }
    ngot++;
    if (escape_armed && ngot > 50000u) longjmp(case_escape, 1);   /* a native stuck in a retry loop around a stand-in */
    return ret_of(fn);
}
static __attribute__((unused)) uint32_t cdx_callee(uint32_t fn, unsigned n, uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4)
{
    uint32_t a[8] = {0}; a[0] = a0; a[1] = a1; a[2] = a2; a[3] = a3; a[4] = a4;
    return cdx_calleeN(fn, n, a);
}
#define CALLEE(sym, fn, n) \
    uint32_t sym(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e) \
    { return cdx_callee(fn, n, (uint32_t)a, (uint32_t)b, (uint32_t)c, (uint32_t)d, (uint32_t)e); }
/* A symbol the code under test must not reach: a call fails the case. */
#define TRAP(sym) \
    uint32_t sym(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e) \
    { (void)a; (void)b; (void)c; (void)d; (void)e; bad_call = 1; trap_name = #sym; \
      if (escape_armed && ++runaway > 100000ul) { longjmp(case_escape, 1); } \
      return 0; }

/* A host-only effect with no retail counterpart (a host stream feed closing, ...): counted, not compared. */
static unsigned host_only_calls;
#define IGNORE(sym) \
    uint32_t sym(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e) \
    { (void)a; (void)b; (void)c; (void)d; (void)e; host_only_calls++; return 0; }

/* Optional (a checker sets it from call(), before the first comparison): normalise an expected callee argument
 * (retail value, 32-bit) to the width of the public prototype's parameter.  Default: compared as is. */
static uint32_t (*want_norm)(uint32_t fn, unsigned idx, uint32_t v);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret);   /* batch dispatcher: 1 = returns a value, 0 = void, -1 unknown */
static int mutant;   /* 0 native, 1 noop, 2 flip (retflip+storeflip), 3 poke0, 4 same (canary controls), 5 retflip (return only), 6 storeflip (last guest write only) */

#ifndef CDX_SEED_ZERO
/* canary-mode diagnostics: classification of every final-image difference / footprint extra, aggregated per (entry, word) */
enum { CL_EXTRA = 0, CL_EXTRA_SAME, CL_MISSING, CL_WRONG, CL_UNLOGGED, CL_N };
static const char *const cl_name[CL_N] = {"extra-store (provider stored, original did not; value differs from pre-image)",
    "extra-store-same-value (footprint only: provider re-stored the pre-image value)",
    "missing-store (original stored, provider left the pre-image)", "wrong-value (both stored the byte, values differ)",
    "unlogged-image-diff (no PE_Store* by the provider: PE_Translate write, or an earlier-case artefact)"};
static unsigned cl_cases[CL_N], cl_words[CL_N], cl_zero_words[CL_N]; static uint32_t cl_case_mark[CL_N];
static unsigned zero_unseeded_entries; static uint8_t zero_unseeded_word[MEM / 4]; static unsigned zero_unseeded_distinct;
static unsigned footprint_only_cases, canary_image_cases;
struct agg { uint32_t entry, addr; unsigned cls, n; uint32_t got, want, pre; };
static struct agg aggs[1024]; static unsigned nagg;
static void agg_add(uint32_t entry, uint32_t addr, unsigned cls, uint32_t got, uint32_t want, uint32_t pre)
{
    unsigned i;
    for (i = 0; i < nagg; i++) if (aggs[i].entry == entry && aggs[i].addr == addr && aggs[i].cls == cls) { aggs[i].n++; return; }
    if (nagg < 1024) { aggs[nagg].entry = entry; aggs[nagg].addr = addr; aggs[nagg].cls = cls; aggs[nagg].n = 1; aggs[nagg].got = got; aggs[nagg].want = want; aggs[nagg].pre = pre; nagg++; }
}
static uint32_t rdw(const uint8_t *m, uint32_t k) { uint32_t v; memcpy(&v, m + k, 4); return v; }
/* Classify the case's differences.  Returns 1 when category 8 (footprint extra) fires; *img = 1 when the image differs (cat 4).
 * Cost: only the 64-byte blocks that differ from `expect` and the bytes the provider stored (wlog, complete below its 8192 cap) are visited. */
static uint32_t wseen[CL_N][MEM / 4];
static void classify_byte(uint32_t entry, uint32_t k, unsigned *fp, int *img, char *first, size_t firstn)
{
    int pw = wrote[k] == cur, os = ostore[k] == cur;
    int diff = ram[k] != expect[k];
    unsigned cls = CL_N;
    if (cdx_exempt(k)) return;
    if (pw && !os) { cls = (ram[k] != snap[k]) ? CL_EXTRA : CL_EXTRA_SAME; *fp = 1; }
    else if (diff) cls = (!pw && os) ? CL_MISSING : (pw && os) ? CL_WRONG : CL_UNLOGGED;
    if (diff) *img = 1;
    if (cls < CL_N) {
        uint32_t w = k & ~3u;
        if (wseen[cls][w >> 2] != cur) {
            int z = (cls == CL_EXTRA) ? (rdw(ram, w) == 0 && rdw(snap, w) != 0) : (cls == CL_MISSING) ? (rdw(expect, w) == 0) : 0;
            wseen[cls][w >> 2] = cur; cl_words[cls]++; if (z) cl_zero_words[cls]++;
            agg_add(entry, 0x80000000u | w, cls, rdw(ram, w), rdw(expect, w), rdw(snap, w));
            if (strlen(first) + 120 < firstn && strlen(first) < 480)
                snprintf(first + strlen(first), firstn - strlen(first), "    %-12s %08X got=%08X want=%08X pre=%08X\n",
                         cls == CL_EXTRA ? "extra" : cls == CL_EXTRA_SAME ? "extra-same" : cls == CL_MISSING ? "missing" : cls == CL_WRONG ? "wrong-value" : "unlogged",
                         0x80000000u | w, rdw(ram, w), rdw(expect, w), rdw(snap, w));
        }
        if (cl_case_mark[cls] != cur) { cl_case_mark[cls] = cur; cl_cases[cls]++; }
    }
}
static unsigned classify_case(uint32_t entry, int *img, char *first, size_t firstn)
{
    unsigned k, j, fp = 0;
    *img = 0; first[0] = 0;
    if (nwlog >= 8192) { for (k = 0; k < MEM; k++) classify_byte(entry, k, &fp, img, first, firstn); return fp; }
    for (k = 0; k < MEM; k += 64) if (memcmp(ram + k, expect + k, 64)) for (j = 0; j < 64; j++) classify_byte(entry, k + j, &fp, img, first, firstn);
    for (j = 0; j < nwlog; j++) { uint32_t bb = wlog[j] & ~63u; if (!memcmp(ram + bb, expect + bb, 64)) classify_byte(entry, wlog[j], &fp, img, first, firstn); }
    return fp;
}
#endif
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
/* failure categories (printed with CDX_CATEGORIES=1): a case can fail several */
#ifndef CDX_SEED_ZERO
#define CDX_NCAT 9
#define CDX_CAT_FOOTPRINT 8
static const char *const cat_name[] = {"return", "read-outside-original-loads", "trap-call", "register-read-replay", "ram-image", "register-stores", "callee-sequence", "callee-write-timing", "store-footprint"};
#else
#define CDX_NCAT 8
static const char *const cat_name[] = {"return", "read-outside-original-loads", "trap-call", "register-read-replay", "ram-image", "register-stores", "callee-sequence", "callee-write-timing"};
#endif
static unsigned cat_count[CDX_NCAT]; static int cat_hit[CDX_NCAT];
#define FAILC(k) do { cat_hit[k] = 1; ok = 0; } while (0)
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
    loose_reads = getenv("CDX_LOOSE_READS") != NULL;
    if (argc > 2) mutant = !strcmp(argv[2], "noop") ? 1 : !strcmp(argv[2], "flip") ? 2 : !strcmp(argv[2], "poke0") ? 3 : !strcmp(argv[2], "same") ? 4 : !strcmp(argv[2], "retflip") ? 5 : !strcmp(argv[2], "storeflip") ? 6 : 0;
#ifndef CDX_SEED_ZERO
    { uint32_t b; for (b = 0; b < MEM; b++) canary_img[b] = cdx_exempt(b) ? 0 : canary_byte(b); }
#endif
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
#ifndef CDX_SEED_ZERO
        memcpy(ram, canary_img, sizeof ram);
#else
        memset(ram, 0, sizeof ram);
#endif
        for (k = (unsigned)li + 1; k < (unsigned)si; k++) {
            unsigned addr, size, val;
            if (!strcmp(tok[k], "-")) continue;
            if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) == 3 && is_replay(addr)) {
                if (nrq < 16384) { rq[nrq][0] = addr; rq[nrq][1] = size; rq[nrq][2] = val; nrq++; }
                continue;
            }
            apply_seed(tok[k]);
        }
        memcpy(snap, ram, sizeof ram); memcpy(expect, ram, sizeof ram);
        nrets_t = 0;
        if (strcmp(tok[ti + 1], "-"))
            for (k = (unsigned)ti + 1; k < (unsigned)ci; k++) {
                char *e = tok[k];
                while (e && *e) {              /* a RETS token is a comma-separated list `fn=v,fn=v/v/v,..` */
                    char *eq = strchr(e, '='), *comma;
                    if (!eq) break;
                    if (nrets_t < 64) {
                        char *q = eq + 1; unsigned nv = 0;
                        rets_fn[nrets_t] = (uint32_t)strtoul(e, 0, 16);
                        for (;;) {
                            char *end; unsigned long val = strtoul(q, &end, 16);
                            if (nv < 8) rets_val[nrets_t][nv++] = (uint32_t)val;
                            if (*end != '/') break;
                            q = end + 1;
                        }
                        rets_len[nrets_t] = nv; rets_ctr[nrets_t] = 0; nrets_t++;
                    }
                    comma = strchr(eq, ',');
                    e = comma ? comma + 1 : NULL;
                }
            }
        for (k = (unsigned)si + 1; k < (unsigned)ti; k++) {          /* the original's final stores */
            unsigned addr, size, val, b;
            if (!strcmp(tok[k], "-") || sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) != 3 || size > 4) continue;
            for (b = 0; b < size; b++) expect[(addr + b) & (MEM - 1)] = (uint8_t)(val >> (8 * b));
#ifndef CDX_SEED_ZERO
            { int allz = 1, unseeded = 1;
              for (b = 0; b < size; b++) {
                  uint32_t m = (addr + b) & (MEM - 1);
                  ostore[m] = cur;
                  if (((val >> (8 * b)) & 0xFFu) != 0) allz = 0;
                  if (readable[m] == cur || cdx_exempt(m) || is_reg(addr)) unseeded = 0;
              }
              if (allz && unseeded) {   /* the blind class of the audit: an original store of 0 to bytes the case never seeds */
                  uint32_t w = ((addr & (MEM - 1)) >> 2);
                  zero_unseeded_entries++;
                  if (!(zero_unseeded_word[w >> 3] & (1u << (w & 7)))) { zero_unseeded_word[w >> 3] |= (uint8_t)(1u << (w & 7)); zero_unseeded_distinct++; }
              }
            }
#endif
        }
        runaway = 0;
        case_terminated = 0;
        if (setjmp(case_escape)) { escape_armed = 0; if (!case_terminated) bad_read = 1; has = 0; got = 0; }
        else { escape_armed = 1; has = call(entry, a, &got); escape_armed = 0; }
        if (has < 0) { fprintf(stderr, "unknown entry %08X\n", entry); return 2; }
        if (mutant == 2 || mutant == 5) { if (has) got ^= 1u; }   /* flip = retflip + storeflip */
        if (mutant == 2 || mutant == 6) { if (last_w_size) ram[last_w_addr & (MEM - 1)] ^= 1u; else ram[0x1000] ^= 1; }
        if (mutant == 3 || mutant == 4) {   /* positive controls for the canary: stores the original never made (see the header comment) */
            uint32_t w;
            for (w = mutant == 5 ? 0x100000u : 0u; w + 4 <= MEM; w += 4) {
                uint32_t b0 = w, ok4 = 1; unsigned q;
                for (q = 0; q < 4; q++) {
                    uint32_t b = b0 + q;
#ifndef CDX_SEED_ZERO
                    if (ostore[b] == cur || cdx_exempt(b)) ok4 = 0;
#endif
                    if (mutant == 5 ? readable[b] == cur : readable[b] != cur) ok4 = 0;
                }
                if (ok4) { uint32_t v = 0; if (mutant == 6) memcpy(&v, ram + w, 4); PE_StoreU32(0x80000000u | w, v); break; }
            }
        }
        memset(cat_hit, 0, sizeof cat_hit);
        if (has && got != wret) FAILC(0);
        if (bad_read) FAILC(1);
        if (bad_call) FAILC(2);
        if (rq_pos != nrq) FAILC(3);
        {   /* final RAM image (the native's frame arena and the original's stack are out) */
            uint32_t lo = PE_GUEST_FRAME_LO & (MEM - 1), hi = PE_GUEST_FRAME_HI & (MEM - 1);
            for (k = 0; k < MEM && !cat_hit[4]; k += 4) {
                if (memcmp(ram + k, expect + k, 4)) {
                    unsigned j;
                    for (j = 0; j < 4; j++)
                        if (ram[k + j] != expect[k + j] && !((k + j) >= lo && (k + j) < hi) &&
                            !((k + j) >= 0x1FE000u && (k + j) < 0x1FF000u) &&
                            !in_scratch(0x80000000u + k + j)) FAILC(4);
                }
            }
        }
#ifndef CDX_SEED_ZERO
        {   /* canary mode: classification + category 8 (every provider-stored byte must be an original-stored byte) */
            int img; char first[600]; unsigned fp = classify_case(entry, &img, first, sizeof first);
            if (fp) FAILC(CDX_CAT_FOOTPRINT);
            if (img) canary_image_cases++;
            if (fp && !img) footprint_only_cases++;
            if (first[0] && bad < (getenv("CDX_MAXPRINT") ? (unsigned)atoi(getenv("CDX_MAXPRINT")) : 8u) && !mutant)
                fprintf(stderr, "CANARY line %u entry %08X (final-image / footprint differences):\n%s", n, entry, first);
        }
#endif
        {   /* register stores in order */
            unsigned want_m = 0, addr, size, val;
            for (k = (unsigned)si + 1; k < (unsigned)ti; k++) {
                if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) != 3 || !is_reg(addr)) continue;
                if (want_m >= nmlog || mlog[want_m][0] != addr || mlog[want_m][1] != size ||
                    mlog[want_m][2] != (val & (size == 4 ? 0xFFFFFFFFu : (1u << (8 * size)) - 1u))) FAILC(5);
                want_m++;
            }
            if (want_m != nmlog) FAILC(5);
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
                {   unsigned ai = 0; uint32_t fnv = 0;
                for (p = strtok(tk, ","); p; p = strtok(NULL, ",")) {
                    char o[24];
                    if (w[0]) strncat(w, ",", sizeof w - strlen(w) - 1);
                    if (!w[0] && p == tk) { fnv = (uint32_t)strtoul(p, 0, 16); snprintf(o, sizeof o, "%08X", (unsigned)fnv); }
                    else {
                        uint32_t av = (uint32_t)strtoul(p, 0, 16);
                        if (want_norm) av = want_norm(fnv, ai, av);   /* cdgen46: compare a typed (narrow) argument at its public width */
                        ai++;
                        fmt_want(o, sizeof o, av);
                    }
                    strncat(w, o, sizeof w - strlen(w) - 1);
                } }
                { char sfx[24]; snprintf(sfx, sizeof sfx, "@%u", cnt); strncat(w, sfx, sizeof w - strlen(w) - 1); }
                if (want_c >= ngot) FAILC(6);
                else if (want_c < MAX_CALLS && strcmp(got_calls[want_c], w)) {
                    /* same callee, order and arguments but a different count of distinct bytes written before the call:
                     * category 7 (write timing); anything else: category 6 */
                    const char *ga = strchr(got_calls[want_c], '@'), *wa = strchr(w, '@');
                    if (ga && wa && (size_t)(ga - got_calls[want_c]) == (size_t)(wa - w) &&
                        !strncmp(got_calls[want_c], w, (size_t)(wa - w))) { if (timing_needs_logged_stores && nwlog == 0) timing_unobservable++; else FAILC(7); } else FAILC(6);
                }
                want_c++;
            }
            if (want_c != ngot) FAILC(6);
        }
        { unsigned q; for (q = 0; q < CDX_NCAT; q++) cat_count[q] += (unsigned)cat_hit[q]; }
        if (!ok) {
            entry_bad[slot]++;
            if (getenv("CDX_LINES")) fprintf(stderr, "FAILLINE %u\n", n);
            if (bad++ < (getenv("CDX_MAXPRINT") ? (unsigned)atoi(getenv("CDX_MAXPRINT")) : 8u) && !mutant) {
                unsigned q;
                fprintf(stderr, "MISMATCH line %u entry %08X (%s%s%s) %.160s\n", n, entry, bad_read ? "read outside original loads; " : "",
                        bad_call ? "unexpected call; " : "", "stores/return/calls/regs", line);
                if (bad_call) fprintf(stderr, "  trap: %s\n", trap_name ? trap_name : "?");
                { const char *cp = strstr(line, " CALL "); if (cp) fprintf(stderr, "  want%.*s\n", getenv("CDX_ALLCALLS") ? 100000 : 300, cp + 5); }
                if (getenv("CDX_DEBUG")) {
                    unsigned q;
                    fprintf(stderr, "  native wrote bytes:");
                    for (q = 0; q < nwlog && q < 48; q++) fprintf(stderr, " %05X=%02X", wlog[q], ram[wlog[q]]);
                    fprintf(stderr, "\n  original S: ");
                    for (q = (unsigned)si + 1; q < (unsigned)ti && q < (unsigned)si + 40; q++) fprintf(stderr, "%s ", tok[q]);
                    fprintf(stderr, "\n");
                }
                for (q = 0; q < ngot && q < (getenv("CDX_ALLCALLS") ? MAX_CALLS : 6u) && q < MAX_CALLS; q++) fprintf(stderr, "  got %s\n", got_calls[q]);
            }
        }
    }
    printf("mode=%s checked %u cases, %u mismatches (host-only calls ignored: %u)\n", mutant == 1 ? "noop" : mutant == 2 ? "flip" : mutant == 3 ? "poke0" : mutant == 4 ? "same" : mutant == 5 ? "retflip" : mutant == 6 ? "storeflip" : "native", n, bad, host_only_calls);
    if (timing_unobservable) printf("  callee write-timing not observable (provider made no logged store): %u records\n", timing_unobservable);
    for (i = 0; i < nentry; i++) printf("  %08X cases=%u mismatches=%u\n", entry_id[i], entries_seen[i], entry_bad[i]);
#ifndef CDX_SEED_ZERO
    printf("canary: original S stores of 0 to never-seeded bytes: %u stores, %u distinct words (the blind set under zero seeding)\n", zero_unseeded_entries, zero_unseeded_distinct);
    printf("canary: cases with a final-image difference %u, footprint-only (extra same-value store, image equal) %u\n", canary_image_cases, footprint_only_cases);
    for (i = 0; i < CL_N; i++) printf("canary: class %-96s cases=%u words=%u (zero-class words %u)\n", cl_name[i], cl_cases[i], cl_words[i], cl_zero_words[i]);
    if (nagg && getenv("CDX_AGG")) {
        fprintf(stderr, "canary aggregate (entry, word, class, cases, first got/want/pre):\n");
        for (i = 0; i < nagg && i < 1024; i++) fprintf(stderr, "  %08X %08X %-11s n=%u got=%08X want=%08X pre=%08X\n", aggs[i].entry, aggs[i].addr,
            aggs[i].cls == CL_EXTRA ? "extra" : aggs[i].cls == CL_EXTRA_SAME ? "extra-same" : aggs[i].cls == CL_MISSING ? "missing" : aggs[i].cls == CL_WRONG ? "wrong-value" : "unlogged", aggs[i].n, aggs[i].got, aggs[i].want, aggs[i].pre);
    }
#endif
    if (getenv("CDX_CATEGORIES")) for (i = 0; i < CDX_NCAT; i++) printf("  category %-28s %u cases\n", cat_name[i], cat_count[i]);
    return mutant ? 0 : bad != 0;
}
#endif
