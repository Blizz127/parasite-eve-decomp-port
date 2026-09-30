#include "retail_controller_sdk_cases.h"
static void test_DAY1_controller_sdk(void)
{
    TEST("DAY1_controller_sdk");
    static const uint32_t ranges[][2]={{0x9B738,12},{0xA5B70,0x1E0},{0x150000,0x400},{0x160000,0x100}};
    for(unsigned k=0;k<sizeof(DAY1_controller_sdk_cases)/sizeof(DAY1_controller_sdk_cases[0]);k++) {
        ResetTestState();unsigned seed=DAY1_controller_sdk_cases[k].seed;
        for(unsigned j=0;j<4;j++)for(unsigned i=0;i<ranges[j][1];i++)
            PE_StoreU8(0x80000000u+ranges[j][0]+i,(uint8_t)(seed+i*29u+(i>>3)));
        PE_StoreU32(0x8009B738u,0x80084B20u);PE_StoreU32(0x8009B740u,0x80084F8Cu);
        for(unsigned j=0;j<2;j++) {
            pe_addr_t p=0x800A5B70u+j*0xF0u;
            PE_StoreU32(p,0x80150000u);PE_StoreU32(p+0x10u,(seed&1u)?p:0);
            PE_StoreU32(p+0x30u,0x80160000u+j*4u);PE_StoreU32(p+0x34u,(seed&2u)?0x10000u:seed&255u);
            PE_StoreU8(p+0x38u,(uint8_t)(seed&4u));PE_StoreU8(p+0x49u,(uint8_t)((seed/8)%8));
            PE_StoreU8(p+0x46u,(seed&16u)?255:1);
            static const uint16_t counts[]={1,2,65535};
            PE_StoreU16(p+0xE6u,(seed&32u)?0:counts[seed%3]);
            PE_StoreU8(p+0xE3u,(uint8_t)(seed%4));PE_StoreU8(p+0xE4u,(uint8_t)(seed&1));
            PE_StoreU8(0x80160000u+j*4u,(uint8_t)(seed&64));
        }
        const uint32_t *a=DAY1_controller_sdk_cases[k].args;
        uint32_t port=DAY1_controller_sdk_cases[k].port,result=0;
        switch(DAY1_controller_sdk_cases[k].kind) {
        case 0:result=(uint32_t)func_800825C0(port);break;
        case 1:result=func_80082680(port,(int32_t)a[0],(int32_t)a[1]);break;
        case 2:result=(uint32_t)func_800828F4(port,a[0]);break;
        case 3:result=(uint32_t)func_8008292C(port,a[0],a[1]);break;
        case 4:func_80082974(port,a[0],a[1]);break;
        }
        ASSERT(result==DAY1_controller_sdk_cases[k].result,"controller SDK result differs from original");
        ASSERT(hit_camera_hash(ranges,4)==DAY1_controller_sdk_cases[k].hash,"controller SDK guest state differs from original");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"controller SDK stopped or hit stub");
    }
    ResetTestState();PE_StoreU32(0x8009B738u,0x80123456u);
    ASSERT(func_800825C0(0)==0 && PE_Port_ShouldStop(),"unknown slot callback must stop");
    ResetTestState();PE_StoreU32(0x8009B738u,0x80084B20u);PE_StoreU32(0x8009B740u,0x80123456u);
    PE_StoreU32(0x800A5B84u,0x12345678u);
    ASSERT(func_800828F4(0,0x80160000u)==0 && PE_Port_ShouldStop(),"unknown busy callback must stop");
    ASSERT(PE_LoadU32(0x800A5B84u)==0x12345678u,"unknown callback must preserve queue prefix");
    PASS();
}
