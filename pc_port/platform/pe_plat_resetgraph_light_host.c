#include <stdint.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_guestcode.h"
#include "pe_plat/gpu_primitive.h"
#include "pe_mmio.h"
#include "pe_bios_string.h"
/* ResetGraph's light path, from the independently tested guest contract. */
int pe_plat_resetgraph_light_host(int mode) {
    int result = 0;
    unsigned epoch;
    if (PE_LoadU8(0x8009574eu) >= 2u) {
        uint32_t log = PE_LoadU32(0x80095748u);
        epoch = PE_Port_StopEpoch();
        if (log == 0x80071A74u) {
            uint32_t arg = (uint32_t)mode;
            (void)PE_Bios_Printf(0x80011800u, &arg, 1);
        } else if (!PE_GuestHook_Call("ResetGraph_debug", "func_80074A44", log, 2,
                                      0x80011800u, (uint32_t)mode, 0, 0, &result)) {
            return 0;
        }
        if (PE_Port_StopEpoch() != epoch) return 0;
    }
    uint32_t root = PE_LoadU32(0x80095744u);
    if (!root || (root & 3u) || root > UINT32_MAX - 0x34u ||
        !PE_RangeIsRam(root + 0x34u, 4u)) {
        PE_GuestHook_Refuse("ResetGraph_jtb13", "func_80074A44", root,
                            1, 1, 0, 0, 0, "invalid GPU jump table");
        return 0;
    }
    uint32_t target = PE_LoadU32(root + 0x34u);
    if (target == 0x80077144u) {
        const uint32_t words[] = {0x80095860u, 0x80095870u, 0x80095854u};
        for (unsigned i=0; i<3; ++i) {
            uint32_t ptr = PE_LoadU32(words[i]);
            if (!ptr || (ptr & 3u) || (!PE_RangeIsRam(ptr,4u) && !PE_MMIO_IsModelled(ptr,4u))) {
                PE_GuestHook_Refuse("ResetGraph_jtb13", "func_80074A44", target,
                                    1, 1, 0, 0, 0, "invalid GPU register pointer");
                return 0;
            }
        }
        return func_80077144(1);
    }
    epoch = PE_Port_StopEpoch();
    if (!PE_GuestHook_Call("ResetGraph_jtb13", "func_80074A44", target,
                           1, 1, 0, 0, 0, &result) ||
        PE_Port_StopEpoch() != epoch) return 0;
    return result;
}
