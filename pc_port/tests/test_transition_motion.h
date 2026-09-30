#include "retail_transition_motion_cases.h"
static const uint32_t DAY1_motion_ranges[][2]={{0x19BFCC,4},{0x19C038,0x24},{0x19CAA8,160},{0x1EA268,160},{0x1EA578,24},{0x150000,512}};
static void DAY1_motion_fixture(unsigned seed,unsigned alias,uint32_t time,unsigned kind,uint32_t id,unsigned fault)
{
    static const int banks[]={0,1,-1,7,-7,8,-8,511,512,513,-511,-512,-513,383,384,385,-383,-384,-385,32767,-32768,200,-200,16};
    static const uint32_t slots[][6]={{0,64,128,192,256,320},{0,0,128,128,256,320},{0,4,128,132,256,320},{0,0,0,0,0,0},{0,144,128,16,256,320}};
    DAY1_camera_fixture(seed,0);
    for(unsigned j=0;j<6;j++)for(unsigned i=0;i<DAY1_motion_ranges[j][1];i++)PE_StoreU8(0x80000000u+DAY1_motion_ranges[j][0]+i,(uint8_t)(seed+i*17));
    PE_StoreU32(0x8019C038u,time);PE_StoreU32(0x8019C03Cu,time);
    for(unsigned i=0;i<6;i++)PE_StoreU32(0x801EA578u+i*4,0x80150000u+slots[alias][i]);
    PE_StoreU16(0x80150000u+slots[alias][0]+44u,(uint16_t)banks[seed]);
    PE_StoreU16(0x80150000u+slots[alias][2]+44u,(uint16_t)-banks[seed]);
    if(seed%4==0)for(int i=-1;i<257;i++)for(unsigned j=0;j<9;j++)for(unsigned k=0;k<3;k++)
        PE_StoreU16(0x80141000u+(uint32_t)(i+1)*96u+8u+j*8u+k*2u,(uint16_t)(i*7919+(int)k*29009+(int)seed*4711));
    if(fault) {
        unsigned index=kind?(id&65535u)+64u:73u+fault;
        pe_addr_t rec=0x80140000u+PE_LoadU32(0x80140000u+index*4u);PE_StoreU16(rec+6u,2);
    }
    for(unsigned i=0;i<24;i++)PE_StoreU8(0x1F800380u+i,(uint8_t)(i*13+7));
}
static void test_DAY1_transition_motion(void)
{
    TEST_RETAIL_DISC1("DAY1_transition_motion"); TEST_RETAIL_FIXUPS(RETAILFIX_transition_path);
    for(unsigned k=0;k<sizeof(DAY1_motion_cases)/sizeof(DAY1_motion_cases[0]);k++) {
        unsigned kind=DAY1_motion_cases[k].kind,fault=DAY1_motion_cases[k].fault;
        uint32_t id=DAY1_motion_cases[k].id;
        if(!DAY1_motion_cases[k].step)DAY1_motion_fixture(DAY1_motion_cases[k].seed,DAY1_motion_cases[k].alias,DAY1_motion_cases[k].time,kind,id,fault);
        if(!kind)func_80193478();else if(kind==1)func_801938E8(id);else func_801939B0(id);
        uint64_t hash=hit_camera_hash(DAY1_motion_ranges,6);
        if(hash!=DAY1_motion_cases[k].hash)fprintf(stderr,"motion%u kind%u step%u RAM %016llX/%016llX\n",k,kind,DAY1_motion_cases[k].step,(unsigned long long)hash,(unsigned long long)DAY1_motion_cases[k].hash);
        ASSERT(hash==DAY1_motion_cases[k].hash,"motion differs from original persistent RAM");
        ASSERT(!!PE_Port_ShouldStop()==!!fault,"motion zero-period boundary differs");
        for(unsigned i=0;i<24;i++)ASSERT(PE_LoadU8(0x1F800380u+i)==(uint8_t)(i*13+7),"motion temporary scratch escaped");
        ASSERT(!g_stub_order_count,"motion used stub");
    }
    PASS();
}
