#include "retail_eve_charge_cases.h"

static uint64_t ATK25_charge_hash(void)
{
    uint8_t linked[256]={0};
    uint32_t bank=PE_LoadU32(0x8009CDDCu),base=0x160000u+bank*0x2000u;
    uint32_t size=PE_LoadU32(0x8009CDD8u),i,j;
    uint64_t hash=UINT64_C(14695981039346656037);
    for (i=0;i<4096;i++) {
        uint32_t p=PE_LoadU32(0x80164000u+bank*0x4000u+i*4u)&0xFFFFFFu;
        while (p>=base && p<base+size && (p-base)/40u<256u && !linked[(p-base)/40u]) {
            linked[(p-base)/40u]=1; p=PE_LoadU32(0x80000000u+p)&0xFFFFFFu;
        }
    }
    for (i=0;i<sizeof(ATK25_charge_ranges)/sizeof(ATK25_charge_ranges[0]);i++) {
        for (j=0;j<ATK25_charge_ranges[i][1];j++) {
            uint32_t a=ATK25_charge_ranges[i][0]+j;
            uint8_t value=PE_LoadU8(0x80000000u+a);
            if (a>=base && a<base+size) {
                uint32_t offset=(a-base)%40u,index=(a-base)/40u;
                if (offset==30u || offset==31u || offset>=38u ||
                    (!linked[index] && (offset<3u || (offset>=32u && offset<36u)))) value=0;
            }
            hash=(hash^value)*UINT64_C(1099511628211);
        }
    }
    return hash;
}

static void test_ATK25_retail_eve_charge(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("ATK25_retail_eve_charge"); TEST_RETAIL_FIXUPS(RETAILFIX_eve_charge);
    for (k=0;k<sizeof(ATK25_charge_cases)/sizeof(ATK25_charge_cases[0]);k++) {
        const uint32_t *a=ATK25_charge_cases[k].args;
        uint32_t result=0;
        uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(ATK25_charge_common)/sizeof(ATK25_charge_common[0]);i++)
            PE_StoreU32(0x80000000u+ATK25_charge_common[i][0],ATK25_charge_common[i][1]);
        for (i=ATK25_charge_cases[k].first;i<ATK25_charge_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ATK25_charge_patches[i][0],ATK25_charge_patches[i][1]);
        PE_GTE_LoadRT33(0x80148000u);g_pe_gte.ofx=160*65536;g_pe_gte.ofy=112*65536;
        g_pe_gte.h=256;g_pe_gte.dqa=(int32_t)ATK25_charge_cases[k].dqa;
        g_pe_gte.dqb=(int32_t)ATK25_charge_cases[k].dqb;
        switch (ATK25_charge_cases[k].entry) {
        case 0:result=(uint32_t)func_8018F330((int32_t)a[0],a[1]);break;
        case 1:result=(uint32_t)func_8018F018((int32_t)a[0],a[1]);break;
        case 2:func_800CEE20(a[0],a[1],(int32_t)a[2],(int32_t)a[3],(int32_t)a[4],a[5],(int32_t)a[6],(int32_t)a[7],a[8]);break;
        case 3:func_800CF3AC(a[0],a[1],(int32_t)a[2]);break;
        case 4:func_800783E4(a[0],a[1],(int32_t)a[2],(int32_t)a[3],a[4]);break;
        case 5:func_80078554(a[0],a[1],(int32_t)a[2],(int32_t)a[3],a[4]);break;
        case 6:result=func_80078CC4(a[0],a[1]);break;
        case 7:func_800786E4(a[0]);break;
        case 8:result=(uint32_t)func_800CE688(a[0]);break;
        case 9:result=(uint32_t)func_800CE78C(a[0]);break;
        case 10:result=(uint32_t)func_800D4704(a[0]);break;
        }
        hash=ATK25_charge_hash();
        if (hash!=ATK25_charge_cases[k].hash || result!=ATK25_charge_cases[k].result) {
            fprintf(stderr,"Eve charge %u: result %08X/%08X hash %016llX/%016llX\n",k,
                result,ATK25_charge_cases[k].result,(unsigned long long)hash,
                (unsigned long long)ATK25_charge_cases[k].hash);
            if (getenv("PE_ATK25_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk25-native-%u.bin",k);out=fopen(path,"wb");
                if (out) { for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out); }
            }
        }
        ASSERT(result==ATK25_charge_cases[k].result,"Eve charge return differs from original");
        ASSERT(hash==ATK25_charge_cases[k].hash,"Eve charge memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"Eve charge graph executes natively");
    }
    PASS();
}
