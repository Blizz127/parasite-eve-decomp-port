#include "retail_inventory_help_cases.h"

static void test_INV5_retail_inventory_help(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV5_retail_inventory_help"); TEST_RETAIL_FIXUPS(RETAILFIX_inventory_help);
    for (k=0;k<sizeof(INV5_inventory_help_cases)/sizeof(INV5_inventory_help_cases[0]);k++) {
        uint32_t a=INV5_inventory_help_cases[k].args[0],result=0u;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(INV5_inventory_help_common)/sizeof(INV5_inventory_help_common[0]);i++)
            PE_StoreU32(0x80000000u+INV5_inventory_help_common[i][0],INV5_inventory_help_common[i][1]);
        for (i=INV5_inventory_help_cases[k].first;i<INV5_inventory_help_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV5_inventory_help_patches[i][0],INV5_inventory_help_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        for (i=0;i<INV5_inventory_help_cases[k].frame;i++) PE_GPU_VBlankStep();
        switch (INV5_inventory_help_cases[k].entry) {
        case 0:result=func_8005DCEC(a);break;
        case 1:result=(uint32_t)func_8005415C((int32_t)a);break;
        case 2:result=(uint32_t)func_80054288();break;
        case 3:result=(uint32_t)func_800556E8((int32_t)a);break;
        case 4:result=(uint32_t)func_80058E08((int32_t)a);break;
        case 5:result=(uint32_t)func_80057ED8((int32_t)a);break;
        case 6:result=func_80058BBC((int32_t)a);break;
        case 7:result=(uint32_t)func_80059F08(a);break;
        case 8:func_80055610();break;
        case 9:func_8004C608(a);break;
        case 10:func_80062FEC();break;
        }
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV5_inventory_help_ranges,sizeof(INV5_inventory_help_ranges)/sizeof(INV5_inventory_help_ranges[0]));
        if (hash!=INV5_inventory_help_cases[k].hash || result!=INV5_inventory_help_cases[k].result) {
            fprintf(stderr,"inventory help %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV5_inventory_help_cases[k].hash,
                result,INV5_inventory_help_cases[k].result);
            if (getenv("PE_INV5_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv5-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV5_inventory_help_cases[k].hash,"inventory help memory differs from original");
        ASSERT(result==INV5_inventory_help_cases[k].result,"inventory help result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"inventory help call graph executes natively");
        ASSERT(PE_GPU_VSyncQuery()==INV5_inventory_help_cases[k].frame,"drawing must not advance VBlank time");
    }
    PASS();
}
