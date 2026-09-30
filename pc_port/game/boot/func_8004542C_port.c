/* Original equipment page construction, filtering and property list.
 * Authority: 340EC.s, 44AA0.s, 486D8.s, 55254.s and matching C leaves. */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_800542A0(uint32_t mask)
{
    int32_t i;unsigned count=0;
    for (i=0;i<(int32_t)D_8009D050;i++) {
        pe_addr_t item=func_8005332C(i);
        unsigned kind=item?PE_LoadU8(item+6u):0u;
        if ((mask>>(kind&31u))&1u) PE_StoreU16(0x800A1D9Cu+count++*2u,(uint16_t)i);
    }
    PE_StoreU32(0x8009D068u,0u);PE_StoreU32(0x8009D040u,count);
}

void func_800543CC(uint32_t mask,int32_t excluded)
{
    int32_t i;unsigned count=0;
    for (i=0;i<(int32_t)D_8009D050;i++) {
        pe_addr_t item=func_8005332C(i);
        if (item && ((mask>>(PE_LoadU8(item+6u)&31u))&1u) &&
            PE_LoadU8(item+20u) && i!=excluded)
            PE_StoreU16(0x800A1D9Cu+count++*2u,(uint16_t)i);
    }
    PE_StoreU32(0x8009D068u,0u);PE_StoreU32(0x8009D040u,count);
}

void func_80059EC8(uint32_t slot,int32_t index)
{
    if (slot>=2u) return;
    PE_StoreU32(0x8009D090u+slot*4u,(uint32_t)index);
    PE_StoreU32(0x8009D098u+slot*4u,D_8009D048!=0x800C0E48u);
}

/* func_80064C20: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80064C20_port.c (src/func_80064C20.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80064B74(pe_addr_t list,int32_t index)
{
    pe_addr_t saved=0x800A3060u+(uint32_t)index*4u;
    PE_StoreU32((list<0x200000u?list|0x80000000u:list)+112u,(uint32_t)index);
    if (!list || index<0) return;
    PE_StoreU32(list+68u,(uint32_t)(int32_t)(int8_t)PE_LoadU8(saved));
    if (!PE_LoadU32(0x8009D16Cu) && !(PE_LoadU32(list+100u)&32u)) return;
    PE_StoreU32(list+72u,(uint32_t)(int32_t)(int8_t)PE_LoadU8(saved+1u));
    if ((int32_t)PE_LoadU32(list+72u)>=(int32_t)PE_LoadU32(list+88u))
        PE_StoreU32(list+72u,PE_LoadU32(list+88u)-1u);
    if (PE_LoadU32(list+104u) && PE_LoadU32(list+68u)==1u &&
        PE_LoadU32(list+72u)==PE_LoadU32(list+88u)-1u) PE_StoreU32(list+68u,0u);
    PE_StoreU32(list+92u,(uint32_t)(int32_t)(int8_t)PE_LoadU8(saved+2u));
}

void func_80045EE4(pe_addr_t parent)
{
    pe_addr_t window=func_80062D2C(6u,parent,0u,0u),list=func_8006322C(6u,window,window);
    PE_StoreU32(window+44u,0x8004620Cu);PE_StoreU32(window+64u,1u);
    PE_StoreU32(list+48u,0x8004F9A0u);PE_StoreU32(list+100u,PE_LoadU32(list+100u)|128u);
    if (PE_LoadU32(0x8009CF1Cu)) func_80064C20(list);
    else if (!PE_LoadU32(0x8009CF18u)) func_80064B74(list,20);
    if ((int32_t)PE_LoadU32(list+68u)>=0) func_80062CB8(list);
}

void func_8004542C(pe_addr_t parent)
{
    pe_addr_t window=func_80062D2C(5u,parent,0u,0u),list=func_8006322C(5u,window,window);
    PE_StoreU32(window+48u,0x80045A98u);PE_StoreU32(window+44u,0x80045D0Cu);
    PE_StoreU32(list+48u,0x8004F978u);
    if (PE_LoadU32(0x8009CF1Cu)) func_80064C20(list);
    else if (!PE_LoadU32(0x8009CF18u)) func_80064B74(list,18);
    if ((int32_t)PE_LoadU32(list+68u)>=0) func_80062CB8(list);
    PE_StoreU32(window+64u,1u);list=func_8006322C(27u,window,window);
    PE_StoreU32(list+48u,0x8004FFD0u);func_80064C20(list);
}

void func_80046378(pe_addr_t parent,uint32_t focus)
{
    pe_addr_t window,list;
    if (PE_LoadU32(0x8009CF1Cu)) {
        int32_t item=func_80059F08(0u),kind=func_8005415C(item);
        func_800543CC(kind>0 && kind<6?62u:1u<<((uint32_t)kind&31u),item);
        window=func_80062A34(1u,47u);
        func_80063158(window,(int32_t)(180u-PE_LoadU32(window+24u)),(int32_t)(164u-PE_LoadU32(window+28u)));
    } else func_800542A0(PE_LoadU32(0x8009CF18u)?510u:512u);
    if (!PE_LoadU32(0x8009CF0Cu) && !func_80054288()) {func_800526C4();return;}
    window=func_80062D2C(7u,parent,0u,0u);list=func_8006322C(7u,window,window);
    PE_StoreU32(window+44u,0x800466C0u);PE_StoreU32(list+48u,0x8004FB48u);
    if (PE_LoadU32(0x8009CF1Cu)) func_80064C20(list);
    if (!PE_LoadU32(0x8009CF18u)) func_80064B74(list,19);
    PE_StoreU32(list+68u,focus?0u:~0u);
    if ((int32_t)PE_LoadU32(list+72u)<0) PE_StoreU32(list+72u,0u);
    if ((int32_t)PE_LoadU32(list+68u)>=0) func_80062CB8(list);
    func_800647D0(list,func_80054288());
    if (PE_LoadU32(0x8009CF0Cu)) {PE_StoreU32(list+68u,~0u);func_80063158(window,0,20);}
    func_800525EC();
}

void func_8004F9A0(pe_addr_t list)
{
    pe_addr_t item=func_8005332C(func_80059F08(0u));
    PE_StoreU32(0x8009CF20u,item);PE_StoreU32(0x8009CF18u,PE_LoadU8(item+6u)!=9u);
    func_800647D0(list,PE_LoadU8(item+20u));PE_StoreU32(0x8009CEF4u,list);
    func_800638D8(list,0x80050AD8u);
}

/* func_80050AD8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050AD8_port.c (src/func_80050AD8.c); hand port retired (port3 switch-over E). */

