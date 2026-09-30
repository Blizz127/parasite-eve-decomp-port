/* M0005I room overlay: complete Eve charging effect and its particles.
 * Original Disc 1 chunk 2 is loaded at 8018EFE8; code remains native. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

static uint32_t charge_clut(void)
{
    uint32_t type=PE_LoadU16(0x800F336Cu),y=PE_LoadU16(0x800E1204u+type*2u);
    if (type==4u && PE_LoadU32(0x800F3428u)) y+=4u;
    return func_80077AA4(32,y);
}

int func_8018F018(int32_t mode, pe_addr_t p)
{
    if (mode==1) {
        int16_t state=(int16_t)PE_LoadU16(p+12u);
        if (!state) {
            int32_t angle=(int16_t)PE_LoadU16(p+8u),radius=(int16_t)PE_LoadU16(p+10u);
            int32_t ystep=func_80077CF4((int32_t)(PE_LoadU32(0x800E27ECu)<<6u))/512;
            int32_t xstep=(int32_t)((uint32_t)func_80077DC4(angle)*(uint32_t)radius)/4096;
            int32_t zstep=(int32_t)((uint32_t)func_80077CF4(angle)*(uint32_t)radius)/4096;
            PE_StoreU16(p+2u,(uint16_t)(PE_LoadU16(p+2u)+ystep));
            PE_StoreU16(p,(uint16_t)(PE_LoadU16(0x80190B84u)+xstep));
            PE_StoreU16(p+4u,(uint16_t)(PE_LoadU16(0x80190B88u)+zstep));
            PE_StoreU16(p+8u,(uint16_t)(angle+24));
            if ((int16_t)PE_LoadU16(p+6u)<=128) PE_StoreU16(p+6u,(uint16_t)(PE_LoadU16(p+6u)+8u));
            if ((int16_t)PE_LoadU16(PE_LoadU32(0x800E2368u)+18u)==1) {
                PE_StoreU16(p+12u,1u);PE_StoreU16(p+14u,0u);
            }
        } else if (state==1) {
            int16_t time=(int16_t)(PE_LoadU16(p+14u)+1u);
            int32_t weight=(int32_t)time*128;
            PE_StoreU16(p+14u,(uint16_t)time);
            func_800783E4(p,0x80190B8Cu,4096-weight,weight,p);
            return (int16_t)PE_LoadU16(p+14u)>=32;
        }
    } else if (mode==2) {
        int16_t angles[4]={0,0,(int16_t)PE_LoadU16(p+8u),0};uint8_t color[3]={0};
        int32_t time=(int32_t)((PE_LoadU32(0x800E27ECu)+(uint32_t)(int32_t)(int16_t)PE_LoadU16(p+2u))&63u);
        PE_EffectColorCF3AC(0x80190AD4u,color,time);
        PE_EffectSpriteCEE20(p,angles,2048,2048,216+(int16_t)PE_LoadU16(0x800F336Au),
            charge_clut(),3,(int16_t)PE_LoadU16(p+6u),color);
        PE_EffectSpriteCEE20(p,angles,4096,4096,216+2*(int16_t)PE_LoadU16(0x800F336Au),
            charge_clut(),3,(int16_t)PE_LoadU16(p+6u),color);
    }
    return 0;
}

int func_8018F330(int32_t mode, pe_addr_t data)
{
    if (!mode) {
        PE_StoreU16(data+8u,0u);PE_StoreU16(data+10u,0u);
        return (int)func_800CE560(PE_LoadU32(PE_LoadU32(0x800F33E0u)+8u),16u,24,0x8018F018u);
    }
    if (mode==1) {
        if (PE_LoadU32(0x800E27ECu)==7u && PE_LoadU32(0x800B0E64u)) {
            int32_t owner=func_800D3FD8();
            func_8006DF50(PE_LoadU32(0x800B0E64u),0x5ABu,owner,128,127);
            if (PE_LoadU32(0x800B0E64u)) func_8006DF50(PE_LoadU32(0x800B0E64u),0x5ACu,128,128,127);
        }
        if ((int32_t)PE_LoadU32(0x800E27ECu)<25) {
            pe_addr_t actor=PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u);
            uint16_t y=(uint16_t)PE_LoadU32(PE_LoadU32(actor+0x238u)+24u);
            pe_addr_t p=func_800CE610(PE_LoadU32(PE_LoadU32(0x800F33E0u)+8u));
            if (p) {
                PE_StoreU16(p+8u,(uint16_t)func_80071A54());
                PE_StoreU16(p+2u,(uint16_t)(y+(func_80071A54()&511u)-256u));
                PE_StoreU16(p+6u,0u);PE_StoreU16(p+10u,(uint16_t)(256u+(func_80071A54()&255u)));
                PE_StoreU16(p+12u,0u);PE_StoreU16(p+14u,0u);
            }
        }
        if ((int16_t)PE_LoadU16(data+8u)==1) {
            int16_t time=(int16_t)(PE_LoadU16(data+10u)+1u);
            PE_StoreU16(data+10u,(uint16_t)time);return time>=32;
        }
        if ((int16_t)PE_LoadU16(PE_LoadU32(0x800E2368u)+18u)) PE_StoreU16(data+8u,1u);
    } else if (mode==2) {
        pe_addr_t actor=PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u);
        func_800CE870(actor,0,0x80190B84u);func_800CE8F0(actor,20u,0x8018EFF4u,0x80190B8Cu);
        PE_StoreU16(0x800F3368u,16u);PE_StoreU16(0x800F336Au,1u);
        PE_StoreU16(0x800F3376u,16u);PE_StoreU16(0x800F3378u,16u);
        PE_StoreU16(0x800F336Cu,2u);PE_StoreU16(0x800F336Eu,0u);
        PE_StoreU16(0x800F3372u,0u);PE_StoreU16(0x800F3374u,8u);
        PE_StoreU16(0x800F3370u,PE_LoadU16(0x800E2850u+PE_LoadU16(0x800E11E8u)*2u));
    }
    return 0;
}

static uint32_t eve_clut(int32_t x,uint32_t extra)
{
    uint32_t type=PE_LoadU16(0x800F336Cu),y=PE_LoadU16(0x800E1204u+type*2u)+extra;
    if (type==4u && PE_LoadU32(0x800F3428u)) y+=4u;
    return func_80077AA4(x,y);
}

static void eve_joint(uint32_t joint,pe_addr_t constant,int16_t out[3])
{
    int16_t local[3];unsigned i;
    for (i=0;i<3;i++) local[i]=(int16_t)PE_LoadU16(constant+i*2u);
    PE_ActorJointPositionCE8F0(PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u),joint,local,out);
}

/* F614..FB84: eyes, hand flare and expanding rings preceding the beam. */
int func_8018F614(int32_t mode,pe_addr_t data)
{
    int32_t time=(int32_t)PE_LoadU32(0x800E27ECu);
    (void)data;
    if (!mode) {
        if (PE_LoadU32(0x800B0E64u)) {
            int32_t owner=func_800D3FD8();
            func_8006DF50(PE_LoadU32(0x800B0E64u),0x5F8u,owner,128,127);
            if (PE_LoadU32(0x800B0E64u)) func_8006DF50(PE_LoadU32(0x800B0E64u),0x5F9u,128,128,127);
        }
    } else if (mode==1) return time>=35;
    else if (mode==2) {
        int16_t position[3],angles[4]={0,0,(int16_t)((uint32_t)time<<6u),0};
        uint8_t color[3]={0};int32_t level;
        PE_StoreU16(0x800F3374u,64u);eve_joint(9u,0x8018F004u,position);
        if (time<35) {
            PE_StoreU16(0x800F3368u,32u);PE_StoreU16(0x800F3376u,32u);PE_StoreU16(0x800F3378u,32u);
            PE_StoreU16(0x800F336Au,2u);PE_StoreU16(0x800F336Cu,3u);
            PE_StoreU16(0x800F336Eu,0u);PE_StoreU16(0x800F3372u,0u);
            PE_StoreU16(0x800F3370u,PE_LoadU16(0x800E2850u+PE_LoadU16(0x800E11EAu)*2u));
            level=func_80077CF4((int32_t)((uint32_t)time<<11u)/34);
            position[0]=(int16_t)(position[0]-16);
            PE_EffectSpriteValuesCEE20(position,angles,4096,4096,2,eve_clut(16,0u),1,level,0);
            position[0]=(int16_t)(position[0]+32);
            PE_EffectSpriteValuesCEE20(position,angles,4096,4096,2,eve_clut(16,0u),1,level,0);
        }
        PE_StoreU16(0x800F3374u,24u);eve_joint(20u,0x8018EFFCu,position);
        if (time<35) {
            uint32_t phase=(uint32_t)time-18u;
            if (phase<17u) {
                level=func_80077DC4((int32_t)(phase<<6u));
                PE_EffectColorCF3AC(0x80190AF4u,color,(int32_t)(phase*2u));
                PE_EffectRingD0728(position,(int32_t)(600u-phase*100u),700,24,0,level,level,0,color,64,1);
                PE_EffectRingD0728(position,700,800,24,0,level,level,color,0,64,1);
            }
            PE_EffectColorCF3AC(0x80190AF4u,color,time);
            PE_StoreU16(0x800F3368u,64u);PE_StoreU16(0x800F3376u,64u);PE_StoreU16(0x800F3378u,64u);
            PE_StoreU16(0x800F336Au,4u);PE_StoreU16(0x800F336Cu,3u);
            PE_StoreU16(0x800F336Eu,1u);PE_StoreU16(0x800F3372u,0u);
            PE_StoreU16(0x800F3370u,PE_LoadU16(0x800E2850u+PE_LoadU16(0x800E11FAu)*2u));
            level=func_80077CF4((int32_t)((uint32_t)time<<10u)/34);
            PE_EffectSpriteValuesCEE20(position,angles,level,level,64,eve_clut(0,2u),1,96,color);
        }
    }
    return 0;
}

