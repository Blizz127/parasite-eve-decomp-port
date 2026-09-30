#include "retail_vblank_cases.h"
static unsigned vblank_seen,vblank_test_mode,vblank_bad_context;
static void test_vblank_callback(void)
{
    vblank_seen++;
    if((PE_IRQ_ReadStatus()&1u) || PE_LoadU16(0x800945E6u)!=1 || PE_LoadU32(0x800956ACu)!=1)vblank_bad_context=1;
    if(vblank_test_mode==1 && vblank_seen==1) {
        PE_Callback_SetSlot(1,0);PE_Callback_SetSlot(7,0x80130000u);
    }
    if(vblank_test_mode==2)PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}
static void test_DAY1_vblank(void)
{
    TEST_RETAIL_DISC1("DAY1_vblank"); TEST_RETAIL_FIXUPS(RETAILFIX_vblank);
    static const uint32_t ranges[][2]={{0x70DCC,0x84},{0x9568C,36},{0xA76A0,48},{0x9D1A0,4},{0xB0DB8,8}};
    for(unsigned n=0;n<512;n++) {
        ResetTestState();PE_Callback_Init();
        PE_Callback_Bind(0x8003E91Cu,func_8003E91C);
        for(unsigned j=0;j<5;j++)for(unsigned i=0;i<ranges[j][1];i+=4)PE_StoreU32(0x80000000u+ranges[j][0]+i,n*101u+i*2654435761u);
        for(unsigned i=0;i<sizeof(DAY1_vblank_rng_code);i++)PE_StoreU8(0x80070DCCu+i,DAY1_vblank_rng_code[i]);
        PE_StoreU32(0x80070E04u,(n%17)*4u);PE_StoreU32(0x80070E08u,((n+3)%17)*4u);
        for(unsigned i=0;i<8;i++)PE_StoreU32(0x8009568Cu+i*4u,(n>>i&1u)?0x8003E91Cu:0);
        PE_StoreU32(0x800956ACu,(n&1)?UINT32_MAX:n);
        static const uint32_t flags[]={0,1,0x40,0x41},gates[]={0,1,0xFFFF},values[]={0,1,UINT32_MAX,0x7FFFFFFF};
        PE_StoreU32(0x8009D1A0u,flags[n/16%4]);PE_StoreU32(0x800B0DB8u,((n/64)%4)<<16);PE_StoreU32(0x800B0DBCu,gates[n/128%3]);
        for(unsigned i=0;i<4;i++){PE_StoreU32(0x800A76A0u+i*12,(n/4+i)%8);PE_StoreU32(0x800A76A4u+i*12,values[(n+i)%4]);}
        ASSERT(PE_Callback_DispatchChecked(),"VBlank original graph stopped");
        ASSERT(hit_camera_hash(ranges,5)==DAY1_vblank_hashes[n],"VBlank RNG/timer/dispatcher state differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"VBlank reached stub");
    }
    for(unsigned mode=0;mode<4;mode++) {
        ResetTestState();PE_Callback_Init();func_80073C94();
        vblank_seen=vblank_bad_context=0;vblank_test_mode=mode;
        PE_Callback_Bind(0x80130000u,test_vblank_callback);
        PE_Callback_SetSlot(0,0x80130000u);PE_Callback_SetSlot(1,0x80135555u);PE_Callback_SetSlot(2,0x80130000u);
        if(mode==0)PE_Callback_SetSlot(1,0);
        PeIrqGeneration generation=PE_IRQ_Generation();
        PE_IRQ_ExchangeMask(0);PE_IRQ_AssertSources(1);
        ASSERT(PE_IRQ_ServicePendingForGeneration(generation)==PE_IRQ_SERVICE_RETURNED && !vblank_seen && PE_IRQ_ReadStatus()==1,"masked VBlank must remain pending");
        ASSERT(PE_IRQ_ServicePendingForGeneration(generation-1)==PE_IRQ_SERVICE_STALE && !vblank_seen,"stale IRQ must not dispatch VBlank");
        PE_IRQ_ExchangeMask(1);
        PeIrqServiceResult result=PE_IRQ_ServicePendingForGeneration(generation);
        ASSERT(!vblank_bad_context,"VBlank callback ran before CPU acknowledgement/active/counter writes");
        ASSERT(vblank_seen==(mode==0?2u:mode==1?3u:1u),"live callback mutation or stop order differs");
        ASSERT(result==(mode<2?PE_IRQ_SERVICE_RETURNED:PE_IRQ_SERVICE_BOUNDARY),"VBlank IRQ return classification differs");
        ASSERT(PE_LoadU16(0x800945E6u)==(mode<2?0u:1u),"nonreturn must retain dispatch-active prefix");
        ASSERT(PE_IRQ_ReadStatus()==0 && PE_LoadU32(0x800956ACu)==1,"VBlank acknowledgement/counter differs");
    }
    PASS();
}
