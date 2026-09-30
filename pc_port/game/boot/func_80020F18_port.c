/* Retail command-selection initialization (20F18, 374E8, 43240) and
 * effect cleanup (70064, 6FC18). Authority: 11718.s, 27C6C.s, 5F484.s.
 * The nine destruction callbacks used by this path are matching C leaves:
 * C7DC4, C8F08, C9C00, CA798, CD960, CCF80, CE1DC, CBFA4, D4850.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_guestcode.h"
#include "game_port.h"

static pe_addr_t effect_slot(uint32_t index)
{
    pe_addr_t slot=index<11u ? PE_LoadU32(0x800942E4u)+index*0xA0Cu :
                              PE_LoadU32(0x800942E8u)+(index-11u)*0x10Cu;
    /* Retirement also runs before the pools are initialized. Retail reads
     * physical RAM in that case; preserve the read through its cached alias. */
    return slot<0x200000u ? slot|0x80000000u : slot;
}

static void clear_effect_slot(pe_addr_t slot)
{
    if (PE_LoadU8(slot+1u)==0x72u) {
        unsigned i;
        for (i=0;i<7;i++) PE_StoreU32(0x800E10A0u+i*4u,0u);
        PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)&~0x10000u);
    }
    PE_StoreU8(slot,0u);
    PE_StoreU8(slot+1u,255u); PE_StoreU8(slot+2u,255u); PE_StoreU8(slot+3u,255u);
    PE_StoreU32(slot+4u,0u); PE_StoreU32(slot+8u,0u);
}

