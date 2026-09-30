/* Damage and hit reaction, retail 28574..28E94 (120D8.s).
 * Integer arithmetic wraps at the same operations as the original CPU.
 * PE power uses the original sequence of IEEE double operations. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

static int32_t damage_mul(int32_t a,int32_t b)
{ return (int32_t)((uint32_t)a*(uint32_t)b); }

static int damage_div(int32_t n,int32_t d,int32_t *out)
{
    if (!d || (n==INT32_MIN && d==-1)) {
        Bootstrap_ReturnVoid("func_80028574_original_division_trap","func_80028574");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY); return 0;
    }
    *out=n/d; return 1;
}

void func_80028C48(pe_addr_t actor)
{
    pe_addr_t body=PE_LoadU32(actor),rec=PE_LoadU32(0x8009D278u);
    uint32_t flags=PE_LoadU32(body),pf=PE_LoadU32(rec+0x4Cu);
    int16_t angle;
    if ((int8_t)PE_LoadU8(body+5u) || (flags&14u)) return;
    if (!((pf&0x80000u) && PE_LoadU8(body+0x1Cu+((flags>>17u)&0x70u)))) {
        if (PE_LoadU8(body+0xBCu)==1u) {
            PE_StoreU8(body+0xBCu,2u); PE_StoreU8(body+0xBDu,PE_LoadU8(actor+14u));
            PE_StoreU8(body+0xBEu,(uint8_t)((PE_LoadU32(actor+0x98u)>>9u)&1u));
            PE_StoreU32(body+0xC0u,PE_LoadU32(actor+0x14u));
            PE_StoreU32(body+0xC4u,PE_LoadU32(actor+0x18u));
            PE_StoreU32(body+0xC8u,PE_LoadU32(actor+0x1Cu));
        }
    }
    angle=(int16_t)func_800305C8(PE_LoadU32(0x8009D254u),actor);
    func_8001A680_command_cut(actor,angle>1024 && angle<3072?1u:0u);
    PE_StoreU32(actor+0x1Cu,0x10000u);
    PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x1000u);
    rec=PE_LoadU32(0x8009D278u);
    if (!(PE_LoadU32(rec+0x4Cu)&0x80000u) && (PE_LoadU32(body)&0x100000u)) {
        unsigned kind=PE_LoadU8(body+0xA4u); uint16_t distance;
        if (!kind) return;
        if (kind==1u) PE_StoreU16(body+0xA6u,400u);
        else if (kind==2u) { PE_StoreU8(body+0xA5u,2u); PE_StoreU16(body+0xA6u,70u); }
        else if (kind==3u) { PE_StoreU8(body+0xA5u,5u); PE_StoreU16(body+0xA6u,20u); }
        angle=(int16_t)(PE_LoadU16(PE_LoadU32(0x8009D254u)+0x3Au)+2048u);
        PE_StoreU16(body+0xA8u,(uint16_t)angle); distance=PE_LoadU16(body+0xA6u);
        PE_StoreU32(actor+0x28u,PE_LoadU32(actor+0x28u)+
            ((uint32_t)damage_mul(distance,func_80077CF4(angle))<<4u));
        PE_StoreU32(actor+0x30u,PE_LoadU32(actor+0x30u)+
            ((uint32_t)damage_mul(distance,func_80077DC4(angle))<<4u));
    }
}

static int damage_status_applies(uint32_t resistance)
{ return resistance==2u || (!resistance && !(func_80071A54()&1u)); }

void func_80028574(pe_addr_t actor)
{
    pe_addr_t body=PE_LoadU32(actor),rec=PE_LoadU32(0x8009D278u);
    pe_addr_t weapon=PE_LoadU32(rec+0x68u);
    uint32_t pf=PE_LoadU32(rec+0x4Cu),wf=PE_LoadU32(weapon+16u);
    int32_t power,defense=PE_LoadU16(body+0x8Cu),damage;
    if (pf&0x80000u) {
        int32_t base=damage_mul((int32_t)PE_LoadU16(rec+0x1Eu)+(int16_t)PE_LoadU16(rec+4u)-25,6);
        volatile double factor=(double)((int8_t)PE_LoadU8(0x8009D2B0u)-1)*0.1;
        volatile double multiplier=factor+1.0;
        volatile double numerator=(double)base*multiplier;
        power=(int32_t)(numerator/7.0);
    } else if (pf&0x100000u) {
        int32_t base=(int32_t)(PE_LoadU16(rec+0x1Eu)/5u)+(int16_t)PE_LoadU16(weapon);
        int32_t scale=((int16_t)PE_LoadU16(rec+4u)*7+120)/160;
        int32_t extra=damage_mul(damage_mul(base,scale),(int16_t)PE_LoadU16(rec+0xAu));
        if (!damage_div(extra,(int16_t)PE_LoadU16(rec+0x2Au),&extra)) return;
        power=(int32_t)((uint32_t)base+(uint32_t)extra);
    } else {
        static const uint8_t scales[]={0,100,60,41,0,25,0,18,0,0,13};
        unsigned shots=wf&15u;
        if (shots>=sizeof(scales)) {
            Bootstrap_ReturnVoid("func_80028574_invalid_shot_count","func_80028574");
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY); return;
        }
        power=damage_mul((int32_t)(PE_LoadU16(rec+0x1Eu)/5u)+(int16_t)PE_LoadU16(weapon),scales[shots])/100;
    }
    if (!(pf&0x180000u) && (int16_t)PE_LoadU16(weapon+6u)!=8)
        if (!damage_div(defense,(int32_t)(wf&15u),&defense)) return;
    damage=(int32_t)((uint32_t)power-(uint32_t)defense);
    if (damage>0 && !(pf&0x180000u)) {
        uint32_t hit=PE_LoadU32(body)&0x38000u;
        unsigned element=0u,impact;
        if (hit==0x8000u) damage=damage_mul(damage,3)/10;
        else if (hit==0x18000u) damage=damage_mul(damage,3)/2;
        else if (hit==0x20000u) damage/=10;
        impact=(PE_LoadU32(body)>>18u)&3u;
        if (impact==3u) PE_StoreU32(body+0xCCu,PE_LoadU32(body+0xCCu)|0x1000000u);
        if (impact<2u) {
            uint32_t resist=PE_LoadU32(body+0xCCu);
            if ((wf&0x400u) && damage_status_applies(resist&3u))
                PE_StoreU32(body,PE_LoadU32(body)|0x400u);
            if (!(PE_LoadU32(body)&0x400u)) {
                if (wf&0x100u) { unsigned v=(resist>>14u)&3u; if (v==1u || v==2u) element=v; }
                if (element!=2u && (wf&0x200u)) { unsigned v=(resist>>16u)&3u; if (v==1u || v==2u) element=v; }
                if ((wf&0x800u) && damage_status_applies((resist>>2u)&3u))
                    PE_StoreU32(body,(PE_LoadU32(body)|0x10u)&~0x3E0u);
                if ((wf&0x1000u) && damage_status_applies((resist>>4u)&3u)) {
                    PE_StoreU32(body,PE_LoadU32(body)|0x1800u);
                    PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x1000u);
                    if ((int8_t)PE_LoadU8(body+5u))
                        func_8001A680_command_cut(actor,(uint16_t)(int16_t)(int8_t)PE_LoadU8(body+6u));
                }
            }
        }
        if (element==1u) damage/=5;
        else if (element==2u) damage=damage_mul(damage,3)/2;
    }
    if (damage<=0) {
        uint32_t flags=PE_LoadU32(body);
        damage=(flags&0x38000u)==0x18000u?2:1;
        if ((flags&0xC0000u)==0xC0000u)
            PE_StoreU32(body+0xCCu,PE_LoadU32(body+0xCCu)|0x1000000u);
    }
    PE_StoreU32(body+0x10u,PE_LoadU32(body+0x10u)-(uint32_t)damage);
    if (!(PE_LoadU32(PE_LoadU32(0x8009D278u)+0x4Cu)&0x80000u)) func_80028C48(actor);
    PE_StoreU32(body,(PE_LoadU32(body)&~0x6000u)|0x4000u);
}
