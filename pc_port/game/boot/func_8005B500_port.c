/* Original inventory category/stat sorting, 486D8.s. The retail quicksort
 * (621E4.s, 723A4/724F4) preserves its pivot and equal-key swap order. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_guestcode.h"

extern unsigned int func_80052F70(void);
static pe_addr_t sort_ram(pe_addr_t p) {return p<0x200000u?p|0x80000000u:p;}
static int32_t sort_short(pe_addr_t p) {return (int16_t)PE_LoadU16(sort_ram(p));}
static pe_addr_t sort_item(int32_t id)
{
    uint32_t fn=PE_LoadU32(0x8009D0B4u);
    if (fn==0x800532B4u) return func_800532B4((uint32_t)id);
    if (fn==0x8005332Cu) return func_8005332C(id);
    if (PE_GuestCode_Resolve(fn))   /* retail jalr through the registry */
        return (pe_addr_t)(uint32_t)PE_GuestCall("@sort_item",fn,1u,(uint32_t)id,0u,0u,0u);
    Bootstrap_ReturnVoid("PE_InventorySortItem","inventory sorting callback");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0u;
}

int32_t func_8005AFFC(pe_addr_t first,pe_addr_t second)
{
    int32_t a=sort_short(first),b=sort_short(second);
    pe_addr_t x,y,category,subtype;uint32_t xc,yc,xs,ys,xi,yi;
    if (PE_LoadU32(0x8009D0B4u)!=0x8005332Cu) {if (!a) return b!=0;if (!b) return -1;}
    x=sort_item(a);if (PE_Port_ShouldStop()) return 0;
    y=sort_item(b);if (PE_Port_ShouldStop()) return 0;
    category=PE_LoadU32(0x8009D0B8u);subtype=PE_LoadU32(0x8009D0BCu);
    /* Retail reads these bytes before testing the returned pointers. Low
     * addresses here refer to guest RAM, including a null provider result. */
    xc=PE_LoadU8(sort_ram(category+PE_LoadU8(sort_ram(x+6u))));
    yc=PE_LoadU8(sort_ram(category+PE_LoadU8(sort_ram(y+6u))));
    xs=PE_LoadU8(sort_ram(subtype+(PE_LoadU8(sort_ram(x+14u))&15u)));
    ys=PE_LoadU8(sort_ram(subtype+(PE_LoadU8(sort_ram(y+14u))&15u)));
    if (x && !y) return -1;if (!x && y) return 1;
    if (xc!=yc) return xc>yc?1:-1;if (xs!=ys) return xs>ys?1:-1;
    xi=PE_LoadU8(sort_ram(x+4u));yi=PE_LoadU8(sort_ram(y+4u));return xi>yi?1:-(int32_t)(xi<yi);
}

int32_t func_8005B124(pe_addr_t first,pe_addr_t second)
{
    pe_addr_t x,y;
    uint32_t mode=PE_LoadU32(0x8009D0A0u),xi,yi;int32_t a=0,b=0;
    x=sort_item(sort_short(first));if (PE_Port_ShouldStop()) return 0;
    y=sort_item(sort_short(second));if (PE_Port_ShouldStop()) return 0;
    if (mode<3u) {
        a=(int32_t)PE_LoadU8(sort_ram(x+7u+mode))+sort_short(x+14u+mode*2u);
        b=(int32_t)PE_LoadU8(sort_ram(y+7u+mode))+sort_short(y+14u+mode*2u);
    }
    if (a!=b) return a>b?-1:1;
    xi=PE_LoadU8(sort_ram(x+4u));yi=PE_LoadU8(sort_ram(y+4u));return xi>yi?1:-(int32_t)(xi<yi);
}

/* sort_compare (the old qsort port's comparator dispatch) removed with it: the
 * clean-room qsort calls comparators through PE_GuestHook_Call (registry). */

/* func_800723A4: replaced by pe_plat_qsort (pe_plat_qsort.c, clean-room) */

static uint32_t sort_end(void)
{return PE_LoadU32(0x8009D0ACu)+PE_LoadU32(0x8009D0B0u)*2u;}
static int sort_category(pe_addr_t p,uint32_t mask)
{
    pe_addr_t item=sort_item(sort_short(p));
    if (PE_Port_ShouldStop()) return 0;
    return (mask>>(PE_LoadU8(sort_ram(item+6u))&31u))&1u;
}
static void sort_stat_group(uint32_t mask,pe_addr_t mode)
{
    pe_addr_t p=PE_LoadU32(0x8009D0ACu),first;
    if (PE_Port_ShouldStop()) return;
    PE_StoreU32(0x8009D0A0u,PE_LoadU32(mode));
    while (p<sort_end()) {if (!sort_short(p) || sort_category(p,mask) || PE_Port_ShouldStop()) break;p+=2u;}
    if (PE_Port_ShouldStop()) return;
    if (p>=sort_end() || !sort_short(p)) return;
    first=p;
    do {if (!sort_category(p,mask) || PE_Port_ShouldStop()) break;p+=2u;} while (p<sort_end() && sort_short(p));
    if (PE_Port_ShouldStop()) return;
    func_800723A4(first,(uint32_t)((int32_t)(p-first)>>1),2u,0x8005B124u);
}
void func_8005B248(void) {sort_stat_group(0x1FEu,0x8009D0A4u);}
void func_8005B3A4(void) {sort_stat_group(0x200u,0x8009D0A8u);}

