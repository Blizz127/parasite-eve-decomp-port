/* Original player status recovery, poison and enemy attack attributes.
 * AB74.s: 1F9C4..209EC. Timers retain signed byte/halfword wrap. */
#include "psx_compat.h"
#include "pe_port_compat.h"

static pe_addr_t player_record(void) { return PE_LoadU32(0x8009D278u); }
static void status_flags(uint32_t clear,uint32_t set)
{
    pe_addr_t p=player_record()+76u;
    PE_StoreU32(p,(PE_LoadU32(p)&~clear)|set);
}

/* func_800201DC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800201DC_port.c (src/func_800201DC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static void recovery_effect(unsigned code,unsigned sound)
{
    pe_addr_t actor=PE_LoadU32(0x8009D254u);
    (void)func_8006F39C(code,actor);
    func_8006DE80((int)sound,0,(int16_t)PE_LoadU16(actor+42u),
        (int16_t)PE_LoadU16(actor+46u),(int16_t)PE_LoadU16(actor+50u));
}

void func_8001F9C4(void)
{
    pe_addr_t p=player_record();
    uint32_t flags=PE_LoadU32(p+76u),armor;
    unsigned i;
    if ((flags&3u)==1u) {
        int16_t timer;
        func_800201DC();timer=(int16_t)(PE_LoadU16(p+64u)-PE_LoadU16(p+62u));
        PE_StoreU16(p+64u,(uint16_t)timer);
        if (timer<=0) status_flags(3u,0u);
    }
    if ((PE_LoadU32(p+76u)&12u)==4u) {
        int16_t timer=(int16_t)(PE_LoadU16(p+66u)-PE_LoadU16(p+60u));
        PE_StoreU16(p+66u,(uint16_t)timer);
        if (timer<=0) {status_flags(12u,0u);PE_StoreU32(0x8009D2E8u,PE_LoadU32(0x8009D2E8u)&~16u);}
    }
    if ((PE_LoadU32(p+76u)&48u)==16u) {
        int16_t timer=(int16_t)(PE_LoadU16(p+68u)-PE_LoadU16(p+60u));
        PE_StoreU16(p+68u,(uint16_t)timer);
        if (timer<=0) status_flags(48u,0u);
    }
    flags=PE_LoadU32(p+76u)&192u;
    if (flags==64u || flags==128u) {
        int16_t timer=(int16_t)(PE_LoadU16(p+70u)-PE_LoadU16(p+60u));
        pe_addr_t actor=PE_LoadU32(0x8009D254u);
        PE_StoreU16(p+70u,(uint16_t)timer);
        if (timer<=0) {
            if (flags==128u && PE_LoadU8(actor+14u)==17u)
                func_8001A680_command_cut(actor,PE_LoadU8(p+18u));
            status_flags(192u,0u);
        } else if (flags==128u) for (i=0;i<3;i++) PE_StoreU32(actor+104u+i*4u,0u);
    }
    if (PE_LoadU32(p+76u)&256u) {
        int16_t timer=(int16_t)(PE_LoadU16(0x8009D228u)-1u);
        PE_StoreU16(0x8009D228u,(uint16_t)timer);
        if (timer<=0) status_flags(256u,0u);
    }
    if ((PE_LoadU32(p+76u)&512u) && (int32_t)PE_LoadU32(p+8u)<=65536) {
        PE_StoreU32(p+8u,65536u);status_flags(512u,0u);
    }
    if (PE_LoadU32(p+76u)&1024u) {
        int32_t cost;
        if (PE_LoadU32(0x8009CDDCu) && (int16_t)PE_LoadU16(p+12u)<(int16_t)PE_LoadU16(p+28u)) {
            PE_StoreU16(p+12u,(uint16_t)(PE_LoadU16(p+12u)+1u));
            PE_StoreU16(p+14u,(uint16_t)(PE_LoadU16(p+14u)+1u));
        }
        cost=(int32_t)PE_LoadU32(p+40u)/((int16_t)PE_LoadU16(p+4u)*30);
        PE_StoreU32(p+8u,PE_LoadU32(p+8u)-(uint32_t)cost);
        if ((int32_t)PE_LoadU32(p+8u)<=65536) {PE_StoreU32(p+8u,65536u);status_flags(1024u,0u);}
    }
    if (PE_LoadU32(p+76u)&0x1000000u) {
        int8_t timer=(int8_t)(PE_LoadU8(0x8009CE34u)-1u);
        PE_StoreU8(0x8009CE34u,(uint8_t)timer);
        if (timer<=0) status_flags(0x1000000u,0u);
    }
    armor=PE_LoadU32(PE_LoadU32(p+108u)+4u);
    if ((armor&0x4000u) && (int16_t)PE_LoadU16(p+12u)>0) {
        int used=0;
        while ((int16_t)PE_LoadU16(p+12u)<(int16_t)PE_LoadU16(p+28u)/5) {
            int item;
            for (item=10;item>=6;item--) if (func_80053E6C(item)) break;
            if (item<6) break;
            used=1;func_80023E14(item);(void)func_8005409C(item);
        }
        if (used) recovery_effect(0x56u,0x4B4u);
    }
    if (PE_LoadU32(PE_LoadU32(p+108u)+4u)&0x8000u) {
        static const unsigned masks[]={3u,12u,48u,192u,4096u};
        static const unsigned values[]={1u,4u,16u,64u,4096u};
        static const int items[]={13,15,14,16,17};
        for (i=0;i<5;i++) {
            unsigned state=PE_LoadU32(p+76u)&masks[i];int item=items[i];
            if (state!=values[i] && !(i==3 && state==128u)) continue;
            if (!func_80053E6C(item)) {
                if (item==17 || !func_80053E6C(17)) continue;
                item=17;
            }
            func_80023E14(item);(void)func_8005409C(item);recovery_effect(0x57u,0x4B5u);
        }
    }
}

void func_80020288(pe_addr_t enemy)
{
    pe_addr_t p=player_record(),action=PE_LoadU32(enemy+24u);
    unsigned kind=PE_LoadU8(action+1u);
    uint32_t flags=PE_LoadU32(p+76u),armor=PE_LoadU32(PE_LoadU32(p+108u)+4u);
    if (kind==1u || (kind>=3u && kind<=6u)) {
        unsigned mask=kind==1u?3u:kind==3u?12u:kind==4u?48u:192u;
        unsigned value=kind==1u?1u:kind==3u?4u:kind==4u?16u:kind==5u?128u:64u;
        unsigned resist=kind==1u?1u:kind==3u?2u:kind==4u?4u:8u;
        unsigned chance=kind==1u || kind==6u?70u:kind==5u?50u:60u;
        if ((flags&mask)==value || (kind==6u && (flags&mask)==128u)) return;
        if ((armor&resist) && func_80071A54()%100u<chance) return;
        if ((flags&mask)==mask) {status_flags(mask,0u);return;}
        if (kind>=5u) status_flags(256u,0u);
        status_flags(mask,value);
        PE_StoreU16(p+(kind==1u?64u:kind==3u?66u:kind==4u?68u:70u),9000u);
        if (kind==1u) {
            PE_StoreU8(p+58u,0u);PE_StoreU16(p+56u,PE_LoadU8(enemy+148u));
            PE_StoreU8(p+59u,PE_LoadU8(enemy+149u));
        } else if (kind==3u) {
            PE_StoreU32(0x8009D2E8u,PE_LoadU32(0x8009D2E8u)|16u);
            status_flags(0x60000u,(func_80071A54()%4u)<<17u);
        }
    } else if (kind==2u || kind==9u) {
        PE_StoreU16(0x8009D228u,0u);status_flags(0xF00u,0u);
        if (kind==9u) {
            int16_t hp=(int16_t)PE_LoadU16(p+12u);
            if (hp>=2) PE_StoreU16(p+12u,1u);
            else if (hp==1) PE_StoreU16(p+12u,65535u);
        }
    } else if (kind==7u) status_flags(0u,4096u);
    else if (kind==8u || kind==16u) {
        int16_t hp=(int16_t)PE_LoadU16(p+12u);
        if (!(flags&512u) && hp>=2) PE_StoreU16(p+12u,(uint16_t)(kind==8u?hp/2:hp*3/4));
    } else if (kind>=10u && kind<=13u && (armor&16u)) {
        if (func_80071A54()%100u<60u) return;
        if (kind<=11u) {
            PE_StoreU16(enemy+160u,(uint16_t)func_8005485C());PE_StoreU8(action+1u,0u);
            (void)func_8005409C((int16_t)PE_LoadU16(enemy+160u));
        } else {func_800553A4(enemy+160u,enemy+162u);PE_StoreU8(action+1u,0u);}
        PE_StoreU8(0x8009D1CEu,1u);
        PE_StoreU32(0x8009D1F8u,func_80054A88((int16_t)PE_LoadU16(enemy+160u),0));
    }
}
