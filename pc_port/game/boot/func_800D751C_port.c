/* Original Aya reaction particles, D70C0..D7760, and D1DEC point glow.
 * Native callbacks share the original effect pools and GPU packet arena. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

static int32_t reaction_time(void) { return (int32_t)PE_LoadU32(0x800E27ECu); }
static int32_t reaction_mul(int32_t a,int32_t b) { return (int32_t)((uint32_t)a*(uint32_t)b); }

static void reaction_glow(const int16_t position[3],const uint8_t color[3],int32_t brightness,int32_t blend)
{
    uint32_t bank=PE_LoadU32(0x8009CDDCu),offset=PE_LoadU32(0x8009CDD8u),xy,z,depth;
    pe_addr_t base=PE_LoadU32(0x800B0E58u+bank*4u),point=base+offset,halo=point+12u,ot;
    unsigned i;
    PE_StoreU32(0x8009CDD8u,offset+28u);
    PE_GTE_SetV0(position[0],position[1],position[2]);PE_GTE_RTPS_coordinates(&xy,&z);
    PE_StoreU8(point+3u,2u);PE_StoreU8(point+7u,0x6Au); /* SetTile1 + SetSemiTrans(1). */
    func_80077C44(halo);
    for (i=0;i<3;i++) {
        uint8_t value=(uint8_t)(reaction_mul(color[i],brightness)/128);
        PE_StoreU8(point+4u+i,value);PE_StoreU8(halo+4u+i,value>>2u);
    }
    PE_StoreU32(point+8u,xy);
    depth=(z>>2u)-PE_LoadU16(0x800F3374u);
    if (depth>=4096u) return;
    PE_StoreU16(halo+8u,(uint16_t)(xy-1u));PE_StoreU16(halo+10u,(uint16_t)((xy>>16u)-1u));
    PE_StoreU16(halo+12u,3u);PE_StoreU16(halo+14u,3u);
    ot=PE_LoadU32(0x800B0E38u+bank*4u)+depth*4u;
    func_80077AC4(ot,point);
    if (blend!=255) {
        pe_addr_t mode=base+PE_LoadU32(0x8009CDD8u);
        PE_StoreU32(0x8009CDD8u,PE_LoadU32(0x8009CDD8u)+8u);
        func_80077C84(mode,0u,1u,func_80077A64(0u,(uint32_t)blend,0u,0u)&65535u);
        PE_StoreU8(halo+7u,PE_LoadU8(halo+7u)|2u);
        func_80077AC4(ot,halo);func_80077AC4(ot,mode);
    } else func_80077AC4(ot,halo);
}

void func_800D1DEC(pe_addr_t position,pe_addr_t color,int32_t brightness,int32_t blend)
{
    int16_t p[3];uint8_t c[3];unsigned i;
    for (i=0;i<3;i++) {p[i]=(int16_t)PE_LoadU16(position+i*2u);c[i]=PE_LoadU8(color+i);}
    reaction_glow(p,c,brightness,blend);
}

int func_800D70C0(int32_t mode,pe_addr_t data)
{
    unsigned i;
    if (mode==1) {
        for (i=0;i<3;i++) {
            uint32_t random=func_80071A54()&(i==1?3u:7u);
            PE_StoreU16(data+i*2u,(uint16_t)(PE_LoadU16(data+i*2u)+random-(i==1?7u:3u)));
        }
        return reaction_time()>=18;
    }
    if (mode==2) {
        int16_t p[3];uint8_t color[3];
        for (i=0;i<3;i++) p[i]=(int16_t)PE_LoadU16(data+i*2u);
        PE_EffectColorCF3AC(0x800E17E0u,color,reaction_time());
        reaction_glow(p,color,128,1);
    }
    return 0;
}

