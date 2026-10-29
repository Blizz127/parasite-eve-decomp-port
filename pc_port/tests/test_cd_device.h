#include "retail_cd_device_tables.h"
#include "retail_cd_startup_notify_cases.h"
static void CdDeviceSeed(void)
{
    HostFB_Init();PE_GPU_Init();PE_Callback_Init();B558_PlantPointers();
    PE_StoreU32(0x8009B290u,0x1F801C00u);
    for(unsigned j=0;j<6;j++) for(unsigned i=0;i<32;i++) PE_StoreU32(0x80000000u+CDDEV_tables[j].address+i*4u,CDDEV_tables[j].values[i]);
    for(unsigned i=0;i<26;i++) PE_StoreU32(0x80011D0Cu+i*4u,CDDEV_command_jumps[i]);
    for(unsigned i=0;i<5;i++) PE_StoreU32(0x80011B8Cu+i*4u,CDACK_jumps[i]);
}

/* Store-handler and DMA register pointers the stream state machine reaches
 * through the retail .rodata constants (asm/disc1/data/818A0.rodata.s).
 * CdDeviceSeed()/B558_PlantPointers() already planted the B27C command
 * family; these are the data/BFRD/bus/index/mailbox/DMA family the 7C564
 * assembly and the 7CEAC DMA3 issue use. Plain pointer values, not planted
 * guest state. Shared by the movie player and movie updater fixtures. */
static void CdStreamSeedRegisters(void)
{
    PE_StoreU32(0x8009B34Cu,0x1F801098u);   /* MDEC DMA1 CHCR (busy check) */
    PE_StoreU32(0x8009B32Cu,0x1F801800u);   /* index/status   */
    PE_StoreU32(0x8009B334u,0x1F801802u);   /* data FIFO      */
    PE_StoreU32(0x8009B338u,0x1F801803u);   /* BFRD / IRQ     */
    PE_StoreU32(0x8009B33Cu,0x1F801018u);   /* bus control    */
    PE_StoreU32(0x8009B340u,0x1F801020u);   /* result mailbox */
    PE_StoreU32(0x8009B344u,0x1F8010F0u);   /* DPCR           */
    PE_StoreU32(0x8009B348u,0x1F8010F4u);   /* DICR           */
    PE_StoreU32(0x8009B35Cu,0x1F8010B8u);   /* DMA3 CHCR      */
}

/* Write the retail Form-1 video chunk header (raw+24: chunk magic 0x0160,
 * 0x8001 channel selector, chunk index, total chunks, frame word) into one
 * fixture sector. payload 0 leaves the rest of the sector zero (a benign
 * decoder input); payload 1 fills MOVAU's deterministic pattern so the
 * assembled 2016-byte slice can be byte-compared with the on-disc bytes. */
