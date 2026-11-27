/* Vector check for pe_plat_cd_leaf.c (and CD_flush, `flush` vectors),
 * pe_plat_spu_reverb.c (`reverb` vectors) and the existing SPU providers
 * pe_plat_spu_store_value / pe_plat_spu_heap_contains in pe_plat_state.c
 * (`spucallees` vectors), pe_plat_card_leaf.c (libcard 0x80082ADC /
 * 0x80082CDC), pe_plat_pad_rc2.c (libpad RCNT2 pair 0x80084FC4 /
 * 0x80084FE4) and pe_plat_card_sio.c (libcard SIO poll 0x80083578), all
 * from ENTRY vectors.  pe_plat_state.c references runtime services the
 * checked functions never reach, so those stay unresolved:
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cd_leaf_vectors_check.c pc_port/platform/pe_plat_cd_leaf.c pc_port/platform/pe_plat_cd_data_ready.c pc_port/platform/pe_plat_spu_reverb.c pc_port/platform/pe_plat_state.c pc_port/platform/pe_plat_card_leaf.c pc_port/platform/pe_plat_pad_rc2.c pc_port/platform/pe_plat_card_sio.c -Wl,--unresolved-symbols=ignore-all -o check
 *   ./check vectors.txt
 * Line: ENTRY seed pattern A args... RET r L addr:size=value... S addr:size=value...
 * (`-` for an empty list), from pc_port/tools/pe_cd_leaf_vectors.py.
 * Memory starts as poison.  Each byte the original loaded gets the value it
 * loaded (first load wins); the code under test may read only those bytes.
 * After the call, the set of bytes written and their final values must equal
 * the original's stores replayed in order.  Register stores (addresses
 * 0x1F80xxxx) are commands, not memory values: the code under test must
 * issue exactly the original's register stores, in the same order, with the
 * same sizes and values.  A trailing CALL list (`callback` vectors, for
 * data_ready_callback 0x8007C214, whose native provider is checked here) gives the original's callback jumps: the code under
 * test must call PE_GuestHook_Call with the same targets in the same order.
 * Register reads are replayed in the original's
 * order (same address and size), each returning the value the original read;
 * the code under test must make exactly those reads.  RET is compared for the
 * entries whose port returns a value (call() returns 1). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_cdreg.h"
#include "pe_plat/cd.h"
#include "pe_plat/audio.h"
#include "pe_plat/state.h"
#include "pe_plat/bios_file.h"
#include "pe_plat/pad.h"
#include "pe_guestcode.h"
#define MEM 0x200000u
static uint8_t ram[MEM], want[MEM];
static uint32_t readable[MEM], wrote[MEM], want_set[MEM], cur;
static uint32_t wlog[4096], nwlog, xlog[4096], nxlog;
static int bad_read;
static void rd(pe_addr_t a, unsigned n)
{
    unsigned i;
    for (i = 0; i < n; i++)
#ifndef CHK_STRICT_READS
        /* a byte the code under test stored itself earlier in this case may be read back (the original does the same and
         * the generator no longer logs such own-written loads); -DCHK_STRICT_READS restores the old rule */
        if (wrote[(a + i) & (MEM - 1)] == cur) { /* own-written */ } else
#endif
        if (readable[(a + i) & (MEM - 1)] != cur) bad_read = 1;
}
/* Register stores by the code under test, in order. */
static uint32_t mlog[256][3], nmlog;
static void mmio(pe_addr_t a, unsigned n, uint32_t v)
{
    if ((a >> 16) == 0x1F80u && nmlog < 256) {
        mlog[nmlog][0] = a; mlog[nmlog][1] = n; mlog[nmlog][2] = v; nmlog++;
    }
}
static void wr(pe_addr_t a, unsigned n)
{
    unsigned i;
    for (i = 0; i < n; i++) {
        uint32_t b = (a + i) & (MEM - 1);
        if (wrote[b] != cur) { wrote[b] = cur; if (nwlog < 4096) wlog[nwlog++] = b; }
    }
}
/* Register reads are hardware inputs: they are replayed in the original's
 * order (same address and size), each returning the value the original
 * read; reads beyond, or out of, that sequence fail the case. */
