#include "retail_inventory_dialogs_cases.h"

static void test_INV8_retail_inventory_dialogs(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV8_retail_inventory_dialogs"); TEST_RETAIL_FIXUPS(RETAILFIX_inventory_dialogs);
    for (k=0;k<sizeof(INV8_inventory_dialogs_cases)/sizeof(INV8_inventory_dialogs_cases[0]);k++) {
        const uint32_t *args=INV8_inventory_dialogs_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(INV8_inventory_dialogs_common)/sizeof(INV8_inventory_dialogs_common[0]);i++)
            PE_StoreU32(0x80000000u+INV8_inventory_dialogs_common[i][0],INV8_inventory_dialogs_common[i][1]);
        for (i=INV8_inventory_dialogs_cases[k].first;i<INV8_inventory_dialogs_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV8_inventory_dialogs_patches[i][0],INV8_inventory_dialogs_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        for (i=0;i<INV8_inventory_dialogs_cases[k].frame;i++) PE_GPU_VBlankStep();
        switch (INV8_inventory_dialogs_cases[k].entry) {
        case 0:func_8005F354(a,(int32_t)args[1]);break;
        case 1:func_8005F594(a);break;
        case 2:func_80064C30(a);break;
        case 3:func_80064C54(a);break;
        case 4:func_80062A7C(a);break;
        case 5:result=func_80053068((int32_t)a);break;
        case 6:func_80052BCC(a,args[1]);break;
        case 7:func_80052C08(a,args[1]);break;
        case 8:func_80050878(a);break;
        case 9:func_8004F910(a);break;
        case 10:func_800509A8(a);break;
        case 11:func_8004F950(a);break;
        case 12:func_8004FFA8(a);break;
        case 13:func_8004CDD4(a);break;
        case 14:func_8004CC50(a,args[1]);break;
        case 15:result=(uint32_t)func_8004D030(a,args[1]);break;
        case 16:func_80044E14(a);break;
        case 17:func_80044F8C();break;
        case 18:func_80062CE4();break;
        case 19:result=(uint32_t)func_80058C4C(a);break;
        case 20:func_80045110(a,args[1]);break;
        case 21:result=(uint32_t)func_80044E98(a,args[1]);break;
        case 22:case 23:case 24:func_80062FEC();break;
        case 25:func_800631AC(a);break;
        case 26:func_8004D024(a);break;
        case 27:func_8005E30C();break;
        }
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV8_inventory_dialogs_ranges,sizeof(INV8_inventory_dialogs_ranges)/sizeof(INV8_inventory_dialogs_ranges[0]));
        if (hash!=INV8_inventory_dialogs_cases[k].hash || result!=INV8_inventory_dialogs_cases[k].result) {
            fprintf(stderr,"inventory dialogs %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV8_inventory_dialogs_cases[k].hash,
                result,INV8_inventory_dialogs_cases[k].result);
            if (getenv("PE_INV8_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv8-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV8_inventory_dialogs_cases[k].hash,"inventory dialogs memory differs from original");
        ASSERT(result==INV8_inventory_dialogs_cases[k].result,"inventory dialogs result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"inventory dialogs call graph executes natively");
        ASSERT(PE_GPU_VSyncQuery()==INV8_inventory_dialogs_cases[k].frame,"drawing must not advance VBlank time");
    }
    PASS();
}
