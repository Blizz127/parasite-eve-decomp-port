/* Original shared weapon-effect renderer, child scheduler and bytecode VM.
 * Authority: B2AF8.s / B3390.s and the matching callback leaves in src/. */
#include "psx_compat.h"
#include "pe_guestcode.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>

/* BE180.s: impact origin selection and the two child initializers. */
static void weapon_hit_origin(void)
{
    pe_addr_t body=PE_LoadU32(0x800E2248u);
    uint32_t index=PE_LoadU32(body+60u);
    pe_addr_t list=PE_LoadU32(body+76u),actor=PE_LoadU32(list+index*4u);
    unsigned i;
    PE_StoreU32(0x800E2804u,actor);
    if (!PE_LoadU32(list+(index+1u)*4u)) PE_StoreU32(PE_LoadU32(0x800E2248u)+64u,1u);
    PE_StoreU32(PE_LoadU32(0x800E2248u)+60u,index+1u);
    actor=PE_LoadU32(0x800E2804u);
    for (i=0;i<3;i++) PE_StoreU16(0x800E27F0u+i*2u,PE_LoadU16(actor+616u+i*2u));
    if (PE_LoadU8(0x800B0CE8u)) (void)func_80086608(PE_LoadU32(0x800B0E14u),0u,128u,127u);
    func_800CEDA8(0);
}

static void weapon_hit_child(pe_addr_t data,int particles)
{
    unsigned i;
    for (i=0;i<3;i++) PE_StoreU16(data+4u+i*2u,PE_LoadU16(0x800E27F0u+i*2u));
    PE_StoreU16(data+10u,(uint16_t)(func_80071A54()%201u-100u));
    PE_StoreU16(data+12u,(uint16_t)(func_80071A54()%101u-50u));
    PE_StoreU8(data+1u,127u);PE_StoreU16(data+14u,0u);
    if (particles) {
        PE_StoreU8(data+3u,0u);
        for (i=0;i<8;i++) {
            PE_StoreU16(data+16u+i*2u,PE_LoadU16(0x800E27F0u));
            PE_StoreU16(data+32u+i*2u,PE_LoadU16(0x800E27F2u));
            PE_StoreU16(data+48u+i*2u,PE_LoadU16(0x800E27F4u));
            PE_StoreU16(data+64u+i*2u,(uint16_t)(func_80071A54()%50u-25u));
            PE_StoreU16(data+80u+i*2u,(uint16_t)(func_80071A54()%50u-25u));
            PE_StoreU16(data+96u+i*2u,0u);
        }
    }
}

