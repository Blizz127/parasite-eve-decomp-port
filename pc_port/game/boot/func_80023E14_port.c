/* Original recovery items and inventory removal used by player statuses.
 * 120D8.s, 42664.s, 43CE4.s, 44AA0.s, 48530.s and 486D8.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

extern unsigned int func_80052F70(void);
extern pe_addr_t func_8005DB44(unsigned int index);

static void item_inventory(void)
{
    D_8009D048=0x800C0E48u;D_8009D050=func_80052F70();
    D_8009D058=0x8009D05Cu;D_8009D064=2u;
    PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
    PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
}

static pe_addr_t item_slot(int32_t index)
{
    int32_t id;
    if (index<0 || index>=(int32_t)D_8009D050) return 0u;
    id=(int16_t)PE_LoadU16(D_8009D048+(uint32_t)index*2u);
    if ((uint32_t)(id-256)<128u) return 0x800BEEACu+(uint32_t)id*32u;
    if ((uint32_t)(id-1)<255u) return func_8005DB44((uint32_t)(id-1));
    if ((uint32_t)(id-512)<9u) return 0x8009DE64u+(uint32_t)id*32u;
    return 0u;
}

static unsigned item_random_byte(void)
{
    uint32_t index=PE_LoadU32(0x8009D038u)+1u;
    if ((int32_t)index>=521) {
        unsigned i;
        for (i=0;i<521;i++) PE_StoreU8(0x800A1B90u+i,
            PE_LoadU8(0x800A1B90u+i)^PE_LoadU8((i<32u?0x800A1D79u:0x800A1B70u)+i));
        index=0u;
    }
    PE_StoreU32(0x8009D038u,index);return PE_LoadU8(0x800A1B90u+index);
}

int32_t func_8005485C(void)
{
    unsigned count=0u;int32_t i;
    for (i=0;i<(int32_t)D_8009D050;i++) {
        pe_addr_t item=item_slot(i);
        if (!item || !(PE_LoadU8(item+5u)&128u)) continue;
        if (D_8009D048==0x800C0E48u &&
            ((int8_t)PE_LoadU8(0x800C0E20u)==i || (int8_t)PE_LoadU8(0x800C0E22u)==i)) continue;
        PE_StoreU16(0x800A1D9Cu+count*2u,(uint16_t)i);count++;
    }
    PE_StoreU32(0x8009D068u,0u);PE_StoreU32(0x8009D040u,count);
    if (count) {
        unsigned chosen=count*item_random_byte()/256u;
        int32_t index=(int16_t)PE_LoadU16(0x800A1D9Cu+chosen*2u);
        return (int16_t)PE_LoadU16(D_8009D048+(uint32_t)index*2u);
    }
    return 0;
}

/* func_800553A4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800553A4_port.c (src/func_800553A4.c); hand port retired (port3 switch-over E). */

void func_800523F8(pe_addr_t rate,pe_addr_t delay)
{
    int32_t index=0;pe_addr_t info;
    PE_func_8005B91C_HostOut(3,PE_LoadU16(0x800C0E2Eu),&index,NULL);
    info=func_8005DBAC((int32_t)((uint32_t)index+PE_LoadU32(0x800A1B3Cu)));
    if (rate) PE_StoreU32(rate,PE_LoadU32(info+16u));
    if (delay) PE_StoreU32(delay,PE_LoadU32(info+12u));
}

void func_8003335C(pe_addr_t actor,unsigned unused,unsigned icon)
{
    unsigned bank;(void)unused;icon&=255u;
    PE_StoreU8(0x8009D235u,30u);
    for (bank=0;bank<2;bank++) {
        pe_addr_t src=0x8009E974u+icon*28u+bank*364u,dst=0x8009EC40u+bank*28u;
        PE_StoreU8(dst+12u,PE_LoadU8(src));PE_StoreU8(dst+13u,PE_LoadU8(src+1u));
        PE_StoreU16(dst+8u,(uint16_t)(PE_LoadU16(actor+528u)-8u));
        PE_StoreU16(dst+10u,(uint16_t)(PE_LoadU16(actor+530u)-16u));
        PE_StoreU16(dst+14u,PE_LoadU16(src+2u));
    }
}

