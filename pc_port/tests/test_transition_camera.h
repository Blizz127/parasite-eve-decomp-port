#include "retail_transition_camera_cases.h"
static const uint32_t DAY1_camera_ranges[][2]={{0x19C020u,0xA0},{0x19C330u,16},{0x19C810u,16},{0x19BFCCu,4}};
static void DAY1_camera_fixture(unsigned seed,uint32_t variant)
{
    ResetTestState();
    for(unsigned i=0;i<1025;i++)PE_StoreU16(0x8009A6ECu+i*2,DAY1_path_atan[i]);
    for(unsigned i=0;i<4;i++)for(unsigned j=0;j<DAY1_camera_ranges[i][1];j++)PE_StoreU8(0x80000000u+DAY1_camera_ranges[i][0]+j,(uint8_t)(seed+j*29));
    PE_StoreU32(0x801D0268u,0x80140000u-0x801D0260u);
    for(int i=-1;i<257;i++){
        pe_addr_t rec=0x80141000u+(uint32_t)(i+1)*96u;
        PE_StoreU32(0x80140000u+(uint32_t)i*4u,rec-0x80140000u);
        PE_StoreU16(rec+6u,(uint16_t)(3+(i+(int)seed+8)%7));
        for(unsigned j=0;j<9;j++)for(unsigned k=0;k<3;k++)PE_StoreU16(rec+8u+j*8+k*2,(uint16_t)(i*7919+(int)j*12347+(int)k*29009+(int)seed*4711));
    }
    pe_addr_t entry=0x801EA378u+(uint32_t)((int32_t)(int16_t)variant*52);
    PE_StoreU8(entry,3);PE_StoreU8(entry+1u,4);
    for(unsigned i=0;i<6;i++)PE_StoreU32(0x1F800240u+i*4u,0xA5A50000u+i);
}
static void test_DAY1_transition_camera(void)
{
    TEST_RETAIL_DISC1("DAY1_transition_camera"); TEST_RETAIL_FIXUPS(RETAILFIX_transition_path);
    static const uint32_t shifts[]={0,1,15,16,31,32,255,0x10001};
    for(unsigned k=0;k<sizeof(DAY1_camera_cases)/sizeof(DAY1_camera_cases[0]);k++){
        unsigned seed=DAY1_camera_cases[k].seed,step=DAY1_camera_cases[k].step;
        if(!step)DAY1_camera_fixture(seed,DAY1_camera_cases[k].variant);
        if(step==0 || step==15)func_80195994(DAY1_camera_cases[k].variant,shifts[seed],shifts[7-seed],DAY1_camera_cases[k].time);
        else if(step==1 || step==14)func_80195E4C(seed&1?0xFFFFFFFFu:5u,123,456,DAY1_camera_cases[k].time);
        else func_80195D3C();
        uint64_t hash=hit_camera_hash(DAY1_camera_ranges,4);
        if(hash!=DAY1_camera_cases[k].hash)fprintf(stderr,"camera case%u step%u %016llX/%016llX\n",k,step,(unsigned long long)hash,(unsigned long long)DAY1_camera_cases[k].hash);
        ASSERT(hash==DAY1_camera_cases[k].hash && !PE_Port_ShouldStop(),"camera state differs from original history");
        for(unsigned i=0;i<6;i++)ASSERT(PE_LoadU32(0x1F800240u+i*4u)==0xA5A50000u+i,"camera temporary scratch escaped");
    }
    PASS();
}
static void test_DAY1_transition_camera_boundary(void)
{
    TEST_RETAIL_DISC1("DAY1_transition_camera_boundary"); TEST_RETAIL_FIXUPS(RETAILFIX_transition_path);
    for(unsigned k=0;k<sizeof(DAY1_camera_faults)/sizeof(DAY1_camera_faults[0]);k++){
        unsigned f=DAY1_camera_faults[k].fn,second=DAY1_camera_faults[k].second;
        DAY1_camera_fixture(0,0);PE_StoreU8(0x8019C040u,3);PE_StoreU8(0x8019C041u,255);PE_StoreU8(0x8019C042u,3);
        unsigned id=f==0?(second?4u:3u):(second?3u:4u);
        pe_addr_t rec=0x80140000u+PE_LoadU32(0x80140000u+id*4u);
        PE_StoreU16(rec+6u,2);
        if(f==0)func_80195994(0,1,1,256);
        else if(f==1)func_80195D3C();
        else func_80195E4C(3,0,0,256);
        uint64_t hash=hit_camera_hash(DAY1_camera_ranges,4);
        if(hash!=DAY1_camera_faults[k].hash)fprintf(stderr,"camera fault%u %016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)DAY1_camera_faults[k].hash);
        ASSERT(hash==DAY1_camera_faults[k].hash && PE_Port_ShouldStop(),"camera advanced past zero-period original prefix");
        for(unsigned i=0;i<6;i++)ASSERT(PE_LoadU32(0x1F800240u+i*4u)==0xA5A50000u+i,"camera boundary lost scratch restore");
    }
    PASS();
}
