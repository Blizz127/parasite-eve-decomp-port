#include "retail_menu_input_cases.h"

static void test_NAM4_retail_menu_input(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("NAM4_retail_menu_input"); TEST_RETAIL_FIXUPS(RETAILFIX_menu_input);
    for (k=0;k<sizeof(NAM4_menu_input_cases)/sizeof(NAM4_menu_input_cases[0]);k++) {
        const uint32_t *a=NAM4_menu_input_cases[k].args;
        uint32_t result=0;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(NAM4_menu_input_common)/sizeof(NAM4_menu_input_common[0]);i++)
            PE_StoreU32(0x80000000u+NAM4_menu_input_common[i][0],NAM4_menu_input_common[i][1]);
        for (i=NAM4_menu_input_cases[k].first;i<NAM4_menu_input_cases[k].end;i++)
            PE_StoreU32(0x80000000u+NAM4_menu_input_patches[i][0],NAM4_menu_input_patches[i][1]);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D050=PE_LoadU32(0x8009D050u);
        if (NAM4_menu_input_cases[k].first_step!=NAM4_menu_input_cases[k].end_step) {
            for (i=NAM4_menu_input_cases[k].first_step;i<NAM4_menu_input_cases[k].end_step;i++) {
                PE_StoreU32(0x8009D26Cu,NAM4_menu_input_steps[i]);func_8005E30C();
            }
        } else switch (NAM4_menu_input_cases[k].entry) {
        case 0:result=(uint32_t)func_80063E0C(a[0],a[1]);break;
        case 1:result=(uint32_t)func_800650E0(a[0],a[1]);break;
        case 2:func_8005E30C();break;
        }
        hash=hit_camera_hash(NAM4_menu_input_ranges,sizeof(NAM4_menu_input_ranges)/sizeof(NAM4_menu_input_ranges[0]));
        if (hash!=NAM4_menu_input_cases[k].hash || result!=NAM4_menu_input_cases[k].result) {
            fprintf(stderr,"menu input %u result %08X/%08X hash %016llX/%016llX\n",k,
                result,NAM4_menu_input_cases[k].result,(unsigned long long)hash,(unsigned long long)NAM4_menu_input_cases[k].hash);
            if (getenv("PE_NAM4_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-nam4-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(result==NAM4_menu_input_cases[k].result,"menu input return differs from original");
        ASSERT(hash==NAM4_menu_input_cases[k].hash,"menu input memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"menu input call graph executes natively");
    }
    PASS();
}
