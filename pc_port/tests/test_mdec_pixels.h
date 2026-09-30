#include "retail_mdec_pixel_tables.h"
/* Analytical vectors use a diagonal programmable scale matrix: two passes
 * divide coefficients by16 without fractional ambiguity. */
static void MdecPixelTables(void)
{
    PE_MDEC_Init();PE_MDEC_WriteControl(0x60000000u);
    PE_StoreU32(0x80140000u,0x40000001u);
    for(unsigned i=0;i<128u;i++) PE_StoreU8(0x80140004u+i,16u);
    (void)PE_MDEC_SubmitInputTable(0x80140000u,32u);
    PE_StoreU32(0x80140000u,0x60000000u);
    for(unsigned i=0;i<64u;i++) PE_StoreU16(0x80140004u+i*2u,(i/8u==i%8u)?16384u:0u);
    (void)PE_MDEC_SubmitInputTable(0x80140000u,32u);
}
static void test_DAY2_mdec_pixels(void)
{
    TEST_RETAIL_DISC1("DAY2_mdec_pixels"); TEST_RETAIL_FIXUPS(RETAILFIX_mdec_pixel_tables);
    static const uint8_t scan[]={0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63};
    for(unsigned bypass=0;bypass<2u;bypass++) for(unsigned position=1;position<64u;position++) for(unsigned negative=0;negative<2u;negative++) {
        ResetTestState();MdecPixelTables();
        PE_StoreU32(0x80144000u,0x28000002u);
        PE_StoreU16(0x80144004u,bypass?0u:8u<<10u);
        unsigned magnitude=bypass?128u:16u;
        PE_StoreU16(0x80144006u,(uint16_t)(((position-1u)<<10u)|(negative?(1024u-magnitude):magnitude)));
        PE_StoreU16(0x80144008u,0xFE00u);PE_StoreU16(0x8014400Au,0xFE00u);
        ASSERT(PE_MDEC_BeginDecode(0x80144000u),"MDEC impulse submission");
        ASSERT(PE_MDEC_ReadPixels(0x80150000u,7u) && PE_MDEC_ReadPixels(0x80150007u,57u),"MDEC split output");
        for(unsigned i=0;i<64u;i++) ASSERT(PE_LoadU8(0x80150000u+i)==(i==(bypass?position:scan[position])?(negative?112u:144u):128u),"MDEC scan/quant/IDCT impulse");
        ASSERT(PE_MDEC_DecodedMacroblocks()==1u && !PE_Port_ShouldStop(),"MDEC impulse completion");
    }
    for(unsigned depth=0;depth<4u;depth++) for(unsigned signed_output=0;signed_output<2u;signed_output++) for(unsigned bit15=0;bit15<2u;bit15++) {
        ResetTestState();MdecPixelTables();
        static const int values[]={-8,16,-16,8,24,48};
        unsigned blocks=depth<2u?1u:6u,halfwords=blocks*65u,words=(halfwords+1u)/2u;
        PE_StoreU32(0x80144000u,0x20000000u|(depth<<27u)|(signed_output<<26u)|(bit15<<25u)|words);
        for(unsigned b=0;b<blocks;b++) {
            int level=depth<2u?32:values[b];
            for(unsigned i=0;i<64u;i++) PE_StoreU16(0x80144004u+(b*65u+i)*2u,(uint16_t)((i?0u:8u<<10u)|((unsigned)level&1023u)));
            PE_StoreU16(0x80144004u+(b*65u+64u)*2u,0xFE00u);
        }
        if(halfwords&1u) PE_StoreU16(0x80144004u+halfwords*2u,0xFE00u);
        ASSERT(PE_MDEC_BeginDecode(0x80144000u),"MDEC format submission");
        unsigned size=depth==0u?32u:depth==1u?64u:depth==2u?768u:512u;
        ASSERT(PE_MDEC_ReadPixels(0x80150000u,size),"MDEC format output");
        if(depth<2u) {
            unsigned value=signed_output?32u:160u;if(!depth)value=(value>>4u)*17u;
            for(unsigned i=0;i<size;i++) ASSERT(PE_LoadU8(0x80150000u+i)==value,"MDEC monochrome packing");
        } else for(unsigned y=0;y<16u;y++) for(unsigned x=0;x<16u;x++) {
            int luma=values[2u+(y/8u)*2u+x/8u];
            /* Cr=-8,Cb=16 gives rounded RGB offsets -11,0,+28. */
            unsigned r=(uint8_t)(luma-11),g=(uint8_t)luma,b=(uint8_t)(luma+28);
            if(!signed_output) {r^=128u;g^=128u;b^=128u;}
            unsigned pixel=y*16u+x;
            if(depth==2u) ASSERT(PE_LoadU8(0x80150000u+pixel*3u)==r && PE_LoadU8(0x80150001u+pixel*3u)==g && PE_LoadU8(0x80150002u+pixel*3u)==b,"MDEC RGB24 ordering/chroma/quadrants");
            else {
                r=(r+4u)>>3u;g=(g+4u)>>3u;b=(b+4u)>>3u;
                unsigned packed=(r>31u?31u:r)|((g>31u?31u:g)<<5u)|((b>31u?31u:b)<<10u)|(bit15<<15u);
                ASSERT(PE_LoadU16(0x80150000u+pixel*2u)==packed,"MDEC RGB15 packing");
            }
        }
        ASSERT(!PE_Port_ShouldStop() && PE_MDEC_DecodedMacroblocks()==1u,"MDEC format completion");
    }
    /* The actual libpress reset uploads the authenticated retail matrices.
     * DC-only +/-64 with quant[0]=2 gives uniform +/-16 output. */
    for(int dc=-64;dc<=64;dc+=64) {
        ResetTestState();PE_MDEC_Init();
        for(unsigned i=0;i<33u;i++) {
            PE_StoreU32(0x8010DA0Cu+i*4u,MDECPIX_quant[i]);
            PE_StoreU32(0x8010DA90u+i*4u,MDECPIX_scale[i]);
        }
        func_8010BE3C(0);
        PE_StoreU32(0x80144000u,0x28000001u);
        PE_StoreU16(0x80144004u,(uint16_t)((8u<<10u)|((unsigned)dc&1023u)));PE_StoreU16(0x80144006u,0xFE00u);
        ASSERT(PE_MDEC_BeginDecode(0x80144000u) && PE_MDEC_ReadPixels(0x80150000u,64u),"retail-table DC decode");
        for(unsigned i=0;i<64u;i++) ASSERT(PE_LoadU8(0x80150000u+i)==(unsigned)(128+dc/4),"retail-table DC pixels");
    }
    /* psx-spx FIFO order: a second decode command issued while the first
     * still holds a whole undecoded 8-bit block queues that block's output
     * ahead of its own.  First command: DC +32 (read) then DC -32 (left);
     * second: DC +16.  FE00h padding at a block start is ignored. */
    {
        PeMdecState st;
        ResetTestState();MdecPixelTables();
        PE_StoreU32(0x80144000u,0x28000003u);
        PE_StoreU16(0x80144004u,(uint16_t)((8u<<10u)|32u));PE_StoreU16(0x80144006u,0xFE00u);
        PE_StoreU16(0x80144008u,0xFE00u);
        PE_StoreU16(0x8014400Au,(uint16_t)((8u<<10u)|(1024u-32u)));PE_StoreU16(0x8014400Cu,0xFE00u);
        PE_StoreU16(0x8014400Eu,0xFE00u);
        ASSERT(PE_MDEC_BeginDecode(0x80144000u) && PE_MDEC_ReadPixels(0x80150000u,64u),"MDEC first block");
        ASSERT(PE_LoadU8(0x80150000u)==160u,"MDEC first block level");
        PE_StoreU32(0x80146000u,0x28000001u);
        PE_StoreU16(0x80146004u,(uint16_t)((8u<<10u)|16u));PE_StoreU16(0x80146006u,0xFE00u);
        ASSERT(PE_MDEC_BeginDecode(0x80146000u),"MDEC second command");
        ASSERT(PE_MDEC_ReadPixels(0x80150000u,128u) && !PE_Port_ShouldStop(),"MDEC stale+new output");
        /* The diagonal test matrix maps DC to pixel 0 only (see the impulse
         * loop above), so each block is identified by its pixel 0. */
        for(unsigned i=0;i<128u;i++) ASSERT(PE_LoadU8(0x80150000u+i)==(i==0u?96u:i==64u?144u:128u),"MDEC stale block precedes the new command's block");
        PE_MDEC_GetState(&st);
        ASSERT(st.stale_input_discards==1u && st.stale_output_bytes==64u,"MDEC stale accounting");
    }
    ResetTestState();MdecPixelTables();PE_StoreU32(0x80144000u,0x28000001u);
    PE_StoreU32(0x80144004u,0u);ASSERT(PE_MDEC_BeginDecode(0x80144000u),"MDEC malformed submission");
    ASSERT(!PE_MDEC_ReadPixels(0x80150000u,64u) && PE_Port_ShouldStop() && CountOrderLog("MDEC_unterminated_block")==1 && !PE_LoadU32(0x80150000u),"MDEC malformed block fabricated output");
    PASS();
}
