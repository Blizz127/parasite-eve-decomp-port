/* libcd data sync (retail 0x8007BDDC, plus its two forwarding ABIs 0x8007A910 and 0x80080B24; all three are
 * bound to this one entry, pe_plat_cd_data_sync, in pe_plat/cd.h): wait for the CD DMA channel 3 transfer to
 * finish, with a 960-retrace time-out.
 *
 * PROVENANCE.  Written clean-room from the black-box vector sets cdgen52 (sync.txt for 0x8007BDDC, fwd_A910.txt
 * and fwd_0B24.txt for the forwarding entries; 600 cases each; batch cdgen52-61,
 * docs/evidence/native-cdx-20261126/cdgen52/REPORT.md), the checker pc_port/tools/pe_cdx_cdgen52_check.c on
 * pe_cdx_common_canary.h, the public headers (pe_plat/cd.h, pe_plat/cd_regs.h, pe_plat/framebuffer_api.h,
 * pe_guestcode.h, pe_bios_string.h, pe_guest_ram.h, bootstrap/game_port.h), the suite tests that exercise the
 * entry (tests/test_native.c CD_data_sync_*), the sibling clean-room providers pe_plat_cd_ready.c and
 * pe_plat2_cdhost_poll.c (for the time-out diagnostics and the host wait contract only) and psx-spx (DMA
 * channel 3 control register 1F8010B8h: bit 24 = transfer busy).  NOT read: the previous provider file,
 * src/, asm/, vendor/, game/decomp_hand/, game/boot bodies, pe_stream.c, *_retail.c, oracle sources,
 * retail_*_cases.h, and pe_lib*.c with ONE disclosed exposure: a grep printed two lines of pe_libcd.c (line 22,
 * a comment, and line 409, a stop-guarded call of the CD flush in another function) before this file was
 * written; the stop exit below (skip the flush) also follows the sibling providers, see the report.  The behaviour below is what the vectors show; no claim is made about the
 * original's source.  Derivation and the unsampled arms: docs/evidence/native-cdx-20261126/cdrw-a/AUTHOR-REPORT.md.
 *
 * Guest words (little-endian):
 *   0x800A3478 deadline, 0x800A347C pass counter, 0x800A3480 owner-name pointer (this wait's own state block)
 *   0x8009B2C0 pointer to the DMA3 channel control register (1F8010B8h)
 *   0x8009B294 / 0x8009B295 CD sync / ready status bytes, 0x8009AFD5 last command byte,
 *   0x8009AFDC + 4*cmd command-name table, 0x8009B05C + 4*status interrupt-name table (no bound check)
 *   0x80011B18 time-out headline, 0x80011B28 time-out format, 0x80011C20 owner name (constants)
 *
 * Behaviour:
 *   entry      t0 = VSync(-1); deadline word = t0 + 0x3C0; counter word = 0; name word = 0x80011C20.
 *   each pass  now = VSync(-1); time-out when (int)deadline word < (int)now (signed; equality does not time out);
 *              counter word += 1; ctl = word at the address held in 0x8009B2C0 (a register read);
 *              bit 24 clear -> return 0;  busy and mode != 0 -> return 1;  busy and mode == 0 -> next pass.
 *   time-out   puts(0x80011B18); printf(0x80011B28, name word, cmdname[last command], inttab[sync byte],
 *              inttab[ready byte]); the CD flush 0x8007B9EC (no arguments, result unused); return -1.
 * Not sampled by the vectors: any bound on the pass counter (the retail counter word is cleared on entry and the
 * oracle's instruction budget ends a case first).  The port's stuck-wait budget below is a host decision.
 *
 * Host contract (hardware wait): the device model only moves when the host clock is ticked, and the original asks
 * the retrace clock once per pass, so HostFB_VSync(-1) is that per-pass tick (it advances the CD device model by one
 * poll quantum).  Every pass checks PE_Port_ShouldStop() (a stop returns -1 without diagnostics or flush) and
 * spends one unit of a stuck-wait budget of 0x3C0000 passes (the figure the retail counter word is bounded by);
 * an exhausted budget requests an unresolved-boundary stop with a stderr diagnostic and returns -1. */
#include <stdint.h>
#include <stdio.h>

#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_bios_string.h"
#include "pe_plat/cd.h"
#include "pe_plat/cd_regs.h"
#include "pe_plat/framebuffer_api.h"
#include "game_port.h"

#define W_DEADLINE     0x800A3478u
#define W_COUNTER      0x800A347Cu
#define W_NAME         0x800A3480u
#define NAME_VALUE     0x80011C20u
#define P_DMA_CTL      0x8009B2C0u
#define B_SYNC_STATUS  0x8009B294u
#define B_READY_STATUS 0x8009B295u
#define B_LAST_COMMAND 0x8009AFD5u
#define T_COMMAND_NAME 0x8009AFDCu
#define T_INTR_NAME    0x8009B05Cu
#define STR_HEADLINE   0x80011B18u
#define STR_FORMAT     0x80011B28u
#define FN_PUTS        0x80073C5Cu
#define FN_CD_FLUSH    0x8007B9ECu
#define DEADLINE_ADD   0x3C0u
#define DMA_BUSY       0x01000000u
#define STUCK_BUDGET   0x3C0000u

static int32_t report_timeout(void)
{
    uint32_t name, cmd_name, sync_name, ready_name, sync, ready;
    uint32_t cmd;

    PE_GuestCall("CD data sync time-out: headline", FN_PUTS, 1u, STR_HEADLINE, 0u, 0u, 0u);
    sync = PE_LoadU8(B_SYNC_STATUS);
    ready = PE_LoadU8(B_READY_STATUS);
    cmd = PE_LoadU8(B_LAST_COMMAND);
    name = PE_LoadU32(W_NAME);
    cmd_name = PE_LoadU32(T_COMMAND_NAME + 4u * cmd);
    sync_name = PE_LoadU32(T_INTR_NAME + 4u * sync);
    ready_name = PE_LoadU32(T_INTR_NAME + 4u * ready);
    PE_Printf_Guest(STR_FORMAT, name, cmd_name, sync_name, ready_name, 0u, 0u, 0u);
    PE_GuestCall("CD data sync time-out: CD flush", FN_CD_FLUSH, 0u, 0u, 0u, 0u, 0u);
    return -1;
}

int pe_plat_cd_data_sync(int mode)
{
    uint32_t passes = 0u;
    uint32_t t0 = HostFB_VSync(-1);

    if (PE_Port_ShouldStop())
        return -1;
    PE_StoreU32(W_DEADLINE, t0 + DEADLINE_ADD);
    PE_StoreU32(W_COUNTER, 0u);
    PE_StoreU32(W_NAME, NAME_VALUE);

    for (;;) {
        uint32_t now = HostFB_VSync(-1);
        uint32_t ctl;

        if (PE_Port_ShouldStop())
            return -1;
        if ((int32_t)PE_LoadU32(W_DEADLINE) < (int32_t)now)
            return report_timeout();
        PE_StoreU32(W_COUNTER, PE_LoadU32(W_COUNTER) + 1u);
        ctl = PE_CdLoadU32(PE_LoadU32(P_DMA_CTL));
        if (!(ctl & DMA_BUSY))
            return 0;
        if (mode != 0)
            return 1;
        if (++passes >= STUCK_BUDGET) {
            fprintf(stderr, "[CD data sync] DMA3 still busy after %u passes; stuck wait\n", (unsigned)passes);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return -1;
        }
    }
}
