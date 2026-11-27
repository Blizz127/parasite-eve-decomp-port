/* Original player battle tick, damage and hit reactions (AB74.s).
 * Restores hit acknowledgement, contact recovery, PE timing and HUD colors. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_plat/enhance.h"

#define GA_D_8009D278 0x8009D278u
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D1D0 0x8009D1D0u
#define GA_D_8009D298 0x8009D298u /* gp+0x528 */
#define GA_D_8009D29A 0x8009D29Au
#define GA_D_8009D29B 0x8009D29Bu
#define GA_D_8009D29C 0x8009D29Cu
#define GA_D_8009D1AC 0x8009D1ACu
#define GA_D_8009D1A0 0x8009D1A0u
#define GA_D_8009D1CE 0x8009D1CEu
#define GA_D_8009D244 0x8009D244u
#define GA_D_8009D28C 0x8009D28Cu
#define GA_D_8009D2E8 0x8009D2E8u

/* BIOS A(2Fh) rand / A(30h) srand: host model in platform/pe_bios_rand.c. */

/* func_800305C8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800305C8_port.c (src/func_800305C8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8001F814: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8001F814_port.c (src/func_8001F814.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 7). */

void func_8001F4D4(pe_addr_t actor)
{
    pe_addr_t rec=PE_LoadU32(GA_D_8009D278),body=PE_LoadU32(actor);
    pe_addr_t action=PE_LoadU32(body+24u),armor=PE_LoadU32(rec+108u);
    uint32_t flags=PE_LoadU32(rec+76u),defense=PE_LoadU16(rec+32u)/5u;
    uint32_t equipment=PE_LoadU32(armor),kind=(PE_LoadU32(body)>>21u)&7u;
    int32_t damage,critical;
    if (flags&0x1000u) defense=((uint16_t)defense*3u)/10u;
    if (flags&0x100u) defense=((uint16_t)defense*3u)/10u;
    if (kind<3u) {
        unsigned element=PE_LoadU8(action+14u);
        if (!element) PE_StoreU8(action,4u);
        else if (element!=1u) PE_StoreU8(action,3u);
    }
    damage=(int32_t)PE_LoadU16(action+12u)-(uint16_t)defense-
        (int32_t)((equipment>>(kind<3u?0u:10u))&1023u);
    critical=(int32_t)PE_LoadU8(body+144u)*(100-(int32_t)((equipment>>20u)&255u))/100;
    if ((int32_t)(func_80071A54()%100u)<critical) {
        damage=damage*3/2;PE_StoreU32(rec+76u,PE_LoadU32(rec+76u)|0x8000u);
    }
    if (!(PE_LoadU32(rec+76u)&0x200u)) {
        if ((int32_t)PE_LoadU32(rec+52u)>0) damage/=2;
        if (damage>0) {
            if (PE_LoadU8(action+1u)==10u) damage/=2;
            if (!pe_enh_player_shielded())          /* assist: god mode (identity when off) */
                PE_StoreU16(rec+12u,(uint16_t)(PE_LoadU16(rec+12u)-damage));
        }
        if ((int16_t)PE_LoadU16(rec+12u)) {
            (void)func_8001F814(actor);
            if (!(PE_LoadU32(PE_LoadU32(GA_D_8009D254)+152u)&0x100u))
                PE_StoreU32(GA_D_8009D1D0,actor);
        }
    } else if (damage>0) {
        int32_t cost;
        if (PE_LoadU8(action+1u)==10u) damage/=2;
        cost=(int32_t)(PE_LoadU32(rec+40u)<<2u)/(int16_t)PE_LoadU16(rec+28u);
        PE_StoreU32(rec+8u,PE_LoadU32(rec+8u)-(uint32_t)damage*(uint32_t)cost);
    }
    if (PE_LoadU8(PE_LoadU32(body+24u)+1u)) func_80020288(body);
}

