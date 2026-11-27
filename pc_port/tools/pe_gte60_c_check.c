/* gte60c vector check: the raw register accessors and the state object of
 * pe_gte.c (PE_GTE_WriteData / ReadData / WriteCtrl / ReadCtrl, PE_GTE_LZCR,
 * PE_GTE_SetLZCS, g_pe_gte).
 *   cc -std=gnu99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/game -Ipc_port/tools pc_port/tools/pe_gte60_c_check.c pc_port/platform/pe_gte.c pc_port/platform/pe_gte_commands.c -o check
 *   ./check vectors.txt        (vectors: pe_gte60_vectors.py OUT c)
 * Control builds: -DGTE60_MUTANT=1..4 (4 = PE_GTE_LZCR(0) returns 31), or the
 * no-op provider pc_port/tools/pe_gte60_noop_stub.c instead of the two sources.
 * Case kinds (NAME): RW (IN written, all 64 read back = OUT; hardware dumps
 * and model sequences; reads twice, the second pass must be identical and
 * must not change g_pe_gte), RWONE (one extra write of A[1] to register A[0],
 * 0-31 data, 32-63 control), ALIAS (contract observed in the public tests, not a
 * hardware fact: indices >= 32 act on index & 31, for writes and reads),
 * LZ (PE_GTE_LZCR(A[0]) = RET and does not change state; PE_GTE_SetLZCS(A[0])
 * then reads = OUT), STRUCT (after the writes the g_pe_gte fields hold the
 * register values per pe_sdk.h; and the same values set directly in the
 * fields read back through ReadData/ReadCtrl). */
#include "pe_gte60_common.h"

int main(int argc, char **argv)
{
    static char line[65536], copy[65536];
    static Case c;
    FILE *f;
    if (argc != 2 || !(f = fopen(argv[1], "r"))) { fprintf(stderr, "usage: %s vectors.txt\n", argv[0]); return 2; }
    while (fgets(line, sizeof line, f)) {
        uint32_t s1[64], s2[64], s0[64];
        PeGteState before;
        int ok = 1, i;
        char why[160] = "";
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        if (!parse_case(copy, &c)) continue;
        cur++;
        mem_begin(&c);
        load_in(c.in);
        if (!strcmp(c.name, "RW")) {
            snapshot(s1);
            before = g_pe_gte;
            snapshot(s2);
            if (!compare_regs(&c, s1, why, sizeof why)) ok = 0;
            if (memcmp(s1, s2, sizeof s1)) { ok = 0; snprintf(why, sizeof why, "second read pass differs"); }
            if (memcmp(&before, &g_pe_gte, sizeof before)) { ok = 0; snprintf(why, sizeof why, "reads changed g_pe_gte"); }
        } else if (!strcmp(c.name, "RWONE")) {
#if GTE60_MUTANT != 3
            if (c.a[0] < 32) PE_GTE_WriteData(c.a[0], c.a[1]); else PE_GTE_WriteCtrl(c.a[0] - 32, c.a[1]);
#endif
            snapshot(s1);
            if (!compare_regs(&c, s1, why, sizeof why)) ok = 0;
        } else if (!strcmp(c.name, "ALIAS")) {
            unsigned m = c.a[1] >> 5;
            if (c.a[0] == 0) PE_GTE_WriteData(c.a[1], c.a[2]); else PE_GTE_WriteCtrl(c.a[1], c.a[2]);
            snapshot(s1);
            if (!compare_regs(&c, s1, why, sizeof why)) ok = 0;
            for (i = 0; i < 32; i++) {
                if (PE_GTE_ReadData((uint32_t)i + 32u * m) != s1[i]) { ok = 0; snprintf(why, sizeof why, "ReadData(%d) alias", i + 32 * (int)m); }
                if (i != 31 && PE_GTE_ReadCtrl((uint32_t)i + 32u * m) != s1[32 + i]) { ok = 0; snprintf(why, sizeof why, "ReadCtrl(%d) alias", i + 32 * (int)m); }
            }
        } else if (!strcmp(c.name, "LZ")) {
            uint32_t r;
            snapshot(s0);
            r = PE_GTE_LZCR(c.a[0]);
#if GTE60_MUTANT == 4
            if (c.a[0] == 0) r = 31;
#endif
            snapshot(s1);
            if (r != c.ret[0]) { ok = 0; snprintf(why, sizeof why, "LZCR(%X) = %u want %u", c.a[0], r, c.ret[0]); }
            if (memcmp(s0, s1, sizeof s0)) { ok = 0; snprintf(why, sizeof why, "LZCR changed GTE state"); }
#if GTE60_MUTANT != 3
            PE_GTE_SetLZCS(c.a[0]);
#endif
            snapshot(s2);
            if (!compare_regs(&c, s2, why, sizeof why)) ok = 0;
        } else if (!strcmp(c.name, "STRUCT")) {
            if (!fields_match(c.out, &g_pe_gte, why, sizeof why)) ok = 0;
            fields_from_reads(c.out, &g_pe_gte);
            snapshot(s1);
            c.fv = 0;
            if (!compare_regs(&c, s1, why, sizeof why)) { ok = 0; strncat(why, " (direct fields)", sizeof why - strlen(why) - 1); }
        } else { fprintf(stderr, "unknown case kind %s\n", c.name); return 2; }
        if (!mem_ok() || nwlog) { ok = 0; snprintf(why, sizeof why, "guest memory touched"); }
        if (!ok) report_fail(&c, why);
        count_case(c.name, ok);
    }
    return finish("gte60c");
}
