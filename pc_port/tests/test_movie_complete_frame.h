#include "retail_movie_complete_frame_cases.h"
static void test_DAY2_movie_complete_frame(void)
{
    TEST_RETAIL_DISC1("DAY2_movie_complete_frame"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    char err[256]={0};uint8_t raw[2352];
    PE_Disc *disc=BTL6_OpenDisc1(err,sizeof(err));
    ASSERT(disc!=NULL,err[0]?err:"opening movie disc unavailable");
    for(unsigned k=0;k<sizeof(MOVCOMPLETE_cases)/sizeof(MOVCOMPLETE_cases[0]);k++) {
        ResetTestState();
        for(unsigned sector=0;sector<38u;sector++) {
            ASSERT(PE_Disc_ReadRawSector(disc,1940u+sector,raw),"libpress sector read");
            memcpy(PE_Translate(0x8010BCF8u+sector*2048u,2048u),raw+24,2048u);
        }
        func_8010BD4C(0x80130000u,0);
        ASSERT(B54K_MdecFnv1a64(0x80130000u,69632u)==MOVCOMPLETE_table_hash,"complete VLC table differs from original");
        unsigned chunks=0;
        for(unsigned lba=MOVCOMPLETE_cases[k].first;lba<=MOVCOMPLETE_cases[k].last;lba++) {
            ASSERT(PE_Disc_ReadRawSector(disc,lba,raw),"opening STR sector read");
            if(!(raw[18]&8u)) continue;
            ASSERT(raw[24]==0x60u && raw[25]==1u && raw[26]==1u && raw[27]==0x80u &&
                raw[28]==chunks && raw[29]==0u && raw[30]==MOVCOMPLETE_cases[k].chunks &&
                raw[32]==MOVCOMPLETE_cases[k].frame,"opening STR chunk sequence");
            memcpy(PE_Translate(0x80150000u+chunks*2016u,2016u),raw+56,2016u);chunks++;
        }
        ASSERT(chunks==MOVCOMPLETE_cases[k].chunks && B54K_MdecFnv1a64(0x80150000u,chunks*2016u)==MOVCOMPLETE_cases[k].input_hash,"complete opening frame payload differs");
        memset(PE_Translate(0x80160000u,MOVCOMPLETE_cases[k].length+16u),0xCD,MOVCOMPLETE_cases[k].length+16u);
        ASSERT(func_8010C89C(0x80150000u,0x80160000u,0x80130000u,0)==0 && !PE_Port_ShouldStop(),"complete opening VLC frame failed to terminate");
        ASSERT(B54K_MdecFnv1a64(0x80160000u,MOVCOMPLETE_cases[k].length)==MOVCOMPLETE_cases[k].output_hash,"complete native RLE differs from original instructions");
        for(unsigned i=0;i<16u;i++) ASSERT(PE_LoadU8(0x80160000u+MOVCOMPLETE_cases[k].length+i)==0xCDu,"VLC exceeded declared output");
        /* Decode every real macroblock in both movie formats. Pixel values are
         * model output, not a captured hardware golden. No callback provider. */
        uint16_t *expected=calloc(480u*240u,sizeof(*expected));
        ASSERT(expected!=NULL,"full movie reference allocation");
        for(unsigned wide=0;wide<2u;wide++) {
            PE_MDEC_Init();func_8010BE3C(0);
            func_8010BFA0(0x80160000u,wide);
            unsigned bytes=wide?768u:512u;
            for(unsigned macro=0;macro<300u;macro++) {
                func_8010C01C(0x80170000u,bytes/4u);PE_MDEC_Service();
                if(wide) for(unsigned y=0;y<16u;y++) for(unsigned x=0;x<24u;x++)
                    expected[((macro%15u)*16u+y)*480u+(macro/15u)*24u+x]=PE_LoadU16(0x80170000u+(y*24u+x)*2u);
                PeMdecState state;PE_MDEC_GetState(&state);
                ASSERT(!PE_Port_ShouldStop() && state.completed_output_count==macro+1u &&
                    PE_MDEC_DecodedMacroblocks()==macro+1u,"real movie macroblock DMA decode failed");
            }
        }
        /* Run the actual updater on this frame while preparing its next copy.
         * Reference pixels above are model output, not hardware goldens. */
        HostFB_Init();PE_GPU_Init();func_80073C94();B54KR_SeedGpuStatic();PE_MDEC_Init();func_8010BE3C(0);
        func_8010C0D8(0x801214D4u);
        memmove(PE_Translate(0x80150040u,chunks*2016u),PE_TranslateConst(0x80150000u,chunks*2016u),chunks*2016u);
        memset(PE_Translate(0x80150000u,64u),0,64u);
        PE_StoreU16(0x80150000u,2u);PE_StoreU16(0x80150006u,1u);PE_StoreU32(0x80150008u,MOVCOMPLETE_cases[k].frame);
        PE_StoreU16(0x80150010u,320u);PE_StoreU16(0x80150012u,240u);
        PE_StoreU32(0x800C0DC8u,0x80150000u);PE_StoreU32(0x800C20C4u,2u);PE_StoreU32(0x800BE9ECu,0u);
        PE_StoreU32(0x801227E4u,0x80181000u);PE_StoreU16(0x80181008u,100u);PE_StoreU16(0x801227E8u,0u);
        PE_StoreU16(0x80122418u,320u);PE_StoreU16(0x8012241Au,240u);
        PE_StoreU32(0x8009B574u,2u);PE_StoreU8(0x800B0DBAu,2u);PE_StoreU8(0x800B0DBBu,1u);PE_StoreU8(0x801223F6u,3u);
        PE_StoreU8(0x801223F5u,0u);PE_StoreU32(0x80122430u,0x80130000u);
        PE_StoreU32(0x801228CCu,0x80160000u);PE_StoreU32(0x801228D0u,0x80164000u);PE_StoreU8(0x801228D4u,0u);
        PE_StoreU32(0x801228D8u,0x80170000u);PE_StoreU32(0x801228DCu,0x80174000u);PE_StoreU8(0x801228E0u,0u);
        for(unsigned bank=0;bank<2u;bank++) {
            PE_StoreU16(0x801228E2u+bank*8u,0u);PE_StoreU16(0x801228E4u+bank*8u,(uint16_t)(bank*240u));
            PE_StoreU16(0x801228E6u+bank*8u,480u);PE_StoreU16(0x801228E8u+bank*8u,240u);
        }
        PE_StoreU8(0x801228F2u,0u);PE_StoreU16(0x801228F4u,0u);PE_StoreU16(0x801228F6u,0u);
        PE_StoreU16(0x801228F8u,24u);PE_StoreU16(0x801228FAu,240u);PE_StoreU8(0x801228FCu,0u);
        ASSERT(func_80122040()==1 && !PE_Port_ShouldStop(),"actual movie updater did not finish real frame");
        ASSERT(PE_MDEC_DecodedMacroblocks()==300u && PE_LoadU8(0x801228D4u)==1u && PE_LoadU16(0x800B0DBCu)==1u &&
            PE_LoadU16(0x80150000u)==0u && PE_LoadU8(0x801228F2u)==1u && !PE_LoadU8(0x801228FCu),"updater frame handoff/completion state");
        ASSERT(B54K_MdecFnv1a64(0x80164000u,MOVCOMPLETE_cases[k].length)==MOVCOMPLETE_cases[k].output_hash,"updater next frame VLC differs from original");
        ASSERT(func_80074DC0(0)==0 && !PE_Port_ShouldStop(),"updater movie uploads did not finish");
        for(unsigned y=0;y<240u;y++) for(unsigned x=0;x<480u;x++) {
            uint16_t actual=0;PE_GPU_ReadVRAM(x,y,&actual);
            if(actual!=expected[y*480u+x]) fprintf(stderr,"movie frame%u pixel%u,%u actual%04X expected%04X\n",k,x,y,actual,expected[y*480u+x]);
            ASSERT(actual==expected[y*480u+x],"full real movie frame slice layout differs in VRAM");
        }
        free(expected);
    }
    PE_Disc_Close(disc);PASS();
}
