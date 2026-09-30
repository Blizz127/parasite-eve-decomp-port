/* Complete original input handler 8003EB04..8003F074,348 words.
 * SHA256 c6a27aa96c180ee441866ac86dfc995683fb48c2366dfd864f0bbd03aeb4660b.
 * Controller setup, hold counters, priority masks, analog axes and edges. */
#include "pe_port_compat.h"
#include "game_port.h"
#define W(a) PE_LoadU32(a)
#define S(a,v) PE_StoreU32(a,v)
#define CHECK() do {if(PE_Port_StopEpoch()!=epoch)return;} while(0)
/* func_8003EB04: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8003EB04_port.c (src/func_8003EB04.c); hand port retired (port3 switch-over C). */
#undef W
#undef S
#undef CHECK
