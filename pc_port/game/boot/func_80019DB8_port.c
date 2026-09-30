/* Script D2/D3,83 original words19DB8..19F04.
 * SHA256 f684e1f70afe44e8a2195316e020111fecf4c6ad3f99e2e1e3b0ad297662effe. */
#include "pe_port_compat.h"
/* func_80019DB8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019DB8_port.c (src/func_80019DB8.c); hand port retired (VM batch 3, port3). */
int func_80019DF4(pe_addr_t args)
{
    /* Original signed multiply-high division sequences truncate toward zero.
     * Reload both the source pointer and its value after each store. */
    int32_t ticks=(int32_t)PE_LoadU32(PE_LoadU32(args));
    PE_StoreU32(PE_LoadU32(args+4u),(uint32_t)(ticks/216000));
    ticks=(int32_t)PE_LoadU32(PE_LoadU32(args));
    PE_StoreU32(PE_LoadU32(args+8u),(uint32_t)((ticks%216000)/3600));
    ticks=(int32_t)PE_LoadU32(PE_LoadU32(args));
    PE_StoreU32(PE_LoadU32(args+12u),(uint32_t)((ticks%3600)/60));
    return 1;
}
