#include "retail_inventory_commit_cases.h"

static void test_INV4_retail_inventory_commit(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV4_retail_inventory_commit"); TEST_RETAIL_FIXUPS(RETAILFIX_inventory_commit);
    for (k=0;k<sizeof(INV4_inventory_commit_cases)/sizeof(INV4_inventory_commit_cases[0]);k++) {
        const uint32_t *a=INV4_inventory_commit_cases[k].args;
        uint64_t hash;
        ResetTestState();
        /* func_800209F0 reads its weapon tables from the disc-loaded EXE. */
        ASSERT(PE_TestRetail_LoadExeRange(0x800106A4u, 0x39u) == 0, "EXE 0x800106A4 table");
        for (i=0;i<sizeof(INV4_inventory_commit_common)/sizeof(INV4_inventory_commit_common[0]);i++)
            PE_StoreU32(0x80000000u+INV4_inventory_commit_common[i][0],INV4_inventory_commit_common[i][1]);
        for (i=INV4_inventory_commit_cases[k].first;i<INV4_inventory_commit_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV4_inventory_commit_patches[i][0],INV4_inventory_commit_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        switch (INV4_inventory_commit_cases[k].entry) {
        case 0:func_800512AC((int32_t)a[0],a[1]);break;
        case 1:func_800218D8();break;
        case 2:func_80021AF8();break;
        case 3:func_800209F0();break;
        case 4:func_800254BC((int32_t)a[0]);break;
        case 5:func_80051244();break;
        case 6:func_8005D970();break;
        }
        /* Export the established native-owned inventory scalars for comparison. */
        PE_StoreU32(0x8009D018u,D_8009D018);PE_StoreU32(0x8009D048u,D_8009D048);
        PE_StoreU32(0x8009D04Cu,D_8009D04C);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D054u,D_8009D054);PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV4_inventory_commit_ranges,sizeof(INV4_inventory_commit_ranges)/sizeof(INV4_inventory_commit_ranges[0]));
        if (hash!=INV4_inventory_commit_cases[k].hash) {
            fprintf(stderr,"inventory command %u hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)INV4_inventory_commit_cases[k].hash);
            if (getenv("PE_INV4_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv4-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV4_inventory_commit_cases[k].hash,"inventory command memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"inventory command call graph executes natively");
    }
    PASS();
}
