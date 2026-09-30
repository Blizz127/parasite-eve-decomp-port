#include "retail_scripted_exit_cases.h"

static void test_ATK20_retail_scripted_exit(void)
{
    unsigned k,i;
    TEST("ATK20_retail_scripted_exit");
    for (k=0;k<sizeof(ATK20_exit_cases)/sizeof(ATK20_exit_cases[0]);k++) {
        uint32_t result=0; uint64_t hash;
        ResetTestState();
        for (i=0;i<sizeof(ATK20_exit_common)/sizeof(ATK20_exit_common[0]);i++)
            PE_StoreU32(0x80000000u+ATK20_exit_common[i][0],ATK20_exit_common[i][1]);
        for (i=ATK20_exit_cases[k].first;i<ATK20_exit_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ATK20_exit_patches[i][0],ATK20_exit_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D250=PE_LoadU32(0x8009D250u);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        switch (ATK20_exit_cases[k].entry) {
        case 0:func_8002DC58();break;
        case 1:result=(uint32_t)func_80027A08(0x80141000u);break;
        case 2:func_800295E4();break;
        case 3:func_80051510();break;
        case 4:func_800703F4();break;
        case 5:result=(uint32_t)func_800702DC();break;
        case 6:result=(uint32_t)func_800701B4();break;
        case 7:result=(uint32_t)func_800192C8();break;
        }
        /* Normalize the established native-owned inventory globals. */
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(ATK20_exit_ranges,sizeof(ATK20_exit_ranges)/sizeof(ATK20_exit_ranges[0]));
        if (hash!=ATK20_exit_cases[k].hash) {
            fprintf(stderr,"scripted exit %u: %016llX expected %016llX\n",k,
                (unsigned long long)hash,(unsigned long long)ATK20_exit_cases[k].hash);
            if (getenv("PE_ATK20_DUMP")) {
                char path[100]; FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk20-native-%u.bin",k);
                out=fopen(path,"wb");
                if (out) { for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out); fclose(out); }
            }
        }
        ASSERT(hash==ATK20_exit_cases[k].hash,"scripted exit differs from original call graph");
        if (ATK20_exit_cases[k].entry==1 || ATK20_exit_cases[k].entry>=5)
            ASSERT(result==ATK20_exit_cases[k].result,"scripted exit return differs from original");
        ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,"scripted exit executes natively");
    }
    PASS();
}
