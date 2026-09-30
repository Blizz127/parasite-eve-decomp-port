#include "retail_menu_text_cases.h"

static void test_NAM5_retail_menu_text(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("NAM5_retail_menu_text"); TEST_RETAIL_FIXUPS(RETAILFIX_menu_text);
    for (k=0;k<sizeof(NAM5_menu_text_cases)/sizeof(NAM5_menu_text_cases[0]);k++) {
        const uint32_t *a=NAM5_menu_text_cases[k].args;
        uint32_t result=0;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(NAM5_menu_text_common)/sizeof(NAM5_menu_text_common[0]);i++)
            PE_StoreU32(0x80000000u+NAM5_menu_text_common[i][0],NAM5_menu_text_common[i][1]);
        for (i=NAM5_menu_text_cases[k].first;i<NAM5_menu_text_cases[k].end;i++)
            PE_StoreU32(0x80000000u+NAM5_menu_text_patches[i][0],NAM5_menu_text_patches[i][1]);
        switch (NAM5_menu_text_cases[k].entry) {
        case 0:func_8005EED4(a[0]);break;
        case 1:func_8005EB64(a[0]);break;
        case 2:result=func_8005F1A0(a[0]);break;
        case 3:func_8005F27C(a[0]);break;
        case 4:func_8005F5B8(a[0]);break;
        case 5:func_8005E8C4();break;
        case 6:func_8005E914();break;
        case 7:func_8005E8A4((int32_t)a[0],(int32_t)a[1]);break;
        case 8:func_80050F10(a[0]);break;
        case 9:func_80050F64(a[0]);break;
        case 10:func_80050FB8(a[0]);break;
        case 11:func_8005100C(a[0]);break;
        case 12:func_8005E6F0();break;
        }
        hash=hit_camera_hash(NAM5_menu_text_ranges,sizeof(NAM5_menu_text_ranges)/sizeof(NAM5_menu_text_ranges[0]));
        if (hash!=NAM5_menu_text_cases[k].hash || result!=NAM5_menu_text_cases[k].result) {
            fprintf(stderr,"menu text %u result %08X/%08X hash %016llX/%016llX\n",k,
                result,NAM5_menu_text_cases[k].result,(unsigned long long)hash,(unsigned long long)NAM5_menu_text_cases[k].hash);
            if (getenv("PE_NAM5_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-nam5-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(result==NAM5_menu_text_cases[k].result,"menu text return differs from original");
        ASSERT(hash==NAM5_menu_text_cases[k].hash,"menu text memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"menu text call graph executes natively");
    }
    PASS();
}
