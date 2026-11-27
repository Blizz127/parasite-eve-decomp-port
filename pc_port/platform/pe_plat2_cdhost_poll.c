/* CD status poll (retail 0x8007B010, bound as func_8007B010 / pe_plat_cd_status_poll_host in
 * pe_plat/cdhost.h): wait for the command in flight to report Complete or DiskError, driving the
 * controller interrupt acknowledge by hand while it waits, and copy the 8-byte result.
 *
 * PROVENANCE.  Written clean-room from the black-box vector sets own/cdgen47/poll.txt and
 * poll_silent.txt (batch cdgen47, docs/evidence/native-cdx-20261126/cdgen47/REPORT.md), the checker
 * pc_port/tools/pe_cdx_cdgen47_check.c + pe_cdx_common.h, the public headers (pe_plat/cd.h,
 * pe_plat/cdhost.h, pe_plat/ctl.h, pe_plat/hostfb_forward.h, pe_plat/framebuffer_api.h, pe_guest_ram.h,
 * pe_guestcode.h, pe_sdk.h, pe_bios_string.h, bootstrap/game_port.h) and psx-spx (CD-ROM controller:
 * index/status register 1F801800h).  Read: nothing else.  NOT read: the previous provider files, src/,
 * asm/, vendor/, decomp_hand/, game/boot bodies, pe_lib*.c, pe_stream.c, *_retail.c, oracle sources,
 * retail_*_cases.h, capture logs.  The behaviour below is what the vectors show; no claim is made
 * about the original's source.  Derivation notes with the vector line numbers: AUTHOR-REPORT.md
 * (docs/evidence/native-cdx-20261126/cdhost-author/).
 *
 * Guest words (little-endian, all read at every use; every address located from the vectors):
 *   0x8009B27C  pointer to the CD index/status register (1F801800h; read as a table word, so any value)
 *   0x8009B294  sync status byte (2 = Complete, 5 = DiskError, 3 = Acknowledge, 0 = none); written 2 when consumed
 *   0x8009B295  ready status byte (1 DataReady, 4 DataEnd, 5 DiskError)
 *   0x8009AFD5  last command byte; 0x8009AFDC + 4*cmd  command-name pointer table
 *   0x8009B05C + 4*status  interrupt-name pointer table (indexed by 0x8009B294 and 0x8009B295, no bound check)
 *   0x8009AFB4  sync callback word, 0x8009AFB8  ready callback word (0 = none)
 *   0x800A3460..67  8-byte command result buffer
 *   0x800A3478 deadline (VSync + 0x3C0), 0x800A347C iteration count, 0x800A3480 owner-name pointer
 *   (the three words are the poll's own state block; the vectors show their final values)
 *   0x80011B18 time-out headline string, 0x80011B28 time-out format, 0x80011BA0 owner name (constants)
 *
 * Behaviour (vector evidence in AUTHOR-REPORT.md):
 *   entry      t0 = VSync(-1); deadline = t0 + 0x3C0; count = 0; name = 0x80011BA0.
 *   each pass  v = VSync(-1); time-out when (int)v > (int)deadline (signed: the t0 = 0x7FFFFFF0 cases);
 *              count++ (stored); when the controller-state halfword (func_80073DE8) is nonzero: remember the
 *              index bits of the status register, call the acknowledge until it returns 0 (each nonzero result
 *              r: bit 4 -> ready callback word, bit 2 -> sync callback word, called as cb(status, response copy: slot 0 0x800A3460 for sync events, slot 1 0x800A3468 for ready events)),
 *              then restore the index bits (value & 3) through the same table pointer;
 *              status = byte 0x8009B294: 2 or 5 -> store 2, copy the result to dst when dst != 0, return status;
 *              otherwise mode != 0 -> return 0 (non-blocking), else next pass.
 *   time-out   puts(0x80011B18); printf("%s:(%s) Sync=%s, Ready=%s\n"-style format 0x80011B28, name, command
 *              name, sync name, ready name); CD flush func_8007B9EC(); return -1.
 * Not sampled by any vector (decided from the ack vectors, flagged in the report): ready callback arguments
 * (taken as the ready status byte), acknowledge results 3/5/7, a count limit.
 *
 * Host contract (hardware wait): every pass ticks the host frame clock through VSync (cd_vsync: HostFB_VSync(-1) lets
 * the frame clock and the CD device model advance, the value the guest sees is PE_GPU_VSyncQuery()), checks
 * PE_Port_ShouldStop() and spends one unit of a stuck-wait budget; an exhausted budget requests a stop with a diagnostic
 * and returns -1.  A stop already requested on entry returns at once without touching any state (0 for a non-blocking
 * poll, -1 for a blocking one). */
