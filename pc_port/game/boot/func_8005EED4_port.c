/* Original menu font/icon packets and saved drawing coordinates.
 * 4F0C4.s / 4F364.s / 4F6D4.s / 4FDB8.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

static pe_addr_t menu_ram(pe_addr_t address)
{ return address<0x200000u?address|0x80000000u:address; }

pe_addr_t PE_MenuPacketAlloc(uint32_t bytes)
{
    pe_addr_t packet=PE_LoadU32(0x8009D100u),next=packet+bytes;
    if (next>=PE_LoadU32(0x8009D104u)+0x4000u) return 0u;
    PE_StoreU32(0x8009D100u,next);return packet;
}

void PE_MenuPacketLink(pe_addr_t packet)
{
    pe_addr_t ot=PE_LoadU32(0x8009D11Cu),p=menu_ram(packet);
    PE_StoreU32(p,(PE_LoadU32(p)&0xFF000000u)|(PE_LoadU32(menu_ram(ot))&0xFFFFFFu));
    PE_StoreU32(menu_ram(ot),(PE_LoadU32(menu_ram(ot))&0xFF000000u)|(packet&0xFFFFFFu));
}

/* func_8005E8A4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005E8A4_port.c (src/func_8005E8A4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_8005E8C4(void)
{
    pe_addr_t stack=PE_LoadU32(0x8009D12Cu);
    if (stack<0x800A22B0u) {
        PE_StoreU32(0x8009D12Cu,stack+8u);
        PE_StoreU32(menu_ram(stack),PE_LoadU32(0x8009D124u));
        PE_StoreU32(menu_ram(stack+4u),PE_LoadU32(0x8009D128u));
    }
}

void func_8005E914(void)
{
    pe_addr_t stack=PE_LoadU32(0x8009D12Cu);
    if (stack>0x800A2270u) {
        uint32_t x=PE_LoadU32(stack-8u),y=PE_LoadU32(stack-4u);
        PE_StoreU32(0x8009D12Cu,stack-8u);PE_StoreU32(0x8009D124u,x);PE_StoreU32(0x8009D128u,y);
    }
}

/* func_8005EB58: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005EB58_port.c (src/func_8005EB58.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8005DC28: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005DC28_port.c (src/func_8005DC28.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void PE_MenuPacketColor(pe_addr_t packet,uint8_t words,uint8_t code)
{
    if (packet) {
        PE_StoreU32(menu_ram(packet+4u),PE_LoadU32(PE_LoadU32(0x8009D10Cu)?0x8009D114u:0x8009D110u));
        PE_StoreU8(menu_ram(packet+3u),words);PE_StoreU8(menu_ram(packet+7u),code);
    }
}

void func_8005EB64(uint32_t icon)
{
    pe_addr_t spec=func_8005DADC(icon),packet=PE_MenuPacketAlloc(20u),p=menu_ram(packet);
    uint32_t page;
    PE_MenuPacketColor(packet,4u,0x64u);
    PE_StoreU16(p+8u,(uint16_t)PE_LoadU32(0x8009D124u));PE_StoreU16(p+10u,(uint16_t)PE_LoadU32(0x8009D128u));
    PE_StoreU8(p+12u,PE_LoadU8(spec));PE_StoreU8(p+13u,PE_LoadU8(spec+1u));
    PE_StoreU16(p+14u,PE_LoadU16(spec+2u));PE_StoreU16(p+16u,PE_LoadU8(spec+4u));
    PE_StoreU16(p+18u,PE_LoadU8(spec+5u));PE_MenuPacketLink(packet);
    page=PE_LoadU8(spec+6u);packet=PE_MenuPacketAlloc(8u);
    if (packet) (void)func_80077C84(packet,0u,0u,((page&3u)<<7u)|7u);
    PE_MenuPacketLink(packet);
}

static int32_t menu_glyph_code(uint32_t character)
{
    uint32_t code=character&255u,prefix=PE_LoadU32(0x8009D0D8u);
    if (prefix) {code+=prefix<<8u;PE_StoreU32(0x8009D0D8u,0u);}
    if ((character&255u)>=250u) {PE_StoreU32(0x8009D0D8u,(character&255u)-250u);return -1;}
    return (int32_t)code;
}

void func_8005EED4(uint32_t character)
{
    int32_t code=menu_glyph_code(character);
    uint32_t row,column,metrics,width,spacing,x,y,u,v;
    pe_addr_t packet,p;
    if (code==256) {func_8005EB64(119u);func_8005E8A4(12,0);return;}
    if (code<0) return;
    if (code>256) code-=19;
    code%=441;row=(uint32_t)code/21u;column=(uint32_t)code%21u;
    packet=PE_MenuPacketAlloc(40u);p=menu_ram(packet);PE_MenuPacketColor(packet,9u,0x2Cu);
    metrics=func_8005DC28((uint32_t)code);width=(metrics>>4u)&15u;
    spacing=1u+(code<10 || code==15);PE_StoreU32(0x8009CDB0u,spacing);
    u=column*12u+(metrics&15u);v=row*12u;
    x=PE_LoadU32(0x8009D124u)+(spacing>>1u);y=PE_LoadU32(0x8009D128u)+1u;
    PE_StoreU8(p+12u,(uint8_t)u);PE_StoreU8(p+28u,(uint8_t)u);PE_StoreU16(p+14u,0x89Cu);
    PE_StoreU8(p+13u,(uint8_t)v);PE_StoreU8(p+21u,(uint8_t)v);
    PE_StoreU8(p+20u,(uint8_t)(u+width));PE_StoreU8(p+36u,(uint8_t)(u+width));
    PE_StoreU8(p+29u,(uint8_t)(v+12u));PE_StoreU8(p+37u,(uint8_t)(v+12u));
    PE_StoreU16(p+8u,(uint16_t)x);PE_StoreU16(p+24u,(uint16_t)x);
    PE_StoreU16(p+16u,(uint16_t)(x+width));PE_StoreU16(p+32u,(uint16_t)(x+width));
    PE_StoreU16(p+10u,(uint16_t)y);PE_StoreU16(p+18u,(uint16_t)y);
    PE_StoreU16(p+26u,(uint16_t)(y+12u));PE_StoreU16(p+34u,(uint16_t)(y+12u));
    PE_StoreU16(p+22u,(uint16_t)func_80077A64(0u,0u,320u,0u));
    PE_MenuPacketLink(packet);func_8005E8A4((int32_t)(width+spacing),0);
}

/* func_8005F1A0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005F1A0_port.c (src/func_8005F1A0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_8005F27C(pe_addr_t text)
{
    if (!text) return;
    func_8005E8C4();
    while (PE_LoadU8(text)!=255u) func_8005EED4(PE_LoadU8(text++));
    func_8005E914();
}

void func_8005F5B8(uint32_t id) { func_8005F27C(func_8005DC4C(id)); }

/* func_80050F10: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050F10_port.c (src/func_80050F10.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static void name_control_icon(uint32_t index,uint32_t first)
{
    if (index!=(uint32_t)func_80063428(PE_LoadU32(0x8009CEF4u))) func_8005EB58(1u);
    func_8005E8A4(-2,-2);func_8005EB64(index+first);
}
/* func_80050F64: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050F64_port.c (src/func_80050F64.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
/* func_80050FB8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050FB8_port.c (src/func_80050FB8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
/* func_8005100C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005100C_port.c (src/func_8005100C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

void func_8005E6F0(void)
{
    uint32_t standalone=PE_LoadU32(0x8009D120u);
    uint32_t bank=standalone?!PE_LoadU32(0x8009D108u):PE_LoadU32(0x8009CDDCu);
    pe_addr_t descriptor=0x800A2180u+bank*120u;
    pe_addr_t packets=PE_LoadU32(descriptor+116u),ot=PE_LoadU32(descriptor+112u);
    PE_StoreU32(0x8009D108u,bank);PE_StoreU32(0x8009D0FCu,descriptor);
    PE_StoreU32(0x8009D104u,packets);PE_StoreU32(0x8009D100u,packets);
    PE_StoreU32(0x8009D118u,ot);PE_StoreU32(0x8009D11Cu,ot+4u);
    if (standalone) func_800752AC(ot,4096);
}
