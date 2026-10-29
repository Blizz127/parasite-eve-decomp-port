/* Original M0023I effects. Native callbacks, not matching C. */
#include "psx_compat.h"
#include <stdlib.h>
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

static int16_t lh(pe_addr_t a){return (int16_t)PE_LoadU16(a);}
static pe_addr_t owner(void){return PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u);}
static pe_addr_t pool(void){return PE_LoadU32(PE_LoadU32(0x800F33E0u)+8u);}
static uint32_t clut(unsigned x)
{
    unsigned type=PE_LoadU16(0x800F336Cu),y=PE_LoadU16(0x800E1204u+type*2u);
    if(type==4 && PE_LoadU32(0x800F3428u)) y+=4;
    return func_80077AA4(x,y)&65535u;
}

int PE_M0023I_Particle(int32_t mode,pe_addr_t p)
{
    int state=lh(p+6u);int32_t time=(int32_t)PE_LoadU32(0x800E27ECu);
    if(mode==1) {
        if(state!=0 && state!=1) return 0;
        if(!state) {
            for(unsigned i=0;i<3;i++) PE_StoreU16(p+i*2u,(uint16_t)(PE_LoadU16(p+i*2u)+PE_LoadU16(p+8u+i*2u)));
            if(time>=4) return 1;
        }
        for(unsigned i=0;i<3;i++) PE_StoreU16(p+i*2u,(uint16_t)(PE_LoadU16(p+i*2u)+PE_LoadU16(p+8u+i*2u)));
        return time>=8;
    }
    if(mode==2) {
        PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
        if(state==0) {
            int16_t position[3],angles[4];uint8_t end_color[3]={128,128,128};
            for(unsigned i=0;i<3;i++){position[i]=lh(0x80190758u+i*2u);angles[i]=lh(p+i*2u);}
            angles[3]=1;
            PE_EffectRibbonD2370(position,angles,960,100,0,(int32_t)(((uint32_t)time-1u)<<4u),
                255,15,clut(0),0,end_color,190,1);
            int32_t texture=(int32_t)(96u+(uint32_t)(int32_t)lh(0x800F336Au)*2u*((uint32_t)time-1u));
            PE_EffectSpriteValuesCEE20(position,angles,4096,4096,texture,clut(32),3,128,0);
        } else if(state==1) {
            int16_t angles[4]={0,0,(int16_t)((uint32_t)time<<6u),0};
            int32_t brightness=func_80077DC4((int32_t)(((uint32_t)time-1u)<<7u))/32;
            int32_t texture=(int32_t)(64u+(uint32_t)(int32_t)lh(0x800F336Au)*(uint32_t)time);
            PE_EffectSpriteCEE20(p,angles,4096,4096,texture,clut(16),1,brightness,0);
        }
    }
    return 0;
}

