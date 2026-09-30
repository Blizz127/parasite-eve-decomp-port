#include "retail_inventory_entry_cases.h"

static void test_INV11_retail_inventory_entry(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV11_retail_inventory_entry"); TEST_RETAIL_FIXUPS(RETAILFIX_inventory_entry);
    for (k=0;k<sizeof(INV11_inventory_entry_cases)/sizeof(INV11_inventory_entry_cases[0]);k++) {
        const uint32_t *args=INV11_inventory_entry_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(INV11_inventory_entry_common)/sizeof(INV11_inventory_entry_common[0]);i++)
            PE_StoreU32(0x80000000u+INV11_inventory_entry_common[i][0],INV11_inventory_entry_common[i][1]);
        for (i=INV11_inventory_entry_cases[k].first;i<INV11_inventory_entry_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV11_inventory_entry_patches[i][0],INV11_inventory_entry_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        switch (INV11_inventory_entry_cases[k].entry) {
        case 0:result=(uint32_t)func_80043DA4(a,args[1]);break;
        case 1:func_8005E30C();break;
        case 2:func_80062FEC();break;
        case 3:PE_FieldMenuFrame();break;
        }
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV11_inventory_entry_ranges,sizeof(INV11_inventory_entry_ranges)/sizeof(INV11_inventory_entry_ranges[0]));
        if (hash!=INV11_inventory_entry_cases[k].hash || result!=INV11_inventory_entry_cases[k].result) {
            fprintf(stderr,"inventory entry %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV11_inventory_entry_cases[k].hash,
                result,INV11_inventory_entry_cases[k].result);
            if (getenv("PE_INV11_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv11-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV11_inventory_entry_cases[k].hash,"inventory entry memory differs from original");
        ASSERT(result==INV11_inventory_entry_cases[k].result,"inventory entry result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"inventory entry call graph executes natively");
    }
    PASS();
}
