/* M0000I camera path selection: original95994,95D3C,95E4C.
 * Temporary sampler outputs use a saved scratchpad window; only values
 * escape, and stop checks preserve the original provider-call prefixes. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

static int camera_sample(uint32_t time,uint32_t id,uint32_t out[3],int *count)
{
    const pe_addr_t scratch=0x1F800240u;
    uint32_t saved[6];unsigned epoch=PE_Port_StopEpoch();
    pe_addr_t package=func_8006EC6C(0x801D0260u,2);
    if(PE_Port_StopEpoch()!=epoch)return 0;
    for(unsigned i=0;i<6;i++)saved[i]=PE_LoadU32(scratch+i*4u);
    *count=func_8018F55C(time,id,package,scratch,scratch+16u);
    for(unsigned i=0;i<3;i++)out[i]=PE_LoadU32(scratch+i*4u);
    for(unsigned i=0;i<6;i++)PE_StoreU32(scratch+i*4u,saved[i]);
    return PE_Port_StopEpoch()==epoch;
}

static void camera_delta(const uint32_t target[3],pe_addr_t source,
                         pe_addr_t delta,pe_addr_t count_addr,uint32_t shift)
{
    uint32_t from[3];
    for(unsigned i=0;i<3;i++)from[i]=PE_LoadU32(source+i*4u);
    shift&=255u;
    PE_StoreU16(count_addr,(uint16_t)shift);
    PE_StoreU16(count_addr-2u,(uint16_t)(1u<<(shift&31u)));
    for(unsigned i=0;i<3;i++)PE_StoreU32(delta+16u+i*4u,0);
    for(unsigned i=0;i<3;i++)PE_StoreU32(delta+i*4u,target[i]-from[i]);
    for(unsigned i=0;i<3;i++)PE_StoreU16(delta+32u+i*2u,(uint16_t)from[i]);
    for(unsigned i=0;i<3;i++)PE_StoreU16(delta+40u+i*2u,(uint16_t)target[i]);
}

void func_80195994(uint32_t variant,uint32_t eye_shift,uint32_t target_shift,uint32_t time)
{
    uint32_t value[3];int count;
    pe_addr_t entry=0x801EA378u+(uint32_t)((int32_t)(int16_t)variant*52);
    if(!camera_sample(time,PE_LoadU8(entry),value,&count))return;
    camera_delta(value,0x8019C810u,0x8019C08Cu,0x8019C056u,eye_shift);
    if(!camera_sample(time,PE_LoadU8(entry+1u),value,&count))return;
    camera_delta(value,0x8019C330u,0x8019C05Cu,0x8019C052u,target_shift);
}

void func_80195D3C(void)
{
    uint32_t value[3];int count;
    unsigned remaining=PE_LoadU8(0x8019C040u);
    if(remaining<2u)return;
    PE_StoreU8(0x8019C040u,(uint8_t)(remaining-1u));
    PE_StoreU8(0x8019C041u,(uint8_t)(PE_LoadU8(0x8019C041u)+1u));
    if(!camera_sample((uint32_t)PE_LoadU8(0x8019C041u)<<8,
                      (uint32_t)PE_LoadU8(0x8019C042u)+1u,value,&count))return;
    for(unsigned i=0;i<3;i++)PE_StoreU32(0x8019C810u+i*4u,value[i]);
    if(!camera_sample((uint32_t)PE_LoadU8(0x8019C041u)<<8,
                      PE_LoadU8(0x8019C042u),value,&count))return;
    for(unsigned i=0;i<3;i++)PE_StoreU32(0x8019C330u+i*4u,value[i]);
}

void func_80195E4C(uint32_t id,uint32_t unused1,uint32_t unused2,uint32_t time)
{
    uint32_t value[3];int count;
    (void)unused1;(void)unused2;
    int index=(int16_t)id;
    if(!camera_sample(time,(uint32_t)(index+1),value,&count))return;
    PE_StoreU16(0x8019C054u,0);
    for(unsigned i=0;i<3;i++)PE_StoreU32(0x8019C810u+i*4u,value[i]);
    if(!camera_sample(time,(uint32_t)index,value,&count))return;
    PE_StoreU8(0x8019C040u,(uint8_t)(count-2));
    PE_StoreU16(0x8019C050u,0);
    PE_StoreU8(0x8019C041u,0);PE_StoreU8(0x8019C042u,(uint8_t)id);
    PE_StoreU16(0x8019C02Cu,0);
    for(unsigned i=0;i<3;i++)PE_StoreU32(0x8019C330u+i*4u,value[i]);
}

/* Original95BC8..95D3C. Inputs may overlap the destination records, so
 * retain the original load/store order rather than copying both inputs. */