int PE_M0023I_Main(int32_t mode,pe_addr_t data)
{
    int16_t local[3];for(unsigned i=0;i<3;i++)local[i]=lh(0x8018EFF4u+i*2u);
    if(mode==0) {
        PE_StoreU16(data+8u,0);PE_StoreU16(data+10u,0);PE_StoreU16(data+12u,0);
        int kind=lh(PE_LoadU32(0x800E2368u)+18u);
        if(kind==0)PE_StoreU16(data+14u,8);
        else if(kind==1)PE_StoreU16(data+14u,16);
        return (int)func_800CE560(pool(),16,12,0x8018F004u);
    }
    if(mode==1) {
        int16_t position[3];
        PE_ActorJointPositionCE8F0(owner(),(uint32_t)(int32_t)lh(data+14u),local,position);
        int16_t timer=(int16_t)(PE_LoadU16(data+12u)+1u);
        PE_StoreU16(data+12u,(uint16_t)timer);
        int interval=lh(PE_LoadU32(0x800E2368u)+20u);
        if(!interval)return 1;
        if(timer<=interval)return 0;
        PE_StoreU16(data+12u,0);
        pe_addr_t matrix=PE_LoadU32(owner()+0x238u);
        for(unsigned i=0;i<3;i++)position[i]=(int16_t)PE_LoadU32(matrix+20u+i*4u);
        pe_addr_t p=func_800CE610(pool());if(!p)return 0;
        if(func_80071A54()&1u) {
            PE_StoreU16(p,(uint16_t)((func_80071A54()&2047u)-1024u));
            PE_StoreU16(p+2u,(uint16_t)func_80071A54());
            PE_StoreU16(p+4u,0);PE_StoreU16(p+6u,0);
        } else {
            for(unsigned i=0;i<3;i++)PE_StoreU16(p+i*2u,(uint16_t)position[i]);
            PE_StoreU16(p+6u,1);
        }
        for(unsigned i=0;i<3;i++)PE_StoreU16(p+8u+i*2u,(uint16_t)((int32_t)func_80071A54()%70-35));
    } else if(mode==2) {
        int16_t position[3];
        PE_ActorJointPositionCE8F0(owner(),(uint32_t)(int32_t)lh(data+14u),local,position);
        for(unsigned i=0;i<3;i++)PE_StoreU16(0x80190758u+i*2u,(uint16_t)position[i]);
        PE_StoreU16(0x800F3368u,32);PE_StoreU16(0x800F336Au,2);
        PE_StoreU16(0x800F3376u,32);PE_StoreU16(0x800F3378u,32);
        PE_StoreU16(0x800F336Cu,3);PE_StoreU16(0x800F336Eu,0);
        PE_StoreU16(0x800F3372u,0);PE_StoreU16(0x800F3374u,12);
        PE_StoreU16(0x800F3370u,PE_LoadU16(0x800E2850u+PE_LoadU16(0x800E11EAu)*2u));
    }
    return 0;
}

/* F710 borrows six bytes from the original caller stack before CE8F0
 * overwrites them. Keep that input/output explicit: it is shared between
 * successive callbacks, not an actor position or an initialized local. */
/* 1 = keep the unresolved caller-stack read as an explicit boundary that
 * stops the port (pre-Day-1 behaviour; tests use it).  0 = Day-1 shim. */
int g_pe_strict_effect_stack = 0;
/* PE_STRICT_BOUNDARY=1: start in strict mode (unported effect callbacks stop
 * the port instead of the logged Day-1 skip). */
__attribute__((constructor)) static void pe_strict_boundary_env(void)
{
    const char *e = getenv("PE_STRICT_BOUNDARY");
    if (e && e[0] == '1') g_pe_strict_effect_stack = 1;
}

/* Day-1 shim diagnostics; nothing reads them back.  Each distinct effect-stack
 * state that leaves F710's borrowed bytes unknown is logged once, and every
 * hit is counted (summary at exit), so a run names the callback whose stack
 * writes must be bound before the shim can go. */
