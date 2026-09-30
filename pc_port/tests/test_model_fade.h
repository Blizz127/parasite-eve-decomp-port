#include "retail_model_fade_cases.h"
static void test_DAY1_model_fade(void)
{
    TEST_RETAIL_DISC1("DAY1_model_fade"); TEST_RETAIL_FIXUPS(RETAILFIX_model_fade);
    for (unsigned k=0;k<sizeof(model_fade_cases)/sizeof(model_fade_cases[0]);k++) {
        const uint32_t *a=model_fade_cases[k].args;
        uint32_t result=0;
        uint64_t hash=UINT64_C(14695981039346656037);
        if (!model_fade_cases[k].frame) {
            ResetTestState();
            for (unsigned i=0;i<sizeof(model_fade_common)/sizeof(model_fade_common[0]);i++)
                PE_StoreU32(0x80000000u+model_fade_common[i][0],model_fade_common[i][1]);
            for (unsigned i=model_fade_cases[k].first;i<model_fade_cases[k].end;i++)
                PE_StoreU32(0x80000000u+model_fade_patches[i][0],model_fade_patches[i][1]);
        }
        PE_StoreU32(0x8009CDDCu,model_fade_cases[k].bank);
        func_8006698C(a[0]);
        switch (model_fade_cases[k].entry) {
        case 0:result=func_8003C638(a[0]);break;
        case 1:result=func_8003C818(a[0]);break;
        case 2:func_8003CCB0(a[0],a[1]);break;
        case 3:func_8003CEF8(a[0],a[1]);break;
        case 4:func_8003B708(a[0],a[1]);break;
        case 5:func_8003C0B4(a[0],a[1],a[2],a[3],a[4]);break;
        case 6:func_8003AF14(a[0],a[1]);break;
        }
        for (unsigned i=0;i<sizeof(model_fade_ranges)/sizeof(model_fade_ranges[0]);i++)
            for (unsigned j=0;j<model_fade_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(0x80000000u+model_fade_ranges[i][0]+j))*UINT64_C(1099511628211);
        if (hash!=model_fade_cases[k].hash || result!=model_fade_cases[k].result || PE_Port_ShouldStop()) {
            fprintf(stderr,"model fade case%u result%X/%X hash%016llX/%016llX\n",k,result,model_fade_cases[k].result,
                (unsigned long long)hash,(unsigned long long)model_fade_cases[k].hash);
            if (getenv("PE_MODEL_FADE_DUMP")) {
                char path[100];snprintf(path,sizeof(path),"/tmp/pe-model-fade-native-%u.bin",k);
                FILE *out=fopen(path,"wb");
                if(out){for(unsigned i=0;i<0x200000u;i++)fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(result==model_fade_cases[k].result,"model fade result differs from original instructions");
        ASSERT(hash==model_fade_cases[k].hash,"model fade bytes differ from original instructions");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"model fades execute natively");
    }
    ResetTestState();PASS();
}
