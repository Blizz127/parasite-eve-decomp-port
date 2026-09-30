/* Club effect constructor, target origin, spark initialization and drawing.
 * Original BE74C/BE96C/BE9FC/BEBB4.s; CE144 is matching src/ C.
 * Shared storage, VM, rendering and matched update leaves remain native. */
#include "psx_compat.h"
#include "pe_port_compat.h"

int func_800CE084(pe_addr_t slot)
{
    unsigned i;
    PE_StoreU32(func_800C22F8(slot),0x800E0FFCu);
    for (i=0;i<3;i++) PE_StoreU32(0x800E22B8u+i*4u,0x300u);
    PE_StoreU8(0x800E22CDu,5u);PE_StoreU16(0x800E22CEu,(uint16_t)-100);
    for (i=0;i<3;i++) PE_StoreU16(0x800E22B0u+i*2u,0u);
    for (i=0;i<3;i++) PE_StoreU8(0x800E22C8u+i,128u);
    return 0;
}

/* func_800CE16C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800CE16C_port.c (src/func_800CE16C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_800CE1FC(void)
{
    pe_addr_t body=PE_LoadU32(0x800E2248u);
    uint32_t index=PE_LoadU32(body+60u);
    pe_addr_t list=PE_LoadU32(body+76u),actor=PE_LoadU32(list+index*4u);
    unsigned i;
    PE_StoreU32(0x800E2848u,actor);
    if (!PE_LoadU32(list+(index+1u)*4u)) PE_StoreU32(PE_LoadU32(0x800E2248u)+64u,1u);
    PE_StoreU32(PE_LoadU32(0x800E2248u)+60u,index+1u);
    actor=PE_LoadU32(0x800E2848u);
    for (i=0;i<3;i++) PE_StoreU16(0x800E2808u+i*2u,PE_LoadU16(actor+616u+i*2u));
    func_800CEDA8(0);
}

/* func_800CE2B4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800CE2B4_port.c (src/func_800CE2B4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800CE3B4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800CE3B4_port.c (src/func_800CE3B4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
