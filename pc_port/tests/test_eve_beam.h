#include "retail_eve_beam_cases.h"

static pe_addr_t ATK29_beam_address(uint32_t offset)
{ return offset<0x400u?0x1F800000u+offset:0x80000000u+offset; }

static uint64_t ATK29_beam_hash(void)
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
        if (length!=7u && length!=9u && length!=12u) return 0;
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
    for (i=0;i<sizeof(ATK29_beam_ranges)/sizeof(ATK29_beam_ranges[0]);i++) {
        for (j=0;j<ATK29_beam_ranges[i][1];j++) {
            uint32_t a=ATK29_beam_ranges[i][0]+j;
            uint8_t value=PE_LoadU8(ATK29_beam_address(a));
            if (a>=base && a<base+size && ignored[a-base]) value=0;
            hash=(hash^value)*UINT64_C(1099511628211);
        }
    }
    return hash;
}

static void test_ATK29_retail_eve_beam(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("ATK29_retail_eve_beam"); TEST_RETAIL_FIXUPS(RETAILFIX_eve_beam);
    for (k=0;k<sizeof(ATK29_beam_cases)/sizeof(ATK29_beam_cases[0]);k++) {
        const uint32_t *a=ATK29_beam_cases[k].args;
        uint32_t result=0;uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(ATK29_beam_common)/sizeof(ATK29_beam_common[0]);i++)
            PE_StoreU32(ATK29_beam_address(ATK29_beam_common[i][0]),ATK29_beam_common[i][1]);
        for (i=ATK29_beam_cases[k].first;i<ATK29_beam_cases[k].end;i++)
            PE_StoreU32(ATK29_beam_address(ATK29_beam_patches[i][0]),ATK29_beam_patches[i][1]);
        PE_GTE_LoadRT33(0x80148000u);g_pe_gte.ofx=160*65536;g_pe_gte.ofy=112*65536;
        g_pe_gte.h=256;g_pe_gte.dqa=(int16_t)ATK29_beam_cases[k].dqa;
        g_pe_gte.dqb=(int32_t)ATK29_beam_cases[k].dqb;g_pe_gte.zsf3=0x155;
        switch (ATK29_beam_cases[k].entry) {
        case 0:result=(uint32_t)func_8018FDC4((int32_t)a[0],a[1]);break;
        case 1:result=(uint32_t)func_8018FB84((int32_t)a[0],a[1]);break;
        case 2:func_800D2370(a[0],a[1],(int32_t)a[2],(int32_t)a[3],(int32_t)a[4],(int32_t)a[5],
            (int32_t)a[6],(int32_t)a[7],a[8],a[9],a[10],(int32_t)a[11],(int32_t)a[12]);break;
        case 3:result=(uint32_t)func_800D4704(a[0]);break;
        case 4:result=(uint32_t)func_800CE688(a[0]);break;
        case 5:result=(uint32_t)func_800CE78C(a[0]);break;
        }
        hash=ATK29_beam_hash();
        if (hash!=ATK29_beam_cases[k].hash || result!=ATK29_beam_cases[k].result) {
            fprintf(stderr,"Eve beam %u: result %08X/%08X hash %016llX/%016llX\n",k,
                result,ATK29_beam_cases[k].result,(unsigned long long)hash,
                (unsigned long long)ATK29_beam_cases[k].hash);
            if (getenv("PE_ATK29_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk29-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(ATK29_beam_address(i)),out);fclose(out);}
            }
        }
        ASSERT(result==ATK29_beam_cases[k].result,"Eve beam return differs from original");
        ASSERT(hash==ATK29_beam_cases[k].hash,"Eve beam memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"Eve beam graph executes natively");
    }
    PASS();
}
