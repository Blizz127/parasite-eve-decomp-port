/* Independent CD command retry (guest 0x800800F4), inferred from black-box
 * original-code vectors. No legacy SDK implementation read. */
#include "pe_guest_ram.h"
#include "pe_sdk.h"
#include "game_port.h"

void pe_plat_cd_retry_command(void)
{
    if (PE_Port_ShouldStop()) return;
    pe_plat_cd_flush();
    if (PE_Port_ShouldStop()) return;
    PE_StoreU32(0x8009B59Cu, PE_LoadU32(0x8009B59Cu) + 1u);
    uint32_t command = PE_LoadU8(0x8009B558u);
    uint32_t flag = PE_LoadU32(0x8009B5A4u + 4u * command);
    PE_StoreU32(0x8009B598u, flag ? 960u : 30u);
    (void)func_8007B558(PE_LoadU8(0x8009B558u), PE_LoadU32(0x8009B560u), 0u, 1u);
}
