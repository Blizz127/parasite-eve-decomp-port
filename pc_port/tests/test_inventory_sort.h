#include "retail_inventory_sort_cases.h"

static void test_DAY1_retail_inventory_sort(void)
{
    unsigned k,i,failures=0;
    TEST_RETAIL_DISC1("DAY1_retail_inventory_sort"); TEST_RETAIL_FIXUPS(RETAILFIX_inventory_sort);
    for (k=0;k<sizeof(DAY1_inventory_sort_cases)/sizeof(DAY1_inventory_sort_cases[0]);k++) {
        const uint32_t *args=DAY1_inventory_sort_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(DAY1_inventory_sort_common)/sizeof(DAY1_inventory_sort_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_inventory_sort_common[i][0],DAY1_inventory_sort_common[i][1]);
        for (i=DAY1_inventory_sort_cases[k].first;i<DAY1_inventory_sort_cases[k].end;i++)
            PE_StoreU32(0x80000000u+DAY1_inventory_sort_patches[i][0],DAY1_inventory_sort_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        D_8009D250=PE_LoadU32(0x8009D250u);g_pe_gte.h=256;
        switch (DAY1_inventory_sort_cases[k].entry) {
        case 0:func_80046DFC(a,args[1]);break;
        case 1:func_80046EAC(a);break;
        case 2:func_80047040(a);break;
        case 3:func_800471BC(a);break;
        case 4:result=(uint32_t)func_800471E4(a,args[1]);break;
        case 5:func_8004732C(a);break;
        case 6:result=(uint32_t)func_80047354(a,args[1]);break;
        case 7:func_800473E4(a,args[1]);break;
        case 8:func_800474A8(a);break;
        case 9:result=(uint32_t)func_800474D0(a,args[1]);break;
        case 10:func_80050280(a);break;
        case 11:func_80050308(a);break;
        case 12:result=(uint32_t)func_8005AFFC(a,args[1]);break;
        case 13:result=(uint32_t)func_8005B124(a,args[1]);break;
        case 14:func_8005B248();break;
        case 15:func_8005B3A4();break;
        case 16:func_8005B500(a,args[1]);break;
        case 17:func_8005B71C(a);break;
        case 18:func_8005B7D0(a,args[1]);break;
        case 19:func_800723A4(a,args[1],args[2],args[3]);break;
        case 20:func_800724F4(a,args[1],args[2]);break;
        case 21:result=(uint32_t)func_80043DA4(a,args[1]);break;
        case 22:func_80062FEC();break;
        case 23:result=(uint32_t)func_80063E0C(a,args[1]);break;
        }
        PE_StoreU32(0x8009D018u,D_8009D018);
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(DAY1_inventory_sort_ranges,sizeof(DAY1_inventory_sort_ranges)/sizeof(DAY1_inventory_sort_ranges[0]));
        if (hash!=DAY1_inventory_sort_cases[k].hash || result!=DAY1_inventory_sort_cases[k].result) {
            fprintf(stderr,"Inventory sort %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)DAY1_inventory_sort_cases[k].hash,
                result,DAY1_inventory_sort_cases[k].result);
            if (getenv("PE_DAY1_SORT_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-day1-sort-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        if (hash!=DAY1_inventory_sort_cases[k].hash || result!=DAY1_inventory_sort_cases[k].result ||
            g_stub_order_count || PE_Port_ShouldStop()) failures++;
    }
    ASSERT(!failures,"Inventory sort original comparisons or native call graphs failed");
    PASS();
}
