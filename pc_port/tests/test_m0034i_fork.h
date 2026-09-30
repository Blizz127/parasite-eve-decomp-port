#include "retail_m0034i_fork_cases.h"
static void test_DAY1_m0034i_fork(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY1_m0034i_fork");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);
    for (unsigned i=0;i<100;i++)
        ASSERT(PE_Disc_ReadUserSector(disc,M34_CHUNK_LBA+i,PE_Translate(0x8018EFE8u+i*2048u,2048)),"M0034I chunk sector");
    PE_Disc_Close(disc);seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"sender/fork seed allocation");
    memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for (unsigned k=0;k<sizeof(M34F_cases)/sizeof(M34F_cases[0]);k++) {
        const pe_addr_t controller=0x80150000u,aya=0x80150800u,task=0x80160000u,atask=task+0xC0u,sender=task+0x80u,child=task+0x40u;
        unsigned v=M34F_cases[k].variant;uint64_t hashes[6];
        uint32_t flags=(v&1?2u:0u)|(v&2?0x80u:0u)|(v&4?0x100u:0u)|(v&8?0x200u:0u);
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        PE_StoreU32(controller+4u,aya);PE_StoreU8(controller+12u,5);PE_StoreU32(controller+0x9Cu,0x801B903Cu);PE_StoreU16(controller+0x24u,0xABCD);
        PE_StoreU32(aya,0x80154000u);PE_StoreU32(aya+0x9Cu,0x801B4B20u);PE_StoreU32(0x8009D254u,aya);PE_StoreU32(0x8009D20Cu,controller);
        PE_StoreU32(aya+0x98u,flags);PE_StoreU32(aya+0x14u,0x12345678u);PE_StoreU32(aya+0x18u,0x87654321u);PE_StoreU32(aya+0x1Cu,0xFFFD0000u);PE_StoreU32(aya+0x1A4u,0x12345678u);PE_StoreU32(aya+0x1A8u,0x87654321u);
        PE_StoreU16(0x80154010u,9000);PE_StoreU32(0x800B0EA8u,0x80154100u);PE_StoreU8(0x80154102u,(uint8_t)M34F_cases[k].cap);
        PE_StoreU32(0x8009D1A0u,0);PE_StoreU32(0x8009D2F0u,aya);
        for (unsigned i=0;i<2;i++)PE_StoreU32(0x80154080u+i*4u,0x801B4B84u+i*4u);
        func_80017588(0x80154080u);
        PE_StoreU32(0x800B1620u,0x801BA270u);func_8001A918();
        PE_StoreU32(0x800BCF88u,0xA501);PE_StoreU32(0x800B6A80u,0);PE_StoreU32(0x8009D2E8u,0xA1);PE_StoreU32(0x800B0CD8u,0xA02000);PE_StoreU32(0x8009D28Cu,8);
        memset(PE_Translate(task,0x180u),0,0x180u);
        PE_StoreU32(task,0x801B98ECu);PE_StoreU16(task+8u,4);PE_StoreU32(task+16u,1);PE_StoreU32(task+20u,112);PE_StoreU32(controller+0xA8u,task);
        PE_StoreU32(child+0x24u,atask);PE_StoreU32(atask+0x24u,atask+0x40u);PE_StoreU32(0x8009CDFCu,child);PE_StoreU16(0x8009D308u,0xFFFF);PE_StoreU8(0x8009CDB4u,0);
#define M34F_HASH() hit_camera_hash(M34F_ranges,sizeof(M34F_ranges)/sizeof(M34F_ranges[0]))
        PE_StoreU32(sender,0x801B91D4u);PE_StoreU16(sender+8u,(uint16_t)(v&3u));PE_StoreU32(sender+16u,1);
        PE_StoreU32(sender+0x24u,v&4u?task+0x140u:0);
        if (v&4u)PE_StoreU32(task+0x168u,sender);
        hashes[0]=M34F_HASH();
        PE_StoreU32(0x8009D2F0u,controller);PE_StoreU32(0x8009D300u,task);func_80017018();hashes[1]=M34F_HASH();
        PE_StoreU32(0x8009D300u,sender);func_80017018();func_80065400();hashes[2]=M34F_HASH();
        PE_StoreU32(0x8009D2F0u,aya);PE_StoreU32(0x8009D300u,atask);func_80017018();hashes[3]=M34F_HASH();
        PE_StoreU32(0x8009D2F0u,controller);PE_StoreU32(0x8009D300u,task);func_80017018();hashes[4]=M34F_HASH();
        PE_StoreU32(0x8009D300u,child);func_80017018();hashes[5]=M34F_HASH();
#undef M34F_HASH
        for (unsigned i=0;i<6;i++) {
            if (hashes[i]!=M34F_cases[k].hash[i])fprintf(stderr,"M34 fork%u stage%u %016llX/%016llX\n",k,i,(unsigned long long)hashes[i],(unsigned long long)M34F_cases[k].hash[i]);
            ASSERT(hashes[i]==M34F_cases[k].hash[i],"M0034I sender/fork differs from original");
        }
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"M0034I sender/fork completed natively");
    }
    free(seed);PASS();
}