#include <stdint.h>
#include <stdio.h>

#include "pe_guest_ram.h"
#include "pe_plat/cd_regs.h"
#include "pe_plat/cdhost.h"
#include "pe_plat/ctl.h"
#include "pe_plat/framebuffer_api.h"
#include "pe_plat/gpu_hw.h"
#include "pe_guestcode.h"
#include "pe_bios_string.h"
#include "pe_sdk.h"
#include "game_port.h"

#define CD_REGPTR_WORD    0x8009B27Cu
#define CD_SYNC_STATUS    0x8009B294u
#define CD_READY_STATUS   0x8009B295u
#define CD_LAST_COMMAND   0x8009AFD5u
#define CD_COMMAND_NAMES  0x8009AFDCu
#define CD_INTR_NAMES     0x8009B05Cu
#define CD_SYNC_CALLBACK  0x8009AFB4u
#define CD_READY_CALLBACK 0x8009AFB8u
#define CD_RESULT_BUFFER  0x800A3460u
#define CD_READY_BUFFER   0x800A3468u   /* 8-byte response copy of slot 1 (ready events: data ready / data end) */
#define POLL_DEADLINE     0x800A3478u
#define POLL_COUNT        0x800A347Cu
#define POLL_NAME         0x800A3480u
#define POLL_NAME_VALUE   0x80011BA0u
#define POLL_HEADLINE     0x80011B18u
#define POLL_FORMAT       0x80011B28u
#define POLL_LIMIT_FRAMES 0x3C0u
#define STATUS_COMPLETE   2u
#define STATUS_DISK_ERROR 5u
/* Host stuck-wait budget (passes).  The guest deadline is VSync based; this only catches a host whose frame
 * counter never advances. */
#define POLL_STUCK_BUDGET 0x400000u

/* Retail VSync(-1).  The host frame clock is ticked first: HostFB_VSync(-1) lets the CD device model and the frame clock advance
 * while the guest waits (the per-poll clock tick of the host wait contract) and returns the host clock value.  The guest-visible
 * counter is the GPU model's, PE_GPU_VSyncQuery(); with the CD device attached both are the same free-running VBlank count, without
 * a device (tests, early boot) only the host clock moves between calls, so the time-out compare uses the host value while the
 * deadline is taken from the guest-visible counter (the suite checks the stored deadline against PE_GPU_VSyncQuery()). */
static uint32_t cd_vsync(uint32_t *host_clock)
{
    uint32_t host = HostFB_VSync(-1);
    if (host_clock) *host_clock = host;
    return PE_GPU_VSyncQuery();
}

/* The retail `jalr` through a callback word: cb(status, response).  A hook site (slot word written by the SDK / game); the
 * target runs through the guest-hook seam, never through a host dispatcher, and a target that cannot run there is refused loudly. */
static void cd_hook_callback(const char *slot, uint32_t target, uint32_t status, uint32_t response)
{
    int ignored = 0;
    (void)PE_GuestHook_Call(slot, "pe_plat_cdhost_poll", target, 2, status, response, 0, 0, &ignored);
}

