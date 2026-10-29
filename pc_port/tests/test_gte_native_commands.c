/* Standalone native GTE verification. No game, disc, CPU interpreter or device.
 * Fixed cases cover formerly skipped selectors and observable register effects.
 * --hardware-log replays the existing MIT ps1-tests real-console expectations;
 * a missing, truncated or malformed supplied log is a failure, never a skip.
 */
#include "pe_sdk.h"
#include "pe_gte_commands.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* These SDK entry points are linked with the GTE TU but none of these tests
 * may read guest memory or request a game stop. Abort if that contract breaks. */
uint32_t PE_LoadU32(pe_addr_t address) { (void)address; abort(); }
uint16_t PE_LoadU16(pe_addr_t address) { (void)address; abort(); }
void PE_Port_RequestStop(PEPortStopReason reason) { (void)reason; abort(); }

static unsigned checks, failures;
#define CHECK(ok, ...) do { ++checks; if (!(ok)) { ++failures; \
    fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)

static void reset(void)
{
    memset(&g_pe_gte, 0, sizeof g_pe_gte);
    for (unsigned i = 0; i < 32u; ++i) PE_GTE_WriteData(i, 0);
    PE_GTE_WriteCtrl(31u, 0xFFFFFFFFu);
}

static void identity(int16_t matrix[3][3])
{
    for (unsigned i = 0; i < 3u; ++i) matrix[i][i] = 4096;
}

static uint32_t mvmva_word(unsigned sf, unsigned mx, unsigned vector,
                          unsigned translation, unsigned lm)
{
    return 0x12u | (sf << 19) | (mx << 17) | (vector << 15) |
           (translation << 13) | (lm << 10);
}

static void vector_result(const char *name, int32_t a, int32_t b, int32_t c,
                          int32_t ia, int32_t ib, int32_t ic, uint32_t flag)
{
    const int32_t mac[3] = { a, b, c }, ir[3] = { ia, ib, ic };
    for (unsigned i = 0; i < 3u; ++i) {
        CHECK(g_pe_gte.mac[i] == mac[i], "%s MAC%u: %08X expected %08X",
              name, i + 1u, (unsigned)g_pe_gte.mac[i], (unsigned)mac[i]);
        CHECK(g_pe_gte.ir[i] == ir[i], "%s IR%u: %08X expected %08X",
              name, i + 1u, (unsigned)g_pe_gte.ir[i], (unsigned)ir[i]);
    }
    CHECK(PE_GTE_ReadCtrl(31u) == flag, "%s FLAG: %08X expected %08X",
          name, (unsigned)PE_GTE_ReadCtrl(31u), (unsigned)flag);
}

static void snapshot(uint32_t registers[64])
{
    for (unsigned n = 0; n < 32u; ++n) {
        registers[n] = PE_GTE_ReadData(n);
        registers[n + 32u] = PE_GTE_ReadCtrl(n);
    }
}

