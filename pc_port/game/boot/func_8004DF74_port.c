/* Original name display, item labels, small digits and naming help.
 * 3E4A4.s / 43CE4.s / 50074.s / naming arms of 3CE08.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>

void func_8005F874(int32_t value)
{
    pe_addr_t packet=PE_MenuPacketAlloc(40u),p=packet?packet:0x80000000u;
    uint32_t x=PE_LoadU16(0x8009D124u),y=PE_LoadU16(0x8009D128u);
    uint32_t u=value<0?88u:PE_LoadU32(0x8009D140u)+(uint32_t)(value%10)*5u;
    uint32_t v=value<0?164u:PE_LoadU32(0x8009D144u);
    PE_MenuPacketColor(packet,9u,0x2Cu);
    PE_StoreU16(p+8u,(uint16_t)x);PE_StoreU16(p+24u,(uint16_t)x);
    PE_StoreU16(p+16u,(uint16_t)(x+5u));PE_StoreU16(p+32u,(uint16_t)(x+5u));
    PE_StoreU16(p+10u,(uint16_t)y);PE_StoreU16(p+18u,(uint16_t)y);
    PE_StoreU16(p+26u,(uint16_t)(y+7u));PE_StoreU16(p+34u,(uint16_t)(y+7u));
    PE_StoreU8(p+12u,(uint8_t)u);PE_StoreU8(p+28u,(uint8_t)u);
    PE_StoreU8(p+20u,(uint8_t)(u+5u));PE_StoreU8(p+36u,(uint8_t)(u+5u));
    PE_StoreU8(p+13u,(uint8_t)v);PE_StoreU8(p+21u,(uint8_t)v);
    PE_StoreU8(p+29u,(uint8_t)(v+7u));PE_StoreU8(p+37u,(uint8_t)(v+7u));
    PE_StoreU16(p+14u,(uint16_t)PE_LoadU32(0x8009D13Cu));PE_StoreU16(p+22u,7u);
    PE_MenuPacketLink(packet);
}

/* func_8005FB74: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005FB74_port.c (src/func_8005FB74.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8006006C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006006C_port.c (src/func_8006006C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80052894: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80052894_port.c (src/func_80052894.c); hand port retired (switch1 lane, audit klass a-replaceable, generator-verified eligible). */

pe_addr_t func_8005DD3C(uint32_t id)
{
    pe_addr_t archive=0x800A8028u+PE_LoadU32(0x800A802Cu);
    pe_addr_t table=archive+PE_LoadU32(archive+16u);
    if (id>=PE_LoadU16(table)) return 0u;
    return table+(uint32_t)(int32_t)(int16_t)PE_LoadU16(table+id*2u+2u);
}

/* func_800534E4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800534E4_port.c (src/func_800534E4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

void func_80053648(pe_addr_t record)
{
    pe_addr_t name;
    if (PE_LoadU8(record+5u)&16u) name=0x800C20A4u+(PE_LoadU8(record+6u)==9u?16u:0u);
    else name=func_8005DC9C(PE_LoadU8(record+4u)-1u);
    func_800534E4(record,name);
}

/* func_8004DF74: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004DF74_port.c (src/func_8004DF74.c); hand port retired (port3 switch-over K). */

static pe_addr_t inventory_description(pe_addr_t record)
{return record?func_8005DCEC(PE_LoadU8(record+4u)-1u):0u;}

static pe_addr_t selected_inventory_help(int32_t index,uint32_t source)
{
    int32_t selected=(int32_t)PE_LoadU32(0x8009CF8Cu);
    if (selected>=0) {
        int32_t kind=func_8005415C(selected);
        pe_addr_t text=func_8005DC4C(kind>=19 && kind<22?14u:15u);
        func_8005E968(0x408040u);return text;
    }
    if (source==52u) return inventory_description(func_80058BBC(index));
    if (source==51u) index=func_80058E08(index);
    return inventory_description(func_8005332C(index));
}

