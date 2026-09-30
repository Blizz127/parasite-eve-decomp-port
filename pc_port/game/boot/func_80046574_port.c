/* Original equipment selection and list drawing. 486D8/44AA0/340EC.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

void func_80059534(int32_t selected)
{
    pe_addr_t history=func_80051098();
    PE_StoreU32(history,2u);
    PE_StoreU32(history+4u,func_8005332C((int8_t)PE_LoadU8(0x800C0E20u)));
    PE_StoreU8(0x800C0E20u,(uint8_t)func_800556E8(selected));
    /* Original passes a stack copy of the index; command2 does not read it. */
    func_800512AC(2,0u);
}

static unsigned equipment_capacity_bonus(pe_addr_t item)
{
    unsigned i;
    if (!item) return 0u;
    for (i=0;i<PE_LoadU8(item+20u);i++) {
        unsigned kind=(PE_LoadU8(item+21u+i)&31u)-8u;
        if (kind<3u) return 1u<<kind;
    }
    return 0u;
}

int32_t func_8005968C(int32_t selected)
{
    int32_t old_index=(int8_t)PE_LoadU8(0x800C0E22u),result;
    pe_addr_t old_item=func_8005332C(old_index);
    unsigned old_extra=equipment_capacity_bonus(old_item);
    unsigned new_extra=equipment_capacity_bonus(func_8005332C(func_800556E8(selected)));
    PE_StoreU8(0x800C0E22u,(uint8_t)func_800556E8(selected));
    result=func_80054E4C((int32_t)(old_extra-new_extra));func_80054CF8();
    if (result==1) {
        pe_addr_t history=func_80051098();PE_StoreU32(history,3u);PE_StoreU32(history+4u,old_item);
        func_800512AC(3,0u);
    } else PE_StoreU8(0x800C0E22u,(uint8_t)old_index);
    return result;
}

int32_t func_80054520(uint32_t mask)
{
    unsigned saved=D_8009D048!=0x800C0E48u,count=0u,bank;
    func_80052E30(0u);
    for (bank=0u;bank<2u;bank++) {
        int32_t i;
        if (bank) {
            if (!D_8009D04C || (mask&256u)) break;
            func_80052E30(1u);
        }
        for (i=0;i<(int32_t)D_8009D050;i++) {
            pe_addr_t item=func_8005332C(i);
            if (item && ((mask>>(PE_LoadU8(item+6u)&31u))&1u) && PE_LoadU8(item+20u)) count++;
        }
    }
    func_80052E30(saved);return (int32_t)count>=2;
}

