/* Scripted battle exit: full 2DC58, 295E4 and 2F9CC guest semantics.
 * Authority: disc 1 19DE4.s, matching src/192C8 and src/2F9CC.
 * VM96 enters mode8; only its completed cleanup publishes mode12.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"



/* Complete 27A08: record a pending hit, then wait for its model flash.
 * The terminating fade returns one immediately; a drawn fade returns zero. */
int func_80027A08(pe_addr_t actor)
{
    pe_addr_t body=PE_LoadU32(actor),walk;
    uint32_t state=PE_LoadU32(body)&0x6000u;
    int8_t kind=(int8_t)PE_LoadU8(body+5u);
    if (state==0x2000u) {
        pe_addr_t action=0x800BE830u+PE_LoadU8(0x8009D1D4u)*8u;
        int16_t command=(int16_t)PE_LoadU16(action+4u);
        if (command!=406 && command!=393) {
            pe_addr_t weapon=PE_LoadU32(PE_LoadU32(0x8009D278u)+0x68u);
            if ((int16_t)PE_LoadU16(weapon+6u)==6) {
                if (PE_LoadU32(action)==actor) {
                    PE_StoreU32(0x8009D208u,(uint32_t)func_8006F39C(0x5Bu,actor));
                    PE_StoreU8(0x8009D23Cu,0u);
                }
            } else {
                uint8_t count=PE_LoadU8(0x8009D23Cu),i=0;
                if (!count) while (PE_LoadU32(0x800B8A90u+i*4u)) {
                    PE_StoreU32(0x800B8A90u+i*4u,0u); i++;
                }
                PE_StoreU8(0x8009D23Cu,(uint8_t)(count+1u));
                PE_StoreU32(0x800B8A90u+count*4u,actor);
            }
        }
        if (kind && (int32_t)PE_LoadU32(body+16u)>0) PE_StoreU8(0x8009CE68u,255u);
        return 0;
    }
    if (state==0x4000u && kind) {
        uint8_t color=(uint8_t)(PE_LoadU8(0x8009CE68u)-8u);
        PE_StoreU8(0x8009CE68u,color);
        if (kind==4) {
            int ready=1;
            for (walk=PE_LoadU32(0x8009D20Cu);walk;walk=PE_LoadU32(walk+4u)) {
                pe_addr_t other=PE_LoadU32(walk);
                if (walk==PE_LoadU32(0x8009D254u) || !other || (int8_t)PE_LoadU8(other+5u)!=4) continue;
                if (color<=128u) {
                    PE_StoreU16(walk+0x250u,PE_LoadU16(walk+0x250u)|0x20u);
                    PE_StoreU32(body,PE_LoadU32(body)&~0x6000u);
                } else { func_8003CAEC(walk+0x1B4u,color,color,color); ready=0; }
            }
            return ready;
        }
        if (kind==1) actor=PE_LoadU32(actor+0x18Cu);
        else if ((int32_t)PE_LoadU32(body+16u)<=0) return 1;
        if (color<=128u) {
            PE_StoreU16(actor+0x250u,PE_LoadU16(actor+0x250u)|0x20u);
            PE_StoreU32(body,PE_LoadU32(body)&~0x6000u);
        } else { func_8003CAEC(actor+0x1B4u,color,color,color); return 0; }
    }
    return 1;
}

static uint32_t exit_flags(void)
{ return D_8009D1A0 | PE_LoadU32(0x8009D1A0u); }

static void exit_set_flags(uint32_t flags)
{ D_8009D1A0=flags; PE_StoreU32(0x8009D1A0u,flags); }

static void exit_rgb(pe_addr_t at,const uint8_t *color)
{
    unsigned i;
    for (i=0;i<3;i++) PE_StoreU8(at+i,color[i]);
}

