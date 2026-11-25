/* Vector check for pe_plat_gte_cmd.c, pe_plat_gte_cmd2.c (batch 2) and
 * pe_plat_gte_div.c (batch 3: SetFogNearFar, SquareRoot0, ratan2) and
 * pe_plat_gte_vn.c (batch 4: VectorNormal) and pe_plat_gte_lv.c (batch 5:
 * ApplyMatrixLV),
 * against the real host GTE model:
 *   cc -std=gnu99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/game \
 *      pc_port/tools/pe_gte_cmd_vectors_check.c pc_port/platform/pe_plat_gte_cmd.c \
 *      pc_port/platform/pe_plat_gte_cmd2.c pc_port/platform/pe_plat_gte_div.c \
 *      pc_port/platform/pe_plat_gte_vn.c pc_port/platform/pe_plat_gte_lv.c \
 *      pc_port/platform/pe_gte.c pc_port/platform/pe_gte_commands.c -o check
 *   ./check vectors.txt
 * Line: ENTRY seed pattern A args... RET r C0 c.. D0 d.. C1 c.. D1 d.. F f L loads S stores
 * (from pc_port/tools/pe_gte_cmd_vectors.py; C/D = the 32 GTE control/data
 * registers before and after, F = interpreter FLAG valid).
 * Per case:
 * - the host model is set from C0/D0 (PE_GTE_WriteCtrl / WriteData; data
 *   15, 28, 29, 31 are derived and not written) and read back; a case whose
 *   state does not survive the round trip is counted as INIT and not judged;
 * - memory starts as poison; only bytes the original loaded before storing
 *   to them are readable at entry (loads marked ^ came after the original's
 *   own store and are not pre-seeded); bytes the code under test writes
 *   become readable to it;
 *   the set of bytes written and their final values must equal the
 *   original's stores replayed in order; RET must match (not for the void
 *   MulRotMatrix 0x800786E4);
 * - afterwards every register read through PE_GTE_ReadCtrl / ReadData must
 *   equal the interpreter's final register under psx-spx read widths (the
 *   same canon rules as pe_gte_cmd_diff.c); FLAG only when F is 1.
 * Every command word these leaves use passes pe_gte_cmd_diff (host model
 * and interpreter agree on every register). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/gte.h"
#include "game_port.h"

uint32_t PE_GTE_ReadData(uint32_t reg);
void PE_GTE_WriteData(uint32_t reg, uint32_t value);
uint32_t PE_GTE_ReadCtrl(uint32_t reg);
void PE_GTE_WriteCtrl(uint32_t reg, uint32_t value);

#define MEM 0x200000u
static uint8_t ram[MEM], want[MEM];
static uint32_t readable[MEM], wrote[MEM], want_set[MEM], cur;
static uint32_t wlog[4096], nwlog, xlog[4096], nxlog;
static int bad_read, stopped;
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
        readable[b] = cur;   /* the code under test may read back its own writes */
    }
}
uint8_t PE_LoadU8(pe_addr_t a) { rd(a, 1); return ram[a & (MEM - 1)]; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; rd(a, 2); memcpy(&v, ram + (a & (MEM - 1)), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; rd(a, 4); memcpy(&v, ram + (a & (MEM - 1)), 4); return v; }
void PE_StoreU8(pe_addr_t a, uint8_t v) { wr(a, 1); ram[a & (MEM - 1)] = v; }
void PE_StoreU16(pe_addr_t a, uint16_t v) { wr(a, 2); memcpy(ram + (a & (MEM - 1)), &v, 2); }
void PE_StoreU32(pe_addr_t a, uint32_t v) { wr(a, 4); memcpy(ram + (a & (MEM - 1)), &v, 4); }
static int stop_reason_ok;
void PE_Port_RequestStop(PEPortStopReason reason)
{
    stopped = 1;
    stop_reason_ok = reason == PE_PORT_STOP_UNRESOLVED_BOUNDARY;
}

static uint32_t sx16(uint32_t v) { return (uint32_t)(int32_t)(int16_t)(uint16_t)v; }
static uint32_t lzcr(uint32_t v)
{
    uint32_t bits = (v & 0x80000000u) ? ~v : v, n = 0;
    while (n < 32 && !(bits & (0x80000000u >> n))) n++;
    return n;
}
static void canon_data(const uint32_t *d, uint32_t *o)
{
    static const int s16[] = {1, 3, 5, 8, 9, 10, 11}, u16[] = {7, 16, 17, 18, 19};
    uint32_t i, rgb = 0;
    memcpy(o, d, 32 * 4);
    for (i = 0; i < 7; i++) o[s16[i]] = sx16(d[s16[i]]);
    for (i = 0; i < 5; i++) o[u16[i]] = d[u16[i]] & 0xFFFFu;
    o[15] = o[14];
    for (i = 0; i < 3; i++) {
        int32_t c = (int32_t)o[9 + i] >> 7;
        c = c < 0 ? 0 : c > 0x1F ? 0x1F : c;
        rgb |= (uint32_t)c << (5 * i);
    }
    o[28] = o[29] = rgb;
    o[31] = lzcr(d[30]);
}
static void canon_ctrl(const uint32_t *c, uint32_t *o)
{
    static const int s16[] = {4, 12, 20, 26, 27, 29, 30};
    uint32_t i, f;
    memcpy(o, c, 32 * 4);
    for (i = 0; i < 7; i++) o[s16[i]] = sx16(c[s16[i]]);
    f = c[31] & 0x7FFFF000u;
    o[31] = f | ((f & 0x7F87E000u) ? 0x80000000u : 0);
}
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    switch (entry) {
    case 0x80078134u: *ret = pe_plat_gte_vector_normal(a[0], a[1]); return 1;
    case 0x80077E64u: pe_plat_gte_set_fog_near_far(a[0], a[1], (int32_t)a[2]); return 0;
    case 0x80078004u: *ret = pe_plat_gte_square_root0(a[0]); return 1;
    case 0x80079FB4u: *ret = (uint32_t)pe_plat_gte_ratan2((int32_t)a[0], (int32_t)a[1]); return 1;
    case 0x800783E4u: pe_plat_gte_load_average_short12(a[0], a[1], (int32_t)a[2], (int32_t)a[3], a[4]); return 0;
    case 0x80078554u: pe_plat_gte_load_average_col(a[0], a[1], (int32_t)a[2], (int32_t)a[3], a[4]); return 0;
    case 0x800791D0u: pe_plat_gte_outer_product0(a[0], a[1], a[2]); return 0;
    case 0x80079244u: *ret = (uint32_t)pe_plat_gte_rot_trans_pers(a[0], a[1], a[2], a[3]); return 1;
    case 0x80079274u: *ret = (uint32_t)pe_plat_gte_rot_trans_pers3(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]); return 1;
    case 0x800792D4u: *ret = pe_plat_gte_rot_trans(a[0], a[1], a[2]); return 1;
    case 0x80079384u: *ret = (uint32_t)pe_plat_gte_rot_average_nclip3(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8]); return 1;
    case 0x80079414u: *ret = (uint32_t)pe_plat_gte_rot_average_nclip4(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10]); return 1;
    case 0x800785D4u: *ret = pe_plat_gte_mul_matrix0(a[0], a[1], a[2]); return 1;
    case 0x800786E4u: pe_plat_gte_mul_rot_matrix(a[0]); return 0;
    case 0x800787D4u: *ret = pe_plat_gte_comp_matrix(a[0], a[1], a[2]); return 1;
    case 0x80078BE4u: *ret = pe_plat_gte_apply_matrix(a[0], a[1], a[2]); return 1;
    case 0x80078C34u: *ret = pe_plat_gte_apply_matrix_sv(a[0], a[1], a[2]); return 1;
    case 0x80078934u: *ret = pe_plat_gte_apply_matrix_lv(a[0], a[1], a[2]); return 1;
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
static void regs(char **tok, int at, uint32_t *v)
{
    int i;
    for (i = 0; i < 32; i++) v[i] = (uint32_t)strtoul(tok[at + 1 + i], 0, 16);
}
int main(int c, char **v)
{
    static char line[65536], copy[65536], *tok[4096];
    unsigned n = 0, bad = 0, init_bad = 0;
    FILE *f;
    if (c != 2 || !(f = fopen(v[1], "r"))) return 2;
    while (fgets(line, sizeof line, f)) {
        int nt = 0, i, ri, c0, li, si, has, ok = 1, na, fv, trap;
        uint32_t entry, a[12] = {0}, wret, got = 0, C0[32], D0[32], C1[32], D1[32], wc[32], wd[32];
        unsigned k;
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < 4096 && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
        if (nt < 8 || strcmp(tok[3], "A")) continue;
        ri = find(tok, nt, 4, "RET"); c0 = ri + 2;
        if (c0 + 136 >= nt || strcmp(tok[c0], "C0") || strcmp(tok[c0 + 33], "D0") ||
            strcmp(tok[c0 + 66], "C1") || strcmp(tok[c0 + 99], "D1") || strcmp(tok[c0 + 132], "F")) continue;
        li = c0 + 134; si = find(tok, nt, li, "S");
        if (strcmp(tok[li], "L") || si >= nt) continue;
        entry = (uint32_t)strtoul(tok[0], 0, 16);
        na = ri - 4;
        for (i = 0; i < na && i < 12; i++) a[i] = (uint32_t)strtoul(tok[4 + i], 0, 16);
        trap = !strcmp(tok[ri + 1], "TRAP");
        wret = trap ? 0 : (uint32_t)strtoul(tok[ri + 1], 0, 16);
        regs(tok, c0, C0); regs(tok, c0 + 33, D0); regs(tok, c0 + 66, C1); regs(tok, c0 + 99, D1);
        fv = atoi(tok[c0 + 133]);
        cur = ++n; nwlog = nxlog = 0; bad_read = stopped = stop_reason_ok = 0;
        for (i = 0; i < 32; i++) PE_GTE_WriteCtrl((uint32_t)i, C0[i]);
        for (i = 0; i < 31; i++)
            if (i != 15 && i != 28 && i != 29) PE_GTE_WriteData((uint32_t)i, D0[i]);
        for (i = 0; i < 32; i++)
            if (PE_GTE_ReadCtrl((uint32_t)i) != C0[i] || PE_GTE_ReadData((uint32_t)i) != D0[i]) ok = 0;
        if (!ok) { init_bad++; continue; }
        for (k = (unsigned)li + 1; k < (unsigned)si; k++)   /* ^ = read after the original's own store */
            if (strcmp(tok[k], "-") && !strchr(tok[k], '^')) apply(tok[k], ram, readable, 1);
        for (k = (unsigned)si + 1; k < (unsigned)nt; k++) if (strcmp(tok[k], "-")) apply(tok[k], want, want_set, 0);
        has = call(entry, a, &got);
        if (has < 0) { fprintf(stderr, "unknown entry %08X\n", entry); return 2; }
        if (has && !trap && got != wret) ok = 0;
        if (bad_read || stopped != trap || (trap && !stop_reason_ok)) ok = 0;
        if (nwlog != nxlog) ok = 0;
        for (k = 0; k < nwlog && ok; k++)
            if (want_set[wlog[k]] != cur || ram[wlog[k]] != want[wlog[k]]) ok = 0;
        canon_ctrl(C1, wc);
        canon_data(D1, wd);
        for (i = 0; i < 32 && !trap; i++) {
            if ((i != 31 || fv) && PE_GTE_ReadCtrl((uint32_t)i) != wc[i]) ok = 0;
            if (PE_GTE_ReadData((uint32_t)i) != wd[i]) ok = 0;
        }
        if (!ok && bad++ < 12) {
            fprintf(stderr, "MISMATCH line %u (%s%s):", n, bad_read ? "read outside original loads; " : "",
                    stopped ? "stop requested; " : "");
            for (i = 0; i < 32; i++) {
                uint32_t r;
                if ((i != 31 || fv) && (r = PE_GTE_ReadCtrl((uint32_t)i)) != wc[i]) fprintf(stderr, " c%d %X!=%X", i, r, wc[i]);
                if ((r = PE_GTE_ReadData((uint32_t)i)) != wd[i]) fprintf(stderr, " d%d %X!=%X", i, r, wd[i]);
            }
            fprintf(stderr, " | %.120s\n", line);
        }
    }
    printf("checked %u cases, %u mismatches, %u init (not judged)\n", n, bad, init_bad);
    return (bad || init_bad) ? 1 : 0;
}
