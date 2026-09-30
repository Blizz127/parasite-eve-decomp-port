#include "retail_menu_list_cases.h"

static void test_NAM7_retail_menu_list(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("NAM7_retail_menu_list"); TEST_RETAIL_FIXUPS(RETAILFIX_menu_list);
    for (k=0;k<sizeof(NAM7_menu_list_cases)/sizeof(NAM7_menu_list_cases[0]);k++) {
        const uint32_t *a=NAM7_menu_list_cases[k].args;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(NAM7_menu_list_common)/sizeof(NAM7_menu_list_common[0]);i++)
            PE_StoreU32(0x80000000u+NAM7_menu_list_common[i][0],NAM7_menu_list_common[i][1]);
        for (i=NAM7_menu_list_cases[k].first;i<NAM7_menu_list_cases[k].end;i++)
            PE_StoreU32(0x80000000u+NAM7_menu_list_patches[i][0],NAM7_menu_list_patches[i][1]);
        for (i=0;i<NAM7_menu_list_cases[k].frame;i++) PE_GPU_VBlankStep();
        switch (NAM7_menu_list_cases[k].entry) {
        case 0:func_800634D4(a[0],a[1],(int32_t)a[2],a[3]);break;
        case 1:func_8006374C(a[0]);break;
        case 2:func_80065260(a[0]);break;
        case 3:func_800638D8(a[0],a[1]);break;
        case 4:func_800500A8(a[0]);break;
        case 5:func_8005010C(a[0]);break;
        case 6:func_80050178(a[0]);break;
        case 7:func_800501C8(a[0]);break;
        case 8:func_80062830(a[0]);break;
        case 9:func_80062FEC();break;
        }
        hash=hit_camera_hash(NAM7_menu_list_ranges,sizeof(NAM7_menu_list_ranges)/sizeof(NAM7_menu_list_ranges[0]));
        if (hash!=NAM7_menu_list_cases[k].hash) {
            fprintf(stderr,"menu list %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)NAM7_menu_list_cases[k].hash);
            if (getenv("PE_NAM7_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-nam7-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==NAM7_menu_list_cases[k].hash,"menu list memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"menu list call graph executes natively");
        ASSERT(PE_GPU_VSyncQuery()==NAM7_menu_list_cases[k].frame,"drawing must not advance VBlank time");
    }
    PASS();
}
