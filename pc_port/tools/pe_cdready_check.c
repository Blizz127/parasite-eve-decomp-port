/* Vector check for the independent re-implementations of libcd CD_ready
 * (0x8007B290, pe_plat_cd_ready.c) and the libds-block transfer-step gate
 * (0x80081110, pe_plat_81110.c), from pe_cd_leaf_vectors.py mode cdgen36.
 * Build and run from the repo root:
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdready_check.c pc_port/platform/pe_plat_cd_ready.c pc_port/platform/pe_plat_81110.c -o check
 *   ./check vectors.txt
 * Each case seeds guest RAM from the first load of every address in the
 * vector's L list (register loads 0x1F80xxxx are replayed in order), runs the
 * native with its callees replaced by the models below, and requires: the
 * same return value; every non-register byte the original loaded is also
 * loaded by the native (a load the native skips would hide a missing arm);
 * the same bytes written (address and value); the same
 * register stores in order; every register read of the original consumed; no
 * read outside the original's loads; the same callee calls in order, each
 * with the original's arguments and made after the same number of distinct
 * written bytes (@n).
 * Callee models (the vector's RETS entries are their parameters):
 *   VSync: the n-th call returns base + n * step.
 *   CheckCallback / getintr: return `pos` for the first k calls, then `final`
 *     (code = k | pos index << 8 | final index << 12); CheckCallback is 16 bits.
 *   CD_ready RETS = printf, VSync base, puts, CheckCallback, getintr, CD_flush,
 *     VSync step, getintr memory-effect code (bit 0: on; bits 1-2: call m;
 *     bits 3-4 / 5-6: values for the ready / sync bytes; bit 7: response bytes),
 *     sync-callback result, ready-callback result (the sentinels 0x80130C40 /
 *     0x80130C80 or their aliases, compared by target, $a0 and $a1).
 *   0x80081110 RETS = VSync base, 0x80081268 return, VSync step, callback return,
 *     pump effect (bit 0: the pump rewrites the counter word 0x8009B6B4 with
 *     -1, 0, 5 or 0x80000000 chosen by bits 1-2).
 * The port-contract hooks (stop request, poll tick) are no-ops here; a
 * requested stop fails the case. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_cdreg.h"
#include "game_port.h"
#include "pe_bios_string.h"
#include "pe_plat/cd.h"
#include "pe_plat/state.h"
#include "pe_plat/bios_file.h"
#include "pe_sdk.h"

#define MEM 0x200000u
static uint8_t ram[MEM], want[MEM];
static uint32_t readable[MEM], wrote[MEM], want_set[MEM], seen[MEM], cur;
static uint32_t wlog[4096], nwlog, xlog[4096], nxlog;
static int bad_read;
static uint32_t mlog[256][3], nmlog;
static uint32_t rq[8192][3], nrq, rq_pos;
static char got_calls[2048][160]; static unsigned ngot;
static uint32_t rets[12]; static unsigned nrets;
static unsigned vs_n, ck_n, gi_n;
static uint32_t cur_entry;

static void rd(pe_addr_t a, unsigned n)
{
    unsigned i;
    for (i = 0; i < n; i++) {
        if (readable[(a + i) & (MEM - 1)] != cur) bad_read = 1;
        seen[(a + i) & (MEM - 1)] = cur;
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
static void mmio(pe_addr_t a, unsigned n, uint32_t v)
{
    if ((a >> 16) == 0x1F80u && nmlog < 256) { mlog[nmlog][0] = a; mlog[nmlog][1] = n; mlog[nmlog][2] = v; nmlog++; }
}
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
/* The pe_cdreg wrappers decode CD addresses into the device model; here every access is the plain value access. */
uint8_t PE_CdLoadU8(pe_addr_t a) { return PE_LoadU8(a); }
void PE_CdStoreU8(pe_addr_t a, uint8_t v) { PE_StoreU8(a, v); }
uint32_t PE_CdLoadU32(pe_addr_t a) { return PE_LoadU32(a); }
void PE_CdStoreU32(pe_addr_t a, uint32_t v) { PE_StoreU32(a, v); }
int PE_CdReg_DeviceEnabled(void) { return 0; }
int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(PEPortStopReason reason) { (void)reason; bad_read = 1; }
void pe_plat_sio_poll_tick(void) { }

