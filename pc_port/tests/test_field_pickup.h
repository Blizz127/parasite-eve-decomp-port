#include "retail_field_pickup_cases.h"

static void test_SEW17_field_pickup(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("SEW17_field_pickup"); TEST_RETAIL_FIXUPS(RETAILFIX_field_pickup);
    for (k=0;k<sizeof(SEW17_cases)/sizeof(SEW17_cases[0]);k++) {
        const uint32_t *args=SEW17_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(SEW17_common)/sizeof(SEW17_common[0]);i++)
            PE_StoreU32(0x80000000u+SEW17_common[i][0],SEW17_common[i][1]);
        for (i=SEW17_cases[k].first;i<SEW17_cases[k].end;i++)
            PE_StoreU32(0x80000000u+SEW17_patches[i][0],SEW17_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        switch (SEW17_cases[k].entry) {
        case 0:result=func_800532B4(a);break;
        case 1:result=(uint32_t)func_800194B0(a);break;
        case 2:func_8004F490(a);break;
        case 3:result=(uint32_t)func_80015BAC(a);break;
        case 4:func_8004F644();break;
        case 5:result=(uint32_t)func_8004F730(a,args[1]);break;
        case 6:func_8004F798();break;
        case 7:func_8004F7D8();break;
        case 8:func_80062FEC();break;
        case 9:func_8005E30C();break;
        case 10:result=(uint32_t)func_80019540(a);break;
        case 11:result=(uint32_t)func_8004C34C(a);break;
        case 12:func_80055E14();break;
        }
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(SEW17_ranges,sizeof(SEW17_ranges)/sizeof(SEW17_ranges[0]));
        if (hash!=SEW17_cases[k].hash || result!=SEW17_cases[k].result) {
            fprintf(stderr,"field pickup %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)SEW17_cases[k].hash,
                result,SEW17_cases[k].result);
            if (getenv("PE_SEW17_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"local/live/pickup-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==SEW17_cases[k].hash,"field pickup memory differs from original");
        ASSERT(result==SEW17_cases[k].result,"field pickup result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"field pickup call graph executes natively");
    }
    PASS();
}
