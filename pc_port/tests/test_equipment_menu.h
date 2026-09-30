#include "retail_equipment_menu_cases.h"

static void test_INV21_retail_equipment_menu(void)
{
    unsigned k,i,failures=0;
    TEST_RETAIL_DISC1("INV21_retail_equipment_menu"); TEST_RETAIL_FIXUPS(RETAILFIX_equipment_menu);
    for (k=0;k<sizeof(INV21_equipment_menu_cases)/sizeof(INV21_equipment_menu_cases[0]);k++) {
        const uint32_t *args=INV21_equipment_menu_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(INV21_equipment_menu_common)/sizeof(INV21_equipment_menu_common[0]);i++)
            PE_StoreU32(0x80000000u+INV21_equipment_menu_common[i][0],INV21_equipment_menu_common[i][1]);
        for (i=INV21_equipment_menu_cases[k].first;i<INV21_equipment_menu_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV21_equipment_menu_patches[i][0],INV21_equipment_menu_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        D_8009D250=PE_LoadU32(0x8009D250u);g_pe_gte.h=256;
        switch (INV21_equipment_menu_cases[k].entry) {
        case 0:func_800542A0(a);break;
        case 1:func_800543CC(a,(int32_t)args[1]);break;
        case 2:func_80059EC8(a,(int32_t)args[1]);break;
        case 3:func_80064B74(a,(int32_t)args[1]);break;
        case 4:func_80064C20(a);break;
        case 5:func_80045EE4(a);break;
        case 6:func_8004542C(a);break;
        case 7:func_80046378(a,args[1]);break;
        case 8:func_8004F9A0(a);break;
        case 9:func_80050AD8((int32_t)a);break;
        case 10:func_8005E988((int32_t)a,(int32_t)args[1]);break;
        case 11:func_8004551C(a);break;
        case 12:func_80045670(a,args[1]);break;
        case 13:func_80045A98(a);break;
        }
        PE_StoreU32(0x8009D018u,D_8009D018);
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV21_equipment_menu_ranges,sizeof(INV21_equipment_menu_ranges)/sizeof(INV21_equipment_menu_ranges[0]));
        if (hash!=INV21_equipment_menu_cases[k].hash || result!=INV21_equipment_menu_cases[k].result) {
            fprintf(stderr,"Equipment menu %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV21_equipment_menu_cases[k].hash,
                result,INV21_equipment_menu_cases[k].result);
            if (getenv("PE_INV21_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv21-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        if (hash!=INV21_equipment_menu_cases[k].hash || result!=INV21_equipment_menu_cases[k].result ||
            g_stub_order_count || PE_Port_ShouldStop()) failures++;
    }
    ASSERT(!failures,"Equipment menu original comparisons or native call graphs failed");
    PASS();
}