enum { FLASH_SHIM_CAUSES=16 };
static struct { PeEffectStackCause cause; unsigned hits; } g_flash_shim[FLASH_SHIM_CAUSES];
static unsigned g_flash_shim_causes,g_flash_shim_untracked,g_flash_shim_total;
static PeEffectStackCause g_flash_shim_last;
static int flash_shim_same(const PeEffectStackCause *a,const PeEffectStackCause *b)
{
    return a->active==b->active && a->context==b->context && a->known==b->known &&
        a->reset_fn==b->reset_fn && a->reset_context==b->reset_context;
}
static void flash_shim_summary(void)
{
    char text[160];
    for(unsigned i=0;i<g_flash_shim_causes;i++) {
        PE_EffectStackDescribe(&g_flash_shim[i].cause,text,sizeof text);
        fprintf(stderr,"[DAY1_SHIM] PE_M0023I_Flash: %u hit(s): %s\n",g_flash_shim[i].hits,text);
    }
    if(g_flash_shim_untracked)
        fprintf(stderr,"[DAY1_SHIM] PE_M0023I_Flash: %u hit(s) with further causes\n",g_flash_shim_untracked);
}
static void flash_shim_note(void)
{
    PeEffectStackCause cause;char text[160];unsigned i;
    PE_EffectStackCause(&cause);g_flash_shim_last=cause;g_flash_shim_total++;
    for(i=0;i<g_flash_shim_causes;i++)
        if(flash_shim_same(&g_flash_shim[i].cause,&cause)) {g_flash_shim[i].hits++;return;}
    if(g_flash_shim_causes==FLASH_SHIM_CAUSES) {g_flash_shim_untracked++;return;}
    if(!g_flash_shim_causes)atexit(flash_shim_summary);
    g_flash_shim[g_flash_shim_causes].cause=cause;g_flash_shim[g_flash_shim_causes++].hits=1;
    PE_EffectStackDescribe(&cause,text,sizeof text);
    fprintf(stderr,"[DAY1_SHIM] PE_M0023I_Flash: F710 caller-stack ring position unresolved (%s); "
        "using owner joint position\n",text);
}
unsigned PE_M0023I_FlashShimLast(PeEffectStackCause *cause)
{
    if(cause)*cause=g_flash_shim_last;
    return g_flash_shim_total;
}

/* borrowed: F710's six caller-stack bytes (NULL: not tracked on this path).
 * known: which of X(1)/Y(2)/Z(4) hold the original bytes.  State 0 reads them
 * as the ring position.  Every mode-2 call ends by writing the joint position
 * over all six (pe_m0023i_flash_oracle.py hashes 0x801FEFB8 after the
 * original call), so a tracked caller knows all six afterwards. */
