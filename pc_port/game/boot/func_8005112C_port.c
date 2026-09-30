/* Original command-history undo and record-to-inventory lookup.
 * 41898.s / 43724.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

static pe_addr_t undo_ram(pe_addr_t p) {return p<0x200000u?p|0x80000000u:p;}

int32_t func_800533D4(pe_addr_t record)
{
    int32_t index=0;
    while (index<(int32_t)D_8009D050) {
        if (func_8005332C(index)==record) break;
        index=(int32_t)((uint32_t)index+1u);
    }
    return index<(int32_t)D_8009D050?index:-1;
}

void func_8005112C(void)
{
    pe_addr_t entry=PE_LoadU32(0x8009D014u),slot,record;
    uint32_t kind;
    if (entry<=0x800A1AA0u) return;
    entry-=36u;kind=PE_LoadU32(undo_ram(entry));PE_StoreU32(0x8009D014u,entry);
    switch (kind) {
    case 0:
        slot=0x800C0E48u+PE_LoadU32(undo_ram(entry+8u))*2u;
        if (!PE_LoadU16(undo_ram(slot))) PE_StoreU16(undo_ram(slot),PE_LoadU32(undo_ram(entry+4u)));
        else func_80053D2C(PE_LoadU32(undo_ram(entry+4u)));
        break;
    case 2:
        PE_StoreU8(0x800C0E20u,(uint8_t)func_800533D4(PE_LoadU32(undo_ram(entry+4u))));
        break;
    case 3:
        record=PE_LoadU32(undo_ram(entry+4u));
        PE_StoreU8(0x800C0E22u,record?(uint8_t)func_800533D4(record):255u);
        break;
    case 4:
        record=PE_LoadU32(undo_ram(entry+4u));PE_StoreU16(undo_ram(record+10u),PE_LoadU32(undo_ram(entry+20u)));
        record=PE_LoadU32(undo_ram(entry+8u));PE_StoreU16(undo_ram(record+10u),PE_LoadU32(undo_ram(entry+28u)));
        record=PE_LoadU32(undo_ram(entry+12u));PE_StoreU16(undo_ram(record+10u),PE_LoadU32(undo_ram(entry+32u)));
        break;
    default:break;
    }
}
