/* Original battle reward setup, score tick/confirmation and level commit.
 * Authority: Disc 1 EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 * Each entry follows the original guest stores and call ordering. Local stat
 * out-parameters use the existing host-out adapter, not guest RAM scratch.
 * Level drawing preserves the original stat interpolation and GPU packets. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

static pe_addr_t reward_ram(pe_addr_t address)
{ return address<0x200000u?address|0x80000000u:address; }

static int32_t reward_word(pe_addr_t address)
{ return (int32_t)PE_LoadU32(address); }

/* 5BA78..5BBE4: remaining points and fraction, with nullable ordered outputs.
 * Unlike 5B91C, this entry does not clamp a fraction greater than 48. */
static void reward_stat_progress(int32_t table_number,int32_t key,
                                 int32_t *remaining,int32_t *fraction)
{
    pe_addr_t table=func_8005DB8C(table_number),row;
    int32_t index=64,step=32;
    do {
        row=table+(uint32_t)index*4u;
        if (key>=reward_word(row)) index+=step;
        else if (key<reward_word(row-4u)) index-=step;
        else step=0;
        step>>=1;
    } while (step);
    if (--index>=99) index=98;
    *remaining=0;*fraction=48;
    if (index<98) {
        uint32_t high,low;
        row=table+(uint32_t)index*4u;
        high=PE_LoadU32(row+4u);low=PE_LoadU32(row);
        *remaining=(int32_t)(high-(uint32_t)key);*fraction=0;
        if (high!=low) {
            int32_t numerator=(int32_t)(((uint32_t)key-low)*49u);
            int32_t denominator=(int32_t)(high-low);
            if (numerator==INT32_MIN && denominator==-1) abort();
            *fraction=numerator/denominator;
        }
    }
}

void func_8005BA78(int32_t table_number,int32_t key,pe_addr_t remaining_out,pe_addr_t fraction_out)
{
    int32_t remaining,fraction;
    reward_stat_progress(table_number,key,&remaining,&fraction);
    if (remaining_out) PE_StoreU32(remaining_out,(uint32_t)remaining);
    if (fraction_out) PE_StoreU32(fraction_out,(uint32_t)fraction);
}

static int32_t reward_animated_stat(uint32_t index)
{
    int32_t product=(int32_t)(PE_LoadU32(0x8009CF40u)*PE_LoadU32(0x800A18FCu+index*4u));
    return (int32_t)(PE_LoadU32(0x800A18D8u+index*4u)+(uint32_t)(product>>7));
}

/* 437B4..438C0: ease forward; hold the full bar while wrapping levels. */
void func_800437B4(uint32_t index)
{
    int32_t remaining,fraction,bar;
    pe_addr_t slot=0x800A1920u+index*4u,timer=0x800A1940u+index*4u;
    reward_stat_progress((int32_t)index,reward_animated_stat(index),&remaining,&fraction);
    bar=reward_word(slot);
    if (fraction>=bar) PE_StoreU32(slot,(uint32_t)((int32_t)((uint32_t)bar+(uint32_t)fraction)>>1));
    else if (bar<48) {PE_StoreU32(slot,48u);PE_StoreU32(timer,4u);}
    else {
        int32_t ticks=reward_word(timer);
        if (ticks>0) ticks-=2;
        PE_StoreU32(timer,(uint32_t)ticks);
        if (!ticks) PE_StoreU32(slot,(uint32_t)fraction);
    }
    func_8006062C(remaining,reward_word(slot));
}

/* Six repeated original FT4 emitters, including physical-zero writes on
 * exhausted allocation. Widths use the reloaded, truncated first vertex. */
