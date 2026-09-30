#include "retail_battle_items_cases.h"

static void test_INV13_retail_battle_items(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV13_retail_battle_items"); TEST_RETAIL_FIXUPS(RETAILFIX_battle_items);
    for (k=0;k<sizeof(INV13_battle_items_cases)/sizeof(INV13_battle_items_cases[0]);k++) {
        const uint32_t *args=INV13_battle_items_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(INV13_battle_items_common)/sizeof(INV13_battle_items_common[0]);i++)
            PE_StoreU32(0x80000000u+INV13_battle_items_common[i][0],INV13_battle_items_common[i][1]);
        for (i=INV13_battle_items_cases[k].first;i<INV13_battle_items_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV13_battle_items_patches[i][0],INV13_battle_items_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        D_8009D250=PE_LoadU32(0x8009D250u);g_pe_gte.h=256;
        switch (INV13_battle_items_cases[k].entry) {
        case 0:result=(uint32_t)func_80026824((int32_t)a);break;
        case 1:result=(uint32_t)(int32_t)func_80025EE8_attack_cut();break;
        case 2:result=func_800299CC_ready_input();break;
        }
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV13_battle_items_ranges,sizeof(INV13_battle_items_ranges)/sizeof(INV13_battle_items_ranges[0]));
        if (hash!=INV13_battle_items_cases[k].hash || result!=INV13_battle_items_cases[k].result) {
            fprintf(stderr,"battle items %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV13_battle_items_cases[k].hash,
                result,INV13_battle_items_cases[k].result);
            if (getenv("PE_INV13_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv13-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV13_battle_items_cases[k].hash,"battle items memory differs from original");
        ASSERT(result==INV13_battle_items_cases[k].result,"battle items result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"battle items call graph executes natively");
    }
    PASS();
}
