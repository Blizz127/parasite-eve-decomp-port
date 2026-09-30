#include "retail_script_timer_cases.h"
static void test_DAY1_script_timer(void)
{
    TEST("DAY1_script_timer");
    static const uint32_t values[]={0,1,59,60,61,3599,3600,3601,215999,216000,216001,219660,0x7FFFFFFFu,0x80000000u,0xFFFFFFFFu,(uint32_t)-60,(uint32_t)-3600,(uint32_t)-216000};
    static const uint32_t ranges[][2]={{0x150000,128}};
    for(unsigned n=0;n<2048;n++) {
        ResetTestState();
        for(unsigned i=0;i<128;i+=4)PE_StoreU32(0x80150000u+i,0xA5A50000u+i);
        uint32_t value=n<1024?values[n%18]:n*2654435761u,index=n%12;
        PE_StoreU32(0x800A76A4u+index*12,value);PE_StoreU32(0x80150040u,index);
        PE_StoreU32(0x80150000u,0x80150040u);PE_StoreU32(0x80150004u,0x80150044u);
        ASSERT(func_80019DB8(0x80150000u)==1,"D2 return differs");
        PE_StoreU32(0x80150000u,0x80150044u);
        for(unsigned j=0;j<3;j++) {
            unsigned alias=n/18%8;pe_addr_t dest=0x80150048u+j*4;
            if(alias==j+1)dest=0x80150044u;
            if(alias==4)dest=0x80150048u;
            if(alias==5 && j==2)dest=0x80150048u;
            if(alias==6 && j==0)dest=0x80150004u;
            if(alias==7 && j==1)dest=0x80150008u;
            PE_StoreU32(0x80150004u+j*4,dest);
        }
        ASSERT(func_80019DF4(0x80150000u)==1,"D3 return differs");
        ASSERT(hit_camera_hash(ranges,1)==DAY1_script_timer_cases[n],"D2/D3 signed divisions or aliased write order differs from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"timer command stopped");
    }
    ResetTestState();
    const pe_addr_t actor=0x800BEA90u,task=0x8009D310u,stream=0x80122000u;
    PE_StoreU32(0x800910A0u+0xD3u*4,0x80019DF4u);PE_StoreU32(0x800910A4u,0x800172BCu);
    PE_StoreU32(0x8009D2F0u,actor);PE_StoreU32(0x8009D300u,task);
    PE_StoreU32(task,stream);PE_StoreU32(task+0x10u,1);PE_StoreU32(actor+0x98u,0x100000E0u);
    PE_StoreU32(stream,0x80D3u);PE_StoreU32(stream+8u,219660u);PE_StoreU32(stream+24u,1);
    func_80017018();
    ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"D3 still reaches VM boundary");
    ASSERT(PE_LoadU32(stream+12u)==1 && PE_LoadU32(stream+16u)==1 && PE_LoadU32(stream+20u)==1,"VM D3 binding/outputs differ");
    ASSERT(PE_LoadU32(actor+0x98u)&0x10u,"VM did not continue to next opcode");
    PASS();
}
