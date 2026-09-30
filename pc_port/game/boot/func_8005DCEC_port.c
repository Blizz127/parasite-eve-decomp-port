/* Original inventory description and filtered-list lookups.
 * 4E44C.s / 43CE4.s / 44AA0.s / 486D8.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

pe_addr_t func_8005DCEC(uint32_t id)
{
    pe_addr_t archive=0x800A8028u+PE_LoadU32(0x800A802Cu);
    pe_addr_t table=archive+PE_LoadU32(archive+12u);
    if (id>=PE_LoadU16(table)) return 0u;
    return table+(uint32_t)(int32_t)(int16_t)PE_LoadU16(table+id*2u+2u);
}

int32_t func_8005415C(int32_t index)
{
    pe_addr_t record=func_8005332C(index);
    return record?PE_LoadU8(record+6u):0;
}

/* func_800556E8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800556E8_port.c (src/func_800556E8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80054288: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80054288_port.c (src/func_80054288.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static int32_t filtered_item(int32_t index,pe_addr_t count,pe_addr_t table)
{
    if (index<0 || index>=(int32_t)PE_LoadU32(count)) return 0;
    return (int16_t)PE_LoadU16(table+(uint32_t)index*2u);
}
/* func_80058E08: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80058E08_port.c (src/func_80058E08.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80057ED8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80057ED8_port.c (src/func_80057ED8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

pe_addr_t func_80058BBC(int32_t index)
{
    int32_t value=(int16_t)PE_LoadU16(PE_LoadU32(0x8009D07Cu)+(uint32_t)index*2u);
    if ((uint32_t)(value-256)<128u) return 0x800BEEACu+(uint32_t)value*32u;
    if ((uint32_t)(value-1)<255u) return func_8005DB44((uint32_t)(value-1));
    if ((uint32_t)(value-512)<9u) return 0x8009DE64u+(uint32_t)value*32u;
    return 0u;
}

int32_t func_80059F08(uint32_t index)
{
    if (index>=2u) return -1;
    func_80052E30(PE_LoadU32(0x8009D098u+index*4u));
    return (int32_t)PE_LoadU32(0x8009D090u+index*4u);
}

/* func_80055610: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80055610_port.c (src/func_80055610.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
