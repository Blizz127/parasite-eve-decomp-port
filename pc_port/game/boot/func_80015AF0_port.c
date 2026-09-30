/* Original E7 two-stage menu entry and its constructor.
 * 15AF0..15BAC and4D18C..4D27C; callback execution is a separate graph. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

extern void func_8005C1EC(int enabled);
extern void func_80042538(void);

void func_8004D18C(void)
{
    pe_addr_t owner=func_80062CC4();
    pe_addr_t window=func_80062D2C(36u,owner,0u,0u);
    pe_addr_t list=func_8006322C(36u,window,window);
    PE_StoreU32(window+44u,0x8004D2DCu);
    PE_StoreU32(list+48u,0x8004FDE8u);PE_StoreU32(list+140u,0x8004FDA4u);
    if(PE_LoadU32(list+72u)==2u)PE_StoreU32(list+72u,0u);
    func_80062CB8(list);
    if(!func_80062A34(1u,19u)) {
        pe_addr_t help=func_80062D2C(19u,0u,0u,0u);
        PE_StoreU32(help+48u,0x8004C608u);
    }
    PE_StoreU32(func_80062A34(1u,19u)+56u,36u);
    PE_StoreU32(0x8009CF50u,1u);
    func_8005C1EC(1);func_80042538();func_8005DE88();
}

/* func_80015AF0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80015AF0_port.c (src/func_80015AF0.c); hand port retired (switch1 lane, audit klass a-replaceable, generator-verified eligible). */

/* func_80042AD8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042AD8_port.c (src/func_80042AD8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80042910: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042910_port.c (src/func_80042910.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_800428C4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800428C4_port.c (src/func_800428C4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80042770: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042770_port.c (src/func_80042770.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_8004FDA4(uint32_t index)
{
    return index==2u || func_80042770(index)!=0u;
}

/* func_80050C08: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050C08_port.c (src/func_80050C08.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_8004FDE8(pe_addr_t list)
{
    unsigned epoch=PE_Port_StopEpoch();
    PE_StoreU32(0x8009CEF4u,list);
    func_800638D8(list,0x80050C08u);
    if(PE_Port_StopEpoch()!=epoch)return;
    func_8005EB58(1);
    uint32_t count=PE_LoadU32(list+56u);
    while(count) {
        func_8005EB64(0x68u);
        if(PE_Port_StopEpoch()!=epoch)return;
        func_8005E8A4(0,16);count--;
    }
}

/* func_80042B28: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042B28_port.c (src/func_80042B28.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80062CD0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80062CD0_port.c (src/func_80062CD0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8004298C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004298C_port.c (src/func_8004298C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
void func_80042A10(void)
{
    /* Identical two-record cleanup loop to42798, followed by these resets. */
    unsigned epoch=PE_Port_StopEpoch();
    func_80042798();
    if(PE_Port_StopEpoch()!=epoch)return;
    PE_StoreU8(0x800A12EDu,0);PE_StoreU8(0x800A0ED5u,0);PE_StoreU32(0x800A1838u,0);
}

void func_8004DAA4(void)
{
    unsigned epoch=PE_Port_StopEpoch();
    if(PE_LoadU32(0x8009CF50u)) {
        if(func_80062A34(1u,42u))return;
        pe_addr_t owner=func_80062A34(2u,36u);
        pe_addr_t text=func_8005DC4C(func_800428C4()+0x47u);
        pe_addr_t window=func_80062D2C(42u,owner,0u,1u);
        pe_addr_t list=func_8006322C(42u,window,window);
        PE_StoreU32(window+48u,0x80044E14u);PE_StoreU32(window+44u,0x80044E98u);
        PE_StoreU32(list+48u,0x8004F950u);PE_StoreU32(0x8009CF14u,0x6Cu);
        func_80062CD0(list);PE_StoreU32(list+68u,1u);
        func_80052E30(PE_LoadU32(0x8009CF10u));
        if(PE_Port_StopEpoch()!=epoch)return;
        if(text)func_80052BCC(0x800A19C0u,text);
        else PE_StoreU8(0x800A19C0u,255u);
        if(PE_Port_StopEpoch()!=epoch)return;
        func_80052C08(0x800A19C0u,func_8005DC4C(0x49u));
        if(PE_Port_StopEpoch()!=epoch)return;
        PE_StoreU32(0x8009CFA4u,0x4Au);
        uint32_t width=func_8005F1A0(0x800A19C0u);
        if(PE_Port_StopEpoch()!=epoch)return;
        width=(int32_t)width<120?120u:func_8005F1A0(0x800A19C0u);
        if(PE_Port_StopEpoch()!=epoch)return;
        PE_StoreU32(window+52u,width+20u);PE_StoreU32(window+56u,66u);
        uint32_t x=300u-width;
        uint32_t saved_width=PE_LoadU32(window+52u);
        PE_StoreU32(window+24u,(uint32_t)((int32_t)x>>1));
        PE_StoreU32(list+24u,(uint32_t)((int32_t)(saved_width-128u)>>1));
        uint32_t height=PE_LoadU32(window+56u);
        PE_StoreU32(0x8009CFA8u,0x80050580u);PE_StoreU32(list+28u,height-20u);
    } else {
        if(func_80062A34(1u,40u))return;
        func_8004CC50(func_800428C4()+0x47u,0x49u);
        if(PE_Port_StopEpoch()!=epoch)return;
        PE_StoreU32(0x8009CFFCu,0x80042910u);
    }
}