/* The original repeated byte stores restore both packet banks. */
static void exit_default_colors(unsigned first)
{
    static const pe_addr_t groups[3][4]={
        {0x800B00ECu,0x800B00FCu,0x800B0110u,0x800B0120u},
        {0x800B0134u,0x800B0144u,0x800B017Cu,0x800B018Cu},
        {0x800B0158u,0x800B0168u,0x800B01A0u,0x800B01B0u}};
    static const uint8_t colors[3][2][3]={
        {{0,70,130},{159,255,249}},{{0,130,54},{74,255,59}},
        {{255,61,129},{131,19,1}}};
    unsigned g,i;
    for (g=first;g<3;g++) for (i=0;i<4;i++) {
        exit_rgb(groups[g][i],colors[g][0]);
        exit_rgb(groups[g][i]+8u,colors[g][1]);
    }
    if (!first) {
        exit_rgb(0x800B692Cu,colors[0][1]); exit_rgb(0x800B6948u,colors[0][1]);
    }
}

/* func_800295E4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800295E4_port.c (src/func_800295E4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* func_8002F9CC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8002F9CC_port.c (src/func_8002F9CC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static void exit_pulse(pe_addr_t record)
{
    static const uint8_t at[4][2][3]={
        {{0,70,130},{159,255,249}},{{80,163,190},{80,163,190}},
        {{159,255,249},{0,70,130}},{{80,163,190},{80,163,190}}};
    static const uint8_t pe[4][2][3]={
        {{255,61,129},{131,19,1}},{{193,40,65},{193,40,65}},
        {{131,19,1},{255,61,129}},{{193,40,65},{193,40,65}}};
    unsigned bank=PE_LoadU32(0x8009CDDCu),phase=PE_LoadU32(0x8009D250u)&3u,i;
    if (PE_LoadU16(record+16u)>=9000u) {
        for (i=0;i<4;i++) exit_rgb(0x800B00ECu+bank*36u+i*8u,at[phase][i&1u]);
        exit_rgb(0x800B692Cu+bank*28u,at[phase][1]);
    }
    if (PE_LoadU32(record+0x4Cu)&0x2000u)
        for (i=0;i<4;i++) exit_rgb(0x800B0158u+bank*72u+i*8u,pe[phase][i&1u]);
}

void func_8002DC58(void)
{
    pe_addr_t aya=PE_LoadU32(0x8009D254u),record=PE_LoadU32(0x8009D278u),actor;
    uint8_t phase;
    int ready=1;
    PE_StoreU32(aya+0x68u,0u); PE_StoreU32(aya+0x6Cu,0u); PE_StoreU32(aya+0x70u,0u);
    exit_pulse(record);
    if (PE_LoadU8(aya+14u)!=PE_LoadU8(record+18u))
        func_8001A680_command_cut(aya,PE_LoadU8(record+18u));
    phase=PE_LoadU8(0x8009CE74u);
    if (!phase) {
        PE_StoreU32(0x8009D2E8u,PE_LoadU32(0x8009D2E8u)|1u);
        for (actor=PE_LoadU32(0x8009D20Cu);actor;actor=PE_LoadU32(actor+4u)) {
            pe_addr_t body=PE_LoadU32(actor);
            unsigned command;
            if (actor==aya || !body || (int8_t)PE_LoadU8(body+5u)==1) continue;
            command=PE_LoadU8(actor+14u);
            if (command<2u || command>3u) {
                if (PE_LoadU8(actor+15u)==PE_LoadU16(actor+26u))
                    func_8001A680_command_cut(actor,(uint16_t)(int16_t)(int8_t)PE_LoadU8(body+6u));
                else if (PE_LoadU32(actor+28u)!=0x10000u) PE_StoreU32(actor+28u,0x10000u);
            }
            if (!(uint8_t)func_80027A08(actor)) ready=0;
        }
        if (!ready) return;
        func_800866A4(0u,255u);
        func_800703F4();
        PE_StoreU8(0x8009CE74u,(uint8_t)(PE_LoadU8(0x8009CE74u)+1u));
    } else if (phase==1u) {
        uint32_t flags;
        if (func_8006D60C(0)==1) return;
        func_8002F9CC();
        flags=exit_flags();
        if (flags&0x1800u) {
            exit_set_flags(flags&~0x1800u);
            func_8001A680_command_cut(aya,24u);
        } else func_8001A680_command_cut(aya,21u);
        func_800295E4();
        exit_default_colors(0u);
        PE_StoreU32(0x8009D28Cu,12u);
        func_800293F4(0u);
    }
}

/* Full 2D1F0, mode 7: AT pulse and floating amounts, followed by enemy
 * hit flashes. Timers capture projected coordinates only on their first tick. */
