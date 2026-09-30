#include "retail_inventory_menu_cases.h"

static void test_INV2_retail_inventory_menu(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV2_retail_inventory_menu"); TEST_RETAIL_FIXUPS(RETAILFIX_inventory_menu);
    for (k=0;k<sizeof(INV2_inventory_menu_cases)/sizeof(INV2_inventory_menu_cases[0]);k++) {
        uint64_t hash;uint32_t result=0u;
        ResetTestState();
        for (i=0;i<sizeof(INV2_inventory_menu_common)/sizeof(INV2_inventory_menu_common[0]);i++)
            PE_StoreU32(0x80000000u+INV2_inventory_menu_common[i][0],INV2_inventory_menu_common[i][1]);
        for (i=INV2_inventory_menu_cases[k].first;i<INV2_inventory_menu_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV2_inventory_menu_patches[i][0],INV2_inventory_menu_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D050=PE_LoadU32(0x8009D050u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        switch (INV2_inventory_menu_cases[k].entry) {
        case 0:func_800438EC();break;
        case 1:func_800439D8();break;
        case 2:func_8004C594();break;
        case 3:func_8005C174((int32_t)INV2_inventory_menu_cases[k].args[0]);break;
        case 4:result=func_80033A20();break;
        case 5:result=(uint32_t)func_8005B89C();break;
        case 6:result=(uint32_t)func_8005257C();break;
        }
        hash=hit_camera_hash(INV2_inventory_menu_ranges,sizeof(INV2_inventory_menu_ranges)/sizeof(INV2_inventory_menu_ranges[0]));
        if (hash!=INV2_inventory_menu_cases[k].hash || result!=INV2_inventory_menu_cases[k].result) {
            fprintf(stderr,"inventory menu %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV2_inventory_menu_cases[k].hash,
                result,INV2_inventory_menu_cases[k].result);
            if (getenv("PE_INV2_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv2-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV2_inventory_menu_cases[k].hash,"inventory menu memory differs from original");
        ASSERT(result==INV2_inventory_menu_cases[k].result,"inventory menu result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"inventory menu construction executes natively");
    }
    PASS();
}