static uint32_t rq[256][3], nrq, rq_pos;
static int mmio_read(pe_addr_t a, unsigned n, uint32_t *v)
{
    if ((a >> 16) != 0x1F80u) return 0;
    if (rq_pos >= nrq || rq[rq_pos][0] != a || rq[rq_pos][1] != n) { bad_read = 1; *v = 0; return 1; }
    *v = rq[rq_pos++][2];
    return 1;
}
uint8_t PE_LoadU8(pe_addr_t a) { uint32_t m; if (mmio_read(a, 1, &m)) return (uint8_t)m; rd(a, 1); return ram[a & (MEM - 1)]; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; uint32_t m; if (mmio_read(a, 2, &m)) return (uint16_t)m; rd(a, 2); memcpy(&v, ram + (a & (MEM - 1)), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; if (mmio_read(a, 4, &v)) return v; rd(a, 4); memcpy(&v, ram + (a & (MEM - 1)), 4); return v; }
void PE_StoreU8(pe_addr_t a, uint8_t v) { mmio(a, 1, v); wr(a, 1); ram[a & (MEM - 1)] = v; }
void PE_StoreU16(pe_addr_t a, uint16_t v) { mmio(a, 2, v); wr(a, 2); memcpy(ram + (a & (MEM - 1)), &v, 2); }
void PE_StoreU32(pe_addr_t a, uint32_t v) { mmio(a, 4, v); wr(a, 4); memcpy(ram + (a & (MEM - 1)), &v, 4); }
/* The oracle supplies ordered device reads; production uses pe_cdreg.
 * Keep the same replay checks for the address-decoding API. */
uint8_t PE_CdLoadU8(pe_addr_t a) { return PE_LoadU8(a); }
void PE_CdStoreU8(pe_addr_t a, uint8_t v) { PE_StoreU8(a, v); }
void PE_CdStoreU32(pe_addr_t a, uint32_t v) { PE_StoreU32(a, v); }
/* Guest callbacks: the code under test must jump to the same callback
 * addresses in the same order as the original (CALL list); each call
 * returns 0 in v0, as the harness did. Data-ready callback vectors also
 * compare all four register values and require the four-register hook ABI. */
int PE_Port_ShouldStop(void) { return 0; }
static uint32_t cb_got[16], ncb_got, cb_args[16][4], cb_arity[16];
int PE_GuestHook_Call(const char *slot, const char *caller, pe_addr_t fn,
                      unsigned arity, uintptr_t a0, uintptr_t a1,
                      uintptr_t a2, uintptr_t a3, int *ret)
{
    (void)slot; (void)caller; (void)arity; (void)a0; (void)a1; (void)a2; (void)a3;
    if (ncb_got < 16) {
        cb_got[ncb_got] = fn; cb_arity[ncb_got] = arity;
        cb_args[ncb_got][0]=a0; cb_args[ncb_got][1]=a1;
        cb_args[ncb_got][2]=a2; cb_args[ncb_got][3]=a3;
    }
    ncb_got++;
    if (ret) *ret = 0;
    return 1;
}
static int call(uint32_t entry, uint32_t *a, uint32_t *ret)
{
    switch (entry) {
    case 0x8007B964u: pe_plat_cd_set_vol(a[0]); return 0;
    case 0x8007BAC0u: *ret = (uint32_t)pe_plat_cd_init_vol(); return 1;
    case 0x8007C394u: pe_plat_cd_st_free_ring(a[0]); return 0;
    case 0x8007C484u: *ret = (uint32_t)pe_plat_cd_st_get_next(a[0], a[1]); return 1;
    case 0x8007C214u: pe_plat_cd_data_ready_callback(); return 0;
    case 0x8007B9ECu: pe_plat_cd_flush(); return 0;
    case 0x8008D140u: pe_plat_spu_set_reverb_attr(a[0]); return 0;
    case 0x8007DAE0u: pe_plat_spu_store_value((int)a[0], a[1], (int)a[2]); return 0;
    case 0x80085BB4u: *ret = (uint32_t)pe_plat_spu_heap_contains(a[0]); return 1;
    case 0x80082ADCu: pe_plat_card_init_timer_callbacks(); return 0;
    case 0x80082CDCu: *ret = (uint32_t)pe_plat_card_word_update(); return 1;
    case 0x80084FC4u: pe_plat_pad_rc2_wait_set(a[0]); return 0;
    case 0x80084FE4u: *ret = (uint32_t)pe_plat_pad_rc2_wait_check(); return 1;
    case 0x80083578u: pe_plat_card_sio_rx_wait(); return 0;
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
        int nt = 0, i, ri, li, si, ci, has, ok = 1, na; uint32_t entry, a[8] = {0}, wret, got = 0; unsigned k;
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < 4096 && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
        if (nt < 8 || strcmp(tok[3], "A")) continue;
        ri = find(tok, nt, 4, "RET"); li = ri + 2; si = find(tok, nt, li, "S");
        if (si >= nt || li >= nt || strcmp(tok[li], "L")) continue;
        entry = (uint32_t)strtoul(tok[0], 0, 16);
        na = ri - 4;
        for (i = 0; i < na && i < 8; i++) if (strcmp(tok[4 + i], "-")) a[i] = (uint32_t)strtoul(tok[4 + i], 0, 16);
        wret = (uint32_t)strtoul(tok[ri + 1], 0, 16);
        cur = ++n; nwlog = nxlog = 0; bad_read = 0; nmlog = 0; nrq = rq_pos = 0; ncb_got = 0;
        for (k = (unsigned)li + 1; k < (unsigned)si; k++) {
            unsigned addr, size, val;
            if (!strcmp(tok[k], "-")) continue;
            if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) == 3 && (addr >> 16) == 0x1F80u) {
                if (nrq < 256) { rq[nrq][0] = addr; rq[nrq][1] = size; rq[nrq][2] = val; nrq++; }
                continue;      /* register read: replayed in order, not memory */
            }
            apply(tok[k], ram, readable, 1);
        }
        ci = si + 1;
        while (ci < nt && strcmp(tok[ci], "CALL")) ci++;
        for (k = (unsigned)si + 1; k < (unsigned)ci; k++) if (strcmp(tok[k], "-")) apply(tok[k], want, want_set, 0);
        has = call(entry, a, &got);
        if (has < 0) { fprintf(stderr, "unknown entry %08X\n", entry); return 2; }
        if (has && got != wret) ok = 0;
        if (bad_read) ok = 0;
        if (nwlog != nxlog) ok = 0;
        if (rq_pos != nrq) ok = 0;     /* every register read the original made */
        {   /* callback jumps: same targets, same order */
            unsigned want_c = 0, fn;
            for (k = (unsigned)ci + 1; k < (unsigned)nt; k++) {
                if (!strcmp(tok[k], "-") || sscanf(tok[k], "%x", &fn) != 1) continue;
                if (want_c >= ncb_got || (want_c < 16 && cb_got[want_c] != fn)) ok = 0;
                if (entry==0x8007C214u) {
                    unsigned args[4], target;
                    if (sscanf(tok[k], "%x,%x,%x,%x,%x", &target, &args[0], &args[1], &args[2], &args[3]) != 5 || want_c>=16) ok=0;
                    else {
                        if (cb_arity[want_c]!=4u) ok=0;
                        for (unsigned j=0;j<4;j++) if (cb_args[want_c][j]!=args[j]) ok=0;
                    }
                }
                want_c++;
            }
            if (want_c != ncb_got) ok = 0;
        }
        {   /* register stores: same sequence as the original's */
            unsigned want_m = 0, addr, size, val;
            for (k = (unsigned)si + 1; k < (unsigned)ci; k++) {
                if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) != 3 || (addr >> 16) != 0x1F80u) continue;
                if (want_m >= nmlog || mlog[want_m][0] != addr || mlog[want_m][1] != size ||
                    mlog[want_m][2] != (val & (size == 4 ? 0xFFFFFFFFu : (1u << (8 * size)) - 1u))) ok = 0;
                want_m++;
            }
            if (want_m != nmlog) ok = 0;
        }
        for (k = 0; k < nwlog && ok; k++)
            if (want_set[wlog[k]] != cur || ram[wlog[k]] != want[wlog[k]]) ok = 0;
        if (!ok && bad++ < 12)
            fprintf(stderr, "MISMATCH line %u (%s%s): %.200s\n", n, bad_read ? "read outside original loads; " : "",
                    "stores/return", line);
    }
    printf("checked %u cases, %u mismatches\n", n, bad);
    return bad != 0;
}
