/* gte60b vector check: the host GTE command wrappers of pe_gte.c (Command,
 * MVMVA, GPF, GPL, OP, NCCT, RTPS_coordinates, RTPT_coordinates, AVSZ3,
 * AVSZ4, NCLIP).
 *   cc -std=gnu99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/game -Ipc_port/tools pc_port/tools/pe_gte60_b_check.c pc_port/platform/pe_gte.c pc_port/platform/pe_gte_commands.c -o check
 *   ./check vectors.txt        (vectors: pe_gte60_vectors.py OUT b)
 * Control builds: -DGTE60_MUTANT=1..4 (4 = GPF/GPL/OP/MVMVA ignore lm), or the
 * no-op provider pc_port/tools/pe_gte60_noop_stub.c instead of the two sources.
 * Per case, twice: (1) the 64 IN values are written through PE_GTE_WriteData /
 * WriteCtrl; (2) the same pre-state is set directly in g_pe_gte fields
 * (layout of pe_sdk.h), then the call runs.  Both must read back OUT (FLAG only
 * when F=1; the direct run skips FLAG: its backing word is the provider's).
 * The direct run is judged only when the pre-state H is below 0x8000: H is an
 * unsigned 16-bit register that merely reads back sign-extended, so how a
 * provider's field holds H >= 0x8000 (signed or zero-extended) is not fixed by
 * the contract; those cases are counted as "direct not judged".
 * Guest RAM must not be read or written.  Return values / out-pointers are
 * compared with RET.  Undocumented opcodes are never fed to Command (the
 * provider aborts on them by contract). */
#include "pe_gte60_common.h"

static int call(const Case *c, uint32_t *ret, int *nret)
{
    const uint32_t *a = c->a;
    *nret = 0;
#if GTE60_MUTANT == 3
    (void)a; return 0;
#endif
    if (!strcmp(c->name, "PE_GTE_Command")) PE_GTE_Command(a[0]);
    else if (!strcmp(c->name, "PE_GTE_MVMVA")) {
#if GTE60_MUTANT == 4
        PE_GTE_MVMVA(a[0] & ~0x400u);
#else
        PE_GTE_MVMVA(a[0]);
#endif
    }
    else if (!strcmp(c->name, "PE_GTE_GPF")) {
#if GTE60_MUTANT == 4
        PE_GTE_GPF((int)a[0], 0);
#else
        PE_GTE_GPF((int)a[0], (int)a[1]);
#endif
    }
    else if (!strcmp(c->name, "PE_GTE_GPL")) {
#if GTE60_MUTANT == 4
        PE_GTE_GPL((int)a[0], 0);
#else
        PE_GTE_GPL((int)a[0], (int)a[1]);
#endif
    }
    else if (!strcmp(c->name, "PE_GTE_OP")) {
#if GTE60_MUTANT == 4
        PE_GTE_OP((int)a[0], 0);
#else
        PE_GTE_OP((int)a[0], (int)a[1]);
#endif
    }
    else if (!strcmp(c->name, "PE_GTE_NCCT")) PE_GTE_NCCT();
    else if (!strcmp(c->name, "PE_GTE_RTPS_coordinates")) { PE_GTE_RTPS_coordinates(&ret[0], &ret[1]); *nret = 2; }
    else if (!strcmp(c->name, "PE_GTE_RTPT_coordinates")) {
        uint32_t xy[3], z[3];
        PE_GTE_RTPT_coordinates(xy, z);
        memcpy(ret, xy, sizeof xy); memcpy(ret + 3, z, sizeof z); *nret = 6;
    }
    else if (!strcmp(c->name, "PE_GTE_AVSZ3")) {
        uint32_t *z = guard_buf(3 * sizeof(uint32_t));
        z[0] = a[0]; z[1] = a[1]; z[2] = a[2];
        PE_GTE_AVSZ3(z);
    }
    else if (!strcmp(c->name, "PE_GTE_AVSZ4")) {
        uint32_t *z = guard_buf(4 * sizeof(uint32_t));
        z[0] = a[0]; z[1] = a[1]; z[2] = a[2]; z[3] = a[3];
        PE_GTE_AVSZ4(z);
    }
    else if (!strcmp(c->name, "PE_GTE_NCLIP")) { ret[0] = (uint32_t)PE_GTE_NCLIP(); *nret = 1; }
    else return -1;
    return 1;
}

static int judge(const Case *c, const uint32_t *ret, int nret, int direct, char *why, size_t wn)
{
    uint32_t snap[64];
    int ok = 1, i;
    Case t = *c;
    snapshot(snap);
    if (direct) t.fv = 0;
    if (!compare_regs(&t, snap, why, wn)) ok = 0;
    if (c->nret && nret) {
        if (c->nret != nret) ok = 0;
        for (i = 0; i < nret && i < c->nret; i++)
            if (ret[i] != c->ret[i]) { ok = 0; snprintf(why, wn, "return[%d] %X want %X", i, ret[i], c->ret[i]); }
    }
    if (!mem_ok() || nwlog) { ok = 0; snprintf(why, wn, "guest memory touched"); }
    return ok;
}

int main(int argc, char **argv)
{
    static char line[65536], copy[65536], dname[96];
    unsigned direct_skipped = 0;
    static Case c;
    FILE *f;
    if (argc != 2 || !(f = fopen(argv[1], "r"))) { fprintf(stderr, "usage: %s vectors.txt\n", argv[0]); return 2; }
    while (fgets(line, sizeof line, f)) {
        uint32_t ret[8] = {0}, pre[64];
        int nret, ok, r;
        char why[160] = "";
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        if (!parse_case(copy, &c)) continue;
        cur++;
        load_in(c.in);
        mem_begin(&c);
        snapshot(pre);
        r = call(&c, ret, &nret);
        if (r < 0) { fprintf(stderr, "unknown function %s\n", c.name); return 2; }
        ok = judge(&c, ret, nret, 0, why, sizeof why);
        if (!ok) report_fail(&c, why);
        count_case(c.name, ok);
        /* direct-field protocol (only where H < 0x8000, see the header) */
        if ((int16_t)pre[32 + 26] < 0) { direct_skipped++; continue; }
        cur++;
        load_in(c.in);
        snapshot(pre);
        fields_from_reads(pre, &g_pe_gte);
        mem_begin(&c);
        memset(ret, 0, sizeof ret);
        call(&c, ret, &nret);
        why[0] = 0;
        ok = judge(&c, ret, nret, 1, why, sizeof why);
        snprintf(dname, sizeof dname, "%s +direct-fields", c.name);
        if (!ok) { strncat(why, " (direct)", sizeof why - strlen(why) - 1); report_fail(&c, why); }
        count_case(dname, ok);
    }
    printf("direct-field protocol: %u cases not judged (pre-state H >= 0x8000)\n", direct_skipped);
    return finish("gte60b");
}
