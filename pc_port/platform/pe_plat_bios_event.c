/* BIOS event-service entry points (kernel B0 vector).  Clean-room: written
 * from the psx-spx kernel/BIOS documentation, "BIOS Event Functions"
 * (https://psx-spx.consoledev.net/kernelbios/); no Sony code or tables.  The
 * retail addresses are four-word BIOS trampolines (load the B0 table address
 * and the function number, jump), so the contract is the BIOS call:
 *   B(0Bh) TestEvent(event)         1 if the event is enabled/ready (and it
 *                                   switches back to enabled/busy), else 0;
 *   B(09h) CloseEvent(event)        releases the event, returns 1;
 *   B(0Dh) DisableEvent(event)      turns the event off, returns 1;
 *   B(20h) UnDeliverEvent(class,spec)  moves enabled/ready mode-2000h events
 *                                   of that class/spec back to enabled/busy.
 * These wrappers only forward to the host event table PE_Event_* (platform/
 * pe_libetc.c, 6f749ac5b "Host DeliverEvent model"); what that table does for
 * Disable/Close/UnDeliver beyond the return values and TestEvent is its own
 * behaviour (known gaps against the text above are listed in
 * docs/evidence/native-pad-bios-rows-20261125/REPORT.md).  The guest names are
 * bound in pe_plat/bios_event.h. */
#include "pe_plat/bios_event.h"
#include "pe_sdk.h"

int pe_plat_bios_test_event(uint32_t handle)
{
    return PE_Event_Test((int)handle);
}

int pe_plat_bios_close_event(int handle)
{
    return PE_Event_Close(handle);
}

int pe_plat_bios_disable_event(int handle)
{
    return PE_Event_Disable(handle);
}

void pe_plat_bios_undeliver_event(unsigned int cls, int spec)
{
    (void)PE_Event_UnDeliver(cls, (uint32_t)spec);
}
