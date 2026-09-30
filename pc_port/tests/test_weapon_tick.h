#include "retail_weapon_tick_cases.h"

static void test_ATK24_retail_weapon_tick(void)
{
    static const pe_addr_t leaves[]={0x800CA4A8u,0x800CA4B4u,0x800CA540u,0x800CDF40u,0x800CDF4Cu,0x800CDFE0u};
    unsigned k,i;
    TEST("ATK24_retail_weapon_tick");
    for (k=0;k<sizeof(ATK24_weapon_cases)/sizeof(ATK24_weapon_cases[0]);k++) {
        const uint32_t *a=ATK24_weapon_cases[k].args;
        uint32_t result=0;
        uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(ATK24_weapon_common)/sizeof(ATK24_weapon_common[0]);i++)
            PE_StoreU32(0x80000000u+ATK24_weapon_common[i][0],ATK24_weapon_common[i][1]);
        for (i=ATK24_weapon_cases[k].first;i<ATK24_weapon_cases[k].end;i++)
            PE_StoreU32(0x80000000u+ATK24_weapon_patches[i][0],ATK24_weapon_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);
        switch (ATK24_weapon_cases[k].entry) {
        case 0: result=(uint32_t)func_800C2758(a[0],a[1],a[2]);break;
        case 1: result=(uint32_t)func_800C251C(a[0],a[1]);break;
        case 2: result=(uint32_t)func_800C2414(a[0],a[1]);break;
        case 3: result=func_800C2B90(a[0],a[1],a[2],a[3]);break;
        case 4: func_800C2D0C(a[0],a[1],a[2]);break;
        case 5: result=(uint32_t)func_800C2DA0(a[0]);break;
        case 6: result=(uint32_t)func_800C2E08();break;
        case 7: result=(uint32_t)func_800C9B90(a[0]);break;
        case 8: result=(uint32_t)func_800CD8F0(a[0]);break;
        case 9: result=(uint32_t)func_800C9B68(a[0]);break;
        case 10:result=(uint32_t)func_800CD8C8(a[0]);break;
        default:PE_WeaponCallback(leaves[ATK24_weapon_cases[k].entry-11u],a[0],a[1],a[2]);break;
        }
        hash=hit_camera_hash(ATK24_weapon_ranges,sizeof(ATK24_weapon_ranges)/sizeof(ATK24_weapon_ranges[0]));
        if (hash!=ATK24_weapon_cases[k].hash || result!=ATK24_weapon_cases[k].result) {
            fprintf(stderr,"weapon tick %u: result %08X/%08X hash %016llX/%016llX\n",k,
                result,ATK24_weapon_cases[k].result,(unsigned long long)hash,
                (unsigned long long)ATK24_weapon_cases[k].hash);
            if (getenv("PE_ATK24_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk24-native-%u.bin",k);out=fopen(path,"wb");
                if (out) { for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out); }
            }
        }
        ASSERT(result==ATK24_weapon_cases[k].result,"weapon return differs from original");
        ASSERT(hash==ATK24_weapon_cases[k].hash,"weapon memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"weapon call graph executes natively");
    }
    PASS();
}
