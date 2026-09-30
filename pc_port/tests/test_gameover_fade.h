#include "retail_gameover_fade_cases.h"

static void test_INV17_retail_gameover_fade(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV17_retail_gameover_fade"); TEST_RETAIL_FIXUPS(RETAILFIX_gameover_fade);
    for (k=0;k<sizeof(INV17_gameover_fade_cases)/sizeof(INV17_gameover_fade_cases[0]);k++) {
        const uint32_t *args=INV17_gameover_fade_cases[k].args;
        uint32_t result=0u;
        (void)args;
        uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(INV17_gameover_fade_common)/sizeof(INV17_gameover_fade_common[0]);i++)
            PE_StoreU32(0x80000000u+INV17_gameover_fade_common[i][0],INV17_gameover_fade_common[i][1]);
        for (i=INV17_gameover_fade_cases[k].first;i<INV17_gameover_fade_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV17_gameover_fade_patches[i][0],INV17_gameover_fade_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        D_8009D250=PE_LoadU32(0x8009D250u);g_pe_gte.h=256;
        func_8002B29C();
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV17_gameover_fade_ranges,sizeof(INV17_gameover_fade_ranges)/sizeof(INV17_gameover_fade_ranges[0]));
        if (hash!=INV17_gameover_fade_cases[k].hash) {
            fprintf(stderr,"Game-over fade %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV17_gameover_fade_cases[k].hash,
                result,INV17_gameover_fade_cases[k].result);
            if (getenv("PE_INV17_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv17-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV17_gameover_fade_cases[k].hash,"Game-over fade memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"Game-over fade call graph executes natively");
    }
    PASS();
}
