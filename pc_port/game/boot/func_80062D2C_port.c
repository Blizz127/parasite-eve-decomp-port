/* Original menu node allocation, list lookup and saved cursor restoration.
 * 534E4.s / 539DC.s / 55454.s. Retail 527C0 is an empty return leaf. */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8005DA8C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005DA8C_port.c (src/func_8005DA8C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8005DAB4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005DAB4_port.c (src/func_8005DAB4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80062F9C(void)
{
    unsigned i;
    for (i=0;i<24;i++) PE_StoreU32(0x800A22E0u+i*144u,0x800A22E0u+(i+1u)*144u);
    PE_StoreU32(0x800A2FD0u,0u);PE_StoreU32(0x8009D158u,0x800A22E0u);
    PE_StoreU32(0x8009D15Cu,0u);PE_StoreU32(0x8009D154u,0u);
}

/* func_80067CBC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80067CBC_port.c (src/func_80067CBC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

pe_addr_t func_80062A34(uint32_t kind,uint32_t id)
{
    pe_addr_t node=PE_LoadU32(0x8009D154u);
    while (node) {
        if (PE_LoadU32(node+32u)==kind && PE_LoadU32(node+36u)==id) break;
        node=PE_LoadU32(node);
    }
    return node;
}

pe_addr_t func_800631DC(void)
{
    pe_addr_t node=PE_LoadU32(0x8009D154u),result=0u;
    while (node) {
        if (PE_LoadU32(node+32u)==1u && PE_LoadU32(node+68u)) result=node;
        node=PE_LoadU32(node);
    }
    return result;
}

static pe_addr_t menu_allocate(pe_addr_t owner,pe_addr_t parent)
{
    pe_addr_t node=PE_LoadU32(0x8009D158u),next=PE_LoadU32(node),head=PE_LoadU32(0x8009D154u);
    unsigned i;
    PE_StoreU32(0x8009D154u,node);PE_StoreU32(node+4u,owner);
    PE_StoreU32(node+44u,0u);PE_StoreU32(node+48u,0u);
    PE_StoreU32(0x8009D158u,next);PE_StoreU32(node,head);
    for (i=0;i<4;i++) PE_StoreU32(node+20u-i*4u,0u);
    PE_StoreU32(node+28u,0u);PE_StoreU32(node+24u,0u);
    PE_StoreU32(node+36u,0u);PE_StoreU32(node+32u,0u);PE_StoreU32(node+40u,0u);
    if (parent) for (i=0;i<4;i++) if (!PE_LoadU32(parent+8u+i*4u)) {
        PE_StoreU32(parent+8u+i*4u,node);break;
    }
    return node;
}

pe_addr_t func_80062D2C(uint32_t id,pe_addr_t owner,pe_addr_t parent,uint32_t modal)
{
    pe_addr_t spec=func_8005DA8C(id),node=menu_allocate(owner,parent);
    if (!modal) {
        pe_addr_t last=func_800631DC();
        if (last) {
            pe_addr_t head=PE_LoadU32(0x8009D154u),next=PE_LoadU32(head),after=PE_LoadU32(last);
            PE_StoreU32(head,after);PE_StoreU32(last,head);PE_StoreU32(0x8009D154u,next);
        }
    }
    PE_StoreU32(node+32u,1u);PE_StoreU32(node+36u,id);
    if (PE_LoadU32(spec)) {
        PE_StoreU32(node+24u,PE_LoadU32(spec));PE_StoreU32(node+28u,PE_LoadU32(spec+4u));
    } else {
        PE_StoreU32(node+24u,160u-(uint32_t)((int32_t)PE_LoadU32(spec+8u)>>1));
        PE_StoreU32(node+28u,(PE_LoadU32(spec+4u)?80u:120u)-(uint32_t)((int32_t)PE_LoadU32(spec+12u)>>1));
    }
    PE_StoreU32(node+52u,PE_LoadU32(spec+8u));PE_StoreU32(node+56u,PE_LoadU32(spec+12u));
    PE_StoreU32(node+60u,0u);PE_StoreU32(node+64u,0u);PE_StoreU32(node+68u,modal);
    PE_StoreU32(node+72u,0u);PE_StoreU32(node+76u,0u);
    return node;
}

void func_80064AC0(pe_addr_t node)
{
    int32_t id;pe_addr_t saved;
    if (!node || (id=(int32_t)PE_LoadU32(node+112u))<0) return;
    saved=0x800A3060u+(uint32_t)id*4u;
    PE_StoreU32(node+68u,(uint32_t)(int32_t)(int8_t)PE_LoadU8(saved));
    if (!PE_LoadU32(0x8009D16Cu) && !(PE_LoadU32(node+100u)&32u)) return;
    PE_StoreU32(node+72u,(uint32_t)(int32_t)(int8_t)PE_LoadU8(saved+1u));
    if ((int32_t)PE_LoadU32(node+72u)>=(int32_t)PE_LoadU32(node+88u))
        PE_StoreU32(node+72u,PE_LoadU32(node+88u)-1u);
    if (PE_LoadU32(node+104u) && PE_LoadU32(node+68u)==1u &&
        PE_LoadU32(node+72u)==PE_LoadU32(node+88u)-1u) PE_StoreU32(node+68u,0u);
    PE_StoreU32(node+92u,(uint32_t)(int32_t)(int8_t)PE_LoadU8(saved+2u));
}

void func_80064D08(pe_addr_t list)
{
    pe_addr_t parent=0u,p=PE_LoadU32(0x8009D154u),node;unsigned i;
    while (p) {
        for (i=0;i<4;i++) if (PE_LoadU32(p+8u+i*4u)==list) break;
        if (i<4) {parent=p;break;}
        p=PE_LoadU32(p);
    }
    node=menu_allocate(PE_LoadU32(list+4u),parent);
    PE_StoreU32(node+32u,3u);PE_StoreU32(node+48u,0x80064EB4u);PE_StoreU32(node+44u,0x800650E0u);
    PE_StoreU32(node+52u,list);PE_StoreU32(node+64u,PE_LoadU32(list+100u));PE_StoreU32(list+128u,node);
}

/* func_80064A54: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80064A54_port.c (src/func_80064A54.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_8006269C(pe_addr_t target)
{
    pe_addr_t node=PE_LoadU32(0x8009D154u),previous=0u;
    unsigned i;
    while (node && node!=target) {previous=node;node=PE_LoadU32(node);}
    if (!node) return;
    if (previous) PE_StoreU32(previous,PE_LoadU32(node));
    else PE_StoreU32(0x8009D154u,PE_LoadU32(node));
    PE_StoreU32(node,PE_LoadU32(0x8009D158u));PE_StoreU32(0x8009D158u,node);
    if (PE_LoadU32(node+32u)==2u) func_80064A54(node);
    for (i=0;i<4;i++) {
        pe_addr_t child=PE_LoadU32(node+8u+i*4u);
        if (child) func_8006269C(child);
    }
    if (func_80062CC4()==node) func_80062CB8(PE_LoadU32(node+4u));
    for (node=PE_LoadU32(0x8009D154u);node;node=PE_LoadU32(node)) {
        if (PE_LoadU32(node+4u)==target) PE_StoreU32(node+4u,0u);
        if (PE_LoadU32(node+32u)==2u) {
            if (PE_LoadU32(node+120u)==target) PE_StoreU32(node+120u,0u);
            if (PE_LoadU32(node+124u)==target) PE_StoreU32(node+124u,0u);
        }
        for (i=0;i<4;i++) if (PE_LoadU32(node+8u+i*4u)==target) PE_StoreU32(node+8u+i*4u,0u);
    }
}

void func_80062F3C(uint32_t id) {func_8006269C(func_80062A34(1u,id));}

void func_80064E90(pe_addr_t node)
{
    PE_StoreU32(PE_LoadU32(node+52u)+128u,0u);func_8006269C(node);
}

void func_800647D0(pe_addr_t node,int32_t items)
{
    int32_t columns=(int32_t)PE_LoadU32(node+52u);
    int32_t numerator=(int32_t)((uint32_t)items+(uint32_t)columns-1u),rows,visible,old_visible;
    pe_addr_t owner,scroll;
    /* Original DIV expands to explicit BREAKs for these invalid inputs. */
    if (!columns || (columns==-1 && numerator==INT32_MIN)) abort();
    rows=numerator/columns;PE_StoreU32(node+88u,(uint32_t)rows);
    PE_StoreU32(node+104u,columns==2?((uint32_t)items&1u):0u);
    if ((int32_t)PE_LoadU32(node+72u)>=rows) PE_StoreU32(node+72u,(uint32_t)rows-1u);
    if (PE_LoadU32(node+104u) && PE_LoadU32(node+68u)==1u &&
        PE_LoadU32(node+72u)==(uint32_t)rows-1u) PE_StoreU32(node+68u,0u);
    old_visible=(int32_t)PE_LoadU32(node+56u);
    visible=(int32_t)PE_LoadU32(node+108u);if (rows<visible) visible=rows;
    PE_StoreU32(node+56u,(uint32_t)visible);
    if ((int32_t)((uint32_t)rows-(uint32_t)visible)<(int32_t)PE_LoadU32(node+92u))
        PE_StoreU32(node+92u,(uint32_t)rows-(uint32_t)visible);
    owner=PE_LoadU32(node+4u);
    if (owner && PE_LoadU32(owner+32u)==1u)
        PE_StoreU32(owner+56u,PE_LoadU32(owner+56u)+PE_LoadU32(node+64u)*((uint32_t)visible-(uint32_t)old_visible));
    scroll=PE_LoadU32(node+128u);
    if (scroll) {
        if ((int32_t)PE_LoadU32(node+108u)>=rows) func_80064E90(scroll);
    } else if ((int32_t)PE_LoadU32(node+108u)<rows) func_80064D08(node);
}

pe_addr_t func_8006322C(uint32_t id,pe_addr_t owner,pe_addr_t parent)
{
    pe_addr_t spec=func_8005DAB4(id),node=menu_allocate(owner,parent);
    PE_StoreU32(node+32u,2u);PE_StoreU32(node+112u,id);PE_StoreU32(node+36u,id);
    PE_StoreU32(node+24u,PE_LoadU32(spec));PE_StoreU32(node+28u,PE_LoadU32(spec+4u));
    PE_StoreU32(node+44u,0x80063E0Cu);
    PE_StoreU32(node+84u,PE_LoadU32(spec+8u));PE_StoreU32(node+52u,PE_LoadU32(spec+8u));
    PE_StoreU32(node+108u,PE_LoadU32(spec+12u));PE_StoreU32(node+56u,PE_LoadU32(spec+12u));
    PE_StoreU32(node+88u,PE_LoadU32(spec+16u));PE_StoreU32(node+60u,PE_LoadU32(spec+20u));
    PE_StoreU32(node+64u,PE_LoadU32(spec+24u));PE_StoreU32(node+72u,0u);PE_StoreU32(node+68u,0u);
    PE_StoreU32(node+80u,UINT32_MAX);PE_StoreU32(node+76u,UINT32_MAX);PE_StoreU32(node+92u,0u);PE_StoreU32(node+96u,0u);
    PE_StoreU32(node+100u,PE_LoadU32(spec+28u));PE_StoreU32(node+124u,0u);PE_StoreU32(node+120u,0u);
    PE_StoreU32(node+132u,0u);PE_StoreU32(node+136u,0u);PE_StoreU32(node+104u,0u);PE_StoreU32(node+140u,0u);PE_StoreU32(node+128u,0u);
    if ((int32_t)PE_LoadU32(node+56u)<(int32_t)PE_LoadU32(node+88u)) func_80064D08(node);
    func_80064AC0(node);return node;
}
