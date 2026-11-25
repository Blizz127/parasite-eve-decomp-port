/* Clean-room STR/BS VLC decoder (pe_plat_mdec_vlc.c) against the original
 * libpress decoder's outputs (retail_mdec_vlc_cases.h).  Test-only: the
 * decoder is not bound into the production movie path. */
#include "pe_vlc_decode.h"
#include "pe_plat/vlc_frame.h"
#include "retail_mdec_vlc_bound_cases.h"
#include "retail_c89c_real_sequence.h"
#include "retail_mdec_vlc_cases.h"
#include "mdec_vlc_check_core.h"

static int PressReadRaw(void *ctx, uint32_t lba, uint8_t raw[2352])
{
    return PE_Disc_ReadRawSector((PE_Disc *)ctx, lba, raw);
}
static void test_PRESS_mdec_vlc(void)
{
    TEST_RETAIL_DISC1("PRESS_mdec_vlc");
    char err[256]={0},msg[300];unsigned nr,ns,nk,nh;
    PE_Disc *disc=BTL6_OpenDisc1(err,sizeof(err));
    ASSERT(disc!=NULL,err[0]?err:"opening movie disc unavailable");
    int ok=MdecVlcCheckAll(PressReadRaw,disc,err,sizeof(err),&nr,&ns,&nk,&nh);
    snprintf(msg,sizeof(msg),"clean-room VLC vs original: %s",ok?"":err);
    ASSERT(ok,msg);
    ASSERT(nr>=100u && ns>=80u && nh==3u,"vector coverage");
    PE_Disc_Close(disc);
    PASS();
}

/* Pipeline: real FMV001 frame -> clean-room VLC -> existing MDEC pixel model
 * (func_8010BFA0/func_8010C01C over PE_MDEC) -> 320x240 RGB555 image.  The VLC
 * output is also compared with the original decoder's pinned hash.  When
 * PE_PRESS_PPM names a file the image is written there (retail pixels: keep it
 * out of the repository). */
static void test_PRESS_mdec_pipeline(void)
{
    TEST_RETAIL_DISC1("PRESS_mdec_pipeline"); TEST_RETAIL_FIXUPS(RETAILFIX_dispenv);
    char err[256]={0};uint8_t raw[2352];
    PE_Disc *disc=BTL6_OpenDisc1(err,sizeof(err));
    ASSERT(disc!=NULL,err[0]?err:"opening movie disc unavailable");
    ResetTestState();
    for(unsigned sector=0;sector<38u;sector++) {
        ASSERT(PE_Disc_ReadRawSector(disc,1940u+sector,raw),"libpress sector read");
        memcpy(PE_Translate(0x8010BCF8u+sector*2048u,2048u),raw+24,2048u);
    }
    ASSERT(func_8010BD4C(0x80130000u,0)==1u && !PE_Port_ShouldStop(),"movie table builder");
    /* last pinned real case: frame data re-read from the disc */
    unsigned k=(unsigned)(sizeof(MDECVLC_real)/sizeof(MDECVLC_real[0]))-1u;
    static uint8_t frame[64u*2016u],out[1u<<20];size_t n=0;
    for(uint32_t lba=MDECVLC_real[k].first;lba<=MDECVLC_real[k].last;lba++) {
        ASSERT(PE_Disc_ReadRawSector(disc,lba,raw),"STR sector read");
        if(!(raw[18]&8u)) continue;
        memcpy(frame+n,raw+56,2016u);n+=2016u;
    }
    size_t outlen=0;
    ASSERT(MdecVlcRun(frame,MDECVLC_real[k].size,out,sizeof(out),&outlen)==PE_VLC_OK &&
           outlen==MDECVLC_real[k].outlen && MdecVlcFnv(out,outlen)==MDECVLC_real[k].out_hash,"clean-room VLC frame");
    memcpy(PE_Translate(0x80160000u,outlen),out,outlen);
    PE_MDEC_Init();func_8010BE3C(0);func_8010BFA0(0x80160000u,0);
    uint16_t *image=calloc(320u*240u,sizeof(*image));
    ASSERT(image!=NULL,"image allocation");
    for(unsigned macro=0;macro<300u;macro++) {
        func_8010C01C(0x80170000u,512u/4u);PE_MDEC_Service();
        for(unsigned y=0;y<16u;y++) for(unsigned x=0;x<16u;x++)
            image[((macro%15u)*16u+y)*320u+(macro/15u)*16u+x]=PE_LoadU16(0x80170000u+(y*16u+x)*2u);
        PeMdecState state;PE_MDEC_GetState(&state);
        ASSERT(!PE_Port_ShouldStop() && state.completed_output_count==macro+1u,"MDEC macroblock decode from clean-room RLE");
    }
    /* image is not all one colour */
    unsigned distinct=0;for(unsigned i=1;i<320u*240u;i++) distinct+=image[i]!=image[0];
    ASSERT(distinct>1000u,"decoded frame has picture content");
    const char *ppm=getenv("PE_PRESS_PPM");
    if(ppm && ppm[0]) {
        FILE *f=fopen(ppm,"wb");
        ASSERT(f!=NULL,"PPM open");
        fprintf(f,"P6\n320 240\n255\n");
        for(unsigned i=0;i<320u*240u;i++) {
            unsigned p=image[i];uint8_t px[3]={(uint8_t)((p&31u)*255u/31u),(uint8_t)(((p>>5)&31u)*255u/31u),(uint8_t)(((p>>10)&31u)*255u/31u)};
            fwrite(px,1,3,f);
        }
        fclose(f);
    }
    free(image);PE_Disc_Close(disc);PASS();
}

