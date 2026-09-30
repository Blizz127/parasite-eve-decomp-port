#include "retail_recovery_item_cases.h"

static void test_ATK33_retail_recovery_items(void)
{
    unsigned k,i;
    TEST("ATK33_retail_recovery_items");
    for (k=0;k<sizeof(ATK33_item_cases)/sizeof(ATK33_item_cases[0]);k++) {
        const uint32_t *a=ATK33_item_cases[k].args;
        uint32_t result=0;uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(ATK33_item_common)/sizeof(ATK33_item_common[0]);i++)
            PE_StoreU32(0x80000000u+ATK33_item_common[i][0],ATK33_item_common[i][1]);
        for (i=ATK33_item_cases[k].first;i<ATK33_item_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ATK33_item_patches[i][0],ATK33_item_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D050=PE_LoadU32(0x8009D050u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        func_80071A64(1u);
        switch (ATK33_item_cases[k].entry) {
        case 0:func_80023E14((int32_t)a[0]);break;
        case 1:func_800523F8(a[0],a[1]);break;
        case 2:func_8003335C(a[0],a[1],a[2]);break;
        case 3:result=(uint32_t)func_8005409C((int32_t)a[0]);break;
        case 4:result=(uint32_t)func_80057D30((int32_t)a[0]);break;
        case 5:result=(uint32_t)func_8005485C();break;
        case 6:func_800553A4(a[0],a[1]);break;
        case 7:result=(uint32_t)func_80059A40(a[0]);break;
        case 8:result=(uint32_t)func_80054E4C((int32_t)a[0]);break;
        case 9:func_80054CF8();break;
        case 10:func_8001F9C4();break;
        case 11:func_80020288(a[0]);break;
        }
        hash=hit_camera_hash(ATK33_item_ranges,sizeof(ATK33_item_ranges)/sizeof(ATK33_item_ranges[0]));
        if (hash!=ATK33_item_cases[k].hash || result!=ATK33_item_cases[k].result) {
            fprintf(stderr,"recovery item %u result %08X/%08X hash %016llX/%016llX\n",k,result,
                ATK33_item_cases[k].result,(unsigned long long)hash,(unsigned long long)ATK33_item_cases[k].hash);
            if (getenv("PE_ATK33_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk33-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(result==ATK33_item_cases[k].result,"recovery item return differs from original");
        ASSERT(hash==ATK33_item_cases[k].hash,"recovery item memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"recovery item call graph executes natively");
    }
    PASS();
}
