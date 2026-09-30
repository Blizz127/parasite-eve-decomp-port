/* Original field/name menu frame and palette transition.
 * 4C998.s / 7640.s / 3E4A4.s / 334C4.s. Inventory/card callbacks
 * remain explicit native boundaries until their implementations land. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"
#include "pe_guestcode.h"
#include <stdio.h>

static void menu_boundary(const char *name)
{
    Bootstrap_ReturnVoid(name,"func_8005C498");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

void func_8004DCA4(uint32_t mode)
{
    pe_addr_t help;
    func_8005DE88();
    if (func_80062A34(1u,13u) || func_80062A34(1u,23u)) return;
    if (mode) {menu_boundary("func_8004DCA4_inventory_rename");return;}
    func_8004DD64(-1);
    if (!func_80062A34(1u,19u)) {
        help=func_80062D2C(19u,0u,0u,0u);PE_StoreU32(help+48u,0x8004C608u);
    }
}

int func_80016F10(pe_addr_t args)
{
    pe_addr_t task;
    if (PE_LoadU32(0x800B0CD8u)&0x1000u) return 0;
    task=PE_LoadU32(0x8009D300u);
    if (PE_LoadU16(task+8u)&32u) {
        func_80067CBC();func_8004DCA4(PE_LoadU32(PE_LoadU32(args)));
        if (PE_Port_ShouldStop()) return 0;
        PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)|0x9000u);
        task=PE_LoadU32(0x8009D300u);
        D_8009D1A0|=4u;PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU16(task+8u,PE_LoadU16(task+8u)&0xFFDFu);
    } else {
        PE_StoreU16(task+8u,PE_LoadU16(task+8u)|32u);
        PE_StoreU32(0x8009CE00u,PE_LoadU32(0x8009CE00u)-12u);
    }
    PE_StoreU32(PE_LoadU32(0x8009D300u)+16u,1u);return 0;
}

void func_80046334(void)
{
    if (PE_LoadU32(0x8009CFB0u) && !(PE_LoadU8(0x800B0CE6u)&3u)) {
        func_8005E114(0);PE_StoreU32(0x8009CFB0u,0u);
    }
}

/* func_8004F464: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004F464_port.c (src/func_8004F464.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8005C488: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005C488_port.c (src/func_8005C488.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80042B6C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042B6C_port.c (src/func_80042B6C.c); hand port retired (port3 switch-over D). */

void func_800339A0(uint32_t style)
{
    pe_addr_t row=0x80010E38u+(style&255u)*4u;
    if ((style&255u)>=4u) {menu_boundary("func_800339A0_invalid_style");return;}
    PE_StoreU16(0x8009CE84u,PE_LoadU16(row));PE_StoreU8(0x8009CE80u,(uint8_t)style);
    PE_StoreU16(0x8009CE86u,PE_LoadU16(row+2u));
}

void func_80042D40(void)
{
    uint32_t value=PE_LoadU32(0x8009CEE8u)+PE_LoadU32(0x8009CEE4u);
    uint32_t light,curve,bias,scale,i;
    pe_addr_t source,destination;
    PE_StoreU32(0x8009CEE8u,value);
    if ((int32_t)value<0) {
        PE_StoreU32(0x8009CEE8u,0u);PE_StoreU32(0x8009CEE4u,0u);PE_StoreU32(0x8009CED8u,0u);
    } else if ((int32_t)value>=(int32_t)PE_LoadU32(0x8009CEE0u)) {
        PE_StoreU32(0x8009CEE8u,PE_LoadU32(0x8009CEE0u)-1u);
        PE_StoreU32(0x8009CEE4u,0u);PE_StoreU32(0x8009CED8u,7u);
    }
    curve=PE_LoadU8(0x800A1878u+PE_LoadU32(0x8009CEE8u));light=PE_LoadU32(0x8009CEECu);
    bias=(curve*light)<<5u;scale=curve*(light+256u);
    source=D_800B0E50;destination=D_800B0E54;
    for (i=0;(int32_t)i<(int32_t)(PE_LoadU32(0x8009CEDCu)<<8u);i++) {
        uint32_t pixel=PE_LoadU16(source),result=0u;
        source+=2u;
        if (pixel) {
            uint32_t r=pixel&31u,g=(pixel>>5u)&31u,b=(pixel>>10u)&31u;
            r=(uint32_t)((int32_t)((r<<16u)+bias-scale*r)>>16);
            g=(uint32_t)((int32_t)((g<<16u)+bias-scale*g)>>16);
            b=(uint32_t)((int32_t)((b<<16u)+bias-scale*b)>>16);
            result=(pixel&0x8000u)|r|(g<<5u)|(b<<10u);
        }
        PE_StoreU16(destination,(uint16_t)result);destination+=2u;
    }
    value=bias>>13u;
    PE_StoreU8(0x800BCE3Fu,(uint8_t)value);PE_StoreU8(0x800BCE3Eu,(uint8_t)value);PE_StoreU8(0x800BCE3Du,(uint8_t)value);
    PE_StoreU8(0x800BCDE3u,(uint8_t)value);PE_StoreU8(0x800BCDE2u,(uint8_t)value);PE_StoreU8(0x800BCDE1u,(uint8_t)value);
}

/* func_80042F44: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042F44_port.c (src/func_80042F44.c); hand port retired (port3 switch-over N). */

void func_8005E788(int32_t wait)
{
    if (PE_LoadU32(0x8009D120u)) {
        pe_addr_t descriptor;RECT rect;
        (void)func_80073A44(1);(void)func_80074DC0(0);(void)func_80073A44(wait==1?0:wait);
        (void)func_80074A44(1);descriptor=PE_LoadU32(0x8009D0FCu);
        (void)func_80075424(descriptor);func_800755F0(PE_LoadU32(0x8009D0FCu)+92u);
        if (PE_LoadU32(0x8009D134u)) {
            rect.x=0;rect.y=PE_LoadU32(0x8009D108u)?235:11;rect.w=320;rect.h=204;
            (void)func_8007506C(&rect,PE_LoadU32(0x8009D134u));
        }
        (void)func_800753B4(PE_LoadU32(0x8009D118u)+0x3FFCu);
    }
}

/* func_8005C498: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005C498_port.c (src/func_8005C498.c); hand port retired (port3 switch-over E). */

void PE_FieldMenuFrame(void)
{
    pe_addr_t aya=PE_LoadU32(0x8009D254u);
    int result;
    if (aya && PE_LoadU32(aya) && (PE_LoadU32(0x8009D1F4u)&128u) &&
        !(D_8009D1A0&0x2000u) && !(PE_LoadU32(0x800B0CD8u)&0x3400u)) {
        func_8005C174(0);func_80067CBC();D_8009D1A0|=4u;
        PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)|0x9000u);
    }
    result=func_8005C498(0x800A76D8u);PE_StoreU16(0x8009D2A4u,(uint16_t)result);
    if ((uint16_t)result) {
        func_80067CBC();D_8009D1A0&=~4u;PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)&~0x9000u);
    }
}
