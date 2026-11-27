/* gte60a vector check: TransMatrix binding (pe_plat_gte.c) and the host
 * matrix / vector loaders and setters of pe_gte.c.
 *   cc -std=gnu99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/game -Ipc_port/tools pc_port/tools/pe_gte60_a_check.c pc_port/platform/pe_plat_gte.c pc_port/platform/pe_gte.c pc_port/platform/pe_gte_commands.c -o check
 *   ./check vectors.txt        (vectors: pe_gte60_vectors.py OUT a)
 * Control builds: add -DGTE60_MUTANT=1..4 (see pe_gte60_common.h; 4 = LoadRT33
 * calls LoadRT instead), or replace the two pe_gte*.c files by
 * pc_port/tools/pe_gte60_noop_stub.c (no-op provider).
 * Functions: pe_plat_gte_translate_matrix (original 0x80078C94, RTL oracle),
 * PE_GTE_LoadRT / LoadRT33 / LoadLCM (original SetRotMatrix(+SetTransMatrix) /
 * SetColorMatrix under the interpreter), LoadLLM_halfs (SetLightMatrix over a
 * 9-halfword host array, guard page behind it), SetV0/1/2, SetIR, SetRGBC,
 * SetBK (hardware register model; checked against the 1100+50 hardware cases).
 * Per case, besides the register compare: guest reads only inside L; no store
 * unless S; the accessors leave g_pe_gte.projection_flags (FLAG's backing word)
 * untouched and every g_pe_gte field equal to the expected register value. */
#include "pe_plat/gte.h"
#include "pe_gte60_common.h"

static int call(const Case *c, uint32_t *ret, int *has_ret)
{
    const uint32_t *a = c->a;
    *has_ret = 0;
#if GTE60_MUTANT == 3
    (void)a; return 1;
#endif
    if (!strcmp(c->name, "pe_plat_gte_translate_matrix")) { *ret = pe_plat_gte_translate_matrix(a[0], a[1]); *has_ret = 1; }
    else if (!strcmp(c->name, "PE_GTE_LoadRT")) PE_GTE_LoadRT(a[0]);
    else if (!strcmp(c->name, "PE_GTE_LoadRT33")) {
#if GTE60_MUTANT == 4
        PE_GTE_LoadRT(a[0]);
#else
        PE_GTE_LoadRT33(a[0]);
#endif
    }
    else if (!strcmp(c->name, "PE_GTE_LoadLCM")) PE_GTE_LoadLCM(a[0]);
    else if (!strcmp(c->name, "PE_GTE_LoadLLM_halfs")) {
        int16_t *h = guard_buf(9 * sizeof(int16_t));
        int i;
        for (i = 0; i < 9; i++) h[i] = (int16_t)(uint16_t)a[i];
        PE_GTE_LoadLLM_halfs(h);
    }
    else if (!strcmp(c->name, "PE_GTE_SetV0")) PE_GTE_SetV0((int16_t)a[0], (int16_t)a[1], (int16_t)a[2]);
    else if (!strcmp(c->name, "PE_GTE_SetV1")) PE_GTE_SetV1((int16_t)a[0], (int16_t)a[1], (int16_t)a[2]);
    else if (!strcmp(c->name, "PE_GTE_SetV2")) PE_GTE_SetV2((int16_t)a[0], (int16_t)a[1], (int16_t)a[2]);
    else if (!strcmp(c->name, "PE_GTE_SetIR")) PE_GTE_SetIR((int16_t)a[0], (int16_t)a[1], (int16_t)a[2]);
    else if (!strcmp(c->name, "PE_GTE_SetRGBC")) PE_GTE_SetRGBC(a[0]);
    else if (!strcmp(c->name, "PE_GTE_SetBK")) PE_GTE_SetBK((int32_t)a[0], (int32_t)a[1], (int32_t)a[2]);
    else return -1;
    return 0;
}

int main(int argc, char **argv)
{
    static char line[65536], copy[65536];
    static Case c;
    FILE *f;
    if (argc != 2 || !(f = fopen(argv[1], "r"))) { fprintf(stderr, "usage: %s vectors.txt\n", argv[0]); return 2; }
    while (fgets(line, sizeof line, f)) {
        uint32_t snap[64], ret = 0, pf0;
        int ok = 1, has, r;
        char why[160] = "";
        strncpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        if (!parse_case(copy, &c)) continue;
        cur++;
        load_in(c.in);
        pf0 = g_pe_gte.projection_flags;   /* FLAG's backing word (ReadCtrl(31) reads it) */
        mem_begin(&c);
        r = call(&c, &ret, &has);
        if (r < 0) { fprintf(stderr, "unknown function %s\n", c.name); return 2; }
        snapshot(snap);
        if (!compare_regs(&c, snap, why, sizeof why)) ok = 0;
        if (has && c.nret && ret != c.ret[0]) { ok = 0; snprintf(why, sizeof why, "return %X want %X", ret, c.ret[0]); }
        if (!mem_ok()) { ok = 0; snprintf(why, sizeof why, "guest memory: %s", bad_read ? "read outside the original loads" : "stores differ"); }
        if (g_pe_gte.projection_flags != pf0) { ok = 0; snprintf(why, sizeof why, "projection_flags changed by a non-command accessor"); }
        if (!fields_match(c.out, &g_pe_gte, why, sizeof why)) ok = 0;   /* whole state object untouched elsewhere */
        if (!ok) report_fail(&c, why);
        count_case(c.name, ok);
    }
    return finish("gte60a");
}
