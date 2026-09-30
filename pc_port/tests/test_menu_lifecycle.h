#include "retail_menu_lifecycle_cases.h"

static void test_INV1_retail_menu_lifecycle(void)
{
    unsigned k,i;
    TEST("INV1_retail_menu_lifecycle");
    for (k=0;k<sizeof(INV1_menu_lifecycle_cases)/sizeof(INV1_menu_lifecycle_cases[0]);k++) {
        const uint32_t *a=INV1_menu_lifecycle_cases[k].args;
        uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(INV1_menu_lifecycle_common)/sizeof(INV1_menu_lifecycle_common[0]);i++)
            PE_StoreU32(0x80000000u+INV1_menu_lifecycle_common[i][0],INV1_menu_lifecycle_common[i][1]);
        for (i=INV1_menu_lifecycle_cases[k].first;i<INV1_menu_lifecycle_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV1_menu_lifecycle_patches[i][0],INV1_menu_lifecycle_patches[i][1]);
        switch (INV1_menu_lifecycle_cases[k].entry) {
        case 0:func_800647D0(a[0],(int32_t)a[1]);break;
        case 1:func_8006269C(a[0]);break;
        case 2:func_80062F3C(a[0]);break;
        case 3:func_80064E90(a[0]);break;
        case 4:func_80064A54(a[0]);break;
        }
        hash=hit_camera_hash(INV1_menu_lifecycle_ranges,sizeof(INV1_menu_lifecycle_ranges)/sizeof(INV1_menu_lifecycle_ranges[0]));
        if (hash!=INV1_menu_lifecycle_cases[k].hash)
            fprintf(stderr,"menu lifecycle %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)INV1_menu_lifecycle_cases[k].hash);
        ASSERT(hash==INV1_menu_lifecycle_cases[k].hash,"menu lifecycle memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"menu lifecycle graph executes natively");
    }
    PASS();
}
