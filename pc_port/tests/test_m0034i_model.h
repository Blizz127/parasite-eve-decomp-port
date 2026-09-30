#include "retail_m0034i_model_cases.h"
static void test_DAY1_m0034i_model(void)
{
    PE_Disc *disc;
    char err[256]={0};
    uint8_t *seed;
    const pe_addr_t model=0x801501B4u,obj=0x80197250u;
    TEST_RETAIL_DISC1("DAY1_m0034i_model");
    ResetTestState();
    disc=BTL6_OpenDisc1(err,sizeof(err));ASSERT(disc!=NULL,err);
    ASSERT(PE_GuestImage_LoadExe(disc,err,sizeof(err))==0,err);
    for (unsigned i=0;i<100;i++)
        ASSERT(PE_Disc_ReadUserSector(disc,M34_CHUNK_LBA+i,PE_Translate(0x8018EFE8u+i*2048u,2048)),"M0034I chunk sector");
    PE_Disc_Close(disc);
    ASSERT(PE_LoadU32(0x8018EFECu)==0x31D0Cu && PE_LoadU8(obj+2u)==52,"M0034I model identity");
    seed=malloc(PE_RAM_SIZE);ASSERT(seed!=NULL,"model seed allocation");
    memcpy(seed,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
    for (unsigned k=0;k<sizeof(M34_cases)/sizeof(M34_cases[0]);k++) {
        uint64_t hash;
        pe_addr_t end,clip;
        int skipped;
        ResetTestState();memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),seed,PE_RAM_SIZE);
        func_8003D050_prefix_cut(model,obj,0x80160000u,M34_cases[k].packets);
        func_8003D050_ptr14_cut(model,obj);
        end=func_8003D050_packets(model);
        skipped=func_8003D050_post_3d94c_skip_cut(model,end);
        func_800794C4(model+0x2Cu,model+0x34u);
        PE_StoreU8(model+0x8Cu,255u);func_8003C5D8(model,50);
        func_8003D050_epilogue_cut(model,skipped);
        hash=hit_camera_hash(M34_ranges,sizeof(M34_ranges)/sizeof(M34_ranges[0]));
        if (hash!=M34_cases[k].init)fprintf(stderr,"M34 init%u %016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)M34_cases[k].init);
        ASSERT(hash==M34_cases[k].init,"real model initialization differs from original");
        /* Supply command publication from the original package directory;
         * loader/relocation acceptance is outside this model comparison. */
        {
            pe_addr_t base=0x8018EFE8u,header=base+PE_LoadU32(base+4u);
            uint32_t packed=PE_LoadU32(header+0x10u);
            pe_addr_t entry=base+(packed&0x3FFFFFu);
            for (unsigned i=0;i<(packed>>22);i++,entry+=12u)
                PE_StoreU32(0x800B0E98u+PE_LoadU8(entry+11u)*192u+PE_LoadU8(entry+7u)*4u,
                            base+(PE_LoadU32(entry+4u)&0xFFFFFFu));
        }
        PE_StoreU8(0x8015000Cu,3u);func_8001A680_command_cut(0x80150000u,M34_cases[k].command);
        clip=PE_LoadU32(0x801501B0u);
        for (unsigned i=0;i<9;i++) {
            PE_StoreU16(0x800BEA40u+i*2u,i%4u?0:4096);
            g_pe_gte.lcm[i/3][i%3]=i%4u?0:4096;
        }
        PE_StoreU32(0x8009CDA0u,0x00808080u);
        func_8003D834(model,clip,(int)M34_cases[k].frame,0x800BEA40u);
        hash=hit_camera_hash(M34_ranges,sizeof(M34_ranges)/sizeof(M34_ranges[0]));
        if (hash!=M34_cases[k].pose)fprintf(stderr,"M34 pose%u %016llX/%016llX\n",k,(unsigned long long)hash,(unsigned long long)M34_cases[k].pose);
        ASSERT(hash==M34_cases[k].pose,"real model pose/packets differ from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"model graph native completion");
    }
    free(seed);PASS();
}
