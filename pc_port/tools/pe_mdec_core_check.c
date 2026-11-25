/* Vector check (batch cdgen40) for pe_plat_mdec_core / pe_plat_mdec_upload_start (guest 0x8010C0FC /
 * 0x8010C1EC) against the output of pc_port/tools/pe_mdec_core_vectors.py.  Each line is an ordered list of the
 * original's guest loads, stores and printf calls; the native's accesses are replayed against it event by
 * event: a load returns the recorded value (and must be at the recorded address), a store must match address
 * and value, the printf call must match its format and first argument, and every event must be consumed.  The
 * host services the native uses are stubbed: the MDEC status word is replayed as the load of 0x1F801824 it
 * stands for, the status-poll host contracts never fire (PE_Port_ShouldStop returns 0 and a RequestStop call is a
 * mismatch).  The original's v0 is not compared (void ABI in the port).
 *
 * Build and run from the repo root:
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_mdec_core_check.c pc_port/platform/pe_plat_mdec_core.c -o check
 *   ./check vectors.txt
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_mdec.h"
#include "pe_bios_string.h"
#include "pe_plat/mdec_callback.h"

typedef struct { char kind; uint32_t a, v, w; } Ev;   /* L/S: address, value; C: target, a1, a0 */
static Ev ev[256];
static int nev, pos, bad_flag;

/* next recorded event must be of this kind at this address (and, for S, carry this value; for C, this a1/a0) */
static uint32_t expect(char kind, uint32_t a, uint32_t v, uint32_t w)
{
    uint32_t r = 0;
    if (pos >= nev || ev[pos].kind != kind || ev[pos].a != a) bad_flag = 1;
    else {
        if (kind == 'S' && ev[pos].v != v) bad_flag = 1;
        if (kind == 'C' && (ev[pos].v != v || ev[pos].w != w)) bad_flag = 1;
        r = ev[pos].v;
    }
    pos++;
    return r;
}

uint32_t PE_LoadU32(pe_addr_t a) { return expect('L', a, 0, 0); }
void PE_StoreU32(pe_addr_t a, uint32_t v) { (void)expect('S', a, v, 0); }
uint32_t PE_MDEC_StatusWord(void) { return expect('L', 0x1F801824u, 0, 0) & 0x20000000u; }
int PE_MDEC_Service(void) { return 0; }
int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(PEPortStopReason r) { (void)r; bad_flag = 1; }
int32_t PE_Printf_Guest(pe_addr_t fmt, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7)
{
    (void)a2; (void)a3; (void)a4; (void)a5; (void)a6; (void)a7;
    (void)expect('C', 0x80071A74u, a1, fmt);
    return 0;
}

int main(int argc, char **argv)
{
    static char line[16384];
    unsigned long total = 0, bad = 0;
    FILE *f;
    if (argc < 2 || !(f = fopen(argv[1], "r"))) { fprintf(stderr, "usage: check vectors.txt\n"); return 2; }
    while (fgets(line, sizeof line, f)) {
        char ent[8]; unsigned seed, x, y = 0; int used = 0;
        char *p, *t;
        if (sscanf(line, "%7s %u A %x%n", ent, &seed, &x, &used) != 3) { fprintf(stderr, "bad line\n"); return 2; }
        p = line + used;
        if (!strcmp(ent, "C1EC")) { if (sscanf(p, " %x%n", &y, &used) != 1) return 2; p += used; }
        p = strstr(p, "EV");
        if (!p) return 2;
        nev = 0;
        for (t = strtok(p + 2, " \n"); t; t = strtok(NULL, " \n")) {
            unsigned a, b, c;
            if (t[0] == 'C') { if (sscanf(t, "C:%x:%x:%x", &a, &b, &c) != 3) return 2; ev[nev].kind = 'C'; ev[nev].a = a; ev[nev].w = b; ev[nev].v = c; }
            else { unsigned sz; if (sscanf(t + 2, "%x:%u:%x", &a, &sz, &b) != 3) return 2; ev[nev].kind = t[0]; ev[nev].a = a; ev[nev].v = b; ev[nev].w = 0; }
            if (ev[nev].kind == 'L' && ev[nev].a >= 0x8010DB00u && ev[nev].a < 0x8010DB60u) continue;   /* overlay pointer-word load: constant, not replayed */
            nev++;
        }
        pos = 0; bad_flag = 0;
        if (!strcmp(ent, "C0FC")) pe_plat_mdec_core((int)x);
        else pe_plat_mdec_upload_start(x, y);
        total++;
        if (bad_flag || pos != nev) { if (bad < 5) fprintf(stderr, "mismatch %s seed %u (consumed %d of %d)\n", ent, seed, pos, nev); bad++; }
    }
    fclose(f);
    printf("checked %lu cases, %lu mismatches\n", total, bad);
    return bad != 0;
}