static int flash(int32_t mode,pe_addr_t data,int16_t borrowed[3],unsigned known)
{
    int16_t local[3];uint8_t ring_color[3],fan_color[3];
    for(unsigned i=0;i<3;i++) {
        local[i]=lh(0x8018EFF4u+i*2u);
        ring_color[i]=PE_LoadU8(0x8018EFFCu+i);fan_color[i]=PE_LoadU8(0x8018F000u+i);
    }
    if(mode==0) {
        int kind=lh(PE_LoadU32(0x800E2368u)+18u);
        if(kind==0) {
            PE_StoreU16(data,8);
            int32_t group=func_800D3FD8();
            (void)func_800D3F64(0x5AEu,(uint32_t)group);(void)func_800D3F64(0x5AFu,128);
        } else if(kind==1)PE_StoreU16(data,16);
        PE_StoreU16(data+4u,0);PE_StoreU16(data+2u,0);
    } else if(mode==1) {
        PE_StoreU16(data+2u,(uint16_t)(PE_LoadU16(data+2u)+1u));
        return lh(data+4u)>=3;
    } else if(mode==2) {
        int state=lh(data+4u),timer=lh(data+2u);int32_t scale=0,brightness=0;
        if(state==0) {
            const int16_t *ring=borrowed;int16_t fallback[3];
            if((!borrowed || known!=7u) && g_pe_strict_effect_stack) {
                Bootstrap_ReturnVoid("PE_M0023I_Flash","original F710 caller stack position is unresolved");
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
            }
            if(!borrowed || known!=7u) {
                /* DAY1 SHIM (docs/ai_context/DAY1_FIDELITY_GAPS.md,
                 * "M0023I F710 flash ring position"): the model has lost the
                 * borrowed bytes (an unbound callback ran at this depth, the
                 * RAM generation changed, or this path is not tracked).
                 * Retail reads whatever their last writer left.  Substitute
                 * the current joint position instead of stopping the game.
                 * That is not the original value; the log names the cause. */
                flash_shim_note();
                PE_ActorJointPositionCE8F0(owner(),(uint32_t)(int32_t)lh(data),local,fallback);
                ring=fallback;
            }
            PE_EffectRingD0728(ring,600,800,32,0,timer,timer,ring_color,0,
                func_80077CF4(timer*128)/32,1);
            scale=timer*256;brightness=80;
            if(timer>=8) {PE_StoreU16(data+4u,1);PE_StoreU16(data+2u,0);}
        } else if(state==1) {
            scale=2048+timer*64;brightness=80+timer*48/32;
            if(timer>=32) {PE_StoreU16(data+4u,2);PE_StoreU16(data+2u,0);}
        } else if(state==2) {
            scale=4096+func_80077DC4(timer*128)/2;brightness=160;
            if(timer>=8) {PE_StoreU16(data+4u,3);PE_StoreU16(data+2u,0);}
        }
        PE_StoreU16(0x800F3368u,64);PE_StoreU16(0x800F3376u,64);PE_StoreU16(0x800F3378u,64);
        PE_StoreU16(0x800F336Au,4);PE_StoreU16(0x800F336Cu,3);PE_StoreU16(0x800F336Eu,0);
        PE_StoreU16(0x800F3372u,0);PE_StoreU16(0x800F3374u,24);
        PE_StoreU16(0x800F3370u,PE_LoadU16(0x800E2850u+PE_LoadU16(0x800E11EAu)*2u));
        uint32_t time=PE_LoadU32(0x800E27ECu);
        if(time&1u)scale=(int32_t)((uint32_t)scale*5u)/4;
        int16_t position[3],angles[4]={0,0,(int16_t)((0u-time)<<6u),0};
        PE_ActorJointPositionCE8F0(owner(),(uint32_t)(int32_t)lh(data),local,position);
        if(borrowed)for(unsigned i=0;i<3;i++)borrowed[i]=position[i];
        int32_t sprite_scale=(int32_t)((uint32_t)scale*3u)/2;
        PE_EffectSpriteValuesCEE20(position,angles,sprite_scale,sprite_scale,128,clut(48),1,
            brightness/(int32_t)(1u+(time&1u)),0);
        PE_EffectSpriteValuesCEE20(position,angles,sprite_scale,sprite_scale,132,clut(64),1,brightness,0);
        PE_EffectFanD004C(position,600,600,12,0,scale,scale,fan_color,0,64,1);
    }
    return 0;
}
int PE_M0023I_Flash(int32_t mode,pe_addr_t data,int16_t retained_position[3])
{
    return flash(mode,data,retained_position,retained_position?7u:0u);
}
int PE_M0023I_FlashTracked(pe_addr_t data,int16_t borrowed[3],unsigned known)
{
    return flash(2,data,borrowed,known);
}

/* SDK 785D4: left rotation becomes the GTE rotation; each right column
 * is multiplied with signed IR saturation. Translation is left untouched. */
static void beam_rotate(PeEffectMatrix *matrix,const int16_t right[9])
{
    int16_t result[9];
    for(unsigned i=0;i<9;i++)g_pe_gte.rt[i/3u][i%3u]=matrix->r[i];
    for(unsigned col=0;col<3;col++) {
        PE_GTE_SetV0(right[col],right[3u+col],right[6u+col]);
        PE_GTE_MVMVA(0x86012u);
        for(unsigned row=0;row<3;row++)result[row*3u+col]=(int16_t)g_pe_gte.ir[row];
    }
    for(unsigned i=0;i<9;i++)matrix->r[i]=result[i];
    matrix->pad=(int16_t)((uint32_t)g_pe_gte.ir[2]>>16u);
}

static void beam_scale(PeEffectMatrix *matrix,int32_t x,int32_t y,int32_t z)
{
    const int32_t scale[3]={x,y,z};
    for(unsigned i=0;i<9;i++) {
        int32_t value=(int32_t)((uint32_t)(int32_t)matrix->r[i]*(uint32_t)scale[i%3u])>>12;
        matrix->r[i]=(int16_t)value;
        if(i==8)matrix->pad=(int16_t)((uint32_t)value>>16u);
    }
}