/* Time-out report: headline, one formatted line, then the CD flush. */
static void poll_timeout_report(void)
{
    uint32_t sync = PE_LoadU8(CD_SYNC_STATUS);
    uint32_t ready = PE_LoadU8(CD_READY_STATUS);
    uint32_t ready_name = PE_LoadU32(CD_INTR_NAMES + 4u * ready);
    uint32_t cmd = PE_LoadU8(CD_LAST_COMMAND);
    uint32_t cmd_name = PE_LoadU32(CD_COMMAND_NAMES + 4u * cmd);
    uint32_t sync_name = PE_LoadU32(CD_INTR_NAMES + 4u * sync);

    PE_GuestCall("cd_status_poll: headline", 0x80073C5Cu, 1, POLL_HEADLINE, 0, 0, 0);
    PE_Printf_Guest(POLL_FORMAT, POLL_NAME_VALUE, cmd_name, sync_name, ready_name, 0, 0, 0);
    func_8007B9EC();
}

int pe_plat_cdhost_poll(uint32_t mode, uint32_t dst)
{
    uint32_t clock0, clock, start = cd_vsync(&clock0), deadline = start + POLL_LIMIT_FRAMES;
    uint32_t count = 0;
    uint32_t budget = POLL_STUCK_BUDGET;

    /* host contract: a stop already requested means no poll and no state change (the vectors never request a stop);
     * a non-blocking poll reports "nothing yet", a blocking one the time-out value */
    if (PE_Port_ShouldStop())
        return mode != 0 ? 0 : -1;

    PE_StoreU32(POLL_DEADLINE, deadline);
    PE_StoreU32(POLL_COUNT, 0);
    PE_StoreU32(POLL_NAME, POLL_NAME_VALUE);

    for (;;) {
        uint32_t status;

        /* host wait contract: leave on a host stop or an exhausted budget (the per-pass clock tick is the VSync in cd_vsync) */
        if (PE_Port_ShouldStop())
            return -1;
        if (budget-- == 0) {
            fprintf(stderr, "[cd_status_poll] stuck wait: %u passes without Complete/DiskError "
                    "(sync status %u, ready status %u); stopping\n", (unsigned)POLL_STUCK_BUDGET,
                    (unsigned)PE_LoadU8(CD_SYNC_STATUS), (unsigned)PE_LoadU8(CD_READY_STATUS));
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return -1;
        }

        /* deadline: signed compare of the frame counter */
        (void)cd_vsync(&clock);
        /* guest-visible time = start value + frames elapsed on the host clock (equal to the GPU counter whenever both run) */
        if ((int32_t)(start + (clock - clock0)) > (int32_t)deadline) {
            poll_timeout_report();
            return -1;
        }
        count++;
        PE_StoreU32(POLL_COUNT, count);

        if ((uint16_t)func_80073DE8() != 0) {
            uint32_t reg_ptr = PE_LoadU32(CD_REGPTR_WORD);
            uint8_t index_bits = PE_CdLoadU8(reg_ptr);
            uint32_t r;

            while ((r = (uint32_t)func_8007AAB4()) != 0) {
                if (r & 4u) {   /* ready event: ready callback word */
                    uint32_t cb = PE_LoadU32(CD_READY_CALLBACK);
                    if (cb)
                        cd_hook_callback("CD ready callback (0x8009AFB8)", cb, PE_LoadU8(CD_READY_STATUS), CD_READY_BUFFER);
                }
                if (r & 2u) {   /* sync event: sync callback word */
                    uint32_t cb = PE_LoadU32(CD_SYNC_CALLBACK);
                    if (cb)
                        cd_hook_callback("CD sync callback (0x8009AFB4)", cb, PE_LoadU8(CD_SYNC_STATUS), CD_RESULT_BUFFER);
                }
            }
            /* restore the register index selected before the acknowledge */
            reg_ptr = PE_LoadU32(CD_REGPTR_WORD);
            PE_CdStoreU8(reg_ptr, (uint8_t)(index_bits & 3u));
        }

        status = PE_LoadU8(CD_SYNC_STATUS);
        if (status == STATUS_COMPLETE || status == STATUS_DISK_ERROR) {
            uint32_t i;

            PE_StoreU8(CD_SYNC_STATUS, (uint8_t)STATUS_COMPLETE);
            if (dst != 0)
                for (i = 0; i < 8; i++)
                    PE_StoreU8(dst + i, PE_LoadU8(CD_RESULT_BUFFER + i));
            return (int)status;
        }
        if (mode != 0)
            return 0;
    }
}
