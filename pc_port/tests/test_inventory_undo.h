#include "retail_inventory_undo_cases.h"

static void test_INV12_retail_inventory_undo(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV12_retail_inventory_undo"); TEST_RETAIL_FIXUPS(RETAILFIX_inventory_undo);
    for (k=0;k<sizeof(INV12_inventory_undo_cases)/sizeof(INV12_inventory_undo_cases[0]);k++) {
        const uint32_t *args=INV12_inventory_undo_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(INV12_inventory_undo_common)/sizeof(INV12_inventory_undo_common[0]);i++)
            PE_StoreU32(0x80000000u+INV12_inventory_undo_common[i][0],INV12_inventory_undo_common[i][1]);
        for (i=INV12_inventory_undo_cases[k].first;i<INV12_inventory_undo_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV12_inventory_undo_patches[i][0],INV12_inventory_undo_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        switch (INV12_inventory_undo_cases[k].entry) {
        case 0:result=(uint32_t)func_800533D4(a);break;
        case 1:case 2:case 3:func_8005112C();break;
        case 4:func_80026CF0_attack_cut();break;
        }
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV12_inventory_undo_ranges,sizeof(INV12_inventory_undo_ranges)/sizeof(INV12_inventory_undo_ranges[0]));
        if (hash!=INV12_inventory_undo_cases[k].hash || result!=INV12_inventory_undo_cases[k].result) {
            fprintf(stderr,"inventory undo %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV12_inventory_undo_cases[k].hash,
                result,INV12_inventory_undo_cases[k].result);
            if (getenv("PE_INV12_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv12-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV12_inventory_undo_cases[k].hash,"inventory undo memory differs from original");
        ASSERT(result==INV12_inventory_undo_cases[k].result,"inventory undo result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"inventory undo call graph executes natively");
    }
    PASS();
}