int PE_M0023I_Beam(int32_t mode,pe_addr_t data)
{
    int16_t local[3];
    for(unsigned i=0;i<3;i++)local[i]=lh(0x8018EFF4u+i*2u);
    pe_addr_t descriptor=PE_LoadU32(0x800E2368u);
    if(mode==0) {
        int kind=lh(descriptor+18u);
        if(kind==0)PE_StoreU32(data+32u,8);
        else if(kind==1)PE_StoreU32(data+32u,16);
        PE_StoreU16(data+40u,0);PE_StoreU16(data+42u,0);
        PE_StoreU32(data+24u,0);PE_StoreU16(data+44u,0);
        if(PE_LoadU8(descriptor+13u)) {
            pe_addr_t actor=owner(),record=actor?PE_LoadU32(actor):0;
            if(record) {
                pe_addr_t action=PE_LoadU32(record+24u);
                if(PE_LoadU8(action)==1)PE_StoreU8(action,2);
            }
        }
        PE_StoreU32(data+36u,1);
        pe_addr_t mesh=func_8006E498(PE_LoadU32(0x800B0E64u),0xC5463704u);
        PE_StoreU32(0x80190760u,mesh);func_800C6D5C(mesh,0,0);
    } else if(mode==1) {
        int16_t position[3],direction[3],offset[3];
        PE_ActorJointPositionCE8F0(owner(),PE_LoadU32(data+32u),local,position);
        for(unsigned i=0;i<3;i++)PE_StoreU16(data+i*2u,(uint16_t)position[i]);
        PE_ActorJointDirectionCE9D4(owner(),0,direction);
        for(unsigned i=0;i<3;i++)PE_StoreU16(data+16u+i*2u,(uint16_t)direction[i]);
        int state=lh(data+40u);
        if(state>=0 && state<=3) {
            int16_t timer=(int16_t)(PE_LoadU16(data+42u)+1u);
            PE_StoreU16(data+42u,(uint16_t)timer);
            if(state==0) {
                PE_StoreU32(data+28u,1024);PE_StoreU32(data+24u,PE_LoadU32(data+24u)+410u);
                PE_StoreU16(data+44u,4096);PE_StoreU16(data+46u,(uint16_t)((int32_t)timer*8/5));
            } else if(state==1) {
                PE_StoreU32(data+28u,(uint32_t)(func_80077DC4((int32_t)timer*128)+4096));
                PE_StoreU16(data+44u,(uint16_t)(func_80077DC4((int32_t)timer*128)+4096));
                PE_StoreU16(data+46u,(uint16_t)(timer+8));
            } else if(state==2) {
                PE_StoreU16(data+44u,4096);PE_StoreU16(data+46u,16);
                PE_StoreU32(data+28u,4096u+((PE_LoadU32(0x800E27ECu)&1u)<<8u));
            } else {
                PE_StoreU16(data+44u,4096);PE_StoreU32(data+28u,4096u-(uint32_t)((int32_t)timer*256));
                PE_StoreU16(data+46u,(uint16_t)(timer+16));
                if(timer>=16)return 1;
            }
            const int limits[3]={5,8,32};
            if(state<3 && timer>=limits[state]) {
                PE_StoreU16(data+40u,(uint16_t)(state+1));PE_StoreU16(data+42u,0);
            }
        }
        PE_EffectOffsetCFB7C(data+16u,lh(data+24u),offset);
        for(unsigned i=0;i<3;i++)PE_StoreU16(data+8u+i*2u,(uint16_t)(PE_LoadU16(data+i*2u)+offset[i]));
        if(func_800CEB8C(data,data+8u,80) && PE_LoadU32(data+36u)) {
            if(PE_LoadU8(descriptor+13u)) {
                pe_addr_t record=PE_LoadU32(owner());
                if((PE_LoadU32(record)&0x3F000000u)==0x01000000u) {
                    pe_addr_t aya=PE_LoadU32(PE_LoadU32(0x8009D254u));
                    PE_StoreU32(aya+76u,PE_LoadU32(aya+76u)|0x4000u);
                    PE_StoreU32(record,(PE_LoadU32(record)&0xC0FFFFFFu)|0x91000000u);
                }
            }
            PE_StoreU32(data+36u,0);
        }
    } else if(mode==2) {
        uint32_t time=PE_LoadU32(0x800E27ECu);
        PE_StoreU16(0x800F3374u,24);PE_StoreU16(0x800F3368u,64);PE_StoreU16(0x800F336Au,4);
        PE_StoreU16(0x800F3376u,64);PE_StoreU16(0x800F3378u,64);
        PE_StoreU16(0x800F336Cu,3);PE_StoreU16(0x800F336Eu,0);PE_StoreU16(0x800F3372u,0);
        PE_StoreU16(0x800F3370u,PE_LoadU16(0x800E2850u+PE_LoadU16(0x800E11EAu)*2u));
        if(lh(data+44u)) {
            int16_t angles[4]={0,0,(int16_t)(time*96u),0},position[3];uint8_t color[3];
            int32_t scale=lh(data+44u)*2/3;
            if(time&1u)scale=scale*9/8;
            PE_EffectColorCF3AC(0x801906F0u,color,lh(data+46u));
            PE_EffectSpriteCEE20(data,angles,scale,scale,128,clut(48),1,128,color);
            for(unsigned i=0;i<3;i++)position[i]=lh(data+i*2u);
            PE_EffectFanD004C(position,400,400,12,0,scale,scale,color,0,64,1);
            PE_EffectFanD004C(position,190,190,8,0,scale,scale,color,0,128,1);
            position[1]=lh(0x800942ECu);angles[0]=1024;angles[3]=1;
            PE_EffectSpriteValuesCEE20(position,angles,scale,scale,128,clut(48),1,64,color);
        }
        int16_t angles[3]={0,0,(int16_t)(time<<7u)},rotation[9];PeEffectMatrix matrix;
        PE_RotMatrix794C4_values(angles,rotation);
        PE_EffectReadMatrix(PE_LoadU32(owner()+0x238u),&matrix);beam_rotate(&matrix,rotation);
        int32_t scale=(int32_t)PE_LoadU32(data+28u)/16;
        beam_scale(&matrix,scale,scale,(int32_t)(PE_LoadU32(data+24u)<<12u)/8000);
        for(unsigned i=0;i<3;i++)matrix.t[i]=lh(data+i*2u);
        PE_StoreU16(0x800F336Cu,3);PE_StoreU16(0x800F336Eu,0);
        PE_StoreU16(0x800F3370u,PE_LoadU16(0x800E2850u+PE_LoadU16(0x800E11EAu)*2u));
        PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
        func_800C6EC0(PE_LoadU16(0x800F3370u)|func_80077A64(0,1,0,0),clut(80));func_800C6ED8(1);
        pe_addr_t mesh=PE_LoadU32(0x80190760u);
        func_800C6EF8(mesh);PE_EffectMeshC71E4(mesh,&matrix);func_800C6F4C(mesh);
        func_800C6EF8(mesh);beam_scale(&matrix,6144,6144,4096);func_800C70EC(mesh,-255,-140,-255);
        PE_EffectMeshC71E4(mesh,&matrix);func_800C6F4C(mesh);
        func_800C6EF8(mesh);matrix.t[1]=lh(0x800942ECu);beam_scale(&matrix,2048,2048,4096);
        func_800C6FA0(mesh,25);PE_EffectMeshC71E4(mesh,&matrix);func_800C6F4C(mesh);
    }
    return 0;
}
