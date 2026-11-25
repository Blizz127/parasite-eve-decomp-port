/* Vector check for pe_plat_gpu_words.c and pe_plat_gpu_drawenv.c:
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform pc_port/tools/pe_gpu_leaf_vectors_check.c pc_port/platform/pe_plat_gpu_words.c pc_port/platform/pe_plat_gpu_drawenv.c -o check
 *   ./check vectors.txt
 * Line: ENTRY seed pattern A args... RET r L addr:size=value... S addr:size=value...
 * (`-` for an empty list), from pc_port/tools/pe_gpu_leaf_vectors.py.
 * Memory starts as poison.  Each byte the original loaded gets the value it
 * loaded (first load wins); the code under test may read only those bytes.
 * After the call, the set of bytes written and their final values must equal
 * the original's stores replayed in order (the original's own stack frame is
 * not logged, so the code under test must not need guest stack memory).
 * RET is compared for 0x80076170, 0x80076208, 0x800762BC and 0x80074924
 * (SetDefDrawEnv, `drawenv` vectors; its fifth argument was on the
 * original's stack, so the load of 0x801FF010 in L is the original reading
 * it, not memory the code under test should read). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/gpu_primitive.h"
#define MEM 0x200000u
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
    }
}
uint8_t PE_LoadU8(pe_addr_t a) { rd(a, 1); return ram[a & (MEM - 1)]; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; rd(a, 2); memcpy(&v, ram + (a & (MEM - 1)), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; rd(a, 4); memcpy(&v, ram + (a & (MEM - 1)), 4); return v; }
void PE_StoreU8(pe_addr_t a, uint8_t v) { wr(a, 1); ram[a & (MEM - 1)] = v; }
void PE_StoreU16(pe_addr_t a, uint16_t v) { wr(a, 2); memcpy(ram + (a & (MEM - 1)), &v, 2); }
void PE_StoreU32(pe_addr_t a, uint32_t v) { wr(a, 4); memcpy(ram + (a & (MEM - 1)), &v, 4); }
static int call(uint32_t entry, uint32_t *a, uint32_t *ret)
{
    switch (entry) {
    case 0x80076170u: *ret = pe_plat_gpu_draw_area_tl_word((short)a[0], (short)a[1]); return 1;
    case 0x80076208u: *ret = pe_plat_gpu_draw_area_br_word((short)a[0], (short)a[1]); return 1;
    case 0x800762BCu: *ret = pe_plat_gpu_tex_window_word(a[0]); return 1;
    case 0x80077AC4u: pe_plat_gpu_add_prim(a[0], a[1]); return 0;
    case 0x80074924u: *ret = pe_plat_gpu_set_def_draw_env(a[0], (int)a[1], (int)a[2], (int)a[3], (int)a[4]); return 1;
    }
    return -1;
}
static int apply(const char *t, uint8_t *dst, uint32_t *set, int first_only)
{
    unsigned addr, size, val, i;
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
static int find(char **tok, int nt, int from, const char *s)
{
    while (from < nt && strcmp(tok[from], s)) from++;
    return from;
}
int main(int c, char **v)
{
    FILE *f; static char line[65536]; unsigned n = 0, bad = 0;
    if (c != 2 || !(f = fopen(v[1], "r"))) return 2;
    while (fgets(line, sizeof line, f)) {
        static char *tok[4096]; static char copy[65536];
        int nt = 0, i, ri, li, si, has, ok = 1, na; uint32_t entry, a[8] = {0}, wret, got = 0; unsigned k;
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < 4096 && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
        if (nt < 8 || strcmp(tok[3], "A")) continue;
        ri = find(tok, nt, 4, "RET"); li = ri + 2; si = find(tok, nt, li, "S");
        if (si >= nt || li >= nt || strcmp(tok[li], "L")) continue;
        entry = (uint32_t)strtoul(tok[0], 0, 16);
        na = ri - 4;
        for (i = 0; i < na && i < 8; i++) if (strcmp(tok[4 + i], "-")) a[i] = (uint32_t)strtoul(tok[4 + i], 0, 16);
        wret = (uint32_t)strtoul(tok[ri + 1], 0, 16);
        cur = ++n; nwlog = nxlog = 0; bad_read = 0;
        for (k = (unsigned)li + 1; k < (unsigned)si; k++) if (strcmp(tok[k], "-")) apply(tok[k], ram, readable, 1);
        for (k = (unsigned)si + 1; k < (unsigned)nt; k++) if (strcmp(tok[k], "-")) apply(tok[k], want, want_set, 0);
        has = call(entry, a, &got);
        if (has < 0) { fprintf(stderr, "unknown entry %08X\n", entry); return 2; }
        if (has && got != wret) ok = 0;
        if (bad_read) ok = 0;
        if (nwlog != nxlog) ok = 0;
        for (k = 0; k < nwlog && ok; k++)
            if (want_set[wlog[k]] != cur || ram[wlog[k]] != want[wlog[k]]) ok = 0;
        if (!ok && bad++ < 12)
            fprintf(stderr, "MISMATCH line %u (%s%s): %.200s\n", n, bad_read ? "read outside original loads; " : "",
                    "stores/return", line);
    }
    printf("checked %u cases, %u mismatches\n", n, bad);
    return bad != 0;
}
