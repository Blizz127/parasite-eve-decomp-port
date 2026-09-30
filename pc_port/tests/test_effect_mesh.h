#include "retail_effect_mesh_cases.h"

static pe_addr_t ATK28_mesh_address(uint32_t offset)
{ return offset<0x400u?0x1F800000u+offset:0x80000000u+offset; }

static void test_ATK28_retail_effect_mesh(void)
{
    unsigned k,i,j;
    TEST("ATK28_retail_effect_mesh");
    for (k=0;k<sizeof(ATK28_mesh_cases)/sizeof(ATK28_mesh_cases[0]);k++) {
        const uint32_t *a=ATK28_mesh_cases[k].args;
        uint64_t hash=UINT64_C(14695981039346656037);
        ResetTestState();
        for (i=0;i<sizeof(ATK28_mesh_common)/sizeof(ATK28_mesh_common[0]);i++)
            PE_StoreU32(ATK28_mesh_address(ATK28_mesh_common[i][0]),ATK28_mesh_common[i][1]);
        for (i=ATK28_mesh_cases[k].first;i<ATK28_mesh_cases[k].end;i++)
            PE_StoreU32(ATK28_mesh_address(ATK28_mesh_patches[i][0]),ATK28_mesh_patches[i][1]);
        g_pe_gte.ofx=160<<16;g_pe_gte.ofy=112<<16;g_pe_gte.h=256;g_pe_gte.zsf3=0x155;
        switch (ATK28_mesh_cases[k].entry) {
        case 0:func_800C71E4(a[0],a[1]);break;
        case 1:func_800C6D5C(a[0],a[1],a[2]);break;
        case 2:func_800C6EC0(a[0],a[1]);break;
        case 3:func_800C6ED8(a[0]);break;
        case 4:func_800C6EF8(a[0]);break;
        case 5:func_800C6F4C(a[0]);break;
        case 6:func_800C6FA0(a[0],a[1]);break;
        case 7:func_800C70EC(a[0],(int32_t)a[1],(int32_t)a[2],(int32_t)a[3]);break;
        }
        for (i=0;i<sizeof(ATK28_mesh_ranges)/sizeof(ATK28_mesh_ranges[0]);i++)
            for (j=0;j<ATK28_mesh_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(ATK28_mesh_address(ATK28_mesh_ranges[i][0]+j)))*UINT64_C(1099511628211);
        if (hash!=ATK28_mesh_cases[k].hash) {
            fprintf(stderr,"effect mesh %u: hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)ATK28_mesh_cases[k].hash);
            if (getenv("PE_ATK28_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk28-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(ATK28_mesh_address(i)),out);fclose(out);}
            }
        }
        ASSERT(hash==ATK28_mesh_cases[k].hash,"effect mesh memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"effect mesh executes natively");
    }
    PASS();
}
