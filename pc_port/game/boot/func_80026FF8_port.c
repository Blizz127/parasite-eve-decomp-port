/* Retail target highlight and restoration, 26FF8..27A08 (120D8.s),
 * plus the complete packet recoloring leaf 3CAEC (2CE38.s). */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8003CAEC(pe_addr_t model, uint32_t red, uint32_t green, uint32_t blue)
{
    static const uint32_t sizes[4]={52u,40u,36u,28u};
    static const uint32_t vertices[4]={4u,3u,4u,3u};
    pe_addr_t geometry=PE_LoadU32(model), packets;
    uint32_t color=(red&255u)|((green&255u)<<8u)|((blue&255u)<<16u);
    unsigned type,i,j;
    if (!geometry || !PE_LoadU16(model+0xBAu)) return;
    PE_StoreU8(model+0x90u,(uint8_t)red);
    PE_StoreU8(model+0x91u,(uint8_t)green);
    PE_StoreU8(model+0x92u,(uint8_t)blue);
    packets=PE_LoadU32(model+0x54u);
    for (type=0;type<4;type++) {
        uint32_t count=PE_LoadU16(geometry+8u+type*2u);
        for (i=0;i<count;i++,packets+=sizes[type]*2u) {
            pe_addr_t packet=packets+(uint32_t)PE_LoadU32(0x8009CDDCu)*sizes[type];
            uint8_t command=PE_LoadU8(packet+7u);
            for (j=0;j<vertices[type];j++)
                PE_StoreU32(packet+4u+j*(type<2?12u:8u),color);
            PE_StoreU8(packet+7u,command);
        }
    }
}

static void target_model_apply(pe_addr_t actor, int restore)
{
    if (restore) PE_StoreU16(actor+0x250u,PE_LoadU16(actor+0x250u)|0x20u);
    else {
        uint32_t color=PE_LoadU8(0x8009CE68u);
        func_8003CAEC(actor+0x1B4u,color,color,color);
    }
}

static void target_apply(pe_addr_t actor, int restore)
{
    int kind=(int8_t)PE_LoadU8(PE_LoadU32(actor)+5u);
    if (kind==1) target_model_apply(PE_LoadU32(actor+0x18Cu),restore);
    else if (kind==4) {
        pe_addr_t walk=PE_LoadU32(0x8009D20Cu), aya=PE_LoadU32(0x8009D254u);
        while (walk) {
            pe_addr_t body=PE_LoadU32(walk);
            if (walk!=aya && body && (int8_t)PE_LoadU8(body+5u)==4)
                target_model_apply(walk,restore);
            walk=PE_LoadU32(walk+4u);
        }
    } else target_model_apply(actor,restore);
}

static void target_set_apply(pe_addr_t targets, int8_t selected, uint32_t shape, int restore)
{
    pe_addr_t chosen=targets+(uint32_t)(int32_t)selected*12u;
    int32_t center=0, low=0, high=0;
    unsigned index=0;
    if (shape==0u) { target_apply(PE_LoadU32(chosen),restore); return; }
    if (shape==2u) {
        center=(int16_t)PE_LoadU16(chosen+8u);
        if (center < -0x600) { low=center+0x200; high=center+0xE00; }
        else if (center < 0x600) { low=center-0x200; high=center+0x200; }
        else { low=center-0xE00; high=center-0x200; }
        low=(int16_t)low; high=(int16_t)high;
    }
    while (PE_LoadU32(targets+index*12u)) {
        pe_addr_t entry=targets+index*12u;
        int32_t angle=(int16_t)PE_LoadU16(entry+8u);
        int eligible=shape!=2u || ((center>=-0x600 && center<0x600)
            ? angle>=low && angle<=high : angle<=low || angle>=high);
        if (eligible) target_apply(PE_LoadU32(entry),restore);
        index=(index+1u)&255u;
    }
}

void func_800275CC(pe_addr_t targets, int8_t selected)
{
    pe_addr_t weapon;
    if (!PE_LoadU8(0x8009D2B0u)) return;
    weapon=PE_LoadU32(PE_LoadU32(0x8009D278u)+0x68u);
    PE_StoreU8(0x8009CE6Cu,0u);
    target_set_apply(targets,selected,(PE_LoadU32(weapon+0x10u)>>6u)&3u,1);
}

void func_80026FF8(pe_addr_t targets, int8_t selected, int8_t mode)
{
    uint32_t color, shape;
    if (!PE_LoadU8(0x8009D2B0u)) return;
    color=PE_LoadU8(0x8009CE68u);
    if (color<=64u) PE_StoreU8(0x8009CE6Cu,8u);
    else if (color>=192u) PE_StoreU8(0x8009CE6Cu,248u);
    PE_StoreU8(0x8009CE68u,(uint8_t)(color+PE_LoadU8(0x8009CE6Cu)));
    if (mode>=8) return;
    shape=0u;
    if (mode<4) {
        pe_addr_t weapon=PE_LoadU32(PE_LoadU32(0x8009D278u)+0x68u);
        shape=(PE_LoadU32(weapon+0x10u)>>6u)&3u;
    }
    target_set_apply(targets,selected,shape,0);
}
