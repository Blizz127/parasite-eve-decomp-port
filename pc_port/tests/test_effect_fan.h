#include "retail_effect_fan_cases.h"

static uint64_t DAY1_fan_hash(void)
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
        if (length!=1u && length!=6u) {p+=4u;continue;}
        if (p+(length+1u)*4u>size) return 0;
        if (length==6u) {ignored[p+15u]=1;ignored[p+23u]=1;}
        if (!linked[p]) {ignored[p]=1;ignored[p+1u]=1;ignored[p+2u]=1;}
        p+=(length+1u)*4u;
    }
    for (i=0;i<sizeof(DAY1_fan_ranges)/sizeof(DAY1_fan_ranges[0]);i++) {
        for (j=0;j<DAY1_fan_ranges[i][1];j++) {
            uint32_t a=DAY1_fan_ranges[i][0]+j;
            uint8_t value=PE_LoadU8(0x80000000u+a);
            if (a>=base && a<base+size && ignored[a-base]) value=0;
            hash=(hash^value)*UINT64_C(1099511628211);
        }
    }
    return hash;
}

static void test_DAY1_retail_effect_fan(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("DAY1_retail_effect_fan"); TEST_RETAIL_FIXUPS(RETAILFIX_effect_fan);
    for (k=0;k<sizeof(DAY1_fan_cases)/sizeof(DAY1_fan_cases[0]);k++) {
        const uint32_t *a=DAY1_fan_cases[k].args;
        uint32_t result=0;uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(DAY1_fan_common)/sizeof(DAY1_fan_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_fan_common[i][0],DAY1_fan_common[i][1]);
        for (i=DAY1_fan_cases[k].first;i<DAY1_fan_cases[k].end;i++)
            PE_StoreU32(0x80000000u+DAY1_fan_patches[i][0],DAY1_fan_patches[i][1]);
        PE_GTE_LoadRT33(0x80148000u);g_pe_gte.ofx=160*65536;g_pe_gte.ofy=112*65536;
        g_pe_gte.h=256;g_pe_gte.dqa=-0x1062;g_pe_gte.dqb=0x1400000;g_pe_gte.zsf3=0x155;
        func_800D004C(a[0],(int32_t)a[1],(int32_t)a[2],(int32_t)a[3],a[4],(int32_t)a[5],
            (int32_t)a[6],a[7],a[8],(int32_t)a[9],(int32_t)a[10]);
        hash=DAY1_fan_hash();
        if (hash!=DAY1_fan_cases[k].hash || result!=DAY1_fan_cases[k].result) {
            fprintf(stderr,"D004C fan %u: result %08X/%08X hash %016llX/%016llX\n",k,
                result,DAY1_fan_cases[k].result,(unsigned long long)hash,
                (unsigned long long)DAY1_fan_cases[k].hash);
            if (getenv("PE_DAY1_FAN_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-day1-fan-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(result==DAY1_fan_cases[k].result,"D004C fan return differs from original");
        ASSERT(hash==DAY1_fan_cases[k].hash,"D004C fan memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"D004C fan graph executes natively");
    }
    PASS();
}
