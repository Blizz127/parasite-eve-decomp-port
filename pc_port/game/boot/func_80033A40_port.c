/* Retail battle HUD: 33A40..347B4 (24240.s), 334AC..339A0
 * (20EE0.s). Uses the existing 30894 packet banks and retail font atlas.
 * Packet links, coordinates, integer gauge widths and status colors are
 * guest-visible. Missing render resources only occur in isolated fixtures.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

static pe_addr_t hud_ot(void)
{
    return PE_LoadU32(0x800B0E38u + PE_LoadU32(0x8009CDDCu)*4u);
}

static void hud_xy(pe_addr_t p, int32_t x, int32_t y)
{
    PE_StoreU16(p+8u, (uint16_t)x);
    PE_StoreU16(p+10u, (uint16_t)y);
}

static void hud_quad(pe_addr_t p, int32_t x, int32_t y, int32_t w)
{
    hud_xy(p,x,y);
    hud_xy(p+8u,x+w,y);
    hud_xy(p+16u,x,y+3);
    hud_xy(p+24u,x+w,y+3);
}

static unsigned hud_digits(int32_t value, uint8_t *digits)
{
    unsigned count=0;
    do {
        int32_t quotient=(int16_t)value/10;
        digits[count++]=(uint8_t)(value-(int16_t)quotient*10);
        value=quotient;
    } while ((int16_t)value != 0);
    return count;
}

/* Full 32B0C: rising damage/heal digits or the MISS sprite. Each actor
 * takes one 140-byte packet row in the current 1400-byte bank. */
void func_80032B0C(unsigned int mode, pe_addr_t amount)
{
    int32_t value=(int16_t)PE_LoadU16(amount);
    unsigned bank=PE_LoadU32(0x8009CDDCu),row=PE_LoadU32(0x8009D230u);
    pe_addr_t base=0x800B01C8u+bank*1400u+row*140u,ot=hud_ot()+28u;
    uint8_t level=(uint8_t)(PE_LoadU8(amount+6u)*4u),digits[6];
    int32_t x=PE_LoadU16(amount+2u),y=PE_LoadU16(amount+4u)+PE_LoadU8(amount+6u)-30;
    unsigned n,i;
    if (value<0 && (mode&255u)==1u) {
        PE_StoreU8(base+4u,level);PE_StoreU8(base+5u,0u);PE_StoreU8(base+6u,0u);
        PE_StoreU8(base+12u,0x50u);PE_StoreU8(base+13u,0xE0u);
        hud_xy(base,x-16,y);PE_StoreU16(base+16u,24u);PE_StoreU16(base+18u,8u);
        func_80077AC4(ot,base-8u);
    } else {
        n=hud_digits(value<0?0:value,digits);x-=(int32_t)n*4;
        for (i=n;i-- >0;) {
            pe_addr_t p=base+i*28u;
            unsigned color=PE_LoadU8(amount+7u);
            if(color<4u) {
                PE_StoreU8(p+4u,color==1u?0u:level);
                PE_StoreU8(p+5u,color==3u?0u:level);
                PE_StoreU8(p+6u,(color==1u || color==2u)?0u:level);
            }
            x+=8;
            PE_StoreU8(p+12u,(uint8_t)(digits[i]*8u));PE_StoreU8(p+13u,0xE0u);
            hud_xy(p,x,y);PE_StoreU16(p+16u,8u);PE_StoreU16(p+18u,8u);
            func_80077AC4(ot,p-8u);
        }
    }
    PE_StoreU32(0x8009D230u,PE_LoadU32(0x8009D230u)+1u);
}

void func_80034104(int32_t maximum, int32_t current)
{
    uint8_t digits[6];
    unsigned n=hud_digits((int16_t)maximum,digits), m, i;
    unsigned bank=PE_LoadU32(0x8009CDDCu);
    int32_t x=(int16_t)PE_LoadU16(0x8009CE84u);
    int32_t y=(int16_t)PE_LoadU16(0x8009CE86u)+11;
    pe_addr_t ot=hud_ot(), aya=PE_LoadU32(0x8009D254u);
    uint32_t flags=PE_LoadU32(PE_LoadU32(aya)+0x4Cu);
    unsigned pulse=PE_LoadU32(0x8009D1E8u)&31u;
    if (pulse>16u) pulse=32u-pulse;
    digits[n]=10; /* slash, followed by maximum HP */
    for (i=n+1; i-- > 0;) {
        pe_addr_t p=0x8009E1D8u+bank*140u+i*28u;
        PE_StoreU8(p+12u,(uint8_t)(digits[i]*8u));
        PE_StoreU8(p+13u,0xE8u);
        hud_xy(p,x+56-(int32_t)i*6,y);
        PE_StoreU8(p+4u,(uint8_t)((flags&0x800u)&&i!=n ? pulse*6+32 : 128));
        PE_StoreU8(p+5u,(uint8_t)((flags&0x800u)&&i!=n ? pulse+96 : 128));
        PE_StoreU8(p+6u,128);
        func_80077AC4(ot+20u,p-8u);
    }
    m=hud_digits((int16_t)current,digits);
    x+=56-(int32_t)n*12;
    if (n>m) x+=(int32_t)(n-m)*6;
    for (i=m; i-- > 0;) {
        pe_addr_t p=0x8009E0F8u+bank*112u+i*28u;
        PE_StoreU8(p+12u,(uint8_t)(digits[i]*8u));
        PE_StoreU8(p+13u,0xE8u);
        hud_xy(p,x,y); x+=6;
        PE_StoreU8(p+4u,(uint8_t)(flags&0x400u ? pulse*8 : 128));
        PE_StoreU8(p+5u,128);
        PE_StoreU8(p+6u,(uint8_t)(flags&0x400u ? pulse*6+32 : 128));
        func_80077AC4(ot+20u,p-8u);
    }
    {
        pe_addr_t p=0x8009E2F0u+bank*28u;
        hud_xy(p,(int16_t)PE_LoadU16(0x8009CE84u)+68,
                 (int16_t)PE_LoadU16(0x8009CE86u)+14);
        func_80077AC4(ot+16u,p-8u);
    }
}

