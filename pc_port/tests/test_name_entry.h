#include "retail_name_entry_cases.h"

static void test_NAM2_retail_name_entry(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("NAM2_retail_name_entry"); TEST_RETAIL_FIXUPS(RETAILFIX_name_entry);
    for (k=0;k<sizeof(NAM2_name_cases)/sizeof(NAM2_name_cases[0]);k++) {
        const uint32_t *a=NAM2_name_cases[k].args;
        uint32_t result=0;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(NAM2_name_common)/sizeof(NAM2_name_common[0]);i++)
            PE_StoreU32(0x80000000u+NAM2_name_common[i][0],NAM2_name_common[i][1]);
        for (i=NAM2_name_cases[k].first;i<NAM2_name_cases[k].end;i++)
            PE_StoreU32(0x80000000u+NAM2_name_patches[i][0],NAM2_name_patches[i][1]);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D050=PE_LoadU32(0x8009D050u);
        switch (NAM2_name_cases[k].entry) {
        case 0:func_8005BD10(a[0]);break;
        case 1:result=(uint32_t)func_8005BD94();break;
        case 2:func_8005BE1C();break;
        case 3:result=(uint32_t)func_8005BF08();break;
        case 4:result=(uint32_t)func_8005BCB0();break;
        case 5:result=func_8005BEDC();break;
        case 6:result=func_8005BEE8();break;
        case 7:result=func_80062A20(a[0],a[1]);break;
        case 8:result=(uint32_t)func_80063428(a[0]);break;
        case 9:func_80052FCC(a[0]);break;
        case 10:func_8004DD64((int32_t)a[0]);break;
        }
        hash=hit_camera_hash(NAM2_name_ranges,sizeof(NAM2_name_ranges)/sizeof(NAM2_name_ranges[0]));
        if (hash!=NAM2_name_cases[k].hash || result!=NAM2_name_cases[k].result) {
            fprintf(stderr,"name entry %u result %08X/%08X hash %016llX/%016llX\n",k,
                result,NAM2_name_cases[k].result,(unsigned long long)hash,(unsigned long long)NAM2_name_cases[k].hash);
            if (getenv("PE_NAM2_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-nam2-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(result==NAM2_name_cases[k].result,"name entry return differs from original");
        ASSERT(hash==NAM2_name_cases[k].hash,"name entry memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"name entry call graph executes natively");
    }
    PASS();
}
