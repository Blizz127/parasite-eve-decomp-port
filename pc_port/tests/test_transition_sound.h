#include "retail_transition_sound_cases.h"

static void test_DAY1_transition_sound(void)
{
    TEST("DAY1_transition_sound");
    for (unsigned k=0;k<sizeof(DAY1_ts_cases)/sizeof(DAY1_ts_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(DAY1_ts_common)/sizeof(DAY1_ts_common[0]);i++)
            PE_StoreU32(0x80000000u+DAY1_ts_common[i][0],DAY1_ts_common[i][1]);
        for (unsigned i=DAY1_ts_cases[k].first;i<DAY1_ts_cases[k].end;i++)
            PE_StoreU32(0x80000000u+DAY1_ts_patches[i][0],DAY1_ts_patches[i][1]);
        memset(&g_pe_gte,0,sizeof(g_pe_gte));
        for (unsigned i=0;i<3u;i++) g_pe_gte.rt[i][i]=4096;
        g_pe_gte.tr[0]=11;g_pe_gte.tr[1]=-22;g_pe_gte.tr[2]=999;
        PE_StoreU32(0x8009C0C0u+0xA0u*4u,0x8008B698u);
        PE_StoreU32(0x8009C0C0u+0xA2u*4u,0x8008BA3Cu);
        for (unsigned i=0;i<6u;i++) PE_StoreU32(0x1F800200u+i*4u,0xABCD0000u+i);
        const uint32_t *args=DAY1_ts_cases[k].args;
        if (DAY1_ts_cases[k].entry==0x80191EFCu) {
            func_80191EFC(args[0],args[1]);
            ASSERT(g_pe_gte.tr[0]==11 && g_pe_gte.tr[1]==-22 && g_pe_gte.tr[2]==999,
                   "transition sound restores GTE translation");
            for (unsigned i=0;i<3u;i++) for (unsigned j=0;j<3u;j++)
                ASSERT(g_pe_gte.rt[i][j]==(i==j?4096:0),"transition sound restores rotation");
            for (unsigned i=0;i<6u;i++)
                ASSERT(PE_LoadU32(0x1F800200u+i*4u)==0xABCD0000u+i,"temporary scratchpad restored");
        } else if (DAY1_ts_cases[k].entry==0x800868F0u)
            func_800868F0(args[0],args[1],args[2]);   /* void: retail $v0 unread (port3 A1) */
        else
            func_80086A28(args[0],args[1],args[2]);
        hash=hit_camera_hash(DAY1_ts_ranges,sizeof(DAY1_ts_ranges)/sizeof(DAY1_ts_ranges[0]));
        if (hash!=DAY1_ts_cases[k].queued)
            fprintf(stderr,"transition sound %u queued %016llX/%016llX\n",k,
                    (unsigned long long)hash,(unsigned long long)DAY1_ts_cases[k].queued);
        ASSERT(hash==DAY1_ts_cases[k].queued,"original transition sound FIFO/state mismatch");
        func_8008CA84();
        hash=hit_camera_hash(DAY1_ts_ranges,sizeof(DAY1_ts_ranges)/sizeof(DAY1_ts_ranges[0]));
        if (hash!=DAY1_ts_cases[k].consumed)
            fprintf(stderr,"transition sound %u consumed %016llX/%016llX\n",k,
                    (unsigned long long)hash,(unsigned long long)DAY1_ts_cases[k].consumed);
        ASSERT(hash==DAY1_ts_cases[k].consumed,"original transition sound voice state mismatch");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"transition sound has no missing calls");
    }
    PASS();
}
