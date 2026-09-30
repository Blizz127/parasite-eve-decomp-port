/* Original PE battle command lifecycle, animation and target cleanup.
 * Authority:120D8.s 22394..22D78 and AB74.s 20D50..20DCC. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

static pe_addr_t pe_action_ram(pe_addr_t p) {return p<0x200000u?p|0x80000000u:p;}
static pe_addr_t pe_action_aya(void) {return pe_action_ram(PE_LoadU32(0x8009D254u));}
static pe_addr_t pe_action_record(void) {return pe_action_ram(PE_LoadU32(0x8009D278u));}
static pe_addr_t pe_action_slot(unsigned index) {return 0x800BE830u+index*8u;}
static void pe_action_flags(pe_addr_t p,uint32_t clear,uint32_t set)
{PE_StoreU32(p,(PE_LoadU32(p)&~clear)|set);}
static void pe_action_release(void)
{
    unsigned next=PE_LoadU16(pe_action_slot(PE_LoadU8(0x8009D1D4u))+4u);
    if (next-3u>=404u || PE_LoadU8(pe_action_aya()+14u)<4u) D_8009D1A0&=~256u;
}
static unsigned pe_action_effect_active(void)
{
    pe_addr_t p=PE_LoadU32(0x800942E4u)+PE_LoadU32(0x8009D258u)*0xA0Cu;
    return (unsigned)PE_LoadU8(pe_action_ram(p))-1u<2u;
}

/* func_80020D50: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80020D50_port.c (src/func_80020D50.c); hand port retired (audit batch, port3). */