void PE_WeaponCallback(pe_addr_t fn,pe_addr_t slot,pe_addr_t rec,pe_addr_t data)
{
    unsigned i;
    PE_EffectStackWeaponUpdateCall();
    PE_M34StackCallback(fn);
    (void)slot;
    if(PE_M34BossEffectOverlay() && PE_M34BossEffectChild(fn,slot,rec,data))return;
    if(fn==0x8018F240u && PE_M0348iEffectOverlay()) { PE_M0348iEffectF240(slot,data); return; }
    if(PE_M0348iEffectChild(fn,slot,rec,data)) return;
    switch (fn) {
    case 0x800CE1FCu:func_800CE1FC();return;
    case 0x800CE2B4u:func_800CE2B4((int)slot,(int)rec,data);return; /* retail a0/a1/a2 */
    case 0x800CE3B4u:func_800CE3B4((int)slot,(int)rec,data);return; /* retail a0/a1/a2 */
    case 0x800CE3ACu:return; /* Original empty draw leaf. */
    case 0x800CE464u:func_800CE464(slot,rec);return;
    case 0x800CE470u:func_800CE470(slot,rec,data);return;
    case 0x800CD980u:weapon_hit_origin();return;
    case 0x800CDA5Cu:weapon_hit_child(data,1);return;
    case 0x800CDC24u:weapon_hit_child(data,0);return;
    case 0x800CDD0Cu:func_800CDD0C(slot,rec,data);return;
    case 0x800CDE90u:func_800CDE90(slot,rec,data);return;
    case 0x800C9C20u:func_800C9C20(slot,rec,data);return;
    case 0x800C9C8Cu:func_800C9C8C(slot,rec,data);return;
    case 0x800C9D9Cu:func_800C9D9C(slot,rec,data);return;
    case 0x800C9EA8u:func_800C9EA8(slot,rec,data);return;
    case 0x800C9FD8u:func_800C9FD8(slot,rec,data);return;
    case 0x800C9EA0u: case 0x800CDD04u: return; /* Original empty draw leaves. */
    case 0x800CA4A8u: case 0x800CDF40u: PE_StoreU8(rec+1u,2u);return;
    case 0x800CA4B4u:
        for (i=0;i<3;i++) PE_StoreU16(data+8u+i*2u,(uint16_t)(PE_LoadU16(data+8u+i*2u)+PE_LoadU16(data+16u+i*2u)));
        PE_StoreU16(data+18u,(uint16_t)(PE_LoadU16(data+18u)+3u));
        PE_StoreU8(data+1u,(uint8_t)(PE_LoadU8(data+1u)+1u));
        if ((int16_t)PE_LoadU16(data+10u)>0) PE_StoreU16(data+18u,(uint16_t)(0u-PE_LoadU16(data+18u)));
        i=PE_LoadU8(data+2u);PE_StoreU8(data+2u,(uint8_t)(i-1u));
        if (!i) PE_StoreU8(rec+1u,2u);
        return;
    case 0x800CA540u:
        PE_StoreU16(data+4u,(uint16_t)(PE_LoadU16(data+4u)-20u));
        if ((int16_t)PE_LoadU16(data+4u)<20) { PE_StoreU16(data+4u,0u);PE_StoreU8(rec+1u,2u); }
        return;
    case 0x800CDF4Cu:
        PE_StoreU8(data+1u,(uint8_t)(PE_LoadU8(data+1u)-16u));
        PE_StoreU8(data+3u,(uint8_t)(PE_LoadU8(data+3u)+12u));
        for (i=0;i<24;i++) PE_StoreU16(data+16u+i*2u,(uint16_t)(PE_LoadU16(data+16u+i*2u)+PE_LoadU16(data+64u+i*2u)));
        if ((int8_t)PE_LoadU8(data+1u)<16) PE_StoreU8(rec+1u,2u);
        return;
    case 0x800CDFE0u:
        i=func_80071A54();
        PE_StoreU16(data+6u,(uint16_t)(PE_LoadU16(data+6u)-4u));
        PE_StoreU8(data+1u,(uint8_t)(PE_LoadU8(data+1u)-3u));
        PE_StoreU16(data+4u,(uint16_t)(PE_LoadU16(data+4u)-5u+(uint32_t)((int32_t)i%11)));
        if ((int8_t)PE_LoadU8(data+1u)<3) PE_StoreU8(rec+1u,2u);
        return;
    }
    if (PE_GuestCode_Resolve(fn)) {   /* generated overlay / EXE TU (pe_guestcode.h) */
        (void)PE_GuestCall("@800C2414:weapon", fn, 3u, (uintptr_t)slot, (uintptr_t)rec, (uintptr_t)data, 0);
        return;
    }
    if (g_pe_strict_effect_stack) {
        fprintf(stderr,"[EFFECT] Unported weapon callback %08X slot %08X record %08X\n",fn,slot,rec);
        Bootstrap_ReturnVoid("PE_WeaponCallback","weapon effect callback");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }
    /* DAY1 SHIM (DAY1_FIDELITY_GAPS.md, "unported room effect init /
     * callback"): skip the unported effect-VM callback and retire its record
     * (rec+1 = 2, as the ported callbacks do when they finish). */
    {
        static pe_addr_t seen[32]; static unsigned nseen; unsigned k;
        for (k = 0; k < nseen && seen[k] != fn; k++) {}
        if (k == nseen && nseen < 32u) {
            seen[nseen++] = fn;
            fprintf(stderr,"[DAY1_SHIM] PE_WeaponCallback: effect callback %08X (token %08X) not ported; record retired\n",
                    fn,(unsigned)PE_LoadU32(0x8009D280u));
        }
    }
    PE_StoreU8(rec+1u,2u);
}

