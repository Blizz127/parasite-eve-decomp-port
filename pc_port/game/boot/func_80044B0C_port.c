/* Original Use / Move / Discard / Reload dispatch and ordinary item use.
 * 340EC.s / 467E0.s / 41A58.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

static pe_addr_t use_ram(pe_addr_t p) {return p<0x200000u?p|0x80000000u:p;}
static void use_boundary(const char *name)
{
    Bootstrap_ReturnVoid(name,"func_80057834");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

void func_800516B4(int32_t item)
{
    pe_addr_t aya,record,weapon;uint32_t bank;
    func_80023E14(item);
    aya=PE_LoadU32(0x8009D254u);if (!aya) return;
    record=PE_LoadU32(aya);if (!record) return;
    PE_StoreU16(0x800C0E08u,PE_LoadU16(record+12u));
    if (!PE_LoadU32(record+104u)) return;
    bank=func_80052F0C();func_80052E30(0u);
    weapon=func_8005332C((int8_t)PE_LoadU8(0x800C0E20u));
    if (weapon) PE_StoreU16(weapon+10u,PE_LoadU32(PE_LoadU32(record+104u)+12u)&0x3FFu);
    func_80052E30(bank);
}

void func_80057834(int32_t index)
{
    pe_addr_t record=func_8005332C(index);
    if (PE_LoadU32(0x8009D028u)) {
        pe_addr_t history=func_80051098(),slot=D_8009D048+(uint32_t)index*2u;
        PE_StoreU32(use_ram(history),0u);
        PE_StoreU32(use_ram(history+8u),(uint32_t)index);
        PE_StoreU32(use_ram(history+4u),(uint32_t)(int32_t)(int16_t)PE_LoadU16(use_ram(slot)));
        PE_MenuCommitResult((uint32_t)(int32_t)(int16_t)PE_LoadU16(use_ram(slot))+3u);
        func_80057D30(index);
    } else {
        unsigned subtype=PE_LoadU8(use_ram(record+14u));
        if (subtype==1u) {
            func_800516B4((int16_t)PE_LoadU16(use_ram(D_8009D048+(uint32_t)index*2u)));
            func_80057D30(index);func_80055760();
        } else if (subtype==2u) use_boundary("func_8004F23C");
        else if ((subtype>=4u && subtype<=6u) || (subtype>=12u && subtype<=14u))
            use_boundary("func_80048918");
    }
}

int func_80044B0C(pe_addr_t window,uint32_t event)
{
    pe_addr_t list,record;uint32_t action;int32_t selected;
    func_80052E30(PE_LoadU32(0x8009CF10u));
    if (!(event&0x10000u)) {
        if (!(event&64u)) return 0;
        func_80062F1C(window);func_80052634();return 1;
    }
    list=func_80062A20(window,0u);
    action=PE_LoadU32(use_ram(0x80092234u+PE_LoadU32(0x8009CDA8u)*12u+
        (uint32_t)func_80063428(list)*4u));
    selected=(int32_t)PE_LoadU32(0x8009CF04u);
    switch (action) {
    case 0:
        record=func_8005332C(selected);
        if (!func_80055FE0(selected) || (PE_LoadU32(0x8009CF0Cu)==1u &&
            PE_LoadU8(use_ram(record+6u))==10u && PE_LoadU8(use_ram(record+14u))>=4u)) {
            func_800526C4();return 1;
        }
        func_80062F1C(window);
        if (PE_LoadU32(0x8009CF0Cu)==2u) {func_80062F3C(51u);func_80062F3C(52u);}
        if (func_8005B89C()) func_80062F1C(PE_LoadU32(use_ram(func_80062CC4()+4u)));
        func_80057834((int32_t)PE_LoadU32(0x8009CF04u));func_800525EC();return 1;
    case 1:
        list=PE_LoadU32(window+4u);
        PE_StoreU32(use_ram(list+76u),PE_LoadU32(use_ram(list+68u)));
        PE_StoreU32(use_ram(list+80u),PE_LoadU32(use_ram(list+72u)));
        if (!PE_LoadU32(0x8009CF0Cu)) {
            func_80055724();func_80055FB4((int32_t)(PE_LoadU32(use_ram(list+84u))*
                PE_LoadU32(use_ram(list+72u))+PE_LoadU32(use_ram(list+68u))));
        }
        func_80062F1C(window);func_800525EC();return 1;
    case 2:
        if (!func_80057654(selected)) {func_800526C4();return 1;}
        if (!func_80052F0C() && (int8_t)PE_LoadU8(0x800C0E22u)==(int32_t)PE_LoadU32(0x8009CF04u) &&
            !func_80059A40(0u)) func_8004CC50(29u,0u);
        else func_80044F8C();
        func_800525EC();return 1;
    case 3:
        if (PE_LoadU32(0x8009CF08u)) {func_800526C4();return 1;}
        func_80062F1C(window);func_80044274((int32_t)PE_LoadU32(0x8009CF04u));
        func_800525EC();return 1;
    default:return 1;
    }
}
