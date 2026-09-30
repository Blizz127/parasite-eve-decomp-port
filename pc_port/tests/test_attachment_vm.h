#include "retail_attachment_vm_cases.h"
static void test_DAY1_attachment_vm(void)
{
    TEST_RETAIL_DISC1("DAY1_attachment_vm"); TEST_RETAIL_FIXUPS(RETAILFIX_attachment_vm);
    for (unsigned k=0;k<sizeof(ATT_cases)/sizeof(ATT_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(ATT_common)/sizeof(ATT_common[0]);i++)
            PE_StoreU32(0x80000000u+ATT_common[i][0],ATT_common[i][1]);
        for (unsigned i=ATT_cases[k].first;i<ATT_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ATT_patches[i][0],ATT_patches[i][1]);
        func_80017018();
        hash=hit_camera_hash(ATT_ranges,sizeof(ATT_ranges)/sizeof(ATT_ranges[0]));
        if (hash!=ATT_cases[k].hash)
            fprintf(stderr,"attachment VM %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)ATT_cases[k].hash);
        ASSERT(hash==ATT_cases[k].hash,"attachment actor state differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"attachment VM graph executes natively");
    }
    PASS();
}
