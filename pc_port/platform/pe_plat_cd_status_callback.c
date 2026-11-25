/* Independent guest 80080164 callback, derived from original-code probes.
 * Includes independently authored command-complete/unexpected-status helpers. */
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_plat/cd_stream.h"
#include "pe_sdk.h"

void pe_plat_cd_status_callback(uint32_t status, pe_addr_t response)
{
    if (PE_Port_ShouldStop()) return;
    status &= 255u;
    func_8008080C(status, response);
    if (PE_Port_ShouldStop()) return;
    if (PE_LoadU8(0x8009B56Cu) & 0x10u) status = 5u;
    PE_StoreU32(0x8009B59Cu, 0u);
    PE_StoreU32(0x8009B598u, 0u);
    uint32_t state = PE_LoadU32(0x8009B570u);
    if (state == 31u) func_80080220(status, response);
    else if (state == 32u) func_80080404(status, response);
    else func_8008068C(status, response);
    if (PE_Port_ShouldStop()) return;
    if (PE_LoadU32(0x8009B598u) == 0u) PE_StoreU32(0x8009B570u, 33u);
}
