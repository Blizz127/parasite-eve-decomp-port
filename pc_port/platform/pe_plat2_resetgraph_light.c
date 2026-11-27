/* ResetGraph light path (guest 0x80074A44, the modes whose low three bits are
 * 1, 2, 4, 6 or 7), written clean-room from the black-box vector set cdgen64
 * of the original code (docs/evidence/native-cdx-20261126/cdgen64/REPORT.md;
 * pc_port/tools/pe_gpu2_vectors.py `cdgen64s` / `cdgen64i`, checked by
 * pc_port/tools/pe_gpu2_cdgen64_check.c).  The acceptance set is cdgen64i, which
 * runs the GPU reset the slot names inline (the sibling provider
 * pe_plat_gpu_gen9.c) and pins its DMA2 / DMA control and GP1 write sequences.
 *
 * Clean-room fence.  Nothing of the original was read: not src/, asm/,
 * vendor/, game/decomp_hand/, the game/boot bodies, pe_lib*.c, any *_retail.c,
 * the oracle scripts under pc_port/tools, the retail case headers or evidence
 * that quotes original code.  The file this one replaces and its history were
 * never opened.  Inputs: the cdgen64 report and checker, the house-style
 * siblings (pe_plat_gpu_drawsync.c, pe_plat_gpu_putdispenv.c,
 * pe_plat_gpu_gen9.c) and the public headers (pe_guestcode.h, gpu_primitive.h).
 *
 * Contract (from the vectors).  The reset modes (low three bits 0, 3, 5) belong
 * to pe_plat_gpu_resetgraph.c.  Here the mode is used for nothing but the log
 * argument.  If the debug-level byte 0x8009574E is 2 or more, the log vector
 * held in the word 0x80095748 is called with (format 0x80011800, mode); when it
 * is the BIOS printf stub 0x80071A74 the host print is used.  Then the function
 * table pointer 0x80095744 is loaded, the word 0x34 bytes into the table (slot
 * 13) is loaded, that function is called with the argument 1 and its result is
 * the result.  The shipped slot is the GPU reset 0x80077144, which runs
 * natively; any other target is called through the guest-hook boundary.  The
 * original follows the table pointer and the slot without checking them, and
 * every register pointer word the reset reads is read by the reset itself.
 * No register pointer word is read or validated here.
 *
 * Host contract (not in the vectors; documented, stop/escape only):
 *   - a table pointer that cannot hold slot 13 (not word-aligned, or not guest
 *     RAM: library never set up, or the canary RAM of a unit test; retail would
 *     fault) is a named refusal and the call returns 0 without touching state;
 *   - a stop requested while the log vector or the slot target ran ends the
 *     call with 0 (the usual epoch check) instead of continuing. */
#include <stdint.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "game_port.h"
#include "pe_bios_string.h"
#include "pe_plat/gpu_primitive.h"
#include "pe_plat/gpu_resetgraph_light.h"

#define B_DEBUG     0x8009574Eu   /* debug-level byte */
#define W_LOG       0x80095748u   /* log vector word */
#define W_TABLE     0x80095744u   /* function table pointer word */
#define FMT_LOG     0x80011800u
#define SLOT_RESET  0x34u         /* table offset of slot 13 */
#define BIOS_PRINTF 0x80071A74u
#define GPU_RESET   0x80077144u   /* the shipped slot-13 target (pe_plat_gpu_2700) */

int pe_plat_resetgraph_light_host(int mode)
{
    uint32_t root, target;
    unsigned epoch;
    int result = 0;

    if (PE_LoadU8(B_DEBUG) >= 2u) {
        uint32_t log = PE_LoadU32(W_LOG);
        epoch = PE_Port_StopEpoch();
        if (log == BIOS_PRINTF) {
            uint32_t arg = (uint32_t)mode;
            (void)PE_Bios_Printf(FMT_LOG, &arg, 1);
        } else {
            int ignored;
            (void)PE_GuestHook_Call("ResetGraph_log", "func_80074A44", log, 2, FMT_LOG, (uint32_t)mode, 0, 0, &ignored);
        }
        if (PE_Port_StopEpoch() != epoch) return 0;
    }
    root = PE_LoadU32(W_TABLE);
    if ((root & 3u) != 0u || !PE_RangeIsRam(root + SLOT_RESET, 4)) {
        PE_GuestHook_Refuse("ResetGraph_slot", "func_80074A44", root + SLOT_RESET, 1, 1, 0, 0, 0,
                            "GPU function table pointer is not guest RAM");
        return 0;
    }
    target = PE_LoadU32(root + SLOT_RESET);
    if (target == GPU_RESET) return pe_plat_gpu_2700(1);
    epoch = PE_Port_StopEpoch();
    if (!PE_GuestHook_Call("ResetGraph_slot", "func_80074A44", target, 1, 1, 0, 0, 0, &result) ||
        PE_Port_StopEpoch() != epoch) return 0;
    return result;
}
