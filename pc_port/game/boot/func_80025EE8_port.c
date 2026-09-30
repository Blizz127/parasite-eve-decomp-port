/* Original target selection, attack/item command input and undo.
 * Authority: 120D8.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

static pe_addr_t attack_weapon(void) { return PE_LoadU32(PE_LoadU32(0x8009D278u)+0x68u); }
static void attack_sound(uint32_t id)
{
    pe_addr_t package=PE_LoadU32(0x800B0E08u);
    if (package) (void)func_8006DF50(package,id,0u,0x80u,0x7Fu);
}
static void command_boundary(const char *name)
{
    Bootstrap_ReturnVoid(name,"func_80025EE8_attack_cut");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

void func_80021128(void)
{
    pe_addr_t weapon=attack_weapon();
    int kind=(int16_t)PE_LoadU16(weapon+6u);
    if (kind==6) {
        if (!(PE_LoadU32(weapon+0xCu)&0x3FFu)) (void)func_800518A8(weapon);
        PE_StoreU32(0x8009D200u,(uint32_t)func_8006F39C(3u,PE_LoadU32(0x8009D254u)));
    } else if (kind==8) {
        PE_StoreU32(0x8009D2FCu,(uint32_t)func_8006F39C(6u,PE_LoadU32(0x8009D254u)));
    } else {
        uint32_t flags,first,second;
        if (!(PE_LoadU32(weapon+0xCu)&0x3FFu)) (void)func_800518A8(weapon);
        weapon=attack_weapon(); kind=(int16_t)PE_LoadU16(weapon+6u);
        flags=PE_LoadU32(weapon+0x10u);
        if (kind==5 || (flags&0x1F00u)) { first=0u; second=5u; }
        else { first=kind==2 || (flags&0xC0u)==0x80u?1u:2u; second=4u; }
        PE_StoreU32(0x8009D200u,(uint32_t)func_8006F39C(first,PE_LoadU32(0x8009D254u)));
        PE_StoreU32(0x8009D2FCu,(uint32_t)func_8006F39C(second,PE_LoadU32(0x8009D254u)));
    }
}

static int8_t commit_commands(unsigned entry)
{
    unsigned first=PE_LoadU16(0x800BE834u),i;
    pe_addr_t record;
    if (!entry) PE_StoreU8(0x8009D1DCu,0u);
    if (first-3u<0x194u && PE_LoadU8(PE_LoadU32(0x8009D254u)+0xEu)>=4u)
        D_8009D1A0|=0x100u;
    if (first-0x183u>=0x15u) {
        for (i=0;i<PE_LoadU8(0x8009CE3Cu);i++) {
            if ((unsigned)PE_LoadU16(0x800BE834u+i*8u)-1u<2u) {
                func_80021128(); break;
            }
        }
    }
    record=PE_LoadU32(0x8009D278u);
    PE_StoreU8(0x8009D288u,0u); PE_StoreU16(record+0x10u,0u);
    PE_StoreU32(record+0x4Cu,PE_LoadU32(record+0x4Cu)|0x200000u);
    func_80051084(); /* 51244 is the identical D014 = A1AA0 setter. */
    if (entry!=2u) {
        func_800275CC(0x8009E000u,(int8_t)PE_LoadU8(0x8009CE44u));
        attack_sound(0x44Cu);
    }
    return 0;
}

void func_800258CC(int8_t mode)
{
    int count=(int8_t)PE_LoadU8(0x8009D2B0u);
    if (count>=2 && mode!=1 && mode!=2) {
        unsigned i;
        for (i=0;i<2;i++) if (PE_LoadU32(0x8009D1F4u)&(i?0x40u:0x10u)) {
            int selected=(int8_t)PE_LoadU8(0x8009CE44u);
            func_800275CC(0x8009E000u,(int8_t)selected);
            count=(int8_t)PE_LoadU8(0x8009D2B0u);
            selected=(selected+(i?count-1:1))%count;
            PE_StoreU8(0x8009CE44u,(uint8_t)selected);
            func_80026FD0(); attack_sound(0x44Eu);
        }
    } else if (!count) mode=8;
    if ((uint8_t)mode<8u) {
        pe_addr_t target,record=PE_LoadU32(0x8009D278u);
        int out=0;
        if (mode<4) {
            pe_addr_t selected;
            int32_t distance,range;
            func_800347B4();
            selected=0x8009E000u+(uint32_t)(int32_t)(int8_t)PE_LoadU8(0x8009CE44u)*12u;
            distance=(int32_t)PE_LoadU32(selected+4u);
            range=(PE_LoadU32(record+0x4Cu)&0x30u)==0x10u?200:
                (int16_t)PE_LoadU16(PE_LoadU32(record+0x68u)+2u);
            out=distance>range;
        }
        target=PE_LoadU32(0x8009E000u+(uint32_t)(int32_t)(int8_t)PE_LoadU8(0x8009CE44u)*12u);
        func_800314E4(target+0x1B4u,mode,(int8_t)out,PE_LoadU8(PE_LoadU32(target)+7u));
    } else if (mode==8) func_800314E4(0u,8,0,0);
    func_80026FF8(0x8009E000u,(int8_t)PE_LoadU8(0x8009CE44u),mode);
}

