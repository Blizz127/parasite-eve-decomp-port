#include "retail_script_readiness_cases.h"
static void test_DAY2_script_readiness(void)
{
    TEST("DAY2_script_readiness");
    for (unsigned k=0;k<sizeof(SRD_cases)/sizeof(SRD_cases[0]);k++) {
        unsigned result;uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(SRD_common)/sizeof(SRD_common[0]);i++)
            PE_StoreU32(0x80000000u+SRD_common[i][0],SRD_common[i][1]);
        for (unsigned i=SRD_cases[k].first;i<SRD_cases[k].end;i++)
            PE_StoreU32(0x80000000u+SRD_patches[i][0],SRD_patches[i][1]);
        result=(unsigned)func_80016910_key2900_cut(0x80154000u);
        hash=hit_camera_hash(SRD_ranges,sizeof(SRD_ranges)/sizeof(SRD_ranges[0]));
        if (result!=SRD_cases[k].result || hash!=SRD_cases[k].hash)
            fprintf(stderr,"readiness%u result%u/%u hash%016llX/%016llX\n",k,result,SRD_cases[k].result,(unsigned long long)hash,(unsigned long long)SRD_cases[k].hash);
        ASSERT(result==SRD_cases[k].result && hash==SRD_cases[k].hash,"ED3100 readiness differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"script readiness completed natively");
    }
    PASS();
}
