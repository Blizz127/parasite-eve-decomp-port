/* CD command issue (retail 0x8007B558, bound as func_8007B558 / pe_plat_cd_command_issue_host in
 * pe_plat/cdhost.h): write one command and its parameter bytes to the controller, optionally waiting
 * for the controller to acknowledge it.
 *
 * PROVENANCE.  Written clean-room from the black-box vector sets own/cdgen47/cmd.txt and cmd_silent.txt
 * (batch cdgen47, docs/evidence/native-cdx-20261126/cdgen47/REPORT.md) and, for the wait loop that
 * cmd.txt barely samples, from the poll vectors poll.txt / poll_silent.txt and the acknowledge vectors
 * ack.txt (the wait loop is the poll's loop with its own name string), the checker
 * pc_port/tools/pe_cdx_cdgen47_check.c + pe_cdx_common.h, the public headers (pe_plat/cd.h,
 * pe_plat/cdhost.h, pe_plat/ctl.h, pe_plat/hostfb_forward.h, pe_plat/framebuffer_api.h, pe_guest_ram.h,
 * pe_guestcode.h, pe_sdk.h, pe_bios_string.h, bootstrap/game_port.h) and psx-spx (CD-ROM controller
 * registers 1F801800h..1F801803h).  NOT read: the previous provider files, src/, asm/, vendor/,
 * decomp_hand/, game/boot bodies, pe_lib*.c, pe_stream.c, *_retail.c, oracle sources, retail_*_cases.h,
 * capture logs.  The behaviour below is what the vectors show; no claim is made about the original's
 * source.  Derivation notes with vector line numbers: AUTHOR-REPORT.md
 * (docs/evidence/native-cdx-20261126/cdhost-author/).
 *
 * Guest words (little-endian, read at every use; located from the vectors):
 *   0x8009AFC0  debug level (signed; > 1 prints the command name and the missing-parameter message)
 *   0x8009AFDC + 4*cmd  command-name pointer table;  0x8009B05C + 4*status  interrupt-name pointer table
 *   0x8009B200 + 4*(cmd-1)  parameter count per command (signed word)
 *   0x8009B100 + 4*(cmd-1)  "starts a read" flag per command (nonzero: the ready status is cleared too)
 *   0x8009B27C / B280 / B284  pointers to the controller registers 1F801800h (index/status),
 *                             1F801801h (command), 1F801802h (parameter FIFO)
 *   0x8009B294  sync status byte, 0x8009B295  ready status byte
 *   0x8009AFD5  last command byte, 0x8009AFD0..D3  last Setloc position (4 bytes), 0x8009AFD4  last Setmode byte
 *   0x8009AFB4 / B8  sync / ready callback words, 0x800A3460..67  8-byte result buffer
 *   0x800A3478 / 7C / 80  wait-loop state block (deadline, count, name pointer 0x80011BCC)
 *   0x80011BB4 "%s...\n" and 0x80011BBC "%s: no param\n" debug formats, 0x80011B18/B28 time-out headline/format
 *
 * Behaviour:
 *   1. level = word 0x8009AFC0; level > 1 (signed): printf("%s...\n", command name).
 *   2. count = parameter count for cmd; count != 0 and input == 0: re-read the level, level > 1: printf
 *      ("%s: no param\n", command name); return -2 (nothing written, no calls).
 *   3. poll(0, 0) (func_8007B010): wait for the previous command; its result is ignored (the vectors return 0
 *      whatever the poll returned).
 *   4. cmd 2 (Setloc): copy input[0..3] to 0x8009AFD0..D3;  cmd 0x0E (Setmode): input[0] -> 0x8009AFD4.
 *   5. sync status := 0; when the "starts a read" flag is nonzero the ready status := 0 too.
 *   6. select register index 0 (store 0 through the 0x8009B27C pointer), write count parameter bytes
 *      input[0..count-1] to the 0x8009B284 pointer (count re-read every pass, signed compare), last
 *      command := cmd, write cmd through the 0x8009B280 pointer.
 *   7. mode != 0: return 0.  mode == 0: wait for the acknowledge (below), return 0, or the time-out report
 *      and -1.
 * Wait loop (mode == 0): sampled by one vector only (a time-out on the first pass, #225): t0 = VSync(-1),
 * deadline = t0 + 0x3C0, state block stored, then per pass the poll's loop: VSync(-1), signed deadline
 * test (time-out report: puts(0x80011B18), printf(0x80011B28, name, command name, sync name, ready name),
 * flush func_8007B9EC(), return -1), count stored, controller-state halfword != 0 -> index save, acknowledge
 * loop with the data callbacks, index restore.  NOT sampled (decided from psx-spx and the ack vectors,
 * flagged in the report): the exit condition (sync status != 0 = the command was acknowledged: the
 * acknowledge handler stores 3 / 2 / 5 there), the 8-byte result copy to dst, the return value 0.
 *
 * Host contract (hardware wait): every wait pass ticks the host frame clock through VSync (cd_vsync: HostFB_VSync(-1)
 * advances the frame clock and the CD device model, the value the guest sees is PE_GPU_VSyncQuery()) and additionally
 * HostFB_StreamTick() (the CD device clock quantum of one poll iteration; the poll itself does not call it), checks
 * PE_Port_ShouldStop() and spends one unit of a stuck-wait budget; an exhausted budget requests a stop with a
 * diagnostic and returns -1.  A stop already requested on entry returns -1 at once with no CD traffic. */
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

