#include "retail_room_cleanup_cases.h"
static void test_SEW16_room_cleanup(void)
{
    TEST("SEW16_room_cleanup");
    for (unsigned k=0; k<sizeof(SEW16_cases)/sizeof(SEW16_cases[0]); k++) {
        uint64_t hash=UINT64_C(1469598103934665603);
        ResetTestState();
        for (unsigned i=0; i<sizeof(SEW16_common)/sizeof(SEW16_common[0]); i++)
            PE_StoreU32(0x80000000u+SEW16_common[i][0],SEW16_common[i][1]);
        for (unsigned i=SEW16_cases[k].first; i<SEW16_cases[k].end; i++)
            PE_StoreU32(0x80000000u+SEW16_patches[i][0],SEW16_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        func_800696F0();
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        for (unsigned i=0; i<sizeof(SEW16_ranges)/sizeof(SEW16_ranges[0]); i++)
            for (unsigned j=0; j<SEW16_ranges[i][1]; j++)
                hash=(hash^PE_LoadU8(0x80000000u+SEW16_ranges[i][0]+j))*UINT64_C(1099511628211);
        ASSERT(hash==SEW16_cases[k].hash,"room cleanup differs from original instructions");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"room finalizers execute natively");
    }
    PASS();
}
