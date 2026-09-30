#include "retail_inventory_ammo_cases.h"

static void test_INV9_retail_inventory_ammo(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV9_retail_inventory_ammo"); TEST_RETAIL_FIXUPS(RETAILFIX_inventory_ammo);
    for (k=0;k<sizeof(INV9_inventory_ammo_cases)/sizeof(INV9_inventory_ammo_cases[0]);k++) {
        const uint32_t *args=INV9_inventory_ammo_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(INV9_inventory_ammo_common)/sizeof(INV9_inventory_ammo_common[0]);i++)
            PE_StoreU32(0x80000000u+INV9_inventory_ammo_common[i][0],INV9_inventory_ammo_common[i][1]);
        for (i=INV9_inventory_ammo_cases[k].first;i<INV9_inventory_ammo_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV9_inventory_ammo_patches[i][0],INV9_inventory_ammo_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        for (i=0;i<INV9_inventory_ammo_cases[k].frame;i++) PE_GPU_VBlankStep();
        switch (INV9_inventory_ammo_cases[k].entry) {
        case 0:result=(uint32_t)func_8005E120();break;
        case 1:result=func_80051098();break;
        case 2:result=(uint32_t)func_80056B24((int32_t)a);break;
        case 3:func_80057094();break;
        case 4:func_80056FB8();break;
        case 5:func_800453E8(a);break;
        case 6:result=(uint32_t)func_800452C0(a,args[1]);break;
        case 7:func_80044274((int32_t)a);break;
        case 8:func_80062FEC();break;
        case 9:func_8005E30C();break;
        case 10:func_800512AC((int32_t)a,args[1]);break;
        }
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV9_inventory_ammo_ranges,sizeof(INV9_inventory_ammo_ranges)/sizeof(INV9_inventory_ammo_ranges[0]));
        if (hash!=INV9_inventory_ammo_cases[k].hash || result!=INV9_inventory_ammo_cases[k].result) {
            fprintf(stderr,"inventory ammo %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV9_inventory_ammo_cases[k].hash,
                result,INV9_inventory_ammo_cases[k].result);
            if (getenv("PE_INV9_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv9-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV9_inventory_ammo_cases[k].hash,"inventory ammo memory differs from original");
        ASSERT(result==INV9_inventory_ammo_cases[k].result,"inventory ammo result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"inventory ammo call graph executes natively");
        ASSERT(PE_GPU_VSyncQuery()==INV9_inventory_ammo_cases[k].frame,"drawing must not advance VBlank time");
    }
    PASS();
}
