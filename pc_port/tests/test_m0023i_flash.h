#include "retail_m0023i_flash_cases.h"

static pe_addr_t DAY1_flash_address(uint32_t offset)
{ return offset<0x400u?0x1F800000u+offset:0x80000000u+offset; }

static uint64_t DAY1_flash_hash(void)
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
    for (i=0;i<sizeof(DAY1_flash_ranges)/sizeof(DAY1_flash_ranges[0]);i++) {
        for (j=0;j<DAY1_flash_ranges[i][1];j++) {
            uint32_t a=DAY1_flash_ranges[i][0]+j;
            uint8_t value=PE_LoadU8(DAY1_flash_address(a));
            if (a>=base && a<base+size && ignored[a-base]) value=0;
            hash=(hash^value)*UINT64_C(1099511628211);
        }
    }
    return hash;
}

static void test_DAY1_retail_m0023i_flash(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("DAY1_retail_m0023i_flash"); TEST_RETAIL_FIXUPS(RETAILFIX_m0023i_flash);
    for (k=0;k<sizeof(DAY1_flash_cases)/sizeof(DAY1_flash_cases[0]);k++) {
        const uint32_t *a=DAY1_flash_cases[k].args;
        uint32_t result=0;uint64_t hash;
        ResetTestState();func_80071A64(DAY1_flash_cases[k].seed);
        for (i=0;i<sizeof(DAY1_flash_common)/sizeof(DAY1_flash_common[0]);i++)
            PE_StoreU32(DAY1_flash_address(DAY1_flash_common[i][0]),DAY1_flash_common[i][1]);
        for (i=DAY1_flash_cases[k].first;i<DAY1_flash_cases[k].end;i++)
            PE_StoreU32(DAY1_flash_address(DAY1_flash_patches[i][0]),DAY1_flash_patches[i][1]);
        PE_GTE_LoadRT33(0x80148000u);g_pe_gte.ofx=160*65536;g_pe_gte.ofy=112*65536;
        g_pe_gte.h=256;g_pe_gte.dqa=-0x1062;
        g_pe_gte.dqb=0x1400000;g_pe_gte.zsf3=0x155;
        int16_t retained[3];
        for(i=0;i<3;i++)retained[i]=(int16_t)PE_LoadU16(0x801FEFB8u+i*2u);
        result=(uint32_t)PE_M0023I_Flash((int32_t)a[0],a[1],retained);
        if(DAY1_flash_cases[k].entry==1) {
            ASSERT(result==0,"first paired flash return differs from original");
            result=(uint32_t)PE_M0023I_Flash(2,0x80150140u,retained);
        }
        for(i=0;i<3;i++)PE_StoreU16(0x801FEFB8u+i*2u,(uint16_t)retained[i]);
        hash=DAY1_flash_hash();
        if (hash!=DAY1_flash_cases[k].hash || result!=DAY1_flash_cases[k].result) {
            fprintf(stderr,"M0023I flash %u: result %08X/%08X hash %016llX/%016llX\n",k,
                result,DAY1_flash_cases[k].result,(unsigned long long)hash,
                (unsigned long long)DAY1_flash_cases[k].hash);
            if (getenv("PE_DAY1_FLASH_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-day1-flash-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(DAY1_flash_address(i)),out);fclose(out);}
            }
        }
        ASSERT(result==DAY1_flash_cases[k].result,"M0023I flash return differs from original");
        ASSERT(hash==DAY1_flash_cases[k].hash,"M0023I flash memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"M0023I flash graph executes natively");
    }
    ResetTestState();
    for(i=0;i<sizeof(DAY1_flash_common)/sizeof(DAY1_flash_common[0]);i++)
        PE_StoreU32(DAY1_flash_address(DAY1_flash_common[i][0]),DAY1_flash_common[i][1]);
    PE_StoreU16(0x80150104u,0);PE_StoreU32(0x8009CDD8u,0);
    g_pe_strict_effect_stack=1;
    (void)PE_EffectCallback(0x8018F710u,2,0x80150100u,0);
    g_pe_strict_effect_stack=0;
    ASSERT(PE_Port_ShouldStop(),"strict mode: unknown original caller stack must remain an explicit boundary");
    ASSERT(PE_LoadU32(0x8009CDD8u)==0,"strict mode: unknown stack position must not emit invented geometry");
    /* Day-1 shim (DAY1_FIDELITY_GAPS.md): the same call keeps the game running. */
    ResetTestState();
    for(i=0;i<sizeof(DAY1_flash_common)/sizeof(DAY1_flash_common[0]);i++)
        PE_StoreU32(DAY1_flash_address(DAY1_flash_common[i][0]),DAY1_flash_common[i][1]);
    PE_StoreU16(0x80150104u,0);PE_StoreU32(0x8009CDD8u,0);
    (void)PE_EffectCallback(0x8018F710u,2,0x80150100u,0);
    ASSERT(!PE_Port_ShouldStop(),"Day-1 shim: unresolved F710 stack position must not stop the port");
    ResetTestState();
    PASS();
}