static void reward_bar_piece(pe_addr_t spec,uint32_t part,int32_t fraction)
{
    pe_addr_t packet=PE_MenuPacketAlloc(40u),p=reward_ram(packet);
    uint32_t x=PE_LoadU32(0x8009D124u),y=PE_LoadU32(0x8009D128u)+1u,right,bottom,u,v;
    PE_MenuPacketColor(packet,9u,0x2Cu);
    switch (part) {
    case 0:x+=1u;break;
    case 1:x+=2u;break;
    case 2:x+=48u-(uint32_t)fraction;break;
    case 3:x+=49u-(uint32_t)fraction;break;
    case 4:x+=50u-(uint32_t)fraction;break;
    default:x+=49u;break;
    }
    PE_StoreU16(p+8u,x);right=PE_LoadU16(p+8u);
    PE_StoreU16(p+18u,y);PE_StoreU16(p+10u,y);
    bottom=PE_LoadU16(p+10u);PE_StoreU16(p+24u,x);
    if (part==1) right+=46u-(uint32_t)fraction;
    else if (part==4) right+=(uint32_t)fraction-1u;
    else right++;
    PE_StoreU16(p+32u,right);PE_StoreU16(p+16u,right);
    bottom+=PE_LoadU8(spec+5u);PE_StoreU16(p+34u,bottom);PE_StoreU16(p+26u,bottom);
    u=PE_LoadU8(spec)+part;PE_StoreU8(p+28u,u);PE_StoreU8(p+12u,u);
    v=PE_LoadU8(spec+1u);u=PE_LoadU8(p+12u);
    PE_StoreU8(p+21u,v);PE_StoreU8(p+13u,v);v=PE_LoadU8(p+13u);
    PE_StoreU8(p+36u,u);PE_StoreU8(p+20u,u);
    v+=PE_LoadU8(spec+5u);PE_StoreU8(p+37u,v);PE_StoreU8(p+29u,v);
    PE_StoreU16(p+14u,PE_LoadU16(spec+2u));PE_StoreU16(p+22u,7u);
    PE_MenuPacketLink(packet);
}

/* 6062C..61044: remaining-point digits and six independently gated bar pieces. */
void func_8006062C(int32_t remaining,int32_t fraction)
{
    pe_addr_t spec=func_8005DADC(72u),packet;
    uint32_t page;
    func_8005E8C4();func_8005E8A4(0,2);func_8005E8C4();func_8005E8A4(39,-2);
    func_8005FA3C(remaining);
    PE_StoreU32(0x8009D110u,0x808080u);PE_StoreU32(0x8009D114u,0x404040u);
    func_8005E914();func_8005EB64(73u);
    if (fraction<47) reward_bar_piece(spec,0u,fraction);
    if (fraction<46) reward_bar_piece(spec,1u,fraction);
    if (fraction<48) reward_bar_piece(spec,2u,fraction);
    if (fraction>0) reward_bar_piece(spec,3u,fraction);
    if (fraction>=3) reward_bar_piece(spec,4u,fraction);
    if (fraction>=2) reward_bar_piece(spec,5u,fraction);
    func_8005E914();page=PE_LoadU8(spec+6u);packet=PE_MenuPacketAlloc(8u);
    if (packet) (void)func_80077C84(packet,0u,0u,((page&3u)<<7u)|7u);
    PE_MenuPacketLink(packet);
}

void func_80050D20(uint32_t index)
{
    int32_t level,stat=reward_animated_stat(++index);
    func_8005E8A4(6,0);func_8005EB64(index+140u);func_8005E8A4(74,0);
    PE_func_8005B91C_HostOut((int32_t)index,stat,&level,NULL);
    func_800605F8((int32_t)((uint32_t)level+1u));
}

