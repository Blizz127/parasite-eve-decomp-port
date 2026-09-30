/* M0000I object pool and constructors, original overlay LBA2805 SHA256
 * c51e36c27422e990d4683d73dc9fc2633e0924721dd0c242a8efc2e8520a4edb.
 * Native translation of 90998..90D3C, 91580..91854 and 9959C..995BC.
 * Preserve the original indexed links, signed halfword indices and partial
 * record initialization. This is not matching src/ C. */
#include "psx_compat.h"
#include "pe_port_compat.h"

static pe_addr_t pool_link(int32_t index)
{
    return 0x801E4A88u + (uint32_t)index * 4u;
}

static pe_addr_t pool_object(int32_t index)
{
    return 0x801E4E00u + (uint32_t)index * 108u;
}

void func_80191580(pe_addr_t object)
{
    PE_StoreU32(object + 0x24u, 1000u);
    PE_StoreU32(object + 0x1Cu, 0u);
    PE_StoreU32(object + 0x20u, 0u);
    for (unsigned i=0;i<9u;i++)
        PE_StoreU16(object + 8u + i*2u, i%4u == 0u ? 4096u : 0u);
    for (unsigned i=0;i<3u;i++) PE_StoreU16(object + 0x28u + i*2u, 0u);
    PE_StoreU32(object + 0x60u, 0x8019C340u);
    PE_StoreU32(object + 0x64u, 0u);
    PE_StoreU32(object + 0x68u, 0u);
}

void func_80190998(void)
{
    for (unsigned i=1;i<200u;i++) func_80191580(pool_object((int32_t)i));
    func_80191580(0x8019C340u);
    PE_StoreU16(0x8019C39Cu, 0u);
    PE_StoreU16(0x8019CBC0u, 0u);
    for (unsigned i=0;i<200u;i++) PE_StoreU16(0x8019C830u+i*2u, (uint16_t)(i+1u));
    for (unsigned i=0;i<3u;i++) {
        uint32_t off=i*108u;
        PE_StoreU32(0x8019CCBCu+off, 0x801EA5E8u+off);
        PE_StoreU32(0x801EA650u+off, 0x8019CC58u+off);
        PE_StoreU32(0x801EA64Cu+off, 0u);
        PE_StoreU32(0x8019CCC0u+off, 0u);
    }
    for (unsigned i=0;i<200u;i++) {
        PE_StoreU16(pool_link((int32_t)i), 0xFFFFu);
        PE_StoreU16(pool_link((int32_t)i)+2u, 0xFFFFu);
    }
    PE_StoreU16(0x8019C9D0u, 200u);
    PE_StoreU16(pool_link(200), 0xFFFFu);
    PE_StoreU16(pool_link(200)+2u, 201u);
    PE_StoreU16(pool_link(201), 200u);
    PE_StoreU16(pool_link(201)+2u, 0xFFFFu);
}

int32_t func_801915DC(void)
{
    int32_t index=(int16_t)PE_LoadU16(0x8019CBC0u);
    uint16_t tail=PE_LoadU16(0x8019C9D0u);
    pe_addr_t link=pool_link(index), previous=pool_link((int16_t)tail);
    PE_StoreU16(link, tail);
    uint16_t next=PE_LoadU16(previous+2u);
    uint16_t free_next=PE_LoadU16(0x8019C830u+(uint32_t)index*2u);
    PE_StoreU16(link+2u, next);
    PE_StoreU16(previous+2u, (uint16_t)index);
    int32_t following=(int16_t)PE_LoadU16(link+2u);
    PE_StoreU16(0x8019C9D0u, (uint16_t)index);
    PE_StoreU16(0x8019CBC0u, free_next);
    PE_StoreU16(pool_link(following), (uint16_t)index);
    return index;
}

void func_80191678(uint32_t id)
{
    int32_t index=(int16_t)id;
    pe_addr_t link=pool_link(index);
    int32_t previous=(int16_t)PE_LoadU16(link);
    uint16_t next=PE_LoadU16(link+2u);
    PE_StoreU16(pool_link(previous)+2u, next);
    int32_t following=(int16_t)PE_LoadU16(link+2u);
    PE_StoreU16(pool_link(following), PE_LoadU16(link));
    if (index==(int16_t)PE_LoadU16(0x8019C9D0u))
        PE_StoreU16(0x8019C9D0u, PE_LoadU16(link));
    uint16_t free_head=PE_LoadU16(0x8019CBC0u);
    PE_StoreU16(link, 0xFFFFu);
    PE_StoreU16(link+2u, 0xFFFFu);
    PE_StoreU16(0x8019CBC0u, (uint16_t)id);
    PE_StoreU16(0x8019C830u+(uint32_t)index*2u, free_head);
}

