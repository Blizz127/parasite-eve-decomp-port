/* Original item-action setup, ammunition compatibility and item-window input.
 * 340EC.s / 467E0.s / 486D8.s / 11718.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

static pe_addr_t item_ram(pe_addr_t p) {return p<0x200000u?p|0x80000000u:p;}

/* func_800210D4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800210D4_port.c (src/func_800210D4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80055724: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80055724_port.c (src/func_80055724.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80052558: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80052558_port.c (src/func_80052558.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80055FB4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80055FB4_port.c (src/func_80055FB4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static uint32_t ammunition_class(uint32_t kind)
{
    if (kind && kind<8u) return kind>4u?kind-4u:1u;
    return kind>=19u?kind-18u:0u;
}

static uint32_t mark_compatible_ammunition(uint32_t kind,uint32_t category,int32_t excluded)
{
    int32_t i;uint32_t count=0u;
    func_80055724();
    for (i=0;i<(int32_t)D_8009D050;i++) {
        pe_addr_t record;uint32_t other,enabled;
        if (i==excluded) continue;
        record=func_8005332C(i);if (!record) continue;
        other=PE_LoadU8(record+6u);
        enabled=ammunition_class(other)==category && (kind-19u<3u || other-19u<3u);
        {
            pe_addr_t bits=D_8009D058+((uint32_t)i>>5u)*4u;
            PE_StoreU32(bits,PE_LoadU32(bits)|(enabled<<((uint32_t)i&31u)));
        }
    }
    for (i=0;i<(int32_t)D_8009D050;i++) count+=(uint32_t)func_80055FE0(i);
    return count;
}

int32_t func_800562A4(int32_t index)
{
    pe_addr_t record=func_8005332C(index);
    uint32_t kind=PE_LoadU8(item_ram(record+6u)),saved=func_80052F0C();
    uint32_t category=ammunition_class(kind),count=mark_compatible_ammunition(kind,category,index);
    if (saved || D_8009D04C) {
        func_80052E30(!saved);count+=mark_compatible_ammunition(kind,category,-1);
        func_80052E30(saved);
    }
    return (int32_t)count;
}

void func_8005600C(pe_addr_t bank,pe_addr_t selected)
{
    uint32_t saved=func_80052F0C();int32_t i;
    func_80052E30(PE_LoadU32(bank));
    for (i=0;i<(int32_t)D_8009D050;i++)
        if (func_80055FE0(i) && i!=(int32_t)PE_LoadU32(selected)) break;
    if (i<(int32_t)D_8009D050) PE_StoreU32(selected,(uint32_t)i);
    else {
        func_80052E30(!PE_LoadU32(bank));
        for (i=0;i<(int32_t)D_8009D050;i++) if (func_80055FE0(i)) break;
        if (i<(int32_t)D_8009D050) {
            PE_StoreU32(bank,!PE_LoadU32(bank));PE_StoreU32(selected,(uint32_t)i);
        } else PE_StoreU32(bank,UINT32_MAX);
    }
    func_80052E30(saved);
}

void PE_CopyItemRecord(pe_addr_t destination,pe_addr_t source)
{
    uint32_t offset=0u;
    while (offset<32u) {
        uint32_t count=offset==24u?2u:3u,values[3],i;
        for (i=0;i<count;i++) values[i]=PE_LoadU32(item_ram(source+offset+i*4u));
        for (i=0;i<count;i++) PE_StoreU32(item_ram(destination+offset+i*4u),values[i]);
        offset+=count*4u;
    }
}

void func_80056C40(uint32_t first_bank,int32_t first,uint32_t second_bank,int32_t second)
{
    func_80052E30(first_bank);PE_StoreU32(0x8009D070u,func_8005332C(first));
    func_80052E30(second_bank);PE_StoreU32(0x8009D074u,func_8005332C(second));
    PE_CopyItemRecord(0x800A1F94u,PE_LoadU32(0x8009D070u));
    PE_CopyItemRecord(0x800A1FB4u,PE_LoadU32(0x8009D074u));
    PE_StoreU8(0x800A1FD3u,0u);PE_StoreU8(0x800A1FB3u,0u);
    PE_StoreU16(0x800A1FC0u,0u);PE_StoreU16(0x800A1FA0u,0u);
}

/* func_80057D18: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80057D18_port.c (src/func_80057D18.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_80057654(int32_t index)
{
    pe_addr_t record=func_8005332C(index);int enabled;int32_t i,count=0;
    if (!record) return 1;
    enabled=!(PE_LoadU8(record+5u)&64u) && (func_80052F0C() || index!=(int8_t)PE_LoadU8(0x800C0E20u));
    if (PE_LoadU8(record+6u)==8u) {
        for (i=0;i<(int32_t)D_8009D050;i++) count+=func_8005415C(i)==8;
        if (count<2) enabled=0;
    }
    return enabled;
}

void func_80044924(pe_addr_t owner,uint32_t bank,int32_t index)
{
    pe_addr_t window,list,record;uint32_t kind,group,id;int unavailable=0,enabled=0;
    PE_StoreU32(0x8009CF10u,bank);func_80052E30(bank);PE_StoreU32(0x8009CF04u,(uint32_t)index);
    kind=(uint32_t)func_8005415C(index);
    group=kind==10u?0u:(kind-12u<4u || kind-8u<2u?2u:1u);
    PE_StoreU32(0x8009CDA8u,group);id=group==2u?3u:2u;
    window=func_80062D2C(id,owner,0u,1u);list=func_8006322C(id,window,window);
    PE_StoreU32(window+44u,0x80044B0Cu);PE_StoreU32(list+48u,0x8004F910u);func_80062CB8(list);
    if (group==1u) unavailable=!func_80052558() || !func_800562A4(index) || func_8005257C();
    PE_StoreU32(0x8009CF08u,(uint32_t)unavailable);func_80055760();
    if (!group) {
        record=func_8005332C(index);
        if (func_80055FE0(index)) enabled=PE_LoadU32(0x8009CF0Cu)!=1u ||
            PE_LoadU8(item_ram(record+6u))!=10u || PE_LoadU8(item_ram(record+14u))<4u;
    }
    if ((!group && !enabled) || unavailable) PE_StoreU32(list+72u,1u);
}

void func_800451D0(pe_addr_t owner)
{
    pe_addr_t window=func_80062D2C(62u,owner,0u,1u),record;
    uint32_t bank,index;
    PE_StoreU32(window+44u,0x800452C0u);PE_StoreU32(window+48u,0x800453E8u);
    PE_StoreU32(window+40u,1u);func_80062CB8(window);
    func_80052E30(PE_LoadU32(0x8009CF88u));record=func_8005332C((int32_t)PE_LoadU32(0x8009CF8Cu));
    if (record && (uint32_t)(PE_LoadU8(record+6u)-19u)>=3u) {
        bank=PE_LoadU32(0x8009CF88u);index=PE_LoadU32(0x8009CF8Cu);
        PE_StoreU32(0x8009CF88u,PE_LoadU32(0x8009CF90u));PE_StoreU32(0x8009CF8Cu,PE_LoadU32(0x8009CF94u));
        PE_StoreU32(0x8009CF90u,bank);PE_StoreU32(0x8009CF94u,index);
    }
    func_80056C40(PE_LoadU32(0x8009CF88u),(int32_t)PE_LoadU32(0x8009CF8Cu),
        PE_LoadU32(0x8009CF90u),(int32_t)PE_LoadU32(0x8009CF94u));
    func_80055724();PE_StoreU32(0x8009CFACu,0u);
}

int func_80044444(pe_addr_t window,uint32_t event)
{
    pe_addr_t list=func_80062A20(window,0u),record,other;int32_t index;uint32_t id=PE_LoadU32(window+36u);
    func_80052E30(id==13u || id==52u);
    if (PE_LoadU32(0x8009CF00u)) {
        if (event&0x10000u) {
            uint32_t value=0u;index=func_80063428(list);record=func_8005332C(index);
            if (!func_80055FE0(index)) {func_800526C4();return 0;}
            if (record) value=(uint32_t)(PE_LoadU8(record+6u)<10u?func_80057D18(index):func_80057D30(index));
            PE_MenuCommitResult(value+3u);
            if (PE_LoadU32(0x8009CF5Cu)) func_80053D2C(PE_LoadU32(0x8009CF5Cu));
            PE_StoreU32(0x8009CF00u,0u);func_800525EC();
        } else if (event&64u) {func_800512AC(9,0u);func_80052634();}
        return 0;
    }
    if (event&0x10000u) {
        index=func_80063428(list);if (id==51u) index=func_80058E08(index);
        if ((int32_t)PE_LoadU32(0x8009CF8Cu)>=0) {
            PE_StoreU32(0x8009CF90u,func_80052F0C());PE_StoreU32(0x8009CF94u,(uint32_t)index);
            if (func_80055FE0(index)) func_800451D0(list);
        } else if (func_80063428(list)>=0 && func_8005332C(index) && !PE_LoadU32(0x8009CEFCu)) {
            if ((int32_t)PE_LoadU32(list+76u)>=0) PE_StoreU32(list+76u,UINT32_MAX);
            func_80044924(list,id==13u || id==52u,index);
        } else if (func_8005415C(index)>=16 && func_8005415C(index)<19) {
            if (func_8005833C(index)) {func_800526C4();return 1;}
        } else {
            PE_StoreU32(list+76u,PE_LoadU32(list+68u));PE_StoreU32(list+80u,PE_LoadU32(list+72u));
        }
        func_800525EC();return 1;
    }
    if (!(event&64u)) return 0;
    if ((int32_t)PE_LoadU32(0x8009CF8Cu)>=0) {func_80055760();PE_StoreU32(0x8009CF8Cu,UINT32_MAX);}
    else if ((int32_t)PE_LoadU32(list+76u)<0) {
        if (id-13u<2u) {PE_StoreU32(list+68u,UINT32_MAX);func_80062CB8(func_80062A34(2u,12u));}
        else if (id==1u) {
            func_80062F1C(window);func_80062F1C(func_80062A34(1u,27u));func_800439D8();
        } else if (id-51u<2u) {
            PE_StoreU32(list+68u,UINT32_MAX);other=func_80062A34(2u,50u);
            PE_StoreU32(other+68u,0u);func_80062CB8(other);
        }
    }
    func_80052634();return 1;
}
