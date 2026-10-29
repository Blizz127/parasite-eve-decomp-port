/* ovl_03C9 effect descriptors 0x6E-0x71: func_801F0690's pool child 0x801F0068.
 * The chain runs through the real mounted retail bytes, the generated registry
 * and actual host callbacks:
 *   - 801F0690 mode 0 builds the record pool with child callback 801F0068
 *     (func_800CE560);
 *   - mode 1 at tick 2 claims eight children;
 *   - the effect manager's update tick (func_800CE688 -> PE_EffectCallback) runs
 *     every child once.
 * Also covers fingerprint rejection.  It does not cover the draw pass, scene,
 * audio, battle or ability lifecycle. */
#ifndef PE_TEST_OVL03C9_EFFECT_NATIVE_H
#define PE_TEST_OVL03C9_EFFECT_NATIVE_H
#include "pe_guestcode.h"
#include "pe_disc_cache.h"
#include "pe_gpu.h"
#include "pe_sdk.h"
#include "game_port.h"
#include "test_retail_fixups.h"
#include "pe_guest_decomp.h"
#include "stub_registry.h"
#include <stdio.h>
#include <string.h>
extern int PE_EffectCallback(pe_addr_t fn,int32_t mode,pe_addr_t data,pe_addr_t extra);
extern int func_800CE688(pe_addr_t pool);
extern int func_800CE78C(pe_addr_t pool);
extern int func_800753B4(pe_addr_t ot);
extern int func_80074DC0(int mode);
extern void func_80077F7C(void);
static int PE_TestEffect801F0068Dispatch(char *why,size_t why_size)
{
    const pe_addr_t base=0x801ED7F8u,parent=0x801F0690u,child=0x801F0068u;
    const pe_addr_t obj=0x80140000u,pool=0x80145000u;
    char sha[41];
    unsigned i,claimed=0,ticked=0;
#define EFFECT_REQUIRE(c,m) do { if(!(c)){snprintf(why,why_size,"%s",m);goto fail;} } while(0)
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();
    EFFECT_REQUIRE(PE_DiscCache_FileRead(PE_DISC_SRC_PEIMG,0x3C9u*2048u,
        PE_Translate(base,0x4800u),0x4800u,why,why_size)==0,"retail ovl03C9 read");
    PE_GuestCode_Sha1Hex(base,0x4800u,sha);
    EFFECT_REQUIRE(!strcmp(sha,"1d0e972cf9446dc700c82a8597292ced45958ef4"),"overlay identity");
    /* Mode 0 does four things:
     *   - copies the anchor position from D_800F32D0->f8 (+0x268);
     *   - sets the rotation with CE9D4;
     *   - runs C6D5C on a zeroed mesh;
     *   - builds 40 entries of 20 bytes with CE560 and the 801F0068 callback. */
    PE_Fill(obj,0x20u,0u);PE_Fill(pool,12u+40u*24u,0u);
    PE_StoreU32(0x800F32D0u,0x80152100u);PE_StoreU32(0x80152108u,0x80153000u);
    PE_StoreU16(0x80153268u,100u);PE_StoreU16(0x8015326Au,(uint16_t)-20);PE_StoreU16(0x8015326Cu,300u);
    PE_StoreU32(0x8009D254u,0x80150000u);PE_StoreU32(0x80150238u,0x80151000u);
    PE_Fill(0x80151000u,0x40u,0u);
    PE_StoreU32(0x80151014u,30u);PE_StoreU32(0x80151018u,(uint32_t)-40);PE_StoreU32(0x8015101Cu,50u);
    PE_StoreU32(0x800E22D4u,0x80154000u);PE_Fill(0x80154000u,0x40u,0u);
    PE_StoreU32(0x800F33E0u,0x80152000u);PE_StoreU32(0x80152008u,pool);
    PE_StoreU32(0x800E2368u,0x80152200u);PE_StoreU32(0x80152208u,0u);
    EFFECT_REQUIRE(PE_EffectCallback(parent,0,obj,0u)==(int)(12u+40u*24u)&&
        PE_LoadU32(pool)==24u&&PE_LoadU32(pool+4u)==40u&&PE_LoadU32(pool+8u)==child&&
        PE_LoadU16(obj)==100u&&(int16_t)PE_LoadU16(obj+2u)==-20&&PE_LoadU16(obj+4u)==300u,
        "801F0690 mode 0 did not build the 801F0068 pool");
    /* Mode 1 at tick 2 claims eight children:
     *   - the centre child (wait 0);
     *   - six random children (wait 1);
     *   - the +0x800 child (wait 2). */
    PE_StoreU32(0x800E27ECu,2u);
    EFFECT_REQUIRE(PE_EffectCallback(parent,1,obj,0u)==0,"801F0690 mode 1 return");
    for(i=0;i<40u;i++) if(PE_LoadU16(pool+12u+i*24u)) claimed++;
    EFFECT_REQUIRE(claimed==8u,"801F0690 mode 1 did not claim eight children");
    /* The effect manager's update tick runs each claimed child once.  Below
     * their lifetimes every child returns 0, so each entry's tick and the
     * child's own step are both 1 afterwards. */
    EFFECT_REQUIRE(func_800CE688(pool)==8&&!PE_Port_ShouldStop(),"801F0068 child update stopped or skipped");
    for(i=0;i<40u;i++) {
        pe_addr_t entry=pool+12u+i*24u;
        if(!PE_LoadU16(entry)) continue;
        EFFECT_REQUIRE(PE_LoadU16(entry+2u)==1u&&PE_LoadU16(entry+4u+0x12u)==1u,"801F0068 child update not executed");
        ticked++;
    }
    EFFECT_REQUIRE(ticked==8u,"801F0068 child count changed");
    {
        const PeGuestCodeEntry *entry=PE_GuestCode_Resolve(child);
        uint8_t before;
        int rejected;
        EFFECT_REQUIRE(entry&&entry->thunk&&entry->size==0x628u&&entry->fingerprint==0x4DD4C67Au,"native fingerprint/typed thunk");
        before=PE_LoadU8(child+0x20u);
        PE_StoreU8(child+0x20u,(uint8_t)(before^1u));
        rejected=PE_GuestCode_Resolve(child)==NULL;
        PE_StoreU8(child+0x20u,before);
        EFFECT_REQUIRE(rejected,"corrupted body still resolved");
        EFFECT_REQUIRE(PE_GuestCode_Resolve(child)!=NULL,"restored body did not resolve");
    }
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();
#undef EFFECT_REQUIRE
    return 1;
fail:
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();
    return 0;
}
static void test_OVL03C9_effect_801F0068_native_dispatch(void)
{
    TEST_RETAIL_DISC1("OVL03C9_effect_801F0068_native_dispatch");
    char why[256];ResetTestState();
    ASSERT(PE_TestEffect801F0068Dispatch(why,sizeof why),why);
    PASS();
}