/* func_80063198: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80063198_port.c (src/func_80063198.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80050D18: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050D18_port.c (src/func_80050D18.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8004FFD0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004FFD0_port.c (src/func_8004FFD0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8004F978: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004F978_port.c (src/func_8004F978.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_800509E0(uint32_t unused)
{
    int32_t item;
    (void)unused;
    if (PE_LoadU32(0x8009CF1Cu)) item=func_80059F08(0u);
    else {
        func_8005EB58(!func_80052558());
        item=(int8_t)PE_LoadU8(PE_LoadU32(0x8009CF18u)?0x800C0E20u:0x800C0E22u);
    }
    if (item>=0) {func_80063198(func_80062A34(1u,6u));func_800536B8(item);}
    else {
        pe_addr_t list;
        func_800631AC(func_80062A34(1u,6u));list=func_80062A34(2u,6u);
        if ((int32_t)PE_LoadU32(list+68u)>=0) {
            PE_StoreU32(list+68u,~0u);list=func_80062A34(2u,5u);PE_StoreU32(list+68u,0u);func_80062CB8(list);
        }
        func_8005F5B8(57u);
    }
}

void func_8004FA10(pe_addr_t list)
{
    pe_addr_t window=func_80062A34(1u,7u),item;
    func_80052E30(func_80063428(func_80062A34(2u,54u))==1);
    if (window && !PE_LoadU32(window+72u) && func_80063428(func_80062A20(window,0u))>=0)
        func_80059EC8(1u,func_800556E8(func_80063428(func_80062A34(2u,7u))));
    item=func_8005332C(func_80059F08(1u));PE_StoreU32(0x8009CF20u,item);
    func_800647D0(list,PE_LoadU8(item+20u));PE_StoreU32(0x8009CEF4u,list);
    func_800638D8(list,0x80050AD8u);
}

void func_8004FB48(pe_addr_t list)
{
    int32_t bank=func_80063428(func_80062A34(2u,54u));
    if (bank>=0) {
        int32_t excluded=-1;uint32_t mask=830u;
        if (PE_LoadU32(list+36u)==7u) {
            int32_t kind;
            excluded=func_80059F08(0u);kind=func_8005415C(excluded);
            mask=kind && kind<6?62u:1u<<((uint32_t)kind&31u);
            if (func_80052F0C()!=(uint32_t)bank) excluded=-1;
        }
        func_80052E30((uint32_t)bank);func_800543CC(mask,excluded);func_800647D0(list,func_80054288());
    }
    PE_StoreU32(0x8009CEF4u,list);func_800638D8(list,0x800430A0u);
}

void func_800430A0(int32_t row)
{
    int32_t item,equipped;
    if (row>=func_80054288()) return;
    if ((PE_LoadU32(0x8009CF34u) || PE_LoadU32(0x8009CF38u)) && !func_80055FE0(row)) func_8005EB58(1u);
    item=func_800556E8(row);equipped=func_80054240(item);
    if (equipped && PE_LoadU32(PE_LoadU32(0x8009CEF4u)+36u)!=13u &&
        !PE_LoadU32(0x8009CF1Cu) && !PE_LoadU32(0x8009CF30u)) func_8005EB58(1u);
    if (PE_LoadU32(0x8009CF1Cu)) {
        uint32_t available=PE_LoadU32(0x800A1888u)+PE_LoadU32(0x800A188Cu)+PE_LoadU32(0x800A1890u)+PE_LoadU32(0x800A1894u);
        unsigned enabled=0u;
        if (available && !(PE_LoadU8(func_8005332C(item)+5u)&64u)) {
            int32_t kind=func_8005415C(item);
            enabled=(unsigned)func_80054520(kind<6?62u:1u<<((uint32_t)kind&31u));
        }
        if (!enabled) func_8005EB58(1u);
    }
    func_800536B8(item);if (equipped) func_80064C80();
}

static pe_addr_t equipment_ram(pe_addr_t p) {return p<0x200000u?p|0x80000000u:p;}
static uint32_t equipment_word(pe_addr_t p,unsigned off) {return PE_LoadU32(equipment_ram(p+off));}
static void equipment_store(pe_addr_t p,unsigned off,uint32_t value) {PE_StoreU32(equipment_ram(p+off),value);}
void func_80048254(void); /* port7_port.c (src/func_80048254.c) */
static void equipment_upgrade_boundary(const char *name)
{
    Bootstrap_ReturnVoid(name,"equipment tool menu");PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}
