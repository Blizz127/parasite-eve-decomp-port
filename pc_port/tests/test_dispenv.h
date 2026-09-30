#include "retail_dispenv_cases.h"
static void DAY1_SeedDisplayDispatch(void)
{
    PE_StoreU32(0x80095744u,0x80095704u);
    PE_StoreU32(0x80095714u,0x80076B20u);
    PE_StoreU32(0x80095748u,0x80071A74u);
    for(unsigned i=0;i<sizeof(DAY1_dispenv_table);i++)PE_StoreU8(0x80095820u+i,DAY1_dispenv_table[i]);
}
static void test_DAY1_dispenv(void)
{
    TEST_RETAIL_DISC1("DAY1_dispenv"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    static const int widths[]={-32768,-1,0,280,281,352,353,400,401,560,561,32767};
    static const int values[]={-32768,-1000,-1,0,1,2,255,256,257,288,289,32767};
    static const uint32_t ranges[][2]={{0x150000,20},{0x957B8,20},{0xA3348,256}};
    for(unsigned n=0;n<2048;n++) {
        ResetTestState();DAY1_SeedDisplayDispatch();
        PE_Fill(0x80150000u,20,0);PE_Fill(0x800957B8u,20,0xA5);PE_Fill(0x800A3348u,256,0);
        PE_StoreU32(0x800956ECu,n%2);
        PE_StoreU8(0x8009574Eu,n%127==0?2:0);PE_StoreU8(0x8009574Fu,n/2%2);
        for(unsigned i=0;i<8;i++)PE_StoreU16(0x80150000u+i*2,i==2?widths[n%12]:values[(n/(i+1)+i)%12]);
        PE_StoreU8(0x80150010u,n/4%2);PE_StoreU8(0x80150011u,n/8%2);PE_StoreU8(0x80150012u,n/16%9);PE_StoreU8(0x80150013u,n/32%256);
        if(n&64){memcpy(PE_Translate(0x800957B8u,8),PE_Translate(0x80150000u,8),8);memcpy(PE_Translate(0x800957C8u,4),PE_Translate(0x80150010u,4),4);}
        if(n&128)memcpy(PE_Translate(0x800957C0u,8),PE_Translate(0x80150008u,8),8);
        ASSERT(func_800755F0(0x80150000u)==0x80150000u,"PutDispEnv return differs");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"PutDispEnv stopped");
        ASSERT(hit_camera_hash(ranges,3)==DAY1_dispenv_cases[n].hash,"PutDispEnv input/cache state differs from original");
        PeGpuState gpu;PE_GPU_GetState(&gpu);
        ASSERT(gpu.display_command_count==DAY1_dispenv_cases[n].count,"PutDispEnv command count differs");
        for(unsigned i=0;i<4;i++)ASSERT(gpu.display_commands[i]==DAY1_dispenv_cases[n].commands[i],"PutDispEnv ordered GP1 words differ");
    }
    /* Unknown live dispatch identities must not copy/present the environment. */
    ResetTestState();DAY1_SeedDisplayDispatch();PE_StoreU32(0x80095714u,0x80123456u);
    PE_Fill(0x800957B8u,20,0xA5);
    func_800755F0(0x80150000u);
    ASSERT(PE_Port_ShouldStop() && PE_LoadU32(0x800957B8u)==0xA5A5A5A5u,"unknown GP1 writer must preserve the nonreturn prefix");
    ResetTestState();DAY1_SeedDisplayDispatch();PE_StoreU32(0x80095714u,0x80123456u);
    PE_StoreU32(0x8009575Cu,0xA5A5A5A5u);
    func_80070E54();
    ASSERT(PE_Port_ShouldStop() && !PE_LoadU32(0x8009CDDCu) && PE_LoadU32(0x8009575Cu)==0xA5A5A5A5u,"frame tail must not draw/flip after display nonreturn");
    ResetTestState();
    for(unsigned v=0;v<256;v++) {
        ASSERT(PE_GPU_WriteGP1(0x08000000u|v),"GPU must accept all display mode payloads");
        uint32_t status=PE_GPU_ReadStatus();
        ASSERT(((status>>17)&63u)==(v&63u) && ((status>>16)&1u)==((v>>6)&1u) && ((status>>14)&1u)==((v>>7)&1u),"GPUSTAT display mode readback differs");
    }
    ASSERT(PE_GPU_WriteGP1(0),"GP1 reset rejected");
    PeGpuState reset;PE_GPU_GetState(&reset);
    ASSERT(!reset.display_mode && !reset.display_start && !(reset.status&0x7F4000u) && reset.display_horizontal_range==0xC00200u && reset.display_vertical_range==0x40010u,"GP1 reset must reset display registers");
    PASS();
}
