/* Original Parasite Energy page, confirmation, gauge and command reservation.
 * Authority:340EC/401A0/40F48/44AA0/467E0/50074.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

static pe_addr_t pe_menu_ram(pe_addr_t p) {return p<0x200000u?p|0x80000000u:p;}

/* func_8006346C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006346C_port.c (src/func_8006346C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80046ABC(pe_addr_t owner)
{
    pe_addr_t window=func_80062D2C(8u,owner,0u,0u),list=func_8006322C(8u,window,window);
    PE_StoreU32(window+44u,0x80046B58u);PE_StoreU32(list+48u,0x8004FC80u);
    PE_StoreU32(list+140u,0x8004FC3Cu);func_80062CB8(list);
    func_80055610();func_800647D0(list,func_80054288());
}

/* func_80046B58: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80046B58_port.c (src/func_80046B58.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80046C20(void)
{
    pe_addr_t owner,window,list,name;uint32_t width;
    PE_StoreU32(0x8009CFB4u,(uint32_t)func_800556E8(func_80063428(func_80062A34(2u,8u))));
    owner=func_80062CC4();name=func_8005DC9C(PE_LoadU32(0x8009CFB4u)+235u);
    window=func_80062D2C(41u,owner,0u,1u);list=func_8006322C(41u,window,window);
    PE_StoreU32(window+48u,0x80044E14u);PE_StoreU32(window+44u,0x80044E98u);
    PE_StoreU32(list+48u,0x8004F950u);PE_StoreU32(0x8009CF14u,5u);func_80062CB8(list);
    func_80052E30(PE_LoadU32(0x8009CF10u));
    if (name) func_80052BCC(0x800A1980u,name);else PE_StoreU8(0x800A1980u,255u);
    func_80052C08(0x800A1980u,func_8005DC4C(26u));PE_StoreU32(0x8009CFA0u,0u);
    width=func_8005F1A0(0x800A1980u);width=(int32_t)width<120?120u:func_8005F1A0(0x800A1980u);
    PE_StoreU32(window+52u,width+20u);PE_StoreU32(window+56u,50u);
    PE_StoreU32(window+24u,(uint32_t)((int32_t)(300u-width)>>1));
    PE_StoreU32(list+24u,(uint32_t)((int32_t)(PE_LoadU32(window+52u)-128u)>>1));
    PE_StoreU32(0x8009CFA8u,0x80046DBCu);PE_StoreU32(list+28u,PE_LoadU32(window+56u)-20u);
}

void func_80057B70(int32_t ability)
{
    pe_addr_t item=func_8005DB44((uint32_t)ability+235u);
    if (PE_LoadU32(0x8009D028u)) {
        int32_t cost=func_800579D4(ability,func_800515F8(0u));
        pe_addr_t history=pe_menu_ram(func_80051098());
        PE_StoreU32(history,1u);PE_StoreU32(history+4u,(uint32_t)ability);PE_StoreU32(history+8u,(uint32_t)cost);
        PE_MenuCommitResult((uint32_t)ability+387u);
    } else if (PE_LoadU8(pe_menu_ram(item+14u))==1u) func_80051770(ability);
}

/* func_80046DBC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80046DBC_port.c (src/func_80046DBC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8004FC3C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004FC3C_port.c (src/func_8004FC3C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80050B48(uint32_t index)
{if ((int32_t)index<func_80054288()) func_8005F27C(func_8005DC9C((uint32_t)func_800556E8((int32_t)index)+235u));}

static void pe_gauge_quad(pe_addr_t p,int32_t left,int32_t right,int32_t top,int32_t bottom,
                          uint32_t first,uint32_t second)
{
    unsigned i;static const unsigned offsets[]={4u,12u,20u,28u};
    if (p) {PE_StoreU32(p+4u,0x38808080u);PE_StoreU8(p+3u,8u);}
    p=pe_menu_ram(p);
    for (i=0;i<4u;i++) {
        uint32_t c=i&1u?second:first;unsigned off=offsets[i];
        PE_StoreU8(p+off,(uint8_t)c);PE_StoreU8(p+off+1u,(uint8_t)(c>>8u));PE_StoreU8(p+off+2u,(uint8_t)(c>>16u));
        PE_StoreU16(p+off+4u,(uint16_t)(i&1u?right:left));
        PE_StoreU16(p+off+6u,(uint16_t)(i<2u?top:bottom));
    }
}

void func_80061044(int32_t value,int32_t maximum)
{
    int32_t y=func_8005257C()?0:7,width=0,clamped=value<maximum?value:maximum;
    pe_addr_t packet=PE_MenuPacketAlloc(36u),p;
    if (maximum>0 && clamped>=0) width=(int32_t)((uint32_t)clamped*56u)/maximum;
    pe_gauge_quad(packet,229,229+width,y+170,y+173,0x368200u,0x3BFF4Au);PE_MenuPacketLink(packet);
    packet=PE_MenuPacketAlloc(36u);
    pe_gauge_quad(packet,229+width,285,y+170,y+173,0x813DFFu,0x011383u);
    PE_StoreU32(0x8009D110u,0x303030u);PE_StoreU32(0x8009D114u,0x181818u);PE_StoreU32(0x8009D10Cu,0u);
    PE_MenuPacketLink(packet);packet=PE_MenuPacketAlloc(16u);p=pe_menu_ram(packet);
    if (packet) {PE_StoreU32(p+4u,0x60303030u);PE_StoreU8(p+3u,3u);}
    PE_StoreU16(p+8u,221u);PE_StoreU16(p+10u,(uint16_t)(y+166));PE_StoreU16(p+12u,84u);PE_StoreU16(p+14u,11u);
    PE_StoreU8(p+7u,PE_LoadU8(p+7u)|2u);PE_MenuPacketLink(packet);
    packet=PE_MenuPacketAlloc(8u);if (packet) func_80077C84(packet,0u,0u,0u);
    PE_StoreU32(0x8009D110u,0x808080u);PE_StoreU32(0x8009D114u,0x404040u);PE_MenuPacketLink(packet);
}

void func_8004FC80(pe_addr_t list)
{
    int32_t ability=func_800556E8(func_80063428(list)),maximum=0;
    int32_t available=PE_MenuPEValues(&maximum),cost=func_800579D4(ability,available);
    func_80061044((int32_t)((uint32_t)available-(uint32_t)cost),maximum);func_800638D8(list,0x80050B48u);
}
