/* Independent CD data-ready callback (guest 0x8007C214).
 * Derived from 12000 original-code vectors, interface declarations and
 * existing hook integration tests; no legacy SDK implementation read. */
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "game_port.h"

void pe_plat_cd_data_ready_callback(void)
{
    uint32_t entry, callback;
    if (PE_Port_ShouldStop()) return;
    entry = PE_LoadU32(0x800C0DC8u) + PE_LoadU32(0x800BE9E4u) * 32u;
    PE_StoreU16(entry, 2u);
    /* Opaque header words: offsets and order are observable in the vectors. */
    PE_StoreU32(0x800A3490u, PE_LoadU32(entry + 28u));
    PE_StoreU32(0x800A3494u, PE_LoadU32(entry + 8u));
    PE_StoreU32(0x800BE9E4u, PE_LoadU32(0x800BE998u));
    callback = PE_LoadU32(0x800B0CC8u);
    if (callback) {
        int result = 0;
        if (!PE_GuestHook_Call("func_8007C214_B0CC8_chain", "func_8007C214",
                              callback, 4u, callback, 0u, 0x800A3490u, 0u,
                              &result) || PE_Port_ShouldStop()) return;
    }
    PE_StoreU32(0x800B89F4u, 0u);
}
