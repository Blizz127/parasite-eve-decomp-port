/* DrawSync (guest 0x80074DC0), written clean-room from black-box vectors of
 * the original code (pc_port/tools/pe_gpu_rows_vectors.py `drawsync`, checked
 * by pe_gpu_rows_check.c) plus the guest data layout the vectors expose.  No
 * SDK source, matched C or disassembly was read; see
 * docs/evidence/native-gpu-rows-20261125/.
 *
 * Contract (from vectors): the debug-level byte at 0x8009574E is read; at
 * level 2 or more the trace function held in the word 0x80095748 is called
 * with (format 0x80011884, mode) -- when it is the BIOS printf stub
 * 0x80071A74 the port goes through PE_Bios_Printf.  Then the GPU jump table
 * pointer 0x80095744 is read and the function at table offset 0x3C is called
 * with mode; its result is returned.  The shipped table entry is the queue
 * pump 0x80077294, which is called natively. */
#include <stdint.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "game_port.h"
#include "pe_bios_string.h"
#include "pe_plat/gpu_dma_queue.h"
#include "pe_plat/gpu_drawsync.h"
#include "pe_plat/gpu_regs.h"

#define B_DEBUG    0x8009574Eu
#define W_TRACE    0x80095748u
#define W_TABLE    0x80095744u
#define FMT_TRACE  0x80011884u
#define SLOT_WAIT  0x3Cu
#define BIOS_PRINTF 0x80071A74u

int pe_plat_gpu_drawsync(int32_t mode)
{
    int result = 0;
    unsigned epoch;
    uint32_t root, target;
    if (PE_LoadU8(B_DEBUG) >= 2u) {
        uint32_t trace = PE_LoadU32(W_TRACE);
        epoch = PE_Port_StopEpoch();
        if (trace == BIOS_PRINTF) {
            uint32_t arg = (uint32_t)mode;
            (void)PE_Bios_Printf(FMT_TRACE, &arg, 1);
        } else if (!PE_GuestHook_Call("DrawSync_debug", "func_80074DC0", trace, 2, FMT_TRACE,
                                      (uint32_t)mode, 0, 0, &result)) {
            return 0;
        }
        if (PE_Port_StopEpoch() != epoch) return 0;
    }
    root = PE_LoadU32(W_TABLE);
    if (!pe_gpu_reg_ok(root + SLOT_WAIT, 4)) return pe_gpu_reg_refuse("DrawSync_table", "func_80074DC0", root);
    target = PE_LoadU32(root + SLOT_WAIT);
    /* A zero slot (jump table not set up yet, as in the unit tests) runs the native queue pump too. */
    if (target == 0x80077294u || target == 0u) return pe_plat_gpu_dma_queue_run(mode);
    epoch = PE_Port_StopEpoch();
    if (!PE_GuestHook_Call("DrawSync_slot", "func_80074DC0", target, 1, (uint32_t)mode, 0, 0, 0, &result) ||
        PE_Port_StopEpoch() != epoch) return 0;
    return result;
}
