/* Original menu borders, textured windows and pulsing selection rectangle.
 * 51CAC.s / 52ABC.s. The SDK frame query reads the host VBlank clock. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "pe_plat/gpu_hw.h"

static pe_addr_t window_ram(pe_addr_t address)
{ return address<0x200000u?address|0x80000000u:address; }

void func_8006153C(uint32_t first,uint32_t second,int32_t thickness,uint32_t shade)
{
    pe_addr_t packet=PE_MenuPacketAlloc(40u),p=window_ram(packet);
    pe_addr_t a=0x800A22B0u+(first&127u)*4u,b=0x800A22B0u+(second&127u)*4u;
    uint32_t color=PE_LoadU32(0x8009D14Cu+shade*4u);
    uint32_t width=thickness<0?0u-(uint32_t)thickness:(uint32_t)thickness;
    uint16_t ax=PE_LoadU16(a+2u),ay=PE_LoadU16(a),bx=PE_LoadU16(b+2u),by=PE_LoadU16(b);
    uint16_t cx=ax,cy=ay,dx=bx,dy=by;
    if (packet) {PE_MenuPacketColor(packet,4u,0x20u);PE_MenuPacketColor(packet+20u,4u,0x20u);}
    PE_StoreU8(p+7u,PE_LoadU8(p+7u)|2u);PE_StoreU8(p+27u,PE_LoadU8(p+27u)|2u);
    if (ay==by) {
        cx=(uint16_t)(cx+width);dx=(uint16_t)((uint32_t)dx+((second&128u)?width:0u-width));
        cy=(uint16_t)((uint32_t)cy+(uint32_t)thickness);dy=(uint16_t)((uint32_t)dy+(uint32_t)thickness);
    } else {
        cx=(uint16_t)((uint32_t)cx+(uint32_t)thickness);dx=(uint16_t)((uint32_t)dx+(uint32_t)thickness);
        cy=(uint16_t)(cy+width);dy=(uint16_t)((uint32_t)dy+((second&128u)?width:0u-width));
        color=(color>>1u)&0x7F7F7Fu;
    }
    PE_StoreU32(p+4u,(PE_LoadU32(p+4u)&0xFF000000u)|color);
    PE_StoreU32(p+24u,(PE_LoadU32(p+24u)&0xFF000000u)|color);
    PE_StoreU16(p+8u,ax);PE_StoreU16(p+10u,ay);PE_StoreU16(p+12u,bx);PE_StoreU16(p+14u,by);
    PE_StoreU16(p+16u,cx);PE_StoreU16(p+18u,cy);
    PE_StoreU16(p+28u,dx);PE_StoreU16(p+30u,dy);PE_StoreU16(p+32u,cx);PE_StoreU16(p+34u,cy);
    PE_StoreU16(p+36u,bx);PE_StoreU16(p+38u,by);
    PE_MenuPacketLink(packet);PE_MenuPacketLink(packet+20u);
}

void func_80061878(pe_addr_t edges,uint32_t shade)
{
    pe_addr_t light=PE_MenuPacketAlloc(8u),dark;
    if (light) (void)func_80077C84(light,0u,0u,((shade+1u)&3u)<<5u);
    dark=PE_MenuPacketAlloc(8u);
    if (dark) (void)func_80077C84(dark,0u,0u,((2u-shade)&3u)<<5u);
    while ((int8_t)PE_LoadU8(edges)>=0) {
        func_8006153C(PE_LoadU8(edges),(uint32_t)(int32_t)(int8_t)PE_LoadU8(edges+1u),2,shade);edges+=2u;
    }
    edges++;PE_MenuPacketLink(light);
    while ((int8_t)PE_LoadU8(edges)>=0) {
        func_8006153C(PE_LoadU8(edges),(uint32_t)(int32_t)(int8_t)PE_LoadU8(edges+1u),-2,shade==0u);edges+=2u;
    }
    PE_MenuPacketLink(dark);
}

static void window_point(uint32_t x,uint32_t y)
{
    pe_addr_t p=PE_LoadU32(0x8009D148u);
    if (p<0x800A22E0u) {
        PE_StoreU16(window_ram(p+2u),(uint16_t)x);PE_StoreU16(window_ram(p),(uint16_t)y);
        PE_StoreU32(0x8009D148u,p+4u);
    }
}

static void window_rectangle(uint32_t width,uint32_t height)
{
    uint32_t x=PE_LoadU32(0x8009D124u),y=PE_LoadU32(0x8009D128u);
    PE_StoreU32(0x8009D148u,0x800A22B0u);
    window_point(x,y);window_point(x+width,y);window_point(x,y+height);window_point(x+width,y+height);
}

void func_80061A3C(uint32_t width,uint32_t height,uint32_t shade)
{
    window_rectangle(width,height);func_80061878(0x800930A8u,shade);
}

void func_80061C34(uint32_t width,uint32_t height,pe_addr_t shape,uint32_t textured)
{
    pe_addr_t edges,packet,p;
    if (shape) {
        PE_StoreU32(0x8009D148u,0x800A22B0u);
        while (PE_LoadU8(shape)!=255u) {
            window_point(PE_LoadU32(0x8009D124u)+PE_LoadU8(shape),PE_LoadU32(0x8009D128u)+PE_LoadU8(shape+1u));shape+=2u;
        }
        edges=shape+1u;
    } else {window_rectangle(width,height);edges=0x800930A8u;}
    func_80061878(edges,0u);
    if (textured) {
        static const int16_t clear[4]={0,0,0,0},tile[4]={0,0,32,32};
        packet=PE_MenuPacketAlloc(12u);
        if (packet) pe_plat_gpu_tex_window_values(packet,clear);
        PE_MenuPacketLink(packet);
        packet=PE_MenuPacketAlloc(20u);p=window_ram(packet);PE_MenuPacketColor(packet,4u,0x64u);
        PE_StoreU16(p+8u,(uint16_t)PE_LoadU32(0x8009D124u));PE_StoreU16(p+10u,(uint16_t)PE_LoadU32(0x8009D128u));
        PE_StoreU8(p+12u,0u);PE_StoreU8(p+13u,0u);PE_StoreU16(p+14u,0x391Cu);
        PE_StoreU16(p+16u,(uint16_t)width);PE_StoreU16(p+18u,(uint16_t)height);PE_MenuPacketLink(packet);
        packet=PE_MenuPacketAlloc(12u);
        if (packet) pe_plat_gpu_tex_window_values(packet,tile);
        PE_MenuPacketLink(packet);
    }
    packet=PE_MenuPacketAlloc(8u);
    if (packet) (void)func_80077C84(packet,0u,0u,7u);
    PE_MenuPacketLink(packet);
}

void func_80062090(uint32_t width,uint32_t height,uint32_t pulse)
{
    pe_addr_t packet=PE_MenuPacketAlloc(16u),p=window_ram(packet);
    uint32_t color;
    PE_MenuPacketColor(packet,3u,0x60u);
    if (pulse) {
        uint32_t frame=PE_GPU_VSyncQuery();
        color=(frame&32u)?(frame&31u)*2u:64u-(frame&31u)*2u;
        color*=0x10101u;
    } else color=PE_LoadU32(0x8009D114u)&0xFFFFFFu;
    PE_StoreU32(p+4u,(PE_LoadU32(p+4u)&0xFF000000u)|color);
    PE_StoreU16(p+12u,(uint16_t)(width-4u));PE_StoreU16(p+14u,(uint16_t)(height-4u));
    PE_StoreU8(p+7u,PE_LoadU8(p+7u)|2u);
    PE_StoreU16(p+8u,(uint16_t)(PE_LoadU32(0x8009D124u)+2u));PE_StoreU16(p+10u,(uint16_t)(PE_LoadU32(0x8009D128u)+2u));
    PE_MenuPacketLink(packet);packet=PE_MenuPacketAlloc(8u);
    if (packet) (void)func_80077C84(packet,0u,0u,32u);
    PE_MenuPacketLink(packet);
}

static void window_border_color(uint32_t color)
{
    uint32_t r=color&255u,g=(color>>8u)&255u,b=(color>>16u)&255u;
    PE_StoreU32(0x8009D14Cu,color&0xFFFFFFu);
    PE_StoreU32(0x8009D150u,((b+g)/2u)|(((b+r)/2u)<<8u)|(((g+r)/2u)<<16u));
}

void func_800622BC(uint32_t width,uint32_t height,uint32_t pressed,uint32_t focused)
{
    if (PE_LoadU32(0x8009D130u)) window_border_color(PE_LoadU32(0x8009D130u));
    window_rectangle(width,height);func_80061878(0x800930A8u,pressed);
    func_8005E8A4(-2,-2);func_80062090(width+4u,height+4u,focused);
    if (PE_LoadU32(0x8009D130u)) window_border_color(PE_LoadU32(0x800C0E44u));
}
