#include "retail_actor_contact_cases.h"

static void test_SEW18_actor_contact(void)
{
    TEST("SEW18_actor_contact");
    for (unsigned k=0;k<sizeof(SEW18_cases)/sizeof(SEW18_cases[0]);k++) {
        const uint32_t *args=SEW18_cases[k].args;
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(SEW18_common)/sizeof(SEW18_common[0]);i++)
            PE_StoreU32(0x80000000u+SEW18_common[i][0],SEW18_common[i][1]);
        for (unsigned i=SEW18_cases[k].first;i<SEW18_cases[k].end;i++)
            PE_StoreU32(0x80000000u+SEW18_patches[i][0],SEW18_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        switch(SEW18_cases[k].entry) {
        case 0:func_80036448();break;
        case 1:func_80035F54(args[0]);break;
        case 2:func_8001D268(args[0],(int32_t)args[1],args[2],(int32_t)args[3]);break;
        case 3:func_80012774();break;
        }
        hash=hit_camera_hash(SEW18_ranges,sizeof(SEW18_ranges)/sizeof(SEW18_ranges[0]));
        if (hash!=SEW18_cases[k].hash) {
            fprintf(stderr,"actor contact case %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)SEW18_cases[k].hash);
            if (getenv("PE_SEW18_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"local/live/contact-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (unsigned i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==SEW18_cases[k].hash,"actor contact memory differs from original instructions");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"actor contact call graph executes natively");
    }
    PASS();
}
