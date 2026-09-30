#include "retail_item_execution_cases.h"

static void test_INV14_retail_item_execution(void)
{
    unsigned k,i;
    TEST("INV14_retail_item_execution");
    for (k=0;k<sizeof(INV14_item_execution_cases)/sizeof(INV14_item_execution_cases[0]);k++) {
        uint32_t result=0;
        uint64_t hash;
        ResetTestState();func_80071A64(INV14_item_execution_cases[k].seed);
        for (i=0;i<sizeof(INV14_item_execution_common)/sizeof(INV14_item_execution_common[0]);i++)
            PE_StoreU32(0x80000000u+INV14_item_execution_common[i][0],INV14_item_execution_common[i][1]);
        for (i=INV14_item_execution_cases[k].first;i<INV14_item_execution_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV14_item_execution_patches[i][0],INV14_item_execution_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        switch (INV14_item_execution_cases[k].entry) {
        case 0:func_80022210();break;
        case 1:func_80021DE0();break;
        case 2:result=(uint32_t)(int32_t)func_800255E4();break;
        }
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV14_item_execution_ranges,sizeof(INV14_item_execution_ranges)/sizeof(INV14_item_execution_ranges[0]));
        if (hash!=INV14_item_execution_cases[k].hash || result!=INV14_item_execution_cases[k].result) {
            fprintf(stderr,"item execution %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV14_item_execution_cases[k].hash,
                result,INV14_item_execution_cases[k].result);
            if (getenv("PE_INV14_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv14-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV14_item_execution_cases[k].hash,"item execution memory differs from original");
        ASSERT(result==INV14_item_execution_cases[k].result,"item execution result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"item execution call graph executes natively");
    }
    PASS();
}
