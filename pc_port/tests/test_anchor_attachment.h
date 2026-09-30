#include "retail_anchor_attachment_cases.h"
static void test_DAY1_anchor_attachment(void)
{
    TEST_RETAIL_DISC1("DAY1_anchor_attachment"); TEST_RETAIL_FIXUPS(RETAILFIX_anchor_attachment);
    for (unsigned k=0;k<sizeof(ANC_cases)/sizeof(ANC_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(ANC_common)/sizeof(ANC_common[0]);i++)
            PE_StoreU32(0x80000000u+ANC_common[i][0],ANC_common[i][1]);
        for (unsigned i=ANC_cases[k].first;i<ANC_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ANC_patches[i][0],ANC_patches[i][1]);
        g_pe_gte.h=256; g_pe_gte.ofx=160<<16; g_pe_gte.ofy=112<<16;
        func_80017018();
        hash=hit_camera_hash(ANC_ranges,sizeof(ANC_ranges)/sizeof(ANC_ranges[0]));
        if (hash!=ANC_cases[k].hash)
            fprintf(stderr,"anchor attachment VM %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)ANC_cases[k].hash);
        ASSERT(hash==ANC_cases[k].hash,"attachment actor state differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"anchor attachment VM graph executes natively");
    }
    PASS();
}