static void CdStreamWriteVideoSector(uint8_t *img,uint32_t lba,uint32_t chunk,
                                    uint32_t chunks,uint32_t frame,int payload)
{
    uint8_t *raw=img+(size_t)lba*PE_DISC_RAW_SECTOR;
    for(unsigned i=0;i<PE_DISC_RAW_SECTOR;i++) raw[i]=0u;
    raw[16]=0u;raw[17]=0u;raw[18]=0x48u;raw[19]=0u;
    raw[20]=0u;raw[21]=0u;raw[22]=0x48u;raw[23]=0u;
    raw[24]=0x60u;raw[25]=0x01u;      /* chunk magic 0x0160 */
    raw[26]=0x01u;raw[27]=0x80u;      /* 0x8001: channel selector low bits 0 */
    raw[28]=(uint8_t)chunk;raw[29]=0u;
    raw[30]=(uint8_t)chunks;raw[31]=0u;
    raw[32]=(uint8_t)frame;raw[33]=(uint8_t)(frame>>8);
    raw[34]=(uint8_t)(frame>>16);raw[35]=(uint8_t)(frame>>24);
    if(payload) for(unsigned i=36u;i<PE_DISC_USER_SECTOR;i++)
        raw[24u+i]=(uint8_t)(i*13u+5u);
}
static void test_DAY2_cd_device(void)
{
    TEST_RETAIL_DISC1("DAY2_cd_device"); TEST_RETAIL_FIXUPS(RETAILFIX_cd_device_tables);
    for(unsigned k=0;k<sizeof(CDNOTIFY_cases)/sizeof(CDNOTIFY_cases[0]);k++) {
        ResetTestState();PE_StoreU32(0x800B8AB8u,CDNOTIFY_cases[k].target);
        func_8007F960(CDNOTIFY_cases[k].status,CDNOTIFY_cases[k].response);
        if(CDNOTIFY_cases[k].target) {
            ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_data_callback")==1 &&
                g_bootstrap_arg4_calls[0].target==CDNOTIFY_cases[k].target &&
                g_bootstrap_arg4_calls[0].arg0==CDNOTIFY_cases[k].arg0 &&
                g_bootstrap_arg4_calls[0].arg1==CDNOTIFY_cases[k].arg1,"startup notification differs from original callback prefix");
        } else ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"null startup notification must return");
    }
    DiscFixture fx={0};PeCdDeviceState device;
    ResetTestState();PE_Disc_SetActive(NULL);
    ASSERT(!PE_CdReg_EnableDevice(7u),"CD device accepted absent disc");
    ASSERT(FxBuild(&fx,0),"CD device fixture");PE_Disc_SetActive(fx.disc);
    CdDeviceSeed();ASSERT(PE_CdReg_EnableDevice(7u),"CD device attachment");
    ASSERT(func_8007EC14()==1 && !PE_Port_ShouldStop(),"real CD startup did not complete");
    PE_CdReg_GetDeviceState(&device);
    ASSERT(device.commands==3u && device.responses==4u && device.command_log[0]==1u && device.command_log[1]==10u && device.command_log[2]==12u,"startup command sequence");
    ASSERT(device.response_log[0]==3u && device.response_log[1]==3u && device.response_log[2]==2u && device.response_log[3]==3u,"startup response ordering");
    ASSERT(PE_LoadU8(0x8009B294u)==2u && PE_LoadU32(0x800945F0u)==0x8007C13Cu && PE_LoadU32(0x8009568Cu)==0x8007FE24u,"startup callback/status publication");
    ASSERT(PE_LoadU32(0x800A36ACu)==0x8007F960u && PE_LoadU32(0x800A36A0u)==0x8007F7E8u,"public CD handlers");
    /* A masked VBlank advances time but cannot invoke the SDK updater. */
    uint16_t mask=PE_IRQ_GetMask();
    (void)PE_IRQ_ExchangeMask((uint16_t)(mask&~1u));
    uint32_t vblanks=PE_LoadU32(0x800956ACu);
    HostFB_VSync(0);PE_CdReg_GetDeviceState(&device);
    ASSERT(device.commands==3u && PE_LoadU32(0x800956ACu)==vblanks && (PE_IRQ_ReadStatus()&1u),"masked VBlank dispatched startup");
    (void)PE_IRQ_ExchangeMask(mask);
    for(unsigned tick=0;tick<10u && PE_LoadU32(0x8009B574u)!=1u && !PE_Port_ShouldStop();tick++) HostFB_VSync(0);
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x800956ACu)>vblanks,"host VBlank startup did not dispatch");
    ASSERT(PE_LoadU32(0x8009B574u)==1u && PE_LoadU32(0x8009B578u)==11u,"CD startup state machine did not become ready");
    PE_CdReg_GetDeviceState(&device);
    ASSERT(device.commands==7u && device.command_log[3]==1u && device.command_log[4]==1u && device.command_log[5]==19u && device.command_log[6]==1u,"startup VBlank command progression");
    /* Transport IRQ masking and Init's held second response. */
    PE_CdReg_Reset();(void)PE_IRQ_ExchangeMask(0u);PE_IRQ_WriteStatus(0u);
    ASSERT(PE_CdReg_EnableDevice(0u),"device masked attachment");
    PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801801u,10u);
    PE_CdReg_ServiceDevice(0x13CCDu);
    ASSERT(!(PE_CdReg_ReadU8(0x1F801800u)&0x20u),"command responded before scheduled cycle");
    PE_CdReg_ServiceDevice(1u);ASSERT(!(PE_IRQ_ReadStatus()&4u),"CD IRQ ignored device mask");
    PE_CdReg_WriteU8(0x1F801800u,1u);ASSERT((PE_CdReg_ReadU8(0x1F801803u)&7u)==3u,"Init first tag");
    PE_CdReg_WriteU8(0x1F801802u,7u);PE_CdReg_ServiceDevice(0u);
    ASSERT(PE_IRQ_ReadStatus()&4u,"pending CD response did not assert after enabling mask");
    PE_CdReg_ServiceDevice(0x200000u);PE_CdReg_GetDeviceState(&device);
    ASSERT(device.responses==1u,"second response overwrote unacknowledged first");
    ASSERT(PE_CdReg_ReadU8(0x1F801801u)==2u,"Init first status byte");
    PE_CdReg_WriteU8(0x1F801803u,7u);PE_CdReg_ServiceDevice(0u);
    ASSERT((PE_CdReg_ReadU8(0x1F801803u)&7u)==2u && PE_CdReg_ReadU8(0x1F801801u)==2u,"Init completion tag/status");
    PE_CdReg_WriteU8(0x1F801803u,7u);
    /* SetMode and Getparam transfer parameters through actual FIFOs. */
    PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801802u,0xA0u);PE_CdReg_WriteU8(0x1F801801u,14u);PE_CdReg_ServiceDevice(0xC4E1u);
    PE_CdReg_WriteU8(0x1F801800u,1u);PE_CdReg_WriteU8(0x1F801803u,7u);
    PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801801u,15u);PE_CdReg_ServiceDevice(0xC4E1u);
    ASSERT(PE_CdReg_ReadU8(0x1F801801u)==2u && PE_CdReg_ReadU8(0x1F801801u)==0xA0u,"mode parameter round trip");
    PE_CdReg_WriteU8(0x1F801800u,1u);PE_CdReg_WriteU8(0x1F801803u,7u);
    /* Bad parameter counts are device errors, unported commands are boundaries. */
    PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801801u,14u);PE_CdReg_ServiceDevice(0xC4E1u);
    PE_CdReg_WriteU8(0x1F801800u,1u);
    ASSERT((PE_CdReg_ReadU8(0x1F801803u)&7u)==5u && PE_CdReg_ReadU8(0x1F801801u)==3u && PE_CdReg_ReadU8(0x1F801801u)==0x20u,"bad parameter error response");
    PE_CdReg_WriteU8(0x1F801803u,7u);PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801801u,3u);
    ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_device_unported_command")==1,"unported Play must stop");
    PE_CdReg_Reset();PE_Disc_SetActive(NULL);FxFree(&fx);PASS();
}