static void weapon_context(pe_addr_t slot,int owner)
{
    PE_StoreU32(0x800E2248u,slot+12u);PE_StoreU32(0x800F34F4u,slot+128u);
    PE_StoreU32(0x800F3330u,slot+512u);PE_StoreU32(0x800F33B0u,PE_LoadU32(slot+120u));
    if (owner) PE_StoreU32(0x800F32A8u,slot);
}

static pe_addr_t weapon_rec(unsigned index) { return PE_LoadU32(0x800F34F4u)+index*6u; }
static pe_addr_t weapon_body(void) { return PE_LoadU32(0x800E2248u); }
static pe_addr_t weapon_data(pe_addr_t rec) { return PE_LoadU32(0x800F3330u)+(uint32_t)(int32_t)(int16_t)PE_LoadU16(rec+4u); }

static void weapon_missing(pe_addr_t message)
{
    /* The original sentinel branch prints a literal diagnostic and continues. */
    uint8_t ch;
    while ((ch=PE_LoadU8(message++))!=0) fputc(ch,stderr);
}

int func_800C2414(pe_addr_t slot,pe_addr_t table)
{
    unsigned i;
    weapon_context(slot,0);
    for (i=0;i<64;i++) {
        pe_addr_t rec=weapon_rec(i),fn;
        if ((int8_t)PE_LoadU8(rec+1u)!=1) continue;
        fn=PE_LoadU32(table+PE_LoadU8(rec)*4u);
        if (fn==0xFFFFFFFFu) weapon_missing(0x800C20C8u);
        else PE_WeaponCallback(fn,slot,rec,weapon_data(rec));
        PE_EffectStackWeaponCallback(fn,weapon_data(rec));
    }
    return 0;
}

void func_800C2D0C(unsigned index,unsigned code,unsigned size)
{
    pe_addr_t rec=weapon_rec(index&65535u),body=weapon_body();
    int32_t offset=(int16_t)PE_LoadU16(body+4u);
    PE_StoreU8(rec+1u,1u);PE_StoreU8(rec,(uint8_t)code);PE_StoreU16(rec+2u,0u);
    if (((uint32_t)offset+(size&65535u))>=0x80Cu) offset=0;
    PE_StoreU16(rec+4u,(uint16_t)offset);PE_StoreU16(body+4u,(uint16_t)((uint32_t)offset+size));
    PE_StoreU8(body+6u,(uint8_t)(PE_LoadU8(body+6u)+1u));
}

