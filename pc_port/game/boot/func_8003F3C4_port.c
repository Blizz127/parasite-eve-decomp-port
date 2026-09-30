/*
 * PE-BTL38 — func_8003F3C4 field-tick named cut (translated
 * retail sites, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 227 words 0x8003F3C4..0x8003F750. Sole TEXT caller 1220C @
 * 0x800123D8. 1220C stores D1C4=D280 immediately before the
 * jal, so the 3F3E8 equality holds on entry.
 *
 * This cut is the live mailbox/fade/message sites only:
 *   3F074 371B0 CE90 once when CE90==0 (not 34FC4/125E0)
 *   host pad idle / state-2 Cross before 3EB04
 *   jal 3EB04 @ 0x8003F40C
 *   jal 65400 @ 0x8003F4E8
 *   jal 35558 @ 0x8003F4F0
 *   jal 68CE0 @ 0x8003F560
 *   jal 37870 @ 0x8003F568
 *   jal 661A4 @ 0x8003F570 (GTE OFX/OFY from BCF94/96)
 *   jal 661CC @ 0x8003F580 (SetGeomOffset 160,112)
 *   jal 68E24 @ 0x8003F588
 *   when (B0CD8&0x100)==0 and (B0CD8&0x200)==0
 *
 * jal E01BC @ 3F578. Live E21A4<=0 early-outs. E026C/E03A0
 * are not this cut.
 * jal 70E54 @ 3F590 is the real frame tail since Phase FTE1
 * (game/boot/func_80070E54_port.c): DrawSync(0), 42FE8,
 * VSync(2|4), 74A44(1), PutDispEnv(BCE80+20*CDDC), then
 * PutDrawEnv or DrawOTagEnv -> 754E4 -> 76C34(76B98) walk,
 * and the guest CDDC flip.
 * 3F5EC VSync(2) then 6A0E8 @ 3F640. Live D1A0&0x10
 * early-out. 66C7C/6A25C are not live.
 * 3F684 D1C4==D280 loops to 3EB04 in retail; this cut
 * is one pass per 1220C tick. Dest-change (D1C4!=D280)
 * takes 74DC0 / 87024 / 3DFC8(1) / 696F0 live tail
 * then D1A0|=0x40 / B0CD8|=2.
 *
 * PE-BTL102: andi 0x100 after 35558 @ 3F500 is not a
 * function return. bnez 3F5F4 skips overlay/draw/6A0E8
 * and still falls into dest-change when D280 moved
 * (player-death 6A25C). 3F624 pad-combo 6A25C is not
 * this cut. 1220C then VSync(0)/SetDispMask(0),
 * B0CD8 &= ~0x100, j 1224C (outer 6A5BC restart).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

#define GA_D_800BE9A2 0x800BE9A2u
#define GA_D_800BCEA8 0x800BCEA8u
#define GA_D_8009CE90 0x8009CE90u
#define GA_D_800B162C 0x800B162Cu
#define GA_OVERLAY    0x800B0CD8u
#define REC_STRIDE    56u



extern void func_8003DFC8(void); /* generated from src/func_8003DFC8.c */
extern void func_80087024(void);
extern void func_800661A4(void);
extern void func_800661CC(void);
extern int func_80074A44(int mode);
extern void func_800754E4(pe_addr_t ot, pe_addr_t env);

/* func_8003DFC8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8003DFC8_port.c (src/func_8003DFC8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static void effect_module_finish(unsigned int code)
{
    pe_addr_t table = PE_LoadU32(0x800942E0u);
    pe_addr_t descriptor = PE_LoadU32(table + code * 4u);
    pe_addr_t callback;
    if (!descriptor) return;
    callback = PE_LoadU32(descriptor + 24u);
    if (!callback) return;
    /* These eight original module finalizers are jr ra / move v0,zero.
     * Their arguments are unused; 696F0 does not initialize a0 here. */
    switch (callback) {
    case 0x800C7DD4u: case 0x800C8F18u: case 0x800C9C10u:
    case 0x800CA7A8u: case 0x800CD970u: case 0x800CCF90u:
    case 0x800CE1ECu: case 0x800CBFB4u:
        return;
    default:
        (void)PE_EffectCallback(callback, (int32_t)code, 0u, 0u);
    }
}