/* func_8004FFF8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004FFF8_port.c (src/func_8004FFF8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* 4BF40..4C1E0: animate level, max HP, BP and six stat bars before voice stop. */
void func_8004BF40(void)
{
    uint32_t busy=0u;
    func_8005E8A4(4,4);func_8005EB64(151u);
    if (reward_word(0x8009CF60u)<reward_word(0x8009CF6Cu)) {
        int32_t timer=reward_word(0x8009CF78u);busy=1u;
        PE_StoreU32(0x8009CF78u,(uint32_t)timer+1u);
        if (timer>=30) {
            PE_StoreU32(0x8009CF78u,0u);PE_StoreU32(0x8009CF60u,PE_LoadU32(0x8009CF60u)+1u);
            func_80055668(reward_word(0x8009CF60u));
        }
    }
    func_8005E968(PE_LoadU8(0x800C0E0Au)<reward_word(0x8009CF60u)?0x8080u:0x808080u);
    func_8005E8A4(15,16);func_8006055C((int32_t)(PE_LoadU32(0x8009CF60u)+1u));
    func_8005E968(0x808080u);func_8005E8A4(-60,20);func_8005EB64(153u);
    if (reward_word(0x8009CF64u)<reward_word(0x8009CF70u)) {
        int32_t timer=reward_word(0x8009CF7Cu);busy=1u;
        PE_StoreU32(0x8009CF7Cu,(uint32_t)timer+1u);
        if (timer>=0) {PE_StoreU32(0x8009CF7Cu,0u);PE_StoreU32(0x8009CF64u,PE_LoadU32(0x8009CF64u)+1u);}
    }
    func_8005E968(PE_LoadU16(0x800C0E06u)<reward_word(0x8009CF64u)?0x8080u:0x808080u);
    func_8005E8A4(15,16);func_8006055C(reward_word(0x8009CF64u));
    func_8005E8A4(-60,19);func_8005E968(0x808080u);func_8005EB64(147u);
    if (reward_word(0x8009CF68u)<reward_word(0x8009CF74u)) {
        PE_StoreU32(0x8009CF68u,PE_LoadU32(0x8009CF68u)+1u);busy=1u;
    }
    func_8005E968(reward_word(0x800C0E10u)<reward_word(0x8009CF68u)?0x8080u:0x808080u);
    func_8005E8A4(15,17);func_8006055C(reward_word(0x8009CF68u));
    func_8005E8A4(10,-86);func_8005E968(0x808080u);
    if (PE_LoadU32(0x8009CF80u)) {
        if (reward_word(0x8009CF40u)<128) PE_StoreU32(0x8009CF40u,PE_LoadU32(0x8009CF40u)+PE_LoadU32(0x8009CF84u));
        if (!busy && reward_word(0x8009CF40u)>=128) {PE_StoreU32(0x8009CF40u,128u);PE_StoreU32(0x8009CF80u,0u);}
    }
    func_8005E8A4(120,2);
    for (uint32_t i=1;i<7u;i++) {func_800437B4(i);func_8005E8A4(0,16);}
    if (!PE_LoadU32(0x8009CF80u)) func_80052764();
}

/* 5B8A8..5B91C: the original bounded binary search (signed thresholds). */
int32_t func_8005B8A8(int32_t experience,pe_addr_t table)
{
    int32_t index=64,step=32;
    do {
        pe_addr_t row=table+(uint32_t)index*4u;
        if (experience>=reward_word(row)) index+=step;
        else if (experience<reward_word(row-4u)) index-=step;
        else step=0;
        step>>=1;
    } while (step);
    index--;
    return index<99?index:98;
}

/* 51DF8..51E48: low-word product followed by signed division by twenty. */
int32_t func_80051DF8(int32_t index)
{
    pe_addr_t row=func_8005DBAC(index);
    int32_t product=(int32_t)((PE_LoadU32(0x800A1B30u)+20u)*PE_LoadU16(row));
    return product/20;
}

