/* Host seams of the CD stream family (cdx-stream): device attach/detach, the address-keyed data-callback
 * dispatcher, the stream auxiliary hook and the named stream stop.
 *
 * PROVENANCE: written clean-room from the contracts only; no original, translated or decompiled code was read
 * (the former translated provider, game/boot/cd_stream_port.c, was never opened).  Sources of each contract:
 *  - pe_sdk.h: the PeCdAttach / PE_Cd_AttachDevice / PE_Cd_DetachDevice prototypes and comments, the
 *    PE_Cd_DispatchDataCallback prototype and comment ("returns 1 when handled, 0 when the default arm raised
 *    the explicit unresolved-callback boundary"); pe_plat/cd_regs.h (PE_CdReg_EnableDevice /
 *    PE_CdReg_DetachDevice: "mid-game detach keeps the CD audio mix"), pe_cdreg.c (PE_CdReg_DetachDevice and
 *    PE_CdReg_EnableDevice bodies), pe_plat/irq_callback.h, pe_plat/cdx_etc.h, pe_callback.h,
 *    pe_guestcode.h / pe_guestcode.c (PE_GuestHook_Call), pe_bootstrap.h;
 *  - callers: bootstrap/func_800698D4_port.c (first mount: Attach, quiet window, mount, Detach; "no disc image,
 *    device already enabled, or not host-booted idle" refusal text), bootstrap/func_8001220C_port.c (credits
 *    entry/exit), platform/pe_plat_cd_stream_handler_host.c and pe_plat_cd_dma_issue_host.c (stream seams),
 *    pe_plat_cd_sector_dma_host.c (the same named-stop idiom), pe_plat_cd_{command_complete,command_issue_host,
 *    status_poll_host,ready,gen7,gen33}.c, pe_plat_ds_gen39.c, pe_plat_81110.c (dispatcher callers),
 *    pe_plat_cd_reset_host.c / pe_plat_cd_initialize_callbacks.c / pe_plat_cd_gen33.c (host-booted and lane words);
 *  - tests: test_credits_native.h (credits_attach_from_host_boot, the CXA scenarios), test_native.c
 *    (698D4_quiet_window_takes_no_emulated_time, 698D4 mount), test_cd_stream.h, test_cd_dma.h,
 *    test_hook_registry.h (HOOKREG_cd_aux_registry_continues), test_cd_dispatch.h, test_cd_command.h,
 *    test_cd_device.h, test_cd_sync_callbacks.h, the retail_cd_stream_cases.h seed/boundary table (as data),
 *    the stub dispatchers in tests/test_cd_*.c.in;
 *  - psx-spx (CD-ROM controller: interrupt enable register, INT1..INT5; Interrupts: I_MASK / I_STAT). */
#include <stdint.h>
#include <stdio.h>

#include "pe_sdk.h"
#include "pe_bootstrap.h"
#include "pe_callback.h"
#include "pe_guestcode.h"
#include "pe_plat/cd_regs.h"
#include "pe_plat/cdx_etc.h"
#include "pe_plat/cdx_stream_seams.h"
#include "pe_plat/irq_callback.h"
#include "game_port.h"

/* ---- guest words the seams read (all little-endian guest RAM) ------- */
#define W_HOST_BOOTED   0x8009B554u  /* 1 once libds runs in host mode (DsInit host branch / CdReset host) */
#define W_LANE          0x8009B574u  /* drive command lane; 1 = idle */
#define W_QUEUE_LEN     0x800A3608u  /* pending command-queue depth */
#define W_IRQ_SLOT2     (0x800945E8u + 8u)  /* CPU interrupt handler slot of source 2 (the CD controller) */
#define W_STREAM_HOOK   0x800B0CCCu  /* stream auxiliary hook word */

#define CD_INTR_HANDLER 0x8007C13Cu  /* handler the device branch registers for source 2 */
#define CD_VSYNC_PUMP   0x8007FE24u  /* per-VBlank drive worker the device branch registers in VSync slot 0 */
#define CD_INTR_ENABLE  7u           /* INT1..INT3 enabled (psx-spx: interrupt enable register) */

/* ---- device attach / detach ------------------------------------------ */
/* Production boots libds in host mode, so the overlay's own DsInit is a no-op and the two hooks of its device
 * branch are never installed.  Attach supplies exactly those two (InterruptCallback(2, handler) and
 * VSyncCallbacks(0, pump)) and enables the controller model; it issues no command (no Getstat/Init/Demute).
 * It refuses (returns 0, saved->attached = 0, nothing changed) unless a disc is active, the device is not
 * already enabled, and libds is host-booted with its lane idle and its queue empty.  Detach restores both slots
 * (which restores I_MASK and the registered-source mask through the same registry path) and ends the device
 * model keeping the CD audio mix latches. */