#define CD_DEBUG_LEVEL    0x8009AFC0u
#define CD_COMMAND_NAMES  0x8009AFDCu
#define CD_INTR_NAMES     0x8009B05Cu
#define CD_PARAM_COUNTS   0x8009B200u
#define CD_READ_FLAGS     0x8009B100u
#define CD_REG_INDEX_PTR  0x8009B27Cu
#define CD_REG_CMD_PTR    0x8009B280u
#define CD_REG_PARAM_PTR  0x8009B284u
#define CD_SYNC_STATUS    0x8009B294u
#define CD_READY_STATUS   0x8009B295u
#define CD_LAST_COMMAND   0x8009AFD5u
#define CD_LAST_POSITION  0x8009AFD0u
#define CD_LAST_MODE      0x8009AFD4u
#define CD_SYNC_CALLBACK  0x8009AFB4u
#define CD_READY_CALLBACK 0x8009AFB8u
#define CD_RESULT_BUFFER  0x800A3460u
#define CD_READY_BUFFER   0x800A3468u   /* 8-byte response copy of slot 1 (ready events: data ready / data end) */
#define WAIT_DEADLINE     0x800A3478u
#define WAIT_COUNT        0x800A347Cu
#define WAIT_NAME         0x800A3480u
#define WAIT_NAME_VALUE   0x80011BCCu
#define FMT_TRACE         0x80011BB4u
#define FMT_NO_PARAM      0x80011BBCu
#define TIMEOUT_HEADLINE  0x80011B18u
#define TIMEOUT_FORMAT    0x80011B28u
#define WAIT_LIMIT_FRAMES 0x3C0u
#define WAIT_STUCK_BUDGET 0x400000u
#define CMD_SETLOC        0x02u
#define CMD_SETMODE       0x0Eu

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
    (void)PE_GuestHook_Call(slot, "pe_plat_cdhost_cmd", target, 2, status, response, 0, 0, &ignored);
}

/* Time-out report shared in shape with the status poll: headline, one formatted line, then the CD flush. */
static void cmd_timeout_report(void)
{
    uint32_t sync = PE_LoadU8(CD_SYNC_STATUS);
    uint32_t ready = PE_LoadU8(CD_READY_STATUS);
    uint32_t ready_name = PE_LoadU32(CD_INTR_NAMES + 4u * ready);
    uint32_t cmd = PE_LoadU8(CD_LAST_COMMAND);
    uint32_t cmd_name = PE_LoadU32(CD_COMMAND_NAMES + 4u * cmd);
    uint32_t sync_name = PE_LoadU32(CD_INTR_NAMES + 4u * sync);

    PE_GuestCall("cd_command_issue: headline", 0x80073C5Cu, 1, TIMEOUT_HEADLINE, 0, 0, 0);
    PE_Printf_Guest(TIMEOUT_FORMAT, WAIT_NAME_VALUE, cmd_name, sync_name, ready_name, 0, 0, 0);
    func_8007B9EC();
}

