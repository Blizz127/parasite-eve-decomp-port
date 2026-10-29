/* Room module descriptors whose slot 0 is a bare `return 0`.
 *
 * The room callback census (tools/analysis/room_callback_census.py; evidence
 * in docs/evidence/room-callback-census-20261025/) finds 16 room module
 * descriptors whose slot 0 holds a bare `return 0` (`jr ra; move v0,zero`).
 * Each stub sits in the header span ahead of the room's first framed
 * function, and no registry row covers it.
 *
 * Only func_8006914C's module init reads slot 0.  The port treats such a stub
 * as a complete `return 0` (func_8006914C_module_init in
 * pc_port/game/boot/func_80029810_port.c).
 *
 * This regression:
 *   - mounts m0049i's chunk 2 from the disc.  Chunk 2 is the room package:
 *     func_8006A8D4 sets D_800B0E64 to D_80011614 - 8, which is 0x8018EFE8;
 *   - publishes the package's own module export (code 0x0E, its descriptor
 *     at 80192BCC) through the room loader's func_8006B4F8_bind_effects;
 *   - runs func_8006914C state 0 under the port binary's stop policy.
 * Only the empty module table, with no standard modules 0-7 and 0x55, is a
 * fixture.  The package, the descriptor and the stub are the room's own
 * bytes. */
#ifndef PE_TEST_ROOM_SLOT0_STUB_H
#define PE_TEST_ROOM_SLOT0_STUB_H
#include "pe_guestcode.h"
#include "pe_guest_decomp.h"
#include "stub_registry.h"
#include <stdio.h>
#include <string.h>

/* Every BOOTSTRAP_RET call, repeats included. */
static unsigned PE_TestRoomSlot0BootstrapCalls(void)
{
    unsigned n=0;
    int i;
    for(i=0;i<g_stub_count;i++)
        if(!strcmp(g_stub_registry[i].classification,"BOOTSTRAP_RET")) n+=(unsigned)g_stub_registry[i].invoked;
    return n;
}

static int PE_TestRoomSlot0Stub(char *why,size_t why_size)
{
    const uint32_t chunk_lba=21909u,chunk_sectors=181u;   /* m0049i chunk 2 */
    const pe_addr_t room=0x8018EFE8u,descriptor=0x80192BCCu,stub=0x8018F034u;
    const pe_addr_t table=0x80151000u;
    const unsigned code=0x0Eu;                            /* m0049i's module export */
    unsigned i,boundaries0,bootstrap0;
    int saved_stop=PE_Decomp_BoundaryStopEnabled(),r;
    char sha[41];
    PE_Disc *disc;
#define SLOT0_REQUIRE(c,m) do { if(!(c)){snprintf(why,why_size,"%s",m);goto fail;} } while(0)
    disc=BTL6_OpenDisc1(why,why_size);
    SLOT0_REQUIRE(disc!=NULL,"disc 1 open");
    if(PE_GuestImage_LoadExe(disc,why,why_size)!=0){PE_Disc_Close(disc);goto fail;}
    for(i=0;i<chunk_sectors;i++)
        if(!PE_Disc_ReadUserSector(disc,chunk_lba+i,PE_Translate(room+i*2048u,2048u))){
            PE_Disc_Close(disc);SLOT0_REQUIRE(0,"m0049i chunk 2 sector read");
        }
    PE_Disc_Close(disc);
    PE_GuestCode_Sha1Hex(room,chunk_sectors*2048u,sha);
    SLOT0_REQUIRE(!strcmp(sha,"320861e23df3605a60e159b7db5fc081aa421607"),"m0049i chunk 2 identity");
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();
    SLOT0_REQUIRE(PE_LoadU32(descriptor)==stub,"m0049i descriptor slot 0 names 8018F034");
    SLOT0_REQUIRE(PE_LoadU32(stub)==0x03E00008u&&PE_LoadU32(stub+4u)==0x00001021u,
        "8018F034 is the bare return-0 stub");
    SLOT0_REQUIRE(PE_GuestCode_Resolve(stub)==NULL,
        "the registry now binds 8018F034: point this test at another slot-0 stub");
    /* The room loader publishes the package's module exports into an empty table. */
    PE_Fill(table,0x56u*4u,0u);
    PE_StoreU32(0x800942E0u,table);
    PE_StoreU32(0x800B0E64u,room);      /* D_800B0CD8+0x18C: the loaded room package */
    func_8006B4F8_bind_effects(room);
    SLOT0_REQUIRE(PE_LoadU32(table+code*4u)==descriptor,"room loader did not publish the m0049i descriptor as module 0x0E");
    PE_StoreU32(0x800B0E60u,0u);        /* +0x188: no command-record arena */
    PE_StoreU8(0x800B0DC7u,0u);         /* +0xEF: state 0 */
    D_8009D1A0=0u;
    PE_Decomp_SetBoundaryStop(1);
    boundaries0=PE_Decomp_BoundaryCount();bootstrap0=PE_TestRoomSlot0BootstrapCalls();
    r=func_8006914C(0);
    SLOT0_REQUIRE(!PE_Port_ShouldStop(),"module init stopped on the slot-0 return-0 stub");
    SLOT0_REQUIRE(PE_Decomp_BoundaryCount()==boundaries0,"module init recorded a boundary");
    SLOT0_REQUIRE(PE_TestRoomSlot0BootstrapCalls()==bootstrap0,"module init took a bootstrap return");
    SLOT0_REQUIRE(r==0&&(PE_LoadU32(0x800B0CD8u)&8u)&&(D_8009D1A0&0x80u)&&PE_LoadU8(0x800B0DC7u)==0u,
        "func_8006914C state 0 did not complete");
    PE_Decomp_SetBoundaryStop(saved_stop);
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();
#undef SLOT0_REQUIRE
    return 1;
fail:
    PE_Decomp_SetBoundaryStop(saved_stop);
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();
    return 0;
}

static void test_ROOM_slot0_return0_stub_module_init(void)
{
    char why[256];
    TEST_RETAIL_DISC1("ROOM_slot0_return0_stub_module_init");
    ResetTestState();
    ASSERT(PE_TestRoomSlot0Stub(why,sizeof why),why);
    PASS();
}
#endif
