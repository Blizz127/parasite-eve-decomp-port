/*
 * Phase 6E-B54K-Q — execution-proven positive path of func_8005C1EC.
 *
 * Retail 0x8005C1EC..0x8005C25C gates a renderer/event state word.  The
 * func_801909B4 caller passes one: the first positive call publishes one at
 * D_8009D030 and sets D_800B0CD8 bits 0xC000; later positive calls are
 * inert.  The zero path first clears D_8009D030 and then calls the
 * translated event-record cleanup func_80042798 (Phase 6E-EV1), whose
 * 72774 vector-trampoline callee remains the narrowed boundary. After a
 * returning cleanup, retail clears scene bits 0xC000 (DAY1/DAY2-72).
 */
#include "psx_compat.h"
#include "game_port.h"

void func_8005C1EC(int enabled)
{
    if (enabled == 0) {
        unsigned epoch = PE_Port_StopEpoch();
        PE_StoreU32(0x8009D030u, 0u);
        func_80042798();
        if (PE_Port_StopEpoch() != epoch) return;
        PE_StoreU32(0x800B0CD8u, PE_LoadU32(0x800B0CD8u) & 0xFFFF3FFFu);
        return;
    }

    if (PE_LoadU32(0x8009D030u) == 0u) {
        PE_StoreU32(0x8009D030u, 1u);
        PE_StoreU32(0x800B0CD8u, PE_LoadU32(0x800B0CD8u) | 0xC000u);
    }
}