void func_800334AC(void)
{
    static const uint32_t masks[9]={12u,48u,192u,3u,0x1000u,0x100u,0x400u,0x800u,0x200u};
    uint32_t flags=PE_LoadU32(PE_LoadU32(0x8009D278u)+0x4Cu);
    unsigned corner=PE_LoadU8(0x8009CE80u), bank=PE_LoadU32(0x8009CDDCu), i;
    int32_t x=corner==1u||corner==2u ? 289 : 15;
    int32_t y=corner<2u ? 193 : 15, step=x==289 ? -16 : 16;
    pe_addr_t ot=hud_ot();
    for (i=0; i<9; i++) {
        unsigned index;
        pe_addr_t p;
        if (!(flags&masks[i])) continue;
        index=i<4 ? i+((flags&masks[i])==masks[i] ? 4u : 0u) : i+4u;
        p=0x8009E968u+bank*364u+index*28u;
        hud_xy(p,x,y); x+=step;
        func_80077AC4(ot+28u,p-8u);
    }
}

void func_80033A40(void)
{
    pe_addr_t record=PE_LoadU32(0x8009D278u), ot=hud_ot(), p;
    unsigned bank=PE_LoadU32(0x8009CDDCu), at;
    int battle=(D_8009D1A0&2u)!=0;
    int pe=(PE_LoadU32(0x8009CEF0u)&2u)!=0; /* 438E0 */
    int32_t energy, maximum, width, x, y;
    if (!ot) return;
    if (!battle) {
        pe_addr_t aya=PE_LoadU32(0x8009D254u);
        if (!aya) return;
        record=PE_LoadU32(aya);
        PE_StoreU32(0x8009D278u,record);
    }
    if (!record) return;
    at=PE_LoadU16(record+16u);
    if (at>9000u) { at=9000u; PE_StoreU16(record+16u,at); }
    energy=(int32_t)PE_LoadU32(record+8u);
    maximum=(int32_t)PE_LoadU32(record+40u);
    if (maximum<energy) PE_StoreU32(record+8u,(uint32_t)maximum);
    else if (energy<0) PE_StoreU32(record+8u,0u);
    if ((int16_t)PE_LoadU16(record+28u)<(int16_t)PE_LoadU16(record+12u))
        PE_StoreU16(record+12u,PE_LoadU16(record+28u));
    maximum=(int16_t)PE_LoadU16(record+42u);
    /* A valid player record has positive maximum PE. Avoid division by
     * zero in resource-free fixtures (retail would take BREAK 7). */
    if (!maximum) return;
    width=(int16_t)PE_LoadU16(record+10u)*56/maximum;
    x=(int16_t)PE_LoadU16(0x8009CE84u);
    y=(int16_t)PE_LoadU16(0x8009CE86u);
    if (battle) {
        hud_xy(0x8009E098u+bank*16u,x+8,y+4);
        hud_quad(0x800B00E8u+bank*36u,x+8,y+4,(int32_t)(at*56u/9000u));
        hud_xy(0x800B6928u+bank*28u,x+8+(int32_t)(at*56u/9000u),y+2);
        hud_xy(0x8009E0C0u+bank*28u,x+68,y+3);
        func_80077AC4(ot+24u,0x8009E098u+bank*16u);
        func_80077AC4(ot+20u,0x800B00E8u+bank*36u);
        func_80077AC4(ot+16u,0x800B6920u+bank*28u);
        func_80077AC4(ot+16u,0x8009E0B8u+bank*28u);
    }
    if (pe) {
        hud_quad(0x800B0130u+bank*72u,x+8,y+25,width);
        hud_quad(0x800B0154u+bank*72u,x+8+width,y+25,56-width);
        hud_xy(0x8009E328u+bank*28u,x+68,y+24);
        func_80077AC4(ot+20u,0x800B0130u+bank*72u);
        func_80077AC4(ot+20u,0x800B0154u+bank*72u);
        func_80077AC4(ot+16u,0x8009E320u+bank*28u);
    }
    p=0x8009E070u+bank*24u;
    hud_xy(p,x,y+(battle ? 0 : 7));
    PE_StoreU16(p+12u,84u);
    PE_StoreU16(p+14u,(uint16_t)(18+(battle ? 7 : 0)+(pe ? 7 : 0)));
    func_80077AC4(ot+28u,p-8u);
    func_80034104((int16_t)PE_LoadU16(record+28u),(int16_t)PE_LoadU16(record+12u));
    func_800334AC();
}