void func_80191740(void) { PE_StoreU16(0x8019CC50u, 200u); }

pe_addr_t func_80191754(void)
{
    int32_t current=(int16_t)PE_LoadU16(0x8019CC50u);
    int32_t next=(int16_t)PE_LoadU16(pool_link(current)+2u);
    PE_StoreU16(0x8019CC50u, (uint16_t)next);
    if (next==201) next=-1;
    return next==-1 ? 0u : pool_object(next);
}

void func_801917BC(pe_addr_t object, pe_addr_t parent)
{
    PE_StoreU32(object+0x60u, parent);
    uint16_t depth=(uint16_t)(PE_LoadU16(parent+0x5Cu)+1u);
    PE_StoreU16(object+0x5Cu, depth);
    pe_addr_t tail=0x8019CCBCu+(uint32_t)(int32_t)(int16_t)depth*108u;
    PE_StoreU32(object+0x64u, PE_LoadU32(tail));
    PE_StoreU32(object+0x68u, PE_LoadU32(PE_LoadU32(tail)+0x68u));
    PE_StoreU32(PE_LoadU32(tail)+0x68u, object);
    PE_StoreU32(tail, object);
}

void func_80191834(pe_addr_t object)
{
    pe_addr_t previous=PE_LoadU32(object+0x64u);
    PE_StoreU32(previous+0x68u, PE_LoadU32(object+0x68u));
    pe_addr_t next=PE_LoadU32(object+0x68u);
    PE_StoreU32(next+0x64u, PE_LoadU32(object+0x64u));
}

uint32_t func_8019959C(pe_addr_t resource, uint32_t index)
{
    pe_addr_t record=resource+PE_LoadU32(resource+index*4u);
    return PE_LoadU32(record+0x30u);
}

pe_addr_t func_80190AEC(pe_addr_t parent, uint32_t value)
{
    int32_t id=func_801915DC();
    pe_addr_t object=pool_object((int16_t)id);
    PE_StoreU16(object+2u, 0u);
    PE_StoreU32(object+4u, value);
    PE_StoreU16(object, (uint16_t)id);
    func_801917BC(object,parent);
    PE_StoreU32(object+0x1Cu,0u);PE_StoreU32(object+0x20u,0u);PE_StoreU32(object+0x24u,0u);
    return object;
}

pe_addr_t func_80190B78(pe_addr_t parent, uint32_t index, pe_addr_t resource)
{
    int32_t id=func_801915DC();
    pe_addr_t object=pool_object((int16_t)id);
    PE_StoreU16(object+2u,(uint16_t)index);
    PE_StoreU32(object+4u,resource);
    PE_StoreU16(object,(uint16_t)id);
    PE_StoreU16(object+0x32u,(uint16_t)func_8019959C(resource,(uint32_t)(int32_t)(int16_t)index));
    func_801917BC(object,parent);
    PE_StoreU32(object+0x1Cu,0u);PE_StoreU32(object+0x20u,0u);PE_StoreU32(object+0x24u,0u);
    return object;
}

pe_addr_t func_80190C1C(pe_addr_t parent, uint32_t index, uint32_t a2,
                       uint32_t a3, uint32_t a4, uint32_t a5, pe_addr_t resource)
{
    int32_t id=func_801915DC();
    pe_addr_t object=pool_object((int16_t)id);
    PE_StoreU16(object+6u,(uint16_t)index);PE_StoreU16(object+4u,(uint16_t)a2);
    PE_StoreU16(object+2u,(uint16_t)a3);PE_StoreU16(object,(uint16_t)id);
    PE_StoreU32(object+8u,resource);
    PE_StoreU16(object+0x36u,(uint16_t)func_8019959C(resource,(uint32_t)(int32_t)(int16_t)index));
    PE_StoreU32(object+0x38u,a4);PE_StoreU32(object+0x3Cu,a5);
    func_801917BC(object,parent);
    PE_StoreU32(object+0x20u,0u);PE_StoreU32(object+0x24u,0u);PE_StoreU32(object+0x28u,0u);
    return object;
}

void func_80190D08(pe_addr_t object)
{
    func_80191834(object);
    func_80191678((uint32_t)(int32_t)(int16_t)PE_LoadU16(object));
}
