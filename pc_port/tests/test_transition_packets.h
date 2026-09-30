#include "retail_transition_packets_cases.h"

static void test_DAY1_transition_packets(void)
{
    TEST("DAY1_transition_packets");
    for (unsigned k=0;k<sizeof(DAY1_packets_steps)/sizeof(DAY1_packets_steps[0]);k++) {
        if (DAY1_packets_steps[k].reset) {
            ResetTestState();
            for (unsigned j=0;j<sizeof(DAY1_packets_ranges)/sizeof(DAY1_packets_ranges[0]);j++)
                for (unsigned i=0;i<DAY1_packets_ranges[j][1];i++)
                    PE_StoreU8(0x80000000u+DAY1_packets_ranges[j][0]+i,
                        (uint8_t)(DAY1_packets_steps[k].seed+i*17u+(i>>8)));
            PE_StoreU32(0x8019C1F0u,DAY1_packets_steps[k].flag);
            PE_StoreU32(0x8019C02Cu,0u);
            PE_StoreU32(0x8019C9C0u,DAY1_packets_steps[k].bank?0x80140000u:0x8019C1F8u);
            PE_StoreU32(0x80140004u,0x80142000u);
            PE_StoreU32(0x8019C1FCu,0x80142000u);
        }
        if (DAY1_packets_steps[k].entry==0x80195F6Cu) func_80195F6C();
        else func_80192740();
        uint64_t hash=hit_camera_hash(DAY1_packets_ranges,
                           sizeof(DAY1_packets_ranges)/sizeof(DAY1_packets_ranges[0]));
        if (hash!=DAY1_packets_steps[k].hash)
            fprintf(stderr,"transition packets %u entry %08X RAM %016llX/%016llX\n",k,
                DAY1_packets_steps[k].entry,(unsigned long long)hash,
                (unsigned long long)DAY1_packets_steps[k].hash);
        ASSERT(hash==DAY1_packets_steps[k].hash,"transition packets/OT differ from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"transition packets execute natively");
    }
    PASS();
}
