/* Retail hit judgement and shot-frame dispatch, 21278 / 236E8.
 * Authority: original 11718.s and 120D8.s. Hit chance uses the game's
 * distance, range and RNG; the damage consumer remains 28574. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

extern int func_80053D2C(int item);

static pe_addr_t hit_record(void) { return PE_LoadU32(0x8009D278u); }
static pe_addr_t hit_weapon(void) { return PE_LoadU32(hit_record()+0x68u); }

void func_80021278(pe_addr_t actor,pe_addr_t result,int32_t previous)
{
    pe_addr_t weapon;
    uint32_t flags;
    int32_t range,distance,roll,kind,critical;
    PE_StoreU8(result+1u,0u);
    if (PE_LoadU32(actor+0x98u)&0x4000u) { PE_StoreU8(result,255u); return; }
    flags=PE_LoadU32(hit_record()+0x4Cu);
    if (flags&0x100000u) { PE_StoreU8(result,1u); PE_StoreU8(result+1u,2u); return; }
    PE_StoreU8(result,0u);
    weapon=hit_weapon();
    range=(flags&0x30u)==0x10u?200:(int16_t)PE_LoadU16(weapon+2u);
    distance=(int32_t)((uint32_t)func_80030534(actor,PE_LoadU32(0x8009D254u))*1000u);
    if (!range) {
        /* The original executes BREAK 7 on division by zero. */
        Bootstrap_ReturnVoid("func_80021278_zero_range", "func_80021278");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY); return;
    }
    distance=(int32_t)((int64_t)distance/range);
    roll=(int16_t)((int32_t)func_80071A54()%100);
    weapon=hit_weapon(); kind=(int16_t)PE_LoadU16(weapon+6u);
    critical=(int)((PE_LoadU32(weapon+16u)>>17u)&1u)*20;
    critical+=distance>=801?5:distance>=501?20:50;
    if (kind==8) {
        if (distance>1000) return;
        if (roll<critical) PE_StoreU8(result+1u,3u);
        PE_StoreU8(result,1u); return;
    }
    if (kind==6 && previous) {
        PE_StoreU8(result,1u);
        PE_StoreU8(result+1u,(uint8_t)(previous==2?1:previous==3?2:4)); return;
    }
    if (distance>1800) return;
    if (distance>1000) {
        int threshold=distance>1500?25:distance>1200?50:80;
        if (roll<threshold) { PE_StoreU8(result,1u); PE_StoreU8(result+1u,1u); }
        return;
    }
    PE_StoreU8(result+1u,(uint8_t)(roll<critical?3:2)); PE_StoreU8(result,1u);
}

pe_addr_t func_8005DC9C(uint32_t index)
{
    pe_addr_t base=0x800A8028u+PE_LoadU32(0x800A802Cu);
    pe_addr_t table=base+PE_LoadU32(base+8u);
    if (index>=PE_LoadU16(table)) return 0u;
    return table+(uint32_t)(int32_t)(int16_t)PE_LoadU16(table+2u+index*2u);
}

static pe_addr_t hit_copy_string(pe_addr_t dest,pe_addr_t source)
{
    uint8_t byte;
    do { byte=PE_LoadU8(source++); PE_StoreU8(dest++,byte); } while (byte!=255u);
    return dest-1u;
}

pe_addr_t func_80054A88(int32_t item,int32_t kind)
{
    pe_addr_t out=0x800A1B50u;
    if (!item) (void)hit_copy_string(out,func_8005DC4C(19u));
    else if (kind>=2) (void)hit_copy_string(out,func_8005DC4C(18u));
    else if (!PE_LoadU32(0x8009D218u)) {
        out=hit_copy_string(out,func_8005DC9C((uint32_t)item-1u));
        (void)hit_copy_string(out,func_8005DC4C((uint32_t)kind+16u));
    } else {
        out=hit_copy_string(out,func_8005DC4C((uint32_t)kind+16u));
        (void)hit_copy_string(out,func_8005DC9C((uint32_t)item-1u));
    }
    return 0x800A1B50u;
}

static void hit_apply_result(pe_addr_t actor)
{
    pe_addr_t body=PE_LoadU32(actor);
    int result=(int8_t)PE_LoadU8(0x8009CE54u);
    if (result==1) {
        uint32_t flags=(PE_LoadU32(body)&~0x6000u)|0x2000u;
        PE_StoreU32(body,flags);
        flags=(flags&~0xC0000u)|((PE_LoadU32(hit_weapon()+12u)>>20u)&3u)<<18u;
        PE_StoreU32(body,flags);
        flags=(flags&~0x38000u)|((uint32_t)PE_LoadU8(0x8009CE55u)&7u)<<15u;
        PE_StoreU32(body,flags);
    } else if (!result) {
        PE_StoreU16(body+0xD0u,65535u); PE_StoreU8(body+0xD6u,30u);
        PE_StoreU16(body+0xD2u,PE_LoadU16(actor+0x218u));
        PE_StoreU16(body+0xD4u,(uint16_t)(PE_LoadU16(actor+0x21Au)-20u));
    }
}