int func_800404A8(void);
int func_8003FFBC(void);
void func_8004C608(pe_addr_t window)
{
    pe_addr_t focused=func_80062CC4(),text=0u;uint32_t id,index;
    func_8005E8C4();func_8005E8A4((int32_t)(PE_LoadU32(window+52u)-39u),1);
    func_8006006C((int32_t)func_80052894(2u),1u);func_8005E8A4(-33,0);func_8005EB64(140u);func_8005E914();
    if (!focused) return;
    id=PE_LoadU32(focused+36u);index=(uint32_t)func_80063428(func_80062A34(2u,id));
    switch (id) {
    case 0: {
        uint32_t enabled=PE_LoadU32(0x8009CEF0u)&(func_8005B89C()?0x1Fu:0x1EFu);
        int32_t remaining=(int32_t)index,option=-1;
        while (remaining>=0) {
            remaining-=(int32_t)(enabled&1u);option++;enabled>>=1u;
            if (option>=9) break;
        }
        text=func_8005DD3C((uint32_t)(option+40));break;
    }
    case 1:case 51:case 52:text=selected_inventory_help((int32_t)index,id);break;
    case 5:
        if ((int32_t)index>=0) text=inventory_description(func_8005332C((int8_t)PE_LoadU8(
            PE_LoadU32(0x8009CF18u)?0x800C0E20u:0x800C0E22u)));
        break;
    case 6:case 11: {
        pe_addr_t record=id==6u?PE_LoadU32(0x8009CF20u):func_8005332C(func_80059F08(1u));
        uint32_t command=PE_LoadU8(record+index+21u)&31u;
        if (command) text=func_8005DD3C(command+(1u-PE_LoadU32(0x8009CF18u))*20u-1u);
        break;
    }
    case 7:
        text=inventory_description(func_8005332C(PE_LoadU32(0x8009CF1Cu)?
            func_80059F08(1u):func_800556E8((int32_t)index)));break;
    case 8:
        if ((int32_t)index<func_80054288()) text=func_8005DCEC((uint32_t)func_800556E8((int32_t)index)+235u);
        break;
    case 13:case 16:
        if (PE_LoadU32(0x8009CEFCu)) text=func_8005DCEC((uint32_t)func_80057ED8((int32_t)index)-1u);
        else text=inventory_description(func_8005332C(func_800556E8((int32_t)index)));
        break;
    case 14:func_80052E30(0u);text=inventory_description(func_8005332C((int32_t)index));break;
    case 17:text=func_8005DC4C(39u);break;
    case 2:case 3:case 4:case 9:case 10:case 15:case 18:case 19:case 20:case 21:case 22:
    case 23:case 24:case 25:case 26:case 30:case 31:case 34:case 40:case 41:case 42:case 43:
    case 44:case 45:case 47:case 53:case 54:case 55:case 57:break;
    case 12:text=func_8005DD3C(index+79u);break;
    case 27:case 28:text=func_8005DD3C(82u);break;
    case 29:text=func_8005DD3C(index+86u-PE_LoadU32(0x8009CF18u)*3u);break;
    case 32:if ((int32_t)index<4) text=func_8005DD3C(index+49u);break;
    case 33:text=func_8005DD3C(index+58u);break;
    case 35:text=func_8005DD3C(index+61u);break;
    case 46:text=func_8005DD3C(index+63u);break;
    case 48:text=func_8005DD3C(index+56u);break;
    case 49:text=func_8005DD3C(index+66u);break;
    case 50:text=func_8005DD3C(PE_LoadU8(0x8009234Cu+
        (PE_LoadU32(0x8009CF0Cu)==1u && index==2u?4u:index)));break;
    case 36:   /* audit item 29: memory-card help (src/func_8004C608.c) */
        if (func_800404A8()) {
            func_8005E8A4(6,5);func_8005F5B8(0x4Cu);func_8005E8A4(0,0x10);func_8005F5B8(0x4Du);break;
        }
        text=func_8005DC4C((func_80042770(0)||func_80042770(1))?0x4Fu:0x4Eu);break;
    case 37:case 38:
        if (PE_LoadU32(0x8009CF50u)) {
            if (PE_LoadU32(0x8009CFF8u)) {
                func_8005EB58(0);func_8005E8A4(6,5);func_8005F5B8(0x58u);func_8005E8A4(0,0x10);
                func_8005F5B8(0x59u);break;
            }
            text=func_8005DC4C(0x5Cu);break;
        }
        text=func_8005DC4C(func_8003FFBC()?0x57u:0x5Bu);break;
    case 39:
        func_8005EB58(0);func_8005E8A4(6,5);func_8005F5B8(0x50u);func_8005E8A4(0,0x10);func_8005F5B8(0x51u);break;
    case 56:func_8005EB58(0u);text=func_8005DD3C(68u);break;
    case 58:text=func_8005DD3C(index+53u);break;
    case 59:text=func_8005DD3C(index+(PE_LoadU32(0x8009CF0Cu)==2u?77u:75u));break;
    case 60:text=func_8005DD3C(index+72u-PE_LoadU32(0x8009CF18u)*3u);break;
    default:
        if (id<61u) {
            fprintf(stderr,"[MENU] Unported help selection %u\n",id);
            Bootstrap_ReturnVoid("func_8004C608","inventory help selection");
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        }
        break;
    }
    if (text) {func_8005E8A4(6,5);func_8005F27C(text);func_8005E968(0x808080u);}
}