static void fixed_cases(void)
{
    reset(); identity(g_pe_gte.llm);
    PE_GTE_SetV1(17, -23, 31);
    PE_GTE_SetBK(2, 3, 4);
    PE_GTE_MVMVA(mvmva_word(1, 1, 1, 1, 0));
    vector_result("LLM V1 BK", 19, -20, 35, 19, -20, 35, 0);

    reset(); identity(g_pe_gte.lcm);
    PE_GTE_SetV2(300, -400, 500);
    PE_GTE_Command(mvmva_word(1, 2, 2, 3, 1));
    vector_result("LCM V2 lm", 300, -400, 500, 300, 0, 500, 0x80800000u);

    reset(); identity(g_pe_gte.rt);
    PE_GTE_SetV0(5, 6, 7);
    g_pe_gte.fc[0] = 100; g_pe_gte.fc[1] = 200; g_pe_gte.fc[2] = 300;
    PE_GTE_MVMVA(mvmva_word(1, 0, 0, 2, 0));
    vector_result("FC discarded first term", 0, 6, 7, 0, 6, 7, 0);

    reset(); identity(g_pe_gte.rt);
    PE_GTE_SetV0(1, 2, 3); g_pe_gte.fc[0] = INT32_MAX;
    PE_GTE_Command(mvmva_word(1, 0, 0, 2, 0));
    vector_result("FC discarded term flags", 0, 2, 3, 0, 2, 3, 0xC1000000u);

    reset(); g_pe_gte.rgbc = 1u; g_pe_gte.ir0 = 7;
    g_pe_gte.rt[0][2] = 11; g_pe_gte.rt[1][1] = 13;
    PE_GTE_SetV2(2, 3, 5);
    PE_GTE_MVMVA(mvmva_word(0, 3, 2, 3, 0));
    vector_result("reserved matrix", 51, 110, 130, 51, 110, 130, 0);

    reset(); g_pe_gte.tr[0] = 0x100000;
    PE_GTE_MVMVA(mvmva_word(0, 0, 0, 0, 0));
    vector_result("IR clamps narrowed MAC", 0, 0, 0, 0, 0, 0, 0);

    reset(); PE_GTE_SetIR(-32768, 32767, 0);
    PE_GTE_Command(0x28u);
    vector_result("SQR", 0x40000000, 0x3FFF0001, 0,
                  32767, 32767, 0, 0x81800000u);

    reset();
    g_pe_gte.sxy[0] = 0xFC00FC00u; g_pe_gte.sxy[1] = 0xFC0003FFu;
    g_pe_gte.sxy[2] = 0x03FFFC00u;
    CHECK(PE_GTE_NCLIP() == 4190209, "NCLIP clean result");
    CHECK(PE_GTE_ReadCtrl(31u) == 0, "NCLIP clears stale FLAG");
    g_pe_gte.sxy[0] = 0x80008000u; g_pe_gte.sxy[1] = 0x80007FFFu;
    g_pe_gte.sxy[2] = 0x7FFF8000u;
    (void)PE_GTE_NCLIP();
    CHECK(PE_GTE_ReadCtrl(31u) == 0x80010000u, "NCLIP positive MAC0 overflow FLAG");

    reset();
    for (unsigned n = 0; n < 32u; ++n) {
        const uint32_t v = 0x5A31C789u + n;
        PE_GTE_WriteData(n + 64u, v);
        CHECK(PE_GTE_ReadData(n + 96u) == PE_GTE_ReadData(n),
              "data register %u alias read", n);
        PE_GTE_WriteCtrl(n + 64u, v);
        CHECK(PE_GTE_ReadCtrl(n + 96u) == PE_GTE_ReadCtrl(n),
              "control register %u alias read", n);
    }

    {
        uint32_t before[64], after[64];
        snapshot(before);
        CHECK(PE_GTE_ExecuteNative(0u) == -1, "unknown command refused");
        snapshot(after);
        CHECK(memcmp(before, after, sizeof before) == 0,
              "unknown command preserves all registers");
    }
}

/* Sub-pixel sidecar (Graphics > Sub-pixel Geometry): RTPS records the
 * fraction of its 16.16 screen position under the SXY word it writes; a
 * saturated axis records 0; every GTE register is the same with capture on. */
static void project(int16_t x, int16_t y, int16_t z, uint32_t regs[64])
{
    reset(); identity(g_pe_gte.rt);
    g_pe_gte.h = 1000;
    g_pe_gte.ofx = 160 * 65536 + 0x8000;   /* 160.5 */
    g_pe_gte.ofy = 120 * 65536 + 0x4000;   /* 120.25 */
    PE_GTE_SetV0(x, y, z);
    PE_GTE_Command(0x180001u);              /* RTPS sf=1 */
    snapshot(regs);
}

