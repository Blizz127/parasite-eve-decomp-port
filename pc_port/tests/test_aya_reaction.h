#include "retail_aya_reaction_cases.h"

static uint64_t ATK35_reaction_hash(void)
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
        if (length!=1u && length!=2u && length!=3u && length!=9u) {p+=4u;continue;}
        if (p+(length+1u)*4u>size) return 0;
        if (length==9u) {ignored[p+30u]=1;ignored[p+31u]=1;ignored[p+38u]=1;ignored[p+39u]=1;}
        if (length==9u && !linked[p]) {
            ignored[p]=1;ignored[p+1u]=1;ignored[p+2u]=1;
            for (i=32;i<36;i++) ignored[p+i]=1;
        }
        p+=(length+1u)*4u;
    }
    for (i=0;i<sizeof(ATK35_reaction_ranges)/sizeof(ATK35_reaction_ranges[0]);i++) {
        for (j=0;j<ATK35_reaction_ranges[i][1];j++) {
            uint32_t a=ATK35_reaction_ranges[i][0]+j;
            uint8_t value=PE_LoadU8(0x80000000u+a);
            if (a>=base && a<base+size && ignored[a-base]) value=0;
            hash=(hash^value)*UINT64_C(1099511628211);
        }
    }
    return hash;
}

static void test_ATK35_retail_aya_reaction(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("ATK35_retail_aya_reaction"); TEST_RETAIL_FIXUPS(RETAILFIX_aya_reaction);
    for (k=0;k<sizeof(ATK35_reaction_cases)/sizeof(ATK35_reaction_cases[0]);k++) {
        const uint32_t *a=ATK35_reaction_cases[k].args;
        uint32_t result=0;uint64_t hash;
        ResetTestState();func_80071A64(ATK35_reaction_cases[k].seed);
        for (i=0;i<sizeof(ATK35_reaction_common)/sizeof(ATK35_reaction_common[0]);i++)
            PE_StoreU32(0x80000000u+ATK35_reaction_common[i][0],ATK35_reaction_common[i][1]);
        for (i=ATK35_reaction_cases[k].first;i<ATK35_reaction_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ATK35_reaction_patches[i][0],ATK35_reaction_patches[i][1]);
        PE_GTE_LoadRT(0x80148000u);g_pe_gte.ofx=160*65536;g_pe_gte.ofy=112*65536;
        g_pe_gte.h=256;g_pe_gte.dqa=-0x1062;g_pe_gte.dqb=0x1400000;g_pe_gte.zsf3=0x155;
        switch (ATK35_reaction_cases[k].entry) {
        case 0:result=(uint32_t)PE_EffectCallback(0x800D751Cu,(int32_t)a[0],a[1],0);break;
        case 1:result=(uint32_t)PE_EffectCallback(0x800D71B8u,(int32_t)a[0],a[1],0);break;
        case 2:result=(uint32_t)PE_EffectCallback(0x800D70C0u,(int32_t)a[0],a[1],0);break;
        case 3:func_800D1DEC(a[0],a[1],(int32_t)a[2],(int32_t)a[3]);break;
        case 4:result=(uint32_t)func_800CE688(a[0]);break;
        case 5:result=(uint32_t)func_800CE78C(a[0]);break;
        }
        hash=ATK35_reaction_hash();
        if (hash!=ATK35_reaction_cases[k].hash || result!=ATK35_reaction_cases[k].result) {
            fprintf(stderr,"Aya reaction %u: result %08X/%08X hash %016llX/%016llX\n",k,
                result,ATK35_reaction_cases[k].result,(unsigned long long)hash,
                (unsigned long long)ATK35_reaction_cases[k].hash);
            if (getenv("PE_ATK35_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk35-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(result==ATK35_reaction_cases[k].result,"Aya reaction return differs from original");
        ASSERT(hash==ATK35_reaction_cases[k].hash,"Aya reaction memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"Aya reaction graph executes natively");
    }
    PASS();
}