/* FB84..FDC4: short textured streaks emitted by the beam. */
int func_8018FB84(int32_t mode,pe_addr_t data)
{
    int32_t time=(int32_t)PE_LoadU32(0x800E27ECu);unsigned i;
    if (mode==1) {
        for (i=0;i<3;i++) PE_StoreU16(data+i*2u,(uint16_t)(PE_LoadU16(data+i*2u)+PE_LoadU16(data+8u+i*2u)));
        return time>=6;
    }
    if (mode==2) {
        int16_t position[3],angles[3];uint8_t color[3];
        int32_t brightness=func_80077DC4((int32_t)((uint32_t)time<<10u)/6)/24;
        PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
        for (i=0;i<3;i++) {
            position[i]=(int16_t)PE_LoadU16(0x80190BA8u+i*2u);
            angles[i]=(int16_t)(PE_LoadU16(data+i*2u)+PE_LoadU16(0x80190BA0u+i*2u));
            color[i]=PE_LoadU8(0x8018F00Cu+i);
        }
        PE_EffectRibbonD2370(position,angles,1100,110,(time&1)*128,32+(((uint32_t)time<<3u)&16u),
            127,15,eve_clut(0,0),color,color,(int16_t)brightness,1);
    }
    return 0;
}

static void eve_scale_matrix(PeEffectMatrix *matrix,int32_t x,int32_t y,int32_t z)
{
    const int32_t scale[3]={x,y,z};unsigned i;
    for (i=0;i<9;i++) {
        int32_t value=(int32_t)((uint32_t)(int32_t)matrix->r[i]*(uint32_t)scale[i%3u])>>12;
        matrix->r[i]=(int16_t)value;
        if (i==8u) matrix->pad=(int16_t)((uint32_t)value>>16u);
    }
}

