/* Original menu button mapping and edge/repeat event producer (4E92C.s).
 * Events retain their guest free-list and FIFO ownership. */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8005267C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005267C_port.c (src/func_8005267C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80052634: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80052634_port.c (src/func_80052634.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_800525EC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800525EC_port.c (src/func_800525EC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8005E038: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005E038_port.c (src/func_8005E038.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8005E114: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005E114_port.c (src/func_8005E114.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static void menu_input_event(uint32_t type,uint32_t buttons)
{
    pe_addr_t node=PE_LoadU32(0x8009D0DCu),tail;
    if (!node) return;
    tail=PE_LoadU32(0x8009D0E4u);PE_StoreU32(0x8009D0DCu,PE_LoadU32(node));PE_StoreU32(node,0u);
    if (tail) PE_StoreU32(tail,node);
    else PE_StoreU32(0x8009D0E0u,node); /* 527C0's inconsistent-queue diagnostic is an empty leaf. */
    PE_StoreU32(0x8009D0E4u,node);PE_StoreU32(node+4u,type);PE_StoreU32(node+8u,buttons);
}

void func_8005E12C(int32_t fast_repeat)
{
    uint32_t buttons=func_8005E038(),released,pressed,repeating=0u;
    int32_t timer;
    if (!PE_LoadU32(0x8009D0E8u)) {
        if (!buttons) PE_StoreU32(0x8009D0E8u,1u);
        return;
    }
    released=PE_LoadU32(0x8009D0F0u)&~buttons;
    if (released) menu_input_event(4u,released);
    if (PE_LoadU32(0x8009D0F0u)!=buttons) PE_StoreU32(0x8009D0F8u,16u);
    timer=(int32_t)(PE_LoadU32(0x8009D0F8u)-2u);PE_StoreU32(0x8009D0F8u,(uint32_t)timer);
    if (timer<0 && ((fast_repeat && timer<-90) || !((uint32_t)timer&3u))) {
        PE_StoreU32(0x8009D0F0u,0u);repeating=1u;
    }
    PE_StoreU32(0x8009D0F4u,timer<-299?8u:1u);
    pressed=buttons&~PE_LoadU32(0x8009D0F0u);
    if (pressed && !(repeating && (pressed&64u))) menu_input_event(repeating?2u:1u,pressed);
    PE_StoreU32(0x8009D0F0u,buttons);
}

static void menu_sound(uint32_t id)
{
    pe_addr_t package=PE_LoadU32(0x800B0E08u);
    if (package) (void)func_8006DF50(package,id,0x100u,128u,127u);
}

/* func_800526C4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800526C4_port.c (src/func_800526C4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
