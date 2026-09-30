/* Original ammunition transfer, confirmation and command history.
 * 467E0.s / 340EC.s / 41898.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

static pe_addr_t ammo_ram(pe_addr_t p) {return p<0x200000u?p|0x80000000u:p;}
/* func_8005E120: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005E120_port.c (src/func_8005E120.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

pe_addr_t func_80051098(void)
{
    pe_addr_t entry=PE_LoadU32(0x8009D014u);
    if (entry<0x800A1B30u) PE_StoreU32(0x8009D014u,entry+36u);
    else {
        for (entry=0x800A1AA0u;entry<0x800A1B0Cu;entry+=36u) {
            uint32_t offset;
            for (offset=0;offset<32u;offset+=16u) {
                uint32_t values[4],i;
                for (i=0;i<4;i++) values[i]=PE_LoadU32(entry+36u+offset+i*4u);
                for (i=0;i<4;i++) PE_StoreU32(entry+offset+i*4u,values[i]);
            }
            PE_StoreU32(entry+32u,PE_LoadU32(entry+68u));
        }
    }
    return entry;
}

/* func_80056B24: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80056B24_port.c (src/func_80056B24.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80057094(void)
{
    pe_addr_t first=PE_LoadU32(0x8009D070u),second=PE_LoadU32(0x8009D074u),history,extra=0u,record;
    uint32_t reserve;
    if (PE_LoadU16(0x800A1F9Eu)==PE_LoadU16(ammo_ram(first+10u)) &&
        PE_LoadU16(0x800A1FBEu)==PE_LoadU16(ammo_ram(second+10u))) return;
    history=func_80051098();PE_StoreU32(ammo_ram(history),4u);PE_StoreU32(ammo_ram(history+12u),0u);
    PE_StoreU32(ammo_ram(history+4u),PE_LoadU32(0x8009D070u));
    PE_StoreU32(ammo_ram(history+8u),PE_LoadU32(0x8009D074u));
    reserve=PE_LoadU16(0x800A1FA0u);
    if (reserve) extra=0x800A1E44u+PE_LoadU8(0x800A1FB3u)*32u;
    else if ((reserve=PE_LoadU16(0x800A1FC0u))!=0u) extra=0x800A1E44u+PE_LoadU8(0x800A1FD3u)*32u;
    if (extra) {
        uint32_t amount=PE_LoadU16(extra+10u)+reserve;
        int32_t capacity=PE_LoadU8(extra+9u)+(int16_t)PE_LoadU16(extra+18u);
        PE_StoreU32(ammo_ram(history+12u),extra);PE_StoreU32(ammo_ram(history+32u),PE_LoadU16(extra+10u));
        if (capacity>=1000) capacity=999;
        if (capacity<(int32_t)amount) amount=(uint32_t)capacity;
        PE_StoreU16(extra+10u,(uint16_t)amount);
    }
    first=PE_LoadU32(0x8009D070u);second=PE_LoadU32(0x8009D074u);
    PE_StoreU32(ammo_ram(history+20u),PE_LoadU16(ammo_ram(first+10u)));
    PE_StoreU32(ammo_ram(history+28u),PE_LoadU16(ammo_ram(second+10u)));
    PE_CopyItemRecord(first,0x800A1F94u);PE_CopyItemRecord(PE_LoadU32(0x8009D074u),0x800A1FB4u);
    record=func_8005332C((int8_t)PE_LoadU8(0x800C0E20u));
    if (record==PE_LoadU32(0x8009D070u) || record==PE_LoadU32(0x8009D074u)) PE_MenuApplyAmmo(record);
    func_800512AC(7,0u);
}

void func_80056FB8(void)
{
    unsigned i;
    for (i=0;i<2;i++) {
        pe_addr_t record=0x800A1F94u+i*32u,name;
        if (i) func_8005E8A4(0,24);
        if (PE_LoadU8(record+5u)&16u) name=PE_LoadU8(record+6u)==9u?0x800C20B4u:0x800C20A4u;
        else name=func_8005DC9C(PE_LoadU8(record+4u)-1u);
        func_800534E4(record,name);
    }
}
/* func_800453E8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800453E8_port.c (src/func_800453E8.c); hand port retired (port3 switch-over L). */

/* func_800452C0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800452C0_port.c (src/func_800452C0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80044274(int32_t index)
{
    int32_t count;pe_addr_t alternate,primary;
    PE_StoreU32(0x8009CF88u,func_80052F0C());PE_StoreU32(0x8009CF8Cu,(uint32_t)index);
    count=func_800562A4(index);
    if (!count) {PE_StoreU32(0x8009CF8Cu,UINT32_MAX);func_80055760();return;}
    alternate=func_80062A34(2u,13u);primary=func_80062A34(2u,14u);
    if (alternate && func_80063428(alternate) && primary) {
        int32_t i;
        PE_StoreU32(alternate+68u,UINT32_MAX);func_80052E30(0u);
        for (i=0;i<(int32_t)func_80052F70();i++) if (func_80055FE0(i)) break;
        PE_StoreU32(primary+68u,0u);PE_StoreU32(primary+72u,i<(int32_t)func_80052F70()?(uint32_t)i:0u);
        func_80062CB8(primary);
    }
    if (count==1) {
        PE_StoreU32(0x8009CF90u,PE_LoadU32(0x8009CF88u));func_8005600C(0x8009CF90u,0x8009CF94u);
        func_800451D0(func_80062CC4());
    }
}
