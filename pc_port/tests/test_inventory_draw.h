#include "retail_inventory_draw_cases.h"

static void test_INV3_retail_inventory_draw(void)
{
    unsigned k,i;
    TEST_RETAIL_DISC1("INV3_retail_inventory_draw"); TEST_RETAIL_FIXUPS(RETAILFIX_inventory_draw);
    for (k=0;k<sizeof(INV3_inventory_draw_cases)/sizeof(INV3_inventory_draw_cases[0]);k++) {
        const uint32_t *a=INV3_inventory_draw_cases[k].args;
        uint64_t hash;uint32_t result=0u;
        ResetTestState();
        for (i=0;i<sizeof(INV3_inventory_draw_common)/sizeof(INV3_inventory_draw_common[0]);i++)
            PE_StoreU32(0x80000000u+INV3_inventory_draw_common[i][0],INV3_inventory_draw_common[i][1]);
        for (i=INV3_inventory_draw_cases[k].first;i<INV3_inventory_draw_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV3_inventory_draw_patches[i][0],INV3_inventory_draw_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D050=PE_LoadU32(0x8009D050u);
        for (i=0;i<INV3_inventory_draw_cases[k].frame;i++) PE_GPU_VBlankStep();
        switch (INV3_inventory_draw_cases[k].entry) {
        case 0:func_800602D0((int32_t)a[0],(int32_t)a[1]);break;
        case 1:func_80060528((int32_t)a[0]);break;
        case 2:func_8006055C((int32_t)a[0]);break;
        case 3:func_80060590((int32_t)a[0]);break;
        case 4:func_800605C4((int32_t)a[0]);break;
        case 5:func_800605F8((int32_t)a[0]);break;
        case 6:func_800536B8((int32_t)a[0]);break;
        case 7:func_80043B0C(a[0]);break;
        case 8:func_80043C64(a[0]);break;
        case 9:func_8004905C(); /* retail leaf ignores a0 */break;
        case 10:func_80050748(a[0]);break;
        case 11:func_8004F838(a[0]);break;
        case 12:result=func_80051E48();break;
        case 13:result=func_8005DBF8();break;
        case 14:result=(uint32_t)func_80021080();break;
        case 15:result=(uint32_t)func_80052534();break;
        }
        hash=hit_camera_hash(INV3_inventory_draw_ranges,sizeof(INV3_inventory_draw_ranges)/sizeof(INV3_inventory_draw_ranges[0]));
        if (hash!=INV3_inventory_draw_cases[k].hash || result!=INV3_inventory_draw_cases[k].result) {
            fprintf(stderr,"inventory drawing %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV3_inventory_draw_cases[k].hash,
                result,INV3_inventory_draw_cases[k].result);
            if (getenv("PE_INV3_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv3-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        ASSERT(hash==INV3_inventory_draw_cases[k].hash,"inventory drawing memory differs from original");
        ASSERT(result==INV3_inventory_draw_cases[k].result,"inventory drawing result differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"inventory drawing call graph executes natively");
        ASSERT(PE_GPU_VSyncQuery()==INV3_inventory_draw_cases[k].frame,"drawing must not advance VBlank time");
    }
    PASS();
}