/* SEW16: original 121 words, 800696F0..800698D4. Finish modules before
 * their overlay is replaced, then clear registry entries (not descriptors). */
void func_800696F0(void)
{
    unsigned int i;
    pe_addr_t table = PE_LoadU32(0x800942E0u);
    if (D_8009D1A0 & 0x80u) {
        pe_addr_t package, header, entry;
        uint32_t packed;
        for (i = 0; i < 8u; i++) effect_module_finish(i);
        effect_module_finish(0x55u);
        package = PE_LoadU32(0x800B0E64u);
        header = package + PE_LoadU32(package + 4u);
        packed = PE_LoadU32(header + 4u);
        entry = package + (packed & 0x3FFFFFu);
        for (i = 0; i < (PE_LoadU32(header + 4u) >> 22u); i++, entry += 12u) {
            unsigned int code = PE_LoadU8(entry + 7u);
            if (code - 8u < 0x4Du) effect_module_finish(code);
        }
        D_8009D1A0 &= ~0x80u;
    }
    table = PE_LoadU32(0x800942E0u);
    if (table != 0u && PE_RangeIsRam(table, 0x55u * 4u)) {
        for (i = 8u; i < 0x55u; i++)
            PE_StoreU32(table + i * 4u, 0u);
    }
    for (i = 0x1Eu; i < 0x68u; i++)
        PE_StoreU32(0x800E1044u + i * 4u, 0u);
}

/* func_8006A0E8 — full retail body (src/func_8006A0E8.c, audio2 lane).
 * Called every field tick after the draw.  On the pause-entry frame
 * (D1A0 bit 0x10, set by the Start toggle above) it draws the pause
 * overlay through the current draw env and sends AKAO command 0x99
 * (func_800867E4(0): pause music + SFX); on the pause-exit frame (bit
 * 0x20) it sends 0x98 (func_8008682C(0): resume).  The earlier cut
 * stopped before both, so music and effects kept playing while paused. */
extern void func_800867E4(int a0);
extern void func_8008682C(int a0);
void func_8006A0E8(void)
{
    const pe_addr_t base = 0x800B0CD8u;
    if ((PE_LoadU32(base) & 0x200u) == 0u && (D_8009D1A0 & 0x10u) != 0u) {
        uint32_t idx = PE_LoadU32(0x8009CDDCu) * 0x5Cu;
        PE_StoreU8(0x800BCDE0u + idx, 0u);
        PE_StoreU8(0x800BCDDFu + idx, 1u);
        (void)func_80075424(0x800BCDC8u + idx);
        (void)func_80075358(base + 0x114u);
        (void)func_80075358(base + 0x104u);
        PE_StoreU8(0x800BCDE0u + idx, 1u);
        PE_StoreU8(0x800BCDDFu + idx, 0u);
        func_800867E4(0);
    }
    if ((D_8009D1A0 & 0x20u) != 0u)
        func_8008682C(0);
}

/* Original E01BC..E0808: effect walk, animation/fade, projected FT4. */
#include "pe_sdk.h"
/* func_800E051C: camera-facing glow sprite (POLY_FT4 code 0x2E), audited line by
 * line against src/func_800E051C.c (inline-GTE leaf; the generator skips asm, so
 * this is its native implementation).  Exported for func_800E026C/800E03A0. */