/* func_8005E988: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005E988_port.c (src/func_8005E988.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static int32_t equipment_total(pe_addr_t item,unsigned stat)
{return PE_LoadU8(item+7u+stat)+(int16_t)PE_LoadU16(item+14u+stat*2u);}
static int32_t equipment_display_total(pe_addr_t item,unsigned stat)
{int32_t total=equipment_total(item,stat);return total<1000?total:999;}
static void equipment_stat_labels(int32_t y)
{
    unsigned glyph=PE_LoadU32(0x8009CF18u)?124u:127u;
    func_8005E8A4(4,y);func_8005EB64(glyph);
    func_8005E8A4(0,14);func_8005EB64(glyph+1u);
    func_8005E8A4(0,14);func_8005EB64(glyph+2u);
}

void func_8004551C(pe_addr_t item)
{
    unsigned i;
    if (!item) return;
    equipment_stat_labels(28);
    if (PE_LoadU32(0x8009CF1Cu) && PE_LoadU32(func_80062CC4()+36u)==7u) {
        func_8005E8A4(24,14);return;
    }
    func_8005E8A4(30,-28);
    for (i=0;i<3;i++) {func_80060590(equipment_display_total(item,i));func_8005E8A4(-36,14);}
}

void func_80045670(pe_addr_t old_item,pe_addr_t new_item)
{
    unsigned i;
    if (!new_item) return;
    if (old_item) {
        if (PE_LoadU32(0x8009CF1Cu)) {
            func_8005E8A4(40,-50);func_8005EB64(136u);func_8005E8A4(0,10);
            for (i=0;i<3;i++) {
                func_8005FF28((int16_t)PE_LoadU16(old_item+14u+i*2u));
                if (i<2) func_8005E8A4(-20,14);
            }
            func_8005E8A4(-38,12);
        }
        func_8005E8A4(38,-12);func_8005EB64(34u);
        func_8005E8A4(0,-14);func_8005EB64(34u);
        func_8005E8A4(0,-14);func_8005EB64(34u);
        if (PE_LoadU32(0x8009CF1Cu)) {
            func_8005E8A4(12,-10);func_8005EB64(136u);
            for (i=0;i<3;i++) {
                if (i) func_8005E8A4(-20,14);
                func_8005E988((int16_t)PE_LoadU16(old_item+14u+i*2u),(int16_t)PE_LoadU16(new_item+14u+i*2u));
                if (!i) func_8005E8A4(0,10);
                func_8005FF28((int16_t)PE_LoadU16(new_item+14u+i*2u));
            }
        } else {
            for (i=0;i<3;i++) {
                if (i) func_8005E8A4(-36,14);
                func_8005E988(equipment_display_total(old_item,i),equipment_display_total(new_item,i));
                if (!i) func_8005E8A4(12,-2);
                func_80060590(equipment_display_total(new_item,i));
            }
        }
    } else {
        equipment_stat_labels(23);func_8005E968(0x8080u);func_8005E8A4(40,-28);
        for (i=0;i<3;i++) {
            if (i) func_8005E8A4(-36,14);
            func_8005E988(0,equipment_total(new_item,i));func_80060590(equipment_display_total(new_item,i));
        }
    }
    func_8005E968(0x808080u);
}

void func_80045A98(pe_addr_t window)
{
    pe_addr_t node=func_80062A34(2u,13u),old_item;
    int32_t index;
    (void)window;
    if (node) {
        if ((int32_t)PE_LoadU32(node+68u)>=0) func_80059EC8(0u,func_800556E8(func_80063428(node)));
    } else if (PE_LoadU32(0x8009CF30u)) {
        node=func_80062A34(2u,16u);
        if (node && (int32_t)PE_LoadU32(node+68u)>=0) func_80059EC8(0u,func_800556E8(func_80063428(node)));
    } else if (!PE_LoadU32(0x8009CF1Cu))
        func_80059EC8(0u,(int8_t)PE_LoadU8(PE_LoadU32(0x8009CF18u)?0x800C0E20u:0x800C0E22u));
    index=PE_LoadU32(0x8009CF1Cu)?func_80059F08(0u):
        (int8_t)PE_LoadU8(PE_LoadU32(0x8009CF18u)?0x800C0E20u:0x800C0E22u);
    old_item=func_8005332C(index);node=func_80062A34(2u,7u);
    if (node==func_80062CC4()) {
        pe_addr_t replacement=func_8005332C(func_80059F08(1u));
        func_8004551C(PE_LoadU32(0x8009CF1Cu)?replacement:old_item);func_80045670(old_item,replacement);
    } else {
        int i;
        func_8004551C(old_item);if (!old_item) return;
        for (i=2;i>=0;i--) {
            func_8005E8A4(i==2?42:-45,i==2?-12:-14);func_8005FDF0(PE_LoadU8(old_item+7u+(unsigned)i));
            func_8005E8A4(5,0);func_8005FF28((int16_t)PE_LoadU16(old_item+14u+(unsigned)i*2u));
        }
        func_8005E8A4(-45,-10);func_8005EB64(135u);func_8005E8A4(25,0);func_8005EB64(136u);
    }
}

static void equipment_small_number(int32_t value,unsigned show_plus)
{
    unsigned digits=4u,i;int32_t divisor=1000;
    if (value<0 || (show_plus && value>0)) {
        unsigned glyph=value<0?82u:137u;
        if (value<0) value=(int32_t)(0u-(uint32_t)value);
        func_8005EB64(glyph);func_8005E8A4(5,0);digits=3u;divisor=100;
    }
    for (i=0;i<digits;i++,divisor/=10) {
        int32_t digit=value/divisor;
        func_8005F874(i+1u<digits && !digit?-1:digit);func_8005E8A4(5,0);
    }
}
/* func_8005FDF0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005FDF0_port.c (src/func_8005FDF0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8005FF28: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005FF28_port.c (src/func_8005FF28.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