/* Guest-shaped adapter (pe_plat_vlc_frame_host) on the same vectors: frame in guest RAM at 0x80150000, output at
 * 0x80160000 (the production callers' layout), table pointer 0x80130000.  Checks the output buffer against the
 * original's, the canary behind it, the return value (0) and the end-pointer word the original leaves at
 * 0x8011EBB4.  Streams on which the original never terminates must end in a boundary stop. */
static int PressAdapterCall(const uint8_t *in,size_t n,uint32_t expect_len,uint64_t expect_hash,uint32_t expect_cursor,int expect_ok,char *why,size_t whysz)
{
    uint32_t declared=4u+(((uint32_t)in[0]|((uint32_t)in[1]<<8))*4u),region=(expect_ok?expect_len:declared)+0x100u;
    ResetTestState();
    memset(PE_Translate(0x80150000u,64u*2016u),0,64u*2016u);
    memcpy(PE_Translate(0x80150000u,n),in,n);
    PE_StoreU32(0x8011EB8Cu,0x00FFFFFFu);             /* shipped value of the bound word */
    memset(PE_Translate(0x80160000u,region),0xCD,region);
    uint32_t a0r=0,a1r=0;
    int ret=pe_plat_vlc_frame_host_regs(0x80150000u,0x80160000u,0x80130000u,0u,&a0r,&a1r);
    if(!expect_ok) {
        if(!PE_Port_ShouldStop()) {snprintf(why,whysz,"never-terminating stream did not stop");return 0;}
        return 1;
    }
    if(PE_Port_ShouldStop() || ret!=0) {snprintf(why,whysz,"stopped or nonzero return");return 0;}
    if(MdecVlcFnv(PE_TranslateConst(0x80160000u,expect_len),expect_len)!=expect_hash) {snprintf(why,whysz,"output differs");return 0;}
    for(uint32_t i=0;i<0x100u;i++) if(PE_LoadU8(0x80160000u+expect_len+i)!=0xCDu) {snprintf(why,whysz,"write past output");return 0;}
    if(PE_LoadU32(0x8011EBB4u)!=0x80160000u+declared) {snprintf(why,whysz,"end-pointer word differs");return 0;}
    if(a1r!=0x80160000u+expect_len || a0r!=0x80150000u+expect_cursor) {snprintf(why,whysz,"returned a0/a1 differ from the original's (fitted cursor)");return 0;}
    return 1;
}
static void test_PRESS_vlc_adapter(void)
{
    TEST_RETAIL_DISC1("PRESS_vlc_adapter");
    char err[256]={0},why[128]={0},msg[400];uint8_t raw[2352];
    PE_Disc *disc=BTL6_OpenDisc1(err,sizeof(err));
    ASSERT(disc!=NULL,err[0]?err:"opening movie disc unavailable");
    static uint8_t frame[64u*2016u];
    for(unsigned k=0;k<sizeof(MDECVLC_real)/sizeof(MDECVLC_real[0]);k++) {
        size_t n=0;
        for(uint32_t lba=MDECVLC_real[k].first;lba<=MDECVLC_real[k].last;lba++) {
            ASSERT(PE_Disc_ReadRawSector(disc,lba,raw),"STR sector read");
            if(!(raw[18]&8u)) continue;
            memcpy(frame+n,raw+56,2016u);n+=2016u;
        }
        snprintf(msg,sizeof(msg),"real %u (frame %u): %%s",k,MDECVLC_real[k].frame);
        char m2[500];
        int ok=PressAdapterCall(frame,MDECVLC_real[k].size,MDECVLC_real[k].outlen,MDECVLC_real[k].out_hash,MDECVLC_real[k].cursor,1,why,sizeof(why));
        snprintf(m2,sizeof(m2),msg,why);
        ASSERT(ok,m2);
    }
    for(unsigned k=0;k<sizeof(MDECVLC_syn)/sizeof(MDECVLC_syn[0]);k++) {
        int ok=PressAdapterCall(MDECVLC_syn[k].in,MDECVLC_syn[k].insize,MDECVLC_syn[k].outlen,MDECVLC_syn[k].out_hash,MDECVLC_syn[k].cursor,1,why,sizeof(why));
        snprintf(msg,sizeof(msg),"adapter syn %s: %s",MDECVLC_syn[k].name,why);
        ASSERT(ok,msg);
    }
    for(unsigned k=0;k<sizeof(MDECVLC_hang)/sizeof(MDECVLC_hang[0]);k++) {
        int ok=PressAdapterCall(MDECVLC_hang[k].in,MDECVLC_hang[k].insize,0,0,0,0,why,sizeof(why));
        snprintf(msg,sizeof(msg),"adapter hang %s: %s",MDECVLC_hang[k].name,why);
        ASSERT(ok,msg);
    }
    ResetTestState();
    PE_Disc_Close(disc);PASS();
}

