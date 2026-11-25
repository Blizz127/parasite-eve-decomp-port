/* Vector check for pe_plat_pad_leaf.c and pe_plat_pad_state.c:
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform pc_port/tools/pe_pad_leaf_vectors_check.c pc_port/platform/pe_plat_pad_leaf.c pc_port/platform/pe_plat_pad_state.c pc_port/platform/pe_plat_pad_actinfo.c -o check
 *   ./check vectors.txt
 * Line: ENTRY seed pattern A a0 a1 a2 a3 RET r L addr:size=value... S addr:size=value...
 * (`-` for an empty list), from pc_port/tools/pe_pad_leaf_vectors.py.
 * Memory starts as poison.  Each byte the original loaded gets the value it
 * loaded (first load wins); the code under test may read only those bytes.
 * After the call, the set of bytes written and their final values must equal
 * the original's stores replayed in order.  RET is compared for the entries
 * with a return value in the port (0x80084B20, 0x80084F8C, 0x80084AE8,
 * 0x80083790, 0x80083644, 0x80083C3C, 0x8008284C, 0x800837C8).
 * pe_plat_pad_record (the host's controller record lookup, standing in for
 * the original's D_8009B738 hook) may be called only by 0x8008284C: it returns
 * 0x800A5C60 if (port & 0xF0), else 0x800A5B70.  Every other use of it, and
 * any use of pe_plat_pad_busy, fails the case. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/pad.h"
#define MEM 0x200000u
/* Per-case stamps: a byte is readable / written / expected when its stamp
 * equals the current case number. */
static uint8_t ram[MEM], want[MEM];
static uint32_t readable[MEM], wrote[MEM], want_set[MEM], cur;
static uint32_t wlog[4096], nwlog, xlog[4096], nxlog;
static int bad_read;
static uint32_t cur_entry;
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
/* Services the leaves must not need. */
uint32_t pe_plat_pad_record(uint32_t port)
{
    if (cur_entry != 0x8008284Cu) { bad_read = 1; return 0; }
    return (port & 0xF0u) ? 0x800A5C60u : 0x800A5B70u;
}
int pe_plat_pad_busy(uint32_t record) { (void)record; bad_read = 1; return 0; }
static int call(uint32_t entry, uint32_t *a, uint32_t *ret)
{
    switch (entry) {
    case 0x80084B20u: *ret = pe_plat_pad_port_record((int)a[0]); return 1;
    case 0x80084F8Cu: *ret = (uint32_t)pe_plat_pad_record_busy(a[0]); return 1;
    case 0x80084AE8u: *ret = (uint32_t)pe_plat_pad_record_port(a[0]); return 1;
    case 0x800847A0u: pe_plat_pad_cmd_shift(a[0]); return 0;
    case 0x80083E70u: pe_plat_pad_cmd_e(a[0]); return 0;
    case 0x80083EE4u: pe_plat_pad_cmd_k(a[0]); return 0;
    case 0x80083C20u: pe_plat_pad_cmd_m(a[0]); return 0;
    case 0x80083E50u: pe_plat_pad_cmd_c(a[0], (unsigned char)a[1]); return 0;
    case 0x80083E84u: pe_plat_pad_cmd_l(a[0], (unsigned char)a[1]); return 0;
    case 0x80083EA4u: pe_plat_pad_cmd_f(a[0], (unsigned char)a[1]); return 0;
    case 0x80083EC4u: pe_plat_pad_cmd_g(a[0], (unsigned char)a[1]); return 0;
    case 0x800835C0u: pe_plat_pad_send_step(a[0]); return 0;
    case 0x8008389Cu: pe_plat_pad_query_step(a[0]); return 0;
    case 0x80083790u: *ret = (uint32_t)pe_plat_pad_act_size(a[0]); return 1;
    case 0x80083D9Cu: pe_plat_pad_main_handler(a[0]); return 0;
    case 0x80084644u: pe_plat_pad_record_init(a[0]); return 0;
    case 0x80084B44u: pe_plat_pad_install_hooks(); return 0;
    case 0x80083644u: *ret = (uint32_t)pe_plat_pad_recv_step(a[0]); return 1;
    case 0x80083C3Cu: *ret = (uint32_t)pe_plat_pad_align_handler(a[0]); return 1;
    case 0x8008284Cu: *ret = (uint32_t)pe_plat_pad_info_comb((int)a[0], (int)a[1], (int)a[2]); return 1;
    case 0x800837C8u: *ret = (uint32_t)pe_plat_pad_load_act_info(a[0], (int)a[1]); return 1;
    }
    return -1;
}
/* Parse "addr:size=value" and copy its bytes into dst, stamping set[].
 * first_only keeps the first value seen for a byte (initial memory);
 * otherwise later values overwrite (stores replayed in order) and each new
 * byte is logged in xlog. */
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
int main(int c, char **v)
{
    FILE *f; static char line[262144]; unsigned n = 0, bad = 0;
    if (c != 2 || !(f = fopen(v[1], "r"))) return 2;
    while (fgets(line, sizeof line, f)) {
        static char *tok[16384]; int nt = 0, i, li, si, has, ok = 1; uint32_t entry, a[4], wret, got = 0; unsigned k;
        static char copy[262144];
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < 16384 && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
        if (nt < 13 || strcmp(tok[3], "A") || strcmp(tok[8], "RET") || strcmp(tok[10], "L")) continue;
        entry = (uint32_t)strtoul(tok[0], 0, 16);
        for (i = 0; i < 4; i++) a[i] = (uint32_t)strtoul(tok[4 + i], 0, 16);
        wret = (uint32_t)strtoul(tok[9], 0, 16);
        for (si = 11; si < nt && strcmp(tok[si], "S"); si++) {}
        if (si >= nt) continue;
        cur = ++n; nwlog = nxlog = 0; bad_read = 0;
        for (li = 11; li < si; li++) if (strcmp(tok[li], "-")) apply(tok[li], ram, readable, 1);
        for (li = si + 1; li < nt; li++) if (strcmp(tok[li], "-")) apply(tok[li], want, want_set, 0);
        cur_entry = entry;
        has = call(entry, a, &got);
        if (has < 0) { fprintf(stderr, "unknown entry %08X\n", entry); return 2; }
        if (has && got != wret) ok = 0;
        if (bad_read) ok = 0;
        if (nwlog != nxlog) ok = 0;
        for (k = 0; k < nwlog && ok; k++)
            if (want_set[wlog[k]] != cur || ram[wlog[k]] != want[wlog[k]]) ok = 0;
        if (!ok && bad++ < 12)
            fprintf(stderr, "MISMATCH line %u (%s%s): %.240s", n, bad_read ? "read outside original loads; " : "",
                    has && got != wret ? "return" : "stores", line);
    }
    printf("checked %u cases, %u mismatches\n", n, bad);
    return bad != 0;
}
