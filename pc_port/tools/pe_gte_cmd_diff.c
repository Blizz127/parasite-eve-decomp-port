/* Host GTE model vs the test-only interpreter, one cop2 command at a time:
 *   cc -std=gnu99 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/game \
 *      pc_port/tools/pe_gte_cmd_diff.c pc_port/platform/pe_gte.c pc_port/platform/pe_gte_commands.c -o diff
 *   ./diff cases.txt          (cases from pc_port/tools/pe_gte_cmd_diff.py)
 * Per case: write C0/D0 into the host model (PE_GTE_WriteCtrl / WriteData;
 * data 15, 28, 29 and 31 are derived registers and are not written), read
 * everything back and compare with C0/D0 (INIT mismatch = the canonical
 * state did not survive the round trip; the case is not judged).  Then run
 * PE_GTE_Command and compare every register, as read by PE_GTE_ReadCtrl /
 * ReadData, with the interpreter's final registers under the same psx-spx
 * read widths as the generator (canon_data / canon_ctrl).  FLAG is compared
 * only when the interpreter marked its flags valid. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

uint32_t PE_GTE_ReadData(uint32_t reg);
void PE_GTE_WriteData(uint32_t reg, uint32_t value);
uint32_t PE_GTE_ReadCtrl(uint32_t reg);
void PE_GTE_WriteCtrl(uint32_t reg, uint32_t value);
void PE_GTE_Command(uint32_t cmd);

static int stopped;
uint16_t PE_LoadU16(uint32_t a) { (void)a; stopped = 1; return 0; }
uint32_t PE_LoadU32(uint32_t a) { (void)a; stopped = 1; return 0; }
void PE_Port_RequestStop(const char *why) { (void)why; stopped = 1; }

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
static int field(char **tok, int nt, int at, const char *tag, uint32_t *v)
{
    int i;
    if (at + 32 >= nt || strcmp(tok[at], tag)) return 0;
    for (i = 0; i < 32; i++) v[i] = (uint32_t)strtoul(tok[at + 1 + i], 0, 16);
    return 1;
}
int main(int c, char **v)
{
    static char line[16384], *tok[256];
    unsigned n = 0, init_bad = 0, bad = 0, bad_reg[2][32] = {{0}}, r;
    FILE *f;
    if (c != 2 || !(f = fopen(v[1], "r"))) return 2;
    while (fgets(line, sizeof line, f)) {
        uint32_t cmd, c0[32], d0[32], c1[32], d1[32], wc[32], wd[32];
        int nt = 0, fv, ok = 1, i;
        while (nt < 256 && (tok[nt] = strtok(nt ? NULL : line, " \n"))) nt++;
        if (nt < 135 || !field(tok, nt, 1, "C0", c0) || !field(tok, nt, 34, "D0", d0) ||
            !field(tok, nt, 67, "C1", c1) || !field(tok, nt, 100, "D1", d1) || strcmp(tok[133], "F")) continue;
        cmd = (uint32_t)strtoul(tok[0], 0, 16);
        fv = atoi(tok[134]);
        n++;
        stopped = 0;
        for (i = 0; i < 32; i++) PE_GTE_WriteCtrl((uint32_t)i, c0[i]);
        for (i = 0; i < 31; i++)
            if (i != 15 && i != 28 && i != 29) PE_GTE_WriteData((uint32_t)i, d0[i]);
        for (i = 0; i < 32; i++)
            if (PE_GTE_ReadCtrl((uint32_t)i) != c0[i] || PE_GTE_ReadData((uint32_t)i) != d0[i]) ok = 0;
        if (!ok) {
            if (init_bad++ < 5) {
                fprintf(stderr, "INIT case %u:", n);
                for (i = 0; i < 32; i++) {
                    if ((r = PE_GTE_ReadCtrl((uint32_t)i)) != c0[i]) fprintf(stderr, " c%d %X!=%X", i, r, c0[i]);
                    if ((r = PE_GTE_ReadData((uint32_t)i)) != d0[i]) fprintf(stderr, " d%d %X!=%X", i, r, d0[i]);
                }
                fprintf(stderr, "\n");
            }
            continue;
        }
        PE_GTE_Command(cmd);
        canon_ctrl(c1, wc);
        canon_data(d1, wd);
        for (i = 0; i < 32; i++) {
            if ((i != 31 || fv) && PE_GTE_ReadCtrl((uint32_t)i) != wc[i]) { ok = 0; bad_reg[0][i]++; }
            if (PE_GTE_ReadData((uint32_t)i) != wd[i]) { ok = 0; bad_reg[1][i]++; }
        }
        if (stopped) ok = 0;
        if (!ok && bad++ < 8) {
            fprintf(stderr, "MISMATCH case %u cmd %06X:", n, cmd);
            for (i = 0; i < 32; i++) {
                if ((i != 31 || fv) && (r = PE_GTE_ReadCtrl((uint32_t)i)) != wc[i]) fprintf(stderr, " c%d host %X interp %X", i, r, wc[i]);
                if ((r = PE_GTE_ReadData((uint32_t)i)) != wd[i]) fprintf(stderr, " d%d host %X interp %X", i, r, wd[i]);
            }
            fprintf(stderr, "%s\n", stopped ? " (stop requested)" : "");
        }
    }
    printf("cases %u, init mismatches %u, command mismatches %u\n", n, init_bad, bad);
    for (r = 0; r < 32; r++) {
        if (bad_reg[0][r]) printf("  ctrl %u: %u\n", r, bad_reg[0][r]);
        if (bad_reg[1][r]) printf("  data %u: %u\n", r, bad_reg[1][r]);
    }
    return (init_bad || bad) ? 1 : 0;
}
