/* M0028I effect 49, original Disc 1 LBA15016 C2 overlay.
 * Native translation of 80191514..801920FC and four EXE motion helpers.
 * SHA256 c15d03313a578f7c7ba07f255345df8e17734e38dd807162426d0bc3a7fd7e94.
 * Original scratchpad temporaries use host locals; persistent data stays guest RAM. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

int PE_M28MovementOverlay(void)
{
    return PE_LoadU32(0x8019151Cu)==0x24020001u && PE_LoadU32(0x801915ACu)==0x2CC2001Au &&
        PE_LoadU32(0x80191894u)==0x27BDFFD8u && PE_LoadU32(0x801920D8u)==0xA0800003u;
}
static int movement_init(pe_addr_t slot,uint32_t relocation)
{
    PE_StoreU8(slot+3u,1u);PE_StoreU8(slot+0x17u,255u);PE_StoreU8(slot+0x16u,255u);
    PE_StoreU8(slot+0x19u,7u);PE_StoreU32(slot+0x94u,0x10000u);PE_StoreU16(slot+0x9Cu,16u);
    pe_addr_t actor=PE_LoadU32(slot+8u);
    PE_StoreU32(slot+0x10u,0u);PE_StoreU16(slot+0x14u,0u);PE_StoreU8(slot+0x1Au,0u);
    PE_StoreU32(slot+0x7Cu,0u);PE_StoreU32(slot+0x5Cu,0u);PE_StoreU32(slot+0x60u,0u);PE_StoreU32(slot+0x64u,0u);
    PE_StoreU16(slot+0x9Eu,0u);PE_StoreU32(slot+0xCu,0x80191894u+relocation);PE_StoreU32(slot+0x80u,0u);PE_StoreU16(slot+0xA0u,0u);
    static const unsigned offsets[]={0x68,0x6C,0x70,0x88,0x8C,0x90,0x78,0x7C,0x80};
    for(unsigned i=0;i<9;i++)PE_StoreU32(actor+offsets[i],0u);
    return 0;
}
static void movement_find(pe_addr_t output,uint32_t type,uint32_t id)
{
    pe_addr_t actor=PE_LoadU32(0x8009D20Cu);PE_StoreU32(output,actor);
    while(actor) {
        if(PE_LoadU8(actor+12u)==type && PE_LoadU8(actor+13u)==id && !(PE_LoadU32(actor+0x98u)&0x10u))break;
        actor=PE_LoadU32(actor+4u);PE_StoreU32(output,actor);
    }
}
static int movement_command(pe_addr_t slot,uint32_t mode,uint32_t command,uint32_t a,uint32_t b,uint32_t c,uint32_t relocation)
{
    pe_addr_t p=slot+12u;
    switch(command) {
    case 0:movement_find(p+0x70u,a,b);break;
    case 6:PE_StoreU32(p+0x50u,a);PE_StoreU32(p+0x54u,b);PE_StoreU32(p+0x58u,c);break;
    case 10:
        PE_StoreU8(p+10u,(uint8_t)a);PE_StoreU16(p+8u,(uint16_t)b);
        if(PE_LoadU8(slot+3u)==1u)PE_StoreU32(p,0x80191824u+relocation);
        break;
    case 11:
        if(PE_LoadU8(slot+3u)==1u){PE_StoreU8(p+11u,(uint8_t)a);PE_StoreU8(p+12u,(uint8_t)b);}break;
    case 13:PE_StoreU32(p+0x88u,a);break;
    case 15:PE_StoreU16(p+0x90u,(uint16_t)a);break;
    case 16:PE_StoreU16(p+0x92u,(uint16_t)a);break;
    case 17:
        PE_StoreU32(p+0x30u,a);PE_StoreU32(p+0x38u,c);
        PE_StoreU32(p+0x34u,b==0xFFFFFFFFu?PE_LoadU32(PE_LoadU32(0x8009D254u)+0x2Cu):b);
        PE_StoreU32(p+0x70u,0u);break;
    case 18:
        if(PE_LoadU8(slot+3u)==2u){PE_StoreU32(p,0x80191D18u+relocation);PE_StoreU32(p+0x7Cu,a);PE_StoreU8(slot+3u,3u);}break;
    case 19:
        if(!mode)PE_StoreU8(slot+3u,(uint8_t)a);else PE_StoreU32(a,PE_LoadU8(slot+3u));break;
    case 21:movement_find(p+0x74u,a,b);PE_StoreU16(p+0x94u,(uint16_t)c);break;
    case 22:PE_StoreU32(p+0x60u,a);PE_StoreU32(p+0x68u,b);PE_StoreU16(p+0x94u,(uint16_t)c);break;
    case 23:PE_StoreU8(p+13u,(a!=0u)|((b!=0u)<<1u)|((c!=0u)<<2u));break;
    case 25:if(mode==1u){PE_StoreU32(p+4u,a);PE_StoreU32(a,1u);}break;
    default:break;
    }
    return 0;
}
/* Original DFB20 animation repeats, DFB78 contact test, DFF80 heading, DFFB8 turn. */
static void movement_animation(pe_addr_t slot)
{
    pe_addr_t actor=PE_LoadU32(slot+8u);
    if(!(int16_t)PE_LoadU16(slot+0x14u) || PE_LoadU16(actor+0x1Au)<PE_LoadU8(actor+15u))return;
    PE_StoreU32(actor+0x14u,(uint32_t)(int32_t)(int8_t)PE_LoadU8(slot+0x18u));
    int count=(int16_t)PE_LoadU16(slot+0x14u);
    if(count!=-1)PE_StoreU16(slot+0x14u,(uint16_t)(count-1));
}
static int movement_contact(pe_addr_t slot)
{
    pe_addr_t actor=PE_LoadU32(slot+8u);
    if((PE_LoadU8(slot+0x19u)&4u) && PE_LoadU8(actor+14u)<2u)return 1;
    uint32_t flags=PE_LoadU32(actor+0x98u);
    if((PE_LoadU8(slot+0x19u)&2u) && (flags&0x80000u)){PE_StoreU32(actor+0x98u,flags&0xFFF7FFFFu);return 1;}
    if((PE_LoadU8(slot+0x19u)&1u) && (flags&0x40000u)){PE_StoreU32(actor+0x98u,flags&0xFFFBFFFFu);return 1;}
    pe_addr_t body=PE_LoadU32(actor);
    return body && (PE_LoadU32(body)&0x180Eu)?1:0;
}
static int movement_heading(uint32_t x0,uint32_t z0,uint32_t x1,uint32_t z1)
{
    return (int16_t)func_80079FB4((int32_t)(x1-x0),(int32_t)(z1-z0));
}
static uint32_t movement_turn(uint32_t from,uint32_t to,uint32_t speed)
{
    from&=4095u;to&=4095u;
    int delta=(int16_t)(to-from),distance=delta<0?-delta:delta;
    if((int16_t)speed>=distance)return to;
    uint32_t step=delta<0?0u-speed:speed;
    if(distance>2048)step=0u-step;
    return (from+step)&4095u;
}
int PE_M28MovementCleanup(pe_addr_t slot)
{
    if((int16_t)PE_LoadU16(slot+0x14u)) {
        pe_addr_t body=PE_LoadU32(PE_LoadU32(slot+8u));
        if(body)PE_StoreU8(PE_LoadU32(body+0x18u),4u);
    }
    PE_StoreU8(slot+3u,0u);PE_StoreU8(slot,4u);
    pe_addr_t output=PE_LoadU32(slot+0x10u);if(output)PE_StoreU32(output,0u);
    return 0;
}
static int movement_div(uint32_t value,int32_t divisor,uint32_t *result)
{
    if(!divisor || (value==0x80000000u && divisor==-1)) {
        Bootstrap_ReturnVoid("M28_movement_divide_trap","original movement effect");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
    }
    *result=(uint32_t)((int32_t)value/divisor);return 1;
}
static void movement_target(pe_addr_t slot,pe_addr_t actor,int second)
{
    uint32_t target[3];pe_addr_t follow=PE_LoadU32(slot+0x7Cu);
    for(unsigned i=0;i<3;i++)target[i]=PE_LoadU32(follow?follow+0x28u+i*4u:slot+0x3Cu+i*4u);
    if(!second && !follow && target[1]==0xFFFFFFFFu)target[1]=PE_LoadU32(PE_LoadU32(0x8009D254u)+0x2Cu);
    unsigned angle=(unsigned)movement_heading(PE_LoadU32(actor+0x28u),PE_LoadU32(actor+0x30u),target[0],target[2])&4095u;
    int16_t sin=(int16_t)PE_LoadU16(0x800966ECu+angle*4u),cos=(int16_t)PE_LoadU16(0x800966EEu+angle*4u);
    const int16_t matrix[3][3]={{cos,0,sin},{0,4096,0},{(int16_t)-sin,0,cos}};
    for(unsigned row=0;row<3;row++)for(unsigned col=0;col<3;col++)g_pe_gte.rt[row][col]=matrix[row][col];
    PE_GTE_SetV0((int16_t)((int32_t)PE_LoadU32(slot+0x5Cu)>>12),0,(int16_t)((int32_t)PE_LoadU32(slot+0x64u)>>12));
    PE_GTE_MVMVA(0x0486012u);
    for(unsigned i=0;i<3;i++)PE_StoreU32(slot+0x4Cu+i*4u,target[i]+((uint32_t)g_pe_gte.mac[i]<<12));
}
static void movement_step(pe_addr_t slot,int second,uint32_t relocation)
{
    pe_addr_t actor=PE_LoadU32(slot+8u),p=slot+12u;
    if(!PE_LoadU8(slot+0x1Au)) {
        PE_StoreU8(slot+0x1Au,1u);PE_StoreU8(slot+0xA2u,0u);
        if(!second) {
            PE_StoreU32(actor+0x98u,(PE_LoadU32(actor+0x98u)&0xFFF3FFFFu)|2u);
            if((int16_t)PE_LoadU16(slot+0x14u)) {
                pe_addr_t body=PE_LoadU32(actor);if(body)PE_StoreU32(body,PE_LoadU32(body)|0x40000000u);
            }
            uint32_t velocity=PE_LoadU32(p+0x88u)*(uint32_t)((int16_t)PE_LoadU16(p+0x90u)-1);
            PE_StoreU32(p+0x7Cu,(uint32_t)((int32_t)velocity>>1));
        }
        movement_target(slot,actor,second);
        int32_t duration=(int16_t)PE_LoadU16(p+0x90u);
        if(second) {
            uint32_t y=PE_LoadU32(actor+0x2Cu),v=PE_LoadU32(p+0x7Cu),g=PE_LoadU32(p+0x88u);duration=0;
            do {y+=v;v+=g;duration=(int32_t)((uint32_t)duration+1u);}
            while((int32_t)y<(int32_t)PE_LoadU32(p+0x44u));
        }
        uint32_t value;
        if(!movement_div(PE_LoadU32(p+0x40u)-PE_LoadU32(actor+0x28u),duration,&value))return;
        PE_StoreU32(p+0x80u,value);
        if(!movement_div(PE_LoadU32(p+0x48u)-PE_LoadU32(actor+0x30u),duration,&value))return;
        PE_StoreU32(p+0x84u,value);
    }
    if(!PE_LoadU8(p+0x96u) && movement_contact(slot)==1) {
        pe_addr_t output=PE_LoadU32(p+4u);PE_StoreU8(p+0x96u,1u);
        PE_StoreU32(p+0x80u,0u);PE_StoreU32(p+0x84u,0u);if(output)PE_StoreU32(output,2u);
    }
    if(PE_LoadU8(actor+14u)<2u)return;
    PE_StoreU32(actor+0x28u,PE_LoadU32(actor+0x28u)+PE_LoadU32(p+0x80u));
    PE_StoreU32(actor+0x2Cu,second?PE_LoadU32(actor+0x2Cu)+PE_LoadU32(p+0x7Cu):PE_LoadU32(actor+0x2Cu)-PE_LoadU32(p+0x7Cu));
    PE_StoreU32(actor+0x30u,PE_LoadU32(actor+0x30u)+PE_LoadU32(p+0x84u));
    uint32_t old=PE_LoadU32(p+0x7Cu),g=PE_LoadU32(p+0x88u),v=second?old+g:old-g;
    PE_StoreU32(p+0x7Cu,v);int landed=0;
    if(second) {
        if((int16_t)PE_LoadU16(actor+0x2Eu)>=(int16_t)PE_LoadU16(p+0x46u)) {
            landed=1;PE_StoreU32(actor+0x2Cu,PE_LoadU32(p+0x44u));
        }
    } else {
        if((int16_t)PE_LoadU16(p+0x92u) && (int32_t)(old^v)<0) {
            PE_StoreU32(p,0x80191D10u+relocation);PE_StoreU8(p+14u,0u);
            PE_StoreU32(p+0x50u,0u);PE_StoreU32(p+0x54u,0u);PE_StoreU32(p+0x58u,0u);PE_StoreU8(slot+3u,2u);
        }
        if((int32_t)PE_LoadU32(p+0x7Cu)<0 && (int32_t)(PE_LoadU32(actor+0x2Cu)-PE_LoadU32(p+0x44u))>=0) {
            PE_StoreU32(actor+0x2Cu,PE_LoadU32(p+0x44u));landed=1;
        }
        if((int16_t)PE_LoadU16(p+0x94u)>0) {
            pe_addr_t follow=PE_LoadU32(p+0x74u);
            if(follow){PE_StoreU32(p+0x60u,PE_LoadU32(follow+0x28u));PE_StoreU32(p+0x68u,PE_LoadU32(follow+0x30u));}
            int heading=movement_heading(PE_LoadU32(p+0x60u),PE_LoadU32(p+0x68u),PE_LoadU32(actor+0x28u),PE_LoadU32(actor+0x30u));
            PE_StoreU16(actor+0x3Au,(uint16_t)movement_turn(PE_LoadU16(actor+0x3Au),(uint32_t)heading,PE_LoadU16(p+0x94u)));
        }
    }
    movement_animation(slot);if(landed)PE_M28MovementCleanup(slot);
}
static int movement_update(pe_addr_t slot,uint32_t relocation)
{
    pe_addr_t callback=PE_LoadU32(slot+12u),actor=PE_LoadU32(slot+8u);
    if(callback==0x80191824u+relocation) {
        int animation=(int8_t)PE_LoadU8(slot+0x16u),frame=(int8_t)PE_LoadU8(slot+0x17u);
        if(animation>=0 && animation!=PE_LoadU8(actor+14u))return 0;
        if(frame>=0 && (frame<PE_LoadU16(actor+0x1Au) || PE_LoadU16(actor+0x16u)<frame))return 0;
        PE_StoreU32(slot+12u,0x80191894u+relocation);
    } else if(callback==0x80191894u+relocation)movement_step(slot,0,relocation);
    else if(callback==0x80191D18u+relocation)movement_step(slot,1,relocation);
    else if(callback!=0x80191D10u+relocation) {
        Bootstrap_ReturnVoid("M28_movement_callback","original movement effect");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    }
    return 0;
}