void func_800E051C(void)
{
    const pe_addr_t scratch=0x1F800000u;
    uint32_t offset=PE_LoadU32(0x8009CDD8u),bank=PE_LoadU32(0x8009CDDCu),xy,z;
    pe_addr_t packet=PE_LoadU32(0x800B0E58u+bank*4u)+offset;
    PE_StoreU32(0x8009CDD8u,offset+40u);
    PE_GTE_LoadRT(PE_LoadU32(scratch+52u));
    PE_GTE_SetV0((int16_t)PE_LoadU16(scratch+36u),(int16_t)PE_LoadU16(scratch+38u),(int16_t)PE_LoadU16(scratch+40u));
    PE_GTE_RTPS_coordinates(&xy,&z);
    PE_StoreU8(packet+3u,9);PE_StoreU8(packet+7u,0x2E);
    for(unsigned i=0;i<3;i++)PE_StoreU8(packet+4u+i,PE_LoadU8(scratch+24u+i));
    PE_StoreU16(packet+14u,PE_LoadU16(scratch+30u));
    PE_StoreU16(packet+22u,PE_LoadU16(scratch+34u));
    for(unsigned i=0;i<4;i++) {
        PE_StoreU8(packet+12u+i*8u,(uint8_t)(PE_LoadU8(scratch+28u)+(i&1u)*16u));
        PE_StoreU8(packet+13u+i*8u,(uint8_t)(PE_LoadU8(scratch+29u)+(i>>1u)*16u));
    }
    PE_StoreU32(scratch,xy);PE_StoreU32(scratch+4u,z);
    uint32_t scale=PE_LoadU32(PE_LoadU32(0x800BCFA8u));
    int32_t numerator=(int32_t)((PE_LoadU32(scratch+44u)<<5u)*scale);
    int32_t denominator=(int32_t)((z<<4u)+scale);
    if(!denominator || (numerator==INT32_MIN && denominator==-1)) {
        Bootstrap_ReturnVoid("func_800E051C_original_division_trap","func_800E051C");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return;
    }
    int32_t half=(numerator/denominator)>>1;
    for(unsigned i=0;i<4;i++) {
        PE_StoreU16(packet+8u+i*8u,(uint16_t)((xy&65535u)+((i&1u)?(uint32_t)half:0u-(uint32_t)half)));
        PE_StoreU16(packet+10u+i*8u,(uint16_t)((xy>>16u)+((i&2u)?(uint32_t)half:0u-(uint32_t)half)));
    }
    pe_addr_t ot=PE_LoadU32(0x800B0E38u+PE_LoadU32(0x8009CDDCu)*4u)+(z&~3u)-32u;
    PE_StoreU32(packet,(PE_LoadU32(packet)&0xFF000000u)|(PE_LoadU32(ot)&0xFFFFFFu));
    PE_StoreU32(ot,(PE_LoadU32(ot)&0xFF000000u)|(packet&0xFFFFFFu));
}
__attribute__((unused)) static void park_effect_tick(pe_addr_t slot,unsigned mode)
{
    const pe_addr_t scratch=0x1F800000u;
    if(!PE_LoadU8(slot))return;
    PE_StoreU16(scratch+30u,mode?0x7713:0x77D3);PE_StoreU16(scratch+34u,0x34);
    PE_StoreU8(scratch+28u,mode?0xC0:PE_LoadU8(slot+3u));PE_StoreU8(scratch+29u,mode?0xCA:0xD0);
    for(unsigned i=0;i<3;i++)PE_StoreU16(scratch+36u+i*2u,PE_LoadU16(slot+4u+i*2u));
    PE_StoreU32(scratch+52u,PE_LoadU32(0x800BCFA4u));
    PE_StoreU32(scratch+44u,(uint32_t)(int32_t)(int16_t)PE_LoadU16(slot+14u));
    for(unsigned i=0;i<3;i++) {
        int color=mode?(int)PE_LoadU8(slot+16u+i)-(int16_t)PE_LoadU16(slot+12u):128;
        PE_StoreU8(scratch+24u+i,(uint8_t)(color<0?0:color));
    }
    if(mode) {
        uint16_t value=(uint16_t)(PE_LoadU16(slot+12u)+(PE_LoadU8(slot)==1?(int)PE_LoadU8(slot+1u):-(int)PE_LoadU8(slot+1u)));
        PE_StoreU16(slot+12u,value);
        if(PE_LoadU8(slot)==1) {
            if((int16_t)value>=256){PE_StoreU16(slot+12u,255);PE_StoreU8(slot,2);}
        } else if((int16_t)value<0) {
            PE_StoreU8(slot,0);PE_StoreU16(slot+12u,0);
            PE_StoreU16(0x800E21A4u,(uint16_t)(PE_LoadU16(0x800E21A4u)-1u));return;
        }
    } else {
        if(PE_LoadU8(slot)!=1)return;
        if(PE_LoadU8(slot+2u))PE_StoreU8(slot+2u,(uint8_t)(PE_LoadU8(slot+2u)-1u));
        else {
            uint8_t frame=(uint8_t)(PE_LoadU8(slot+3u)+16u);
            PE_StoreU8(slot+3u,frame);PE_StoreU8(slot+2u,PE_LoadU8(slot+1u));
            if(frame>=49u) {
                PE_StoreU8(slot,0);PE_StoreU8(slot+3u,0);
                PE_StoreU16(0x800E21A4u,(uint16_t)(PE_LoadU16(0x800E21A4u)-1u));return;
            }
        }
    }
    func_800E051C();
}
/* func_800E01BC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800E01BC_port.c (src/func_800E01BC.c); hand port retired (port3 switch-over J). */
void func_800E01BC(void);