static void note(const char *t)
{
    if (ngot < 2048) snprintf(got_calls[ngot], sizeof got_calls[ngot], "%s@%u", t, nwlog);
    ngot++;
}
static const uint32_t POS[10] = {0u, 1u, 2u, 0xFFFFFFFFu, 4u, 6u, 3u, 8u, 0x10000u, 0x100u};
static const uint32_t FIN[10] = {0u, 1u, 2u, 3u, 5u, 0xFFFFFFFFu, 0x10000u, 4u, 6u, 7u};
static const uint8_t BYTES[8] = {0, 0, 0, 1, 2, 0x55, 0x80, 0xFF};
static uint32_t poll_model(uint32_t code, unsigned n, uint32_t mask)
{
    uint32_t k = code & 0xFFu, pos = POS[((code >> 8) & 0xFu) % 10u], fin = FIN[((code >> 12) & 0xFu) % 10u];
    return (n <= k ? pos : fin) & mask;
}
uint32_t HostFB_VSync(int mode);
void HostFB_GetState(int *vsync, int *drawsync, int *presented, int *mask)
{
    (void)drawsync; (void)presented; (void)mask;
    *vsync = (int)HostFB_VSync(-1);        /* the checker's VSync model: the counter query is VSync(-1) */
}
uint32_t HostFB_VSync(int mode)
{
    char t[40];
    snprintf(t, sizeof t, "80073A44,%08X", (uint32_t)mode); note(t);
    vs_n++;
    return rets[cur_entry == 0x8007B290u ? 1 : 0] + vs_n * rets[cur_entry == 0x8007B290u ? 6 : 2];
}
uint16_t pe_plat_ctl_callback_state(void)
{
    note("80073DE8"); ck_n++;
    return (uint16_t)poll_model(rets[3], ck_n, 0xFFFFu);
}
int pe_plat_cd_acknowledge_host(void)
{
    uint32_t eff = rets[7];
    note("8007AAB4"); gi_n++;
    if (gi_n == 1u && (eff >> 8) < 8u) {      /* the first call writes the CD index byte through the pointer word */
        static const uint8_t IDX[8] = {0, 1, 2, 3, 0x10, 0x21, 0xFF, 0x80};
        uint32_t ptr;
        memcpy(&ptr, ram + 0x9B27Cu, 4);
        PE_StoreU8(ptr, IDX[eff >> 8]);
    }
    if ((eff & 1u) && gi_n == 1u + ((eff >> 1) & 3u) % 3u) {
        unsigned j;
        PE_StoreU8(0x8009B296u, BYTES[((eff >> 3) & 3u) | (4u * ((eff >> 3) & 1u))]);
        PE_StoreU8(0x8009B295u, BYTES[((eff >> 5) & 3u) | (4u * ((eff >> 5) & 1u))]);
        if (eff & 0x80u)
            for (j = 0; j < 8; j++) {
                PE_StoreU8(0x800A3470u + j, (uint8_t)(0x31u * j + eff));
                PE_StoreU8(0x800A3468u + j, (uint8_t)(0x17u * j + eff));
            }
    }
    return (int)poll_model(rets[4], gi_n, 0xFFFFFFFFu);
}
void pe_plat_cd_flush(void) { note("8007B9EC"); }
int32_t PE_Printf_Guest(pe_addr_t fmt, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7)
{
    char t[160];
    snprintf(t, sizeof t, "80071A74,%08X,%08X,%08X,%08X,%08X,%08X,%08X,%08X", fmt, a1, a2, a3, a4, a5, a6, a7); note(t);
    return (int32_t)rets[0];
}
int PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    char t[100];
    (void)site; (void)a2; (void)a3;
    if (fn == 0x80073C5Cu) {
        snprintf(t, sizeof t, "80073C5C,%08X", (uint32_t)a0); note(t); return 0;
    }
    if (fn == 0x80081268u) {
        static const uint32_t counter[4] = {0xFFFFFFFFu, 0u, 5u, 0x80000000u};
        note("80081268");
        if (rets[4] & 1u) PE_StoreU32(0x8009B6B4u, counter[(rets[4] >> 1) & 3u]);   /* the pump's absolute counter write */
        return (int)rets[1];
    }
    snprintf(t, sizeof t, "%08X,%08X,%08X", fn, (uint32_t)a0, (uint32_t)a1); note(t);
    if (arity != 2u) bad_read = 1;
    return (int)rets[3];
}
int PE_Cd_DispatchDataCallback(pe_addr_t target, uint32_t status, pe_addr_t response)
{
    char t[100];
    snprintf(t, sizeof t, "%08X,%08X,%08X", target, status, response); note(t);
    return 1;
}
int pe_plat_cd_ready_poll(uint32_t mode, pe_addr_t result);
int pe_plat_ds_81110_value(void);

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
/* A vector call token as the text the native's stub would have written. */
static void want_token(char *w, size_t n, const char *tk, unsigned cnt)
{
    unsigned fn = 0, x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0;
    char fields[8][12]; int nf = 0; const char *p = tk;
    while (*p && nf < 8) {
        char *e; unsigned long v = strtoul(p, &e, 16); (void)v;
        snprintf(fields[nf++], sizeof fields[0], "%.*s", (int)(e - p), p);
        p = (*e == ',') ? e + 1 : e;
    }
    sscanf(tk, "%x,%x,%x,%x,%x,%x", &fn, &x0, &x1, &x2, &x3, &x4);
    if (fn == 0x80071A74u) {            /* printf: the recorded fields, completed with zero arguments */
        int k, o;
        o = snprintf(w, n, "80071A74");
        for (k = 1; k < 9; k++)
            o += snprintf(w + o, n - (size_t)o, ",%s", k < nf ? fields[k] : "00000000");
        snprintf(w + o, n - (size_t)o, "@%u", cnt);
    } else if (fn == 0x80073A44u || fn == 0x80073C5Cu) snprintf(w, n, "%08X,%08X@%u", fn, x0, cnt);
    else if ((fn & 0x1FFFFFFFu) == 0x00130B40u || (fn & 0x1FFFFFFFu) == 0x00130C40u || (fn & 0x1FFFFFFFu) == 0x00130C80u)
        snprintf(w, n, "%08X,%08X,%08X@%u", fn, x0, x1, cnt);
    else snprintf(w, n, "%08X@%u", fn, cnt);
}
int main(int c, char **v)
{
    FILE *f; static char line[262144]; unsigned n = 0, bad = 0;
    if (c != 2 || !(f = fopen(v[1], "r"))) return 2;
    while (fgets(line, sizeof line, f)) {
        static char *tok[16384]; static char copy[262144];
        int nt = 0, i, ri, li, si, ci, na, ok = 1; uint32_t entry, a[8] = {0}, wret, got = 0; unsigned k;
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        while (nt < 16384 && (tok[nt] = strtok(nt ? NULL : copy, " \n"))) nt++;
        if (nt < 8 || strcmp(tok[3], "A")) continue;
        ri = find(tok, nt, 4, "RET"); li = ri + 2; si = find(tok, nt, li, "S");
        if (si >= nt || li >= nt || strcmp(tok[li], "L")) continue;
        entry = (uint32_t)strtoul(tok[0], 0, 16);
        na = ri - 4;
        for (i = 0; i < na && i < 8; i++) a[i] = (uint32_t)strtoul(tok[4 + i], 0, 16);
        wret = (uint32_t)strtoul(tok[ri + 1], 0, 16);
        cur = ++n; nwlog = nxlog = 0; bad_read = 0; nmlog = 0; nrq = rq_pos = 0; ngot = 0;
        vs_n = ck_n = gi_n = 0;
        for (k = (unsigned)li + 1; k < (unsigned)si; k++) {
            unsigned addr, size, val;
            if (!strcmp(tok[k], "-")) continue;
            if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) == 3 && (addr >> 16) == 0x1F80u) {
                if (nrq < 8192) { rq[nrq][0] = addr; rq[nrq][1] = size; rq[nrq][2] = val; nrq++; }
                continue;
            }
            apply(tok[k], ram, readable, 1);
        }
        ci = si + 1;
        while (ci < nt && strcmp(tok[ci], "CALL")) ci++;
        nrets = 0;
        for (k = (unsigned)si + 1; k + 1 < (unsigned)ci; k++)
            if (!strcmp(tok[k], "RETS")) {
                char *e = tok[k + 1];
                while (nrets < 12) { rets[nrets++] = (uint32_t)strtoul(e, &e, 16); if (*e != ',') break; e++; }
            }
        for (k = (unsigned)si + 1; k < (unsigned)ci; k++) {
            if (!strcmp(tok[k], "RETS")) { k++; continue; }
            if (strcmp(tok[k], "-")) apply(tok[k], want, want_set, 0);
        }
        cur_entry = entry;
        if (entry == 0x8007B290u) got = (uint32_t)pe_plat_cd_ready_poll(a[0], a[1]);
        else if (entry == 0x80081110u) got = (uint32_t)pe_plat_ds_81110_value();
        else { fprintf(stderr, "unknown entry %08X\n", entry); return 2; }
        if (got != wret) ok = 0;
        if (bad_read) ok = 0;
        if (nwlog != nxlog) ok = 0;
        if (rq_pos != nrq) ok = 0;
        for (k = (unsigned)li + 1; k < (unsigned)si; k++) {    /* load coverage */
            unsigned addr, size, val, b;
            if (sscanf(tok[k], "%x:%u=%x", &addr, &size, &val) != 3 || (addr >> 16) == 0x1F80u) continue;
            for (b = 0; b < size; b++)
                if (seen[(addr + b) & (MEM - 1)] != cur) { ok = 0; if (bad < 3) fprintf(stderr, "  load %08X not made by the native\n", addr + b); }
        }
        {
            unsigned want_c = 0;
            for (k = (unsigned)ci + 1; k < (unsigned)nt; k++) {
                char w[200], tk[160], *at; unsigned cnt = 0;
                if (!strcmp(tok[k], "-")) continue;
                strncpy(tk, tok[k], sizeof tk - 1); tk[sizeof tk - 1] = 0;
                if ((at = strchr(tk, '@')) != NULL) {
                    unsigned nst = (unsigned)strtoul(at + 1, 0, 10), q, b, seen = 0;
                    static uint32_t mark[MEM]; static uint32_t gen;
                    gen++;
                    for (q = (unsigned)si + 1; q < (unsigned)ci && nst; q++) {
                        unsigned ad, sz, vv;
                        if (!strcmp(tok[q], "RETS")) break;
                        if (sscanf(tok[q], "%x:%u=%x", &ad, &sz, &vv) != 3) continue;
                        for (b = 0; b < sz; b++) {
                            uint32_t m = (ad + b) & (MEM - 1);
                            if (mark[m] != gen) { mark[m] = gen; seen++; }
                        }
                        nst--;
                    }
                    cnt = seen;
                    *at = 0;
                }
                want_token(w, sizeof w, tk, cnt);
                if (want_c >= ngot || want_c >= 2048 || strcmp(got_calls[want_c], w)) ok = 0;
                want_c++;
            }
            if (want_c != ngot) ok = 0;
        }
        {
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
        if (!ok && bad < 3) {
            unsigned q;
            fprintf(stderr, "  got ret=%08X want=%08X reads_ok=%d regs %u/%u writes %u/%u\n", got, wret, !bad_read, rq_pos, nrq, nwlog, nxlog);
            for (q = 0; q < ngot && q < 12; q++) fprintf(stderr, "  got %s\n", got_calls[q]);
        }
        if (!ok && bad++ < 12)
            fprintf(stderr, "MISMATCH line %u: %.300s\n", n, line);
    }
    printf("checked %u cases, %u mismatches\n", n, bad);
    return bad != 0;
}