/* func_800C9B68: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C9B68_port.c (src/func_800C9B68.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_800C2DA0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C2DA0_port.c (src/func_800C2DA0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_800C2E08(void)
{
    int result=0;
    unsigned i;
    for (i=0;i<64;i++) if ((int8_t)PE_LoadU8(weapon_rec(i)+1u)) result|=func_800C2DA0(i);
    return result;
}

pe_addr_t func_800C2B90(pe_addr_t slot,unsigned code,pe_addr_t sizes,pe_addr_t callbacks)
{
    PE_EffectStackWeaponUpdateCall();
    unsigned i;
    int kind;
    pe_addr_t rec,data,fn;
    for (i=0;i<64;i++) if (!(int8_t)PE_LoadU8(weapon_rec(i)+1u)) break;
    if (i==64) { PE_StoreU32(weapon_body()+64u,1u);return 0u; }
    kind=func_800C6CE0(slot);
    if (kind!=3 && kind!=4 && kind!=5) return 0u;
    code&=255u;func_800C2D0C(i,code,PE_LoadU16(sizes+code*2u));
    rec=weapon_rec(i);data=weapon_data(rec);fn=PE_LoadU32(callbacks+code*4u);
    if (fn!=0xFFFFFFFFu) PE_WeaponCallback(fn,slot,rec,data);
    return data;
}

int func_800C251C(pe_addr_t slot,pe_addr_t table)
{
    pe_addr_t owner=PE_LoadU32(slot+8u),body,action;
    unsigned i;
    int result=0;
    weapon_context(slot,1);PE_StoreU8(slot+3u,PE_LoadU8(slot+18u));
    if (func_800C6CE0(slot)==3 && PE_LoadU8(slot+1u)!=36u) {
        action=PE_LoadU32(PE_LoadU32(owner)+24u);
        if (PE_LoadU8(action)==1u) PE_StoreU8(action,2u);
    }
    for (i=0;i<64;i++) {
        pe_addr_t rec=weapon_rec(i),fn;
        if ((int8_t)PE_LoadU8(rec+1u)==1) {
            fn=PE_LoadU32(table+PE_LoadU8(rec)*4u);
            if (fn==0xFFFFFFFFu) weapon_missing(0x800C20ECu);
            else PE_WeaponCallback(fn,slot,rec,weapon_data(rec));
            rec=weapon_rec(i);PE_StoreU16(rec+2u,(uint16_t)(PE_LoadU16(rec+2u)+1u));
        }
        if ((int8_t)PE_LoadU8(weapon_rec(i)+1u)==2) result|=func_800C2DA0(i);
    }
    if (func_800C6CE0(slot)==3) {
        body=PE_LoadU32(owner);
        if (!PE_LoadU8(body+28u+((PE_LoadU32(body)>>17u)&0x70u)) && (PE_LoadU32(body)&0x180Eu)) result=-1;
    }
    return result;
}

int func_800C2758(pe_addr_t slot,pe_addr_t callbacks,pe_addr_t sizes)
{
    pe_addr_t body;
    int result=0,stop=0;
    int16_t delay;
    weapon_context(slot,1);delay=(int16_t)PE_LoadU16(slot+12u);
    if (delay) { PE_StoreU16(slot+12u,(uint16_t)(delay-1));return 0; }
    while (!stop) {
        uint32_t op,high;
        int32_t value;
        pe_addr_t local;
        body=weapon_body();
        op=PE_LoadU32(PE_LoadU32(0x800F33B0u)+(uint32_t)(int32_t)(int16_t)PE_LoadU16(body+2u)*4u);
        if (op==0xFFFFFFFDu) result|=func_800C2E08();
        if (op==0xFFFFFFFEu) op=0;
        high=op>>16u;value=(int16_t)op;
        if (op==0xFFFFFFFFu) { stop=1;PE_StoreU8(weapon_body()+7u,1u); }
        else {
            if (high==1u) (void)func_800C2B90(slot,op&255u,sizes,callbacks);
            if (high==2u) { PE_StoreU16(weapon_body(),(uint16_t)op);stop=1; }
            body=weapon_body();
            if (high>=0x10u && high<0x20u) PE_StoreU32(body+8u+(high-0x10u)*4u,(uint32_t)value);
            if (high>=0x20u && high<0x30u) {
                local=body+8u+(high-0x20u)*4u;PE_StoreU32(local,PE_LoadU32(local)+(uint32_t)value);
            }
            if (high>=0x30u && high<0x40u) {
                local=body+8u+(high-0x30u)*4u;PE_StoreU32(local,PE_LoadU32(local)-(uint32_t)value);
            }
            if (high>=0x40u && high<0x50u) PE_StoreU32(body+8u+(high-0x40u)*4u,PE_LoadU32(body+72u+(op&65535u)*4u));
            if (high>=0x50u && high<0x60u) PE_StoreU32(body+72u+(op&65535u)*4u,PE_LoadU32(body+8u+(high-0x50u)*4u));
            if (high>=0x1000u && high<0x3000u) {
                int equal=PE_LoadU32(body+8u+((high&0xF00u)>>6u))==(uint32_t)value;
                if (equal==(high<0x2000u)) PE_StoreU16(body+2u,(uint16_t)(PE_LoadU16(body+2u)+(int8_t)high));
            }
            if (high>=0x3000u && high<0x4000u) PE_StoreU16(body+2u,(uint16_t)(PE_LoadU16(body+2u)+(int8_t)high));
        }
        body=weapon_body();
        if (!(int8_t)PE_LoadU8(body+7u)) PE_StoreU16(body+2u,(uint16_t)(PE_LoadU16(body+2u)+1u));
        else if (!(int8_t)PE_LoadU8(body+6u)) result=-1;
        if (PE_Port_ShouldStop()) return result;
    }
    return result;
}

/* func_800CD8C8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800CD8C8_port.c (src/func_800CD8C8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_800C9B90: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C9B90_port.c (src/func_800C9B90.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_800CD8F0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800CD8F0_port.c (src/func_800CD8F0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