/* CD-XA device commands (psx-spx CDROM): Setfilter 0Dh,file,channel ->
 * INT3(stat); Stop 08h -> INT3(stat, bit5 clear) -> INT2(stat, bit1 clear);
 * GetlocP 11h -> INT3 with 8 BCD bytes and no stat byte; with XA-ADPCM or
 * the XA filter on, "INT1 is generated only for non-ADPCM sectors". */
static void CdxaWriteSector(uint8_t *img,uint32_t lba,uint8_t file,uint8_t chan,uint8_t submode)
{
    uint8_t *raw=img+(size_t)lba*PE_DISC_RAW_SECTOR;
    for(unsigned i=0;i<PE_DISC_RAW_SECTOR;i++) raw[i]=0u;
    for(unsigned i=1;i<11u;i++) raw[i]=0xFFu;
    raw[15]=2u;
    raw[16]=raw[20]=file;raw[17]=raw[21]=chan;raw[18]=raw[22]=submode;raw[19]=raw[23]=1u;
}
static void CdxaCommand(uint8_t command,const uint8_t *params,unsigned n)
{
    PE_CdReg_WriteU8(0x1F801800u,0u);
    for(unsigned i=0;i<n;i++) PE_CdReg_WriteU8(0x1F801802u,params[i]);
    PE_CdReg_WriteU8(0x1F801801u,command);
}
static unsigned CdxaTag(void) {PE_CdReg_WriteU8(0x1F801800u,1u);return PE_CdReg_ReadU8(0x1F801803u)&7u;}
static void CdxaAck(void) {PE_CdReg_WriteU8(0x1F801800u,1u);PE_CdReg_WriteU8(0x1F801803u,7u);}
static uint8_t CdxaByte(void) {return PE_CdReg_ReadU8(0x1F801801u);}

