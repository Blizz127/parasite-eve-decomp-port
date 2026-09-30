#include "retail_inventory_use_cases.h"

static void test_INV10_retail_inventory_use(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV10_retail_inventory_use"); TEST_RETAIL_FIXUPS(RETAILFIX_inventory_use);
    for (k=0;k<sizeof(INV10_inventory_use_cases)/sizeof(INV10_inventory_use_cases[0]);k++) {
        const uint32_t *args=INV10_inventory_use_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(INV10_inventory_use_common)/sizeof(INV10_inventory_use_common[0]);i++)
            PE_StoreU32(0x80000000u+INV10_inventory_use_common[i][0],INV10_inventory_use_common[i][1]);
        for (i=INV10_inventory_use_cases[k].first;i<INV10_inventory_use_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV10_inventory_use_patches[i][0],INV10_inventory_use_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        switch (INV10_inventory_use_cases[k].entry) {
        case 0:func_800516B4((int32_t)a);break;
        case 1:func_80057834((int32_t)a);break;
        case 2:result=(uint32_t)func_80044B0C(a,args[1]);break;
        case 3:func_8005E30C();break;
        case 4:func_80062FEC();break;
        }
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV10_inventory_use_ranges,sizeof(INV10_inventory_use_ranges)/sizeof(INV10_inventory_use_ranges[0]));
        if (hash!=INV10_inventory_use_cases[k].hash || result!=INV10_inventory_use_cases[k].result) {
            fprintf(stderr,"inventory use %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV10_inventory_use_cases[k].hash,
                result,INV10_inventory_use_cases[k].result);
            if (getenv("PE_INV10_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv10-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV10_inventory_use_cases[k].hash,"inventory use memory differs from original");
        ASSERT(result==INV10_inventory_use_cases[k].result,"inventory use result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"inventory use call graph executes natively");
    }
    PASS();
}
