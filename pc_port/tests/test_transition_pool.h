#include "retail_transition_pool_cases.h"

static void test_DAY1_transition_pool(void)
{
    TEST("DAY1_transition_pool");
    for (unsigned k=0;k<sizeof(DAY1_pool_steps)/sizeof(DAY1_pool_steps[0]);k++) {
        const uint32_t *a=DAY1_pool_steps[k].args;
        uint32_t ret=0;
        uint64_t hash;
        if (DAY1_pool_steps[k].reset) {
            ResetTestState();
            for (unsigned j=0;j<sizeof(DAY1_pool_ranges)/sizeof(DAY1_pool_ranges[0]);j++)
                for (unsigned i=0;i<DAY1_pool_ranges[j][1];i++)
                    PE_StoreU8(0x80000000u+DAY1_pool_ranges[j][0]+i,
                               (uint8_t)(DAY1_pool_steps[k].seed+i*17u+(i>>8)));
            for (unsigned i=0;i<4u;i++) {
                PE_StoreU32(0x8014007Cu+i*4u,0x100u+i*0x40u);
                PE_StoreU32(0x801401B0u+i*0x40u,0xABCD0000u+i*0x1234u);
            }
        }
        switch (DAY1_pool_steps[k].entry) {
        case 0x80190998u: func_80190998(); break;
        case 0x80191580u: func_80191580(a[0]); break;
        case 0x801915DCu: ret=(uint32_t)func_801915DC(); break;
        case 0x80191678u: func_80191678(a[0]); break;
        case 0x80191740u: func_80191740(); break;
        case 0x80191754u: ret=func_80191754(); break;
        case 0x80190AECu: ret=func_80190AEC(a[0],a[1]); break;
        case 0x80190B78u: ret=func_80190B78(a[0],a[1],a[2]); break;
        case 0x80190C1Cu: ret=func_80190C1C(a[0],a[1],a[2],a[3],a[4],a[5],a[6]); break;
        case 0x80190D08u: func_80190D08(a[0]); break;
        case 0x8019959Cu: ret=func_8019959C(a[0],a[1]); break;
        default: ASSERT(0,"unknown pool fixture entry");
        }
        hash=hit_camera_hash(DAY1_pool_ranges,sizeof(DAY1_pool_ranges)/sizeof(DAY1_pool_ranges[0]));
        if (hash!=DAY1_pool_steps[k].hash || ret!=DAY1_pool_steps[k].ret)
            fprintf(stderr,"pool step %u entry %08X ret %08X/%08X RAM %016llX/%016llX\n",k,
                    DAY1_pool_steps[k].entry,ret,DAY1_pool_steps[k].ret,
                    (unsigned long long)hash,(unsigned long long)DAY1_pool_steps[k].hash);
        ASSERT(hash==DAY1_pool_steps[k].hash,"pool RAM differs from original history");
        ASSERT(ret==DAY1_pool_steps[k].ret,"pool result differs from original history");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"pool graph executes natively");
    }
    PASS();
}
