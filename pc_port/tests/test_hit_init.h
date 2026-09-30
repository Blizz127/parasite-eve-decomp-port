#include "retail_hit_init_cases.h"
static void test_DAY1_hit_init(void)
{
    static const pe_addr_t entry[]={0x800CD980u,0x800CDA5Cu,0x800CDC24u};
    TEST("DAY1_hit_init");
    for(unsigned k=0;k<sizeof(DAY1_hit_init_cases)/sizeof(DAY1_hit_init_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(DAY1_hit_init_common)/sizeof(DAY1_hit_init_common[0]);i++)
            PE_StoreU32(ATK28_mesh_address(DAY1_hit_init_common[i][0]),DAY1_hit_init_common[i][1]);
        for(unsigned i=DAY1_hit_init_cases[k].first;i<DAY1_hit_init_cases[k].end;i++)
            PE_StoreU32(ATK28_mesh_address(DAY1_hit_init_patches[i][0]),DAY1_hit_init_patches[i][1]);
        func_80071A64(DAY1_hit_init_cases[k].seed);
        int result=0;
        if(DAY1_hit_init_cases[k].entry==3u)result=func_800C2758(0x80150000u,0x80171100u,0x80171180u);
        else PE_WeaponCallback(entry[DAY1_hit_init_cases[k].entry],0x80150000u,0x80150080u,0x80150200u);
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned i=0;i<sizeof(DAY1_hit_init_ranges)/sizeof(DAY1_hit_init_ranges[0]);i++)
            for(unsigned j=0;j<DAY1_hit_init_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(ATK28_mesh_address(DAY1_hit_init_ranges[i][0]+j)))*UINT64_C(1099511628211);
        if(hash!=DAY1_hit_init_cases[k].hash || (unsigned)result!=DAY1_hit_init_cases[k].result) {
            fprintf(stderr,"hit init case%u result%d/%u hash%016llX/%016llX\n",k,result,DAY1_hit_init_cases[k].result,
                (unsigned long long)hash,(unsigned long long)DAY1_hit_init_cases[k].hash);
            if(getenv("PE_DAY1_HIT_INIT_DUMP")) {
                FILE *out=fopen("local/live/hit-init-native-mismatch.bin","wb");
                if(out){for(unsigned i=0;i<0x200000u;i++)fputc(PE_LoadU8(ATK28_mesh_address(i)),out);fclose(out);}
            }
        }
        ASSERT((unsigned)result==DAY1_hit_init_cases[k].result,"hit init return differs from original");
        ASSERT(hash==DAY1_hit_init_cases[k].hash,"hit init RAM differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"hit initialization executes natively");
    }
    PASS();
}
