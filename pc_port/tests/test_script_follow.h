#include "retail_script_follow_cases.h"
static void test_DAY1_script_follow(void)
{
    TEST_RETAIL_DISC1("DAY1_script_follow"); TEST_RETAIL_FIXUPS(RETAILFIX_script_follow);
    for(unsigned k=0;k<sizeof(DAY1_follow_cases)/sizeof(DAY1_follow_cases[0]);k++) {
        if(!DAY1_follow_cases[k].frame) {
            ResetTestState();
            for(unsigned i=0;i<sizeof(DAY1_follow_common)/sizeof(DAY1_follow_common[0]);i++)
                PE_StoreU32(0x80000000u+DAY1_follow_common[i][0],DAY1_follow_common[i][1]);
            for(unsigned i=DAY1_follow_cases[k].first;i<DAY1_follow_cases[k].end;i++)
                PE_StoreU32(0x80000000u+DAY1_follow_patches[i][0],DAY1_follow_patches[i][1]);
        }
        unsigned motion=DAY1_follow_cases[k].motion,frame=DAY1_follow_cases[k].frame;
        PE_StoreU32(0x8009D300u,0x80140100u);D_8009D1A0=0;
        if(motion) {
            for(unsigned off=0x28u;off<=0x30u;off+=8u)
                PE_StoreU32(0x80145000u+off,PE_LoadU32(0x80145000u+off)+PE_LoadU32(0x80145040u+off));
            if(motion==2u) {
                PE_StoreU32(0x80145428u,(100u+frame*3u)<<16u);
                PE_StoreU32(0x80145430u,(100u-frame*2u)<<16u);
            }
            if(frame==3u && motion==3u)PE_StoreU32(0x80145498u,0x10u);
            if(frame==3u && motion==4u)PE_StoreU16(0x80145424u,0x1235u);
        }
        int result=0;
        if(DAY1_follow_cases[k].vm)func_80017018();else result=func_80013E84(0x80140000u);
        uint64_t hash=hit_camera_hash(DAY1_follow_ranges,sizeof(DAY1_follow_ranges)/sizeof(DAY1_follow_ranges[0]));
        if(hash!=DAY1_follow_cases[k].hash || (unsigned)result!=DAY1_follow_cases[k].result) {
            fprintf(stderr,"follow history%u frame%u result%d/%u hash%016llX/%016llX\n",
                DAY1_follow_cases[k].history,frame,result,DAY1_follow_cases[k].result,
                (unsigned long long)hash,(unsigned long long)DAY1_follow_cases[k].hash);
            if(getenv("PE_DAY1_FOLLOW_DUMP")) {
                FILE *out=fopen("local/live/script-follow-native-mismatch.bin","wb");
                if(out){for(unsigned i=0;i<0x200000u;i++)fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT((unsigned)result==DAY1_follow_cases[k].result,"follow return differs from original");
        ASSERT(hash==DAY1_follow_cases[k].hash,"follow memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"follow executes natively");
    }
    PASS();
}
