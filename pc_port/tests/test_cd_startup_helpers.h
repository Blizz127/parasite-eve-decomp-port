#include "retail_cd_startup_helpers_cases.h"
static void test_DAY2_cd_startup_helpers(void)
{
    TEST("DAY2_cd_startup_helpers");
    for(unsigned k=0;k<sizeof(CDAUDIO_cases)/sizeof(CDAUDIO_cases[0]);k++) for(unsigned mapping=0;mapping<4;mapping++) {
        static const pe_addr_t bases[]={0x80130000u,0x1F801C00u,0x9F801C00u,0xBF801C00u};
        ResetTestState();B558_PlantPointers();
        for(unsigned i=0;i<0x200u;i++) PE_StoreU8(0x80130000u+i,(uint8_t)(CDAUDIO_cases[k].seed+i*17u));
        PE_StoreU16(0x801301B8u,(uint16_t)CDAUDIO_cases[k].left);PE_StoreU16(0x801301BAu,(uint16_t)CDAUDIO_cases[k].right);
        if(mapping) for(unsigned i=0;i<0x200u;i+=2) PE_SpuRegister_StoreU16(i,PE_LoadU16(0x80130000u+i));
        PE_StoreU32(0x8009B290u,bases[mapping]);
        for(unsigned i=0;i<4;i++) PE_CdReg_WriteU8(0x1F801800u+i,(uint8_t)(CDAUDIO_cases[k].seed+i*17u));
        ASSERT(func_8007BAC0()==0,"CD audio setup return");
        if(mapping) for(unsigned i=0;i<0x200u;i+=2) PE_StoreU16(0x80130000u+i,PE_SpuRegister_LoadU16(i));
        PE_StoreU32(0x80131000u,PE_CdReg_ReadU32(0x1F801800u));
        ASSERT(hit_camera_hash(CDAUDIO_ranges,2)==CDAUDIO_cases[k].hash,"CD audio setup differs from original");
        uint8_t gain[4];PE_CdReg_GetAudioVolumes(gain);
        ASSERT(gain[0]==0x80u && gain[1]==0u && gain[2]==0x80u && gain[3]==0u,"CD stereo matrix was not applied");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"CD audio setup unexpected boundary");
    }
    for(unsigned k=0;k<sizeof(CDSLOT_cases)/sizeof(CDSLOT_cases[0]);k++) {
        static const uint32_t ranges[][2]={{0x9568Cu,36u}};
        ResetTestState();PE_Callback_Init();
        for(unsigned i=0;i<36;i++) PE_StoreU8(0x8009568Cu+i,(uint8_t)(37u+i*17u));
        PE_StoreU32(0x8009568Cu+CDSLOT_cases[k].slot*4u,CDSLOT_cases[k].old);
        ASSERT(func_80073D58(CDSLOT_cases[k].slot,CDSLOT_cases[k].next)==CDSLOT_cases[k].result && hit_camera_hash(ranges,1)==CDSLOT_cases[k].hash,"VBlank wrapper differs from original");
    }
    for(unsigned k=0;k<sizeof(CDMODE_cases)/sizeof(CDMODE_cases[0]);k++) {
        ResetTestState();PE_StoreU32(0x8009B6B8u,CDMODE_cases[k].old);func_800812F4(CDMODE_cases[k].mode);
        ASSERT(PE_LoadU32(0x8009B6B8u)==CDMODE_cases[k].result,"CD mode setter differs from original");
    }
    /* Device contract: all pending gains apply atomically; other banks cannot
     * apply them or acknowledge a queued response through volume writes. */
    ResetTestState();
    { uint8_t gain[4],response=2u;
      ASSERT(PE_CdReg_PushResponse(3u,&response,1u),"audio test response ingress");
      PE_CdReg_WriteU8(0x1F801800u,2u);PE_CdReg_WriteU8(0x1F801802u,17u);PE_CdReg_WriteU8(0x1F801803u,34u);
      PE_CdReg_WriteU8(0x1F801800u,3u);PE_CdReg_WriteU8(0x1F801801u,51u);PE_CdReg_WriteU8(0x1F801802u,68u);
      PE_CdReg_WriteU8(0x1F801803u,0u);PE_CdReg_GetAudioVolumes(gain);
      ASSERT(!(gain[0]|gain[1]|gain[2]|gain[3]),"volumes applied before apply bit");
      PE_CdReg_WriteU8(0x1F801803u,0x20u);PE_CdReg_GetAudioVolumes(gain);
      ASSERT(gain[0]==17u && gain[1]==34u && gain[2]==51u && gain[3]==68u,"banked audio gain application");
      PE_CdReg_WriteU8(0x1F801800u,1u);
      ASSERT((PE_CdReg_ReadU8(0x1F801803u)&7u)==3u && PE_CdReg_ReadU8(0x1F801801u)==2u,"volume writes consumed CD response");
      PE_CdReg_Reset();PE_CdReg_GetAudioVolumes(gain);
      ASSERT(!(gain[0]|gain[1]|gain[2]|gain[3]),"CD reset retained active volumes"); }
    PASS();
}