void func_8005B500(uint32_t kind,uint32_t selection)
{
    int32_t weapon,armor,i;
    if (PE_Port_ShouldStop()) return;
    if (!kind) {PE_StoreU32(0x8009D0B8u,0x80092428u);PE_StoreU32(0x8009D0A8u,selection);}
    else if (kind==1u) {PE_StoreU32(0x8009D0B8u,0x800923F8u);PE_StoreU32(0x8009D0A4u,selection);}
    else if (kind==2u) {
        PE_StoreU32(0x8009D0B8u,0x80092440u);PE_StoreU32(0x8009D0BCu,selection?0x80092458u:0x80092468u);
    }
    D_8009D048=0x800C0E48u;PE_StoreU32(0x8009D048u,D_8009D048);
    D_8009D050=func_80052F70();PE_StoreU32(0x8009D050u,D_8009D050);
    D_8009D058=0x8009D05Cu;PE_StoreU32(0x8009D058u,D_8009D058);
    D_8009D064=2u;PE_StoreU32(0x8009D064u,D_8009D064);
    weapon=sort_short(D_8009D048+(uint32_t)(int32_t)(int8_t)PE_LoadU8(0x800C0E20u)*2u);
    i=(int8_t)PE_LoadU8(0x800C0E22u);armor=i<0?-1:sort_short(D_8009D048+(uint32_t)i*2u);
    PE_StoreU32(0x8009D0B4u,0x800532B4u);PE_StoreU32(0x8009D0ACu,D_8009D048);PE_StoreU32(0x8009D0B0u,D_8009D050);
    func_800723A4(D_8009D048,D_8009D050,2u,0x8005AFFCu);if (PE_Port_ShouldStop()) return;
    func_8005B248();if (PE_Port_ShouldStop()) return;
    func_8005B3A4();if (PE_Port_ShouldStop()) return;
    for (i=0;i<(int32_t)PE_LoadU32(0x8009D050u);i++)
        if (sort_short(PE_LoadU32(0x8009D048u)+(uint32_t)i*2u)==weapon) {PE_StoreU8(0x800C0E20u,(uint8_t)i);break;}
    if (armor>=0) for (i=0;i<(int32_t)PE_LoadU32(0x8009D050u);i++)
        if (sort_short(PE_LoadU32(0x8009D048u)+(uint32_t)i*2u)==armor) {PE_StoreU8(0x800C0E22u,(uint8_t)i);break;}
    func_80055760();
}

void func_8005B71C(uint32_t selection)
{
    if (PE_Port_ShouldStop()) return;
    PE_StoreU32(0x8009D0B8u,selection?0x800923F8u:0x80092410u);
    PE_StoreU32(0x8009D0BCu,selection?0x80092458u:0x80092468u);
    PE_StoreU32(0x8009D0B0u,100u);PE_StoreU32(0x8009D0ACu,0x800C1EB8u);PE_StoreU32(0x8009D0B4u,0x800532B4u);
    func_800723A4(0x800C1EB8u,100u,2u,0x8005AFFCu);if (PE_Port_ShouldStop()) return;
    func_8005B248();if (PE_Port_ShouldStop()) return;
    func_8005B3A4();if (PE_Port_ShouldStop()) return;
    func_8005B500(2u,selection);if (PE_Port_ShouldStop()) return;
    (void)func_80058C4C(0xF400u);
}

void func_8005B7D0(uint32_t kind,uint32_t selection)
{
    if (PE_Port_ShouldStop()) return;
    if (kind) {PE_StoreU32(0x8009D0A4u,selection);PE_StoreU32(0x8009D0B8u,0x800923F8u);}
    else {PE_StoreU32(0x8009D0A8u,selection);PE_StoreU32(0x8009D0B8u,0x80092410u);}
    PE_StoreU32(0x8009D0BCu,0x80092458u);PE_StoreU32(0x8009D0B0u,82u);
    PE_StoreU32(0x8009D0ACu,0x800C1F80u);PE_StoreU32(0x8009D0B4u,0x800532B4u);
    func_800723A4(0x800C1F80u,82u,2u,0x8005AFFCu);if (PE_Port_ShouldStop()) return;
    func_8005B248();if (PE_Port_ShouldStop()) return;
    func_8005B3A4();if (PE_Port_ShouldStop()) return;
    func_8005B500(kind,selection);if (PE_Port_ShouldStop()) return;
    (void)func_80058C4C(0x3803FEu);
}
