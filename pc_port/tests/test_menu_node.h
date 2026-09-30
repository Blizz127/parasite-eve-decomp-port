#include "retail_menu_node_cases.h"

static void test_NAM1_retail_menu_nodes(void)
{
    unsigned k,i;
    TEST("NAM1_retail_menu_nodes");
    for (k=0;k<sizeof(NAM1_menu_cases)/sizeof(NAM1_menu_cases[0]);k++) {
        const uint32_t *a=NAM1_menu_cases[k].args;
        uint32_t result=0;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(NAM1_menu_common)/sizeof(NAM1_menu_common[0]);i++)
            PE_StoreU32(0x80000000u+NAM1_menu_common[i][0],NAM1_menu_common[i][1]);
        for (i=NAM1_menu_cases[k].first;i<NAM1_menu_cases[k].end;i++)
            PE_StoreU32(0x80000000u+NAM1_menu_patches[i][0],NAM1_menu_patches[i][1]);
        switch (NAM1_menu_cases[k].entry) {
        case 0:result=func_80062D2C(a[0],a[1],a[2],a[3]);break;
        case 1:result=func_8006322C(a[0],a[1],a[2]);break;
        case 2:result=func_80062A34(a[0],a[1]);break;
        case 3:result=func_800631DC();break;
        case 4:func_80064AC0(a[0]);break;
        case 5:func_80064D08(a[0]);break;
        case 6:result=func_8005DA8C(a[0]);break;
        case 7:result=func_8005DAB4(a[0]);break;
        }
        hash=hit_camera_hash(NAM1_menu_ranges,sizeof(NAM1_menu_ranges)/sizeof(NAM1_menu_ranges[0]));
        if (hash!=NAM1_menu_cases[k].hash || result!=NAM1_menu_cases[k].result) {
            fprintf(stderr,"menu node %u result %08X/%08X hash %016llX/%016llX\n",k,
                result,NAM1_menu_cases[k].result,(unsigned long long)hash,(unsigned long long)NAM1_menu_cases[k].hash);
            if (getenv("PE_NAM1_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-nam1-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(result==NAM1_menu_cases[k].result,"menu node return differs from original");
        ASSERT(hash==NAM1_menu_cases[k].hash,"menu node memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"menu node call graph executes natively");
    }
    PASS();
}