/* func_800631C0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800631C0_port.c (src/func_800631C0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_80045D0C(pe_addr_t window,uint32_t event)
{
    pe_addr_t list=func_80062A20(window,0u),other;
    if (event&0x10000u) {
        if (!func_80054288()) {func_800526C4();return 1;}
        if (PE_LoadU32(0x8009CF1Cu)) {func_80048254();return 1;}
        if (!func_80052558()) {func_800526C4();return 1;}
        other=func_80062D2C(53u,0u,0u,0u);other=func_8006322C(53u,other,other);
        PE_StoreU32(other+48u,0x8004FA10u);list=func_80062A34(2u,7u);
        equipment_store(list,68u,0u);func_80062CB8(list);func_800525EC();return 1;
    }
    if (event&64u) {
        if (PE_LoadU32(0x8009CF1Cu)) {equipment_upgrade_boundary("func_80047678");return 1;}
        func_80062F3C(7u);func_80062F1C(window);other=func_80062A34(1u,6u);
        if (other) {func_80062F1C(other);func_800439D8();func_80052634();}
        return 1;
    }
    if ((event&0x4000u) && func_800631C0(func_80062A34(1u,6u))) {
        equipment_store(list,68u,~0u);other=func_80062A20(window,1u);
        if (other) PE_StoreU32(other+68u,~0u);
        other=func_80062A34(2u,6u);
        if (other) {PE_StoreU32(other+72u,0u);PE_StoreU32(other+68u,0u);func_80062CB8(other);}
        func_8005267C();
    }
    return 1;
}

int func_8004620C(pe_addr_t window,uint32_t event)
{
    pe_addr_t list=func_80062A20(window,0u),tools=func_80062A34(2u,11u),other;
    if (tools) {equipment_upgrade_boundary("func_80047FE0");return 1;}
    if (event&64u) {
        func_80062F3C(7u);func_80062F1C(window);func_80062F3C(5u);func_800439D8();func_80052634();return 1;
    }
    if (event&0x1000u) {
        equipment_store(list,68u,~0u);other=func_80062A34(2u,5u);
        if (other) {PE_StoreU32(other+68u,0u);PE_StoreU32(other+72u,PE_LoadU32(other+88u)-1u);func_80062CB8(other);}
        func_8005267C();
    }
    return 1;
}

void func_80046574(pe_addr_t window,uint32_t confirmed)
{
    pe_addr_t list,other;unsigned selected_weapon;
    (void)window;if (!confirmed) return;
    list=func_80062A34(2u,7u);selected_weapon=PE_LoadU32(0x8009CF18u);
    if (selected_weapon) func_80059534(func_80063428(list));
    else if (!func_8005968C(func_80063428(list))) {func_8004CC50(29u,0u);goto refresh;}
    equipment_store(list,68u,~0u);other=func_80062A34(2u,5u);
    if ((int32_t)equipment_word(other,68u)<0) other=func_80062A34(2u,6u);
    func_80062CB8(other);func_80062F3C(53u);
refresh:
    func_800542A0(PE_LoadU32(0x8009CF18u)?510u:512u);other=func_80062A34(2u,6u);
    if (other==func_80062CC4()) {equipment_store(other,72u,0u);equipment_store(other,68u,0u);}
    if (PE_LoadU32(0x8009CF18u)) {func_8005E114(1);PE_StoreU32(0x8009CFB0u,1u);}
}

static void equipment_confirmation(pe_addr_t owner,int32_t selected)
{
    pe_addr_t window=func_80062D2C(41u,owner,0u,1u),list=func_8006322C(41u,window,window);
    uint32_t width;
    PE_StoreU32(window+48u,0x80044E14u);PE_StoreU32(window+44u,0x80044E98u);
    PE_StoreU32(list+48u,0x8004F950u);PE_StoreU32(0x8009CF14u,5u);func_80062CB8(list);
    func_80052E30(PE_LoadU32(0x8009CF10u));PE_StoreU8(0x800A1980u,255u);
    func_80052C08(0x800A1980u,func_8005DC4C(0u));PE_StoreU32(0x8009CFA0u,0u);
    width=func_8005F1A0(0x800A1980u);width=(int32_t)width<120?120u:func_8005F1A0(0x800A1980u);
    PE_StoreU32(window+52u,width+20u);PE_StoreU32(window+56u,50u);
    PE_StoreU32(window+24u,(uint32_t)((int32_t)(300u-width)>>1));
    PE_StoreU32(list+24u,(uint32_t)((int32_t)(PE_LoadU32(window+52u)-128u)>>1));
    PE_StoreU32(0x8009CFA8u,0x80046574u);PE_StoreU32(list+28u,PE_LoadU32(window+56u)-20u);
    func_80052BCC(0x800A1980u,func_8005DC4C(113u));func_80052C08(0x800A1980u,func_80053068(selected));
    width=func_8005F1A0(0x800A1980u);width=(int32_t)width<120?120u:func_8005F1A0(0x800A1980u);
    window=func_80062A34(1u,41u);PE_StoreU32(window+52u,width+20u);
    PE_StoreU32(window+24u,(uint32_t)((int32_t)(300u-width)>>1));list=func_80062A34(2u,41u);
    PE_StoreU32(list+24u,(uint32_t)((int32_t)(PE_LoadU32(window+52u)-128u)>>1));
}

int func_800466C0(pe_addr_t window,uint32_t event)
{
    pe_addr_t list=func_80062A20(window,0u),other;
    if (event&0x10000u) {
        int32_t selected=func_800556E8(func_80063428(list));
        if (PE_LoadU32(0x8009CF1Cu)) {equipment_upgrade_boundary("func_80047560");return 1;}
        if (func_80054240(selected)) {func_800526C4();return 1;}
        if (func_8005B89C()) equipment_confirmation(list,selected);
        else func_80046574(window,1u);
        func_800525EC();return 1;
    }
    if (!(event&0x1040u)) return 0;
    if (PE_LoadU32(0x8009CF0Cu)) {
        func_80062F3C(53u);equipment_store(list,68u,~0u);equipment_store(list,92u,0u);
        func_80062CB8(func_80062A34(2u,54u));func_80052634();return 1;
    }
    if (!(event&64u)) return 1;
    func_80062F3C(53u);
    if (PE_LoadU32(0x8009CF1Cu)) {equipment_upgrade_boundary("func_80048918");return 1;}
    equipment_store(list,68u,~0u);other=func_80062A34(2u,6u);
    if (other==func_80062CC4()) equipment_store(other,68u,!equipment_word(other,104u));
    else other=func_80062A34(2u,5u);
    func_80062CB8(other);func_80052634();return 1;
}
