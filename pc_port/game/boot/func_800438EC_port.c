/* Original inventory entry and window construction.
 * 340EC.s / 3C740.s / 4C974.s. Drawing/input callbacks are dispatched
 * separately; construction does not replace their original function IDs. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

extern unsigned int func_80052F70(void);

/* func_80033A20: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80033A20_port.c (src/func_80033A20.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8005B89C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005B89C_port.c (src/func_8005B89C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8005257C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005257C_port.c (src/func_8005257C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_8004C594(void)
{
    if (!func_80062A34(1u,19u)) {
        pe_addr_t help=func_80062D2C(19u,0u,0u,0u);
        PE_StoreU32(help+48u,0x8004C608u);
    }
}

void func_800439D8(void)
{
    pe_addr_t node;
    unsigned i;
    node=func_80062D2C(18u,0u,0u,0u);
    PE_StoreU32(node+48u,0x80043B0Cu);PE_StoreU32(node+76u,0x80092258u);
    node=func_80062D2C(24u,0u,0u,0u);PE_StoreU32(node+48u,0x8004905Cu);
    node=func_80062D2C(45u,0u,0u,0u);
    PE_StoreU32(node+48u,0x80043C64u);PE_StoreU32(node+76u,0x80092298u);
    for (i=0;i<7;i++) {
        uint32_t value=PE_LoadU16(0x800C0E28u+i*2u);
        PE_StoreU32(0x800A18D8u+i*4u,value);PE_StoreU32(0x800A18FCu+i*4u,0u);
        func_8005B91C((int32_t)i,(int32_t)value,0x800A18B4u+i*4u,0u);
    }
    PE_StoreU32(0x8009CF68u,PE_LoadU32(0x800C0E10u));
}

void func_800438EC(void)
{
    pe_addr_t window,node;
    uint32_t enabled,count=0u;
    unsigned i;
    if (func_80062A34(1u,0u)) return;
    window=func_80062D2C(0u,0u,0u,0u);node=func_8006322C(0u,window,window);
    PE_StoreU32(window+44u,0x80043DA4u);PE_StoreU32(node+48u,0x8004F838u);func_80062CB8(node);
    enabled=PE_LoadU32(0x8009CEF0u)&(func_8005B89C()?0x1Fu:0x1EFu);
    for (i=0;i<9;i++) {count+=enabled&1u;enabled>>=1u;}
    func_800647D0(node,(int32_t)count);
    PE_StoreU32(0x8009CEFCu,0u);PE_StoreU32(0x8009CEF8u,1u);
    func_800439D8();func_8004C594();
}

void func_8005C174(int32_t battle)
{
    PE_StoreU32(0x8009D028u,(uint32_t)battle);
    PE_StoreU32(0x8009D02Cu,func_80033A20());func_800339A0(0u);func_80051510();
    D_8009D048=0x800C0E48u;PE_StoreU32(0x8009D048u,D_8009D048);
    D_8009D050=func_80052F70();PE_StoreU32(0x8009D050u,D_8009D050);
    D_8009D058=0x8009D05Cu;PE_StoreU32(0x8009D058u,D_8009D058);
    D_8009D064=2u;PE_StoreU32(0x8009D064u,D_8009D064);
    func_800438EC();func_800525EC();
}

void func_80052E30(uint32_t a0);
int func_80053F20(int a0);
int func_80053F90(int a0);
void func_80048918(int a0, int a1, int a2);
/* Main menu dispatch (src/func_80043DA4.c): all nine commands. */
int func_80043DA4(pe_addr_t window,uint32_t event)
{
    if (event&0x10000u) {
        pe_addr_t list=func_80062A20(window,0u);
        int32_t selected=func_80063428(list),command=-1;
        uint32_t enabled=PE_LoadU32(0x8009CEF0u)&(func_8005B89C()?0x1Fu:0x1EFu);
        while (command<9 && selected>=0) {
            selected=(int32_t)((uint32_t)selected-(enabled&1u));enabled>>=1u;command++;
        }
        if ((uint32_t)command>=9u) return 1;
        if (command==4 && !func_80052534()) {func_800526C4();return 1;}
        func_80062F3C(45u);func_80062F3C(24u);func_80062F3C(18u);
        if (!command) func_80044174(list);
        else if (command==1) func_80046ABC(list);
        else if (command==2 || command==3) {
            PE_StoreU32(0x8009CF18u,command==2);func_80045EE4(list);func_8004542C(list);
            func_80046378(func_80062A34(2u,5u),0u);
        }
        else if (command==4) func_800512AC(8,0u);
        else if (command==5) func_8004AD9C(list);
        else if (command==6) func_80046DFC(list,0u);
        else if (command==7) {
            /* Audit item 28: src/func_80043DA4.c case 7 (status page). */
            PE_StoreU32(0x8009CF1Cu,1u);
            func_80052E30(0u);
            PE_StoreU32(0x800A1888u,(uint32_t)(func_80053F20(14)?999:func_80053F20(12)));
            PE_StoreU32(0x800A188Cu,(uint32_t)(func_80053F20(15)?999:func_80053F20(13)));
            if (PE_LoadU32(0x8009CF0Cu)) {
                PE_StoreU32(0x800A1890u,(uint32_t)(func_80053F90(14)?999:func_80053F90(12)));
                PE_StoreU32(0x800A1894u,(uint32_t)(func_80053F90(15)?999:func_80053F90(13)));
            } else {
                PE_StoreU32(0x800A1894u,0u);PE_StoreU32(0x800A1890u,0u);
            }
            func_80048918(830,-1,-1);
            PE_StoreU32(0x8009CEF8u,0u);
        } else {
            /* case 8 */
            PE_StoreU32(0x8009CF30u,1u);
            func_80048918(894,-1,-1);
            PE_StoreU32(0x8009CEF8u,0u);
        }
        func_800525EC();return 1;
    }
    if (event&64u) {
        func_80062F9C();func_800512AC(9,0u);
        if (!func_8005B89C()) func_80051244();
        func_80052634();return 1;
    }
    if (event&2u) func_8005D970();
    return 0;
}
