#include "retail_cd_registration_cases.h"
static void test_DAY2_cd_registration(void)
{
    TEST("DAY2_cd_registration");
    for(unsigned k=0;k<sizeof(CDREG_cases)/sizeof(CDREG_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<0x32u;i++) PE_StoreU8(0x800945E4u+i,(uint8_t)(37u+i*17u));
        PE_StoreU16(0x800945E4u,(uint16_t)CDREG_cases[k].guard);
        PE_StoreU32(0x800945F0u,CDREG_cases[k].old);
        PE_StoreU16(0x80094614u,(uint16_t)CDREG_cases[k].registered);
        (void)PE_IRQ_ExchangeMask((uint16_t)CDREG_cases[k].mask);
        pe_addr_t result=func_80073CC4(2u,CDREG_cases[k].next);
        ASSERT(result==CDREG_cases[k].result && hit_camera_hash(CDREG_ranges,1)==CDREG_cases[k].hash,"CD registration RAM/return differs from original");
        ASSERT(PE_IRQ_GetMask()==CDREG_cases[k].final_mask,"CD registration mask differs from original");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"CD registration unexpected boundary");
    }
    PASS();
}
