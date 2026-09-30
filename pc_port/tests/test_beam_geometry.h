#include "retail_beam_geometry_cases.h"

static pe_addr_t ATK27_beam_address(uint32_t offset)
{ return offset<0x400u?0x1F800000u+offset:0x80000000u+offset; }

static void test_ATK27_retail_beam_geometry(void)
{
    unsigned k,i,j;
    TEST_RETAIL_DISC1("ATK27_retail_beam_geometry"); TEST_RETAIL_FIXUPS(RETAILFIX_beam_geometry);
    for (k=0;k<sizeof(ATK27_beam_cases)/sizeof(ATK27_beam_cases[0]);k++) {
        const uint32_t *a=ATK27_beam_cases[k].args;
        uint32_t result=0;uint64_t hash=UINT64_C(14695981039346656037);
        ResetTestState();
        for (i=0;i<sizeof(ATK27_beam_common)/sizeof(ATK27_beam_common[0]);i++)
            PE_StoreU32(ATK27_beam_address(ATK27_beam_common[i][0]),ATK27_beam_common[i][1]);
        for (i=ATK27_beam_cases[k].first;i<ATK27_beam_cases[k].end;i++)
            PE_StoreU32(ATK27_beam_address(ATK27_beam_patches[i][0]),ATK27_beam_patches[i][1]);
        switch (ATK27_beam_cases[k].entry) {
        case 0:result=func_80078004(a[0]);break;
        case 1:func_800CFAA8(a[0],a[1],a[2]);break;
        case 2:func_800CE9D4(a[0],a[1],a[2]);break;
        case 3:func_800CFB7C(a[0],(int32_t)a[1],a[2]);break;
        case 4:result=(uint32_t)func_800C62DC(a[0],a[1]);break;
        case 5:result=(uint32_t)func_800C6B20(a[0]);break;
        case 6:result=(uint32_t)func_800CEB8C(a[0],a[1],(int32_t)a[2]);break;
        }
        for (i=0;i<sizeof(ATK27_beam_ranges)/sizeof(ATK27_beam_ranges[0]);i++)
            for (j=0;j<ATK27_beam_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(ATK27_beam_address(ATK27_beam_ranges[i][0]+j)))*UINT64_C(1099511628211);
        if (hash!=ATK27_beam_cases[k].hash || result!=ATK27_beam_cases[k].result) {
            fprintf(stderr,"beam geometry %u: result %08X/%08X hash %016llX/%016llX\n",k,
                result,ATK27_beam_cases[k].result,(unsigned long long)hash,(unsigned long long)ATK27_beam_cases[k].hash);
            if (getenv("PE_ATK27_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk27-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(ATK27_beam_address(i)),out);fclose(out);}
            }
        }
        ASSERT(result==ATK27_beam_cases[k].result,"beam geometry return differs from original");
        ASSERT(hash==ATK27_beam_cases[k].hash,"beam geometry memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"beam geometry executes natively");
    }
    PASS();
}
