/* sw-vec-gpu2 batch gpu2d checker: libpress - table expander func_8010BD4C, DecDCToutCallback func_8010C0D8 and
 * DecDCTReset func_8010BE3C (func_8010BE3C is bound to the clean-room pe_plat_mdec_reset.c; the boot file
 * func_8010BE3C_port.c now binds only func_8010C0D8).
 * Generator: pc_port/tools/pe_gpu2_vectors.py gpu2d (callback lines) and `expand` (expander lines, retail-derived:
 * kept under scratch with hashes committed); contract: pe_gpu2_check_core.h.
 * Expander lines (type X): `8010BD4C seed X dst PATCH off=val.. RET r SHA sha256 DELTA off:len:rle..`; the checker needs
 * two LOCAL files (retail-derived, never committed): argv[2] = build/extracted/disc1/peimg/ovl_039F.bin (table bytes at
 * file offset 0xF04.. = 0x8010CBFC..0x8011ECF8, sha256 in the vector file header) and argv[3] = <vectors>.base.bin (the
 * output for the unpatched table).  The provider may read only those table bytes the original read plus its own output.
 * Build lines (run from the repo root, verbatim; both link the CURRENT binding as a black box):
 *   cc -std=gnu11 -O1 -w -DG2_SMOKE -DCHK_EXPAND -DCHK_CB -DCHK_RESET -DG2_BATCH_NAME=\"gpu2d\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2d_check.c pc_port/game/boot/func_8010BD4C_port.c pc_port/game/boot/func_8010BE3C_port.c pc_port/platform/pe_plat_mdec_reset.c -o check_gpu2d
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_RESET -DG2_BATCH_NAME=\"gpu2d-reset\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2d_check.c pc_port/platform/pe_plat_mdec_reset.c -o check_gpu2d_reset
 *   cc -std=gnu11 -O1 -w -DCHK_CB -DG2_BATCH_NAME=\"gpu2d-cb\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2d_check.c pc_port/game/boot/func_8010BE3C_port.c -o check_gpu2d_cb
 *   (control build against the removed translated file; recorded in docs/evidence/native-gpu2-20261126/gpu2d/AUTHOR-REPORT.md; not runnable since 2026-11-27): the first and third cc lines name pc_port/game/boot/func_8010BD4C_port.c and func_8010BE3C_port.c
 *   (the two lines above are clean-room shapes - no -DG2_SMOKE - run with the real clean-room pe_plat_mdec_reset.c and the current func_8010C0D8 provider)
 *   ./check_gpu2d vectors/gpu2d.txt build/extracted/disc1/peimg/ovl_039F.bin /path/to/expand.txt.base.bin   (expand lines)
 * Groups (select with -D): CHK_EXPAND (func_8010BD4C -> pe_plat_mdec_expand_table_host), CHK_CB (func_8010C0D8 ->
 * pe_plat_mdec_out_callback_host), CHK_RESET (func_8010BE3C -> pe_plat_mdec_reset; callee func_8010C0FC =
 * pe_plat_mdec_core and func_80073C94 = pe_plat_reset_callback are stubs).  The smoke line selects all three
 * (func_8010BE3C there is the clean-room pe_plat_mdec_reset.c).
 * Clean-room shapes (no -DG2_SMOKE; a missing provider fails at link):
 *   TEMPLATE cc -std=gnu11 -Wall -Wextra -Werror -DCHK_EXPAND -DG2_BATCH_NAME=\"gpu2d-expand\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2d_check.c <provider.c> -o check_gpu2d_expand
 *   TEMPLATE cc -std=gnu11 -Wall -Wextra -Werror -DCHK_CB -DG2_BATCH_NAME=\"gpu2d-cb\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2d_check.c <provider.c> -o check_gpu2d_cb
 *   TEMPLATE cc -std=gnu11 -Wall -Wextra -Werror -DCHK_RESET -DG2_BATCH_NAME=\"gpu2d-reset\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2d_check.c <provider.c> -o check_gpu2d_reset
 * Mutants: -DGPU2_MUTANT=1 (no-op stub), =2 (return bit flip), =3 (store bit flip). */
#define G2_HAVE_EXTRA
#include "pe_gpu2_check_core.h"
#include "pe_guest_ram.h"
#include "pe_gpu2_host_stubs.h"
#ifdef G2_SMOKE
#include "pe_gpu2_smoke_host.h"
int Bootstrap_ReturnInt1(const char *a, const char *b, int v) { (void)a; (void)b; return v; }
#endif
#ifdef CHK_EXPAND
uint32_t pe_plat_mdec_expand_table_host(uint32_t dst, uint32_t count);
#endif
#ifdef CHK_CB
uint32_t pe_plat_mdec_out_callback_host(uint32_t callback);
#endif
#ifdef CHK_RESET
void pe_plat_mdec_reset(int mode);
#endif
void pe_plat_mdec_core(int mode) { (void)g2_call(0x8010C0FCu, (uint32_t)mode, 0, 0, 0); }
uint32_t pe_plat_reset_callback(void) { return g2_call(0x80073C94u, 0, 0, 0, 0); }
static unsigned g2_selected_list(uint32_t *out)
{
    unsigned n = 0;
#ifdef CHK_EXPAND
    out[n++] = 0x8010BD4Cu;
#endif
#ifdef CHK_CB
    out[n++] = 0x8010C0D8u;
#endif
#ifdef CHK_RESET
    out[n++] = 0x8010BE3Cu;
#endif
    (void)out;
    return n;
}

