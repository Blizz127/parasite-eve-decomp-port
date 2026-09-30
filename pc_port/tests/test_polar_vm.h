#include "retail_polar_vm_cases.h"
static void test_DAY1_polar_vm(void)
{
    TEST_RETAIL_DISC1("DAY1_polar_vm"); TEST_RETAIL_FIXUPS(RETAILFIX_polar_vm);
    for(unsigned k=0;k<sizeof(polar_vm_cases)/sizeof(polar_vm_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(polar_vm_common)/sizeof(polar_vm_common[0]);i++)
            PE_StoreU32(0x80000000u+polar_vm_common[i][0],polar_vm_common[i][1]);
        for(unsigned i=polar_vm_cases[k].first;i<polar_vm_cases[k].end;i++)
            PE_StoreU32(0x80000000u+polar_vm_patches[i][0],polar_vm_patches[i][1]);
        func_80017018();
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned i=0;i<sizeof(polar_vm_ranges)/sizeof(polar_vm_ranges[0]);i++)
            for(unsigned j=0;j<polar_vm_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(0x80000000u+polar_vm_ranges[i][0]+j))*UINT64_C(1099511628211);
        if(hash!=polar_vm_cases[k].hash) fprintf(stderr,"polar VM case%u hash%016llX/%016llX\n",k,
            (unsigned long long)hash,(unsigned long long)polar_vm_cases[k].hash);
        ASSERT(hash==polar_vm_cases[k].hash,"polar offsets, aliased operands and following wait match original VM");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"opDD executes natively");
    }
    ResetTestState();PASS();
}
