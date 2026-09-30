#include "retail_equipment_selection_cases.h"

static void test_INV22_retail_equipment_selection(void)
{
    unsigned k,i,failures=0;
    TEST_RETAIL_DISC1("INV22_retail_equipment_selection"); TEST_RETAIL_FIXUPS(RETAILFIX_equipment_selection);
    for (k=0;k<sizeof(INV22_equipment_selection_cases)/sizeof(INV22_equipment_selection_cases[0]);k++) {
        const uint32_t *args=INV22_equipment_selection_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(INV22_equipment_selection_common)/sizeof(INV22_equipment_selection_common[0]);i++)
            PE_StoreU32(0x80000000u+INV22_equipment_selection_common[i][0],INV22_equipment_selection_common[i][1]);
        for (i=INV22_equipment_selection_cases[k].first;i<INV22_equipment_selection_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV22_equipment_selection_patches[i][0],INV22_equipment_selection_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        D_8009D250=PE_LoadU32(0x8009D250u);g_pe_gte.h=256;
        switch (INV22_equipment_selection_cases[k].entry) {
        case 0:func_80059534((int32_t)a);break;
        case 1:result=(uint32_t)func_8005968C((int32_t)a);break;
        case 2:result=(uint32_t)func_80054520(a);break;
        case 3:func_800509E0(a);break;
        case 4:func_8004FA10(a);break;
        case 5:func_8004FB48(a);break;
        case 6:func_800430A0((int32_t)a);break;
        case 7:func_8004F978(a);break;
        case 8:func_8004FFD0(a);break;
        case 9:result=(uint32_t)func_80045D0C(a,args[1]);break;
        case 10:result=(uint32_t)func_8004620C(a,args[1]);break;
        case 11:result=(uint32_t)func_800466C0(a,args[1]);break;
        case 12:func_80046574(a,args[1]);break;
        case 13:result=(uint32_t)func_80043DA4(a,args[1]);break;
        case 14:func_80062FEC();break;
        case 15:func_8005E30C();break;
        case 16:result=(uint32_t)func_80044E98(a,args[1]);break;
        }
        PE_StoreU32(0x8009D018u,D_8009D018);
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV22_equipment_selection_ranges,sizeof(INV22_equipment_selection_ranges)/sizeof(INV22_equipment_selection_ranges[0]));
        if (hash!=INV22_equipment_selection_cases[k].hash || result!=INV22_equipment_selection_cases[k].result) {
            fprintf(stderr,"Equipment menu %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV22_equipment_selection_cases[k].hash,
                result,INV22_equipment_selection_cases[k].result);
            if (getenv("PE_INV22_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv22-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        if (hash!=INV22_equipment_selection_cases[k].hash || result!=INV22_equipment_selection_cases[k].result ||
            g_stub_order_count || PE_Port_ShouldStop()) failures++;
    }
    ASSERT(!failures,"Equipment menu original comparisons or native call graphs failed");
    PASS();
}
