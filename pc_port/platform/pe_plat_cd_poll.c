/* Independent CD read poll (guest 0x800811E4), inferred from original-code
 * vectors and interface headers. No legacy SDK implementation read. */
#include "pe_guest_ram.h"
#include "pe_plat/cd.h"
#include "psx_compat.h"

static int deadline_expired(uint32_t now)
{
    uint32_t deadline = PE_LoadU32(0x8009B6C4u) + 1200u;
    /* Compare signed 32-bit words after deadline wrap, without C overflow. */
    return (now ^ 0x80000000u) > (deadline ^ 0x80000000u);
}

int pe_plat_cd_read_poll(uint32_t fp)
{
    int timed_out = deadline_expired(func_80073A44(-1));
    int result;
    if (timed_out) { pe_plat_ds_81268(); result = -1; }
    else result = (int32_t)PE_LoadU32(0x8009B6B4u);
    (void)pe_plat_ds_ready((int32_t)fp);
    return result;
}

/* Existing synchronous-host contract, independently observed from the frozen
 * production object: query its counter without advancing it; timeout clears
 * the active flag locally. Do not enter the retail abort/callback chain here.
 * The retail core above remains separately oracle-checked, not production-
 * bound. Host equivalence is not full retail-device coverage. */
int pe_plat_cd_read_poll_host(uint32_t fp)
{
    int vsync;
    (void)fp;
    HostFB_GetState(&vsync, NULL, NULL, NULL);
    if (deadline_expired((uint32_t)vsync)) {
        PE_StoreU32(0x8009B6CCu, 0u);
        return -1;
    }
    return (int32_t)PE_LoadU32(0x8009B6B4u);
}