/* func_80057ECC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80057ECC_port.c (src/func_80057ECC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80052764: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80052764_port.c (src/func_80052764.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_8004B90C(void)
{
    pe_addr_t window=func_80062D2C(20u,0u,0u,0u);
    PE_StoreU32(window+48u,0x8004B970u);
    PE_StoreU32(window+44u,0x8004BB80u);
    func_80062CB8(window);
    PE_StoreU32(window+76u,0x800922F4u);
}

void func_8004B70C(uint32_t experience,uint32_t bonus,pe_addr_t items)
{
    int32_t index;
    func_8005B890(0);func_80051510();
    PE_StoreU32(0x8009CFE8u,PE_LoadU32(0x800C0E00u));
    PE_StoreU32(0x8009CFECu,PE_LoadU32(0x800C0E00u)+experience);
    PE_StoreU32(0x8009CFF0u,(uint32_t)func_8005B8A8(reward_word(0x8009CFE8u),func_8005DBF8()));
    PE_StoreU32(0x8009CEFCu,1u);
    PE_StoreU16(0x800C0E1Eu,PE_LoadU16(0x800C0E1Eu)+bonus);
    PE_StoreU32(0x8009CF60u,PE_LoadU8(0x800C0E0Au));
    PE_StoreU32(0x8009CF64u,PE_LoadU16(0x800C0E06u));
    PE_StoreU32(0x8009CF74u,PE_LoadU32(0x800C0E10u));
    PE_StoreU32(0x8009CF68u,PE_LoadU32(0x800C0E10u));
    PE_StoreU32(0x8009CF6Cu,(uint32_t)func_8005B8A8(reward_word(0x8009CFECu),func_8005DBF8()));
    if (reward_word(0x8009CF60u)<reward_word(0x8009CF6Cu)) {
        int32_t total=(int32_t)(PE_LoadU32(0x8009CF74u)+PE_LoadU16(0x800C0E1Eu));
        PE_StoreU32(0x8009CF74u,(uint32_t)(total>99999?99999:total));
        PE_StoreU16(0x800C0E1Eu,0u);
    }
    for (uint32_t i=0;i<7u;i++) {
        int32_t stat=(int16_t)PE_LoadU16(0x800C0E28u+i*2u);
        PE_StoreU32(0x800A18D8u+i*4u,(uint32_t)stat);
        func_8005B91C((int32_t)i,stat,0x800A18B4u+i*4u,0u);
        PE_StoreU32(0x800A18FCu+i*4u,(PE_LoadU32(0x8009CF6Cu)-PE_LoadU32(0x8009CF60u))*10u);
    }
    PE_func_8005B91C_HostOut(0,(int32_t)(PE_LoadU32(0x800A18D8u)+PE_LoadU32(0x800A18FCu)),&index,NULL);
    PE_StoreU32(0x8009CF70u,(uint32_t)func_80051DF8(index));
    PE_StoreU32(0x8009CF84u,2u);func_80057E14(items);
    func_8004B90C();func_8005270C();func_8005C144();
}

void func_8004BC80(void)
{
    pe_addr_t window=func_80062D2C(22u,0u,0u,0u);
    PE_StoreU32(window+48u,0x8004BCB4u);
}

/* func_8004BCB4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004BCB4_port.c (src/func_8004BCB4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_8004B970(void)
{
    uint32_t increment=reward_word(0x8009CFE8u)<reward_word(0x8009CFECu);
    int32_t level,remaining=0,score;
    if (!increment) func_80052764();
    level=func_8005B8A8(reward_word(0x8009CFE8u),func_8005DBF8());
    if (level<98) remaining=(int32_t)(PE_LoadU32(func_8005DBF8()+(uint32_t)level*4u+4u)-PE_LoadU32(0x8009CFE8u));
    PE_StoreU32(0x8009CFE8u,PE_LoadU32(0x8009CFE8u)+increment);
    if (reward_word(0x8009CFF0u)<level) {
        PE_StoreU32(0x8009CFF0u,(uint32_t)level);PE_StoreU32(0x8009CF78u,60u);
        if (!func_80062A34(1u,22u)) func_8004BC80();
    }
    if (PE_LoadU32(0x8009CF78u)) {
        uint32_t timer=PE_LoadU32(0x8009CF78u)-1u;PE_StoreU32(0x8009CF78u,timer);
        if (!timer) func_80062F1C(func_80062A34(1u,22u));
    }
    func_8005E8A4(4,4);func_8005E8C4();func_8005F27C(func_8005DC4C(9u));
    func_8005E8A4(80,0);score=reward_word(0x8009CFE8u);func_80060528(score>999999?999999:score);
    func_8005E8A4(2,2);func_8005EB64(138u);func_8005E914();
    func_8005E8A4(0,14);func_8005E8C4();func_8005F27C(func_8005DC4C(10u));
    func_8005E8A4(80,0);func_80060528(remaining);
    func_8005E8A4(2,2);func_8005EB64(138u);func_8005E914();
    func_8005E8A4(0,20);func_8005E8C4();func_8005F27C(func_8005DC4C(12u));
    func_8005E8A4(116,0);func_800605F8(func_80057ECC());func_8005E914();
}

int func_8004BB80(pe_addr_t window,uint32_t event)
{
    if (event&0x10000u) {
        if (reward_word(0x8009CFE8u)<reward_word(0x8009CFECu))
            PE_StoreU32(0x8009CFE8u,PE_LoadU32(0x8009CFECu));
        else {
            int32_t level,old;
            func_80062F1C(window);func_80062F1C(func_80062A34(1u,22u));func_80052764();
            level=func_8005B8A8(reward_word(0x8009CFE8u),func_8005DBF8());
            old=func_8005B8A8(reward_word(0x800C0E00u),func_8005DBF8());
            if (level!=old) {func_8004BE4C();func_8005270C();}
            else if (func_80057ECC()) func_80048654();
            else func_800512AC(10,0u);
            PE_StoreU32(0x800C0E00u,PE_LoadU32(0x8009CFE8u));
        }
        func_800525EC();
    }
    return 1;
}

void func_8004C4B4(uint32_t ability)
{
    if (!func_80062A34(1u,30u)) {
        pe_addr_t window=func_80062D2C(30u,0u,0u,0u);
        PE_StoreU32(window+48u,0x8004C520u);func_80062D2C(28u,0u,0u,0u);
    }
    PE_StoreU32(0x8009CFF4u,ability);
}

/* func_8004C520: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004C520_port.c (src/func_8004C520.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_80055668(uint32_t level)
{
    pe_addr_t table=func_8005DC10();uint32_t i=0;
    while (i<20u && PE_LoadU16(table+i*4u)!=level) i++;
    if (i==20u) return 0;
    PE_StoreU32(0x800C0E24u,PE_LoadU32(0x800C0E24u)|(1u<<i));
    func_8004C4B4(i);return 1;
}

int func_8004C1E0(pe_addr_t window,uint32_t event)
{
    if (event&0x10000u) {
        if (PE_LoadU32(0x8009CF80u)) {
            PE_StoreU32(0x8009CF40u,128u);
            PE_StoreU32(0x8009CF64u,PE_LoadU32(0x8009CF70u));
            PE_StoreU32(0x8009CF68u,PE_LoadU32(0x8009CF74u));
            while (reward_word(0x8009CF60u)<reward_word(0x8009CF6Cu)) {
                uint32_t level=PE_LoadU32(0x8009CF60u)+1u;PE_StoreU32(0x8009CF60u,level);
                if (func_80055668(level)) break;
            }
        } else {
            /* Retail commits NINE halfwords, although setup initializes seven. */
            for (uint32_t i=0;i<9u;i++)
                PE_StoreU16(0x800C0E28u+i*2u,PE_LoadU32(0x800A18D8u+i*4u)+PE_LoadU32(0x800A18FCu+i*4u));
            PE_StoreU8(0x800C0E0Au,PE_LoadU32(0x8009CF60u));
            PE_StoreU16(0x800C0E06u,PE_LoadU32(0x8009CF64u));
            PE_StoreU32(0x800C0E10u,PE_LoadU32(0x8009CF68u));func_8005218C();
            func_80062F1C(window);func_80062F1C(func_80062A34(1u,30u));func_80062F3C(28u);
            func_80052764();
            if (func_80057ECC()) func_80048654();else func_800512AC(10,0u);
        }
        func_800525EC();
    }
    return 1;
}

