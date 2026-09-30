#include "retail_cd_command_cases.h"
static void test_DAY2_cd_command(void)
{
    TEST("DAY2_cd_command");
    for(unsigned k=0;k<sizeof(CDCMD_cases)/sizeof(CDCMD_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<0x50u;i++) PE_StoreU8(0x8009B554u+i,(uint8_t)(37u+i*17u));
        for(unsigned i=0;i<26u;i++) PE_StoreU32(0x80011D0Cu+i*4u,CDCMD_table[i]);
        PE_StoreU32(0x8009B554u,CDCMD_cases[k].cb==2u?0u:1u);
        PE_StoreU8(0x8009B558u,(uint8_t)CDCMD_cases[k].cmd);PE_StoreU8(0x8009B559u,(uint8_t)CDCMD_cases[k].mode);
        PE_StoreU8(0x8009B581u,0u);PE_StoreU32(0x8009B570u,CDCMD_cases[k].kind);
        PE_StoreU32(0x8009B578u,CDCMD_cases[k].state);PE_StoreU32(0x8009B57Cu,CDCMD_cases[k].phase);
        PE_StoreU32(0x8009B58Cu,CDCMD_cases[k].timer);PE_StoreU32(0x8009B5A0u,CDCMD_cases[k].delay);
        PE_StoreU32(0x8009B624u+CDCMD_cases[k].cmd*4u,1u);
        for(unsigned i=0;i<3u;i++) PE_StoreU32(0x800A36A4u+i*4u,CDCMD_cases[k].cb?0x80170000u+i*16u:0u);
        for(unsigned i=0;i<8u;i++) PE_StoreU8(0x80140000u+i,(uint8_t)(165u+i*17u));
        PE_StoreU8(0x80140000u,(uint8_t)CDCMD_cases[k].flags);
        func_80080164(CDCMD_cases[k].status,0x80140000u);
        uint64_t hash=hit_camera_hash(CDCMD_ranges,1);
        if(hash!=CDCMD_cases[k].hash) fprintf(stderr,"CD command case%u kind%u cmd%u hash%016llX/%016llX\n",k,CDCMD_cases[k].kind,CDCMD_cases[k].cmd,(unsigned long long)hash,(unsigned long long)CDCMD_cases[k].hash);
        ASSERT(hash==CDCMD_cases[k].hash,"command completion differs from original");
        if(CDCMD_cases[k].boundary) {
            ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_data_callback")==1 &&
                g_bootstrap_arg4_calls[0].target==CDCMD_cases[k].boundary &&
                g_bootstrap_arg4_calls[0].arg0==CDCMD_cases[k].arg0 &&
                g_bootstrap_arg4_calls[0].arg1==CDCMD_cases[k].arg1,"command callback boundary contract");
        } else ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"command completion unexpected boundary");
    }
    PASS();
}
