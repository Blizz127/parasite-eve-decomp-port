#include "retail_mirror_effect_cases.h"
static void test_SEW15_mirror_effect(void)
{
    TEST_RETAIL_DISC1("SEW15_mirror_effect"); TEST_RETAIL_FIXUPS(RETAILFIX_mirror_effect);
    for(unsigned k=0;k<sizeof(SEW15_cases)/sizeof(SEW15_cases[0]);k++) {
        const uint32_t *a=SEW15_cases[k].args;uint32_t result=0;uint64_t hash=UINT64_C(1469598103934665603);
        ResetTestState();
        for(unsigned i=0;i<sizeof(SEW15_common)/sizeof(SEW15_common[0]);i++)
            PE_StoreU32(0x80000000u+SEW15_common[i][0],SEW15_common[i][1]);
        for(unsigned i=SEW15_cases[k].first;i<SEW15_cases[k].end;i++)
            PE_StoreU32(0x80000000u+SEW15_patches[i][0],SEW15_patches[i][1]);
        D_8009D1A0=0x80u;
        g_pe_gte.ofx=160*65536;g_pe_gte.ofy=112*65536;g_pe_gte.h=256;
        switch(SEW15_cases[k].entry) {
        case 0:result=(uint32_t)PE_MirrorInit(a[0]);break;
        case 1:result=(uint32_t)PE_MirrorCommand(a[0],a[1],a[2],a[3],a[4]);break;
        case 2:PE_MirrorBind(a[0],a[1],(int32_t)a[2],(int32_t)a[3],(int32_t)a[4],(int32_t)a[5]);break;
        case 3:PE_MirrorPose(a[0]);break;
        case 4:PE_MirrorPackets(a[0]);break;
        case 5:result=(uint32_t)PE_EffectCallback(0x8018F0F0u,2,a[0],0);break;
        case 6:result=(uint32_t)func_8006F39C(a[0],a[1]);break;
        case 7:result=(uint32_t)func_8006F6D4(a[0],a[1],a[2],a[3],a[4],a[5]);break;
        case 8:result=(uint32_t)func_8006F8EC(a[0]);break;
        case 10:result=(uint32_t)func_800187C0(a[0]);break;
        case 9:result=(uint32_t)func_8006F9F0(a[0]);break;
        }
        for(unsigned i=0;i<sizeof(SEW15_ranges)/sizeof(SEW15_ranges[0]);i++)
            for(unsigned j=0;j<SEW15_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(0x80000000u+SEW15_ranges[i][0]+j))*UINT64_C(1099511628211);
        if(hash!=SEW15_cases[k].hash || result!=SEW15_cases[k].result) {
            fprintf(stderr,"Mirror %u result %08X/%08X hash %016llX/%016llX\n",k,result,SEW15_cases[k].result,
                (unsigned long long)hash,(unsigned long long)SEW15_cases[k].hash);
            if(getenv("PE_SEW15_DUMP")) {
                char path[128];snprintf(path,sizeof(path),"local/live/mirror-native-%u.bin",k);FILE*f=fopen(path,"wb");
                if(f){for(unsigned i=0;i<0x200000;i++)fputc(PE_LoadU8(0x80000000u+i),f);fclose(f);}
            }
        }
        ASSERT(hash==SEW15_cases[k].hash && result==SEW15_cases[k].result,"mirror differs from original instructions");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"mirror callback executes natively");
    }
    PASS();
}
