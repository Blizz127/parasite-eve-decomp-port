#include "retail_music_start_cases.h"
static void test_DAY1_music_start(void)
{
    TEST("DAY1_music_start");
    for (unsigned k = 0; k < sizeof(DAY1_music_cases) / sizeof(DAY1_music_cases[0]); k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i = 0; i < sizeof(DAY1_music_common) / sizeof(DAY1_music_common[0]); i++)
            PE_StoreU32(0x80000000u + DAY1_music_common[i][0], DAY1_music_common[i][1]);
        for (unsigned i = DAY1_music_cases[k].first; i < DAY1_music_cases[k].end; i++)
            PE_StoreU32(0x80000000u + DAY1_music_patches[i][0], DAY1_music_patches[i][1]);
        ASSERT(func_80015DAC_default_cut(0x80140000u) == (int)DAY1_music_cases[k].result,
               "music script result");
        hash = hit_camera_hash(DAY1_music_ranges,
                               sizeof(DAY1_music_ranges) / sizeof(DAY1_music_ranges[0]));
        if (hash != DAY1_music_cases[k].hash) {
            fprintf(stderr, "music start %u hash %016llX/%016llX\n", k,
                    (unsigned long long)hash,
                    (unsigned long long)DAY1_music_cases[k].hash);
            if (getenv("PE_DAY1_MUSIC_DUMP")) {
                char path[100];
                FILE *out;
                snprintf(path, sizeof(path), "local/live/music-start-native-%u.bin", k);
                out = fopen(path, "wb");
                if (out) {
                    for (unsigned i = 0; i < 0x200000u; i++)
                        fputc(PE_LoadU8(0x80000000u + i), out);
                    fclose(out);
                }
            }
        }
        ASSERT(hash == DAY1_music_cases[k].hash, "music start memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(), "music start graph executes natively");
    }
    PASS();
}