/* Bounded/resume chains of the original and the 253-call real playback sequence, replayed through the adapter in guest RAM. */
#define C89CR_RESET() ResetTestState()
#define C89CR_STOPPED() PE_Port_ShouldStop()
#include "c89c_replay_core.h"
static int PressReplayRaw(void *ctx, uint32_t lba, uint8_t raw[2352])
{
    return PE_Disc_ReadRawSector((PE_Disc *)ctx, lba, raw);
}
static void test_PRESS_c89c_replay(void)
{
    TEST_RETAIL_DISC1("PRESS_c89c_replay");
    char err[256]={0},msg[400];unsigned ns,nk,nc,nr;
    PE_Disc *disc=BTL6_OpenDisc1(err,sizeof(err));
    ASSERT(disc!=NULL,err[0]?err:"opening movie disc unavailable");
    int ok=C89CReplayBound(PressReplayRaw,disc,err,sizeof(err),&ns,&nk,&nc);
    snprintf(msg,sizeof(msg),"C89C bound/resume replay: %s",ok?"":err);
    ASSERT(ok,msg);
    ASSERT(nc==96u && ns>=1390u && nk==4u,"C89C bound replay coverage");
    ok=C89CReplayReal(PressReplayRaw,disc,err,sizeof(err),&nr);
    snprintf(msg,sizeof(msg),"C89C real-sequence replay: %s",ok?"":err);
    ASSERT(ok && nr==253u,msg);
    ResetTestState();
    PE_Disc_Close(disc);PASS();
}
