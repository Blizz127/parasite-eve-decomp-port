#include "retail_club_effect_cases.h"
static void test_DAY1_club_effect(void)
{
    TEST_RETAIL_DISC1("DAY1_club_effect"); TEST_RETAIL_FIXUPS(RETAILFIX_club_effect); TEST_RETAIL_FIXUPS(RETAILFIX_hit_draw);
    for (unsigned k=0;k<sizeof(DAY1_club_cases)/sizeof(DAY1_club_cases[0]);k++) {
        const uint32_t *a=DAY1_club_cases[k].args;
        uint32_t result=0;
        uint64_t hash=UINT64_C(14695981039346656037);
        ResetTestState();
        const uint32_t (*common)[2]=DAY1_club_cases[k].family?DAY1_club_common1:DAY1_club_common0;
        unsigned count=DAY1_club_cases[k].family?
            sizeof(DAY1_club_common1)/sizeof(DAY1_club_common1[0]):
            sizeof(DAY1_club_common0)/sizeof(DAY1_club_common0[0]);
        for (unsigned i=0;i<count;i++)
            PE_StoreU32(DAY1_hit_draw_address(common[i][0]),common[i][1]);
        for (unsigned i=DAY1_club_cases[k].first;i<DAY1_club_cases[k].end;i++)
            PE_StoreU32(DAY1_hit_draw_address(DAY1_club_patches[i][0]),DAY1_club_patches[i][1]);
        func_80071A64(DAY1_club_cases[k].seed);
        g_pe_gte.ofx=160<<16;g_pe_gte.ofy=112<<16;g_pe_gte.h=256;g_pe_gte.zsf3=0x155;
        switch (DAY1_club_cases[k].entry) {
        case 0:result=func_800CE084(a[0]);break;
        case 1:PE_WeaponCallback(0x800CE1FCu,a[0],a[1],a[2]);break;
        case 2:PE_WeaponCallback(0x800CE2B4u,a[0],a[1],a[2]);break;
        case 3:PE_WeaponCallback(0x800CE3B4u,a[0],a[1],a[2]);break;
        case 4:result=func_800CE16C(a[0]);break;
        case 5:result=func_800CE144((int32_t)a[0]);break;
        case 6:result=func_8006F6D4(a[0],a[1],a[2],a[3],a[4],a[5]);break;
        case 7:PE_WeaponCallback(0x800CE464u,a[0],a[1],a[2]);break;
        case 8:PE_WeaponCallback(0x800CE470u,a[0],a[1],a[2]);break;
        case 9:result=func_8006F39C(a[0],a[1]);break;
        }
        for (unsigned i=0;i<sizeof(DAY1_club_ranges)/sizeof(DAY1_club_ranges[0]);i++)
            for (unsigned j=0;j<DAY1_club_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(DAY1_hit_draw_address(DAY1_club_ranges[i][0]+j)))*UINT64_C(1099511628211);
        if (hash!=DAY1_club_cases[k].hash || result!=DAY1_club_cases[k].result || PE_Port_ShouldStop()) {
            fprintf(stderr,"club case%u result%X/%X hash%016llX/%016llX\n",k,result,DAY1_club_cases[k].result,
                (unsigned long long)hash,(unsigned long long)DAY1_club_cases[k].hash);
            if (getenv("PE_CLUB_DUMP")) {
                char path[100];snprintf(path,sizeof(path),"/tmp/pe-club-native-%u.bin",k);
                FILE *out=fopen(path,"wb");
                if(out){for(unsigned i=0;i<0x200000u;i++)fputc(PE_LoadU8(DAY1_hit_draw_address(i)),out);fclose(out);}
            }
        }
        ASSERT(result==DAY1_club_cases[k].result,"club result differs from original instructions");
        ASSERT(hash==DAY1_club_cases[k].hash,"club bytes differ from original instructions");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"club effects execute natively");
    }
    ResetTestState();PASS();
}