/* 5382C..53968: find consecutive empty inventory slots and select its bank. */
int32_t func_8005382C(int32_t count)
{
    pe_addr_t first,cursor,end;
    uint32_t size;
    PE_StoreU32(0x8009D048u,0x800C0E48u);size=func_80052F70();
    first=PE_LoadU32(0x8009D048u);PE_StoreU32(0x8009D058u,0x8009D05Cu);
    PE_StoreU32(0x8009D050u,size);PE_StoreU32(0x8009D064u,2u);
    end=first+size*2u-(uint32_t)count*2u+2u;
    for (cursor=first;cursor<end;cursor+=2u) {
        int32_t i=1;
        if (PE_LoadU16(cursor)) continue;
        while (i<count && !PE_LoadU16(cursor+(uint32_t)i*2u)) i++;
        if (i>=count) return (int32_t)(cursor-first)>>1;
    }
    return -1;
}

/* func_80063D78: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80063D78_port.c (src/func_80063D78.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80048654(void)
{
    pe_addr_t window,list,items,inventory;int32_t free_slot;
    window=func_80062D2C(12u,0u,0u,0u);list=func_8006322C(12u,window,window);
    PE_StoreU32(window+44u,0x80048838u);PE_StoreU32(list+48u,0x8004FCF8u);func_80062CB8(list);
    window=func_80062D2C(13u,list,0u,0u);items=func_8006322C(13u,window,window);
    PE_StoreU32(window+64u,1u);PE_StoreU32(window+44u,0x80044444u);
    PE_StoreU32(items+48u,0x8004FD68u);PE_StoreU32(items+132u,0x80058030u);func_80064C20(items);
    PE_StoreU32(items+100u,PE_LoadU32(items+100u)&~4u);func_800647D0(items,func_80057ECC());
    window=func_80062D2C(14u,items,0u,0u);inventory=func_8006322C(14u,window,window);
    PE_StoreU32(window+44u,0x80044444u);PE_StoreU32(inventory+48u,0x8004F8D0u);
    PE_StoreU32(inventory+136u,0x80050260u);PE_StoreU32(inventory+132u,0x80058030u);
    PE_StoreU32(inventory+68u,UINT32_MAX);PE_StoreU32(0x8009CF94u,UINT32_MAX);PE_StoreU32(0x8009CF8Cu,UINT32_MAX);
    func_800647D0(inventory,(int32_t)func_80052F70());
    PE_StoreU32(inventory+120u,items);PE_StoreU32(items+124u,inventory);
    free_slot=func_8005382C(func_80057ECC());
    if (free_slot>=0) func_80063D78(inventory,0,(int32_t)((uint32_t)free_slot+(uint32_t)func_80057ECC()-1u));
    else {
        free_slot=func_8005382C(1);
        if (free_slot>=0) func_80063D78(inventory,0,(int32_t)((uint32_t)free_slot+7u));
    }
    PE_StoreU32(inventory+68u,UINT32_MAX);func_8004C594();PE_StoreU32(0x8009CF00u,0u);
}

/* 4FCF8..4FDA4 / 50B94..50C08 / 57F14..58030: loot menu drawing. */
void func_8004FCF8(pe_addr_t list)
{
    PE_StoreU32(0x8009CEF4u,list);func_800638D8(list,0x80050B94u);func_8005EB58(1u);
    for (uint32_t rows=PE_LoadU32(list+56u);rows;rows--) {
        func_8005EB64(104u);func_8005E8A4(0,16);
    }
}

