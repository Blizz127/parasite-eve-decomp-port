#include "retail_inventory_items_cases.h"

static void test_INV6_retail_inventory_items(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV6_retail_inventory_items"); TEST_RETAIL_FIXUPS(RETAILFIX_inventory_items);
    for (k=0;k<sizeof(INV6_inventory_items_cases)/sizeof(INV6_inventory_items_cases[0]);k++) {
        const uint32_t *args=INV6_inventory_items_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(INV6_inventory_items_common)/sizeof(INV6_inventory_items_common[0]);i++)
            PE_StoreU32(0x80000000u+INV6_inventory_items_common[i][0],INV6_inventory_items_common[i][1]);
        for (i=INV6_inventory_items_cases[k].first;i<INV6_inventory_items_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV6_inventory_items_patches[i][0],INV6_inventory_items_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        for (i=0;i<INV6_inventory_items_cases[k].frame;i++) PE_GPU_VBlankStep();
        switch (INV6_inventory_items_cases[k].entry) {
        case 0:func_80055760();break;
        case 1:func_80050260();break;
        case 2:result=(uint32_t)func_80055FE0((int32_t)a);break;
        case 3:result=(uint32_t)func_80057C54(a,(int32_t)args[1],args[2],(int32_t)args[3]);break;
        case 4:result=(uint32_t)func_8005401C();break;
        case 5:result=(uint32_t)func_80054240((int32_t)a);break;
        case 6:func_8005FA3C((int32_t)a);break;
        case 7:func_80063158(a,(int32_t)args[1],(int32_t)args[2]);break;
        case 8:func_80062F1C(a);break;
        case 9:func_80064C80();break;
        case 10:func_80050804(a);break;
        case 11:func_8004F8D0(a);break;
        case 12:func_800447F0(a);break;
        case 13:func_80044174(a);break;
        case 14:func_80064EB4(a);break;
        case 15:func_80062FEC();break;
        case 16:result=(uint32_t)func_80063E0C(a,args[1]);break;
        case 17:result=(uint32_t)func_8004E970();break;
        }
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV6_inventory_items_ranges,sizeof(INV6_inventory_items_ranges)/sizeof(INV6_inventory_items_ranges[0]));
        if (hash!=INV6_inventory_items_cases[k].hash || result!=INV6_inventory_items_cases[k].result) {
            fprintf(stderr,"inventory items %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV6_inventory_items_cases[k].hash,
                result,INV6_inventory_items_cases[k].result);
            if (getenv("PE_INV6_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv6-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV6_inventory_items_cases[k].hash,"inventory items memory differs from original");
        ASSERT(result==INV6_inventory_items_cases[k].result,"inventory items result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"inventory items call graph executes natively");
        ASSERT(PE_GPU_VSyncQuery()==INV6_inventory_items_cases[k].frame,"drawing must not advance VBlank time");
    }
    PASS();
}
