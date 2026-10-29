/* Real disc room bytes; fixtures below supply only particle state. */
#ifndef PE_TEST_EFFECT_ROOM_IDENTITY_H
#define PE_TEST_EFFECT_ROOM_IDENTITY_H
#include "pe_guestcode.h"
static int PE_TestLoadEffectRoom(int park,char *why,size_t cap)
{
    const pe_addr_t room=0x8018EFE8u;
    unsigned lba=park?30958u:15016u,n=park?187u:69u,i;
    char sha[41];
    PE_Disc *disc=BTL6_OpenDisc1(why,cap);
    if(!disc)return 0;
    if(PE_GuestImage_LoadExe(disc,why,cap)!=0){PE_Disc_Close(disc);return 0;}
    for(i=0;i<n;i++)if(!PE_Disc_ReadUserSector(disc,lba+i,PE_Translate(room+i*2048u,2048u))){
        PE_Disc_Close(disc);snprintf(why,cap,"effect room sector read");return 0;
    }
    PE_Disc_Close(disc);
    PE_GuestCode_Sha1Hex(room,n*2048u,sha);
    if(strcmp(sha,park?"06bee6ec7422f225a5ea845d3ff33937736253ca":"47a298d84aad055727271a924b9fb846c4dbb3cc")){
        snprintf(why,cap,"effect room identity");return 0;
    }
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();return 1;
}
static unsigned char pe_test_m28_bodies[0xC74u];
static int PE_TestReadM28Bodies(char *why,size_t cap)
{
    if(!PE_TestLoadEffectRoom(0,why,cap))return 0;
    memcpy(pe_test_m28_bodies,PE_Translate(0x801924F8u,sizeof pe_test_m28_bodies),sizeof pe_test_m28_bodies);
    return 1;
}
static void PE_TestRestoreM28Bodies(void)
{
    /* Three contiguous callback bodies, excluding mutable data after 9316C. */
    memcpy(PE_Translate(0x801924F8u,sizeof pe_test_m28_bodies),pe_test_m28_bodies,sizeof pe_test_m28_bodies);
}
static void test_EFFECT_room_identity_m0089(void)
{
    char why[256];const pe_addr_t p=0x80150000u,v=0x80150100u;
    unsigned boundaries;
    TEST_RETAIL_DISC1("EFFECT_room_identity_m0089");ResetTestState();
    ASSERT(PE_TestLoadEffectRoom(1,why,sizeof why),why);
    ASSERT(PE_M28MovementOverlay(),"m0089 shares the m28 movement graph");
    ASSERT(PE_GuestCode_Fnv1a(0x801924F8u,0x538u)==0xC9AC3D84u,"park particle body");
    ASSERT(PE_GuestCode_Resolve(0x801924F8u)!=NULL,"park native registry row");
    PE_Fill(p,64,0);PE_StoreU16(p+48,7);PE_StoreU16(p+50,1);
    PE_StoreU16(v,11);PE_StoreU16(v+2,22);PE_StoreU16(v+4,33);
    PE_StoreU32(0x800E27ECu,0);boundaries=PE_Decomp_BoundaryCount();
    ASSERT(PE_EffectCallback(0x801924F8u,1,p,v)==1,"park particle expires at tick 8");
    ASSERT(PE_LoadU16(p+48)==8 && PE_LoadU16(p)==11 && PE_LoadU16(p+2)==22 && PE_LoadU16(p+4)==33,
        "park particle copies its supplied vector and advances its own timer");
    ASSERT(!PE_Port_ShouldStop() && PE_Decomp_BoundaryCount()==boundaries && !g_stub_order_count,
        "park particle uses compiled native code without boundary");PASS();
}
static void test_EFFECT_room_identity_m0028(void)
{
    char why[256];const pe_addr_t p=0x80150000u;
    unsigned boundaries;
    TEST_RETAIL_DISC1("EFFECT_room_identity_m0028");ResetTestState();
    ASSERT(PE_TestLoadEffectRoom(0,why,sizeof why),why);
    ASSERT(PE_M28MovementOverlay(),"m28 movement identity");
    ASSERT(PE_GuestCode_Fnv1a(0x801924F8u,0x208u)==0xF4F2FCACu &&
        PE_GuestCode_Fnv1a(0x80192700u,0xA48u)==0xF2111279u &&
        PE_GuestCode_Fnv1a(0x80193148u,0x24u)==0x4A3088EBu,"m28 hand callback identities");
    PE_Fill(p,64,0);PE_StoreU16(p+2,100);PE_StoreU32(0x800E27ECu,14);
    boundaries=PE_Decomp_BoundaryCount();
    ASSERT(PE_EffectCallback(0x801924F8u,1,p,0)==1 && PE_LoadU16(p+2)==98,
        "m28 particle retains original timer and gravity");
    PE_EffectCallback_SetExtra1(0x12345678u);
    ASSERT((uint32_t)PE_EffectCallback(0x80193148u,0,p,0x55u)==0x80193288u &&
        PE_LoadU32(0x80193288u)==p && PE_LoadU32(0x8019328Cu)==0x55u &&
        PE_LoadU32(0x80193290u)==0x12345678u,"m28 command retains argument stores");
    ASSERT(!PE_Port_ShouldStop() && PE_Decomp_BoundaryCount()==boundaries && !g_stub_order_count,
        "m28 callbacks remain compiled native");PASS();
}
#endif