static void eve_texture(uint32_t slot,uint32_t split)
{
    PE_StoreU16(0x800F336Cu,3u);PE_StoreU16(0x800F336Eu,(uint16_t)split);
    PE_StoreU16(0x800F3370u,PE_LoadU16(0x800E2850u+PE_LoadU16(slot)*2u));
}

/* Complete FDC4..190A6C: beam growth/fade, hit handshake and all mesh layers. */
int func_8018FDC4(int32_t mode,pe_addr_t data)
{
    int32_t time=(int32_t)PE_LoadU32(0x800E27ECu);unsigned i;
    if (!mode) {
        pe_addr_t actor=PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u),mesh;
        int16_t position[3];eve_joint(20u,0x8018F010u,position);
        for (i=0;i<3;i++) PE_StoreU16(data+i*2u,(uint16_t)position[i]);
        func_800CE9D4(actor,0,data+16u);
        PE_StoreU16(data+22u,1u);PE_StoreU16(data+32u,0u);PE_StoreU16(data+34u,0u);PE_StoreU32(data+24u,0u);
        if (PE_LoadU8(PE_LoadU32(0x800E2368u)+13u) && actor && PE_LoadU32(actor)) {
            pe_addr_t action=PE_LoadU32(PE_LoadU32(actor)+24u);
            if (PE_LoadU8(action)==1u) PE_StoreU8(action,2u);
        }
        mesh=func_8006E498(PE_LoadU32(0x800B0E64u),0xC5462704u);
        PE_StoreU32(0x80190B94u,mesh);func_800C6D5C(mesh,0,0);
        mesh=func_8006E498(PE_LoadU32(0x800B0E64u),0xC5862704u);
        PE_StoreU32(0x80190B98u,mesh);func_800C6D5C(mesh,0,0);
        return (int)func_800CE560(PE_LoadU32(PE_LoadU32(0x800F33E0u)+8u),16u,8,0x8018FB84u);
    }
    if (mode==1) {
        int16_t offset[3],state;
        if (time<10) {
            pe_addr_t p=func_800CE610(PE_LoadU32(PE_LoadU32(0x800F33E0u)+8u));
            if (p) {
                PE_StoreU16(p,(uint16_t)((func_80071A54()&1023u)-512u));
                PE_StoreU16(p+2u,(uint16_t)((func_80071A54()&1023u)-512u));PE_StoreU16(p+4u,0u);
                PE_StoreU16(p+8u,(uint16_t)((func_80071A54()&7u)-3u));
                PE_StoreU16(p+10u,(uint16_t)((func_80071A54()&7u)-3u));PE_StoreU16(p+12u,0u);
            }
        }
        state=(int16_t)PE_LoadU16(data+32u);
        if (!state) {
            PE_StoreU32(data+28u,4096u);PE_StoreU16(data+34u,(uint16_t)(PE_LoadU16(data+34u)+1u));
            PE_StoreU32(data+24u,PE_LoadU32(data+24u)+512u);
            if ((int16_t)PE_LoadU16(data+34u)>=5) {PE_StoreU16(data+32u,1u);PE_StoreU16(data+34u,0u);}
        } else if (state==1) {
            int16_t timer=(int16_t)(PE_LoadU16(data+34u)+1u);
            PE_StoreU16(data+34u,(uint16_t)timer);PE_StoreU32(data+28u,4096u-(uint32_t)((int32_t)timer*512));
            if (timer>=8) return 1;
        }
        PE_EffectOffsetCFB7C(data+16u,(int16_t)PE_LoadU16(data+24u),offset);
        for (i=0;i<3;i++) PE_StoreU16(data+8u+i*2u,(uint16_t)(PE_LoadU16(data+i*2u)+offset[i]));
        if (func_800CEB8C(data,data+8u,100) && PE_LoadU8(PE_LoadU32(0x800E2368u)+13u)) {
            pe_addr_t record=PE_LoadU32(PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u));
            if ((PE_LoadU32(record)&0x3F000000u)==0x01000000u) {
                pe_addr_t aya=PE_LoadU32(PE_LoadU32(0x8009D254u));
                PE_StoreU32(aya+76u,PE_LoadU32(aya+76u)|0x4000u);
                PE_StoreU32(record,(PE_LoadU32(record)&0xC0FFFFFFu)|0x91000000u);
            }
        }
    } else if (mode==2) {
        pe_addr_t mesh=PE_LoadU32(0x80190B94u);
        PeEffectMatrix matrix;int32_t amplitude=(int32_t)PE_LoadU32(data+28u),scale,brightness=160;
        int16_t position[3],angles[4]={1024,0,0,1};
        PE_StoreU16(0x800F3374u,8u);
        PE_EffectReadMatrix(PE_LoadU32(PE_LoadU32(PE_LoadU32(0x800F32D0u)+8u)+0x238u),&matrix);
        eve_scale_matrix(&matrix,amplitude/40,amplitude/40,(int32_t)(PE_LoadU32(data+24u)<<12u)/8680);
        for (i=0;i<3;i++) matrix.t[i]=(int16_t)PE_LoadU16(data+i*2u);
        eve_texture(0x800E11EAu,0u);PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
        func_800C6EC0(PE_LoadU16(0x800F3370u)|func_80077A64(0,1,0,0),eve_clut(48,0));func_800C6ED8(1u);
        func_800C6EF8(mesh);func_800C6FA0(mesh,50);PE_EffectMeshC71E4(mesh,&matrix);func_800C6F4C(mesh);
        func_800C6EF8(mesh);eve_scale_matrix(&matrix,8192,8192,4096);func_800C70EC(mesh,-255,-130,48);
        PE_EffectMeshC71E4(mesh,&matrix);func_800C6F4C(mesh);
        func_800C6EF8(mesh);eve_scale_matrix(&matrix,1024,1024,4096);func_800C6FA0(mesh,200);
        PE_EffectMeshC71E4(mesh,&matrix);func_800C6F4C(mesh);
        func_800C6EF8(mesh);matrix.t[1]=(int16_t)PE_LoadU16(0x800942ECu);
        eve_scale_matrix(&matrix,8192,8192,4096);func_800C6FA0(mesh,24);
        PE_EffectMeshC71E4(mesh,&matrix);func_800C6F4C(mesh);
        PE_StoreU16(0x800F3368u,32u);PE_StoreU16(0x800F336Au,2u);
        PE_StoreU16(0x800F3376u,32u);PE_StoreU16(0x800F3378u,32u);
        eve_texture(0x800E11FAu,1u);PE_StoreU16(0x800F3372u,0u);PE_StoreU16(0x800F3374u,32u);
        scale=amplitude;
        if (time&1) {brightness=128;scale=(int32_t)((uint32_t)scale*3u)/2;}
        PE_EffectSpriteCEE20(data,0,scale,scale,2,func_80077AA4(0,PE_LoadU16(0x800E120Au)+4u),1,brightness,0);
        PE_EffectSpriteCEE20(data+8u,0,amplitude/2,amplitude/2,2,eve_clut(0,4u),1,brightness,0);
        for (i=0;i<3;i++) position[i]=(int16_t)PE_LoadU16(data+i*2u);
        position[1]=(int16_t)PE_LoadU16(0x800942ECu);
        PE_EffectSpriteValuesCEE20(position,angles,scale,scale,2,eve_clut(0,4u),3,brightness,0);
        if (time<13) {
            int32_t phase=(int32_t)((uint32_t)time<<10u)/12;
            brightness=func_80077DC4(phase)/32;scale=func_80077CF4(phase)/2;
            PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));eve_texture(0x800E11FAu,1u);
            func_800C6EC0(PE_LoadU16(0x800F3370u)|func_80077A64(0,1,0,0),eve_clut(0,3u));func_800C6ED8(1u);
            angles[0]=(int16_t)(PE_LoadU16(data+16u)+2048u);angles[1]=(int16_t)PE_LoadU16(data+18u);
            angles[2]=(int16_t)((uint32_t)time<<7u);PE_RotMatrix79754_values(angles,matrix.r);
            for (i=0;i<3;i++) matrix.t[i]=(int16_t)PE_LoadU16(data+i*2u);
            eve_scale_matrix(&matrix,scale,scale,scale);mesh=PE_LoadU32(0x80190B98u);
            func_800C6EF8(mesh);func_800C6FA0(mesh,(uint16_t)(brightness/2));
            PE_EffectMeshC71E4(mesh,&matrix);func_800C6F4C(mesh);
        }
        PE_StoreU16(0x800F3368u,32u);PE_StoreU16(0x800F336Au,2u);
        PE_StoreU16(0x800F3376u,32u);PE_StoreU16(0x800F3378u,32u);
        eve_texture(0x800E11EAu,0u);PE_StoreU16(0x800F3372u,0u);PE_StoreU16(0x800F3374u,4u);
        for (i=0;i<3;i++) PE_StoreU16(0x80190BA8u+i*2u,PE_LoadU16(data+i*2u));
        PE_StoreU16(0x80190BA0u,0u);PE_StoreU16(0x80190BA2u,PE_LoadU16(data+18u));PE_StoreU16(0x80190BA4u,0u);
    }
    return 0;
}