/* func_8004FD68: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004FD68_port.c (src/func_8004FD68.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80050B94: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050B94_port.c (src/func_80050B94.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80050BE8(uint32_t index) {func_80057F14(index);}

void func_80057F14(uint32_t index)
{
    int32_t id=(int16_t)PE_LoadU16(0x800A1FD4u+index*2u);
    pe_addr_t record,name;
    if (id<512) {
        record=func_800532B4((uint32_t)id);
        if (!record) return;
        name=func_8005DC9C(PE_LoadU8(record+4u)-1u);
    } else {
        /* Original high-ID arm has no 520 clamp. */
        record=0x8009DE64u+(uint32_t)id*32u;
        if (PE_LoadU8(record+5u)&16u)
            name=0x800C20A4u+(PE_LoadU8(record+6u)==9u?16u:0u);
        else name=func_8005DC9C(PE_LoadU8(record+4u)-1u);
    }
    func_800534E4(record,name);
}

static void reward_inventory_bank(void)
{
    PE_StoreU32(0x8009D048u,0x800C0E48u);
    PE_StoreU32(0x8009D050u,func_80052F70());
    PE_StoreU32(0x8009D058u,0x8009D05Cu);PE_StoreU32(0x8009D064u,2u);
}

/* Original XOR stores matter even for overlapping/same-address operands. */
static void reward_swap(pe_addr_t first,pe_addr_t second)
{
    uint16_t value=PE_LoadU16(first)^PE_LoadU16(second);
    PE_StoreU16(first,value);
    value=PE_LoadU16(second)^value;PE_StoreU16(second,value);
    PE_StoreU16(first,PE_LoadU16(first)^value);
}