/* func_80042848: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042848_port.c (src/func_80042848.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_8004D2DC(pe_addr_t window,uint32_t event)
{
    unsigned epoch=PE_Port_StopEpoch();
    int32_t selection=func_8006346C(func_80062A20(window,0u));
    if(event&0x10000u) {
        if(selection<0 || selection>2) {func_800526C4();return 1;}
        if(selection<2) {
            int ready=func_80042848((uint32_t)selection);
            if(PE_Port_StopEpoch()!=epoch || !ready)return 1;
            if(!func_80042770((uint32_t)selection) || func_80042B28())return 1;
            func_80042A10();if(PE_Port_StopEpoch()!=epoch)return 1;
            func_8004298C((uint32_t)selection,1u);if(PE_Port_StopEpoch()!=epoch)return 1;
            func_800525EC();return 1;
        }
    } else if(!(event&0x40u))return 1;
    const uint32_t ids[]={37,38,36,19};
    for(unsigned i=0;i<4;i++) {
        func_80062F1C(func_80062A34(1u,ids[i]));
        if(PE_Port_StopEpoch()!=epoch)return 1;
    }
    func_8005C1EC(0);if(PE_Port_StopEpoch()!=epoch)return 1;
    func_800512AC(9,0);if(PE_Port_StopEpoch()!=epoch)return 1;
    PE_StoreU32(0x8009CFF8u,0);
    if(event&0x10000u)func_800525EC();else func_80052634();
    return 1;
}

/* Card confirmation, progress display and delayed record transition. */
/* func_800428D4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800428D4_port.c (src/func_800428D4.c); hand port retired (port3 switch-over D). */

/* func_80042B50: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042B50_port.c (src/func_80042B50.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8004DA9C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004DA9C_port.c (src/func_8004DA9C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80042464: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042464_port.c (src/func_80042464.c); hand port retired (port3 switch-over D). */

/* func_8004DA04: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004DA04_port.c (src/func_8004DA04.c); hand port retired (port3 switch-over D). */

/* func_8004CFD4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004CFD4_port.c (src/func_8004CFD4.c); hand port retired (port3 switch-over D). */

void func_8004CE28(uint32_t first,uint32_t second)
{
    unsigned epoch=PE_Port_StopEpoch();
    pe_addr_t window=func_80062D2C(40u,func_80062CC4(),0u,1u);
    pe_addr_t list=func_8006322C(40u,window,window);
    PE_StoreU32(window+48u,0x8004CFD4u);PE_StoreU32(window+44u,0x8004D030u);
    PE_StoreU32(list+48u,0x8004FFA8u);func_80062CB8(list);func_8004D024(0u);
    func_80052BCC(0x800A1A20u,func_8005DC4C(first));
    if(PE_Port_StopEpoch()!=epoch)return;
    func_80052BCC(0x800A1A60u,func_8005DC4C(second));
    if(PE_Port_StopEpoch()!=epoch)return;
    uint32_t a=func_8005F1A0(0x800A1A20u),b=func_8005F1A0(0x800A1A60u);
    if(PE_Port_StopEpoch()!=epoch)return;
    uint32_t width=func_8005F1A0((int32_t)b<(int32_t)a?0x800A1A20u:0x800A1A60u);
    if(PE_Port_StopEpoch()!=epoch)return;
    if((int32_t)width<100)width=100u;
    else {
        a=func_8005F1A0(0x800A1A20u);b=func_8005F1A0(0x800A1A60u);
        if(PE_Port_StopEpoch()!=epoch)return;
        width=func_8005F1A0((int32_t)b<(int32_t)a?0x800A1A20u:0x800A1A60u);
        if(PE_Port_StopEpoch()!=epoch)return;
    }
    PE_StoreU32(window+52u,width+20u);
    uint32_t height=PE_LoadU32(window+56u);
    PE_StoreU32(window+24u,(uint32_t)((int32_t)(300u-width)>>1));
    width=PE_LoadU32(window+52u);
    PE_StoreU32(window+56u,height+14u);
    uint32_t y=PE_LoadU32(list+28u);
    PE_StoreU32(list+24u,width-68u);PE_StoreU32(list+28u,y+14u);
    func_8005DE88();
}

void func_80050580(pe_addr_t unused,uint32_t confirmed)
{
    (void)unused;
    unsigned epoch=PE_Port_StopEpoch();
    if(confirmed) {
        pe_addr_t window=func_80062D2C(39u,func_80062CC4(),0u,1u);
        PE_StoreU32(window+48u,0x8004DA04u);PE_StoreU32(window+44u,0x8004DA9Cu);
        func_80062CB8(window);PE_StoreU32(0x8009D000u,0x40u);
        func_80042B50(0x800428D4u);
    } else {
        func_8004CE28(func_800428C4()+0x47u,0x4Bu);
        if(PE_Port_StopEpoch()!=epoch)return;
        PE_StoreU32(0x8009CFFCu,0x80042910u);
    }
}
