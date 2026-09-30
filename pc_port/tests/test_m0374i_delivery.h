#include "retail_m0374i_delivery_cases.h"
static void test_DAY2_m0374i_delivery(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY2_m0374i_delivery");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);
    for (unsigned i=0;i<M374_CHUNK_SECTORS;i++)
        ASSERT(PE_Disc_ReadUserSector(disc,M374_CHUNK_LBA+i,PE_Translate(0x8018EFE8u+i*2048u,2048)),"M0374I chunk sector");
    PE_Disc_Close(disc);seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"M0374I seed allocation");
    memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for (unsigned k=0;k<sizeof(M374D_cases)/sizeof(M374D_cases[0]);k++) {
        const pe_addr_t sender=0x80150000u,actor=0x80150200u,task=0x80160000u,mail=task+0x40u,child=task+0x80u,old=task+0x100u;
        uint64_t hashes[3];unsigned linked=M374D_cases[k].linked;
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        memset(PE_Translate(sender,0x400),0,0x400);memset(PE_Translate(task,0x180),0,0x180);
        PE_StoreU32(sender+4u,actor);PE_StoreU16(sender+0x24u,0x1234);PE_StoreU32(sender+0x9Cu,0x801C4E04u);
        PE_StoreU8(actor+12u,2);PE_StoreU32(actor+0x9Cu,0x801C5840u);PE_StoreU32(actor+0xA8u,linked?old:0);
        PE_StoreU32(0x8009D20Cu,sender);PE_StoreU32(0x8009D2F0u,actor);PE_StoreU32(0x8009D1A0u,0);
        PE_StoreU32(0x80154000u,0x801C58C4u);PE_StoreU32(0x80154004u,0x801C58C8u);func_80017588(0x80154000u);
        PE_StoreU32(task,0x801C54B8u);PE_StoreU32(task+16u,1);PE_StoreU32(sender+0xA8u,task);
        PE_StoreU32(mail+0x24u,child);PE_StoreU32(child+0x24u,task+0xC0u);PE_StoreU32(0x8009CDFCu,mail);
        PE_StoreU16(0x8009D308u,(uint16_t)M374D_cases[k].serial);PE_StoreU8(0x8009CDB4u,0);PE_StoreU32(0x800A7918u,M374D_cases[k].story);
#define M374D_HASH() hit_camera_hash(M374D_ranges,sizeof(M374D_ranges)/sizeof(M374D_ranges[0]))
        hashes[0]=M374D_HASH();
        PE_StoreU32(0x8009D2F0u,sender);PE_StoreU32(0x8009D300u,task);func_80017018();func_80065400();hashes[1]=M374D_HASH();
        PE_StoreU32(0x8009D2F0u,actor);PE_StoreU32(0x8009D300u,mail);func_80017018();hashes[2]=M374D_HASH();
#undef M374D_HASH
        for (unsigned i=0;i<3;i++) {
            if(hashes[i]!=M374D_cases[k].hash[i])fprintf(stderr,"M374 delivery%u stage%u %016llX/%016llX\n",k,i,(unsigned long long)hashes[i],(unsigned long long)M374D_cases[k].hash[i]);
            ASSERT(hashes[i]==M374D_cases[k].hash[i],"M0374I delivery/scene start differs from original");
        }
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"M0374I delivery completed natively to animation wait");
    }
    free(seed);PASS();
}
