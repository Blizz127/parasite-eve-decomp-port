#include "retail_card_operation_cases.h"
static int card_operation_boundary_matches(uint32_t target,uint32_t mask,const uint32_t args[4],uint32_t fifth)
{
    if(!target)return !PE_Port_ShouldStop() && !g_stub_order_count;
    if(!PE_Port_ShouldStop() || g_bootstrap_arg4_call_count!=1)return 0;
    const BootstrapArgCall4 *call=&g_bootstrap_arg4_calls[0];
    uint32_t metadata[2]={0,0};
    if(call->payload_size!=sizeof(metadata))return 0;
    memcpy(metadata,call->payload,sizeof(metadata));
    return call->target==target && call->arg0==args[0] && call->arg1==args[1] &&
           call->arg2==args[2] && call->arg3==args[3] && metadata[0]==mask && metadata[1]==fifth;
}
static void test_DAY1_card_operation(void)
{
    TEST("DAY1_card_operation");
    static const uint32_t ranges[][2]={{0xA0ED4,0xA00},{0x92224,4},{0x92230,4},{0x160000,16},{0x9D154,16}};
    static const uint8_t flags[]={0,1,5,255},counts[]={0,1,2,15},centers[]={0,7,14},statuses[]={0,1,4,255};
    static const uint16_t sizes[]={0,127,128,129,1023,1024,1025,0x8000,0xFFFF};
    for(unsigned n=0;n<8192;n++) {
        ResetTestState();
        for(unsigned i=0;i<0xA00;i++)PE_StoreU8(0x800A0ED4u+i,0xA5);
        unsigned index=n/64%2,variant=n/128;pe_addr_t record=0x800A0ED4u+index*0x418u;
        unsigned state=n%16!=15 || variant%2==0?n%16:255;
        PE_StoreU8(record,flags[n/16%4]);PE_StoreU8(record+1u,state);
        PE_StoreU8(record+2u,counts[variant%4]);PE_StoreU8(record+3u,variant%15);
        PE_StoreU8(record+5u,centers[variant/4%3]);PE_StoreU8(record+6u,variant%31);
        PE_StoreU8(record+8u,statuses[variant/4%4]);PE_StoreU8(record+11u,variant*3);
        PE_StoreU8(0x800A0EDCu+(1-index)*0x418u,statuses[variant/16%4]);
        PE_StoreU16(record+20u,sizes[variant%9]);PE_StoreU32(record+12u,0xF1000000u+n);PE_StoreU32(record+24u,0x80170000u+n*4);
        for(unsigned i=0;i<15;i++) {PE_StoreU8(record+i*0x44u+29u,(i+variant)%3);PE_StoreU8(record+i*0x44u+69u,i+variant);}
        PE_StoreU32(0x80092224u,0x80161000u);PE_StoreU32(0x80092230u,0x80160000u);PE_StoreU32(0x800A1704u,n*0x10203u);
        for(unsigned i=0;i<4;i++)PE_StoreU32(0x8009D154u+i*4,0u);
        PE_StoreU32(0x800A185Cu,0u);PE_StoreU32(0x8009CFFCu,0u);PE_StoreU32(0x8009CF44u,0u);
        func_80041108(index);
        uint64_t hash=hit_camera_hash(ranges,5);
        if(hash!=DAY1_card_operation_cases[n].hash)fprintf(stderr,"card operation %u state differs\n",n);
        ASSERT(hash==DAY1_card_operation_cases[n].hash,"operation effects differ from original");
        ASSERT(card_operation_boundary_matches(DAY1_card_operation_cases[n].target,DAY1_card_operation_cases[n].mask,DAY1_card_operation_cases[n].args,DAY1_card_operation_cases[n].fifth),"operation call boundary differs from original");
    }
    PASS();
}