int func_800D71B8(int32_t mode,pe_addr_t data)
{
    int32_t time=reaction_time();unsigned i;
    if (mode==1) {
        int32_t radius=(int16_t)PE_LoadU16(data+12u),angle=(int16_t)PE_LoadU16(data+2u);
        PE_StoreU16(data+4u,(uint16_t)(PE_LoadU16(data+4u)+1u));
        PE_StoreU16(data+8u,(uint16_t)(reaction_mul(time,700)/36+PE_LoadU16(0x800E21E2u)-700));
        PE_StoreU16(data+6u,(uint16_t)(reaction_mul(func_80077DC4(angle),radius)/4096+PE_LoadU16(0x800E21E0u)));
        PE_StoreU16(data+10u,(uint16_t)(reaction_mul(func_80077CF4(angle),radius)/4096+PE_LoadU16(0x800E21E4u)));
        PE_StoreU16(data+2u,(uint16_t)(PE_LoadU16(data+2u)+128u));
        if (!(func_80071A54()&7u)) {
            pe_addr_t child=func_800CE610(PE_LoadU32(0x800E21E8u));
            if (child) for (i=0;i<3;i++) PE_StoreU16(child+i*2u,PE_LoadU16(data+6u+i*2u));
        }
        return time>=36;
    }
    if (mode==2) {
        int32_t state=(int16_t)PE_LoadU16(data),age=(int16_t)PE_LoadU16(data+4u),scale=4096,brightness=64;
        int16_t p[3],angles[4]={0,0,(int16_t)((uint32_t)time*128u),0};uint8_t color[3];
        uint32_t type=PE_LoadU16(0x800F336Cu),palette=PE_LoadU16(0x800E1204u+type*2u);
        if (state==0) {
            scale=func_80077CF4(reaction_mul(age,1024)/12);
            if (age>=12) {PE_StoreU16(data+4u,0u);PE_StoreU16(data,1u);}
        } else if (state==1) {
            if (age>=12) {PE_StoreU16(data+4u,0u);PE_StoreU16(data,2u);}
        } else {
            brightness=func_80077DC4(reaction_mul(age,1024)/12)/64;
            PE_StoreU16(data+12u,(uint16_t)(PE_LoadU16(data+12u)+(uint32_t)age*4u));
        }
        scale=reaction_mul(scale,3)/2;
        for (i=0;i<3;i++) {p[i]=(int16_t)PE_LoadU16(data+6u+i*2u);color[i]=PE_LoadU8(0x800C22D4u+i);}
        palette+=type==4u && PE_LoadU32(0x800F3428u)?7u:3u;
        PE_EffectSpriteValuesCEE20(p,angles,scale,scale,36,func_80077AA4(0,palette)&65535u,1,brightness,color);
    }
    return 0;
}

int func_800D751C(int32_t mode,pe_addr_t data)
{
    pe_addr_t rec=PE_LoadU32(0x800F33E0u);int32_t time=reaction_time();
    if (mode==0) {
        uint32_t size;
        PE_StoreU32(data,func_80071A54());
        size=func_800CE560(PE_LoadU32(rec+8u),16u,24,0x800D71B8u);
        return (int)(size+func_800CE5AC(data+4u,size,8u,18,0x800D70C0u));
    }
    if (mode==1) {
        if (time<32 && ((uint32_t)time&1u)) {
            pe_addr_t child=func_800CE610(PE_LoadU32(rec+8u));
            if (child) {
                PE_StoreU16(child+12u,200u);PE_StoreU16(child,0u);PE_StoreU16(child+4u,0u);
                PE_StoreU16(child+2u,(uint16_t)PE_LoadU32(data));PE_StoreU32(data,PE_LoadU32(data)-0x555u);
            }
        }
        if (time>=70) return 1;
        func_800CE688(PE_LoadU32(data+4u));
        PE_StoreU32(0x800E21E8u,PE_LoadU32(data+4u));
    } else if (mode==2) {
        PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));PE_StoreU16(0x800F3374u,8u);
        func_800CE78C(PE_LoadU32(data+4u));
        func_800CE870(PE_LoadU32(0x8009D254u),1,0x800E21E0u);
        PE_StoreU16(0x800F3368u,32u);PE_StoreU16(0x800F336Au,2u);
        PE_StoreU16(0x800F3376u,32u);PE_StoreU16(0x800F3378u,32u);PE_StoreU16(0x800F336Cu,1u);
        PE_StoreU16(0x800F3370u,PE_LoadU16(0x800E2850u+PE_LoadU16(0x800E11F6u)*2u));
        func_800CEDA8(1);PE_StoreU16(0x800F336Eu,1u);PE_StoreU16(0x800F3372u,0u);
    }
    return 0;
}
