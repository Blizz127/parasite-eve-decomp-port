/* Retail shot animation controller, 21F38/22D7C/23008/2312C in 120D8.s.
 * Animation frames, aim and ammunition advance through the original state
 * transitions. Effects are signaled through the native 6F6D4 dispatcher. */
#include "psx_compat.h"
#include "pe_port_compat.h"

static pe_addr_t shot_aya(void) { return PE_LoadU32(0x8009D254u); }
static pe_addr_t shot_record(void) { return PE_LoadU32(0x8009D278u); }
static pe_addr_t shot_weapon(void) { return PE_LoadU32(shot_record()+0x68u); }
static pe_addr_t shot_queue(unsigned i) { return 0x800BE830u+i*8u; }
static void shot_flags(pe_addr_t base,uint32_t offset,uint32_t clear,uint32_t set)
{ PE_StoreU32(base+offset,(PE_LoadU32(base+offset)&~clear)|set); }

/* func_80030584: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80030584_port.c (src/func_80030584.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static unsigned shot_elevation(pe_addr_t target)
{
    int32_t height=(int16_t)PE_LoadU16(target+0x26Au)-(int16_t)PE_LoadU16(0x8009D27Cu);
    int32_t angle=func_80079FB4(height,func_80030534(target,shot_aya()));
    return angle < -171 ? 0u : angle < 228 ? 1u : 2u;
}
static void shot_aim(pe_addr_t target,unsigned offset)
{
    unsigned elevation=shot_elevation(target);
    func_8001A680_command_cut(shot_aya(),PE_LoadU8(shot_record()+offset+elevation));
}
static void shot_sound(int32_t id,pe_addr_t position)
{
    func_8006DE80(id,0,(int16_t)PE_LoadU16(position),
        (int16_t)PE_LoadU16(position+4u),(int16_t)PE_LoadU16(position+8u));
}
static void shot_weapon_sound(unsigned index)
{
    pe_addr_t aya=shot_aya();
    (void)func_8006DD38(index,0u,(int16_t)PE_LoadU16(aya+0x2Au),
        (int16_t)PE_LoadU16(aya+0x2Eu),(int16_t)PE_LoadU16(aya+0x32u));
}
static void shot_end_effects(int check_secondary)
{
    if ((int16_t)PE_LoadU16(shot_weapon()+6u)!=8)
        (void)func_8006F6D4(PE_LoadU32(0x8009D200u),0u,0u,2u,0u,0u);
    if (!check_secondary || PE_LoadU32(0x8009D2FCu)!=0xFFFFFFFFu)
        (void)func_8006F6D4(PE_LoadU32(0x8009D2FCu),0u,0u,2u,0u,0u);
}

void func_80022D7C(pe_addr_t target)
{
    uint32_t flags=PE_LoadU32(shot_weapon()+16u),resist=PE_LoadU32(PE_LoadU32(target)+0xCCu);
    unsigned effect=0u,mode=0u,sound=0u;
    int needs_argument=0;
    if ((flags&0x400u) && (resist&3u)!=1u) { effect=7;mode=2;sound=0x484;needs_argument=1; }
    else if ((flags&0x300u)==0x300u && (resist&0x3C000u)!=0x14000u) { effect=0x5A;sound=0x488; }
    else if ((flags&0x100u) && (resist&0xC000u)!=0x4000u) { effect=0x58;sound=0x488; }
    else if ((flags&0x200u) && (resist&0x30000u)!=0x10000u) { effect=0x59;sound=0x486; }
    else if ((flags&0x800u) && (resist&12u)!=4u) { effect=7;mode=1;sound=0x482;needs_argument=1; }
    else if ((flags&0x1000u) && (resist&0x30u)!=0x10u) { effect=7;mode=0;sound=0x480;needs_argument=1; }
    if (effect) {
        uint32_t index=(uint32_t)func_8006F39C(effect,target);
        PE_StoreU32(0x8009D208u,index);
        if (needs_argument) (void)func_8006F6D4(index,0u,0u,mode,0u,0u);
        /* Hit anchors are packed shorts, unlike the 16.16 actor pose. */
        func_8006DE80((int)sound,0,(int16_t)PE_LoadU16(target+0x268u),
            (int16_t)PE_LoadU16(target+0x26Au),(int16_t)PE_LoadU16(target+0x26Cu));
    }
}