static void player_stop_motion(void)
{
    pe_addr_t actor=PE_LoadU32(GA_D_8009D254);
    unsigned i;
    for (i=0;i<3;i++) PE_StoreU32(actor+104u+i*4u,0u);
}

static void player_quad_colors(pe_addr_t first,unsigned phase,unsigned empty)
{
    static const uint8_t colors[2][3][3]={
        {{0,130,54},{74,255,59},{37,193,57}},
        {{255,61,129},{131,19,1},{193,40,65}}
    };
    unsigned vertex,channel;
    for (vertex=0;vertex<4;vertex++) {
        unsigned color=(phase&1u)?2u:((vertex&1u)^(phase>>1u));
        for (channel=0;channel<3;channel++)
            PE_StoreU8(first+vertex*8u+channel,colors[empty][color][channel]);
    }
}

static void player_reset_pe_colors(unsigned empty)
{
    unsigned bank;
    for (bank=0;bank<2;bank++) player_quad_colors(0x800B0134u+empty*36u+bank*72u,0u,empty);
}

static void player_reset_at_colors(void)
{
    static const pe_addr_t starts[]={0x800B00ECu,0x800B0110u};
    static const uint8_t left[]={0,70,130},right[]={159,255,249};
    unsigned bank,vertex,channel;
    for (bank=0;bank<2;bank++) {
        for (vertex=0;vertex<4;vertex++) for (channel=0;channel<3;channel++)
            PE_StoreU8(starts[bank]+vertex*8u+channel,(vertex&1u)?right[channel]:left[channel]);
        for (channel=0;channel<3;channel++) PE_StoreU8(0x800B692Cu+bank*28u+channel,right[channel]);
    }
}

static void player_damage_number(void)
{
    pe_addr_t rec=PE_LoadU32(GA_D_8009D278),actor=PE_LoadU32(GA_D_8009D254);
    if ((int16_t)PE_LoadU16(rec+12u)>(int16_t)PE_LoadU16(rec+28u)) return;
    PE_StoreU16(rec+80u,(uint16_t)(PE_LoadU16(rec+14u)-PE_LoadU16(rec+12u)));
    PE_StoreU16(rec+82u,PE_LoadU16(actor+528u));PE_StoreU16(rec+84u,PE_LoadU16(actor+530u));
    PE_StoreU8(rec+86u,30u);
    PE_StoreU8(rec+87u,PE_LoadU16(rec+80u)?(uint8_t)((PE_LoadU32(rec+76u)>>14u)&2u):0u);
    PE_StoreU32(rec+76u,PE_LoadU32(rec+76u)&~0x8000u);
}

static void player_cancel_attacks(void)
{
    if ((int8_t)func_80021054()!=PE_LoadU8(0x8009D1D4u)) {
        static const pe_addr_t indices[]={0x8009D200u,0x8009D2FCu};
        unsigned i;
        for (i=0;i<2;i++) if (PE_LoadU32(indices[i])!=UINT32_MAX) {
            (void)func_8006F6D4((int32_t)PE_LoadU32(indices[i]),0,0,2,0,0);
            if (PE_Port_ShouldStop()) return;
            PE_StoreU32(indices[i],UINT32_MAX);
        }
    }
    func_80021D4C();
}

void func_80020CE4(void)
{
    pe_addr_t rec=PE_LoadU32(GA_D_8009D278);
    PE_StoreU32(GA_D_8009D2E8,PE_LoadU32(GA_D_8009D2E8)&~1u);
    PE_StoreU32(rec+76u,PE_LoadU32(rec+76u)&~0x10000u);
    player_stop_motion();
    func_8001A680_command_cut(PE_LoadU32(GA_D_8009D254),PE_LoadU8(rec+18u));
}

