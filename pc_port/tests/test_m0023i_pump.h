#include "retail_m0023i_pump_cases.h"

static pe_addr_t DAY1_pump_address(uint32_t offset)
{ return offset<0x400u?0x1F800000u+offset:0x80000000u+offset; }

static uint64_t DAY1_pump_hash(void)
{
    uint8_t linked[8192]={0},ignored[8192]={0};
    uint32_t bank=PE_LoadU32(0x8009CDDCu),base=0x160000u+bank*0x2000u;
    uint32_t size=PE_LoadU32(0x8009CDD8u),i,j,p;
    uint64_t hash=UINT64_C(14695981039346656037);
    if (size>sizeof(linked)) return 0;
    for (i=0;i<4096;i++) {
        p=PE_LoadU32(0x80164000u+bank*0x4000u+i*4u)&0xFFFFFFu;
        while (p>=base && p<base+size && !linked[p-base]) {
            linked[p-base]=1;p=PE_LoadU32(0x80000000u+p)&0xFFFFFFu;
        }
    }
    for (p=0;p<size;) {
        uint32_t length=PE_LoadU8(0x80000000u+base+p+3u);
        if (!length) {p+=4u;continue;}
        if (length!=1u && length!=6u && length!=7u && length!=8u && length!=9u && length!=12u) return 0;
        if (p+(length+1u)*4u>size) return 0;
        if (length==6u) {ignored[p+15u]=1;ignored[p+23u]=1;}
        if (length==8u) {
            ignored[p+15u]=1;ignored[p+23u]=1;ignored[p+31u]=1;
            if (!linked[p]) for(i=32;i<36;i++)ignored[p+i]=1;
        }
        if ((length==1u || length==6u || length==8u) && !linked[p]) {
            ignored[p]=1;ignored[p+1u]=1;ignored[p+2u]=1;
        }
        if (p+(length+1u)*4u>size) return 0;
        if (length==9u && (PE_LoadU8(0x80000000u+base+p+7u)&0xFCu)==0x2Cu) {
            ignored[p+30u]=1;ignored[p+31u]=1;ignored[p+38u]=1;ignored[p+39u]=1;
            if (!linked[p]) {
                ignored[p]=1;ignored[p+1u]=1;ignored[p+2u]=1;
                for (i=32;i<36;i++) ignored[p+i]=1;
            }
        }
        p+=(length+1u)*4u;
    }
    for (i=0;i<sizeof(DAY1_pump_ranges)/sizeof(DAY1_pump_ranges[0]);i++) {
        for (j=0;j<DAY1_pump_ranges[i][1];j++) {
            uint32_t a=DAY1_pump_ranges[i][0]+j;
            uint8_t value=PE_LoadU8(DAY1_pump_address(a));
            if (a>=base && a<base+size && ignored[a-base]) value=0;
            hash=(hash^value)*UINT64_C(1099511628211);
        }
    }
    return hash;
}

static void test_DAY1_retail_m0023i_pump(void)
{
    TEST_RETAIL_DISC1("DAY1_retail_m0023i_pump"); TEST_RETAIL_FIXUPS(RETAILFIX_m0023i_pump);
    for(unsigned k=0;k<sizeof(DAY1_pump_cases)/sizeof(DAY1_pump_cases[0]);k++) {
        unsigned i,j;
        if(!DAY1_pump_cases[k].frame) {
            ResetTestState();
            for(i=0;i<sizeof(DAY1_pump_common)/sizeof(DAY1_pump_common[0]);i++)
                PE_StoreU32(DAY1_pump_address(DAY1_pump_common[i][0]),DAY1_pump_common[i][1]);
            for(i=DAY1_pump_cases[k].first;i<DAY1_pump_cases[k].end;i++)
                PE_StoreU32(DAY1_pump_address(DAY1_pump_patches[i][0]),DAY1_pump_patches[i][1]);
        }
        D_8009D1A0=DAY1_pump_cases[k].flags;PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009CDD8u,0);
        for(i=0;i<0x4000u;i+=4u)PE_StoreU32(0x80160000u+i,0);
        for(i=0;i<2;i++)for(j=0;j<4096;j++)PE_StoreU32(0x80164000u+i*0x4000u+j*4u,0xABFFFFFFu);
        func_80071A64(1);PE_GTE_LoadRT33(0x80148000u);
        for(i=0;i<3;i++)g_pe_gte.tr[i]=0;
        g_pe_gte.ofx=160*65536;g_pe_gte.ofy=112*65536;g_pe_gte.h=256;
        g_pe_gte.dqa=-0x1062;g_pe_gte.dqb=0x1400000;g_pe_gte.zsf3=0x155;
        uint32_t result=(uint32_t)func_80069594();uint64_t hash=DAY1_pump_hash();
        if(hash!=DAY1_pump_cases[k].hash || result!=DAY1_pump_cases[k].result || PE_Port_ShouldStop()) {
            fprintf(stderr,"M0023I pump %u frame %u: result %X/%X hash %016llX/%016llX\n",
                DAY1_pump_cases[k].history,DAY1_pump_cases[k].frame,result,DAY1_pump_cases[k].result,
                (unsigned long long)hash,(unsigned long long)DAY1_pump_cases[k].hash);
            if(getenv("PE_DAY1_PUMP_DUMP")) {
                char path[100];snprintf(path,sizeof(path),"/tmp/pe-day1-pump-native-%u-%u.bin",DAY1_pump_cases[k].history,DAY1_pump_cases[k].frame);
                FILE *out=fopen(path,"wb");
                if(out) {for(i=0;i<0x200000u;i++)fputc(PE_LoadU8(DAY1_pump_address(i)),out);fclose(out);}
            }
        }
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"original effect-pump history must run natively");
        ASSERT(result==DAY1_pump_cases[k].result,"effect-pump return differs from original");
        ASSERT(hash==DAY1_pump_cases[k].hash,"effect-pump history differs from original");
    }
    /* A fresh RAM generation has no prior CE8F0 update at the borrowed
     * stack depth. Spawning a flash then drawing must retain the boundary. */
    ResetTestState();
    for(unsigned i=0;i<sizeof(DAY1_pump_common)/sizeof(DAY1_pump_common[0]);i++)
        PE_StoreU32(DAY1_pump_address(DAY1_pump_common[i][0]),DAY1_pump_common[i][1]);
    D_8009D1A0=0x84u;(void)func_800D401C(1);
    g_pe_strict_effect_stack=1;
    (void)func_80069594();
    g_pe_strict_effect_stack=0;
    ASSERT(PE_Port_ShouldStop(),"strict mode: a fresh effect pump must not inherit unknown stack Z from a prior RAM generation");
    /* Day-1 shim (DAY1_FIDELITY_GAPS.md): same history keeps running. */
    ResetTestState();
    for(unsigned i=0;i<sizeof(DAY1_pump_common)/sizeof(DAY1_pump_common[0]);i++)
        PE_StoreU32(DAY1_pump_address(DAY1_pump_common[i][0]),DAY1_pump_common[i][1]);
    D_8009D1A0=0x84u;(void)func_800D401C(1);
    (void)func_80069594();
    ASSERT(!PE_Port_ShouldStop(),"Day-1 shim: a fresh effect pump keeps the port running");
    ResetTestState();PASS();
}
