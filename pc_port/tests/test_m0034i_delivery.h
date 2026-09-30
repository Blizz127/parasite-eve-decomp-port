#include "retail_m0034i_delivery_cases.h"
static void test_DAY1_m0034i_delivery(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY1_m0034i_delivery");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);
    for (unsigned i=0;i<100;i++)
        ASSERT(PE_Disc_ReadUserSector(disc,M34_CHUNK_LBA+i,PE_Translate(0x8018EFE8u+i*2048u,2048)),"M0034I chunk sector");
    PE_Disc_Close(disc);seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"delivery seed allocation");
    memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for (unsigned k=0;k<sizeof(M34D_cases)/sizeof(M34D_cases[0]);k++) {
        const pe_addr_t sender=0x80150000u,actor=0x80150400u,aya=0x80150800u,task=0x80152000u,node=0x80160000u;
        unsigned p=M34D_cases[k].profile,w=M34D_cases[k].waits,n=0;
        uint64_t hashes[5]={0};
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        PE_StoreU32(sender+4u,actor);PE_StoreU8(sender+12u,4);PE_StoreU16(sender+0x24u,(uint16_t)(0xFF00u+p));PE_StoreU32(sender+0x9Cu,0x801B8BCCu);
        PE_StoreU8(actor+12u,5);PE_StoreU32(actor+0x9Cu,0x801B903Cu);PE_StoreU32(actor+0xA8u,0x80152040u);
        PE_StoreU32(0x8009D20Cu,sender);PE_StoreU32(0x8009D2F0u,actor);PE_StoreU32(0x8009D300u,task);PE_StoreU32(0x8009D1A0u,0);
        for (unsigned i=0;i<2;i++)PE_StoreU32(0x80152200u+i*4u,0x801B90A4u+i*4u);
        func_80017588(0x80152200u);
        PE_StoreU32(0x8009D254u,aya);PE_StoreU32(aya,aya+0x100u);
        PE_StoreU16(aya+0x104u,M34D_levels[p%12]);PE_StoreU16(aya+0x110u,p>=12?9000:123);PE_StoreU16(aya+0x122u,(uint16_t)(1+p%6));
        PE_StoreU32(0x800B6A80u,w?0:16);PE_StoreU32(0x800B6A84u,p*7+1);PE_StoreU32(0x800B6A88u,1+p%3);PE_StoreU32(0x800B6A8Cu,0xCAFEBABEu);PE_StoreU32(0x800B6A90u,p*3);
        PE_StoreU32(0x8009D2E8u,0xA1);PE_StoreU32(0x800B0CD8u,0xA02000);PE_StoreU32(0x8009D28Cu,8);
        memset(PE_Translate(node,128),0xA5,128);PE_StoreU32(node+0x24u,node+0x40u);PE_StoreU32(node+0x64u,0);PE_StoreU32(0x8009CDFCu,node);PE_StoreU16(0x8009D308u,0xFFFF);
        PE_StoreU8(0x8009CDB4u,0);PE_StoreU32(0x8009D2F0u,sender);PE_StoreU32(task,0x801B901Cu);PE_StoreU32(task+16u,1);
        PE_StoreU32(0x80070E04u,0);PE_StoreU32(0x80070E08u,8);
        for (unsigned i=0;i<17;i++)PE_StoreU32(0x80070E0Cu+i*4u,0);
        PE_StoreU32(0x80070E0Cu,(M34D_cases[k].draw*65536u+10u)/11u);
#define M34D_HASH() hit_camera_hash(M34D_ranges,sizeof(M34D_ranges)/sizeof(M34D_ranges[0]))
        func_80017018();hashes[n++]=M34D_HASH();
        func_80065400();hashes[n++]=M34D_HASH();
        PE_StoreU32(0x8009D2F0u,actor);
        for (unsigned i=0;i<w;i++) {PE_StoreU32(0x8009D300u,node);func_80017018();hashes[n++]=M34D_HASH();}
        PE_StoreU32(0x800B6A80u,16);PE_StoreU32(0x8009D300u,node);func_80017018();hashes[n++]=M34D_HASH();
#undef M34D_HASH
        for (unsigned i=0;i<n;i++) {
            if (hashes[i]!=M34D_cases[k].hash[i])fprintf(stderr,"M34 delivery%u stage%u %016llX/%016llX\n",k,i,(unsigned long long)hashes[i],(unsigned long long)M34D_cases[k].hash[i]);
            ASSERT(hashes[i]==M34D_cases[k].hash[i],"M0034I delivery/exit differs from original");
        }
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"M0034I delivery/exit completed natively");
    }
    for (unsigned k=0;k<sizeof(M34D_setters)/sizeof(M34D_setters[0]);k++) {
        static const uint32_t ranges[][2]={{0x150000u,0xC00u},{0x942ECu,2u}};
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        memset(PE_Translate(0x80150000u,0xC00u),0xA5,0xC00u);
        PE_StoreU32(0x8009D254u,0x80150000u);PE_StoreU32(0x80150000u,0x80150800u);PE_StoreU16(0x800942ECu,0xCAFEu);
        func_8002FF78(M34D_setters[k].tag,M34D_setters[k].value);
        ASSERT(hit_camera_hash(ranges,2)==M34D_setters[k].hash,"Aya setter differs from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"Aya setter completed natively");
    }
    free(seed);PASS();
}