static int reward_equipped(int32_t index)
{
    return PE_LoadU32(0x8009D048u)==0x800C0E48u &&
        ((int8_t)PE_LoadU8(0x800C0E20u)==index || (int8_t)PE_LoadU8(0x800C0E22u)==index);
}

/* 58030..5833C: manual swaps within and between inventory/loot lists. */
int func_80058030(uint32_t first_list,int32_t first,uint32_t second_list,int32_t second)
{
    int result=1;
    reward_inventory_bank();
    if (first_list==13u && second_list==13u)
        reward_swap(0x800A1FD4u+(uint32_t)first*2u,0x800A1FD4u+(uint32_t)second*2u);
    else if (first_list==14u && second_list==14u) {
        reward_swap(PE_LoadU32(0x8009D048u)+(uint32_t)first*2u,
                    PE_LoadU32(0x8009D048u)+(uint32_t)second*2u);
        for (pe_addr_t slot=0x800C0E20u;slot<=0x800C0E22u;slot+=2u) {
            int32_t equipped=(int8_t)PE_LoadU8(slot);
            if (equipped==first) PE_StoreU8(slot,(uint32_t)second);
            else if (equipped==second) PE_StoreU8(slot,(uint32_t)first);
        }
    } else if (first_list==13u) {
        pe_addr_t entry=0x800A1FD4u+(uint32_t)first*2u,record;
        if (!func_80057654(second) || reward_equipped(second)) return 0;
        /* Retail deliberately uses 5DB44 directly for this subtype check. */
        record=func_8005DB44((uint32_t)(int32_t)(int16_t)PE_LoadU16(entry)-1u);
        if (PE_LoadU8(reward_ram(record+6u))>=16u &&
            PE_LoadU8(reward_ram(func_8005DB44((uint32_t)(int32_t)(int16_t)PE_LoadU16(entry)-1u)+6u))<19u)
            result=func_8005833C(first)==0;
        else reward_swap(entry,PE_LoadU32(0x8009D048u)+(uint32_t)second*2u);
    } else {
        if (!func_80057654(first) || reward_equipped(first)) return 0;
        reward_swap(PE_LoadU32(0x8009D048u)+(uint32_t)first*2u,0x800A1FD4u+(uint32_t)second*2u);
    }
    func_80055760();return result;
}

/* 5833C..58454: accumulate a resource reward and clear it only on success. */
int32_t func_8005833C(int32_t index)
{
    pe_addr_t entry,record;int32_t result;
    if (index<0 || index>=reward_word(0x8009D078u)) return 1;
    entry=0x800A1FD4u+(uint32_t)index*2u;
    record=func_800532B4((uint32_t)(int32_t)(int16_t)PE_LoadU16(entry));
    reward_inventory_bank();result=func_80053B48(reward_ram(record));
    if (!result) PE_StoreU16(entry,0u);
    return result;
}