void func_8002D1F0(void)
{
    static const uint8_t colors[4][2][3]={
        {{0,70,130},{159,255,249}},{{80,163,190},{80,163,190}},
        {{159,255,249},{0,70,130}},{{80,163,190},{80,163,190}}};
    pe_addr_t record=PE_LoadU32(0x8009D278u),actor;
    if (PE_LoadU16(record+0x10u)>=9000u) {
        unsigned bank=PE_LoadU32(0x8009CDDCu),phase=PE_LoadU32(0x8009D250u)&3u;
        for (unsigned i=0;i<4;i++)
            exit_rgb(0x800B00ECu+bank*36u+i*8u,colors[phase][i&1u]);
        exit_rgb(0x800B692Cu+bank*28u,colors[phase][1]);
    }
    for (unsigned offset=0x50u;offset<=0x58u;offset+=8u) {
        record=PE_LoadU32(0x8009D278u);
        if (!PE_LoadU8(record+offset+6u)) continue;
        if (PE_LoadU8(record+offset+6u)==30u) {
            pe_addr_t aya=PE_LoadU32(0x8009D254u);
            PE_StoreU16(record+offset+2u,PE_LoadU16(aya+0x210u));
            PE_StoreU16(record+offset+4u,PE_LoadU16(aya+0x212u));
            if (offset==0x58u) PE_StoreU8(record+0x5Fu,1u);
        }
        func_80032B0C(0u,PE_LoadU32(0x8009D278u)+offset);
        if (PE_Port_ShouldStop()) return;
        record=PE_LoadU32(0x8009D278u);
        PE_StoreU8(record+offset+6u,(uint8_t)(PE_LoadU8(record+offset+6u)-1u));
    }
    for (actor=PE_LoadU32(0x8009D20Cu);actor;actor=PE_LoadU32(actor+4u)) {
        pe_addr_t body;
        if (actor==PE_LoadU32(0x8009D254u)) continue;
        body=PE_LoadU32(actor);
        if (!body) continue;
        if (PE_LoadU32(body)&0x6000u) {
            func_80027A08(actor);
            if (PE_Port_ShouldStop()) return;
            if ((PE_LoadU32(body)&0x6000u)==0x2000u)
                PE_StoreU32(body,(PE_LoadU32(body)&~0x6000u)|0x4000u);
        }
        if (!PE_LoadU8(body+0xD6u)) continue;
        if (PE_LoadU8(body+0xD6u)==30u) {
            PE_StoreU16(body+0xD2u,PE_LoadU16(actor+0x210u));
            PE_StoreU16(body+0xD4u,PE_LoadU16(actor+0x212u));
        }
        func_80032B0C(1u,body+0xD0u);
        if (PE_Port_ShouldStop()) return;
        PE_StoreU8(body+0xD6u,(uint8_t)(PE_LoadU8(body+0xD6u)-1u));
    }
}

static void pe_battle_ready_tail(void)
{
    static const uint16_t groups[3][4] = {
        {0xECu,0xFCu,0x110u,0x120u},
        {0x134u,0x144u,0x17Cu,0x18Cu},
        {0x158u,0x168u,0x1A0u,0x1B0u}
    };
    static const uint8_t colors[3][6] = {
        {0u,0x46u,0x82u,0x9Fu,0xFFu,0xF9u},
        {0u,0x82u,0x36u,0x4Au,0xFFu,0x3Bu},
        {0xFFu,0x3Du,0x81u,0x83u,0x13u,1u}
    };
    pe_addr_t record = PE_LoadU32(0x8009D278u);
    unsigned group, bank, color;
    func_800866A4(0u, 0xFFu);
    if (record) {
        PE_StoreU8(record + 0x49u, 0u);
        PE_StoreU8(record + 0x48u, 0u);
    }
    func_8002CF24_mode7_cut();
    for (group=0; group<3; group++)
        for (bank=0; bank<4; bank++)
            for (color=0; color<3; color++) {
                pe_addr_t packet=0x800B0000u+groups[group][bank];
                PE_StoreU8(packet+color,colors[group][color]);
                PE_StoreU8(packet+8u+color,colors[group][3u+color]);
            }
    for (color=0; color<3; color++) {
        PE_StoreU8(0x800B692Cu+color,colors[0][3u+color]);
        PE_StoreU8(0x800B6948u+color,colors[0][3u+color]);
    }
    PE_StoreU32(0x8009D2E8u, PE_LoadU32(0x8009D2E8u) & ~1u);
}