/* Draw pass with real callees over the children's whole lifetime.
 *
 * Fixtures:
 *   - the retail EXE image is loaded the way the boot loader lays it out
 *     (text+data from the PS-X header), then ovl_03C9 is mounted over it;
 *   - libgpu dispatch and DMA2 are seeded as the GPU tests seed them;
 *   - InitGeom (func_80077F7C) then SetGeomOffset(160, 112), with H also
 *     published through *D_800BCFA8;
 *   - an identity camera translated 1000 forward;
 *   - each frame, a 0x1000-entry reverse OT (ClearOTagR layout) and a fresh
 *     packet arena.
 *
 * Synthetic: the runtime-loaded effect mesh (D_800E22D4, zero in the EXE
 * image), a two-triangle flat textured mesh with both windings so one
 * faces the camera; and the texture pointers D_800B0E18/1C, aimed at a
 * nonzero pattern.
 *
 * Each frame runs one update tick (func_800CE688) and one draw tick
 * (func_800CE78C), then submits the OT through DrawOTag (func_800753B4,
 * libgpu jtb queue -> worker 80076B98, which validates and executes every
 * node) and drains it with DrawSync(0) (func_80074DC0).  This shows the
 * child's draw calls run, emit OT-linked packets the native GPU validator
 * accepts and executes, and upload the texture page.  It is not a visual,
 * scene, audio or battle check: the geometry, texture and camera are
 * fixtures. */