/* 58454..58670: take all eligible rewards in original list order. */
void func_80058454(void)
{
    reward_inventory_bank();
    for (int32_t i=0;i<reward_word(0x8009D078u);i++) {
        pe_addr_t entry=0x800A1FD4u+(uint32_t)i*2u;
        int32_t id=(int16_t)PE_LoadU16(entry);
        pe_addr_t record=func_800532B4((uint32_t)id);
        if (PE_LoadU8(reward_ram(record+6u))>=16u && PE_LoadU8(reward_ram(func_800532B4((uint32_t)id)+6u))<19u) {
            if (!func_8005833C(i)) PE_StoreU16(entry,0u);
        } else if (id<512) {
            pe_addr_t cursor=PE_LoadU32(0x8009D048u);
            pe_addr_t end=cursor+PE_LoadU32(0x8009D050u)*2u;
            while (cursor<end && PE_LoadU16(cursor)) cursor+=2u;
            if (cursor<PE_LoadU32(0x8009D048u)+PE_LoadU32(0x8009D050u)*2u) {
                PE_StoreU16(cursor,(uint32_t)id);PE_StoreU16(entry,0u);
            }
        }
    }
    func_80055760();
}

/* 58670..588EC: materialize weapon/armor records before leaving rewards. */
void func_80058670(void)
{
    PE_StoreU32(0x8009D04Cu,0u);PE_StoreU32(0x8009D054u,0u);
    reward_inventory_bank();
    for (int32_t i=0;i<reward_word(0x8009D050u);i++) {
        pe_addr_t record=func_8005332C(i);
        uint32_t kind=record?PE_LoadU8(record+6u):0u;
        pe_addr_t entry,free_record,source;
        if (kind-1u>=9u) continue;
        entry=PE_LoadU32(0x8009D048u)+(uint32_t)i*2u;
        if ((uint32_t)(PE_LoadU16(entry)-256u)<128u) continue;
        for (free_record=0x800C0EACu;free_record<0x800C1EACu;free_record+=32u)
            if (!PE_LoadU8(free_record)) break;
        if (free_record>=0x800C1EACu) continue;
        source=func_8005DB44((uint32_t)(int32_t)(int16_t)PE_LoadU16(entry)-1u);
        /* The executable loads and stores each sixteen-byte group in order. */
        for (uint32_t offset=0;offset<32u;offset+=16u) {
            uint32_t data[4];
            for (uint32_t j=0;j<4u;j++) {
                pe_addr_t p=source+offset+j*4u;
                data[j]=PE_LoadU8(p)|(uint32_t)PE_LoadU8(p+1u)<<8u|
                    (uint32_t)PE_LoadU8(p+2u)<<16u|(uint32_t)PE_LoadU8(p+3u)<<24u;
            }
            for (uint32_t j=0;j<4u;j++) PE_StoreU32(free_record+offset+j*4u,data[j]);
        }
        PE_StoreU16(PE_LoadU32(0x8009D048u)+(uint32_t)i*2u,256u+(free_record-0x800C0EACu)/32u);
    }
}

/* 48838..48918: Take All / Select / Done, with original focus and teardown. */
int func_80048838(pe_addr_t window,uint32_t event)
{
    if (event&0x10000u) {
        pe_addr_t list=func_80062A20(window,0u);
        switch (func_80063428(list)) {
        case 0:func_80058454();PE_StoreU32(list+72u,2u);break;
        case 1:
            list=func_80062A34(2u,13u);PE_StoreU32(list+72u,0u);
            PE_StoreU32(list+68u,0u);func_80062CB8(list);break;
        case 2:func_80058670();func_80062F9C();func_800512AC(10,0u);break;
        default:return 1;
        }
        func_800525EC();
    }
    return 1;
}
