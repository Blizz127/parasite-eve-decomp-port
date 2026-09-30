/* Original item-action labels and confirmation dialogs.
 * 4F6D4.s / 340EC.s / 43724.s / 3D830.s / 486D8.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_guestcode.h"
#include <stdio.h>

void func_8005F354(pe_addr_t text,int32_t width)
{
    uint32_t measured;
    func_8005E8C4();measured=func_8005F1A0(text);
    func_8005E8A4((int32_t)((uint32_t)width-measured-4u)>>1,0);
    func_8005F27C(text);func_8005E914();
}
/* func_8005F594: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005F594_port.c (src/func_8005F594.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80064C30: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80064C30_port.c (src/func_80064C30.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
void func_80064C54(uint32_t id) {func_80064C30(func_8005DC4C(id));}
/* func_80062A7C: shortcut retired; line-by-line port of src/func_80062A7C.c (with the draw-state stack push/pop and retail $v0) in pc_port/game/decomp_hand/func_80044E14_port.c (port3 switch-over batch O). */

pe_addr_t func_80053068(int32_t index)
{
    int32_t id=(int16_t)PE_LoadU16(D_8009D048+(uint32_t)index*2u);
    if ((uint32_t)(id-256)<128u) {
        pe_addr_t record=0x800BEEACu+(uint32_t)id*32u;
        if (PE_LoadU8(record+5u)&16u) return PE_LoadU8(record+6u)==9u?0x800C20B4u:0x800C20A4u;
        return func_8005DC9C(PE_LoadU8(record+4u)-1u);
    }
    if ((uint32_t)(id-1)<255u) return func_8005DC9C((uint32_t)(id-1));
    if ((uint32_t)(id-512)<9u) return func_8005DC9C(D_8009D03C+(uint32_t)id-513u);
    return 0u;
}

/* func_80052BCC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80052BCC_port.c (src/func_80052BCC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80052C08: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80052C08_port.c (src/func_80052C08.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80050878(uint32_t index)
{
    uint32_t action=PE_LoadU32(0x80092234u+PE_LoadU32(0x8009CDA8u)*12u+index*4u);
    int32_t selected=(int32_t)PE_LoadU32(0x8009CF04u);
    if (!action) {
        pe_addr_t record=func_8005332C(selected);int enabled=0;
        if (func_80055FE0(selected)) enabled=PE_LoadU32(0x8009CF0Cu)!=1u ||
            PE_LoadU8(record+6u)!=10u || PE_LoadU8(record+14u)<4u;
        func_8005EB58(!enabled);
    } else if (action==2u) func_8005EB58(!func_80057654(selected));
    else if (action==3u) func_8005EB58(PE_LoadU32(0x8009CF08u));
    func_80064C30(func_8005DC4C(action));
}
/* func_8004F910: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004F910_port.c (src/func_8004F910.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_800509A8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800509A8_port.c (src/func_800509A8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8004F950: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004F950_port.c (src/func_8004F950.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80050CF8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050CF8_port.c (src/func_80050CF8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8004FFA8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004FFA8_port.c (src/func_8004FFA8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800631AC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800631AC_port.c (src/func_800631AC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8004D024: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004D024_port.c (src/func_8004D024.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
void func_8004CDD4(pe_addr_t window)
{func_8005E8A4(0,10);func_8005F594(PE_LoadU32(window+36u)==61u?0x800A1A60u:0x800A1A20u);}

void func_8004CC50(uint32_t first,uint32_t second)
{
    uint32_t nested=func_80062A34(1u,40u)!=0u,id=nested?61u:40u,width;
    pe_addr_t window=func_80062D2C(id,func_80062CC4(),0u,1u),list=func_8006322C(id,window,window);
    pe_addr_t text=0x800A1A20u+nested*64u;
    PE_StoreU32(window+48u,0x8004CDD4u);PE_StoreU32(window+44u,0x8004D030u);
    PE_StoreU32(list+48u,0x8004FFA8u);func_80062CB8(list);func_8004D024(0u);
    func_80052BCC(text,func_8005DC4C(first));if (second) func_80052C08(text,func_8005DC4C(second));
    width=func_8005F1A0(text);width=(int32_t)width<100?100u:func_8005F1A0(text);
    PE_StoreU32(window+52u,width+20u);PE_StoreU32(window+24u,(uint32_t)((int32_t)(300u-width)>>1));
    PE_StoreU32(list+24u,PE_LoadU32(window+52u)-68u);func_8005DE88();
}

int func_8004D030(pe_addr_t window,uint32_t event)
{
    if (event&0x10040u) {
        pe_addr_t callback;
        func_80062F1C(window);callback=PE_LoadU32(0x8009CFFCu);
        if (callback) {
            if (callback==0x80062F9Cu) func_80062F9C();
            else if (callback==0x8005C488u) func_8005C488();
            else if (callback==0x80042910u) func_80042910();
            else if (callback==0x80042928u) {
                unsigned epoch=PE_Port_StopEpoch();func_80042928();
                if(PE_Port_StopEpoch()!=epoch)return 1;
            }
            else if (PE_GuestCode_Resolve(callback)) {
                /* Retail jalr through D_8009CFFC: other code pointers (e.g.
                 * 80042228 after "File loaded") via the guest-code registry. */
                unsigned epoch=PE_Port_StopEpoch();
                (void)PE_GuestCall("func_8004D030",callback,0u,0u,0u,0u,0u);
                if(PE_Port_StopEpoch()!=epoch)return 1;
            }
            else {
                fprintf(stderr,"[MENU] Unported notice callback %08X\n",callback);
                Bootstrap_ReturnVoid("PE_MenuNoticeCallback","func_8004D030");
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 1;
            }
            PE_StoreU32(0x8009CFFCu,0u);
        }
        func_800525EC();
    }
    return 1;
}

