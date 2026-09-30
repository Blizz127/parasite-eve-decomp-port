/* Original name-entry construction and encoded eight-character editing.
 * 3E4A4.s / 4C4BC.s / 4C708.s; the renderer and input callbacks are separate. */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8005BCB0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005BCB0_port.c (src/func_8005BCB0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8005BEDC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005BEDC_port.c (src/func_8005BEDC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8005BEE8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005BEE8_port.c (src/func_8005BEE8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
pe_addr_t func_80062A20(pe_addr_t node,uint32_t index) { return PE_LoadU32(node+8u+index*4u); }

/* func_80063428: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80063428_port.c (src/func_80063428.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8005BD10: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005BD10_port.c (src/func_8005BD10.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_8005BD94(void)
{
    int32_t count=0,limit=(int32_t)PE_LoadU32(0x8009D0C4u);
    pe_addr_t p=func_8005BEDC();
    if (limit>0) while (PE_LoadU8(p)!=255u) {
        count+=PE_LoadU8(p)<250u;p++;
        if (count>=limit) break;
    }
    if (count<=0) return 0;
    p--;PE_StoreU8(p,255u);p--;
    if (p>=func_8005BEDC() && PE_LoadU8(p)>=250u) PE_StoreU8(p,255u);
    return 1;
}

void func_8005BE1C(void)
{
    pe_addr_t p=func_8005BEDC(),record,source;
    uint8_t ch;
    while (p<func_8005BEDC()+PE_LoadU32(0x8009D0C4u)) PE_StoreU8(p++,255u);
    record=PE_LoadU32(0x8009D0C8u);p=func_8005BEDC();
    source=record?func_8005DC9C(PE_LoadU8(record+4u)-1u):func_8005DC4C(30u);
    do {ch=PE_LoadU8(source++);PE_StoreU8(p++,ch);} while (ch!=255u);
}

/* func_8005BF08: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005BF08_port.c (src/func_8005BF08.c); hand port retired (port3 switch-over E). */

/* func_80052FCC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80052FCC_port.c (src/func_80052FCC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_8004DD64(int32_t id)
{
    pe_addr_t window=func_80062D2C(23u,0,0,0),node,name,record;
    int alternate;
    PE_StoreU32(window+44u,0x8004E2E4u);
    PE_StoreU32(window+76u,func_8005BCB0()?0x80092380u:0x80092354u);
    alternate=func_8005BCB0();
    if (!alternate) {
        node=func_8006322C(23u,window,window);PE_StoreU32(node+48u,0x8005010Cu);
    }
    node=func_8006322C(24u,window,window);PE_StoreU32(node+48u,0x80050178u);
    if (alternate) PE_StoreU32(node+28u,PE_LoadU32(node+28u)-52u);
    node=func_8006322C(25u,window,window);PE_StoreU32(node+48u,0x800501C8u);
    if (alternate) PE_StoreU32(node+28u,PE_LoadU32(node+28u)-52u);
    PE_StoreU32(node+68u,UINT32_MAX);
    window=func_80062D2C(17u,func_80062A34(2u,23u),0,0);
    node=func_8006322C(17u,window,window);
    PE_StoreU32(window+44u,0x8004E074u);PE_StoreU32(node+48u,0x800500A8u);
    PE_StoreU32(node+68u,0u);PE_StoreU32(node+72u,0u);func_80062CB8(node);
    name=func_80062D2C(26u,0,0,0);PE_StoreU32(name+48u,0x8004DF74u);
    record=func_8005332C(id);PE_StoreU32(0x8009D004u,record);
    if (!record) PE_StoreU32(name+76u,0x800923A0u);
    func_8005BCBC(record);func_8005BE1C();func_80052FCC(record);
    PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)|0x8000u);
}

static void name_confirm(void)
{
    pe_addr_t p=func_8005BEDC(),start=p;
    int all_spaces=1;
    while (PE_LoadU8(p)!=255u) if (PE_LoadU8(p++)!=15u) all_spaces=0;
    if (p==start || all_spaces) {func_800526C4();return;}
    func_800512AC(9,0);
    if (!PE_LoadU32(0x8009D004u)) (void)func_80052594(func_8005BEE8());
    PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)&~0x8000u);
    func_800525EC();
}

static void name_node_store(pe_addr_t node,uint32_t offset,uint32_t value)
{
    /* The alternate grid can select the absent group 23 from a negative
     * cursor row. Retail writes physical RAM 0x44/0x48, then clears focus. */
    pe_addr_t address=node+offset;
    PE_StoreU32(address<0x200000u?address|0x80000000u:address,value);
}

int func_8004E074(pe_addr_t window,uint32_t event)
{
    pe_addr_t node=func_80062A20(window,0u);
    if (event&0x8000u) {
        int32_t row=(int32_t)(PE_LoadU32(node+72u)+(uint32_t)func_8005BCB0()*3u);
        PE_StoreU32(node+68u,UINT32_MAX);
        if (row>=0 && row<3) {node=func_80062A34(2u,23u);name_node_store(node,72u,(uint32_t)row);}
        else if (row>=0 && row<5) {node=func_80062A34(2u,24u);name_node_store(node,72u,(uint32_t)row-3u);}
        else {node=func_80062A34(2u,25u);name_node_store(node,72u,0u);}
        name_node_store(node,68u,0u);func_80062CB8(node);func_8005267C();
    } else if (event&0x10000u) {
        pe_addr_t group=func_80062A34(2u,23u);
        pe_addr_t text=func_8005DC4C(group?PE_LoadU32(group+72u)+0x73u:0x75u);
        PE_StoreU32(0x8009CF54u,text);
        func_8005BD10(PE_LoadU8(text+(uint32_t)func_80063428(node)));func_800525EC();
    } else if (event&0x40u) {
        if (func_8005BD94()) func_80052634();else func_800526C4();
    } else if (event&0x800u) {
        if (PE_LoadU32(node+36u)!=25u) {
            PE_StoreU32(node+68u,UINT32_MAX);node=func_80062A34(2u,25u);
            PE_StoreU32(node+68u,0u);func_80062CB8(node);
        } else name_confirm();
    }
    return 1;
}

int func_8004E2E4(pe_addr_t window,uint32_t event)
{
    pe_addr_t node=func_80062CC4();uint32_t id;int32_t alternate,row;
    (void)window;
    id=PE_LoadU32(node+36u);
    if (event&0x10000u) {
        if (id==23u) event|=0x2000u;
        else if (id==24u) {
            if (PE_LoadU32(node+72u)) {func_8005BE1C();func_800525EC();}
            else if (func_8005BD94()) func_80052634();else func_800526C4();
        } else if (id==25u) name_confirm();
    }
    if (event&0x2000u) {
        alternate=func_8005BCB0()*3;
        row=(int32_t)(PE_LoadU32(node+72u)-(uint32_t)alternate);
        if (id==24u) row=(int32_t)((uint32_t)row+3u);
        else if (id!=23u) row=(int32_t)((uint32_t)row+5u);
        if (id!=23u) PE_StoreU32(node+68u,UINT32_MAX);
        node=func_80062A34(2u,17u);PE_StoreU32(node+68u,0u);
        if (row<5-alternate || (int32_t)PE_LoadU32(node+72u)<6-alternate)
            PE_StoreU32(node+72u,(uint32_t)row);
        func_80062CB8(node);func_8005267C();
    } else if (event&0x1000u) {
        if (id!=23u) PE_StoreU32(node+68u,UINT32_MAX);
        alternate=func_8005BCB0();
        node=func_80062A34(2u,(int32_t)id>alternate+23?id-1u:25u);
        PE_StoreU32(node+68u,0u);PE_StoreU32(node+72u,PE_LoadU32(node+88u)-1u);
        func_80062CB8(node);func_8005267C();
    } else if (event&0x4000u) {
        if (id!=23u) PE_StoreU32(node+68u,UINT32_MAX);
        node=func_80062A34(2u,(int32_t)id<25?id+1u:(uint32_t)func_8005BCB0()+23u);
        PE_StoreU32(node+68u,0u);PE_StoreU32(node+72u,0u);
        func_80062CB8(node);func_8005267C();
    } else if (event&0x800u) {
        if (id!=25u) {
            PE_StoreU32(node+68u,UINT32_MAX);node=func_80062A34(2u,25u);
            PE_StoreU32(node+68u,0u);func_80062CB8(node);
        } else name_confirm();
    }
    return 1;
}