#ifdef CHK_EXPAND
static uint8_t g2_tab[0x13000], g2_base[0x11000];
static int g2_loaded;
static int g2_unrle(const char *s, uint8_t *dst, unsigned len)
{
    unsigned i = 0;
    while (*s && i < len) {
        if (*s == ',') { s++; continue; }
        if (*s == 'z') { char *e; unsigned long n = strtoul(s + 1, &e, 16); if (i + n > len) return 0; memset(dst + i, 0, n); i += (unsigned)n; s = e; continue; }
        {
            unsigned v;
            if (sscanf(s, "%2x", &v) != 1) return 0;
            dst[i++] = (uint8_t)v; s += 2;
        }
    }
    return i == len;
}
static int g2_extra(char **tok, int nt, uint32_t case_no, const char **why)
{
    uint32_t dst = (uint32_t)strtoul(tok[3], 0, 16), want_ret = 0, got = 0;
    int i, ri = -1, di = -1, pi = -1;
    unsigned k, d0;
    static uint8_t want[0x11000];
    uint32_t tb = 0x8010CBFCu;
    if (!g2_loaded) {
        FILE *f;
        if (g2_argc < 4 || !(f = fopen(g2_argv[2], "rb"))) { *why = "missing overlay file argument"; return 0; }
        if (fread(g2_tab, 1, sizeof g2_tab, f) != 0x13000) { fclose(f); *why = "short overlay file"; return 0; }
        fclose(f);
        if (!(f = fopen(g2_argv[3], "rb"))) { *why = "missing base output file argument"; return 0; }
        if (fread(g2_base, 1, sizeof g2_base, f) != sizeof g2_base) { fclose(f); *why = "short base output file"; return 0; }
        fclose(f);
        g2_loaded = 1;
    }
    for (i = 4; i < nt; i++) {
        if (!strcmp(tok[i], "PATCH")) pi = i;
        if (!strcmp(tok[i], "RET")) ri = i;
        if (!strcmp(tok[i], "DELTA")) di = i;
    }
    if (ri < 0 || di < 0 || pi < 0) { *why = "malformed expander line"; return 0; }
    want_ret = (uint32_t)strtoul(tok[ri + 1], 0, 16);
    g2_cur = case_no; g2_fail = 0; g2_pos = 0; g2_nev = 0; g2_nw = 0; g2_nx = 0; g2_msg[0] = 0;
    /* only the table bytes the original reads (offsets 0..0xE0C, see pe_gpu2_expand_independence.py) are readable */
    for (k = tb; k < tb + 0xE0Du; k++) { uint32_t b = k & (G2_MEM - 1); g2_ram[b] = g2_tab[k - 0x8010BCF8u]; g2_known[b] = g2_cur; }
    for (i = pi + 1; i < ri; i++) {
        unsigned off, val;
        if (strcmp(tok[i], "-") && sscanf(tok[i], "%x=%x", &off, &val) == 2) g2_ram[(tb + off) & (G2_MEM - 1)] = (uint8_t)val;
    }
    d0 = dst & (G2_MEM - 1);
    for (k = 0; k < 0x11000; k++) { g2_ram[(d0 + k) & (G2_MEM - 1)] = 0; g2_known[(d0 + k) & (G2_MEM - 1)] = g2_cur; }
    memcpy(want, g2_base, sizeof want);
    for (i = di + 1; i < nt; i++) {
        unsigned off, len; char *c1, *c2;
        if (!strcmp(tok[i], "-")) continue;
        off = (unsigned)strtoul(tok[i], &c1, 16);
        len = (unsigned)strtoul(c1 + 1, &c2, 16);
        if (off + len > 0x11000 || !g2_unrle(c2 + 1, want + off, len)) { *why = "bad delta"; return 0; }
    }
    for (k = 0; k < 0x11000; k++) { uint32_t b = (d0 + k) & (G2_MEM - 1); g2_exp[b] = g2_cur; g2_expval[b] = want[k]; g2_xlog[g2_nx++] = b; }
    {
        uint32_t a[2]; a[0] = dst; a[1] = 0;
        if (g2_entry(0x8010BD4Cu, a, &got) < 0) { *why = "no binding"; return 0; }
    }
#if defined(GPU2_MUTANT) && GPU2_MUTANT == 2
    got ^= 1u;
#endif
    if (g2_fail) { *why = g2_msg; return 0; }
    if (got != want_ret) { *why = "return value"; return 0; }
    if (g2_nw != g2_nx) { *why = "written byte count"; return 0; }
    for (k = 0; k < g2_nw; k++)
        if (g2_exp[g2_wlog[k]] != g2_cur || g2_ram[g2_wlog[k]] != g2_expval[g2_wlog[k]]) { *why = "output byte"; return 0; }
    return 1;
}
#else
static int g2_extra(char **tok, int nt, uint32_t case_no, const char **why)
{ (void)tok; (void)nt; (void)case_no; *why = "expander group not selected"; return 0; }
#endif
static int g2_entry(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
#if defined(GPU2_MUTANT) && GPU2_MUTANT == 1
    {
        uint32_t sel[8]; unsigned i, n = g2_selected_list(sel);
        (void)a; (void)ret;
        for (i = 0; i < n; i++) if (sel[i] == entry) return 1;
        return -2;
    }
#else
    switch (entry) {
#ifdef CHK_EXPAND
    case 0x8010BD4Cu: *ret = pe_plat_mdec_expand_table_host(a[0], a[1]); return 1;
#endif
#ifdef CHK_CB
    case 0x8010C0D8u: *ret = pe_plat_mdec_out_callback_host(a[0]); return 1;
#endif
#ifdef CHK_RESET
    case 0x8010BE3Cu: pe_plat_mdec_reset((int)a[0]); return 0;
#endif
    }
    (void)a; (void)ret; (void)entry;
    return -2;
#endif
}
int main(int c, char **v) { return g2_main(c, v); }
