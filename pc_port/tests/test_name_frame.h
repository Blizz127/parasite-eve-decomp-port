#include "retail_name_frame_cases.h"

static void test_NAM9_retail_name_frame(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("NAM9_retail_name_frame"); TEST_RETAIL_FIXUPS(RETAILFIX_name_frame);
    for (k=0;k<sizeof(NAM9_name_frame_cases)/sizeof(NAM9_name_frame_cases[0]);k++) {
        const uint32_t *a=NAM9_name_frame_cases[k].args;
        uint64_t hash;uint32_t result=0u;
        ResetTestState();
        for (i=0;i<sizeof(NAM9_name_frame_common)/sizeof(NAM9_name_frame_common[0]);i++)
            PE_StoreU32(0x80000000u+NAM9_name_frame_common[i][0],NAM9_name_frame_common[i][1]);
        for (i=NAM9_name_frame_cases[k].first;i<NAM9_name_frame_cases[k].end;i++)
            PE_StoreU32(0x80000000u+NAM9_name_frame_patches[i][0],NAM9_name_frame_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D050=PE_LoadU32(0x8009D050u);
        D_800B0E50=PE_LoadU32(0x800B0E50u);D_800B0E54=PE_LoadU32(0x800B0E54u);
        switch (NAM9_name_frame_cases[k].entry) {
        case 0:func_8004DCA4(a[0]);break;
        case 1:result=(uint32_t)func_80016F10(a[0]);break;
        case 2:func_80046334();break;
        case 3:func_8004F464();break;
        case 4:func_8005C488();break;
        case 5:func_80042B6C();break;
        case 6:func_800339A0(a[0]);break;
        case 7:func_80042D40();break;
        case 8:func_80042F44();break;
        case 9:func_8005E788((int32_t)a[0]);break;
        case 10:result=(uint32_t)func_8005C498(a[0]);break;
        case 11:PE_FieldMenuFrame();break;
        case 12:
            D_800B0E38=PE_LoadU32(0x800B0E38u);D_800B0E3C=PE_LoadU32(0x800B0E3Cu);
            func_8005E588();func_8005E6F0();break;
        }
        hash=hit_camera_hash(NAM9_name_frame_ranges,sizeof(NAM9_name_frame_ranges)/sizeof(NAM9_name_frame_ranges[0]));
        if (hash!=NAM9_name_frame_cases[k].hash || result!=NAM9_name_frame_cases[k].result) {
            fprintf(stderr,"name frame %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)NAM9_name_frame_cases[k].hash,
                result,NAM9_name_frame_cases[k].result);
            if (getenv("PE_NAM9_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-nam9-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==NAM9_name_frame_cases[k].hash,"name frame memory differs from original");
        ASSERT(result==NAM9_name_frame_cases[k].result,"name frame result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"name frame call graph executes natively");
    }
    PASS();
}
