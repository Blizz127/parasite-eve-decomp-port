#include "retail_hit_draw_cases.h"
static pe_addr_t DAY1_hit_draw_address(uint32_t offset)
{ return offset<0x400u?0x1F800000u+offset:0x80000000u+offset; }
static void test_DAY1_hit_draw(void)
{
    unsigned k,i,j;
    TEST_RETAIL_DISC1("DAY1_hit_draw"); TEST_RETAIL_FIXUPS(RETAILFIX_hit_draw);
    for (k=0;k<sizeof(DAY1_hit_draw_cases)/sizeof(DAY1_hit_draw_cases[0]);k++) {
        const uint32_t *a=DAY1_hit_draw_cases[k].args;
        uint64_t hash=UINT64_C(14695981039346656037);
        ResetTestState();
        for (i=0;i<sizeof(DAY1_hit_draw_common)/sizeof(DAY1_hit_draw_common[0]);i++)
            PE_StoreU32(DAY1_hit_draw_address(DAY1_hit_draw_common[i][0]),DAY1_hit_draw_common[i][1]);
        for (i=DAY1_hit_draw_cases[k].first;i<DAY1_hit_draw_cases[k].end;i++)
            PE_StoreU32(DAY1_hit_draw_address(DAY1_hit_draw_patches[i][0]),DAY1_hit_draw_patches[i][1]);
        g_pe_gte.ofx=160<<16;g_pe_gte.ofy=112<<16;g_pe_gte.h=256;g_pe_gte.zsf3=0x155;
        switch (DAY1_hit_draw_cases[k].entry) {
        case 0:func_800C3B04(a[0]);break;
        case 1:PE_WeaponCallback(0x800CDD0Cu,a[0],a[1],a[2]);break;
        default:PE_WeaponCallback(0x800CDE90u,a[0],a[1],a[2]);break;
        }
        for (i=0;i<sizeof(DAY1_hit_draw_ranges)/sizeof(DAY1_hit_draw_ranges[0]);i++)
            for (j=0;j<DAY1_hit_draw_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(DAY1_hit_draw_address(DAY1_hit_draw_ranges[i][0]+j)))*UINT64_C(1099511628211);
        if (hash!=DAY1_hit_draw_cases[k].hash) {
            fprintf(stderr,"hit draw case%u hash%016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)DAY1_hit_draw_cases[k].hash);
            if (getenv("PE_DAY1_HIT_DRAW_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"local/live/hit-draw-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(DAY1_hit_draw_address(i)),out);fclose(out);}
            }
        }
        ASSERT(hash==DAY1_hit_draw_cases[k].hash,"hit draw RAM differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"hit draw executes natively");
    }
    PASS();
}
