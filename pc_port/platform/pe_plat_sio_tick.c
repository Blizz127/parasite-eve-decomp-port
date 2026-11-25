/* One iteration of a guest SIO / interrupt wait loop (port glue, not game or
 * SDK behaviour).  The port's clock is deterministic CPU-cycle time: nothing
 * advances it while native code spins on a register, so a wait whose exit is
 * an RCNT2 timeout (0x80084FC4 / 0x80084FE4) would never end.  Each poll
 * therefore stands for the CPU time of one guest poll iteration:
 *   - with the CD device attached, HostFB_DevicePoll() (64 cycles of device
 *     time, which advances RCNT2 together with the CD drive);
 *   - otherwise 64 cycles of RCNT2 time directly.
 * 64 cycles is the cost HostFB_DevicePoll already charges for one guest poll
 * iteration (a few dozen instructions with uncached register reads). */
#include "pe_plat/bios_file.h"
#include "pe_plat/timing.h"
#include "host_framebuffer.h"
#include "pe_cdreg.h"

#define SIO_POLL_CYCLES 64u

void pe_plat_sio_poll_tick(void)
{
    if (PE_CdReg_DeviceEnabled())
        HostFB_DevicePoll();
    else
        pe_plat_timing_advance_cycles(SIO_POLL_CYCLES);
}