void func_80023E14(int32_t item)
{
    pe_addr_t actor=PE_LoadU32(0x8009D254u),p;
    int icon=-1,field=(D_8009D1A0&2u)==0u;
    if (field) PE_StoreU32(0x8009D278u,PE_LoadU32(actor));
    p=PE_LoadU32(0x8009D278u);
    func_8006DE80(item<13?0x4B4:0x4B5,field,(int16_t)PE_LoadU16(actor+42u),
        (int16_t)PE_LoadU16(actor+46u),(int16_t)PE_LoadU16(actor+50u));
    if (item>=6 && item<=9) {
        static const unsigned heal[]={45,90,180,400};
        PE_StoreU16(p+12u,(uint16_t)(PE_LoadU16(p+12u)+heal[item-6]));
    } else if (item==10) PE_StoreU16(p+12u,PE_LoadU16(p+28u));
    else if (item==11 || item==12)
        PE_StoreU32(p+8u,PE_LoadU32(p+8u)+(uint32_t)((int32_t)PE_LoadU32(p+40u)/(item==11?4:2)));
    else if (item>=13 && item<=16) {
        static const unsigned masks[]={3u,48u,12u,192u};
        static const int icons[]={7,5,4,6};
        unsigned mask=masks[item-13],flags=PE_LoadU32(p+76u),state=flags&mask;
        if (!state) PE_StoreU32(p+76u,flags|mask);
        else if (state!=mask) {
            PE_StoreU32(p+76u,flags&~mask);
            if (item==15) PE_StoreU32(0x8009D2E8u,PE_LoadU32(0x8009D2E8u)&~16u);
        }
        icon=icons[item-13];
    } else if (item==17) {
        static const unsigned masks[]={3u,12u,48u,192u};unsigned i;
        func_800523F8(p+44u,p+48u);
        for (i=0;i<4;i++) {
            uint32_t flags=PE_LoadU32(p+76u);
            if ((flags&masks[i])==masks[i]) continue;
            PE_StoreU32(p+76u,flags&~masks[i]);
            if (i==1) PE_StoreU32(0x8009D2E8u,PE_LoadU32(0x8009D2E8u)&~16u);
        }
        PE_StoreU32(p+76u,PE_LoadU32(p+76u)&~4096u);
    }
    if ((int16_t)PE_LoadU16(p+12u)>(int16_t)PE_LoadU16(p+28u)) PE_StoreU16(p+12u,PE_LoadU16(p+28u));
    if (icon!=-1 && (D_8009D1A0&2u)) func_8003335C(actor,0u,(unsigned)icon);
}

static int item_capacity(void)
{
    int32_t count=(int32_t)(D_8009D018+PE_LoadU8(0x800C0E0Cu));
    return count<51?count:50;
}

int32_t PE_ItemArmorCapacity59A40(uint32_t *extra)
{
    pe_addr_t armor=item_slot((int8_t)PE_LoadU8(0x800C0E22u));
    unsigned reserve=0u,i,used=0u;int count;
    if (armor) for (i=0;i<PE_LoadU8(armor+20u);i++) {
        unsigned kind=(PE_LoadU8(armor+21u+i)&31u)-8u;
        if (kind<3u) {reserve=1u<<kind;break;}
    }
    count=item_capacity();item_inventory();
    for (i=0;i<D_8009D050;i++) if (PE_LoadU16(D_8009D048+i*2u)) used++;
    if (extra) *extra=reserve;
    return (int32_t)((uint32_t)count-used)>=(int32_t)reserve;
}

int32_t func_80059A40(pe_addr_t out)
{
    uint32_t extra;int32_t result=PE_ItemArmorCapacity59A40(&extra);
    if (out) PE_StoreU32(out,extra);return result;
}

int32_t func_80054E4C(int32_t extra)
{
    unsigned i,used=0u;int count;
    item_inventory();count=item_capacity();item_inventory();
    for (i=0;i<D_8009D050;i++) if (PE_LoadU16(D_8009D048+i*2u)) used++;
    PE_StoreU32(0x8009D06Cu,(uint32_t)extra);
    return (int32_t)((uint32_t)count-used)>=extra;
}

/* func_80054CF8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80054CF8_port.c (src/func_80054CF8.c); hand port retired (port3 switch-over H). */

int32_t func_80057D30(int32_t index)
{
    int32_t item;uint32_t extra=0u;
    if (D_8009D048==0x800C0E48u && (int8_t)PE_LoadU8(0x800C0E22u)==index)
        (void)PE_ItemArmorCapacity59A40(&extra);
    item=(int16_t)PE_LoadU16(D_8009D048+(uint32_t)index*2u);
    PE_StoreU16(D_8009D048+(uint32_t)index*2u,0u);
    if (item>=256) PE_StoreU8(0x800C0EACu+(uint32_t)(item-256)*32u,0u);
    if (D_8009D048==0x800C0E48u && (int8_t)PE_LoadU8(0x800C0E22u)==index) {
        PE_StoreU8(0x800C0E22u,255u);(void)func_80054E4C((int32_t)extra);
        func_80054CF8();func_800512AC(3,0u);
    }
    return item;
}

int32_t func_8005409C(int32_t item)
{
    unsigned i;
    item_inventory();
    for (i=0;i<D_8009D050;i++) if ((int16_t)PE_LoadU16(D_8009D048+i*2u)==item) {
        (void)func_80057D30((int32_t)i);return 0;
    }
    return -1;
}