void func_80022394(void)
{
    unsigned index=PE_LoadU8(0x8009D1D4u),i;
    pe_addr_t slot=pe_action_slot(index),aya,record,target;
    uint32_t flags;
    if (!(int8_t)PE_LoadU8(0x8009D2A0u)) {
        for (i=index;(uint8_t)i<PE_LoadU8(0x8009CE3Cu);i++) {
            unsigned id=PE_LoadU16(pe_action_slot((uint8_t)i)+4u);
            if (id-3u<384u) func_80053D2C((int16_t)id-3);
        }
        PE_StoreU8(0x8009CE3Cu,0u);PE_StoreU8(0x8009D1D4u,0u);D_8009D1A0&=~256u;return;
    }
    if (PE_LoadU32(pe_action_record()+76u)&0x200000u) {
        int32_t ability=(int16_t)PE_LoadU16(slot+4u)-387;
        int invalid=0;
        target=PE_LoadU32(slot);aya=pe_action_aya();
        if (target!=aya) {
            PE_StoreU16(pe_action_aya()+58u,(uint16_t)func_80030584(target+436u,aya+40u));
            index=PE_LoadU8(0x8009D1D4u);
            if ((int16_t)PE_LoadU16(pe_action_slot(index)+4u)-387==19) {
                pe_addr_t replacement=0u;
                for (i=index;(uint8_t)i<index+7u;i++) {
                    pe_addr_t actor=PE_LoadU32(pe_action_slot((uint8_t)i));
                    if (actor && (int32_t)PE_LoadU32(pe_action_ram(PE_LoadU32(actor)+16u))>0 &&
                        !(PE_LoadU32(actor+152u)&0x4000u)) {replacement=actor;break;}
                }
                if (!replacement) {PE_StoreU8(0x8009D1D4u,(uint8_t)(index+7u));invalid=1;}
                else for (i=index;(uint8_t)i<index+7u;i++) {
                    pe_addr_t p=pe_action_slot((uint8_t)i),actor=PE_LoadU32(p);
                    if ((int32_t)PE_LoadU32(pe_action_ram(PE_LoadU32(pe_action_ram(actor))+16u))<=0 || !actor)
                        PE_StoreU32(p,replacement);
                }
            } else {
                pe_addr_t actor=PE_LoadU32(pe_action_slot(index));
                if (!actor || (int32_t)PE_LoadU32(pe_action_ram(PE_LoadU32(actor)+16u))<=0 ||
                    (PE_LoadU32(actor+152u)&0x4000u)) {
                    PE_StoreU8(0x8009D1D4u,(uint8_t)(index+1u));invalid=1;
                }
            }
        }
        if (invalid) {
            pe_action_release();func_8001A680_command_cut(pe_action_aya(),PE_LoadU8(pe_action_record()+18u));
            pe_action_flags(pe_action_record()+76u,0u,0x200000u);return;
        }
        func_80070064();
        if (ability==6) {
            pe_addr_t weapon=PE_LoadU32(pe_action_record()+104u);int32_t pitch,vertical;
            if (!(PE_LoadU32(pe_action_ram(weapon+12u))&1023u)) func_800518A8(weapon);
            pe_action_flags(pe_action_record()+76u,0u,0x100000u);
            target=PE_LoadU32(slot);vertical=(int16_t)PE_LoadU16(pe_action_ram(target+618u))-(int16_t)PE_LoadU16(0x8009D27Cu);
            pitch=func_80079FB4(vertical,func_80030534(target,pe_action_aya()));
            i=pitch < -171?0u:pitch<228?1u:2u;
            func_8001A680_command_cut(pe_action_aya(),PE_LoadU8(pe_action_record()+20u+i));
            pe_action_flags(pe_action_record()+76u,0x200000u,0u);
            PE_StoreU8(0x8009CE39u,88u);PE_StoreU8(0x8009CE38u,0u);PE_StoreU8(0x8009CE3Au,24u);PE_StoreU8(0x8009CE3Bu,0u);
        } else func_8001A680_command_cut(pe_action_aya(),15u);
        if (ability!=19) {
            int32_t effect=func_8006F39C(PE_LoadU8(pe_action_ram(0x8001074Cu+(uint32_t)ability)),PE_LoadU32(slot));
            PE_StoreU32(0x8009D258u,(uint32_t)effect);
            if (PE_Port_ShouldStop()) return;
        }
        target=((uint32_t)ability-7u<2u || ability==10)?PE_LoadU32(slot)+616u:pe_action_aya()+42u;
        func_8006DDCC(PE_LoadU16(pe_action_ram(0x80010760u+(uint32_t)ability*2u)),0,
            (int16_t)PE_LoadU16(pe_action_ram(target)),(int16_t)PE_LoadU16(pe_action_ram(target+4u)),
            (int16_t)PE_LoadU16(pe_action_ram(target+8u)));
        D_8009D1A0|=256u;pe_action_flags(pe_action_record()+76u,0x200000u,0u);
    }
    record=pe_action_record();flags=PE_LoadU32(record+76u);
    if (flags&0x80000u) {
        pe_addr_t actor;
        if (!(uint8_t)func_80024A3C()) return;
        index=PE_LoadU8(0x8009D1D4u);PE_StoreU32(pe_action_record()+8u,0u);pe_action_flags(pe_action_aya()+152u,256u,0u);
        for (i=(uint8_t)(index-7u);(uint8_t)i<PE_LoadU8(0x8009D1D4u);i++) {
            actor=PE_LoadU32(pe_action_slot((uint8_t)i));if (actor!=pe_action_aya()) func_80028C48(actor);
        }
        for (actor=PE_LoadU32(0x8009D20Cu);actor;actor=PE_LoadU32(actor+4u)) {
            pe_addr_t body=PE_LoadU32(actor),command;
            if (actor==pe_action_aya() || !body || !(command=PE_LoadU32(body+24u))) continue;
            PE_StoreU8(command,4u);pe_action_flags(PE_LoadU32(actor),0x3F000000u,0u);
        }
        pe_action_flags(pe_action_record()+76u,0x80000u,0u);func_80020D50();return;
    }
    if (flags&0x100000u) {
        PE_StoreU16(pe_action_aya()+58u,(uint16_t)func_80030584(PE_LoadU32(slot)+436u,pe_action_aya()+40u));
        func_8002312C(slot);if (pe_action_effect_active()) return;
        PE_StoreU32(pe_action_record()+8u,0u);pe_action_flags(pe_action_aya()+152u,256u,0u);D_8009D1A0&=~256u;
        pe_action_flags(pe_action_record()+76u,0x100000u,0x200000u);func_80020D50();return;
    }
    aya=pe_action_aya();
    if (PE_LoadU8(aya+15u)==PE_LoadU16(aya+26u)) func_8001A680_command_cut(aya,PE_LoadU8(record+18u));
    if (pe_action_effect_active() && (int16_t)PE_LoadU16(slot+4u)!=406) return;
    pe_action_flags(pe_action_record()+76u,0u,0x200000u);
    func_80024250((int16_t)PE_LoadU16(slot+4u)-387,PE_LoadU32(slot));
    if ((int16_t)PE_LoadU16(slot+4u)==406) return;
    PE_StoreU8(0x8009D1D4u,(uint8_t)(PE_LoadU8(0x8009D1D4u)+1u));pe_action_release();
    for (i=PE_LoadU8(0x8009D1D4u);(uint8_t)i<PE_LoadU8(0x8009CE3Cu);i++) {
        unsigned next=PE_LoadU16(pe_action_slot((uint8_t)i)+4u);
        if (next-1u<2u) {func_80021128();return;}
    }
}
