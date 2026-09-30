/* Complete original M0000I update942FC..958D4 and subtitle958D4..95994.
 * Borrowed message-list tails are explicit inputs until the outer frame loop
 * supplies their proven original stack writers. Never invent zero tails. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#define A(o) (0x80190000u+(o))
#define B(o) PE_LoadU8(A(o))
#define H(o) ((int32_t)(int16_t)PE_LoadU16(A(o)))
#define W(o) PE_LoadU32(A(o))
#define SB(o,v) PE_StoreU8(A(o),(uint8_t)(v))
#define SH(o,v) PE_StoreU16(A(o),(uint16_t)(v))
#define SW(o,v) PE_StoreU32(A(o),(uint32_t)(v))
#define CHECK() do {if(PE_Port_StopEpoch()!=epoch)goto done;} while(0)

/* func_80038940: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80038940_port.c (src/func_80038940.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static int message_list(pe_addr_t scratch,const uint16_t *tail)
{
    if(!tail){PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;}
    PE_StoreU16(scratch,0);
    for(unsigned i=0;i<4;i++)PE_StoreU16(scratch+2u+i*2u,tail[i]);
    return 1;
}
int PE_TransitionSubtitle(uint32_t id,uint32_t action,const uint16_t *tail)
{
    const pe_addr_t scratch=0x1F8003D0u;
    uint8_t saved[10];int value=(int16_t)id;
    for(unsigned i=0;i<10;i++)saved[i]=PE_LoadU8(scratch+i);
    if(!(action&255u) && (uint32_t)value!=W(0xC140)) {
        /* A missing tail stops before mutating dialogue state. */
        if(!message_list(scratch,tail))goto done;
        func_8003746C(H(0xC144));func_80038940(value,128,128,128);
        func_800375E0(value,3,scratch);SW(0xC144,value);
    }
    if((action&255u)==1u)func_8003746C(H(0xC144));
 done:
    for(unsigned i=0;i<10;i++)PE_StoreU8(scratch+i,saved[i]);
    return value;
}
static pe_addr_t selected_entry(void)
{return 0x801EA378u+(uint32_t)(H(0xCC52)*52);}
static void select_camera(uint32_t x,uint32_t y,uint32_t time)
{func_80195994((uint32_t)H(0xCC52),x,y,time);}
static void transition_ui_sound(uint32_t id)
{
    if(PE_LoadU32(0x800B0E08u))func_8006DF50(PE_LoadU32(0x800B0E08u),id,0,128,127);
}
static void transition_exit_request(void)
{SH(0xC02A,1);SH(0xC026,16);SW(0xCA68,H(0xCC52));}
static void transition_camera_reset(void)
{SH(0xC02C,1);SB(0xC045,0);select_camera(4,5,0);}
static void transition_sample(uint32_t time,uint32_t id,pe_addr_t pos,pe_addr_t rot)
{
    unsigned epoch=PE_Port_StopEpoch();
    pe_addr_t package=func_8006EC6C(0x801D0260u,2);
    if(PE_Port_StopEpoch()==epoch)func_8018F55C(time,id,package,pos,rot);
}
static void transition_blend(unsigned count,unsigned delta,unsigned output)
{
    if(!H(count))return;
    SH(count,H(count)-1);
    uint32_t v[3];
    for(unsigned i=0;i<3;i++)v[i]=W(delta+16u+i*4u)+W(delta+i*4u);
    for(unsigned i=0;i<3;i++)SW(delta+16u+i*4u,v[i]);
    unsigned shift=(unsigned)H(count+2u)&31u;
    for(unsigned i=0;i<3;i++)SW(output+i*4u,(uint32_t)H(delta+32u+i*2u)+(uint32_t)((int32_t)v[i]>>shift));
    if(!H(count))for(unsigned i=0;i<3;i++)SW(output+i*4u,H(delta+40u+i*2u));
}
void PE_TransitionUpdate(const uint16_t *menu_tail,const uint16_t *subtitle_tail)
{
    const pe_addr_t scratch=0x1F800300u;
    uint8_t saved[160];unsigned epoch=PE_Port_StopEpoch();
    for(unsigned i=0;i<160;i++)saved[i]=PE_LoadU8(scratch+i);
    if(H(0xC026)) {SH(0xC026,H(0xC026)-1);if(H(0xC026)==1)SH(0xC024,1);}
    if(B(0xC045)==1) {
        select_camera(0,0,W(0xCC1C)<<8);CHECK();
        pe_addr_t entry=selected_entry();uint32_t time=W(0xCC1C);
        if((int32_t)time>=(int16_t)PE_LoadU16(entry+40u))SH(0xC032,0);
        if((int32_t)time<(int32_t)(PE_LoadU32(entry+4u)-1u))SW(0xCC1C,time+1u);else SB(0xC045,0);
    }
    if(H(0xC026) && !B(0xC045) && H(0xC034)!=10 && !B(0xC046) && W(0xBFF8)==UINT32_MAX) {
        SW(0xCC1C,W(0xCC1C)+1u);select_camera(0,0,W(0xCC1C)<<8);CHECK();
    }
    if(B(0xC044)==1) {
        SW(0xC048,W(0xC048)+1u);
        if((int32_t)W(0xC048)>200 && !B(0xC040)) {
            if(!B(0xC046))SW(0xC04C,0);
            SB(0xC046,1);
        } else SB(0xC046,0);
    } else SB(0xC046,0);
    if(H(0xC034)!=10 && !H(0xC026) && PE_LoadU32(0x8009D26Cu)) {
        if((PE_LoadU32(0x8009D1F4u)&2u) && W(0xBFF8)==UINT32_MAX) {
            int next=(int16_t)(H(0xC034)+1);SH(0xC034,next%3);
            if(!H(0xC034)) {
                transition_camera_reset();CHECK();
                if(!H(0xC026)){transition_ui_sound(0x44D);CHECK();}
            }
            if(H(0xC034)==1) {
                if(PE_LoadU32(0x8009D26Cu)&32u) {
                    if(!H(0xC026)){transition_ui_sound(0x44C);CHECK();}
                    SH(0xC02C,-1);SH(0xC032,1);SW(0xC048,0);SB(0xC044,0);SB(0xC045,0);SH(0xC13C,0);
                    transition_sample(W(0xC038)+32u,74,scratch,scratch+80u);CHECK();
                    transition_sample(W(0xC038)+32u,75,scratch+16u,scratch+80u);CHECK();
                    for(unsigned i=0;i<3;i++) {
                        PE_StoreU16(scratch+64u+i*2u,(uint16_t)(PE_LoadU32(scratch+i*4u)-(i==1?100u:0u)));
                        PE_StoreU16(scratch+72u+i*2u,(uint16_t)W(0xC810+i*4u));
                    }
                    func_80195BC8(scratch+64u,scratch+72u,4,5);CHECK();SH(0xC13E,1);
                } else {SH(0xC034,2);SH(0xC13E,0);}
            }
            if(H(0xC034)==2) {
                if(!H(0xC026)){transition_ui_sound(0x44C);CHECK();}
                if(!H(0xC13E))SH(0xC02C,-1);
                if(H(0xC13E)==1)SH(0xC02C,0);
                SH(0xC032,1);SW(0xC048,0);SB(0xC044,0);SB(0xC045,0);SH(0xC13C,0);
                transition_sample(W(0xC03C)+256u,76,scratch+32u,scratch+80u);CHECK();
                transition_sample(W(0xC03C)+256u,77,scratch+48u,scratch+80u);CHECK();
                for(unsigned i=0;i<3;i++) {
                    PE_StoreU16(scratch+64u+i*2u,(uint16_t)PE_LoadU32(scratch+48u+i*4u));
                    PE_StoreU16(scratch+72u+i*2u,(uint16_t)PE_LoadU32(scratch+32u+i*4u));
                }
                func_80195BC8(scratch+64u,scratch+72u,4,5);CHECK();
            }
        }
        if((PE_LoadU32(0x8009D1F4u)&0x50u) && H(0xC034)) {
            SH(0xC034,0);transition_camera_reset();CHECK();
            if(!H(0xC026)){transition_ui_sound(0x44D);CHECK();}
        }
        if(B(0xC040)<2 && !H(0xC034) && !H(0xC13C)) {
            if((PE_LoadU32(0x8009D1F4u)&0x20000000u) && !H(0xC026)) {
                if(B(0xC040)==1)transition_exit_request();
                if(!B(0xC044)) {
                    transition_ui_sound(0x44C);CHECK();
                    SH(0xC02C,-1);SB(0xC045,1);SB(0xC044,1);SW(0xCC1C,0);SW(0xC048,1);
                } else {transition_ui_sound(0x44C);CHECK();transition_exit_request();}
                if(W(0xC000)==1 && (uint32_t)H(0xCC52)==W(0xC004)) {
                    SH(0xC02C,-1);SH(0xC02A,1);SB(0xC045,0);SB(0xC044,0);SW(0xCC1C,0);SW(0xC048,0);SH(0xC026,16);SW(0xCA68,H(0xCC52));
                }
            }
            if((PE_LoadU32(0x8009D1F4u)&0x40000000u) && W(0xBFF8)==UINT32_MAX && !H(0xC026)) {
                SH(0xC032,1);
                if(B(0xC044)==1) {
                    transition_ui_sound(0x44D);CHECK();select_camera(4,5,0);CHECK();
                    SH(0xC02C,1);SB(0xC044,0);SB(0xC045,0);
                }
            }
            for(unsigned direction=0;direction<2;direction++) {
                if(!(PE_LoadU32(0x8009D1F4u)&(direction?0x10u:0x40u)) || W(0xBFF8)!=UINT32_MAX)continue;
                if(B(0xC044)==1)SH(0xC02C,1);
                SB(0xC045,0);SB(0xC044,0);SW(0xCC1C,0);SH(0xC032,1);
                if(PE_LoadU8(selected_entry()+26u+direction)==255u)continue;
                do {SH(0xCC52,PE_LoadU8(selected_entry()+26u+direction));}while(!PE_LoadU8(selected_entry()+28u));
                uint32_t frames=PE_LoadU32(selected_entry()+4u);
                SW(0xC018,1);SW(0xCC1C,frames);select_camera(4,5,0);CHECK();
                func_801939B0((uint32_t)H(0xCC52));CHECK();
            }
        }
    }
    if((H(0xC034)==1 || H(0xC034)==2) && !H(0xC054) && !H(0xC050)) {
        int mode=H(0xC034);pe_addr_t actor=PE_LoadU32(mode==1?0x801EA58Cu:0x801EA580u);
        for(unsigned i=0;i<3;i++)SW(0xC330+i*4u,PE_LoadU32(actor+28u+i*4u)-(mode==1 && i==1?300u:0u));
        actor=PE_LoadU32(0x801EA578u);
        for(unsigned i=0;i<3;i++)SW(0xC810+i*4u,PE_LoadU32(actor+28u+i*4u));
    }
    if(B(0xC046)==1) {
        transition_sample(W(0xC04C),PE_LoadU8(selected_entry()+3u),A(0xC330),scratch+80u);CHECK();
        transition_sample(W(0xC04C),PE_LoadU8(selected_entry()+2u),A(0xC810),scratch+80u);CHECK();
        SW(0xC04C,W(0xC04C)+128u);
    }
    if(H(0xC034)==10) {
        SH(0xC030,H(0xC030)+1);
        transition_sample(W(0xC04C),78,A(0xC330),scratch+80u);CHECK();
        transition_sample(W(0xC04C),79,A(0xC810),scratch+80u);CHECK();SW(0xC04C,W(0xC04C)+16u);
        static const unsigned times[]={75,285,315,525,555,765,795,1005,1035};
        static const unsigned ids[]={11,14,12,14,13,14,14,14,14};
        for(unsigned i=0;i<9;i++)if(H(0xC030)==(int32_t)times[i]) {
            int value=PE_TransitionSubtitle(ids[i],(i&1u)||i==8u,subtitle_tail);CHECK();
            if(!(i&1u)){SH(0xC0C4,value);if(i!=8u)SW(0xC0C8,0);}
        }
        uint32_t age=W(0xC0C8);
        if(!age){SB(0xC0CD,0);SB(0xC0CC,0);}
        if(age==24)SB(0xC0CD,1);
        if(age==186)SB(0xC0CD,2);
        SW(0xC0C8,age+1u);
        if(!B(0xC0CD) && B(0xC0CC)<120)SB(0xC0CC,B(0xC0CC)+5);
        if(B(0xC0CD)==2 && B(0xC0CC)>10)SB(0xC0CC,B(0xC0CC)-5);
        func_80038940(H(0xC0C4),B(0xC0CC),B(0xC0CC),B(0xC0CC));
        SW(0xC0BC,W(0xC0BC)+1u);
        if(W(0xC0BC)==1201){transition_exit_request();func_800868AC(300,0);CHECK();SW(0xC0C0,1);}
        if(W(0xC0BC)==1050){func_80086C5C((int8_t)PE_LoadU8(0x800B0DB5u),220,0);CHECK();}
    }
    func_80195D3C();CHECK();
    if(W(0xC018)) {
        /* Original local list has only its first halfword initialized. */
        SW(0xC018,0);
        if(W(0xC01C)==1){func_8003746C(H(0xC028));CHECK();}
        func_80038940(PE_LoadU8(selected_entry()+29u),128,128,128);
        if(H(0xC034)!=10) {
            if(!message_list(scratch+112u,menu_tail))goto done;
            func_800375E0(PE_LoadU8(selected_entry()+29u),3,scratch+112u);CHECK();
        }
        SW(0xC01C,1);SH(0xC028,PE_LoadU8(selected_entry()+29u));
    }
    transition_blend(0xC054,0xC08C,0xC810);transition_blend(0xC050,0xC05C,0xC330);
 done:
    for(unsigned i=0;i<160;i++)PE_StoreU8(scratch+i,saved[i]);
}
void func_801942FC(void)
{
    /* Sole original caller9234C supplies s2=0/1,s3=00FFFFFF. The most
     * recent F55C saves them at the subtitle list's borrowed bytes:
     * s2.high=0, s3.low=-1. Opener stops at -1; later bytes are unread.
     * pe_transition_stack_audit.py verifies these last writers. */
    static const uint16_t subtitle_tail[]={0,0xFFFF,0,0};
    PE_TransitionUpdate(NULL,subtitle_tail);
}
#undef A
#undef B
#undef H
#undef W
#undef SB
#undef SH
#undef SW
#undef CHECK
