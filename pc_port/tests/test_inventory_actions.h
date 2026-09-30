#include "retail_inventory_actions_cases.h"

static void test_INV7_retail_inventory_actions(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV7_retail_inventory_actions"); TEST_RETAIL_FIXUPS(RETAILFIX_inventory_actions);
    for (k=0;k<sizeof(INV7_inventory_actions_cases)/sizeof(INV7_inventory_actions_cases[0]);k++) {
        const uint32_t *args=INV7_inventory_actions_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(INV7_inventory_actions_common)/sizeof(INV7_inventory_actions_common[0]);i++)
            PE_StoreU32(0x80000000u+INV7_inventory_actions_common[i][0],INV7_inventory_actions_common[i][1]);
        for (i=INV7_inventory_actions_cases[k].first;i<INV7_inventory_actions_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV7_inventory_actions_patches[i][0],INV7_inventory_actions_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        switch (INV7_inventory_actions_cases[k].entry) {
        case 0:result=(uint32_t)func_800210D4();break;
        case 1:result=(uint32_t)func_80052558();break;
        case 2:func_80055724();break;
        case 3:func_80055FB4((int32_t)a);break;
        case 4:result=(uint32_t)func_800562A4((int32_t)a);break;
        case 5:func_8005600C(a,args[1]);break;
        case 6:func_80056C40(a,(int32_t)args[1],args[2],(int32_t)args[3]);break;
        case 7:result=(uint32_t)func_80057D18((int32_t)a);break;
        case 8:result=(uint32_t)func_8005833C((int32_t)a);break;
        case 9:result=(uint32_t)func_80057654((int32_t)a);break;
        case 10:func_80044924(a,args[1],(int32_t)args[2]);break;
        case 11:func_800451D0(a);break;
        case 12:result=(uint32_t)func_80044444(a,args[1]);break;
        }
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV7_inventory_actions_ranges,sizeof(INV7_inventory_actions_ranges)/sizeof(INV7_inventory_actions_ranges[0]));
        if (hash!=INV7_inventory_actions_cases[k].hash || result!=INV7_inventory_actions_cases[k].result) {
            fprintf(stderr,"inventory actions %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV7_inventory_actions_cases[k].hash,
                result,INV7_inventory_actions_cases[k].result);
            if (getenv("PE_INV7_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv7-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV7_inventory_actions_cases[k].hash,"inventory actions memory differs from original");
        ASSERT(result==INV7_inventory_actions_cases[k].result,"inventory actions result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"inventory actions call graph executes natively");
    }
    PASS();
}
