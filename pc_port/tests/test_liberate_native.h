/* Real mounted retail bytes + generated registry + actual host callbacks.
 * This covers one init, two update branches and fingerprint rejection. It is not an
 * ability lifecycle, pool, rendering/audio, battle or campaign execution test. */
#ifndef PE_TEST_LIBERATE_NATIVE_H
#define PE_TEST_LIBERATE_NATIVE_H
#include "pe_guestcode.h"
#include "pe_disc_cache.h"
#include "game_port.h"
#include <stdio.h>
#include <string.h>
extern int PE_EffectCallback(pe_addr_t fn,int32_t mode,pe_addr_t data,pe_addr_t extra);
static int PE_TestLiberateDispatch(char *why,size_t why_size)
{
    const pe_addr_t base=0x801ED7F8u,data=0x80140000u;
    const pe_addr_t targets[2]={0x801EDC44u,0x801EEBC8u};
    const uint32_t sizes[2]={0x950u,0x998u},fps[2]={0xC7367DCEu,0x4493F36Bu};
    char sha[41];
#define LIBERATE_REQUIRE(c,m) do { if(!(c)){snprintf(why,why_size,"%s",m);goto fail;} } while(0)
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();
    LIBERATE_REQUIRE(PE_DiscCache_FileRead(PE_DISC_SRC_PEIMG,0x3C9u*2048u,
        PE_Translate(base,0x4800u),0x4800u,why,why_size)==0,"retail ovl03C9 read");
    PE_GuestCode_Sha1Hex(base,0x4800u,sha);
    LIBERATE_REQUIRE(!strcmp(sha,"1d0e972cf9446dc700c82a8597292ced45958ef4"),"overlay identity");
    for(unsigned i=0;i<2;i++) {
        const PeGuestCodeEntry *entry=PE_GuestCode_Resolve(targets[i]);
        LIBERATE_REQUIRE(entry&&entry->thunk&&entry->size==sizes[i]&&entry->fingerprint==fps[i],"native fingerprint/typed thunk");
        uint8_t before=PE_LoadU8(targets[i]+0x20u);
        PE_StoreU8(targets[i]+0x20u,(uint8_t)(before^1u));
        int rejected=PE_GuestCode_Resolve(targets[i])==NULL;
        PE_StoreU8(targets[i]+0x20u,before);
        LIBERATE_REQUIRE(rejected,"corrupted body still resolved");
        LIBERATE_REQUIRE(PE_GuestCode_Resolve(targets[i])!=NULL,"restored body did not resolve");
    }
    /* Actual CE870 position and CE560 allocation; the absent sound package
     * intentionally takes existing 6DED4's resource-free -1 arm, not audio. */
    PE_Fill(data,648u,0u);PE_StoreU32(0x8009D254u,0x80150000u);
    PE_StoreU32(0x80150238u,0x80151000u);
    PE_StoreU32(0x80151014u,30u);PE_StoreU32(0x80151018u,(uint32_t)-40);
    PE_StoreU32(0x8015101Cu,50u);PE_StoreU32(0x800B0E08u,0u);
    PE_StoreU32(0x800F33E0u,0x80152000u);PE_StoreU32(0x80152008u,0x80145000u);
    LIBERATE_REQUIRE(PE_EffectCallback(targets[0],0,data,0u)==1212&&
        PE_LoadU16(data)==30u&&(int16_t)PE_LoadU16(data+2u)==-40&&PE_LoadU16(data+4u)==50u&&
        PE_LoadU32(0x80145000u)==24u&&PE_LoadU32(0x80145004u)==50u&&
        PE_LoadU32(0x80145008u)==0x801ED858u,"EDC44 host init/pool not executed");
    PE_Fill(data,648u,0u);PE_StoreU16(data+0x10u,1u);PE_StoreU16(data+0x12u,47u);
    LIBERATE_REQUIRE(PE_EffectCallback(targets[0],1,data,0u)==1&&PE_LoadU16(data+0x12u)==48u,"EDC44 host update not executed");
    PE_Fill(data,648u,0u);PE_StoreU16(data+6u,7u);PE_StoreU16(0x801F1F38u,1u);
    LIBERATE_REQUIRE(PE_EffectCallback(targets[1],1,data,0u)==2&&PE_LoadU16(data+6u)==8u&&PE_LoadU16(0x801F1F3Au)==1u,"EEBC8 host update not executed");
    LIBERATE_REQUIRE(!PE_Port_ShouldStop(),"callback requested a runtime stop");
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();
#undef LIBERATE_REQUIRE
    return 1;
fail:
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();
    return 0;
}
#ifndef PE_LIBERATE_FOCUSED_ONLY
static void test_LIBERATE_native_dispatch(void)
{
    TEST_RETAIL_DISC1("LIBERATE_native_dispatch");
    char why[256];ResetTestState();
    ASSERT(PE_TestLiberateDispatch(why,sizeof why),why);
    PASS();
}
#endif
#endif
