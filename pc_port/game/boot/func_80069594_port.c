/* Original first effect-pool draw/update pump (69594 / 6F8EC).
 * D4704 and 6F9F0 are implemented with the room-effect VM. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

#define GA_D_800942E0 0x800942E0u
#define GA_D_800942E4 0x800942E4u
#define GA_D_800942E8 0x800942E8u
#define GA_D_800F32D0 0x800F32D0u
#define GA_D_800E2368 0x800E2368u
#define GA_FN_D4704   0x800D4704u



int func_8006F8EC(unsigned int index)
{
    pe_addr_t pool;
    pe_addr_t slot;
    pe_addr_t table;
    pe_addr_t entry;
    pe_addr_t fn;
    unsigned int used;
    unsigned int code;
    unsigned int stride;

    if (index >= 0x16u)
        return -16;
    if (index < 0xBu) {
        pool = PE_LoadU32(GA_D_800942E4);
        stride = 0xA0Cu;
        slot = pool + index * stride;
    } else {
        pool = PE_LoadU32(GA_D_800942E8);
        stride = 0x10Cu;
        slot = pool + (index - 0xBu) * stride;
    }

    used = PE_LoadU8(slot);
    if ((used - 1u) >= 2u)
        return 0;
    code = PE_LoadU8(slot + 1u);
    if (code >= 0xC0u)
        return -17;
    if (code >= 0x55u)
        code = 0x55u;
    table = PE_LoadU32(GA_D_800942E0);
    entry = PE_LoadU32(table + code * 4u);
    if (entry == 0u)
        return -18;
    fn = PE_LoadU32(entry + 0x0Cu);
    if (fn == 0u)
        return -1;
    if (fn == GA_FN_D4704)
        return func_800D4704(slot);
    if (fn==0x800C9B68u || fn==0x800CD8C8u || fn==0x800CE144u) return PE_EffectStackWeaponDraw(fn,slot);
    PE_EffectStackInvalidate();
    return PE_EffectCallback(fn,2,slot,0u);
}

int func_80069594(void)
{
    unsigned int i;

    if ((D_8009D1A0 & 0x80u) == 0u) {
        PE_EffectStackInvalidate();return 0;
    }
    PE_EffectStackBegin();
    func_800661A4();
    for (i = 0; i < 11u; i++) {
        (void)func_8006F8EC(i);
        if(PE_Port_ShouldStop())break;
    }
    func_800661CC();
    if ((D_8009D1A0&4u) || PE_Port_ShouldStop()) {PE_EffectStackEnd();return 0;}
    for (i=0;i<11u;i++) {
        unsigned code=PE_LoadU8(PE_LoadU32(GA_D_800942E4)+i*0xA0Cu+1u);
        if (!(D_8009D1A0&0x100u) || (code-0x55u)<0x1Eu)
            (void)func_8006F9F0(i);
    }
    PE_EffectStackEnd();return 0;
}

/* Original 69660: small-pool draw followed by updates, before field collision. */
/* func_80069660: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80069660_port.c (src/func_80069660.c); hand port retired (port3 switch-over A). */
