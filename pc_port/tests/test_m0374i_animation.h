#include "retail_m0374i_animation_cases.h"
static void test_DAY2_m0374i_animation(void)
{
    PE_Disc *disc;char err[256]={0};uint8_t *seed;
    TEST_RETAIL_DISC1("DAY2_m0374i_animation");
    ResetTestState();disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);
    for (unsigned i=0;i<M374A_CHUNK_SECTORS;i++)
        ASSERT(PE_Disc_ReadUserSector(disc,M374A_CHUNK_LBA+i,PE_Translate(0x8018EFE8u+i*2048u,2048)),"M0374I chunk sector");
    PE_Disc_Close(disc);seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"M0374I seed allocation");
    memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for (unsigned k=0;k<sizeof(M374A_cases)/sizeof(M374A_cases[0]);k++) {
        const pe_addr_t sender=0x80150000u,actor=0x80150200u,task=0x80160000u,mail=task+0x40u,child=task+0x80u,old=task+0x100u;
        uint64_t hashes[5];unsigned linked=0;
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        memset(PE_Translate(sender,0x600),0,0x600);memset(PE_Translate(task,0x180),0,0x180);
        PE_StoreU32(sender+4u,actor);PE_StoreU16(sender+0x24u,0x1234);PE_StoreU32(sender+0x9Cu,0x801C4E04u);
        PE_StoreU8(actor+12u,2);PE_StoreU32(actor+0x9Cu,0x801C5840u);PE_StoreU32(actor+0xA8u,linked?old:0);
        PE_StoreU32(0x8009D20Cu,sender);PE_StoreU32(0x8009D2F0u,actor);PE_StoreU32(0x8009D1A0u,0);
        PE_StoreU8(0x800B0CE9u,0);
        for (unsigned c=0;c<2;c++) {
            pe_addr_t pc=0x801C5840u+c*32u;
            for (unsigned i=0;i<6;i++)PE_StoreU32(0x80154020u+i*4u,i<5?pc+8u+i*4u:0x800B6AC4u);
            func_80015DAC_default_cut(0x80154020u);
        }
        PE_StoreU32(0x80154000u,0x801C58C4u);PE_StoreU32(0x80154004u,0x801C58C8u);func_80017588(0x80154000u);
        PE_StoreU32(task,0x801C54B8u);PE_StoreU32(task+16u,1);PE_StoreU32(sender+0xA8u,task);
        PE_StoreU32(mail+0x24u,child);PE_StoreU32(child+0x24u,task+0xC0u);PE_StoreU32(0x8009CDFCu,mail);
        PE_StoreU16(0x8009D308u,(uint16_t)0);PE_StoreU8(0x8009CDB4u,0);PE_StoreU32(0x800A7918u,0xE4u);
        {
            pe_addr_t base=0x8018EFE8u,header=base+PE_LoadU32(base+4u),model=actor+0x1B4u,obj=0x8019690Cu,end;
            uint32_t packed=PE_LoadU32(header+0x10u);pe_addr_t entry=base+(packed&0x3FFFFFu);int skipped;
            for (unsigned i=0;i<(packed>>22);i++,entry+=12u)
                PE_StoreU32(0x800B0E98u+PE_LoadU8(entry+11u)*192u+PE_LoadU8(entry+7u)*4u,base+(PE_LoadU32(entry+4u)&0xFFFFFFu));
            func_8003D050_prefix_cut(model,obj,0x80170000u,M374A_cases[k].packets);
            func_8003D050_ptr14_cut(model,obj);end=func_8003D050_packets(model);
            skipped=func_8003D050_post_3d94c_skip_cut(model,end);func_800794C4(model+0x2Cu,model+0x34u);
            PE_StoreU8(model+0x8Cu,255u);func_8003C5D8(model,50);func_8003D050_epilogue_cut(model,skipped);
            entry=base+(PE_LoadU32(header+24u)&0x3FFFFFu);
            PE_StoreU32(0x800B1620u,base+(PE_LoadU32(entry+4u)&0xFFFFFFu));func_8001A918();
            PE_StoreU32(actor+0x1Cu,M374A_cases[k].speed);
        }
#define M374A_HASH() hit_camera_hash(M374A_ranges,sizeof(M374A_ranges)/sizeof(M374A_ranges[0]))
        hashes[0]=M374A_HASH();
        PE_StoreU32(0x8009D2F0u,sender);PE_StoreU32(0x8009D300u,task);func_80017018();func_80065400();hashes[1]=M374A_HASH();
        PE_StoreU32(0x8009D2F0u,actor);PE_StoreU32(0x8009D300u,mail);func_80017018();
        for (unsigned i=0;i<9;i++) {
            PE_StoreU16(0x800BEA40u+i*2u,i%4u?0:4096);
            g_pe_gte.lcm[i/3][i%3]=i%4u?0:4096;
        }
        PE_StoreU32(0x8009CDA0u,0x00808080u);
        func_8003D834(actor+0x1B4u,PE_LoadU32(actor+0x1B0u),(int)(PE_LoadU32(actor+0x14u)>>16),0x800BEA40u);
        hashes[2]=M374A_HASH();
        for (unsigned tick=0;tick<M374A_cases[k].ticks;tick++) {
            ASSERT(PE_LoadU32(child)==0x801C5D94u,"real clip remains in animation wait before final tick");
            func_8001A4AC(actor);PE_StoreU32(0x8009D300u,child);func_80017018();
        }
        ASSERT(PE_LoadU32(child)==0x801C5D9Cu && PE_LoadU32(actor+0x14u)==(122u<<16),"real clip releases animation wait at frame122");
        func_8003D834(actor+0x1B4u,PE_LoadU32(actor+0x1B0u),122,0x800BEA40u);hashes[3]=M374A_HASH();
        PE_StoreU32(0x8009D300u,child);func_80017018();hashes[4]=M374A_HASH();
        ASSERT(PE_LoadU32(child)==0x801C5DC0u,"scene waits for dialogue62 after animation and placement");
#undef M374A_HASH
        for (unsigned i=0;i<5;i++) {
            if(hashes[i]!=M374A_cases[k].hash[i])fprintf(stderr,"M374 delivery%u stage%u %016llX/%016llX\n",k,i,(unsigned long long)hashes[i],(unsigned long long)M374A_cases[k].hash[i]);
            ASSERT(hashes[i]==M374A_cases[k].hash[i],"M0374I model/animation continuation differs from original");
        }
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"M0374I animation completed natively to dialogue wait");
    }
    free(seed);PASS();
}
