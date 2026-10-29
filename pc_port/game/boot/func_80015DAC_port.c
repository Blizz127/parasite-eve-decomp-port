/* Field audio adapter: the complete matched-source 15DAC dispatcher runs
 * in generated native C. Keep the existing entry/log contract, not a second
 * hand translation or a fixed guest scratch slot. Retail differential and
 * coverage limits: docs/evidence/native-audio-dispatch-20261027/.
 * 173F4 below remains its seven-word native assignment adapter.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_plat/timing.h"
#include <stdio.h>
#include <stdlib.h>

extern int func_80015DAC(pe_addr_t args);
#define GA_EA_TABLE 0x800101B0u

/* Historical exported entry; callers supply the ordinary EA argument list. */
int func_80015DAC_key190_cut(pe_addr_t args)
{
    return func_80015DAC(args);
}

int func_80015DAC_default_cut(pe_addr_t args)
{
    uint32_t key;

    key = PE_LoadU32(PE_LoadU32(args));
    {
        /* audio2 diagnostics: PE_AUDIO_EA_LOG=path logs every EA key with
         * its first operands. The matched dispatcher supplies every retail
         * case; unknown keys take its original no-op default. */
        static FILE *ea_log;
        static int ea_init;
        if (!ea_init) {
            const char *p = getenv("PE_AUDIO_EA_LOG");
            ea_init = 1;
            if (p && *p) ea_log = fopen(p, "w");
        }
        if (ea_log) {
            uint32_t i2 = key - 100u;
            pe_addr_t tg = i2 < 311u ? PE_LoadU32(GA_EA_TABLE + i2 * 4u) : 0u;
            uint32_t op[4];
            for (unsigned k = 0; k < 4u; k++) {
                pe_addr_t pp = PE_RangeIsRam(args + (k + 1u) * 4u, 4u) ?
                               PE_LoadU32(args + (k + 1u) * 4u) : 0u;
                op[k] = PE_RangeIsRam(pp, 4u) ? PE_LoadU32(pp) : 0xDEADu;
            }
            fprintf(ea_log, "vbl=%u EA %u (table %08X) ops %X %X %X %X\n",
                    pe_plat_timing_vblank_count(), key, tg, op[0], op[1], op[2], op[3]);
            fflush(ea_log);
        }
    }
    return func_80015DAC(args);
}

int func_800173F4(pe_addr_t args)
{
    PE_StoreU32(PE_LoadU32(args), PE_LoadU32(PE_LoadU32(args + 4u)));
    return 1;
}

/* func_80017E20: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80017E20_port.c (src/func_80017E20.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