int PE_Cd_AttachDevice(PeCdAttach *saved)
{
    uint32_t irq2, vsync0;

    if (!saved) return 0;
    saved->irq2 = 0u;
    saved->vsync0 = 0u;
    saved->attached = 0;
    if (PE_CdReg_DeviceEnabled()) return 0;
    if (PE_LoadU32(W_HOST_BOOTED) != 1u || PE_LoadU32(W_LANE) != 1u || PE_LoadU32(W_QUEUE_LEN) != 0u) return 0;
    irq2 = PE_LoadU32(W_IRQ_SLOT2);
    vsync0 = PE_Callback_GetSlot(0u);
    if (!PE_CdReg_EnableDevice(CD_INTR_ENABLE)) return 0;
    if (PE_Callback_Bind(CD_VSYNC_PUMP, func_8007FE24) != 0) {
        PE_CdReg_DetachDevice();
        return 0;
    }
    (void)pe_plat_interrupt_callback(2u, CD_INTR_HANDLER);
    (void)pe_plat_cdxetc_vsync_callbacks(0u, CD_VSYNC_PUMP);
    saved->irq2 = irq2;
    saved->vsync0 = vsync0;
    saved->attached = 1;
    return 1;
}

void PE_Cd_DetachDevice(const PeCdAttach *saved)
{
    if (!saved || !saved->attached) return;
    (void)pe_plat_cdxetc_vsync_callbacks(0u, saved->vsync0);
    (void)pe_plat_interrupt_callback(2u, saved->irq2);
    PE_CdReg_DetachDevice();
}

/* ---- data-callback dispatcher ---------------------------------------- */
/* The retail `jalr` through a CD callback word, called with (status, response).  The target runs through the
 * guest-code registry (native C at that address, entries with up to two register parameters; a registered
 * entry that takes more is refused because the site passes only two).  A target with no C there stops at the
 * named boundary "CD_data_callback" carrying the target and both arguments (a refusal, which does not depend on
 * the boundary policy).  The registry's variadic-style entries (the BIOS printf entry takes its arguments from
 * a1 upward) run through PE_GuestCall, as the tests CDSYNC_registry_arguments and CDSYNC_unknown_callbacks
 * require; PE_GuestHook_Call would refuse them.  Returns 1 when the target was called (the caller checks
 * PE_Port_ShouldStop afterwards; the callee may itself have requested a stop), 0 when it was refused. */
int PE_Cd_DispatchDataCallback(pe_addr_t target, uint32_t status, pe_addr_t response)
{
    const PeGuestCodeEntry *e = target ? PE_GuestCode_Resolve(target) : NULL;

    if (!e || (!e->thunk && !e->thunk8)) {
        PE_GuestHook_Refuse("CD_data_callback", "PE_Cd_DispatchDataCallback", target, 2u, status, response, 0u, 0u,
                            "no C for this callback target");
        return 0;
    }
    if (!e->thunk && e->nargs > 2u) {
        PE_GuestHook_Refuse("CD_data_callback", "PE_Cd_DispatchDataCallback", target, 2u, status, response, 0u, 0u,
                            "its C takes stack arguments the callback site does not pass");
        return 0;
    }
    (void)PE_GuestCall("CD_data_callback", target, 2u, status, response, 0u, 0u);
    return 1;
}

/* ---- stream handler seams -------------------------------------------- */
/* The stream-assembly handler's `jalr` through D_800B0CCC with $a0 = the record/start word it hands over.
 * An empty hook word is nothing to call. */
int pe_plat_cd_stream_auxiliary(uint32_t record)
{
    uint32_t hook = PE_LoadU32(W_STREAM_HOOK);

    if (hook == 0u) return 1;
    return PE_GuestHook_Call("func_8007C564_auxiliary_callback", "func_8007C564", hook, 1u,
                             record, 0u, 0u, 0u, NULL);
}

/* A named unresolved-boundary stop (psx behaviour with no host model): logs the label like every named stub
 * (aborts in strict-stubs mode) and requests the unresolved-boundary stop. */
void pe_plat_cd_stream_stop(const char *name)
{
    Bootstrap_ReturnVoid(name, "func_8007C564");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}
