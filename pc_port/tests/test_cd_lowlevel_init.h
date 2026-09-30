#include "retail_cd_lowlevel_init_cases.h"
static void CdLowSeed(uint32_t seed)
{
    for(unsigned j=0;j<sizeof(CDLOW_ranges)/sizeof(CDLOW_ranges[0]);j++)
        for(unsigned i=0;i<CDLOW_ranges[j][1];i++) PE_StoreU8(0x80000000u+CDLOW_ranges[j][0]+i,(uint8_t)(seed+i*17u));
}
static void test_DAY2_cd_lowlevel_init(void)
{
    TEST("DAY2_cd_lowlevel_init");
    for(unsigned k=0;k<sizeof(CDLOW_guards)/sizeof(CDLOW_guards[0]);k++) {
        ResetTestState();CdLowSeed(CDLOW_guards[k].seed);PE_StoreU32(0x8009B554u,CDLOW_guards[k].guard);
        ASSERT((uint32_t)func_8007EC14()==CDLOW_guards[k].result &&
            hit_camera_hash(CDLOW_ranges,11)==CDLOW_guards[k].hash,"public CD guard differs from original");
    }
    for(unsigned k=0;k<sizeof(CDLOW_cases)/sizeof(CDLOW_cases[0]);k++) {
        ResetTestState();HostFB_Init();PE_GPU_Init();PE_Callback_Init();B558_PlantPointers();CdLowSeed(CDLOW_cases[k].seed);
        PE_StoreU16(0x800945E4u,1u);PE_StoreU16(0x800945E6u,0u);PE_StoreU16(0x80094614u,9u);
        PE_StoreU32(0x800945F0u,CDLOW_cases[k].old);PE_StoreU32(0x8009AFC0u,0u);
        (void)PE_IRQ_ExchangeMask((uint16_t)CDLOW_cases[k].mask);
        for(unsigned i=0;i<4;i++) PE_CdReg_WriteU8(0x1F801800u+i,(uint8_t)(CDLOW_cases[k].seed+i*17u));
        PE_CdReg_WriteU8(0x1F801803u,0u);PE_CdReg_WriteU32(0x1F801020u,PE_LoadU32(0x80130004u));
        PE_StoreU16(0x801501B8u,(uint16_t)CDLOW_cases[k].audio);PE_StoreU16(0x801501BAu,0u);
        for(unsigned i=0;i<0x200u;i+=2) PE_SpuRegister_StoreU16(i,PE_LoadU16(0x80150000u+i));
        PE_StoreU32(0x8009B290u,0x1F801C00u);PE_StoreU32(0x8009B200u,1u);PE_StoreU32(0x8009B224u,1u);
        for(unsigned i=0;i<36;i++) PE_StoreU8(0x8009568Cu+i,0u);
        ASSERT(func_8007F994()==1 && !PE_Port_ShouldStop(),"low-level init should ignore ordinary controller rejection");
        if(CDLOW_cases[k].tick) ASSERT(PE_Callback_DispatchChecked(),"installed CD updater failed to dispatch");
        PE_StoreU32(0x80130000u,PE_CdReg_ReadU32(0x1F801800u));PE_StoreU32(0x80130004u,PE_CdReg_ReadU32(0x1F801020u));
        PE_StoreU16(0x80130020u,PE_IRQ_GetMask());
        for(unsigned i=0;i<0x200u;i+=2) PE_StoreU16(0x80150000u+i,PE_SpuRegister_LoadU16(i));
        uint64_t hash=hit_camera_hash(CDLOW_ranges,11);
        if(hash!=CDLOW_cases[k].hash) fprintf(stderr,"CD low init case%u hash%016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)CDLOW_cases[k].hash);
        ASSERT(hash==CDLOW_cases[k].hash,"low-level initializer differs from original");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"low-level init unexpected boundary");
    }
    PASS();
}
