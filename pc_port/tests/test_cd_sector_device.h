/* Protocol/device integration against distinct bytes in a mounted raw image. */
static void CdSectorCommand(uint8_t command,const uint8_t *parameters,unsigned count)
{
    PE_CdReg_WriteU8(PE_CDREG_BASE,0u);
    for(unsigned i=0;i<count;i++) PE_CdReg_WriteU8(PE_CDREG_BASE+2u,parameters[i]);
    PE_CdReg_WriteU8(PE_CDREG_BASE+1u,command);
    PE_CdReg_ServiceDevice(0xC4E1u);
}
static unsigned CdSectorReply(uint8_t *response,unsigned count)
{
    PE_CdReg_WriteU8(PE_CDREG_BASE,1u);
    unsigned tag=PE_CdReg_ReadU8(PE_CDREG_BASE+3u)&7u;
    for(unsigned i=0;i<count;i++) response[i]=PE_CdReg_ReadU8(PE_CDREG_BASE+1u);
    PE_CdReg_WriteU8(PE_CDREG_BASE+3u,7u);
    return tag;
}
static void test_DAY2_cd_sector_device(void)
{
    TEST_RETAIL_DISC1("DAY2_cd_sector_device"); TEST_RETAIL_FIXUPS(RETAILFIX_cd_device_tables);
    DiscFixture fx={0};uint8_t response[5],raw[2352];
    ResetTestState();ASSERT(FxBuild(&fx,0),"sector fixture");PE_Disc_SetActive(fx.disc);
    /* Full raw contents, including the header and ECC tail, must survive. */
    for(unsigned lba=20u;lba<23u;lba++)
        for(unsigned i=0;i<2352u;i++) fx.img[lba*2352u+i]=(uint8_t)(lba*13u+i*7u+(i>>8u));
    ASSERT(PE_Disc_ReadRawSector(fx.disc,20u,raw) && !memcmp(raw,fx.img+20u*2352u,2352u),"raw sector bytes");
    ASSERT(!PE_Disc_ReadRawSector(fx.disc,PE_Disc_UserSectorCount(fx.disc),raw) && !PE_Disc_ReadRawSector(NULL,0,raw) && !PE_Disc_ReadRawSector(fx.disc,0,NULL),"raw sector bounds");
    /* End-to-end: public initialization, SDK command issue, CPU source2,
     * translated acknowledgment/data callback and actual FIFO consumption. */
    CdDeviceSeed();ASSERT(PE_CdReg_EnableDevice(7u),"SDK read attachment");
    ASSERT(func_8007EC14()==1 && !PE_Port_ShouldStop(),"SDK read startup");
    for(unsigned tick=0;tick<10u && PE_LoadU32(0x8009B574u)!=1u && !PE_Port_ShouldStop();tick++) HostFB_VSync(0);
    PE_StoreU32(0x80140000u,0x00200200u);
    ASSERT(func_8007FB44(2u,0x80140000u)==1,"SDK Setloc issue");
    for(unsigned poll=0;poll<2048u && PE_LoadU32(0x8009B598u) && !PE_Port_ShouldStop();poll++) HostFB_VSync(-1);
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x8009B574u)==1u,"SDK Setloc completion");
    ASSERT(func_8007FB44(27u,0u)==1,"SDK ReadS issue");
    for(unsigned poll=0;poll<2048u && !PE_LoadU32(0x800A3520u) && !PE_Port_ShouldStop();poll++) HostFB_VSync(-1);
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x800A3520u)==1u && PE_LoadU8(0x800A3524u)==1u && PE_LoadU8(0x800A3525u)==0x22u,"SDK data-ready publication");
    PE_CdReg_WriteU8(PE_CDREG_BASE,0u);PE_CdReg_WriteU8(PE_CDREG_BASE+3u,0u);PE_CdReg_WriteU8(PE_CDREG_BASE+3u,0x80u);
    for(unsigned i=0;i<2340u;i++) ASSERT(PE_CdReg_ReadU8(PE_CDREG_BASE+2u)==fx.img[20u*2352u+12u+i],"SDK sector FIFO bytes");
    for(unsigned mode_case=0;mode_case<2u;mode_case++) {
        PE_CdReg_Reset();ASSERT(PE_CdReg_EnableDevice(7u),"sector attachment");
        uint8_t mode=mode_case?0xA0u:0u,loc[]={0u,2u,0x20u};
        CdSectorCommand(14u,&mode,1);ASSERT(CdSectorReply(response,1)==3u,"sector mode acknowledgement");
        CdSectorCommand(2u,loc,3);ASSERT(CdSectorReply(response,1)==3u,"Setloc acknowledgement");
        /* Seek's completion is separate, and does not fabricate data. */
        CdSectorCommand(21u,NULL,0);ASSERT(CdSectorReply(response,1)==3u,"SeekL first response");
        PE_CdReg_ServiceDevice(0x100000u);ASSERT(CdSectorReply(response,1)==2u && response[0]==2u,"SeekL completion");
        CdSectorCommand(mode_case?27u:6u,NULL,0);
        ASSERT(CdSectorReply(response,1)==3u,"read acknowledgement");
        unsigned period=mode_case?225792u:451584u,size=mode_case?2340u:2048u,offset=mode_case?12u:24u;
        for(unsigned sector=0;sector<2u;sector++) {
            PE_CdReg_ServiceDevice(period-1u);
            ASSERT(!(PE_CdReg_ReadU8(PE_CDREG_BASE)&0x60u),"sector arrived early");
            PE_CdReg_ServiceDevice(1u);
            ASSERT(!(PE_CdReg_ReadU8(PE_CDREG_BASE)&0x40u),"FIFO available before BFRD");
            ASSERT(CdSectorReply(response,1)==1u && response[0]==0x22u,"data-ready response");
            PE_CdReg_WriteU8(PE_CDREG_BASE,0u);PE_CdReg_WriteU8(PE_CDREG_BASE+3u,0u);
            PE_CdReg_WriteU8(PE_CDREG_BASE+3u,0x80u);
            ASSERT(PE_CdReg_ReadU8(PE_CDREG_BASE)&0x40u,"BFRD did not expose FIFO");
            for(unsigned i=0;i<size;i++) ASSERT(PE_CdReg_ReadU8(PE_CDREG_BASE+2u)==fx.img[(20u+sector)*2352u+offset+i],"sector FIFO byte mismatch");
            ASSERT(!(PE_CdReg_ReadU8(PE_CDREG_BASE)&0x40u),"FIFO request stayed active after drain");
        }
        CdSectorCommand(9u,NULL,0);ASSERT(CdSectorReply(response,1)==3u && response[0]==0x22u,"Pause first response");
        PE_CdReg_ServiceDevice(0x100000u);ASSERT(CdSectorReply(response,1)==2u && response[0]==2u,"Pause completion");
        PE_CdReg_ServiceDevice(period*3u);
        PeCdDeviceState state;PE_CdReg_GetDeviceState(&state);
        ASSERT(!state.reading && state.sectors==2u && state.next_lba==22u && !state.data_remaining && !PE_Port_ShouldStop(),"sequential read/pause state");
        loc[2]=0x7Au;CdSectorCommand(2u,loc,3);
        ASSERT(CdSectorReply(response,2)==5u && response[0]==3u && response[1]==0x10u,"invalid BCD must report parameter value error");
    }
    /* DAY2-158 B0CD0 catchup: func_8007C564 defers with D_800B0CD0 set and
     * no BFRD.  Hold the unread sector across the next cadence. */
    PE_CdReg_Reset();ASSERT(PE_CdReg_EnableDevice(7u),"catchup attachment");
    uint8_t loc[]={0u,2u,0x20u};CdSectorCommand(2u,loc,3);(void)CdSectorReply(response,1);
    CdSectorCommand(6u,NULL,0);(void)CdSectorReply(response,1);
    PE_CdReg_ServiceDevice(451584u);ASSERT(CdSectorReply(response,1)==1u,"catchup first data response");
    PE_StoreU16(0x800B0CD0u,1u);
    PE_CdReg_ServiceDevice(451584u);
    {
        PeCdDeviceState held;
        PE_CdReg_GetDeviceState(&held);
        ASSERT(!PE_Port_ShouldStop() && held.sectors==1u,
            "B0CD0 catchup must hold the unread sector, not overrun");
    }
    /* Unread data is retained; no silent sector substitution on overrun. */
    PE_StoreU16(0x800B0CD0u,0u);
    PE_CdReg_Reset();ASSERT(PE_CdReg_EnableDevice(7u),"overrun attachment");
    CdSectorCommand(2u,loc,3);(void)CdSectorReply(response,1);
    CdSectorCommand(6u,NULL,0);(void)CdSectorReply(response,1);
    PE_CdReg_ServiceDevice(451584u);ASSERT(CdSectorReply(response,1)==1u,"overrun first data response");
    PE_CdReg_ServiceDevice(451584u);
    ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_device_sector_overrun")==1,"unimplemented multi-sector buffering must stop");
    PE_CdReg_Reset();PE_Disc_SetActive(NULL);FxFree(&fx);PASS();
}