static void test_CDXA_device_commands(void)
{
    static const uint8_t mode_xa[1]={0xC8u},mode_plain[1]={0x00u},filter[2]={1u,0u},one[1]={1u};
    static const uint8_t loc50[3]={0x00u,0x02u,0x50u};   /* LBA 50 = 00:02:50 */
    DiscFixture fx={0};PeCdDeviceState device;uint8_t b[8];
    TEST("CDXA_device_commands");
    ResetTestState();
    ASSERT(FxBuild(&fx,0),"fixture");
    CdxaWriteSector(fx.img,50u,1u,0u,0x64u);   /* audio+real-time+form2, ch 0 */
    CdxaWriteSector(fx.img,51u,1u,1u,0x64u);   /* the other channel */
    CdxaWriteSector(fx.img,52u,1u,0u,0x00u);   /* null (data) sector */
    PE_Disc_SetActive(fx.disc);PE_CdReg_Reset();(void)PE_IRQ_ExchangeMask(0u);
    ASSERT(PE_CdReg_EnableDevice(0u),"device attachment");
    CdxaCommand(14u,mode_xa,1u);PE_CdReg_ServiceDevice(0xC4E1u);
    ASSERT(CdxaTag()==3u,"Setmode 0xC8");CdxaAck();
    CdxaCommand(13u,one,1u);PE_CdReg_ServiceDevice(0xC4E1u);
    ASSERT(CdxaTag()==5u && (CdxaByte()&1u) && CdxaByte()==0x20u,"Setfilter with one parameter is an error");CdxaAck();
    CdxaCommand(13u,filter,2u);PE_CdReg_ServiceDevice(0xC4E1u);
    ASSERT(CdxaTag()==3u && CdxaByte()==2u && !PE_Port_ShouldStop(),"Setfilter file 1 channel 0 -> INT3(stat)");CdxaAck();
    CdxaCommand(2u,loc50,3u);PE_CdReg_ServiceDevice(0xC4E1u);ASSERT(CdxaTag()==3u,"Setloc");CdxaAck();
    CdxaCommand(27u,NULL,0u);PE_CdReg_ServiceDevice(0xC4E1u);ASSERT(CdxaTag()==3u,"ReadS");CdxaAck();
    PE_CdReg_ServiceDevice(225792u);PE_CdReg_GetDeviceState(&device);
    ASSERT(CdxaTag()==0u && device.xa_sectors==1u && device.next_lba==51u,"matching ADPCM sector goes to the SPU, no INT1");
    PE_CdReg_ServiceDevice(225792u);PE_CdReg_GetDeviceState(&device);
    ASSERT(CdxaTag()==0u && device.xa_sectors==2u && device.next_lba==52u,"filtered-out ADPCM sector is dropped, no INT1");
    PE_CdReg_ServiceDevice(225792u);PE_CdReg_GetDeviceState(&device);
    ASSERT(CdxaTag()==1u && device.xa_sectors==2u && device.next_lba==53u,"null sector is data: INT1");CdxaAck();
    CdxaCommand(17u,NULL,0u);PE_CdReg_ServiceDevice(0xC4E1u);
    ASSERT(CdxaTag()==3u,"GetlocP INT3");
    for(unsigned i=0;i<8u;i++) b[i]=CdxaByte();
    ASSERT(b[0]==1u && b[1]==1u && b[2]==0u && b[3]==0u && b[4]==0x52u &&
           b[5]==0u && b[6]==2u && b[7]==0x52u,"GetlocP: track 1 index 1, 00:00:52 in track, 00:02:52 on disc");CdxaAck();
    CdxaCommand(8u,NULL,0u);PE_CdReg_ServiceDevice(0xC4E1u);
    ASSERT(CdxaTag()==3u && CdxaByte()==2u,"Stop INT3: motor on, bit5 already clear");CdxaAck();
    PE_CdReg_ServiceDevice(PE_CdReg_TimeSlice(0x2000000u));PE_CdReg_GetDeviceState(&device);
    ASSERT(CdxaTag()==2u && CdxaByte()==0u && device.motor_off && !device.reading,"Stop INT2: motor off");CdxaAck();
    CdxaCommand(17u,NULL,0u);PE_CdReg_ServiceDevice(0xC4E1u);
    ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_device_getlocp_spun_down")==1,"GetlocP on a stopped disc is not invented");
    /* XA-ADPCM and the filter off: the same audio sector is plain data. */
    ResetTestState();PE_Disc_SetActive(fx.disc);PE_CdReg_Reset();(void)PE_IRQ_ExchangeMask(0u);
    ASSERT(PE_CdReg_EnableDevice(0u),"second attachment");
    CdxaCommand(14u,mode_plain,1u);PE_CdReg_ServiceDevice(0xC4E1u);CdxaAck();
    CdxaCommand(2u,loc50,3u);PE_CdReg_ServiceDevice(0xC4E1u);CdxaAck();
    CdxaCommand(6u,NULL,0u);PE_CdReg_ServiceDevice(0xC4E1u);CdxaAck();
    PE_CdReg_ServiceDevice(451584u);PE_CdReg_GetDeviceState(&device);
    ASSERT(CdxaTag()==1u && device.xa_sectors==0u,"with mode 0 the ADPCM sector raises INT1");
    PE_CdReg_Reset();PE_Disc_SetActive(NULL);FxFree(&fx);PASS();
}