typedef struct {
    unsigned frames,draw_ticks,child_draws;
    uint32_t packet_bytes,ot_links;
    uint64_t polygons,lines,rectangles;
    int texture_uploaded;
    unsigned boundaries,bootstrap_calls;   /* deltas under the production stop policy */
} PeEffectDrawCoverage;

static uint32_t PE_TestEffectLe32(const uint8_t *p)
{
    return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}

/* Every BOOTSTRAP_RET call, repeats included (Bootstrap_InvocationCount
 * counts unique symbols only). */
static unsigned PE_TestEffectBootstrapCalls(void)
{
    unsigned n=0;
    int i;
    for(i=0;i<g_stub_count;i++)
        if(!strcmp(g_stub_registry[i].classification,"BOOTSTRAP_RET")) n+=(unsigned)g_stub_registry[i].invoked;
    return n;
}

static int PE_TestEffect801F0068Draw(char *why,size_t why_size,PeEffectDrawCoverage *cov)
{
    const pe_addr_t base=0x801ED7F8u,parent=0x801F0690u,child=0x801F0068u;
    const pe_addr_t obj=0x80140000u,pool=0x80145000u,mesh=0x80154000u;
    const pe_addr_t camera=0x80156000u,hword=0x80156100u,texture=0x80158000u;
    const pe_addr_t ot=0x80160000u,arena=0x80170000u;
    const unsigned ot_entries=0x1000u;
    uint8_t hdr[0x800];
    char sha[41];
    unsigned i,frame,boundaries0,bootstrap0;
    const int saved_stop=PE_Decomp_BoundaryStopEnabled();
    PeGpuState before,after;
#define DRAW_REQUIRE(c,m) do { if(!(c)){snprintf(why,why_size,"%s",m);goto fail;} } while(0)
    memset(cov,0,sizeof *cov);
    /* 1. Retail EXE image first (it spans most of RAM), then the overlay. */
    DRAW_REQUIRE(PE_DiscCache_FileRead(PE_DISC_SRC_EXE,0u,hdr,sizeof hdr,why,why_size)==0,"retail EXE header read");
    DRAW_REQUIRE(PE_TestRetail_LoadExeRange(PE_TestEffectLe32(hdr+0x18),PE_TestEffectLe32(hdr+0x1C))==0,"retail EXE image load");
    B54KR_SeedGpuStatic();
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();
    DRAW_REQUIRE(PE_DiscCache_FileRead(PE_DISC_SRC_PEIMG,0x3C9u*2048u,
        PE_Translate(base,0x4800u),0x4800u,why,why_size)==0,"retail ovl03C9 read");
    PE_GuestCode_Sha1Hex(base,0x4800u,sha);
    DRAW_REQUIRE(!strcmp(sha,"1d0e972cf9446dc700c82a8597292ced45958ef4"),"overlay identity");
    /* 2. Synthetic runtime-loaded inputs: effect mesh and texture buffer. */
    PE_Fill(mesh,0x80u,0u);
    PE_StoreU16(mesh+0u,2u);                       /* group 0: two flat textured triangles */
    PE_StoreU16(mesh+8u,0x40u);PE_StoreU16(mesh+10u,1u);   /* one colour word at +0x40 */
    {
        static const uint16_t faces[12]={0x50u,0x58u,0x60u,0x0000u,0x001Fu,0x1F00u,
                                         0x50u,0x60u,0x58u,0x0000u,0x1F00u,0x001Fu};
        static const int16_t points[3][3]={{0,0,0},{200,0,0},{0,200,0}};
        for(i=0;i<12u;i++) PE_StoreU16(mesh+16u+i*2u,faces[i]);
        PE_StoreU32(mesh+0x40u,0x00808080u);
        for(i=0;i<3u;i++) {
            PE_StoreU16(mesh+0x50u+i*8u,(uint16_t)points[i][0]);PE_StoreU16(mesh+0x52u+i*8u,(uint16_t)points[i][1]);
            PE_StoreU16(mesh+0x54u+i*8u,(uint16_t)points[i][2]);PE_StoreU16(mesh+0x56u+i*8u,0x40u);
        }
    }
    PE_StoreU32(0x800E22D4u,mesh);
    PE_Fill(texture,0x8000u,0x7Fu);
    PE_StoreU32(0x800B0E18u,texture);PE_StoreU32(0x800B0E1Cu,texture);
    /* 3. Spawner state, as in the update test. */
    PE_Fill(obj,0x20u,0u);PE_Fill(pool,12u+40u*24u,0u);
    PE_StoreU32(0x800F32D0u,0x80152100u);PE_StoreU32(0x80152108u,0x80153000u);
    PE_StoreU16(0x80153268u,100u);PE_StoreU16(0x8015326Au,(uint16_t)-20);PE_StoreU16(0x8015326Cu,300u);
    PE_StoreU32(0x8009D254u,0x80150000u);PE_StoreU32(0x80150238u,0x80151000u);
    PE_Fill(0x80151000u,0x40u,0u);
    PE_StoreU32(0x80151014u,30u);PE_StoreU32(0x80151018u,(uint32_t)-40);PE_StoreU32(0x8015101Cu,50u);
    PE_StoreU32(0x800F33E0u,0x80152000u);PE_StoreU32(0x80152008u,pool);
    PE_StoreU32(0x800E2368u,0x80152200u);PE_StoreU32(0x80152208u,0u);
    DRAW_REQUIRE(PE_EffectCallback(parent,0,obj,0u)==(int)(12u+40u*24u),"801F0690 mode 0");
    PE_StoreU32(0x800E27ECu,2u);
    DRAW_REQUIRE(PE_EffectCallback(parent,1,obj,0u)==0,"801F0690 mode 1");
    /* 4. Camera and projection. */
    PE_Fill(camera,32u,0u);
    PE_StoreU16(camera+0u,0x1000u);PE_StoreU16(camera+8u,0x1000u);PE_StoreU16(camera+16u,0x1000u);
    PE_StoreU32(camera+28u,1000u);
    PE_StoreU32(0x800BCFA4u,camera);PE_StoreU32(hword,0x3E8u);PE_StoreU32(0x800BCFA8u,hword);
    func_80077F7C();
    PE_GTE_WriteCtrl(24u,160u<<16);PE_GTE_WriteCtrl(25u,112u<<16);
    PE_StoreU32(0x8009CDDCu,0u);PE_StoreU32(0x800B0E58u,arena);PE_StoreU32(0x800B0E38u,ot);
    PE_GPU_GetState(&before);
    /* The test binary defaults to the record policy, where an unported
     * callback is logged and skipped.  Run the frames under the port
     * binary's default stop policy (port_main.c: PE_Decomp_SetBoundaryStop(1)):
     * every boundary then requests a stop, which the loop checks after each
     * step.  Boundary symbols and bootstrap-return calls are also counted,
     * so a skipped call cannot pass as a draw. */
    PE_Decomp_SetBoundaryStop(1);
    boundaries0=PE_Decomp_BoundaryCount();bootstrap0=PE_TestEffectBootstrapCalls();
    /* 5. Frames until every child has expired (lifetimes are 32/40 steps). */
    for(frame=0;frame<48u;frame++) {
        unsigned active=0,links=0;
        for(i=0;i<40u;i++) if(PE_LoadU16(pool+12u+i*24u)) active++;
        if(!active) break;
        PE_StoreU32(ot,0x00FFFFFFu);
        for(i=1;i<ot_entries;i++) PE_StoreU32(ot+i*4u,(ot+(i-1u)*4u)&0x00FFFFFFu);
        PE_StoreU32(0x8009CDD8u,0u);
        (void)func_800CE688(pool);
        DRAW_REQUIRE(!PE_Port_ShouldStop(),"801F0068 update tick stopped");
        cov->child_draws+=(unsigned)func_800CE78C(pool);
        DRAW_REQUIRE(!PE_Port_ShouldStop(),"801F0068 draw tick stopped");
        cov->draw_ticks++;
        cov->packet_bytes+=PE_LoadU32(0x8009CDD8u);
        for(i=1;i<ot_entries;i++) if(PE_LoadU32(ot+i*4u)!=((ot+(i-1u)*4u)&0x00FFFFFFu)) links++;
        if(PE_LoadU32(ot)!=0x00FFFFFFu) links++;
        cov->ot_links+=links;
        (void)func_800753B4(ot+(ot_entries-1u)*4u);
        DRAW_REQUIRE(!PE_Port_ShouldStop(),"DrawOTag of the 801F0068 frame stopped");
        (void)func_80074DC0(0);
        DRAW_REQUIRE(!PE_Port_ShouldStop(),"DrawSync of the 801F0068 frame stopped");
        cov->frames++;
    }
    cov->boundaries=PE_Decomp_BoundaryCount()-boundaries0;
    cov->bootstrap_calls=PE_TestEffectBootstrapCalls()-bootstrap0;
    PE_Decomp_SetBoundaryStop(saved_stop);
    PE_GPU_GetState(&after);
    cov->polygons=after.polygon_count-before.polygon_count;
    cov->lines=after.line_count-before.line_count;
    cov->rectangles=after.rectangle_count-before.rectangle_count;
    cov->texture_uploaded=PE_LoadU16(0x800F34E4u)==1u;
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();
#undef DRAW_REQUIRE
    return 1;
fail:
    PE_Decomp_SetBoundaryStop(saved_stop);
    PE_Overlay_ResetResidency();PE_GuestCode_InstallGenerated();
    return 0;
}

