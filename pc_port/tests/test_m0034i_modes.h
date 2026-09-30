#include "retail_m0034i_modes_cases.h"
static void test_DAY1_m0034i_modes(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY1_m0034i_modes");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);
    for (unsigned i=0;i<100;i++)ASSERT(PE_Disc_ReadUserSector(disc,M34_CHUNK_LBA+i,PE_Translate(0x8018EFE8u+i*2048u,2048)),"M0034I sector");
    PE_Disc_Close(disc);seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"mode seed allocation");
    memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for (unsigned k=0;k<sizeof(M34M_cases)/sizeof(M34M_cases[0]);k++) {
        const pe_addr_t actor=0x80150000u,aya=0x80150800u,task=0x80160040u;
        unsigned v=M34M_cases[k].variant;static const uint8_t scenes[]={0,1,10,255};
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        PE_StoreU32(actor+0x9Cu,0x801B903Cu);PE_StoreU8(actor+12u,5);
        PE_StoreU32(actor+0xA0u,0x80160000u);PE_StoreU32(actor+0xA4u,task);PE_StoreU32(actor+0xA8u,task+0x40u);
        PE_StoreU32(0x80160024u,task);PE_StoreU32(task+0x28u,0x80160000u);
        PE_StoreU32(0x8009D254u,aya);PE_StoreU32(0x8009D2F0u,actor);PE_StoreU32(0x8009D300u,task);PE_StoreU32(0x8009D1A0u,0);
        PE_StoreU32(actor+0xC4u,v&8u?1:0);PE_StoreU8(0x8009CDB4u,0);
        PE_StoreU32(aya+0x28u,v&8u?0:0x10000u);PE_StoreU32(aya+0x30u,v&8u?0:0xFFFF0000u);
        PE_StoreU32(task,0x801B9270u);PE_StoreU32(task+16u,1);PE_StoreU32(0x8009D28Cu,M34M_cases[k].mode);
        PE_StoreU32(0x800B6A80u,(v&3u)*0x1000u);PE_StoreU32(0x800A7838u,0xA5A50000u|v);PE_StoreU32(0x800A7820u,0x12345678u);
        PE_StoreU32(0x8009D2E8u,0xA4);PE_StoreU32(0x800B0CD8u,0xA00000u);PE_StoreU8(0x80091A1Cu,1);PE_StoreU8(0x80091A1Du,scenes[v>>2]);PE_StoreU8(0x800BCFEEu,0);
        for (unsigned i=0;i<2;i++) {
            uint64_t hash;
            PE_StoreU32(0x8009D300u,task);func_80017018();
            hash=hit_camera_hash(M34M_ranges,sizeof(M34M_ranges)/sizeof(M34M_ranges[0]));
            if (hash!=M34M_cases[k].hash[i])fprintf(stderr,"M34 mode%u pass%u %016llX/%016llX\n",k,i,(unsigned long long)hash,(unsigned long long)M34M_cases[k].hash[i]);
            ASSERT(hash==M34M_cases[k].hash[i],"M0034I mode continuation differs from original");
            ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"M0034I mode continuation completed natively");
        }
    }
    free(seed);PASS();
}
