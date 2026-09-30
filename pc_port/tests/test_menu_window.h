#include "retail_menu_window_cases.h"

static void test_NAM6_retail_menu_window(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("NAM6_retail_menu_window"); TEST_RETAIL_FIXUPS(RETAILFIX_menu_window);
    for (k=0;k<sizeof(NAM6_menu_window_cases)/sizeof(NAM6_menu_window_cases[0]);k++) {
        const uint32_t *a=NAM6_menu_window_cases[k].args;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(NAM6_menu_window_common)/sizeof(NAM6_menu_window_common[0]);i++)
            PE_StoreU32(0x80000000u+NAM6_menu_window_common[i][0],NAM6_menu_window_common[i][1]);
        for (i=NAM6_menu_window_cases[k].first;i<NAM6_menu_window_cases[k].end;i++)
            PE_StoreU32(0x80000000u+NAM6_menu_window_patches[i][0],NAM6_menu_window_patches[i][1]);
        for (i=0;i<NAM6_menu_window_cases[k].frame;i++) PE_GPU_VBlankStep();
        switch (NAM6_menu_window_cases[k].entry) {
        case 0:func_8006153C(a[0],a[1],(int32_t)a[2],a[3]);break;
        case 1:func_80061878(a[0],a[1]);break;
        case 2:func_80061C34(a[0],a[1],a[2],a[3]);break;
        case 3:func_80062090(a[0],a[1],a[2]);break;
        case 4:func_800622BC(a[0],a[1],a[2],a[3]);break;
        case 5:func_80075B4C(a[0],a[1]);break;
        case 6:func_80075B84(a[0],a[1]);break;
        }
        hash=hit_camera_hash(NAM6_menu_window_ranges,sizeof(NAM6_menu_window_ranges)/sizeof(NAM6_menu_window_ranges[0]));
        if (hash!=NAM6_menu_window_cases[k].hash) {
            fprintf(stderr,"menu window %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)NAM6_menu_window_cases[k].hash);
            if (getenv("PE_NAM6_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-nam6-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==NAM6_menu_window_cases[k].hash,"menu window memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"menu window call graph executes natively");
        ASSERT(PE_GPU_VSyncQuery()==NAM6_menu_window_cases[k].frame,"drawing must not advance VBlank time");
    }
    PASS();
}

static void test_NAM6_menu_frame_clock(void)
{
    unsigned i;
    TEST("NAM6_menu_frame_clock");
    ResetTestState();
    ASSERT(PE_GPU_VSyncQuery()==0u,"VBlank clock starts at zero");
    HostFB_VSync(2);
    ASSERT(PE_GPU_VSyncQuery()==2u,"30 Hz game frame waits for two VBlanks");
    for (i=0;i<100;i++) ASSERT(PE_GPU_VSyncQuery()==2u,"reading clock does not advance it");
    HostFB_VSync(-1);HostFB_VSync(1);
    ASSERT(PE_GPU_VSyncQuery()==2u,"query modes do not advance VBlank time");
    HostFB_VSync(0);HostFB_VSync(4);
    ASSERT(PE_GPU_VSyncQuery()==7u,"waiting modes advance the shared clock");
    PASS();
}
