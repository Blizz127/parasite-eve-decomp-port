/* Native CD DMA data-sync path, based on bounded original-code observations.
 * Device time advances through the existing HostFB_VSync/CD register model. */
#include "pe_plat/cd.h"
#include "pe_guest_ram.h"
#include "pe_cdreg.h"
#include "host_framebuffer.h"
#include "game_port.h"
#include "pe_guestcode.h"
#include "pe_bios_string.h"

static int cd_sync_timeout(void)
{
    uint32_t first;
    uint32_t second;
    (void)PE_GuestCall("func_8007BDDC_timeout_puts", 0x80073c5cu, 1u,
                       0x80011b18u, 0u, 0u, 0u);
    if (PE_Port_ShouldStop())
        return -1;
    first = PE_LoadU8(0x8009b294u);
    second = PE_LoadU8(0x8009b295u);
    (void)PE_Printf_Guest(0x80011b28u, PE_LoadU32(0x800a3480u),
                          PE_LoadU32(0x8009afdcu + PE_LoadU8(0x8009afd5u) * 4u),
                          PE_LoadU32(0x8009b05cu + first * 4u),
                          PE_LoadU32(0x8009b05cu + second * 4u), 0u, 0u, 0u);
    return -1;
}

int pe_plat_cd_data_sync(int mode)
{
    uint32_t now;
    if (PE_Port_ShouldStop())
        return -1;
    now = HostFB_VSync(-1);
    if (PE_Port_ShouldStop())
        return -1;
    PE_StoreU32(0x800a3478u, now + 0x3c0u);
    PE_StoreU32(0x800a347cu, 0u);
    PE_StoreU32(0x800a3480u, 0x80011c20u);
    for (;;) {
        uint32_t count;
        uint32_t control;
        now = HostFB_VSync(-1);
        if (PE_Port_ShouldStop())
            return -1;
        if ((int32_t)PE_LoadU32(0x800a3478u) < (int32_t)now)
            return cd_sync_timeout();
        count = PE_LoadU32(0x800a347cu);
        PE_StoreU32(0x800a347cu, count + 1u);
        if ((int32_t)count > 0x3c0000)
            return cd_sync_timeout();
        control = PE_CdLoadU32(PE_LoadU32(0x8009b2c0u));
        if ((control & 0x01000000u) == 0u)
            return 0;
        if (mode != 0)
            return 1;
    }
}