/* func_8006EC08: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006EC08_port.c (src/func_8006EC08.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
int func_8006EC08(void);

static uint32_t pe_3f3c4_loaded_dest;
static uint64_t pe_3f3c4_ram_generation;

static int pe_3f3c4_kseg(pe_addr_t p)
{
    return p >= 0x80000000u && p < 0x80200000u;
}

static int pe_3f3c4_message_state2(void)
{
    unsigned int i;

    for (i = 0; i < 4u; i++) {
        if (PE_LoadU8(GA_D_800BCEA8 + i * REC_STRIDE) == 2u)
            return 1;
    }
    return 0;
}

/*
 * 3F074 @ 3F3D4 is the first jal. Full 3F074 also jals 34FC4
 * (wipes D20C) and 125E0 every call, so this cut is the 3F244
 * 371B0 site only, and only while CE90 is still 0.
 * 6B4F8_12574_publish_cut runs first when overlay+0x18C is
 * already a KSEG chunk2 dest and B162C is still 0.
 */
static void pe_3f3c4_ce90_once(void)
{
    pe_addr_t chunk;
    pe_addr_t stream;

    if (PE_LoadU32(GA_D_8009CE90) != 0u)
        return;
    chunk = PE_LoadU32(GA_OVERLAY + 0x18Cu);
    /* The pre-publish exists for callers that plant chunk 2 directly.  It
     * must not run on a dest buffer whose pointer is valid but whose
     * contents were never loaded (first New-Game tick before 6B4F8): the
     * header walk would follow garbage offsets.  overlay+0x944 == 0 with a
     * nonzero header word at chunk+4 is the "planted, not yet published"
     * shape; dest_change's own load path publishes for everything else. */
    if (pe_3f3c4_kseg(chunk) && PE_LoadU32(GA_D_800B162C) == 0u &&
        PE_LoadU32(GA_OVERLAY + 0x944u) == 0u &&
        pe_3f3c4_kseg(chunk + (PE_LoadU32(chunk + 4u) & 0x003FFFFFu)))
        func_8006B4F8_12574_publish_cut();
    stream = func_8003F074_371b0_a0();
    if (pe_3f3c4_kseg(stream))
        func_800371B0(stream);
    pe_3f3c4_loaded_dest = D_8009D280;
}

/*
 * 3F074 @ 3F088 jals 6B4F8(D280) every tick. Host still
 * change-gates (not a retail skip). Dest-ready is 6B35C +
 * 6B4F8 + 6BECC==0 + 6C5BC==0 + 125E0. Does not write
 * mode 7/9/10. Type-1 clip is not manufactured.
 */
