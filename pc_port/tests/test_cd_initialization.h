#include "retail_cd_initialization_cases.h"
static void test_DAY2_cd_initialization(void)
{
    TEST("DAY2_cd_initialization");
    for(unsigned k=0;k<sizeof(CDINIT_states)/sizeof(CDINIT_states[0]);k++) {
        static const uint32_t ranges[][2]={{0x9B554u,0x50u}};
        ResetTestState();
        for(unsigned i=0;i<0x50u;i++) PE_StoreU8(0x8009B554u+i,(uint8_t)(CDINIT_states[k].seed+i*17u));
        func_8007FA2C();
        ASSERT(hit_camera_hash(ranges,1)==CDINIT_states[k].hash,"CD SDK startup state differs from original");
    }
    for(unsigned k=0;k<sizeof(CDINIT_cases)/sizeof(CDINIT_cases[0]);k++) {
        ResetTestState();B558_PlantPointers();
        for(unsigned j=0;j<6;j++) for(unsigned i=0;i<CDINIT_ranges[j][1];i++)
            PE_StoreU8(0x80000000u+CDINIT_ranges[j][0]+i,(uint8_t)(CDINIT_cases[k].seed+i*17u));
        PE_StoreU16(0x800945E4u,1u);PE_StoreU16(0x80094614u,9u);PE_StoreU32(0x800945F0u,CDINIT_cases[k].old);
        PE_StoreU32(0x8009AFC0u,CDINIT_cases[k].debug);(void)PE_IRQ_ExchangeMask((uint16_t)CDINIT_cases[k].mask);
        for(unsigned i=0;i<4;i++) PE_CdReg_WriteU8(0x1F801800u+i,(uint8_t)(CDINIT_cases[k].seed+i*17u));
        PE_CdReg_WriteU8(0x1F801803u,0u);
        PE_CdReg_WriteU32(0x1F801020u,PE_LoadU32(0x80130004u));
        PE_StoreU32(0x8009B200u,1u);PE_StoreU32(0x8009B224u,1u);
        int result=func_8007BBFC();
        for(unsigned i=0;i<4;i++) PE_StoreU8(0x80130000u+i,PE_CdReg_ReadU8(0x1F801800u+i));
        PE_StoreU32(0x80130004u,PE_CdReg_ReadU32(0x1F801020u));PE_StoreU16(0x80130020u,PE_IRQ_GetMask());
        ASSERT((uint32_t)result==CDINIT_cases[k].result,"CD rejected Init result differs from original");
        ASSERT(hit_camera_hash(CDINIT_ranges,6)==CDINIT_cases[k].hash,"CD startup rejected-command state differs from original");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"CD startup rejection unexpected boundary");
    }
    PASS();
}