void func_8001D340(unsigned int mode)
{
    pe_addr_t rec=PE_LoadU32(GA_D_8009D278),actor=PE_LoadU32(GA_D_8009D254);
    uint32_t flags,phase=PE_LoadU32(0x8009D1E8u)&3u;
    if ((mode&255u)!=0u) {
        if (!(D_8009D1A0&0x100u)) {
            unsigned speed=PE_LoadU16(rec+36u),at=PE_LoadU16(rec+16u)+speed;
            flags=PE_LoadU32(rec+76u);
            if ((flags&0xC0u)==0x40u || (flags&0xC0u)==0x80u) at-=speed*2u/5u;
            else if (flags&0x100u) at+=speed/2u;
            PE_StoreU16(rec+16u,(uint16_t)at);
            if ((int32_t)PE_LoadU32(rec+8u)<(int32_t)PE_LoadU32(rec+40u) && !(flags&0x2600u)) {
                int32_t delay=(int32_t)(PE_LoadU32(rec+48u)-3u),rate;
                if (delay<=0) delay=1;
                PE_StoreU32(rec+48u,(uint32_t)delay);
                rate=(int32_t)(PE_LoadU32(rec+44u)-(uint32_t)((int32_t)PE_LoadU32(rec+40u)/(int32_t)((uint32_t)delay*100u)));
                if (rate<0x1999) rate=0x1999;
                PE_StoreU32(rec+44u,(uint32_t)rate);PE_StoreU32(rec+8u,PE_LoadU32(rec+8u)+(uint32_t)rate);
                if ((int32_t)PE_LoadU32(rec+8u)>=(int32_t)PE_LoadU32(rec+40u)) {
                    PE_StoreU32(rec+52u,240u);
                    if (PE_LoadU32(0x800B0E08u)) func_8006DF50(PE_LoadU32(0x800B0E08u),0x455,0,128,127);
                }
            }
        }
        if ((mode&255u)==1u) {
            unsigned cmd=PE_LoadU8(actor+14u);
            int move=cmd>=14u;
            if (cmd==PE_LoadU8(rec+18u) || cmd==5u) {
                move=1;
                if ((PE_LoadU32(PE_LoadU32(rec+108u)+4u)&0x4000u) &&
                    (PE_LoadU32(rec+76u)&0x4000u) && (int8_t)func_80021054()>0) move=cmd>=14u;
            }
            PE_StoreU32(GA_D_8009D2E8,move?PE_LoadU32(GA_D_8009D2E8)&~1u:PE_LoadU32(GA_D_8009D2E8)|1u);
        }
    } else PE_StoreU32(GA_D_8009D2E8,PE_LoadU32(GA_D_8009D2E8)|1u);
    if ((int32_t)PE_LoadU32(rec+8u)<0) PE_StoreU32(rec+8u,0u);
    if (PE_LoadU32(rec+76u)&0x2000u) {
        player_quad_colors(0x800B0158u+PE_LoadU32(0x8009CDDCu)*72u,phase,1u);
        if (PE_LoadU8(actor+14u)!=18u) func_8001A680_command_cut(actor,18u);
        if ((int8_t)PE_LoadU8(0x8009CE30u)==90) {
            func_8001A680_command_cut(actor,PE_LoadU8(rec+18u));
            PE_StoreU8(0x8009CE30u,0u);PE_StoreU32(rec+76u,PE_LoadU32(rec+76u)&~0x2000u);
            PE_StoreU32(rec+8u,65536u);player_reset_pe_colors(1u);
        } else {
            PE_StoreU8(0x8009CE30u,(uint8_t)(PE_LoadU8(0x8009CE30u)+1u));player_stop_motion();
        }
    }
    if (!(D_8009D1A0&0x100u)) {
        if ((int32_t)PE_LoadU32(rec+52u)>0) {
            PE_StoreU32(rec+52u,PE_LoadU32(rec+52u)-1u);
            if (PE_LoadU32(rec+52u)) {
                player_quad_colors(0x800B0134u+PE_LoadU32(0x8009CDDCu)*72u,phase,0u);
                if ((int32_t)PE_LoadU32(rec+8u)<(int32_t)PE_LoadU32(rec+40u)) PE_StoreU32(rec+52u,0u);
            }
            if (!PE_LoadU32(rec+52u)) player_reset_pe_colors(0u);
        }
        flags=PE_LoadU32(rec+76u);
        if (flags&0x800000u) {
            uint8_t timer=(uint8_t)(PE_LoadU8(0x8009D234u)-1u);
            PE_StoreU8(0x8009D234u,timer);
            if ((int8_t)timer<=0) PE_StoreU32(rec+76u,flags&~0x800000u);
        } else if (!(flags&0x180000u)) {
            pe_addr_t enemy=PE_LoadU32(GA_D_8009D20C);
            for (;enemy;enemy=PE_LoadU32(enemy+4u)) {
                pe_addr_t body=PE_LoadU32(enemy),action;
                if (enemy==actor || !body || (PE_LoadU32(enemy+152u)&16u)) continue;
                action=PE_LoadU32(body+24u);
                if (action && (PE_LoadU32(rec+76u)&0x4000u)) {
                    if ((int32_t)PE_LoadU32(body)<0 && (unsigned)(PE_LoadU8(action)-1u)<2u) {
                        unsigned kind=(PE_LoadU32(body)>>21u)&7u;
                        if (kind<3u) func_8006DCE4(PE_LoadU16(body+182u+kind*2u),0u,
                            (int16_t)PE_LoadU16(enemy+616u),(int16_t)PE_LoadU16(enemy+618u),(int16_t)PE_LoadU16(enemy+620u));
                        PE_StoreU32(rec+76u,PE_LoadU32(rec+76u)|0x10000000u);
                        func_8001F4D4(enemy);player_stop_motion();
                        PE_StoreU32(body,PE_LoadU32(body)&0x7FFFFFFFu);
                        PE_StoreU16(body+156u,(uint16_t)(PE_LoadU16(body+156u)+1u));
                        PE_StoreU8(0x8009CE34u,90u);PE_StoreU32(rec+76u,PE_LoadU32(rec+76u)|0x1000000u);
                    }
                    player_damage_number();
                }
                if ((PE_LoadU32(enemy+152u)&0x2000000u) && !(PE_LoadU32(rec+76u)&0x1000000u) &&
                    (int32_t)PE_LoadU32(body+16u)>0) {
                    if (!(PE_LoadU32(rec+76u)&0x200u) && !pe_enh_player_shielded())
                        PE_StoreU16(rec+12u,(uint16_t)(PE_LoadU16(rec+12u)-PE_LoadU8(body+146u)));
                    PE_StoreU8(0x8009CE34u,90u);PE_StoreU32(rec+76u,PE_LoadU32(rec+76u)|0x1000000u);
                    player_damage_number();player_stop_motion();
                    PE_StoreU32(actor+152u,PE_LoadU32(actor+152u)&~0xC0000u);
                    PE_StoreU16(rec+74u,(uint16_t)func_8001F814(enemy));
                    PE_StoreU8(rec+73u,PE_LoadU8(body+147u));PE_StoreU8(rec+72u,6u);
                }
            }
            if ((int16_t)PE_LoadU16(rec+12u)<(int16_t)PE_LoadU16(rec+14u)) {
                player_damage_number();PE_StoreU16(rec+14u,PE_LoadU16(rec+12u));
            }
        }
        PE_StoreU32(rec+76u,PE_LoadU32(rec+76u)&~0x4000u);
        if ((int8_t)PE_LoadU8(rec+72u)) {
            if ((PE_LoadU32(actor+152u)&0xC0000u) || !PE_LoadU8(rec+73u)) {
                PE_StoreU32(actor+152u,PE_LoadU32(actor+152u)&~0xC0000u);
                PE_StoreU8(rec+72u,0u);PE_StoreU8(rec+73u,0u);
            } else {
                unsigned speed=PE_LoadU8(rec+73u);int32_t angle=(int16_t)PE_LoadU16(rec+74u);
                PE_StoreU32(actor+40u,PE_LoadU32(actor+64u)+((uint32_t)func_80077CF4(angle)*speed<<4u));
                PE_StoreU32(actor+48u,PE_LoadU32(actor+72u)+((uint32_t)func_80077DC4(angle)*speed<<4u));
                PE_StoreU8(rec+73u,(uint8_t)((int32_t)speed-(int32_t)speed/(int8_t)PE_LoadU8(rec+72u)));
                PE_StoreU8(rec+72u,(uint8_t)(PE_LoadU8(rec+72u)-1u));
            }
        }
        func_8001F9C4();
        if (PE_Port_ShouldStop()) return;
    }
    if (PE_LoadU8(rec+86u)) {func_80032B0C(0u,rec+80u);PE_StoreU8(rec+86u,(uint8_t)(PE_LoadU8(rec+86u)-1u));}
    if ((int16_t)PE_LoadU16(rec+14u)<(int16_t)PE_LoadU16(rec+12u)) {
        PE_StoreU16(rec+88u,(uint16_t)(PE_LoadU16(rec+12u)-PE_LoadU16(rec+14u)));
        PE_StoreU16(rec+90u,PE_LoadU16(actor+528u));PE_StoreU16(rec+92u,(uint16_t)(PE_LoadU16(actor+530u)-8u));
        PE_StoreU8(rec+94u,30u);PE_StoreU8(rec+95u,1u);PE_StoreU16(rec+14u,PE_LoadU16(rec+12u));
    }
    if (PE_LoadU8(rec+94u)) {func_80032B0C(0u,rec+88u);PE_StoreU8(rec+94u,(uint8_t)(PE_LoadU8(rec+94u)-1u));}
    if (PE_LoadU8(rec+102u)) {func_80032B0C(0u,rec+96u);PE_StoreU8(rec+102u,(uint8_t)(PE_LoadU8(rec+102u)-1u));}
    PE_StoreU16(rec+14u,PE_LoadU16(rec+12u));
    if ((int32_t)PE_LoadU32(rec+8u)<=0) {
        PE_StoreU32(rec+76u,PE_LoadU32(rec+76u)|0x2000u);player_cancel_attacks();
        if (PE_Port_ShouldStop()) return;
    }
    if (PE_LoadU32(GA_D_8009D28C)==1u) {player_reset_at_colors();player_reset_pe_colors(0u);}
    if ((int16_t)PE_LoadU16(rec+12u)>0) return;
    player_stop_motion();player_reset_at_colors();player_reset_pe_colors(0u);player_reset_pe_colors(1u);
    if (PE_LoadU32(rec+76u)&0x10000u) func_80020CE4();
    func_800374E8();
    if (PE_Port_ShouldStop()) return;
    player_cancel_attacks();
    if (PE_Port_ShouldStop()) return;
    PE_StoreU32(GA_D_8009D1AC,PE_LoadU32(GA_D_8009D1AC)&~0x300u);
    PE_StoreU32(GA_D_8009D28C,3u);PE_StoreU8(GA_D_8009D1CE,0u);
    func_8006DE80(0x46B,0,(int16_t)PE_LoadU16(actor+42u),(int16_t)PE_LoadU16(actor+46u),(int16_t)PE_LoadU16(actor+50u));
    if (PE_Port_ShouldStop()) return;
    PE_StoreU8(GA_D_8009D244,0u);func_80062F9C();
    if (PE_LoadU32(0x800BCF88u)&0x2000u) func_80067CBC();
    D_8009D1A0&=~4u;PE_StoreU32(GA_D_8009D1A0,PE_LoadU32(GA_D_8009D1A0)&~4u);
    PE_StoreU32(GA_D_8009D2E8,PE_LoadU32(GA_D_8009D2E8)|1u);
    func_8001A680_command_cut(actor,19u);PE_StoreU16(rec+12u,0u);
}