static void pe_3f3c4_dest_change(void)
{
    pe_addr_t dest0;
    pe_addr_t dest1;
    pe_addr_t dest2;

    if (D_8009D280 == 0u || D_8009D280 == pe_3f3c4_loaded_dest)
        return;
    dest0 = PE_LoadU32(GA_OVERLAY + 0x194u);
    dest1 = PE_LoadU32(GA_OVERLAY + 0x168u);
    dest2 = PE_LoadU32(GA_OVERLAY + 0x18Cu);
    if (!pe_3f3c4_kseg(dest0) || !pe_3f3c4_kseg(dest1) ||
        !pe_3f3c4_kseg(dest2))
        return;
    if (PE_LoadU32(0x800B0DD8u) == 0u)
        return;
    if (func_8003F074_dest_ready_cut(D_8009D280) != 0)
        pe_3f3c4_loaded_dest = D_8009D280;
}

/*
 * Host pad before 3EB04. Retail idle raw is 0xFFFF. Host RAM
 * zero after reset is uninitialized, not all-buttons-pressed.
 * A registered window pad source owns input, including dialogue. The
 * deterministic state-2 confirm remains only for callers without a source.
 */
static void pe_3f3c4_host_pad(void)
{
    uint16_t raw;

    if (PE_Port_HasPadSource()) {
        /* BIOS pad buffer: successful reply, ID 41h (digital, one word).
         * The window supplies buttons only; never retain an analog ID. */
        PE_StoreU16(0x800BE9A0u,0x4100u);
        raw = PE_Port_ReadPadRaw();
    }
    else if (pe_3f3c4_message_state2())
        raw = 0xBFFFu;
    else {
        raw = PE_LoadU16(GA_D_800BE9A2);
        if (raw == 0u)
            raw = 0xFFFFu;
    }
    PE_StoreU16(GA_D_800BE9A2, raw);
}

/*
 * PE-FT1 — full func_8003F3C4 control flow (retail 0x8003F3C4..0x8003F754,
 * asm/disc1/2F174.s), replacing the BTL38 live-sites cut.  Register map:
 * s0 = &D_800B0CD8 (overlay word), s2 = &D_800B0CEA, s1 = &D_800BCFE8,
 * gp+0x34 = D_8009CDA4 (frame counter), gp+0x430 = D_8009D1A0,
 * gp+0x454 = D_8009D1C4, gp+0x510 = D_8009D280, gp+0x68 = D_8009CDD8.
 *
 * Host adaptations, both pre-existing and documented:
 *  - 3F074 is the dest-change / 371B0 cut pair, not the whole function.
 *  - The 3F684 `beq D1C4,D280 -> 3F404` inner loop is one pass per 1220C
 *    tick: 1220C stores D1C4=D280 before every jal and re-dispatches here
 *    while D280 is unchanged, so the host main loop (with its stop/frame
 *    policy) stands in for the retail inner loop.  The exit epilogue at
 *    3F68C therefore runs when D280 moved during this pass.
 *  - Host pad before 3EB04 (retail idle raw is 0xFFFF).
 *  - No mid-tick stop polling: as before, every reached site runs and the
 *    1220C caller honors a latched stop at its next continuation point.
 * The overlay message path (3F51C: 122040 / 121A00 / 6E60C) lives in the
 * disc-loaded field code and is an honest boundary if 6EC08 selects it.
 */
#define GA_D_8009CDA4 0x8009CDA4u   /* gp+0x34 frame counter */
#define GA_D_8009CDD8 0x8009CDD8u
#define GA_D_8009CDDC 0x8009CDDCu
#define GA_D_8009D1F4 0x8009D1F4u
#define GA_D_8009D238 0x8009D238u
#define GA_D_8009D26C 0x8009D26Cu
#define GA_D_800B0CEA 0x800B0CEAu
#define GA_D_800BCFE8 0x800BCFE8u
#define GA_OT_TABLE   0x800B0E38u   /* s0 + 0x160 */