void func_80023008(void)
{
    pe_addr_t weapon=shot_weapon();
    if ((int16_t)PE_LoadU16(weapon+6u)==8) {
        PE_StoreU8(0x8009D294u,1u); shot_sound(0x46C,shot_aya()+0x2Au);
    } else if (PE_LoadU32(weapon+12u)&0x3FFu) {
        PE_StoreU8(0x8009D294u,1u);
        if (!(PE_LoadU32(shot_record()+0x4Cu)&0x100000u)) {
            (void)func_8006F6D4(PE_LoadU32(0x8009D200u),0u,0u,1u,0u,0u);
            shot_weapon_sound(0);
        }
        weapon=shot_weapon();
        PE_StoreU32(weapon+12u,(PE_LoadU32(weapon+12u)&~0x3FFu)|
            ((PE_LoadU32(weapon+12u)-1u)&0x3FFu));
    } else {
        PE_StoreU8(0x8009D294u,0u); shot_sound(0x46E,shot_aya()+0x2Au);
    }
}

int func_8002312C(pe_addr_t slot)
{
    pe_addr_t aya=shot_aya(),record=shot_record(),weapon=shot_weapon();
    unsigned command=PE_LoadU8(aya+0xEu),index=PE_LoadU8(0x8009D1D4u);
    if (command>=6u && command<=11u) {
        if (!(command&1u)) {
            if (PE_LoadU8(aya+0xFu)!=PE_LoadU16(aya+0x16u)) return 0;
            if (PE_LoadU8(0x8009CE38u) || PE_LoadU8(0x8009CE39u)) {
                pe_addr_t timer=PE_LoadU8(0x8009CE38u)?0x8009CE38u:0x8009CE39u;
                PE_StoreU8(timer,(uint8_t)(PE_LoadU8(timer)-1u));
                shot_flags(aya,0x98u,0u,0x100u); return 0;
            }
            PE_StoreU8(0x8009D274u,0u); PE_StoreU8(0x8009CE39u,PE_LoadU8(weapon+0x15u));
            shot_aim(PE_LoadU32(slot),0x17u);
            shot_flags(shot_aya(),0x98u,0x100u,0u);
            PE_StoreU8(0x8009D1DCu,(uint8_t)(PE_LoadU32(shot_weapon()+16u)&15u));
            func_80023008(); return 0;
        }
        if (PE_LoadU8(aya+0xFu)!=PE_LoadU16(aya+0x1Au)) return 0;
        if (PE_LoadU16(shot_queue(index)+4u)==0x189u) {
            func_8001A680_command_cut(aya,PE_LoadU8(record+0x12u)); return 1;
        }
        if ((PE_LoadU32(weapon+16u)&0xC0u)!=0xC0u || PE_LoadU8(0x8009D1DCu)==1u) {
            pe_addr_t next=shot_queue(index+1u),body;
            PE_StoreU8(0x8009D1DCu,0u);
            if ((int16_t)PE_LoadU16(next+4u)>=3 || index+1u==PE_LoadU8(0x8009CE3Cu)) {
                func_8001A680_command_cut(aya,PE_LoadU8(record+0x12u));
                shot_flags(shot_record(),0x4Cu,0u,0x200000u);
                PE_StoreU8(0x8009CE38u,PE_LoadU8(shot_weapon()+0x14u)); return 1;
            }
            body=PE_LoadU32(PE_LoadU32(next));
            if (!body || (int32_t)PE_LoadU32(body+0x10u)<=0) {
                func_8001A680_command_cut(aya,PE_LoadU8(record+0x12u));
                PE_StoreU8(0x8009D1D4u,(uint8_t)(PE_LoadU8(0x8009D1D4u)+1u));
                shot_flags(shot_record(),0x4Cu,0u,0x200000u); return 1;
            }
            if (PE_LoadU32(weapon+12u)&0x3FFu) {
                shot_aim(PE_LoadU32(slot),0x14u);
                aya=shot_aya(); PE_StoreU32(aya+0x14u,(uint32_t)PE_LoadU8(aya+0xFu)<<16u);
                shot_flags(aya,0x98u,0u,0x100u);
                PE_StoreU8(0x8009CE39u,(uint8_t)(PE_LoadU8(0x8009CE39u)+PE_LoadU8(0x8009CE3Au)));
                PE_StoreU8(0x8009D1DCu,(uint8_t)(PE_LoadU32(shot_weapon()+16u)&15u)); return 1;
            }
            if (func_800518A8(weapon)) {
                func_8001A680_command_cut(shot_aya(),12u); shot_weapon_sound(1);
            }
            return 1;
        }
        if (PE_LoadU32(weapon+12u)&0x3FFu) {
            pe_addr_t body=PE_LoadU32(PE_LoadU32(shot_queue(index)));
            if (!body || (int32_t)PE_LoadU32(body+0x10u)<=0) {
                func_8001A680_command_cut(aya,PE_LoadU8(record+0x12u));
                shot_flags(shot_record(),0x4Cu,0u,0x200000u);
                PE_StoreU8(0x8009D1DCu,(uint8_t)(PE_LoadU32(shot_weapon()+16u)&15u)); return 1;
            }
            PE_StoreU8(0x8009D1DCu,(uint8_t)(PE_LoadU8(0x8009D1DCu)-1u));
            func_80023008(); return 0;
        }
        if (func_800518A8(weapon)) {
            func_8001A680_command_cut(shot_aya(),12u); shot_weapon_sound(1); return 0;
        }
        func_8001A680_command_cut(shot_aya(),PE_LoadU8(shot_record()+0x12u)); return 1;
    }
    if (PE_LoadU16(shot_queue(index)+4u)==0x189u) return 0;
    PE_StoreU8(0x8009D1D4u,(uint8_t)(index+1u));
    if (((index+1u)&255u)==PE_LoadU8(0x8009CE3Cu) && (int32_t)PE_LoadU32(0x8009D200u)>=0)
        shot_end_effects(0);
    return 0;
}

