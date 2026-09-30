/* Port lane round 7: hand adapters vs the retail-instruction oracle
 * (pc_port/tools/pe_port7_oracle.py, which runs the retail EXE). */
#include "retail_port7_cases.h"

static void test_P7_retail_hand_adapters(void)
{
    unsigned k, i, bad = 0u;
    TEST_RETAIL_DISC1("P7_retail_hand_adapters"); TEST_RETAIL_FIXUPS(RETAILFIX_port7);
    for (k = 0; k < sizeof(P7_cases) / sizeof(P7_cases[0]); k++) {
        const uint32_t *a = P7_cases[k].args;
        uint32_t result = 0;
        uint64_t hash;
        ResetTestState();
        PE_Decomp_ResetBoundaries();
        for (i = 0; i < sizeof(P7_ranges) / sizeof(P7_ranges[0]); i++)
            PE_Fill(0x80000000u + P7_ranges[i][0], P7_ranges[i][1], 0);
        for (i = 0; i < sizeof(P7_common) / sizeof(P7_common[0]); i++)
            PE_StoreU32(P7_common[i][0], P7_common[i][1]);
        for (i = P7_cases[k].first; i < P7_cases[k].end; i++) {
            unsigned j;
            for (j = 0; j < P7_runs[i][1]; j++)
                PE_StoreU32(P7_runs[i][0] + 4u * j, P7_words[P7_runs[i][2] + j]);
        }
        if (getenv("PE_P7_TRACE"))
            fprintf(stderr, "P7 case %u entry %u\n", k, P7_cases[k].entry);
        switch (P7_cases[k].entry) {
        case 0: result = (uint32_t)func_80068710(a[0], a[1], (unsigned char)a[2], (unsigned char)a[3]); break;
        case 1: result = (uint32_t)func_80083644(a[0]); break;
        case 2: func_80084C4C(a[0]); result = P7_cases[k].result; break;
        case 3: func_8001CBA0(a[0], a[1], (unsigned short)a[2], (short)a[3]); result = P7_cases[k].result; break;
        case 5: func_8008A068(a[0]); result = P7_cases[k].result; break;
        case 6: func_80081E70((unsigned char)a[0], a[1]); result = P7_cases[k].result; break;
        case 7: func_8001D170(); result = P7_cases[k].result; break;
        case 8: result = (uint32_t)func_800DE7A8((int)a[0], a[1]); break;
        case 9: result = (uint32_t)func_80058E44((int)a[0]); break;
        case 10: result = (uint32_t)func_80058FEC((int)a[0], (int)a[1], (int)a[2], (int)a[3]); break;
        case 11: result = (uint32_t)func_80054F58((int)a[0], (int)a[1]); break;
        case 12: func_80088344(a[0], a[1]); result = P7_cases[k].result; break;
        case 13: func_80048254(); result = P7_cases[k].result; break;
        default: result = (uint32_t)func_8001CE88((short)a[0], (short)a[1], a[2], (unsigned short)a[3]); break;
        }
        hash = hit_camera_hash(P7_ranges, sizeof(P7_ranges) / sizeof(P7_ranges[0]));
        if (getenv("PE_P7_DUMP") && (unsigned)atoi(getenv("PE_P7_DUMP")) == k) {
            char path[128];
            FILE *out;
            unsigned r, b;
            snprintf(path, sizeof(path), "build/lanes/port/p7_native_%u.bin", k);
            out = fopen(path, "wb");
            if (out) {
                for (r = 0; r < sizeof(P7_ranges) / sizeof(P7_ranges[0]); r++)
                    for (b = 0; b < P7_ranges[r][1]; b++)
                        fputc(PE_LoadU8(0x80000000u + P7_ranges[r][0] + b), out);
                fclose(out);
            }
        }
        if (hash != P7_cases[k].hash || result != P7_cases[k].result ||
            PE_Decomp_BoundaryCount() != 0 || PE_Port_ShouldStop()) {
            if (bad++ < 8u)
                fprintf(stderr, "P7 case %u (entry %u): result %08X/%08X hash %016llX/%016llX bnd %d\n",
                        k, P7_cases[k].entry, result, P7_cases[k].result,
                        (unsigned long long)hash, (unsigned long long)P7_cases[k].hash,
                        (int)PE_Decomp_BoundaryCount());
        }
    }
    ASSERT(bad == 0u, "round-7 adapter differs from the retail instructions");
    PASS();
}

static void test_P7_all(void)
{
    test_P7_retail_hand_adapters();
}