/* func_80044E14: moved to pc_port/game/decomp_hand/func_80044E14_port.c (src/func_80044E14.c with the retail $v0 of func_80062A7C; port3 switch-over batch O). */

void func_80044F8C(void)
{
    pe_addr_t owner,window,list,name;uint32_t width;
    func_80052E30(PE_LoadU32(0x8009CF10u));owner=func_80062CC4();
    name=func_80053068((int32_t)PE_LoadU32(0x8009CF04u));
    window=func_80062D2C(41u,owner,0u,1u);list=func_8006322C(41u,window,window);
    PE_StoreU32(window+48u,0x80044E14u);PE_StoreU32(window+44u,0x80044E98u);
    PE_StoreU32(list+48u,0x8004F950u);PE_StoreU32(0x8009CF14u,5u);func_80062CB8(list);
    PE_StoreU32(list+68u,1u);func_80052E30(PE_LoadU32(0x8009CF10u));
    if (name) func_80052BCC(0x800A1980u,name);else PE_StoreU8(0x800A1980u,255u);
    func_80052C08(0x800A1980u,func_8005DC4C(4u));PE_StoreU32(0x8009CFA0u,0u);
    width=func_8005F1A0(0x800A1980u);width=(int32_t)width<120?120u:func_8005F1A0(0x800A1980u);
    PE_StoreU32(window+52u,width+20u);PE_StoreU32(window+56u,50u);
    PE_StoreU32(window+24u,(uint32_t)((int32_t)(300u-width)>>1));
    PE_StoreU32(list+24u,(uint32_t)((int32_t)(PE_LoadU32(window+52u)-128u)>>1));
    PE_StoreU32(0x8009CFA8u,0x80045110u);PE_StoreU32(list+28u,PE_LoadU32(window+56u)-20u);
}

void func_80062CE4(void)
{
    pe_addr_t p=PE_LoadU32(0x8009D154u),saved=PE_LoadU32(0x8009D160u);
    while (p && p!=saved) p=PE_LoadU32(p);
    if (p) PE_StoreU32(0x8009D15Cu,p);PE_StoreU32(0x8009D160u,0u);
}

int32_t func_80058C4C(uint32_t categories)
{
    pe_addr_t next=0x800A1E00u;int32_t i;
    func_80052E30(0u);
    for (i=0;i<(int32_t)D_8009D050;i++) if ((categories>>((uint32_t)func_8005415C(i)&31u))&1u) {
        PE_StoreU16(next,(uint16_t)i);next+=2u;
    }
    PE_StoreU32(0x8009D044u,(next-0x800A1E00u)/2u);func_80055760();
    for (i=0;i<(int32_t)D_8009D050;i++) if (!PE_LoadU16(D_8009D048+(uint32_t)i*2u)) {
        PE_StoreU16(next,(uint16_t)i);next+=2u;
    }
    PE_StoreU32(0x8009D044u,(next-0x800A1E00u)/2u);return (int32_t)((next-0x800A1E00u)/2u);
}

void func_80045110(pe_addr_t window,uint32_t confirmed)
{
    pe_addr_t list,parent;
    if (!confirmed) return;
    func_80052E30(PE_LoadU32(0x8009CF10u));(void)func_80057D30((int32_t)PE_LoadU32(0x8009CF04u));
    func_80055760();parent=PE_LoadU32(window+4u);func_80062F1C(PE_LoadU32(parent+4u));
    list=func_80062A34(2u,1u);if (list) func_800647D0(list,(int32_t)func_80052F70());
    if (PE_LoadU32(0x8009CF0Cu)==2u) {
        list=func_80062A34(2u,51u);if (list) func_800647D0(list,func_80058C4C(0x3803FEu));
    }
}

/* func_80044E98: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80044E98_port.c (src/func_80044E98.c); hand port retired (port3 switch-over M). */
