#include "retail_name_input_cases.h"

static void test_NAM3_retail_name_input(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("NAM3_retail_name_input"); TEST_RETAIL_FIXUPS(RETAILFIX_name_input);
    for (k=0;k<sizeof(NAM3_input_cases)/sizeof(NAM3_input_cases[0]);k++) {
        const uint32_t *a=NAM3_input_cases[k].args;
        uint32_t result=0;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(NAM3_input_common)/sizeof(NAM3_input_common[0]);i++)
            PE_StoreU32(0x80000000u+NAM3_input_common[i][0],NAM3_input_common[i][1]);
        for (i=NAM3_input_cases[k].first;i<NAM3_input_cases[k].end;i++)
            PE_StoreU32(0x80000000u+NAM3_input_patches[i][0],NAM3_input_patches[i][1]);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D050=PE_LoadU32(0x8009D050u);
        switch (NAM3_input_cases[k].entry) {
        case 0:result=func_8005E038();break;
        case 1:func_8005E12C((int32_t)a[0]);break;
        case 2:func_8005E114((int32_t)a[0]);break;
        case 3:result=(uint32_t)func_8004E074(a[0],a[1]);break;
        case 4:result=(uint32_t)func_8004E2E4(a[0],a[1]);break;
        }
        hash=hit_camera_hash(NAM3_input_ranges,sizeof(NAM3_input_ranges)/sizeof(NAM3_input_ranges[0]));
        if (hash!=NAM3_input_cases[k].hash || result!=NAM3_input_cases[k].result) {
            fprintf(stderr,"name input %u result %08X/%08X hash %016llX/%016llX\n",k,
                result,NAM3_input_cases[k].result,(unsigned long long)hash,(unsigned long long)NAM3_input_cases[k].hash);
            if (getenv("PE_NAM3_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-nam3-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(result==NAM3_input_cases[k].result,"name input return differs from original");
        ASSERT(hash==NAM3_input_cases[k].hash,"name input memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"name input call graph executes natively");
    }
    PASS();
}
