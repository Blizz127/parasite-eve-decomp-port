/* M0013I corridor projectile: original overlay 8018F004..8018FC54.
 * Native translation, verified by pe_m0013i_effect_oracle.py. Transient
 * retail stack vectors live on the host stack; all persistent data is RAM. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

static int32_t mul32(int32_t a,int32_t b)
{ return (int32_t)((uint32_t)a*(uint32_t)b); }
static int16_t lh(pe_addr_t a) { return (int16_t)PE_LoadU16(a); }
static pe_addr_t owner(void) { return PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u); }
static pe_addr_t pool(void) { return PE_LoadU32(PE_LoadU32(0x800F33E0u)+8u); }

int32_t func_800D3F64(uint32_t id,uint32_t group)
{
    pe_addr_t matrix=PE_LoadU32(owner()+0x238u);
    return func_8006DCE4(id,group,(int16_t)PE_LoadU32(matrix+20u),
        (int16_t)PE_LoadU32(matrix+24u),(int16_t)PE_LoadU32(matrix+28u));
}

/* func_800C6B90: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C6B90_port.c (src/func_800C6B90.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static uint32_t clut(void)
{
    uint32_t type=PE_LoadU16(0x800F336Cu),y=PE_LoadU16(0x800E1204u+type*2u);
    if (type==4u && PE_LoadU32(0x800F3428u)) y+=4u;
    return func_80077AA4(64,y)&65535u;
}

static void texture_setup(int first)
{
    PE_StoreU16(0x800F3368u,16);PE_StoreU16(0x800F336Au,1);
    PE_StoreU16(0x800F3376u,16);PE_StoreU16(0x800F3378u,16);
    PE_StoreU16(0x800F336Cu,2);PE_StoreU16(0x800F336Eu,0);
    if (first) {PE_StoreU16(0x800F3372u,0);PE_StoreU16(0x800F3374u,8);}
    PE_StoreU16(0x800F3370u,PE_LoadU16(0x800E2850u+PE_LoadU16(0x800E11E8u)*2u));
}

int PE_M0013I_Particle(int32_t mode,pe_addr_t p)
{
    int32_t time=(int32_t)PE_LoadU32(0x800E27ECu);
    if (mode==1) {
        PE_StoreU16(p+2u,(uint16_t)(PE_LoadU16(p+2u)-2u));
        PE_StoreU16(p,(uint16_t)(PE_LoadU16(p)-3u+(func_80071A54()&7u)));
        PE_StoreU16(p+4u,(uint16_t)(PE_LoadU16(p+4u)-3u+(func_80071A54()&7u)));
        return time>=14;
    }
    if (mode==2) {
        int32_t angle=(int32_t)((uint32_t)time<<10u)/14;
        int32_t brightness=func_80077DC4(angle)/50;
        int32_t size=mul32(func_80077DC4(angle)/2+2048,(int32_t)PE_LoadU32(p+8u))/4096+1024;
        int32_t frame=(int32_t)((uint32_t)time<<3u)/14;
        PE_EffectSpriteCEE20(p,0,size,size,
            (int32_t)(0xE0u+(uint32_t)mul32(lh(0x800F336Au),frame)),clut(),1,brightness,0);
    }
    return 0;
}

static void emit_particle(pe_addr_t data)
{
    pe_addr_t p=func_800CE610(pool());unsigned i;
    if (!p) return;
    for (i=0;i<3;i++) PE_StoreU16(p+i*2u,PE_LoadU16(data+i*2u));
    PE_StoreU32(p+8u,(uint32_t)(int32_t)lh(data+16u));
}

static void finish_projectile(pe_addr_t data)
{
    int32_t sound=lh(data+12u);
    PE_StoreU16(data+8u,2);PE_StoreU16(data+10u,0);
    if (sound!=-1) func_800866A4((uint32_t)sound,0);
}

static int projectile_main(int32_t mode,pe_addr_t data,pe_addr_t extra,
                           pe_addr_t joint_vector,pe_addr_t angle_vector,pe_addr_t particle_callback)
{
    int16_t angles[4];unsigned i;
    for (i=0;i<4;i++) angles[i]=lh(angle_vector+i*2u);
    if (mode==0) {
        int32_t kind=lh(PE_LoadU32(0x800E2368u)+18u);
        for (i=0;i<3;i++) PE_StoreU16(data+i*2u,PE_LoadU16(owner()+0x268u+i*2u));
        PE_StoreU16(data+8u,0);PE_StoreU16(data+10u,0);PE_StoreU16(data+12u,65535);
        if (kind==0) {
            PE_StoreU16(data+14u,(uint16_t)(0u-PE_LoadU32(extra+4u)));
            PE_StoreU16(data+12u,(uint16_t)func_800D3F64(0x565u,(uint32_t)func_800D3FD8()));
        } else if (kind==1) PE_StoreU16(data+14u,0);
        else if (kind==2) PE_StoreU16(data+14u,(uint16_t)PE_LoadU32(extra+4u));
        return (int)func_800CE560(pool(),12,16,particle_callback);
    }
    if (mode==1) {
        int32_t state=lh(data+8u);
        PE_StoreU16(data+10u,(uint16_t)(PE_LoadU16(data+10u)+1u));
        if (state==0) {
            func_800CE8F0(owner(),PE_LoadU32(extra+8u)?0:25,joint_vector,data);
            PE_StoreU16(data+16u,(uint16_t)(func_80077CF4(lh(data+10u)*1024/52)*3/2));
            PE_StoreU16(data+18u,(uint16_t)(lh(data+16u)/48));
            if ((int32_t)PE_LoadU32(0x800E27ECu)%3==0) emit_particle(data);
            if (lh(data+10u)>=52) {
                pe_addr_t body=PE_LoadU32(0x800E2368u),actor,record;
                PE_StoreU16(data+8u,1);PE_StoreU16(data+10u,0);
                PE_ActorJointDirectionCE9D4(owner(),0,angles);
                PE_StoreU16(data+14u,(uint16_t)(PE_LoadU16(data+14u)+1024u-(uint16_t)angles[1]));
                if (PE_LoadU8(body+13u) && (actor=owner()) && (record=PE_LoadU32(actor))) {
                    pe_addr_t action=PE_LoadU32(record+24u);
                    if (action<0x200000u) action|=0x80000000u;
                    if (PE_LoadU8(action)==1) PE_StoreU8(action,2);
                }
            }
        } else if (state==1) {
            int32_t speed=(int32_t)PE_LoadU32(extra);
            PE_StoreU16(data,(uint16_t)(PE_LoadU16(data)+mul32(func_80077DC4(lh(data+14u)),speed)/4096));
            PE_StoreU16(data+4u,(uint16_t)(PE_LoadU16(data+4u)+mul32(func_80077CF4(lh(data+14u)),speed)/4096));
            PE_StoreU16(data+16u,4096);
            PE_StoreU16(data+18u,PE_LoadU32(0x800E27ECu)&1u?128:96);
            if (PE_LoadU32(0x800E27ECu)&1u) emit_particle(data);
            if (!func_8001CAB0((int32_t)((uint32_t)(int32_t)lh(data)<<16u),
                (int32_t)((uint32_t)(int32_t)lh(data+4u)<<16u),PE_LoadU32(0x8009D248u),PE_LoadU16(0x8009D1CCu)))
                finish_projectile(data);
            if (func_800C6B90(data,57)) {
                pe_addr_t body=PE_LoadU32(0x800E2368u);
                if (PE_LoadU8(body+13u)) {
                    pe_addr_t record=PE_LoadU32(owner());
                    if ((PE_LoadU32(record)&0x3F000000u)==0x01000000u) {
                        pe_addr_t player=PE_LoadU32(PE_LoadU32(0x8009D254u));
                        PE_StoreU32(player+76u,PE_LoadU32(player+76u)|0x4000u);
                        PE_StoreU32(record,(PE_LoadU32(record)&0xC0FFFFFFu)|0x11000000u);
                        record=PE_LoadU32(owner());
                        PE_StoreU32(record,PE_LoadU32(record)|0x80000000u);
                    }
                }
                finish_projectile(data);
            }
        } else if (state==2) {
            PE_StoreU16(data+2u,(uint16_t)(PE_LoadU16(data+2u)-1u));
            return lh(data+10u)>=16;
        }
    } else if (mode==2) {
        int32_t state=lh(data+8u),size,brightness,texture;
        int16_t floor[3]={lh(data),lh(0x800942ECu),lh(data+4u)};
        const int16_t floor_angles[4]={1024,0,0,1};
        texture_setup(1);
        if (state==0 || state==1) {
            size=lh(data+16u);brightness=lh(data+18u);
            angles[2]=(int16_t)(PE_LoadU32(0x800E27ECu)<<7u);
            PE_EffectSpriteCEE20(data,angles,size,size,220+2*lh(0x800F336Au),
                func_80077AA4(64,PE_LoadU16(0x800E1208u))&65535u,1,brightness,0);
            texture=216+2*lh(0x800F336Au);
            PE_EffectSpriteCEE20(data,0,size*2,size*2,texture,clut(),3,brightness,0);
            PE_EffectSpriteValuesCEE20(floor,floor_angles,size,size,216+2*lh(0x800F336Au),clut(),3,brightness,0);
        } else if (state==2) {
            size=func_80077CF4(lh(data+10u)*64)+2048;
            brightness=func_80077DC4(lh(data+10u)*64)/32;
            PE_EffectSpriteCEE20(data,0,size,size,253+lh(0x800F336Au),clut(),1,brightness,0);
            PE_EffectSpriteValuesCEE20(floor,floor_angles,size,size,253+lh(0x800F336Au),clut(),3,brightness,0);
        }
        texture_setup(0);
    }
    return 0;
}

int PE_M0013I_Main(int32_t mode,pe_addr_t data,pe_addr_t extra)
{
    return projectile_main(mode,data,extra,0x8018EFF4u,0x8018EFFCu,0x8018F004u);
}

/* M0028I 801924F8..80193148 matches M0013I 8018F004..8018FC54
 * after internal jumps and these three address references are relocated. */
int PE_M28ProjectileMain(int32_t mode,pe_addr_t data,pe_addr_t extra)
{
    return projectile_main(mode,data,extra,0x8018F1CCu,0x8018F1D4u,0x801924F8u);
}