void func_80021F38(void)
{
    pe_addr_t aya=shot_aya(),slot=shot_queue(PE_LoadU8(0x8009D1D4u)),target=PE_LoadU32(slot);
    if (PE_LoadU8(aya+0xEu)==12u) {
        if (PE_LoadU8(aya+0xFu)==PE_LoadU16(aya+0x1Au)) shot_aim(target,0x14u);
        return;
    }
    if (PE_LoadU32(shot_record()+0x4Cu)&0x200000u) {
        pe_addr_t body=PE_LoadU32(target);
        if (body && (int32_t)PE_LoadU32(body+0x10u)>0) {
            shot_aim(target,0x14u); shot_flags(shot_record(),0x4Cu,0x200000u,0u);
        }
    }
    PE_StoreU16(shot_aya()+0x3Au,(uint16_t)func_80030584(target+0x1B4u,shot_aya()+0x28u));
    if ((uint8_t)func_8002312C(slot)==1u) {
        unsigned index=PE_LoadU8(0x8009D1D4u);
        if (target!=PE_LoadU32(shot_queue(index+1u)) ||
            (int32_t)PE_LoadU32(PE_LoadU32(target)+0x10u)<=0) {
            PE_StoreU8(0x8009CE39u,(uint8_t)(PE_LoadU8(0x8009CE39u)+PE_LoadU8(0x8009CE3Bu)));
            if (PE_LoadU32(shot_weapon()+12u)&0x3FFu) func_80022D7C(target);
        }
        index=PE_LoadU8(0x8009D1D4u)+1u; PE_StoreU8(0x8009D1D4u,(uint8_t)index);
        if ((index&255u)==PE_LoadU8(0x8009CE3Cu)) shot_end_effects(1);
    }
}