void func_80195BC8(pe_addr_t target,pe_addr_t eye,uint32_t eye_shift,uint32_t target_shift)
{
    uint32_t v=(uint32_t)(int32_t)(int16_t)PE_LoadU16(eye);
    uint32_t x=PE_LoadU32(0x8019C810u),y=PE_LoadU32(0x8019C814u),z;
    PE_StoreU32(0x8019C08Cu,v-x);
    v=(uint32_t)(int32_t)(int16_t)PE_LoadU16(eye+2u);
    z=PE_LoadU32(0x8019C818u);PE_StoreU32(0x8019C090u,v-y);
    v=(uint32_t)(int32_t)(int16_t)PE_LoadU16(eye+4u);
    PE_StoreU16(0x8019C0ACu,(uint16_t)x);x=PE_LoadU32(0x8019C330u);
    PE_StoreU16(0x8019C0AEu,(uint16_t)y);y=PE_LoadU32(0x8019C334u);
    PE_StoreU32(0x8019C09Cu,0);PE_StoreU32(0x8019C0A0u,0);PE_StoreU32(0x8019C0A4u,0);
    PE_StoreU16(0x8019C0B0u,(uint16_t)z);PE_StoreU32(0x8019C094u,v-z);
    PE_StoreU16(0x8019C0B4u,PE_LoadU16(eye));
    PE_StoreU16(0x8019C0B6u,PE_LoadU16(eye+2u));
    v=PE_LoadU16(eye+4u);eye_shift&=255u;
    PE_StoreU16(0x8019C056u,(uint16_t)eye_shift);PE_StoreU16(0x8019C0B8u,(uint16_t)v);
    v=(uint32_t)(int32_t)(int16_t)PE_LoadU16(target);PE_StoreU32(0x8019C05Cu,v-x);
    v=(uint32_t)(int32_t)(int16_t)PE_LoadU16(target+2u);z=PE_LoadU32(0x8019C338u);
    PE_StoreU32(0x8019C060u,v-y);
    v=(uint32_t)(int32_t)(int16_t)PE_LoadU16(target+4u);
    PE_StoreU16(0x8019C054u,(uint16_t)(1u<<(eye_shift&31u)));
    PE_StoreU32(0x8019C06Cu,0);PE_StoreU32(0x8019C064u,v-z);
    PE_StoreU32(0x8019C070u,0);PE_StoreU32(0x8019C074u,0);
    PE_StoreU16(0x8019C07Cu,(uint16_t)x);PE_StoreU16(0x8019C07Eu,(uint16_t)y);PE_StoreU16(0x8019C080u,(uint16_t)z);
    PE_StoreU16(0x8019C084u,PE_LoadU16(target));
    v=PE_LoadU16(target+2u);target_shift&=255u;
    PE_StoreU16(0x8019C086u,(uint16_t)v);v=PE_LoadU16(target+4u);
    PE_StoreU16(0x8019C052u,(uint16_t)target_shift);
    PE_StoreU16(0x8019C050u,(uint16_t)(1u<<(target_shift&31u)));
    PE_StoreU16(0x8019C088u,(uint16_t)v);
}
