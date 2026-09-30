#include "retail_spu_mode_cases.h"
static void test_DAY2_spu_mode(void)
{
    TEST_RETAIL_DISC1("DAY2_spu_mode"); TEST_RETAIL_FIXUPS(RETAILFIX_spu_mode);
    for(unsigned k=0;k<sizeof(SM_cases)/sizeof(SM_cases[0]);k++) {
        for(unsigned physical=0;physical<2;physical++) {
        ResetTestState();func_8007D15C();
        memset(PE_Translate(0x80160000u,0x40000u),0xA5,0x40000u);
        for(unsigned i=0;i<2;i++) {
            ASSERT(PE_SpuDma_Begin(0x80160000u,i*0x40000u,0x40000u,0),"prefill SPU RAM");
            ASSERT(PE_SpuDma_Service() && PE_Event_ConsumeSpuDma((int)PE_LoadU32(0x8009B384u)),"prefill completed");
        }
        for(unsigned i=0;i<sizeof(SM_ranges)/sizeof(SM_ranges[0]);i++)memset(PE_Translate(0x80000000u|SM_ranges[i][0],SM_ranges[i][1]),0,SM_ranges[i][1]);
        for(unsigned i=0;i<sizeof(SM_common)/sizeof(SM_common[0]);i++)PE_StoreU32(0x80000000u|SM_common[i][0],SM_common[i][1]);
        for(unsigned i=SM_cases[k].first;i<SM_cases[k].end;i++)PE_StoreU32(0x80000000u|SM_patches[i][0],SM_patches[i][1]);
        if(physical) {
            for(unsigned i=0;i<0x200u;i+=2)PE_SpuRegister_StoreU16(i,PE_LoadU16(0x80151000u+i));
            PE_StoreU32(0x8009B3FCu,0x1F801C00u);
        }
        func_8008CB54(SM_cases[k].mode);
        if(physical) {
            for(unsigned i=0;i<0x200u;i+=2)PE_StoreU16(0x80151000u+i,PE_SpuRegister_LoadU16(i));
            PE_StoreU32(0x8009B3FCu,0x80151000u);
        }
        uint64_t ram=hit_camera_hash(SM_ranges,sizeof(SM_ranges)/sizeof(SM_ranges[0])),spu=UINT64_C(14695981039346656037);
        for(unsigned i=0;i<PE_SPU_RAM_SIZE;i++){spu^=PE_SpuRam_LoadU8(i);spu*=UINT64_C(1099511628211);}
        if(ram!=SM_cases[k].ram || spu!=SM_cases[k].spu)fprintf(stderr,"SPU mode case%u RAM %016llX/%016llX SPU %016llX/%016llX\n",k,(unsigned long long)ram,(unsigned long long)SM_cases[k].ram,(unsigned long long)spu,(unsigned long long)SM_cases[k].spu);
        ASSERT(ram==SM_cases[k].ram && spu==SM_cases[k].spu,"mode graph matches original RAM and DMA result");
        PeSpuDmaState state;PE_SpuDma_GetState(&state);
        ASSERT(state.event_count==SM_cases[k].events+2u && !PE_Port_ShouldStop(),"mode waits completed through real DMA events");
        }
    }
    PASS();
}
