/* Original healing particles D4928/D4C24, C5060.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

static int32_t medicine_time(void) {return (int32_t)PE_LoadU32(0x800E27ECu);}

int func_800DF87C(int32_t mode,pe_addr_t data)
{
    int32_t time=medicine_time();(void)data;
    if (mode==1) return time>=24;
    if (mode==2) {
        pe_addr_t actor=PE_LoadU32(0x8009D254u);
        int16_t position[3],angles[4]={1024,0,(int16_t)((uint32_t)time*256u),1};
        uint8_t color[3];int32_t scale;unsigned i;
        PE_StoreU16(0x800F3374u,0u);
        for (i=0;i<3;i++) position[i]=(int16_t)PE_LoadU16(actor+42u+i*4u);
        position[1]=(int16_t)(position[1]-300);
        PE_EffectColorCF3AC(0x800E1494u,color,time);
        scale=func_80077CF4((int32_t)((uint32_t)time*1024u)/24);
        PE_EffectRingD0728(position,300,400,10,angles,scale,scale,0,color,128,1);
    }
    return 0;
}

int func_800D4928(int32_t mode,pe_addr_t data)
{
    int32_t time=medicine_time();
    if (mode==1) {
        int32_t x=(int16_t)PE_LoadU16(data+6u),z=(int16_t)PE_LoadU16(data+10u);
        PE_StoreU16(data,(uint16_t)(PE_LoadU16(data)+x));
        PE_StoreU16(data+2u,(uint16_t)(PE_LoadU16(data+2u)+PE_LoadU16(data+8u)));
        PE_StoreU16(data+6u,(uint16_t)(x*6/7));
        PE_StoreU16(data+4u,(uint16_t)(PE_LoadU16(data+4u)+z));
        PE_StoreU16(data+10u,(uint16_t)(z*6/7));
        if ((int16_t)PE_LoadU16(data+2u)>0)
            PE_StoreU16(data+8u,(uint16_t)-(int16_t)PE_LoadU16(data+8u));
        PE_StoreU16(data+8u,(uint16_t)(PE_LoadU16(data+8u)+3u));
        return time>=(int16_t)PE_LoadU16(data+12u);
    }
    if (mode==2) {
        int32_t kind=(int16_t)PE_LoadU16(PE_LoadU32(0x800E2368u)+30u);
        int32_t life=(int16_t)PE_LoadU16(data+12u),scale,phase;
        int16_t position[3],angles[4]={0,0,0,0};uint8_t color[3];unsigned i;
        uint32_t type=PE_LoadU16(0x800F336Cu),palette;
        int32_t color_time=(int32_t)((uint32_t)time*48u),growth=(int32_t)((uint32_t)time*2048u);
        if (!life || (life==-1 && (color_time==INT32_MIN || growth==INT32_MIN))) {
            Bootstrap_ReturnVoid("func_800D4928_lifetime_divide","func_800D4928");
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
        }
        PE_EffectColorCF3AC(kind==0?0x800E141Cu:kind==1?0x800E1444u:0x800E146Cu,color,color_time/life);
        for (i=0;i<3;i++) position[i]=(int16_t)PE_LoadU16(data+i*2u);
        angles[2]=(int16_t)(PE_LoadU16(data+14u)+(uint32_t)time*128u);
        scale=(int32_t)((uint32_t)(growth/life)+1024u);
        palette=PE_LoadU16(0x800E1204u+type*2u);
        if (type==4u && PE_LoadU32(0x800F3428u)) palette+=4u;
        phase=232+(int16_t)PE_LoadU16(0x800F336Au)*(time%7);
        PE_EffectSpriteValuesCEE20(position,angles,scale,scale,phase,
            func_80077AA4(32u,palette)&65535u,1,128,color);
    }
    return 0;
}

int func_800D4C24(int32_t mode,pe_addr_t data)
{
    if (!mode) {
        PE_StoreU32(data,func_80071A54());
        return (int)func_800CE560(PE_LoadU32(PE_LoadU32(0x800F33E0u)+8u),16u,16,0x800D4928u);
    }
    if (mode==1) {
        if (medicine_time()<8) {
            pe_addr_t child=func_800CE610(PE_LoadU32(PE_LoadU32(0x800F33E0u)+8u));
            if (child) {
                pe_addr_t actor=PE_LoadU32(0x8009D254u);
                /* CE870(actor,1) writes an original stack-local position. */
                uint16_t x=PE_LoadU16(actor+42u),y=PE_LoadU16(actor+46u),z=PE_LoadU16(actor+50u);
                int32_t radius;
                PE_StoreU16(child,x);
                PE_StoreU16(child+2u,(uint16_t)(y-((int16_t)PE_LoadU16(PE_LoadU32(0x800E2368u)+30u)?400:600)));
                PE_StoreU16(child+4u,z);
                radius=(func_80071A54()&7u)+32;
                PE_StoreU16(child+6u,(uint16_t)(func_80077CF4((int32_t)PE_LoadU32(data))*radius/4096));
                PE_StoreU16(child+10u,(uint16_t)(func_80077DC4((int32_t)PE_LoadU32(data))*radius/4096));
                PE_StoreU16(child+8u,(uint16_t)(-(int32_t)(func_80071A54()&7u)-12));
                PE_StoreU16(child+12u,(uint16_t)((func_80071A54()&3u)+26u));
                PE_StoreU16(child+14u,(uint16_t)func_80071A54());
                PE_StoreU32(data,PE_LoadU32(data)+512u);
            }
        }
        return medicine_time()>=40;
    }
    if (mode==2) {
        PE_StoreU16(0x800F336Au,1u);PE_StoreU16(0x800F3368u,16u);
        PE_StoreU16(0x800F3376u,16u);PE_StoreU16(0x800F3378u,16u);
        PE_StoreU16(0x800F3376u,32u);PE_StoreU16(0x800F3378u,16u);
        PE_StoreU16(0x800F336Cu,2u);PE_StoreU16(0x800F336Eu,0u);
        PE_StoreU16(0x800F3372u,0u);PE_StoreU16(0x800F3374u,32u);
        PE_StoreU16(0x800F3370u,PE_LoadU16(0x800E2850u+PE_LoadU16(0x800E11E8u)*2u));
    }
    return 0;
}
