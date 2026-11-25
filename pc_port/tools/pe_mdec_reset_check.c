/* Vector check (batch cdgen37) for pe_plat_mdec_reset (guest 0x8010BE3C) against the output of
 * pc_port/tools/pe_mdec_reset_vectors.py.  The two exits are stubbed and
 * record (target, a0); the native is compared with the original's call list
 * (count, targets in order, and a0 of the reset core 0x8010C0FC; the argument
 * of 0x80073C94 is not compared: it is 0 whenever it is called).  The
 * original's v0 (the core's result passed through) is not compared: the port's
 * ABI for the row is void.  The native must not touch guest memory: the build
 * links no PE_* symbol, so a guest access fails to link.
 *
 * Build and run from the repo root (NATIVE defaults to the clean-room file;
 * the control mutants in docs/evidence/native-be3c-20261125/controls/ replace it):
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform pc_port/tools/pe_mdec_reset_check.c pc_port/platform/pe_plat_mdec_reset.c -o check
 *   ./check vectors.txt
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pe_plat/mdec_callback.h"

static int ncalls;
static uint32_t ctarget[8], ca0[8];

/* stubs: the reset core is renamed by on the build line;
 * the reset callback is stubbed under its pe_plat binding name */
uint32_t pe_plat_reset_callback(void) { if (ncalls < 8) { ctarget[ncalls] = 0x80073C94u; ca0[ncalls] = 0; } ncalls++; return 0; }
void pe_plat_mdec_core(int mode) { if (ncalls < 8) { ctarget[ncalls] = 0x8010C0FCu; ca0[ncalls] = (uint32_t)mode; } ncalls++; }

int main(int argc, char **argv)
{
    char line[2048];
    unsigned long total = 0, bad = 0;
    FILE *f;
    if (argc < 2 || !(f = fopen(argv[1], "r"))) { fprintf(stderr, "usage: check vectors.txt\n"); return 2; }
    while (fgets(line, sizeof line, f)) {
        unsigned seed, a0, a1, a2, a3, r1, r2, out, n, k;
        int pos = 0, used;
        uint32_t wt[8], wa[8];
        char *p;
        if (sscanf(line, "BE3C %u A %x %x %x %x R %x %x OUT %x N %u%n", &seed, &a0, &a1, &a2, &a3, &r1, &r2, &out, &n, &used) != 9 || n > 8) {
            fprintf(stderr, "bad line: %s", line); return 2;
        }
        p = line + used;
        for (k = 0; k < n; k++) {
            unsigned t, x, y, z, w; int u;
            if (sscanf(p, " C %x:%x:%x:%x:%x%n", &t, &x, &y, &z, &w, &u) != 5) { fprintf(stderr, "bad call: %s", line); return 2; }
            wt[k] = t; wa[k] = x; p += u;
        }
        (void)pos;
        ncalls = 0;
        pe_plat_mdec_reset((int)a0);
        total++;
        {
            int ok = ncalls == (int)n;
            for (k = 0; ok && k < n; k++)
                if (ctarget[k] != wt[k] || (wt[k] == 0x8010C0FCu && ca0[k] != wa[k])) ok = 0;
            if (!ok) { if (bad < 5) fprintf(stderr, "mismatch seed %u mode %08X: got %d calls\n", seed, a0, ncalls); bad++; }
        }
    }
    fclose(f);
    printf("checked %lu cases, %lu mismatches\n", total, bad);
    return bad != 0;
}
