#include "retail_actor_flag_vm_cases.h"
static void test_DAY1_actor_flag_vm(void)
{
    TEST_RETAIL_DISC1("DAY1_actor_flag_vm"); TEST_RETAIL_FIXUPS(RETAILFIX_actor_flag_vm);
    for(unsigned k=0;k<sizeof(actor_flag_cases)/sizeof(actor_flag_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(actor_flag_common)/sizeof(actor_flag_common[0]);i++)
            PE_StoreU32(0x80000000u+actor_flag_common[i][0],actor_flag_common[i][1]);
        for(unsigned i=actor_flag_cases[k].first;i<actor_flag_cases[k].end;i++)
            PE_StoreU32(0x80000000u+actor_flag_patches[i][0],actor_flag_patches[i][1]);
        func_80017018();
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned i=0;i<sizeof(actor_flag_ranges)/sizeof(actor_flag_ranges[0]);i++)
            for(unsigned j=0;j<actor_flag_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(0x80000000u+actor_flag_ranges[i][0]+j))*UINT64_C(1099511628211);
        if(hash!=actor_flag_cases[k].hash) fprintf(stderr,"actor flag VM case%u hash%016llX/%016llX\n",k,
            (unsigned long long)hash,(unsigned long long)actor_flag_cases[k].hash);
        ASSERT(hash==actor_flag_cases[k].hash,"actor flags, decoded operands and following wait match original VM");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"op13 executes natively");
    }
    ResetTestState();PASS();
}