static int hit_eligible(pe_addr_t actor)
{
    pe_addr_t body;
    uint32_t flags;
    if (actor==PE_LoadU32(0x8009D254u)) return 0;
    body=PE_LoadU32(actor);
    if (!body) return 0;
    flags=PE_LoadU32(actor+0x98u);
    return (flags&0x2040u)!=0x40u && !(flags&0x4000u) && (int32_t)PE_LoadU32(body+0x10u)>0;
}

void func_800236E8(void)
{
    pe_addr_t weapon=hit_weapon(),queue=0x800BE830u+(uint32_t)PE_LoadU8(0x8009D1D4u)*8u;
    pe_addr_t actor;
    int kind;
    uint8_t frame=(uint8_t)(PE_LoadU8(0x8009D274u)+1u);
    PE_StoreU8(0x8009D274u,frame);
    if (frame!=PE_LoadU32(weapon+8u)) return;
    kind=(int16_t)PE_LoadU16(weapon+6u);
    if (kind==8) {
        actor=PE_LoadU32(queue);
        func_80021278(actor,0x8009CE54u,0);
        if ((int8_t)PE_LoadU8(0x8009CE54u)==1) {
            uint32_t resistance;
            func_8006DE80(0x46D,0,(int16_t)PE_LoadU16(actor+0x268u),
                (int16_t)PE_LoadU16(actor+0x26Au),(int16_t)PE_LoadU16(actor+0x26Cu));
            if (PE_LoadU32(hit_weapon()+16u)&0x6000u) {
                pe_addr_t body=PE_LoadU32(PE_LoadU32(queue));
                resistance=(PE_LoadU32(body+0xCCu)>>12u)&3u;
                if (resistance==2u || (!resistance && !(func_80071A54()&1u))) {
                    int full=func_80053D2C(PE_LoadU8(body+0x9Fu));
                    body=PE_LoadU32(PE_LoadU32(queue));
                    PE_StoreU8(0x8009D1CEu,1u);
                    PE_StoreU32(0x8009D1F8u,func_80054A88(PE_LoadU8(body+0x9Fu),full?2:1));
                    if (!full) PE_StoreU8(PE_LoadU32(PE_LoadU32(queue))+0x9Fu,0u);
                }
            }
        }
        hit_apply_result(PE_LoadU32(queue));
    } else if (kind==6) {
        int previous;
        actor=PE_LoadU32(queue); func_80021278(actor,0x8009CE54u,0); hit_apply_result(actor);
        previous=(int8_t)PE_LoadU8(0x8009CE55u);
        if ((int8_t)PE_LoadU8(0x8009CE54u)==1) {
            for (actor=PE_LoadU32(0x8009D20Cu);actor;actor=PE_LoadU32(actor+4u)) {
                if (!hit_eligible(actor) || actor==PE_LoadU32(queue)) continue;
                func_80021278(actor,0x8009CE54u,previous); hit_apply_result(actor);
            }
        }
    } else if ((PE_LoadU32(weapon+16u)&0xC0u)==0x80u && !(PE_LoadU32(hit_record()+0x4Cu)&0x100000u)) {
        uint32_t heading=PE_LoadU16(PE_LoadU32(0x8009D254u)+0x3Au);
        int center=(int16_t)(heading-2048u),lower,upper;
        if (center < -1536) { lower=(int16_t)(heading-1536u); upper=(int16_t)(heading+1536u); }
        else if (center < 1536) { lower=(int16_t)(heading-2560u); upper=(int16_t)(heading-1536u); }
        else { lower=(int16_t)(heading-5632u); upper=(int16_t)(heading-2560u); }
        for (actor=PE_LoadU32(0x8009D20Cu);actor;actor=PE_LoadU32(actor+4u)) {
            pe_addr_t aya; int angle;
            if (!hit_eligible(actor)) continue;
            aya=PE_LoadU32(0x8009D254u);
            angle=(int16_t)func_80079FB4((int16_t)PE_LoadU16(actor+0x268u)-(int16_t)PE_LoadU16(aya+0x2Au),
                                        (int16_t)PE_LoadU16(actor+0x26Cu)-(int16_t)PE_LoadU16(aya+0x32u));
            if (center>=-1536 && center<1536) { if (angle<lower || angle>upper) continue; }
            else if (angle<upper && angle>lower) continue;
            func_80021278(actor,0x8009CE54u,0); hit_apply_result(actor);
        }
    } else {
        actor=PE_LoadU32(queue);
        if (actor) { func_80021278(actor,0x8009CE54u,0); hit_apply_result(PE_LoadU32(queue)); }
    }
    PE_StoreU8(0x8009D294u,0u);
    PE_StoreU8(0x8009D274u,0u);
}
