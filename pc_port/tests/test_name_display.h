#include "retail_name_display_cases.h"

static void test_NAM8_retail_name_display(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("NAM8_retail_name_display"); TEST_RETAIL_FIXUPS(RETAILFIX_name_display);
    for (k=0;k<sizeof(NAM8_name_display_cases)/sizeof(NAM8_name_display_cases[0]);k++) {
        const uint32_t *a=NAM8_name_display_cases[k].args;
        uint64_t hash;uint32_t result=0u;
        ResetTestState();
        for (i=0;i<sizeof(NAM8_name_display_common)/sizeof(NAM8_name_display_common[0]);i++)
            PE_StoreU32(0x80000000u+NAM8_name_display_common[i][0],NAM8_name_display_common[i][1]);
        for (i=NAM8_name_display_cases[k].first;i<NAM8_name_display_cases[k].end;i++)
            PE_StoreU32(0x80000000u+NAM8_name_display_patches[i][0],NAM8_name_display_patches[i][1]);
        for (i=0;i<NAM8_name_display_cases[k].frame;i++) PE_GPU_VBlankStep();
        switch (NAM8_name_display_cases[k].entry) {
        case 0:func_8005F874((int32_t)a[0]);break;
        case 1:func_8005FB74((int32_t)a[0]);break;
        case 2:func_8006006C((int32_t)a[0],a[1]);break;
        case 3:result=func_80052894(a[0]);break;
        case 4:result=func_8005DD3C(a[0]);break;
        case 5:func_800534E4(a[0],a[1]);break;
        case 6:func_80053648(a[0]);break;
        case 7:func_80061A3C(a[0],a[1],a[2]);break;
        case 8:func_8004DF74(a[0]);break;
        case 9:func_8004C608(a[0]);break;
        case 10:func_80062FEC();break;
        }
        hash=hit_camera_hash(NAM8_name_display_ranges,sizeof(NAM8_name_display_ranges)/sizeof(NAM8_name_display_ranges[0]));
        if (hash!=NAM8_name_display_cases[k].hash || result!=NAM8_name_display_cases[k].result) {
            fprintf(stderr,"name display %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)NAM8_name_display_cases[k].hash,
                result,NAM8_name_display_cases[k].result);
            if (getenv("PE_NAM8_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-nam8-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==NAM8_name_display_cases[k].hash,"name display memory differs from original");
        ASSERT(result==NAM8_name_display_cases[k].result,"name display result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"name display call graph executes natively");
        ASSERT(PE_GPU_VSyncQuery()==NAM8_name_display_cases[k].frame,"drawing must not advance VBlank time");
    }
    PASS();
}
