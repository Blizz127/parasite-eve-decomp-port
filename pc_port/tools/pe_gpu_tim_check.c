/* Vector check for pe_plat_gpu_tim.c (guest 0x800718D0):
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu_tim_check.c pc_port/platform/pe_plat_gpu_tim.c -Wl,--unresolved-symbols=ignore-all -o check
 *   ./check vectors.txt
 * Line (pc_port/tools/pe_gpu_tim_vectors.py): ENTRY seed pattern A tim RET r
 * L addr:size=value... S - RETS v CALL 8007506C,a0,a1@stores#RECTHEX ...
 * Memory starts as poison.  Each byte the original loaded gets the value it
 * loaded, and the eight rectangle bytes each recorded upload call would read
 * (what the callee, not the original, reads) are made readable too; the code
 * under test may read only those bytes.  The upload callee is stubbed under
 * its host name: it records the rectangle it is handed (compared with the
 * original's eight bytes), the data address (compared with the original's
 * second argument) and returns the scripted RETS value.  Calls must match in
 * number and order; there must be no stores; the return value must match.
 * Controls: compile this checker against mutated copies of the provider (call order
 * swapped, palette call dropped, wrong flag bit, size word ignored, wrong data
 * pointer, extra size-word read) and a no-op; each must fail (see REPORT.md).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"
#define MEM 0x200000u
static uint8_t ram[MEM];
static uint32_t readable[MEM], cur;
static int bad_read, nstore;
static void rd(pe_addr_t a, unsigned n)
{
    unsigned i;
    for (i = 0; i < n; i++)
        if (readable[(a + i) & (MEM - 1)] != cur) bad_read = 1;
}
uint8_t PE_LoadU8(pe_addr_t a) { rd(a, 1); return ram[a & (MEM - 1)]; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; rd(a, 2); memcpy(&v, ram + (a & (MEM - 1)), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; rd(a, 4); memcpy(&v, ram + (a & (MEM - 1)), 4); return v; }
void PE_StoreU8(pe_addr_t a, uint8_t v) { (void)a; (void)v; nstore++; }
void PE_StoreU16(pe_addr_t a, uint16_t v) { (void)a; (void)v; nstore++; }
void PE_StoreU32(pe_addr_t a, uint32_t v) { (void)a; (void)v; nstore++; }
#define MAXCALL 8
static struct { uint8_t rect[8]; uint32_t data; } got[MAXCALL];
static unsigned ngot;
static uint32_t scripted;
int pe_plat_gpu_load_rect_host(const RECT *r, uint32_t data)
{
    if (ngot < MAXCALL) {
        memcpy(got[ngot].rect, r, 8);
        got[ngot].data = data;
    }
    ngot++;
    return (int)scripted;
}
extern uint32_t pe_plat_gpu_load_tim(uint32_t tim);
static int hexv(const char *s, uint8_t *dst, unsigned n)
{
    unsigned i;
    for (i = 0; i < n; i++) {
        unsigned b;
        if (sscanf(s + 2 * i, "%2x", &b) != 1) return 0;
        dst[i] = (uint8_t)b;
    }
    return 1;
}
int main(int c, char **v)
{
    FILE *f; static char line[65536]; unsigned n = 0, bad = 0;
    if (c != 2 || !(f = fopen(v[1], "r"))) return 2;
    while (fgets(line, sizeof line, f)) {
        static char *tok[4096]; static char copy[65536];
        int nt = 0, ok = 1, k, ri, ci;
        uint32_t tim, wret, ret;
        unsigned nexp = 0;
        static struct { uint8_t rect[8]; uint32_t data; } exp[MAXCALL];
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < 4096 && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
        if (nt < 8 || strcmp(tok[3], "A")) continue;
        for (ri = 4; ri < nt && strcmp(tok[ri], "RET"); ri++) ;
        for (ci = ri; ci < nt && strcmp(tok[ci], "CALL"); ci++) ;
        if (ci >= nt) continue;
        tim = (uint32_t)strtoul(tok[4], 0, 16);
        wret = (uint32_t)strtoul(tok[ri + 1], 0, 16);
        cur = ++n; bad_read = 0; nstore = 0; ngot = 0;
        for (k = 0; k < nt; k++)
            if (!strcmp(tok[k], "RETS")) scripted = (uint32_t)strtoul(tok[k + 1], 0, 16);
        for (k = ri + 3; k < nt && strcmp(tok[k], "S"); k++) {   /* L entries */
            unsigned addr, size, val, i;
            if (!strcmp(tok[k], "-")) continue;
            if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) != 3 || size == 0 || size > 4) { ok = 0; break; }
            for (i = 0; i < size; i++) {
                uint32_t b = (addr + i) & (MEM - 1);
                if (readable[b] != cur) { ram[b] = (uint8_t)(val >> (8 * i)); readable[b] = cur; }
            }
        }
        for (k = ci + 1; k < nt && nexp < MAXCALL; k++) {
            unsigned a0, a1, fn, i; char *h;
            if (!strcmp(tok[k], "-")) continue;
            if (sscanf(tok[k], "%x,%x,%x@", &fn, &a0, &a1) != 3 || !(h = strchr(tok[k], '#')) ||
                !hexv(h + 1, exp[nexp].rect, 8)) { ok = 0; break; }
            exp[nexp].data = a1;
            for (i = 0; i < 8; i++) {
                uint32_t b = (a0 + i) & (MEM - 1);
                if (readable[b] != cur) { ram[b] = exp[nexp].rect[i]; readable[b] = cur; }
            }
            nexp++;
        }
        ret = pe_plat_gpu_load_tim(tim);
        if (ret != wret || bad_read || nstore || ngot != nexp) ok = 0;
        for (k = 0; ok && (unsigned)k < nexp; k++)
            if (memcmp(got[k].rect, exp[k].rect, 8) || got[k].data != exp[k].data) ok = 0;
        if (!ok && bad++ < 12)
            fprintf(stderr, "MISMATCH line %u (ret %08X want %08X, calls %u want %u%s%s): %.160s\n", n, ret, wret,
                    ngot, nexp, bad_read ? ", read outside original loads" : "", nstore ? ", store" : "", line);
    }
    printf("checked %u cases, %u mismatches\n", n, bad);
    return bad != 0;
}
