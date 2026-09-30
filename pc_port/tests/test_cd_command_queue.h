#include "retail_cd_command_queue_cases.h"
static void test_DAY2_cd_command_queue(void)
{
    TEST_RETAIL_DISC1("DAY2_cd_command_queue"); TEST_RETAIL_FIXUPS(RETAILFIX_cd_device_tables);
    for(unsigned k=0;k<sizeof(CDQUEUE_issue)/sizeof(CDQUEUE_issue[0]);k++) {
        ResetTestState();
        for(unsigned j=0;j<sizeof(CDQUEUE_issue_ranges)/sizeof(CDQUEUE_issue_ranges[0]);j++)
            for(unsigned i=0;i<CDQUEUE_issue_ranges[j][1];i++) PE_StoreU8(0x80000000u+CDQUEUE_issue_ranges[j][0]+i,(uint8_t)(37u+i*17u));
        PE_StoreU32(0x8009B4BCu+9u*4u,CDQUEUE_issue[k].prefix);PE_StoreU32(0x8009B53Cu,CDQUEUE_issue[k].sequence);
        PE_StoreU32(0x8009B574u,CDQUEUE_issue[k].lane);PE_StoreU32(0x800A3600u,CDQUEUE_issue[k].head);
        PE_StoreU32(0x800A3604u,CDQUEUE_issue[k].head);PE_StoreU32(0x800A3608u,CDQUEUE_issue[k].count);
        ASSERT(func_8007EE84(0x109u,CDQUEUE_issue[k].param,0x12345678u,0x89ABCDEFu)==CDQUEUE_issue[k].result &&
            hit_camera_hash(CDQUEUE_issue_ranges,sizeof(CDQUEUE_issue_ranges)/sizeof(CDQUEUE_issue_ranges[0]))==CDQUEUE_issue[k].hash && !PE_Port_ShouldStop(),"command queue differs from original graph");
    }
    for(unsigned k=0;k<sizeof(CDQUEUE_poll)/sizeof(CDQUEUE_poll[0]);k++) {
        ResetTestState();HostFB_Init();
        for(unsigned j=0;j<sizeof(CDQUEUE_poll_ranges)/sizeof(CDQUEUE_poll_ranges[0]);j++)
            for(unsigned i=0;i<CDQUEUE_poll_ranges[j][1];i++) PE_StoreU8(0x80000000u+CDQUEUE_poll_ranges[j][0]+i,(uint8_t)(37u+i*17u));
        PE_StoreU32(0x800A3690u,CDQUEUE_poll[k].head);
        for(unsigned i=0;i<8u;i++) {
            PE_StoreU32(0x800A3610u+i*16u,10u+i);PE_StoreU8(0x800A3614u+i*16u,(uint8_t)CDQUEUE_poll[k].status);
        }
        PE_StoreU32(0x800A3610u+CDQUEUE_poll[k].match*16u,42u);
        ASSERT((uint32_t)func_8007F418(CDQUEUE_poll[k].sequence,CDQUEUE_poll[k].response)==CDQUEUE_poll[k].result &&
            hit_camera_hash(CDQUEUE_poll_ranges,sizeof(CDQUEUE_poll_ranges)/sizeof(CDQUEUE_poll_ranges[0]))==CDQUEUE_poll[k].hash && !PE_Port_ShouldStop(),"command response ring differs from original graph");
    }
    DiscFixture fx={0};ResetTestState();ASSERT(FxBuild(&fx,0),"command queue disc fixture");PE_Disc_SetActive(fx.disc);
    CdDeviceSeed();ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14()==1,"queued command startup");
    for(unsigned tick=0;tick<10u && PE_LoadU32(0x8009B574u)!=1u && !PE_Port_ShouldStop();tick++) HostFB_VSync(0);
    PE_StoreU32(0x80140000u,0x00200200u);
    memset(PE_Translate(0x80140010u,12u),0xCD,12u);
    ASSERT(func_80080D5C(2,0x80140000u,0x80140010u)==1 && !PE_Port_ShouldStop(),"queued Setloc with response did not complete");
    ASSERT(PE_LoadU8(0x80140010u)==2u && PE_LoadU8(0x80140018u)==0xCDu && !PE_LoadU32(0x800A3608u),"Setloc response or queue retirement");
    ASSERT(func_80080DC4(9,0u,0x80140010u)==1 && !PE_Port_ShouldStop(),"queued Pause did not complete");
    ASSERT(PE_LoadU8(0x80140010u)==2u && !PE_LoadU32(0x800A3608u) && PE_LoadU32(0x8009B574u)==1u,"Pause response or queue retirement");
    PE_StoreU32(0x80140000u,0x0020AA00u);
    ASSERT(func_80080D5C(2,0x80140000u,0x80140010u)==0 && !PE_Port_ShouldStop(),"invalid Setloc must return actual command failure");
    ASSERT(PE_LoadU8(0x80140010u)==3u && PE_LoadU8(0x80140011u)==0x10u &&
        !PE_LoadU32(0x800A3608u),"invalid Setloc response or queue retirement");
    FxFree(&fx);PASS();
}
