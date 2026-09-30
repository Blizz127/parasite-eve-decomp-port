/*
 * Link/ownership smoke test for libpe_field_runtime.a.
 *
 * This is a host-consumer test, not new game behavior.  It verifies that the
 * archive's public include/link contract supplies the established guest-RAM,
 * callback/bootstrap, run-control, framebuffer, globals, and one translated
 * retail helper without relying on the CLI executable's source files.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_guest_ram.h"
#include "pe_callback.h"
#include "pe_bootstrap.h"
#include "game_port.h"
#include "host_framebuffer.h"

#include <stdint.h>

/* Translated code exposes this host trace hook as part of the consumer
 * contract.  The smoke consumer deliberately installs a no-op sink. */
void Trace_Direct(const char *event)
{
    (void)event;
}

int main(void)
{
    const pe_addr_t probe = 0x801FFFC0u;

    PE_RamInit();
    PE_Callback_Init();
    Bootstrap_Init();
    HostFB_Init();
    PE_Port_RunControlReset();

    PE_StoreU32(probe, 0x50454652u);
    if (PE_LoadU32(probe) != 0x50454652u) {
        PE_RamDestroy();
        return 1;
    }

    /* Retail-proven func_80077AA4 example from its existing native oracle. */
    if (func_80077AA4(0x130, 0x1F8) != 0x7E13u) {
        PE_RamDestroy();
        return 2;
    }

    PE_RamDestroy();
    return 0;
}