static void queue_shot(pe_addr_t actor, uint16_t command)
{
    unsigned count=PE_LoadU8(0x8009CE3Cu);
    pe_addr_t slot=0x800BE830u+count*8u;
    PE_StoreU32(slot,actor); PE_StoreU16(slot+4u,command);
    PE_StoreU16(slot+6u,(uint16_t)(int16_t)(int8_t)PE_LoadU8(0x8009D2D8u));
    PE_StoreU8(0x8009CE3Cu,(uint8_t)(count+1u));
}

void func_80026600(pe_addr_t target)
{
    uint32_t flags=PE_LoadU32(attack_weapon()+16u),shape=flags&0xC0u;
    unsigned i;
    if (shape==0xC0u) {
        for (i=0;PE_LoadU32(0x8009E000u+i*12u);i=(i+1u)&255u)
            queue_shot(PE_LoadU32(0x8009E000u+i*12u),2u);
        PE_StoreU8(0x8009D1DCu,0u);
    } else if (shape==0x40u) {
        for (i=0;i<((PE_LoadU32(attack_weapon()+16u)&15u)*3u)/2u;i++) {
            int count=(int8_t)PE_LoadU8(0x8009D2B0u);
            if (!count) { command_boundary("func_80026600_target_divide"); return; }
            queue_shot(PE_LoadU32(0x8009E000u+(func_80071A54()%(uint32_t)count)*12u),2u);
        }
        PE_StoreU8(0x8009D1DCu,0u);
    } else {
        queue_shot(PE_LoadU32(target),1u);
        PE_StoreU8(0x8009D1DCu,(uint8_t)(PE_LoadU8(0x8009D1DCu)-1u));
    }
}

void func_80026CF0_attack_cut(void)
{
    unsigned count=PE_LoadU8(0x8009CE3Cu), saved=count, i;
    int command=(int16_t)PE_LoadU16(0x800BE834u+(count-1u)*8u);
    uint32_t flags=PE_LoadU32(attack_weapon()+16u);
    PE_StoreU8(0x8009CE44u,0u);
    if (command==1) {
        int whole=PE_LoadU8(0x8009D1DCu)==(flags&15u);
        if (whole) PE_StoreU8(0x8009D2D8u,(uint8_t)(PE_LoadU8(0x8009D2D8u)+1u));
        do { count=(count-1u)&255u; }
        while (count && PE_LoadU16(0x800BE836u+count*8u)==PE_LoadU16(0x800BE836u+(count-1u)*8u));
        if (whole && PE_LoadU8(0x8009CE60u)) {
            PE_StoreU8(0x8009CE60u,0u); count=saved;
        }
        PE_StoreU8(0x8009CE3Cu,(uint8_t)count);
    } else if (command==2) {
        if ((flags&0xC0u)==0xC0u || (flags&0xC0u)==0x40u) {
            PE_StoreU8(0x8009D2D8u,(uint8_t)(PE_LoadU8(0x8009D2D8u)+1u));
            count-=(flags&0xC0u)==0xC0u?PE_LoadU8(0x8009D2B0u):(flags&15u)*3u/2u;
            PE_StoreU8(0x8009CE3Cu,(uint8_t)count);
        }
    } else if (command<407) {
        PE_StoreU8(0x8009D2D8u,(uint8_t)(PE_LoadU8(0x8009D2D8u)+1u));
        if (!PE_LoadU8(0x8009CE60u)) func_8005112C();
        count=PE_LoadU8(0x8009CE3Cu);saved=count;
        do {count=(count-1u)&255u;PE_StoreU8(0x8009CE3Cu,(uint8_t)count);}
        while (count && PE_LoadU16(0x800BE836u+count*8u)==PE_LoadU16(0x800BE836u+(count-1u)*8u));
        if (PE_LoadU8(0x8009CE60u)) {PE_StoreU8(0x8009CE60u,0u);PE_StoreU8(0x8009CE3Cu,(uint8_t)saved);}
    }
    if (command<407) PE_StoreU8(0x8009D1DCu,(uint8_t)(PE_LoadU32(attack_weapon()+16u)&15u));
    for (i=0;i<45;i++) {
        pe_addr_t slot=0x800BE830u+i*8u;
        if ((int16_t)PE_LoadU16(slot+6u)==(int8_t)PE_LoadU8(0x8009D2D8u)) {
            PE_StoreU32(slot,0u); PE_StoreU16(slot+4u,0u); PE_StoreU16(slot+6u,0u);
        }
    }
}

