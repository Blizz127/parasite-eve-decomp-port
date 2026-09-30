#include "retail_script_audio_control_cases.h"
#include "retail_day2_audio_dispatch_cases.h"
static void test_DAY2_script_audio_control(void)
{
    TEST("DAY2_script_audio_control");
    for (unsigned k=0;k<sizeof(SAC_cases)/sizeof(SAC_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(SAC_common)/sizeof(SAC_common[0]);i++)
            PE_StoreU32(0x80000000u+SAC_common[i][0],SAC_common[i][1]);
        for (unsigned i=SAC_cases[k].first;i<SAC_cases[k].end;i++)
            PE_StoreU32(0x80000000u+SAC_patches[i][0],SAC_patches[i][1]);
        ASSERT(func_80015DAC_default_cut(0x80140200u)==1,"EA305/314 control completes");
        hash=hit_camera_hash(SAC_ranges,sizeof(SAC_ranges)/sizeof(SAC_ranges[0]));
        ASSERT(hash==SAC_cases[k].hash,"EA305/314 audio control and FIFO differ from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"audio control completes natively");
    }
    PASS();
}

static void test_DAY2_audio_dispatch(void)
{
    TEST("DAY2_audio_dispatch");
    for (unsigned k = 0; k < sizeof(D2AD_cases) / sizeof(D2AD_cases[0]); k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i = 0; i < sizeof(D2AD_ranges) / sizeof(D2AD_ranges[0]); i++)
            for (unsigned j = 0; j < D2AD_ranges[i][1]; j += 4u)
                PE_StoreU32(0x80000000u + D2AD_ranges[i][0] + j, 0u);
        for (unsigned i = 0; i < sizeof(D2AD_common) / sizeof(D2AD_common[0]); i++)
            PE_StoreU32(0x80000000u + D2AD_common[i][0], D2AD_common[i][1]);
        for (unsigned i = D2AD_cases[k].first; i < D2AD_cases[k].end; i++)
            PE_StoreU32(0x80000000u + D2AD_patches[i][0], D2AD_patches[i][1]);
        ASSERT(func_80015DAC_default_cut(0x80140200u) == (int)D2AD_cases[k].result,
               "EA control completion/retry differs from original");
        hash = hit_camera_hash(D2AD_ranges, sizeof(D2AD_ranges) / sizeof(D2AD_ranges[0]));
        if (hash != D2AD_cases[k].hash)
            fprintf(stderr, "audio dispatch case %u key %u: %016llX/%016llX\n", k,
                    PE_LoadU32(0x80140240u), (unsigned long long)hash,
                    (unsigned long long)D2AD_cases[k].hash);
        ASSERT(hash == D2AD_cases[k].hash, "EA control guest bytes differ from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(), "EA control graph completes natively");

        if (PE_LoadU32(0x80140240u) == 409u) {
            uint8_t volumes[4];
            uint8_t value = PE_LoadU8(0x80140244u);
            /* Repeat with the actual CD register addresses. The golden RAM
             * run above compares the SDK's pointer-based stores; this checks
             * bank selection and application to all four hardware gains. */
            for (unsigned i = 0; i < 4; i++)
                PE_StoreU32(0x8009B27Cu + i * 4u, 0x1F801800u + i);
            ASSERT(func_80015DAC_default_cut(0x80140200u) == 1, "EA409 applies CD gains");
            PE_CdReg_GetAudioVolumes(volumes);
            ASSERT(volumes[0] == value && volumes[1] == 0u &&
                   volumes[2] == value && volumes[3] == 0u,
                   "EA409 applies retail stereo gains with zero crossfeed");
        }
    }
    PASS();
}