/* Full 2BC90: settle commands and transient amounts before entering mode7.
 * A changed command or a nonzero amount timer delays readiness for this tick,
 * even if its last frame/timer is consumed by the calls below. */
void func_8002BC90(void)
{
    pe_addr_t aya,actor,record;
    int ready=1;
    func_80021D4C();
    if (PE_Port_ShouldStop()) return;
    func_800374E8();
    if (PE_Port_ShouldStop()) return;
    PE_StoreU32(0x8009D2E8u,PE_LoadU32(0x8009D2E8u)|1u);
    aya=PE_LoadU32(0x8009D254u);
    PE_StoreU32(aya+0x68u,0u);PE_StoreU32(aya+0x6Cu,0u);PE_StoreU32(aya+0x70u,0u);
    exit_set_flags(exit_flags()&~4u);
    exit_pulse(PE_LoadU32(0x8009D278u));
    for (unsigned offset=0x50u;offset<=0x60u;offset+=8u) {
        record=PE_LoadU32(0x8009D278u);
        if (!PE_LoadU8(record+offset+6u)) continue;
        func_80032B0C(0u,record+offset);
        if (PE_Port_ShouldStop()) return;
        record=PE_LoadU32(0x8009D278u);
        PE_StoreU8(record+offset+6u,(uint8_t)(PE_LoadU8(record+offset+6u)-1u));
        ready=0;
    }
    for (actor=PE_LoadU32(0x8009D20Cu);actor;actor=PE_LoadU32(actor+4u)) {
        pe_addr_t body;
        if (actor==PE_LoadU32(0x8009D254u)) {
            record=PE_LoadU32(0x8009D278u);
            if (PE_LoadU8(actor+14u)==PE_LoadU8(record+18u)) continue;
            ready=0;
            PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)&~0x100u);
            if (PE_LoadU8(actor+15u)!=PE_LoadU16(actor+26u)) continue;
            if (PE_LoadU8(actor+14u)<4u && PE_LoadU16(0x8009D298u)) {
                if (PE_LoadU16(0x8009D298u)>=2u)
                    PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x100u);
                PE_StoreU16(0x8009D298u,0u);
                func_8001A680_command_cut(PE_LoadU32(0x8009D254u),PE_LoadU8(0x8009D29Au));
                if (PE_Port_ShouldStop()) return;
                aya=PE_LoadU32(0x8009D254u);
                PE_StoreU32(aya+0x14u,PE_LoadU32(0x8009D29Cu));
                PE_StoreU32(aya+0x18u,PE_LoadU32(0x8009D29Cu)-0x10000u);
            } else {
                func_8001A680_command_cut(actor,PE_LoadU8(record+18u));
                if (PE_Port_ShouldStop()) return;
            }
            continue;
        }
        body=PE_LoadU32(actor);
        if (!body || (int8_t)PE_LoadU8(body+5u)==1) continue;
        if ((uint32_t)(PE_LoadU8(actor+14u)-2u)>=2u) {
            PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)&~0x1000u);
            func_80036254(actor);
            if (PE_Port_ShouldStop()) return;
            ready=0;
            if (PE_LoadU8(actor+15u)==PE_LoadU16(actor+26u)) {
                func_8001A680_command_cut(actor,(uint16_t)(int16_t)(int8_t)PE_LoadU8(PE_LoadU32(actor)+6u));
                if (PE_Port_ShouldStop()) return;
            } else if (PE_LoadU32(actor+28u)!=0x10000u) PE_StoreU32(actor+28u,0x10000u);
        }
        func_80027D14(actor);
        if (PE_Port_ShouldStop()) return;
    }
    if (func_8006914C(0)!=0) ready=0;
    if (PE_Port_ShouldStop()) return;
    if (ready) pe_battle_ready_tail();
}