static void subpixel_cases(void)
{
    uint32_t off[64], on[64], frac = 0;
    CHECK(!PE_GTE_GetSubpixelCapture(), "sub-pixel capture is off by default");
    project(0, 0, 1000, off);
    CHECK(!PE_GTE_SubpixelLookup(off[14], &frac), "nothing is recorded while capture is off");
    PE_GTE_SetSubpixelCapture(1);
    CHECK(PE_GTE_GetSubpixelCapture(), "sub-pixel capture on");
    project(0, 0, 1000, on);
    CHECK(memcmp(off, on, sizeof off) == 0, "RTPS registers are identical with capture on");
    CHECK(on[14] == 0x007800A0u, "SXY2 is (160,120): %08X", (unsigned)on[14]);
    CHECK(PE_GTE_SubpixelLookup(on[14], &frac) && frac == (0x80u | (0x40u << 8)),
          "fraction x 0.5, y 0.25 recorded under SXY2: %04X", (unsigned)frac);
    project(2000, 0, 100, on);                /* x saturates at 0x3FF */
    CHECK((on[63] & (1u << 14)) && (on[14] & 0xFFFFu) == 0x3FFu, "SX2 saturates (FLAG 14)");
    CHECK(PE_GTE_SubpixelLookup(on[14], &frac) && frac == (0x40u << 8),
          "a saturated axis records fraction 0: %04X", (unsigned)frac);
    CHECK(!PE_GTE_SubpixelLookup(0x12345678u, &frac), "an unknown word is not found");
    PE_GTE_SetSubpixelCapture(0);
    CHECK(!PE_GTE_SubpixelLookup(0x007800A0u, &frac), "switching capture off clears the table");
}

static void load_registers(const uint32_t in[64])
{
    for (unsigned n = 0; n < 32u; ++n) PE_GTE_WriteData(n, in[n]);
    for (unsigned n = 0; n < 32u; ++n) PE_GTE_WriteCtrl(n, in[n + 32u]);
}

static void compare_registers(const char *which, unsigned test, const uint32_t expected[64])
{
    uint32_t actual[64];
    snapshot(actual);
    for (unsigned n = 0; n < 64u; ++n)
        CHECK(actual[n] == expected[n], "%s hardware case %u r[%u]: %08X expected %08X",
              which, test, n, (unsigned)actual[n], (unsigned)expected[n]);
}

static void run_hardware_case(unsigned test, const uint32_t in[64],
                              const uint32_t expected[64], unsigned op,
                              unsigned sf, unsigned lm, unsigned tx,
                              unsigned vx, unsigned mx)
{
    const uint32_t word = 0x4A000000u | op | (sf << 19) | (lm << 10) |
                          (tx << 13) | (vx << 15) | (mx << 17);
    load_registers(in); PE_GTE_Command(word);
    compare_registers("command", test, expected);

    load_registers(in);
    switch (op) {
    case 0x12u: PE_GTE_MVMVA(word & ~0x3Fu); break; /* short form too */
    case 0x06u: (void)PE_GTE_NCLIP(); break;
    case 0x0Cu: PE_GTE_OP((int)sf, (int)lm); break;
    case 0x3Du: PE_GTE_GPF((int)sf, (int)lm); break;
    case 0x3Eu: PE_GTE_GPL((int)sf, (int)lm); break;
    case 0x3Fu: if (sf != 1u || lm != 1u) return; PE_GTE_NCCT(); break;
    case 0x01u: {
        uint32_t xy, z;
        if (sf != 1u || lm != 0u) return;
        PE_GTE_RTPS_coordinates(&xy, &z);
        CHECK(xy == expected[14] && z == expected[19], "RTPS returned coordinates case %u", test);
        break;
    }
    case 0x30u: {
        uint32_t xy[3], z[3];
        if (sf != 1u || lm != 0u) return;
        PE_GTE_RTPT_coordinates(xy, z);
        for (unsigned i = 0; i < 3u; ++i)
            CHECK(xy[i] == expected[12u + i] && z[i] == expected[17u + i],
                  "RTPT returned coordinates case %u vertex %u", test, i);
        break;
    }
    default: return;
    }
    compare_registers("helper", test, expected);
}