/* Lead-out is a transport completion, not a failed image read. Exercise both
 * read commands, raw XA routing and the final data sector's unread FIFO. */
static void test_CDXA_end_of_disc(void)
{
    static const uint8_t loc[3]={0x00u,0x02u,0x63u}; /* last of 64 sectors */
    DiscFixture fx={0};PeCdDeviceState d;
    TEST("CDXA_end_of_disc");
    ASSERT(FxBuild(&fx,0),"fixture");
    for(unsigned xa=0;xa<2u;xa++) for(unsigned cmd=6u;cmd<=27u;cmd+=21u) {
        uint8_t mode=xa?0xC8u:0u;
        ResetTestState();PE_Disc_SetActive(fx.disc);(void)PE_IRQ_ExchangeMask(0u);
        CdxaWriteSector(fx.img,63u,1u,0u,xa?0x64u:0x08u);
        fx.img[63u*PE_DISC_RAW_SECTOR+24u]=0xA5u;
        ASSERT(PE_CdReg_EnableDevice(4u),"attach");
        CdxaCommand(14u,&mode,1u);PE_CdReg_ServiceDevice(0xC4E1u);CdxaAck();
        CdxaCommand(2u,loc,3u);PE_CdReg_ServiceDevice(0xC4E1u);CdxaAck();
        CdxaCommand((uint8_t)cmd,NULL,0u);PE_CdReg_ServiceDevice(0xC4E1u);
        PE_CdReg_ServiceDevice(0x100000u);
        ASSERT(CdxaTag()==3u,"read ack must not be overwritten");CdxaAck();
        PE_CdReg_ServiceDevice(0x100000u);
        if(!xa) {
            ASSERT(CdxaTag()==1u && CdxaByte()==0x22u,"last sector is INT1 while reading");
            PE_CdReg_ServiceDevice(0x100000u);
            ASSERT(CdxaTag()==1u,"DataEnd must wait for final INT1 ack");
            PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801803u,0x80u);
            CdxaAck();
        }
        PE_IRQ_WriteStatus(0u);PE_CdReg_ServiceDevice(0x100000u);
        ASSERT(!PE_Port_ShouldStop() && CdxaTag()==4u && CdxaByte()==0u,
               "end of disc must stop the drive and respond INT4(stat=0)");
        ASSERT(PE_IRQ_ReadStatus()&4u,"DataEnd must assert enabled CD IRQ");
        PE_CdReg_GetDeviceState(&d);
        ASSERT(!d.reading && d.motor_off && d.next_lba==64u && d.sectors==1u &&
               d.xa_sectors==xa,"lead-out must not invent another sector");
        if(!xa) {
            PE_CdReg_WriteU8(0x1F801800u,0u);
            ASSERT(PE_CdReg_ReadU8(0x1F801802u)==0xA5u,"DataEnd preserves final unread FIFO");
        }
        CdxaAck();PE_CdReg_ServiceDevice(0x100000u);
        ASSERT(CdxaTag()==0u && !PE_Port_ShouldStop(),"DataEnd occurs once");
    }
    PE_CdReg_Reset();PE_Disc_SetActive(NULL);FxFree(&fx);PASS();
}

/* Opening fixes image geometry. A later short file is an actual read failure,
 * not lead-out: never turn it into a successful DataEnd. */
static void test_CDXA_short_image_read_error(void)
{
    DiscFixture fx={0};PE_Disc *disc;char err[256];
    char path[512];const char *tmp=getenv("TMPDIR");int fd;
    static const uint8_t loc[3]={0u,2u,0x63u};
    TEST("CDXA_short_image_read_error");
    ResetTestState();ASSERT(FxBuild(&fx,0),"fixture");
    snprintf(path,sizeof path,"%s/pe-cdend-XXXXXX",tmp?tmp:"/tmp");
    fd=mkstemp(path);ASSERT(fd>=0,"private temporary image");
    ASSERT(write(fd,fx.img,FX_SECTORS*PE_DISC_RAW_SECTOR)==FX_SECTORS*PE_DISC_RAW_SECTOR,"write image");
    disc=PE_Disc_Open(path,err,sizeof err);
    ASSERT(disc && PE_Disc_UserSectorCount(disc)==64u,"validated geometry");
    ASSERT(ftruncate(fd,32u*PE_DISC_RAW_SECTOR)==0,"truncate backing file after open");
    close(fd);unlink(path);
    PE_Disc_SetActive(disc);ASSERT(PE_CdReg_EnableDevice(7u),"attach");
    CdxaCommand(2u,loc,3u);PE_CdReg_ServiceDevice(0xC4E1u);CdxaAck();
    CdxaCommand(27u,NULL,0u);PE_CdReg_ServiceDevice(0xC4E1u);CdxaAck();
    PE_CdReg_ServiceDevice(451584u);
    ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_device_sector_read")==1 && CdxaTag()!=4u,
           "in-range short read stays an unresolved boundary");
    PE_CdReg_Reset();PE_Disc_SetActive(NULL);PE_Disc_Close(disc);FxFree(&fx);PASS();
}

