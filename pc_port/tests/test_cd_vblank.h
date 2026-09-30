#include "retail_cd_vblank_cases.h"
static void test_DAY2_cd_vblank(void)
{
    TEST_RETAIL_DISC1("DAY2_cd_vblank"); TEST_RETAIL_FIXUPS(RETAILFIX_cd_vblank);
    for(unsigned k=0;k<sizeof(CDVB_cases)/sizeof(CDVB_cases[0]);k++) {
        ResetTestState();HostFB_Init();PE_GPU_Init();B558_PlantPointers();PE_Callback_Init();
        for(unsigned j=0;j<5;j++) for(unsigned i=0;i<CDVB_ranges[j][1];i++) PE_StoreU8(0x80000000u+CDVB_ranges[j][0]+i,0u);
        PE_StoreU32(0x8009B574u,CDVB_cases[k].lane);PE_StoreU32(0x8009B578u,CDVB_cases[k].state);
        PE_StoreU32(0x8009B57Cu,CDVB_cases[k].phase);PE_StoreU32(0x8009B598u,CDVB_cases[k].pending);
        PE_StoreU32(0x8009B594u,CDVB_cases[k].flag);PE_StoreU32(0x8009B5A0u,2u);
        PE_StoreU32(0x8009B58Cu,0xFFFFFFFFu);PE_StoreU32(0x8009B59Cu,0xFFFFFFFFu);
        PE_StoreU8(0x8009B588u,(uint8_t)CDVB_cases[k].flag);PE_StoreU8(0x8009B58Au,(uint8_t)CDVB_cases[k].flag);PE_StoreU8(0x8009B58Bu,(uint8_t)CDVB_cases[k].flag);
        PE_StoreU32(0x8009B6A4u,CDVB_cases[k].flag);PE_StoreU32(0x8009B554u,CDVB_cases[k].cb==2u?0u:1u);
        PE_StoreU32(0x800A36A0u,CDVB_cases[k].cb==0u?0u:CDVB_cases[k].cb>=3u?0x8007F7E8u:0x80170000u);
        PE_StoreU32(0x800A3608u,CDVB_cases[k].cb==3u?1u:0u);PE_StoreU32(0x800A3604u,7u);
        PE_StoreU32(0x800A35E8u,11u);PE_StoreU8(0x800A35ECu,14u);PE_StoreU32(0x800A35F4u,0x80140000u);
        PE_StoreU8(0x8009B558u,14u);PE_StoreU32(0x8009B560u,0x80140000u);PE_StoreU32(0x80140000u,0x87654321u);
        PE_StoreU32(0x801FFE90u,0x12345678u);PE_StoreU32(0x8009AFC0u,0u);
        for(unsigned i=0;i<32;i++) {
            PE_StoreU32(0x8009B5A4u+i*4u,CDVB_cases[k].flag);
            PE_StoreU32(0x8009B1FCu+i*4u,CDVB_params[i]);PE_StoreU32(0x8009B0FCu+i*4u,CDVB_clear[i]);
        }
        PE_StoreU16(0x800945E6u,0u);
        /* Real registered VBlank-slot dispatch; wrap yields the oracle's0 query. */
        PE_StoreU32(0x800956ACu,0xFFFFFFFFu);
        ASSERT(PE_Callback_Bind(0x8007FE24u,func_8007FE24)==0,"bind CD VBlank updater");
        (void)func_80073D58(0u,0x8007FE24u);
        int completed=PE_Callback_DispatchChecked();
        for(unsigned i=0;i<4;i++) PE_StoreU8(0x80130000u+i,PE_CdReg_ReadU8(0x1F801800u+i));
        PE_StoreU32(0x80130004u,PE_CdReg_ReadU32(0x1F801020u));
        uint64_t hash=hit_camera_hash(CDVB_ranges,5);
        if(hash!=CDVB_cases[k].hash) fprintf(stderr,"CD VBlank case%u state%u phase%u pending%u hash%016llX/%016llX\n",k,CDVB_cases[k].state,CDVB_cases[k].phase,CDVB_cases[k].pending,(unsigned long long)hash,(unsigned long long)CDVB_cases[k].hash);
        ASSERT(hash==CDVB_cases[k].hash,"CD VBlank differs from original");
        ASSERT(PE_LoadU32(0x800956ACu)==0u && completed==!CDVB_cases[k].boundary,"CD VBlank dispatch completion");
        if(CDVB_cases[k].boundary) ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_vblank_callback")==1 && g_bootstrap_arg4_calls[0].target==CDVB_cases[k].boundary,"CD VBlank nested boundary");
        else ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"CD VBlank unexpected boundary");
    }
    PASS();
}