static void hardware_log(const char *path)
{
    FILE *f = fopen(path, "r");
    char line[256], name[32];
    uint32_t in[64] = {0}, expected[64] = {0};
    uint64_t input_mask = 0, output_mask = 0;
    unsigned op = 0, sf = 0, lm = 0, tx = 0, vx = 0, mx = 0;
    unsigned have_command = 0, active = 0, cases = 0, seen[64] = {0};
    CHECK(f != NULL, "cannot open supplied hardware log: %s", path);
    if (!f) return;
    for (;;) {
        char *s = fgets(line, sizeof line, f);
        const int boundary = !s || !strncmp(s, "Test ", 5) || !strncmp(s, "----", 4);
        if (boundary && active) {
            CHECK(have_command && input_mask == UINT64_MAX && output_mask == UINT64_MAX,
                  "incomplete hardware case %u", cases + 1u);
            if (!have_command || input_mask != UINT64_MAX || output_mask != UINT64_MAX) {
                fclose(f); return;
            }
            ++cases; ++seen[op];
            run_hardware_case(cases, in, expected, op, sf, lm, tx, vx, mx);
        }
        if (!s) break;
        /* The log closes with a "GTE 0x40 ---" section: 50 register dumps with
         * no command (an opcode beyond the 6-bit field).  The 22 documented
         * operations end there. */
        {
            unsigned section;
            if (sscanf(s, "-------------- GTE 0x%x", &section) == 1 && section >= 0x40u) break;
        }
        if (boundary) { active = 0; have_command = 0; input_mask = output_mask = 0; continue; }
        unsigned k, v;
        if (sscanf(s, "> r[%u] = %x", &k, &v) == 2 && k < 64u) {
            CHECK(!(input_mask & (UINT64_C(1) << k)), "duplicate input register");
            in[k] = v; input_mask |= UINT64_C(1) << k; active = 1; continue;
        }
        if (sscanf(s, "< r[%u] = %x", &k, &v) == 2 && k < 64u) {
            CHECK(!(output_mask & (UINT64_C(1) << k)), "duplicate output register");
            expected[k] = v; output_mask |= UINT64_C(1) << k; active = 1; continue;
        }
        if (sscanf(s, "GTE 0x%x %31s (sf=%u, lm=%u, tx=%u, vx=%u, mx=%u)",
                   &op, name, &sf, &lm, &tx, &vx, &mx) == 7) {
            CHECK(!have_command && op < 64u && sf < 2u && lm < 2u &&
                  tx < 4u && vx < 4u && mx < 4u, "invalid hardware command");
            if (op >= 64u || sf >= 2u || lm >= 2u || tx >= 4u || vx >= 4u || mx >= 4u) {
                fclose(f); return;
            }
            have_command = 1; active = 1;
        }
    }
    CHECK(!ferror(f), "hardware log read error");
    fclose(f);
    unsigned operations = 0;
    for (unsigned n = 0; n < 64u; ++n) operations += seen[n] != 0u;
    CHECK(cases == 1100u && operations == 22u,
          "hardware log coverage: %u cases, %u operations; expected 1100 and 22", cases, operations);
    printf("hardware log: %u cases, %u operations\n", cases, operations);
}

int main(int argc, char **argv)
{
    if (argc != 1 && (argc != 3 || strcmp(argv[1], "--hardware-log"))) {
        fprintf(stderr, "usage: %s [--hardware-log FILE]\n", argv[0]); return 2;
    }
    fixed_cases();
    subpixel_cases();
    PE_GTE_SetSubpixelCapture(1);           /* the sidecar changes no GTE result */
    fixed_cases();
    PE_GTE_SetSubpixelCapture(0);
    if (argc == 3) {
        hardware_log(argv[2]);
        PE_GTE_SetSubpixelCapture(1);
        hardware_log(argv[2]);
        PE_GTE_SetSubpixelCapture(0);
        puts("hardware log replayed with sub-pixel capture off and on");
    }
    else puts("hardware log: NOT RUN (fixed cases only)");
    printf("native GTE: %u checks, %u failures\n", checks, failures);
    return failures ? 1 : 0;
}
