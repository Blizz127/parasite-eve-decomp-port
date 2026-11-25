/* Vector check for pe_plat_gte_regs.c and pe_plat_gte_stack.c:
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform pc_port/tools/pe_gte_leaf_vectors_check.c pc_port/platform/pe_plat_gte_regs.c pc_port/platform/pe_plat_gte_stack.c -o check
 *   ./check vectors.txt
 * Line: ENTRY seed pattern A args... RET r C0 c0..c31 C1 c0..c31 D d L addr:size=value... S addr:size=value...
 * [CALL fmt,a1 ...] (`-` for an empty list), from pc_port/tools/pe_gte_leaf_vectors.py
 * (the stack batch, PushMatrix/PopMatrix, adds CALL: the original's printf
 * calls, in order; only the format addresses are compared, through
 * PE_Bios_Printf).
 * GTE control registers start at C0 (raw words).  The code under test may
 * read them with PE_GTE_ReadCtrl (as cfc2 reads them: registers 4, 12 and 20
 * sign-extend their low 16 bits, psx-spx) and write them with
 * PE_GTE_WriteCtrl (raw word, as ctc2 writes it); afterwards all 32 must
 * equal C1.  Any use of a
 * GTE data register or command fails the case (no original case changed a
 * data register).  Memory starts as poison.  Each byte the original loaded
 * gets the value it loaded (first load wins); the code under test may read
 * only those bytes.  After the call, the set of bytes written and their
 * final values must equal the original's stores replayed in order.  RET is
 * compared for 0x80078CC4. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/gte.h"
#include "pe_bios_string.h"
#define MEM 0x200000u
static uint8_t ram[MEM], want[MEM];
static uint32_t readable[MEM], wrote[MEM], want_set[MEM], cur;
static uint32_t wlog[4096], nwlog, xlog[4096], nxlog;
static uint32_t ctrl[32];
static int bad_read, bad_gte;
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
/* GTE register access (declared in pe_sdk.h). */
uint32_t PE_GTE_ReadCtrl(uint32_t reg)
{
    if (reg > 31) { bad_gte = 1; return 0; }
    if (reg == 4 || reg == 12 || reg == 20) return (uint32_t)(int32_t)(int16_t)(uint16_t)ctrl[reg];
    return ctrl[reg];
}
void PE_GTE_WriteCtrl(uint32_t reg, uint32_t v) { if (reg > 31) { bad_gte = 1; return; } ctrl[reg] = v; }
uint32_t PE_GTE_ReadData(uint32_t reg) { (void)reg; bad_gte = 1; return 0; }
void PE_GTE_WriteData(uint32_t reg, uint32_t v) { (void)reg; (void)v; bad_gte = 1; }
void PE_GTE_Command(uint32_t cmd) { (void)cmd; bad_gte = 1; }
/* printf: format addresses in call order. */
static uint32_t pcalls[8], npcalls;
int32_t PE_Bios_Printf(pe_addr_t fmt, const uint32_t *args, int nargs)
{
    (void)args; (void)nargs;
    if (npcalls < 8) pcalls[npcalls] = fmt;
    npcalls++;
    return 0;
}
static int call(uint32_t entry, uint32_t *a, uint32_t *ret)
{
    switch (entry) {
    case 0x80078E04u: pe_plat_gte_set_rot_matrix(a[0]); return 0;
    case 0x80078E34u: pe_plat_gte_set_light_matrix(a[0]); return 0;
    case 0x80078E64u: pe_plat_gte_set_color_matrix(a[0]); return 0;
    case 0x80078E94u: pe_plat_gte_set_trans_matrix(a[0]); return 0;
    case 0x80078FC4u: pe_plat_gte_set_back_color(a[0], a[1], a[2]); return 0;
    case 0x80078FE4u: pe_plat_gte_set_far_color(a[0], a[1], a[2]); return 0;
    case 0x80079004u: pe_plat_gte_set_geom_offset((int)a[0], (int)a[1]); return 0;
    case 0x80079024u: pe_plat_gte_set_geom_screen((int)a[0]); return 0;
    case 0x80078CC4u: *ret = pe_plat_gte_scale_matrix(a[0], a[1]); return 1;
    case 0x80078A94u: pe_plat_gte_push_matrix(); return 0;
    case 0x80078B38u: pe_plat_gte_pop_matrix(); return 0;
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
        int nt = 0, i, ri, c0, c1, di, li, si, ci, has, ok = 1, na; uint32_t entry, a[4] = {0}, wret, got = 0, want_c[32]; unsigned k;
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < 4096 && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
        if (nt < 8 || strcmp(tok[3], "A")) continue;
        ri = find(tok, nt, 4, "RET"); c0 = find(tok, nt, ri, "C0"); c1 = c0 + 33; di = c1 + 33;
        li = di + 2; si = find(tok, nt, li, "S"); ci = find(tok, nt, si, "CALL");
        if (si >= nt || c1 >= nt || strcmp(tok[c1], "C1") || strcmp(tok[di], "D") || strcmp(tok[li], "L")) continue;
        entry = (uint32_t)strtoul(tok[0], 0, 16);
        na = ri - 4;
        for (i = 0; i < na && i < 4; i++) if (strcmp(tok[4 + i], "-")) a[i] = (uint32_t)strtoul(tok[4 + i], 0, 16);
        wret = (uint32_t)strtoul(tok[ri + 1], 0, 16);
        for (i = 0; i < 32; i++) {
            ctrl[i] = (uint32_t)strtoul(tok[c0 + 1 + i], 0, 16);
            want_c[i] = (uint32_t)strtoul(tok[c1 + 1 + i], 0, 16);
        }
        cur = ++n; nwlog = nxlog = 0; bad_read = bad_gte = 0; npcalls = 0;
        for (k = (unsigned)li + 1; k < (unsigned)si; k++) if (strcmp(tok[k], "-")) apply(tok[k], ram, readable, 1);
        for (k = (unsigned)si + 1; k < (unsigned)ci; k++) if (strcmp(tok[k], "-")) apply(tok[k], want, want_set, 0);
        has = call(entry, a, &got);
        if (has < 0) { fprintf(stderr, "unknown entry %08X\n", entry); return 2; }
        if (has && got != wret) ok = 0;
        if (bad_read || bad_gte) ok = 0;
        for (i = 0; i < 32; i++) if (ctrl[i] != want_c[i]) ok = 0;
        {   /* printf calls: count and format addresses */
            unsigned want_n = 0;
            for (k = (unsigned)ci + 1; k < (unsigned)nt; k++) {
                if (!strcmp(tok[k], "-")) continue;
                if (want_n < 8 && (want_n >= npcalls || pcalls[want_n] != (uint32_t)strtoul(tok[k], 0, 16))) ok = 0;
                want_n++;
            }
            if (want_n != npcalls) ok = 0;
        }
        if (nwlog != nxlog) ok = 0;
        for (k = 0; k < nwlog && ok; k++)
            if (want_set[wlog[k]] != cur || ram[wlog[k]] != want[wlog[k]]) ok = 0;
        if (!ok && bad++ < 12)
            fprintf(stderr, "MISMATCH line %u (%s%s): %.200s\n", n, bad_read ? "read outside original loads; " : "",
                    bad_gte ? "GTE data/command use" : "regs/stores/return", line);
    }
    printf("checked %u cases, %u mismatches\n", n, bad);
    return bad != 0;
}