/* Wait until the controller acknowledged the command just written (sync status nonzero). */
static int cmd_wait_acknowledge(uint32_t dst)
{
    uint32_t clock0, clock, start = cd_vsync(&clock0), deadline = start + WAIT_LIMIT_FRAMES;
    uint32_t count = 0;
    uint32_t budget = WAIT_STUCK_BUDGET;

    PE_StoreU32(WAIT_DEADLINE, deadline);
    PE_StoreU32(WAIT_COUNT, 0);
    PE_StoreU32(WAIT_NAME, WAIT_NAME_VALUE);

    for (;;) {
        HostFB_StreamTick();                 /* host: advance the CD device clock */
        if (PE_Port_ShouldStop())
            return -1;
        if (budget-- == 0) {
            fprintf(stderr, "[cd_command_issue] stuck wait: no acknowledge after %u passes "
                    "(sync status %u, ready status %u); stopping\n", (unsigned)WAIT_STUCK_BUDGET,
                    (unsigned)PE_LoadU8(CD_SYNC_STATUS), (unsigned)PE_LoadU8(CD_READY_STATUS));
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return -1;
        }
        (void)cd_vsync(&clock);
        /* guest-visible time = start value + frames elapsed on the host clock (equal to the GPU counter whenever both run) */
        if ((int32_t)(start + (clock - clock0)) > (int32_t)deadline) {
            cmd_timeout_report();
            return -1;
        }
        count++;
        PE_StoreU32(WAIT_COUNT, count);

        if ((uint16_t)func_80073DE8() != 0) {
            uint32_t reg_ptr = PE_LoadU32(CD_REG_INDEX_PTR);
            uint8_t index_bits = PE_CdLoadU8(reg_ptr);
            uint32_t r;

            while ((r = (uint32_t)func_8007AAB4()) != 0) {
                if (r & 4u) {   /* ready event */
                    uint32_t cb = PE_LoadU32(CD_READY_CALLBACK);
                    if (cb)
                        cd_hook_callback("CD ready callback (0x8009AFB8)", cb, PE_LoadU8(CD_READY_STATUS), CD_READY_BUFFER);
                }
                if (r & 2u) {   /* sync event */
                    uint32_t cb = PE_LoadU32(CD_SYNC_CALLBACK);
                    if (cb)
                        cd_hook_callback("CD sync callback (0x8009AFB4)", cb, PE_LoadU8(CD_SYNC_STATUS), CD_RESULT_BUFFER);
                }
            }
            reg_ptr = PE_LoadU32(CD_REG_INDEX_PTR);
            PE_CdStoreU8(reg_ptr, (uint8_t)(index_bits & 3u));
        }

        if (PE_LoadU8(CD_SYNC_STATUS) != 0) {
            uint32_t i;

            if (dst != 0)
                for (i = 0; i < 8; i++)
                    PE_StoreU8(dst + i, PE_LoadU8(CD_RESULT_BUFFER + i));
            return 0;
        }
    }
}

int pe_plat_cdhost_cmd(uint32_t cmd, uint32_t input, uint32_t dst, uint32_t mode)
{
    uint32_t name_slot = CD_COMMAND_NAMES + 4u * cmd;
    uint32_t table_slot = 4u * (cmd - 1u);
    uint32_t i;

    /* host contract: a stop already requested means no CD traffic and no state is touched (the vectors never request a stop) */
    if (PE_Port_ShouldStop())
        return -1;

    /* 1. trace */
    if ((int32_t)PE_LoadU32(CD_DEBUG_LEVEL) > 1)
        PE_Printf_Guest(FMT_TRACE, PE_LoadU32(name_slot), 0, 0, 0, 0, 0, 0);

    /* 2. parameters required but no buffer given */
    if ((int32_t)PE_LoadU32(CD_PARAM_COUNTS + table_slot) != 0 && input == 0) {
        /* level > 0 (the vectors: level 0 prints nothing, 0x100 prints both lines; the unsampled level 1 prints this line only,
         * as the suite's missing-parameter test expects) */
        if ((int32_t)PE_LoadU32(CD_DEBUG_LEVEL) > 0)
            PE_Printf_Guest(FMT_NO_PARAM, PE_LoadU32(name_slot), 0, 0, 0, 0, 0, 0);
        return -2;
    }

    /* 3. wait for the previous command (result ignored) */
    (void)func_8007B010(0, 0);

    /* 4. remember the last Setloc position / Setmode byte */
    if (cmd == CMD_SETLOC) {
        for (i = 0; i < 4; i++)
            PE_StoreU8(CD_LAST_POSITION + i, PE_LoadU8(input + i));
    } else if (cmd == CMD_SETMODE) {
        PE_StoreU8(CD_LAST_MODE, PE_LoadU8(input));
    }

    /* 5. clear the status bytes */
    {
        uint32_t starts_read = PE_LoadU32(CD_READ_FLAGS + table_slot);
        PE_StoreU8(CD_SYNC_STATUS, 0);
        if (starts_read != 0)
            PE_StoreU8(CD_READY_STATUS, 0);
    }

    /* 6. controller: index 0, parameter bytes, command */
    PE_CdStoreU8(PE_LoadU32(CD_REG_INDEX_PTR), 0);
    for (i = 0; (int32_t)i < (int32_t)PE_LoadU32(CD_PARAM_COUNTS + table_slot); i++)
        PE_CdStoreU8(PE_LoadU32(CD_REG_PARAM_PTR), PE_LoadU8(input + i));
    PE_StoreU8(CD_LAST_COMMAND, (uint8_t)cmd);
    PE_CdStoreU8(PE_LoadU32(CD_REG_CMD_PTR), (uint8_t)cmd);

    /* 7. asynchronous (mode != 0) or wait for the acknowledge */
    if (mode != 0)
        return 0;
    return cmd_wait_acknowledge(dst);
}