/* Original M0032I moves this entire 764-word graph by -16 bytes. Only
 * 21 internal branches and five stored callback addresses differ. */
int PE_M32MovementOverlay(void)
{
    return PE_LoadU32(0x8019150Cu)==0x24020001u && PE_LoadU32(0x8019159Cu)==0x2CC2001Au &&
        PE_LoadU32(0x80191884u)==0x27BDFFD8u && PE_LoadU32(0x801920C8u)==0xA0800003u;
}
int PE_M28MovementInit(pe_addr_t slot) {return movement_init(slot,0u);}
int PE_M32MovementInit(pe_addr_t slot) {return movement_init(slot,0xFFFFFFF0u);}
int PE_M28MovementCommand(pe_addr_t slot,uint32_t mode,uint32_t command,uint32_t a,uint32_t b,uint32_t c)
{return movement_command(slot,mode,command,a,b,c,0u);}
int PE_M32MovementCommand(pe_addr_t slot,uint32_t mode,uint32_t command,uint32_t a,uint32_t b,uint32_t c)
{return movement_command(slot,mode,command,a,b,c,0xFFFFFFF0u);}
int PE_M28MovementUpdate(pe_addr_t slot) {return movement_update(slot,0u);}
int PE_M32MovementUpdate(pe_addr_t slot) {return movement_update(slot,0xFFFFFFF0u);}
