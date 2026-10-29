#include "retail_effect_tick_cases.h"

static void test_effect_nested_callback_stop(void)
{
    const pe_addr_t slot=0x80150000u,body=slot+12u,desc=0x80154000u;
    const pe_addr_t rec=body+32u,data=0x80156000u;
    int policy=PE_Decomp_BoundaryStopEnabled();
    TEST("effect_nested_callback_stop");
    ResetTestState(); PE_Decomp_SetBoundaryStop(1);
    PE_StoreU16(body+14u,2u); PE_StoreU8(body+12u,1u);
    for (unsigned i=0;i<8u;i++) PE_StoreU16(rec+i*12u,0xFFFFu);
    PE_StoreU16(rec,0u); PE_StoreU16(rec+2u,7u);
    PE_StoreU32(body+128u,desc); PE_StoreU32(desc,0x81234567u);
    (void)func_800D413C(slot);
    ASSERT(PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           PE_LoadU16(rec)==0u && PE_LoadU16(rec+2u)==7u && PE_LoadU8(body+12u)==1u,
           "effect record changed after stopped update callback");

    ResetTestState(); PE_Decomp_SetBoundaryStop(1);
    PE_StoreU32(slot+0x8Cu,desc); PE_StoreU32(desc+0x30u,0x81234567u);
    PE_StoreU32(slot+0x14u,0xA5A5A5A5u);
    (void)func_800D4698(slot,0u,1u,0u,0u,0u);
    ASSERT(PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           PE_LoadU32(slot+0x14u)==0xA5A5A5A5u,
           "effect initialization published a stopped callback result");

    ResetTestState(); PE_Decomp_SetBoundaryStop(1);
    PE_StoreU32(0x800E2368u,body); PE_StoreU32(body+4u,data);
    PE_StoreU32(body+128u,desc); PE_StoreU32(desc,0x81234567u);
    PE_StoreU16(desc+0x20u,4u);
    for (unsigned i=0;i<8u;i++) PE_StoreU16(rec+i*12u,0xFFFFu);
    (void)func_800D401C(0);
    ASSERT(PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           PE_LoadU32(rec+8u)==data+4u && PE_LoadU32(body+4u)==data,
           "effect allocation continued after stopped initializer");
    PE_Decomp_SetBoundaryStop(policy);
    PASS();
}

static void test_effect_update_callback_stop(void)
{
    const pe_addr_t pool=0x80150000u,table=0x80158000u,entry=0x80158400u;
    TEST("effect_update_callback_stop");
    ResetTestState();
    PE_StoreU32(0x800942E4u,pool); PE_StoreU32(0x800942E0u,table);
    PE_StoreU32(table,entry); PE_StoreU32(entry+16u,0x81234567u);
    PE_StoreU8(pool,2u); PE_StoreU32(pool+4u,0x12345678u);
    PE_StoreU8(pool+0xA0Cu,4u);
    D_8009D1A0=0x80u;
    int policy=PE_Decomp_BoundaryStopEnabled();
    PE_Decomp_SetBoundaryStop(1);
    (void)func_80069594();
    PE_Decomp_SetBoundaryStop(policy);
    ASSERT(PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           PE_LoadU32(pool+4u)==0x12345678u && PE_LoadU8(pool+0xA0Cu)==4u,
           "effect pump mutated the stopped or later slot");
    PASS();
}

static void test_ATK23_retail_effect_tick(void)
{
    unsigned k,i;
    TEST("ATK23_retail_effect_tick");
    for (k=0;k<sizeof(ATK23_effect_cases)/sizeof(ATK23_effect_cases[0]);k++) {
        const uint32_t *a=ATK23_effect_cases[k].args;
        uint32_t result=0;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(ATK23_effect_common)/sizeof(ATK23_effect_common[0]);i++)
            PE_StoreU32(0x80000000u+ATK23_effect_common[i][0],ATK23_effect_common[i][1]);
        for (i=ATK23_effect_cases[k].first;i<ATK23_effect_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ATK23_effect_patches[i][0],ATK23_effect_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        switch (ATK23_effect_cases[k].entry) {
        case 0: result=(uint32_t)func_8006F9F0(a[0]);break;
        case 1: result=(uint32_t)func_800D413C(a[0]);break;
        case 2: result=(uint32_t)func_80069594();break;
        case 3: result=(uint32_t)func_800D4704(a[0]);break;
        case 4: result=(uint32_t)func_800D401C((int32_t)a[0]);break;
        case 5: result=func_800CE560(a[0],a[1],(int32_t)a[2],a[3]);break;
        case 6: result=func_800CE5AC(a[0],a[1],a[2],(int32_t)a[3],a[4]);break;
        case 7: result=func_800CE610(a[0]);break;
        case 8: result=(uint32_t)func_800CE688(a[0]);break;
        case 9: result=(uint32_t)func_800CE78C(a[0]);break;
        case 10:func_800CE870(a[0],(int32_t)a[1],a[2]);break;
        case 11:func_800CE8F0(a[0],a[1],a[2],a[3]);break;
        case 12:result=(uint32_t)func_800D3FD8();break;
        case 13:result=(uint32_t)func_8006DC18(a[0]);break;
        }
        hash=hit_camera_hash(ATK23_effect_ranges,sizeof(ATK23_effect_ranges)/sizeof(ATK23_effect_ranges[0]));
        if (hash!=ATK23_effect_cases[k].hash || result!=ATK23_effect_cases[k].result) {
            fprintf(stderr,"effect tick %u: result %08X/%08X hash %016llX/%016llX\n",k,
                result,ATK23_effect_cases[k].result,(unsigned long long)hash,
                (unsigned long long)ATK23_effect_cases[k].hash);
            if (getenv("PE_ATK23_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk23-native-%u.bin",k);out=fopen(path,"wb");
                if (out) { for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out); }
            }
        }
        ASSERT(result==ATK23_effect_cases[k].result,"effect return differs from original");
        ASSERT(hash==ATK23_effect_cases[k].hash,"effect memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"effect call graph executes natively");
    }
    PASS();
}
