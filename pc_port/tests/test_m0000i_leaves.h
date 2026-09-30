#include "retail_m0000i_leaves_cases.h"
static void test_DAY1_m0000i_leaves(void)
{
    TEST("DAY1_m0000i_leaves");
    for (unsigned k = 0; k < sizeof(DAY1_m0000i_cases) / sizeof(DAY1_m0000i_cases[0]); k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i = 0; i < sizeof(DAY1_m0000i_common) / sizeof(DAY1_m0000i_common[0]); i++)
            PE_StoreU32(0x80000000u + DAY1_m0000i_common[i][0], DAY1_m0000i_common[i][1]);
        for (unsigned i = DAY1_m0000i_cases[k].first; i < DAY1_m0000i_cases[k].end; i++)
            PE_StoreU32(0x80000000u + DAY1_m0000i_patches[i][0], DAY1_m0000i_patches[i][1]);
        int ret = 0;
        if (DAY1_m0000i_cases[k].kind == 0u)
            func_80191DE8((int)DAY1_m0000i_cases[k].arg);
        else if (DAY1_m0000i_cases[k].kind == 1u)
            func_8019BF8C(DAY1_m0000i_cases[k].arg);
        else if (DAY1_m0000i_cases[k].kind == 2u)
            func_80193AB0();
        else if (DAY1_m0000i_cases[k].kind == 3u)
            func_801941A4(DAY1_m0000i_cases[k].arg);
        else if (DAY1_m0000i_cases[k].kind == 4u)
            ret = func_80194108((int)DAY1_m0000i_cases[k].arg);
        else if (DAY1_m0000i_cases[k].kind == 5u)
            func_80192740();
        else
            ret = func_80193B5C((int)DAY1_m0000i_cases[k].arg);
        hash = hit_camera_hash(DAY1_m0000i_ranges,
                               sizeof(DAY1_m0000i_ranges) / sizeof(DAY1_m0000i_ranges[0]));
        if (hash != DAY1_m0000i_cases[k].hash) {
            fprintf(stderr, "m0000i leaf %u hash %016llX/%016llX\n", k,
                    (unsigned long long)hash,
                    (unsigned long long)DAY1_m0000i_cases[k].hash);
            for (unsigned r = 0; r < sizeof(DAY1_m0000i_ranges) / sizeof(DAY1_m0000i_ranges[0]); r++) {
                unsigned a = DAY1_m0000i_ranges[r][0], n = DAY1_m0000i_ranges[r][1], b;
                fprintf(stderr, "  %06X:", a);
                for (b = 0; b < n; b++)
                    fprintf(stderr, " %02X", PE_LoadU8(0x80000000u + a + b));
                fprintf(stderr, "\n");
            }
        }
        ASSERT(hash == DAY1_m0000i_cases[k].hash, "M0000I leaf memory differs from original");
        if (DAY1_m0000i_cases[k].kind == 4u || DAY1_m0000i_cases[k].kind == 6u)
            ASSERT((uint32_t)ret == DAY1_m0000i_cases[k].ret, "M0000I fade stepper v0 differs");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(), "M0000I leaf executes natively");
    }
    PASS();
}
