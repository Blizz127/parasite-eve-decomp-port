#include "retail_script_polygon_cases.h"
static void test_SEW22_script_polygon(void)
{
    TEST("SEW22_script_polygon");
    for (unsigned k=0;k<sizeof(SEW22_cases)/sizeof(SEW22_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(SEW22_common)/sizeof(SEW22_common[0]);i++)
            PE_StoreU32(0x80000000u+SEW22_common[i][0],SEW22_common[i][1]);
        for (unsigned i=SEW22_cases[k].first;i<SEW22_cases[k].end;i++)
            PE_StoreU32(0x80000000u+SEW22_patches[i][0],SEW22_patches[i][1]);
        D_8009D1A0=0;
        if (SEW22_cases[k].vm) func_80017018();
        else { ASSERT(func_8001A390(0x80140000u)==1,"polygon script completes"); }
        hash=hit_camera_hash(SEW22_ranges,sizeof(SEW22_ranges)/sizeof(SEW22_ranges[0]));
        if (hash!=SEW22_cases[k].hash) {
            fprintf(stderr,"script polygon %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)SEW22_cases[k].hash);
            if (getenv("PE_SEW22_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"local/live/script-polygon-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (unsigned i=0;i<0x200000u;i++)fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==SEW22_cases[k].hash,"script polygon memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"script polygon graph executes natively");
    }
    PASS();
}