int8_t func_80025EE8_attack_cut(void)
{
    pe_addr_t weapon;int advanced=0,opened=0;
    if (!PE_LoadU8(0x8009D2D8u)) return commit_commands(0u);
    if ((PE_LoadU32(0x8009D1F4u)&0x2000u) && !func_80062A34(1u,0u) &&
        (int8_t)PE_LoadU8(0x8009D1F0u)!=2) {
        int8_t mode=(int8_t)PE_LoadU8(0x8009D1F0u);
        advanced=1;
        PE_StoreU8(0x8009CE60u,PE_LoadU8(0x8009D1DCu)==(PE_LoadU32(attack_weapon()+16u)&15u));
        if (mode==1) func_80062F9C();else if (mode==2) func_8005112C();
        PE_StoreU8(0x8009D2D8u,(uint8_t)(PE_LoadU8(0x8009D2D8u)-1u));
        if (!PE_LoadU8(0x8009D2D8u)) return commit_commands(1u);
        PE_StoreU8(0x8009D1DCu,(uint8_t)(PE_LoadU32(attack_weapon()+16u)&15u));attack_sound(0x44Cu);
    }
    if ((int8_t)PE_LoadU8(0x8009D1F0u)) {
        int8_t mode=func_80026824((int8_t)PE_LoadU8(0x8009D1F0u));
        PE_StoreU8(0x8009D1F0u,(uint8_t)mode);
        if (mode==-1) {PE_StoreU8(0x8009D1F0u,0u);return commit_commands(2u);}
        if (!mode) func_80026FD0();
        return 1;
    }
    weapon=attack_weapon();
    if (PE_LoadU8(0x8009D2D8u)) {
        uint32_t shape=(PE_LoadU32(weapon+16u)>>6u)&3u;
        int8_t mode=(int16_t)PE_LoadU16(weapon+6u)==8?3:shape==1u?1:shape==3u?2:0;
        func_800258CC(mode);
    }
    if ((PE_LoadU32(0x8009D1F4u)&0x200u) && !advanced && PE_LoadU8(0x8009D2D8u)) {
        PE_StoreU8(0x8009CE60u,0u);
        if (PE_LoadU8(0x8009D2B0u)) {
            func_80026600(0x8009E000u+(uint32_t)(int32_t)(int8_t)PE_LoadU8(0x8009CE44u)*12u);
            if (!PE_LoadU8(0x8009D1DCu)) {
                PE_StoreU8(0x8009D2D8u,(uint8_t)(PE_LoadU8(0x8009D2D8u)-1u));
                PE_StoreU8(0x8009D1DCu,(uint8_t)(PE_LoadU32(attack_weapon()+16u)&15u));
            }
        }
        attack_sound(0x44Cu);
    }
    if (PE_LoadU8(0x8009D1DCu)==(PE_LoadU32(attack_weapon()+16u)&15u) &&
        (PE_LoadU32(0x8009D1F4u)&0x80u) && PE_LoadU8(0x8009D2D8u) && !advanced) {
        PE_StoreU8(0x8009D1F0u,1u);func_800275CC(0x8009E000u,(int8_t)PE_LoadU8(0x8009CE44u));
        attack_sound(0x44Cu);func_8005C174(1);func_80067CBC();opened=1;
    }
    if ((PE_LoadU32(0x8009D1F4u)&0x400u) && !opened && !advanced) {
        int8_t remain=1;
        func_800275CC(0x8009E000u,(int8_t)PE_LoadU8(0x8009CE44u));
        if (PE_LoadU8(0x8009CE3Cu)) { func_80026CF0_attack_cut(); func_80026FD0(); }
        else remain=0;
        attack_sound(0x44Du); return remain;
    }
    return 1;
}