int32_t func_8006FC18(uint32_t index, pe_addr_t owner, uint32_t force)
{
    /* src/func_8006FC18.c (Pm_Stop, audit item 27): the destroy callback
     * (entry+0x14) runs with (slot, clamped code, force) and its result is
     * the return value; the slot is then freed (0x72 also clears the seven
     * D_800E10A0 words and D_800B0CD8 bit 16 -- clear_effect_slot). */
    pe_addr_t slot, entry, fn;
    uint32_t state, code;
    int32_t result;
    if (index>=22u) return -22;
    slot=effect_slot(index); state=PE_LoadU8(slot);
    if (!state || state==6u || (!force && PE_LoadU32(slot+8u)!=owner)) return 0;
    code=PE_LoadU8(slot+1u);
    if (code>=0xC0u) return -23;
    if (code>=0x55u) code=0x55u;
    entry=PE_LoadU32(PE_LoadU32(0x800942E0u)+code*4u);
    if (!entry) return -24;
    fn=PE_LoadU32(entry+0x14u);
    if (!fn) return -1;
    if ((fn==0x80192090u && PE_M32MovementOverlay()) ||
        (fn==0x801920A0u && PE_M28MovementOverlay()))
        result=PE_M28MovementCleanup(slot);          /* host room-overlay callbacks */
    else if (fn==0x8018F1B8u && PE_M34BossEffectOverlay())
        result=PE_M34BossEffectCleanup(slot);
    else if (fn==0x8018F1B4u && PE_M0348iEffectOverlay())
        result=PE_M0348iEffectCleanup(slot);
    else if (PE_GuestCode_Resolve(fn)) {
        unsigned epoch=PE_Port_StopEpoch();
        result=PE_GuestCall("func_8006FC18",fn,3u,slot,code,force,0u);
        if (PE_Port_StopEpoch()!=epoch) return result;
    } else {
        if (g_pe_strict_effect_stack) {
            (void)Bootstrap_ReturnInt4Indirect("effect_destroy_callback", "func_8006FC18",
                -1,fn,slot,owner,force,0u,NULL,0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return -1;
        }
        /* DAY1 SHIM (DAY1_FIDELITY_GAPS.md, "unported room effect init /
         * callback"): a destroy callback with no host implementation is
         * skipped (result 0) and the slot freed. */
        {
            static pe_addr_t seen[16]; static unsigned nseen; unsigned k;
            for (k = 0; k < nseen && seen[k] != fn; k++) {}
            if (k == nseen && nseen < 16u) {
                seen[nseen++] = fn;
                fprintf(stderr,"[DAY1_SHIM] func_8006FC18: destroy callback %08X (token %08X) not ported; skipped\n",
                        fn,(unsigned)PE_LoadU32(0x8009D280u));
            }
        }
        result=0;
    }
    clear_effect_slot(effect_slot(index));
    return result;
}

/* func_80070064: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80070064_port.c (src/func_80070064.c); hand port retired (audit batch, port3). */

/* 6FE14 releases every effect owned by an actor, across both pools. The
 * second clear is present in retail even when 6FC18 ignored a dormant slot. */
/* func_8006FE14: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006FE14_port.c (src/func_8006FE14.c); hand port retired (port3 switch-over C). */

/* 702DC and 701B4: release each complete pool, including dormant headers. */
static int32_t clear_effect_pool(unsigned first)
{
    unsigned i;
    for (i=first;i<first+11u;i++) {
        int32_t result=func_8006FC18(i,0u,1u);
        if (result) return result;
        clear_effect_slot(effect_slot(i));
    }
    return 0;
}

/* func_800702DC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800702DC_port.c (src/func_800702DC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
int32_t func_800701B4(void) { return clear_effect_pool(11u); }
/* func_800703F4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800703F4_port.c (src/func_800703F4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* Script 98: find the selected live actor, release its effects and continue.
 * This is used by Eve's first-hit scene before the next animation/dialogue. */
int func_8001930C(pe_addr_t args)
{
    pe_addr_t actor;
    uint32_t kind=PE_LoadU32(PE_LoadU32(args));
    if (!kind) actor=PE_LoadU32(0x8009D254u);
    else {
        actor=PE_LoadU32(0x8009D20Cu);
        while (actor) {
            if (PE_LoadU8(actor+0xCu)==kind &&
                PE_LoadU8(actor+0xDu)==PE_LoadU32(PE_LoadU32(args+4u)) &&
                !(PE_LoadU32(actor+0x98u)&0x10u)) break;
            actor=PE_LoadU32(actor+4u);
        }
    }
    (void)func_8006FE14(actor);
    return 1;
}

/* func_800374E8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800374E8_port.c (src/func_800374E8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80043240: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80043240_port.c (src/func_80043240.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80020F18(void)
{
    pe_addr_t record, weapon;
    unsigned i;
    (void)func_80070064();
    PE_StoreU32(0x8009D200u,0xFFFFFFFFu);
    PE_StoreU32(0x8009D2FCu,0xFFFFFFFFu);
    PE_StoreU32(0x8009D258u,0xFFFFFFFFu);
    PE_StoreU32(0x8009D208u,0xFFFFFFFFu);
    for (i=0;i<45;i++) {
        pe_addr_t out=0x800BE830u+i*8u;
        PE_StoreU32(out,0u); PE_StoreU16(out+6u,0u); PE_StoreU16(out+4u,0u);
    }
    record=PE_LoadU32(0x8009D278u); weapon=PE_LoadU32(record+0x68u);
    PE_StoreU8(0x8009CE44u,0u); PE_StoreU8(0x8009CE40u,0u);
    PE_StoreU8(0x8009D294u,0u);
    PE_StoreU8(0x8009D2D8u,(uint8_t)((PE_LoadU32(weapon+0x10u)>>4u)&3u));
    PE_StoreU8(0x8009D1DCu,(uint8_t)(PE_LoadU32(weapon+0x10u)&15u));
    for (i=0;i<4;i++) PE_StoreU8(0x8009CE38u+i,PE_LoadU8(weapon+0x14u+i));
    func_800374E8();
    PE_StoreU8(0x8009D1CEu,0u);
    PE_StoreU32(0x8009D1ACu,PE_LoadU32(0x8009D1ACu)&~0x300u);
    func_80026FD0();
    PE_StoreU8(0x8009CE60u,0u);
}