/* Reproduce production frame-sized time steps and the GetlocP/INT1 flush
 * race using register ingress, not forced device internals. */
static void test_CDXA_cadence_and_flush(void)
{
    TEST("CDXA_cadence_and_flush");
    DiscFixture fx={0};PeCdDeviceState d;
    uint8_t mode=0xC8u,loc[3]={0u,2u,0x50u};
    ResetTestState();ASSERT(FxBuild(&fx,0),"fixture");
    for(unsigned i=50u;i<64u;i++) CdxaWriteSector(fx.img,i,1u,0u,0x64u);
    PE_Disc_SetActive(fx.disc);CdDeviceSeed();
    (void)PE_IRQ_ExchangeMask(0u);
    ASSERT(PE_CdReg_EnableDevice(0u),"attachment");
    CdxaCommand(14u,&mode,1u);PE_CdReg_ServiceDevice(0xC4E1u);CdxaAck();
    CdxaCommand(2u,loc,3u);PE_CdReg_ServiceDevice(0xC4E1u);CdxaAck();
    CdxaCommand(27u,NULL,0u);PE_CdReg_ServiceDevice(0xC4E1u);CdxaAck();
    HostFB_VSync(0);PE_CdReg_GetDeviceState(&d);
    ASSERT(d.sectors==2u && d.xa_sectors==2u,"double speed must deliver two sectors in first frame");
    HostFB_VSync(0);PE_CdReg_GetDeviceState(&d);
    ASSERT(d.sectors==5u,"second frame must retain half-sector surplus");
    HostFB_VSync(2);PE_CdReg_GetDeviceState(&d);
    ASSERT(d.sectors==10u && !PE_Port_ShouldStop(),"four frames must deliver ten sectors");
    PE_CdReg_Reset();PE_Disc_SetActive(NULL);FxFree(&fx);PASS();
}
static void test_CDXA_pending_interrupt_flush(void)
{
    TEST("CDXA_pending_interrupt_flush");
    DiscFixture fx={0};PeCdDeviceState d;
    uint8_t loc[3]={0u,2u,0x50u};
    ResetTestState();ASSERT(FxBuild(&fx,0),"fixture");
    PE_Disc_SetActive(fx.disc);CdDeviceSeed();(void)PE_IRQ_ExchangeMask(0u);
    ASSERT(PE_CdReg_EnableDevice(0u),"attachment");
    for(unsigned tag=1;tag<=5;tag++) {
        uint8_t response=0x22u;
        ASSERT(PE_CdReg_PushResponse((uint8_t)tag,&response,1u),"inject pending response");
        func_8007B9EC();
        ASSERT(!PE_Port_ShouldStop() && CdxaTag()==0u,"retail flush must acknowledge pending interrupt");
        ASSERT(PE_LoadU8(0x8009B294u)==2u && PE_LoadU8(0x8009B295u)==0u &&
               PE_LoadU8(0x8009B296u)==0u,"flush latches");
    }
    CdxaCommand(2u,loc,3u);PE_CdReg_ServiceDevice(0xC4E1u);CdxaAck();
    CdxaCommand(6u,NULL,0u);PE_CdReg_ServiceDevice(0xC4E1u);CdxaAck();
    PE_CdReg_ServiceDevice(451584u);ASSERT(CdxaTag()==1u,"data INT1");
    func_8007B9EC();
    /* No BFRD: the acknowledged sector can be superseded by the next one. */
    PE_CdReg_ServiceDevice(451584u);PE_CdReg_GetDeviceState(&d);
    ASSERT(!PE_Port_ShouldStop() && d.sectors==2u && CdxaTag()==1u,"acknowledged unread sector must not stop next delivery");
    PE_CdReg_Reset();PE_Disc_SetActive(NULL);FxFree(&fx);PASS();
}
