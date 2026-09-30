#include "retail_pistol_effect_cases.h"

static void test_ATK34_retail_pistol_effect(void)
{
    unsigned k,i,j;
    TEST_RETAIL_DISC1("ATK34_retail_pistol_effect"); TEST_RETAIL_FIXUPS(RETAILFIX_pistol_effect);
    for (k=0;k<sizeof(ATK34_pistol_cases)/sizeof(ATK34_pistol_cases[0]);k++) {
        const uint32_t *a=ATK34_pistol_cases[k].args;
        uint64_t hash=UINT64_C(14695981039346656037);
        ResetTestState();
        for (i=0;i<sizeof(ATK34_pistol_common)/sizeof(ATK34_pistol_common[0]);i++)
            PE_StoreU32(ATK28_mesh_address(ATK34_pistol_common[i][0]),ATK34_pistol_common[i][1]);
        for (i=ATK34_pistol_cases[k].first;i<ATK34_pistol_cases[k].end;i++)
            PE_StoreU32(ATK28_mesh_address(ATK34_pistol_patches[i][0]),ATK34_pistol_patches[i][1]);
        g_pe_gte.ofx=160<<16;g_pe_gte.ofy=112<<16;g_pe_gte.h=256;g_pe_gte.zsf3=0x155;
        func_80071A64(ATK34_pistol_cases[k].seed);
        switch (ATK34_pistol_cases[k].entry) {
        case 0:func_800C42A4(a[0],a[1],a[2]);break;
        case 1:PE_WeaponCallback(0x800C9C20u,a[0],a[1],a[2]);break;
        case 2:PE_WeaponCallback(0x800C9C8Cu,a[0],a[1],a[2]);break;
        case 3:PE_WeaponCallback(0x800C9D9Cu,a[0],a[1],a[2]);break;
        case 4:PE_WeaponCallback(0x800C9EA8u,a[0],a[1],a[2]);break;
        case 5:PE_WeaponCallback(0x800C9FD8u,a[0],a[1],a[2]);break;
        case 6:func_800C2EAC(a[0]);break;
        case 7:func_800C3098((int32_t)a[0]);break;
        case 8:func_800C2FF0(a[0],a[1]);break;
        case 9:func_800C3238(a[0]);break;
        case 10:func_800C608C((int32_t)a[0],a[1],a[2]);break;
        case 11:func_800CEDA8((int32_t)a[0]);break;
        }
        for (i=0;i<sizeof(ATK34_pistol_ranges)/sizeof(ATK34_pistol_ranges[0]);i++)
            for (j=0;j<ATK34_pistol_ranges[i][1];j++)
                hash=(hash^PE_LoadU8(ATK28_mesh_address(ATK34_pistol_ranges[i][0]+j)))*UINT64_C(1099511628211);
        if (hash!=ATK34_pistol_cases[k].hash) {
            fprintf(stderr,"pistol effect %u: hash %016llX/%016llX\n",k,
                (unsigned long long)hash,(unsigned long long)ATK34_pistol_cases[k].hash);
            if (getenv("PE_ATK34_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-atk34-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(ATK28_mesh_address(i)),out);fclose(out);}
            }
        }
        ASSERT(hash==ATK34_pistol_cases[k].hash,"pistol effect memory differs from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"pistol effects execute natively");
    }
    PASS();
}

static void test_ATK34_pistol_texture_upload(void)
{
    unsigned x,y,bank;int calls;
    TEST("ATK34_pistol_texture_upload");
    ResetTestState();B53E_SetupRetailGpu();B52_SeedGpuDispatch();
    PE_StoreU32(0x800B0E18u,0x80170000u);PE_StoreU32(0x800B0E1Cu,0x80178000u);
    for (bank=0;bank<2;bank++) for (y=0;y<256;y++) for (x=0;x<64;x++)
        PE_StoreU16(0x80170000u+bank*32768u+(y*64u+x)*2u,(uint16_t)(bank*12345u+y*64u+x));
    PE_StoreU16(0x800F34E4u,65535u);Stub_ResetOrderLog();
    for (bank=0;bank<2;bank++) {
        func_800CEDA8((int32_t)bank);
        ASSERT(PE_LoadU16(0x800F34E4u)==bank,"texture cache records uploaded bank");
        ASSERT(PE_GPU_DMA2Pending(),"pistol texture upload does not issue DMA");
        ASSERT(PE_GPU_ServiceDMA2Completion(PE_GPU_DMA2EventToken())==1,
            "pistol texture DMA does not complete");
        for (y=0;y<256;y++) for (x=0;x<64;x++) {
            if (B53B_Pixel(896u+x,256u+y)!=(uint16_t)(bank*12345u+y*64u+x))
                fprintf(stderr,"pistol texture bank %u (%u,%u): %04X/%04X DMA pending %d completion %d stop %d\n",
                    bank,x,y,B53B_Pixel(896u+x,256u+y),(uint16_t)(bank*12345u+y*64u+x),
                    PE_GPU_DMA2Pending(),PE_GPU_DMA2CompletionPending(),PE_Port_ShouldStop());
            ASSERT(B53B_Pixel(896u+x,256u+y)==(uint16_t)(bank*12345u+y*64u+x),"pistol texture transfer differs from source");
        }
        calls=g_bootstrap_arg4_call_count;
        PE_StoreU16(0x80170000u+bank*32768u,0xFFFFu);func_800CEDA8((int32_t)bank);
        ASSERT(g_bootstrap_arg4_call_count==calls && B53B_Pixel(896u,256u)==(uint16_t)(bank*12345u),"cached texture uploads again");
    }
    ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"pistol texture transfer reaches native GPU");
    PASS();
}
