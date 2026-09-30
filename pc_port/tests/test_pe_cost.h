#include "retail_pe_cost_cases.h"

static void test_INV16_retail_pe_cost(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV16_retail_pe_cost"); TEST_RETAIL_FIXUPS(RETAILFIX_pe_cost);
    for (k=0;k<sizeof(INV16_pe_cost_cases)/sizeof(INV16_pe_cost_cases[0]);k++) {
        const uint32_t *args=INV16_pe_cost_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(INV16_pe_cost_common)/sizeof(INV16_pe_cost_common[0]);i++)
            PE_StoreU32(0x80000000u+INV16_pe_cost_common[i][0],INV16_pe_cost_common[i][1]);
        for (i=INV16_pe_cost_cases[k].first;i<INV16_pe_cost_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV16_pe_cost_patches[i][0],INV16_pe_cost_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        D_8009D250=PE_LoadU32(0x8009D250u);g_pe_gte.h=256;
        switch (INV16_pe_cost_cases[k].entry) {
        case 0:result=(uint32_t)func_800515F8(a);break;
        case 1:result=(uint32_t)func_800579D4((int32_t)a,(int32_t)args[1]);break;
        case 2:result=(uint32_t)func_8004324C((int32_t)a);break;
        case 3:result=(uint32_t)func_800524D0();break;
        case 4:result=func_8005DC10();break;
        }
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV16_pe_cost_ranges,sizeof(INV16_pe_cost_ranges)/sizeof(INV16_pe_cost_ranges[0]));
        if (hash!=INV16_pe_cost_cases[k].hash || result!=INV16_pe_cost_cases[k].result) {
            fprintf(stderr,"PE cost %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV16_pe_cost_cases[k].hash,
                result,INV16_pe_cost_cases[k].result);
            if (getenv("PE_INV16_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv16-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV16_pe_cost_cases[k].hash,"PE cost memory differs from original");
        ASSERT(result==INV16_pe_cost_cases[k].result,"PE cost result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"PE cost call graph executes natively");
    }
    PASS();
}