static void test_OVL03C9_effect_801F0068_native_draw(void)
{
    PeEffectDrawCoverage cov;
    TEST_RETAIL_DISC1("OVL03C9_effect_801F0068_native_draw");
    char why[256];ResetTestState();func_80071A64(1u);   /* host-static BIOS rand seed: order-independent counts */
    ASSERT(PE_TestEffect801F0068Draw(why,sizeof why,&cov),why);
    printf("[801F0068 draw] frames %u, draw ticks %u, child draws %u, packet bytes %u, OT links %u, "
           "polygons %llu, lines %llu, rectangles %llu, texture page uploaded %d, boundaries %u, bootstrap returns %u\n",
           cov.frames,cov.draw_ticks,cov.child_draws,(unsigned)cov.packet_bytes,(unsigned)cov.ot_links,
           (unsigned long long)cov.polygons,(unsigned long long)cov.lines,(unsigned long long)cov.rectangles,
           cov.texture_uploaded,cov.boundaries,cov.bootstrap_calls);
    ASSERT(cov.boundaries==0u&&cov.bootstrap_calls==0u,"a boundary or bootstrap return was recorded during the frames");
    ASSERT(cov.frames>=32u&&cov.child_draws>0u,"children did not live and draw across frames");
    ASSERT(cov.packet_bytes>0u&&cov.ot_links>0u,"draw pass emitted no OT-linked packets");
    ASSERT(cov.polygons>0u,"GPU executed no polygon from the 801F0068 draw pass");
    ASSERT(cov.texture_uploaded,"texture page upload did not run");
    PASS();
}
#endif