void func_8003F3C4(void)
{
    uint32_t bits;
    uint32_t dest0;
    uint32_t d1a0;
    uint32_t a1;
    uint32_t a0;

    /* A destination cached against a previous RAM image is not loaded
     * after reset, even when New Game chooses the same token. */
    if (pe_3f3c4_ram_generation != PE_RamGeneration()) {
        pe_3f3c4_loaded_dest = 0u;
        pe_3f3c4_ram_generation = PE_RamGeneration();
    }
    dest0 = D_8009D280;
    /* 3F3D4 jal 3F074.  Retail order inside 3F074: 6B35C + 6B4F8 dest
     * load (3F088) precede the 3F244 371B0 site; loading first is also
     * what makes the first New-Game tick sound, because ce90_once reads
     * the chunk-2 header the load has just filled. */
    pe_3f3c4_dest_change();
    pe_3f3c4_ce90_once();
    /* 3F3E8: `bne D1C4, D280 -> 3F68C`.  1220C stores D1C4 = D280
     * immediately before the jal, so on entry D1C4 == dest0; the snapshot
     * is compared instead of the native D_8009D1C4 so tests that drive the
     * tick directly (stale D1C4) keep the 1220C contract.  A dest that
     * 3F074 already moved skips straight to the epilogue. */
    if (dest0 != D_8009D280)
        goto epilogue;

    /* .L8003F404 */
    PE_StoreU32(GA_D_8009CDD8, 0u);
    PE_StoreU8(GA_D_800B0CEA, 0u);            /* jal delay slot */
    pe_3f3c4_host_pad();
    func_8003EB04();

    /* 3F414..3F484: D1A0 &= ~0x30, then the optional 0x10/0x20 select. */
    a1 = D_8009D1A0;
    a0 = a1 & ~0x30u;
    D_8009D1A0 = a0;
    PE_StoreU32(0x8009D1A0u, a0);
    if ((PE_LoadU32(0x800B0CD8u) & 0x8000u) == 0u &&
        PE_LoadU32(GA_D_8009CDA4) != 0u &&
        (PE_LoadU32(GA_D_8009D1F4) & 0x4u) != 0u &&
        (PE_LoadU32(GA_D_8009D238) & 0xB0002380u) == 0u) {
        d1a0 = ((a1 & 1u) != 0u ? (a0 | 0x20u) : (a0 | 0x10u)) ^ 1u;
        D_8009D1A0 = d1a0;
        PE_StoreU32(0x8009D1A0u, d1a0);
    }

    /* .L8003F488: D1A0 bit 0 skips the whole update/draw to VSync(2). */
    if ((D_8009D1A0 & 1u) != 0u) {
        func_80073A44(2);                      /* .L8003F5EC */
        goto after_draw;
    }
    /* 3F4A4: per-frame ClearOTagR(OT[CDDC], 0x1000) unless bit 9. */
    if ((PE_LoadU32(0x800B0CD8u) & 0x200u) == 0u) {
        uint32_t cddc = PE_LoadU32(GA_D_8009CDDC);
        func_800752AC(PE_LoadU32(GA_OT_TABLE + cddc * 4u), 0x1000);
    }
    /* .L8003F4D0 — update the same guest word that 3E680 zeroes. */
    D_8009D250++;
    func_80065400();
    func_80035558_walk_cut();
    bits = PE_LoadU32(0x800B0CD8u);
    /* 3F500 andi 0x100 / bnez 3F5F4: skip update+draw, not jr. */
    if ((bits & 0x100u) != 0u)
        goto after_draw;
    /* 3F50C: 6EC08 low byte nonzero selects the overlay message path. */
    if ((func_8006EC08() & 0xFFu) != 0u) {
        int movie=func_80122040();
        if (PE_Port_ShouldStop()) return;
        if ((uint8_t)movie) goto frame_tail;
        func_80121A00();
        if (PE_Port_ShouldStop()) return;
        func_8006E60C();
        if (PE_Port_ShouldStop()) return;
        goto loop_tail;
    }
    /* .L8003F54C */
    if ((PE_LoadU32(0x800B0CD8u) & 0x200u) == 0u) {
        func_80068CE0();
        func_80037870();
        func_800661A4();
        func_800E01BC();
        func_800661CC();
        func_80068E24();
    }
    /* .L8003F590: the real frame tail (Phase FTE1,
     * game/boot/func_80070E54_port.c) — DrawSync, 42FE8, VSync,
     * 74A44(1), PutDispEnv, then PutDrawEnv or DrawOTagEnv through the
     * 754E4 -> 76C34(76B98) chain walk, and the CDDC flip. */
frame_tail:
    func_80070E54();
    /* 3F598..3F5E4: first frame only, BCFE8 == 0x00FF00FF / +4 == 0xFF /
     * +6 bit 6 -> 66C7C(0xF). */
    if (PE_LoadU32(GA_D_8009CDA4) == 0u &&
        PE_LoadU32(GA_D_800BCFE8) == 0x00FF00FFu &&
        (int16_t)PE_LoadU16(GA_D_800BCFE8 + 4u) == 0xFF &&
        (PE_LoadU8(GA_D_800BCFE8 + 6u) & 0x40u) != 0u) {
        (void)func_80066C7C(0xFu);
    }

after_draw:
    /* .L8003F5F4: pad-combo player-death 6A25C unless bits 9/14. */
    if ((PE_LoadU32(0x800B0CD8u) & 0x4200u) == 0u &&
        (PE_LoadU32(GA_D_8009D26C) & 0x0F000006u) == 0x0F000006u) {
        func_8006A25C();
    }
    /* .L8003F62C */
    if ((PE_LoadU32(0x800B0CD8u) & 0x100u) == 0u)
        func_8006A0E8();
    /* .L8003F648: frame counter; D1A0 bit 13 with overlay bit 11 exits. */
    PE_StoreU32(GA_D_8009CDA4, PE_LoadU32(GA_D_8009CDA4) + 1u);
    if ((D_8009D1A0 & 0x2000u) != 0u &&
        (PE_LoadU32(0x800B0CD8u) & 0x800u) != 0u)
        goto epilogue;
    /* .L8003F678: retail loops to 3F404 while D1C4 == D280 (host: one
     * pass per 1220C tick, see header).  Entry D1C4 == dest0. */
loop_tail:
    if (dest0 == D_8009D280)
        return;

epilogue:
    /* .L8003F68C */
    if ((PE_LoadU32(0x800B0CD8u) & 0x200u) != 0u) {
        RECT rect;

        rect.x = 0;
        rect.y = 0;
        rect.w = 0x140;
        rect.h = 0x1C0;
        func_80074F44(&rect, 0, 0, 1);         /* ClearImage */
    }
    /* .L8003F6CC */
    func_80074DC0(0);
    func_80087024();
    func_8003DFC8();
    func_800696F0();
    /* 3F6EC..3F738: D1A0 = (D1A0|0x40) & ~0x3800; B0CD8 = (B0CD8|2) &
     * ~0x800, then if bit 9 was set B0CD8 = (that|2) & 0xFFFF7DFF. */
    d1a0 = (D_8009D1A0 | 0x40u) & ~0x3800u;
    D_8009D1A0 = d1a0;
    PE_StoreU32(0x8009D1A0u, d1a0);
    {
        uint32_t v1 = PE_LoadU32(0x800B0CD8u) | 2u;
        uint32_t v0 = v1 & ~0x800u;

        PE_StoreU32(0x800B0CD8u, v0);
        if ((v1 & 0x200u) != 0u)
            PE_StoreU32(0x800B0CD8u, (v0 | 2u) & 0xFFFF7DFFu);
    }
}
