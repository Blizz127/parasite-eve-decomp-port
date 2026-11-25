/* ResetGraph (guest 0x80074A44), written clean-room from black-box vectors of
 * the original code (pc_port/tools/pe_gpu_rows_vectors.py `resetgraph`,
 * checked by pe_gpu_rows_check.c set E) and psx-spx.  No SDK source, matched
 * C or disassembly was read; see docs/evidence/native-resetgraph-20261125/.
 *
 * Contract (from vectors).  Modes whose low three bits are 0, 3 or 5 reset the
 * library, in this order:
 *   1. (not for low bits 5) print the diagnostic format 0x800117E0 with the
 *      arguments 0x80095704 (the shipped jump table) and 0x8009574C (the
 *      library state block) through the BIOS printf stub;
 *   2. clear the 0x80 state bytes at 0x8009574C;
 *   3. reset the libetc callbacks (ResetCallback, 0x80073C94);
 *   4. hand the low 24 bits of the jump-table pointer word 0x80095744 to the
 *      GPU command-word writer 0x80077A54 (GP0 opcode 00h, a no-op word;
 *      host side: pe_plat_gpu_cw in pe_plat_gpu_host.c);
 *   5. run the GPU control function 0x80077144 with the mode; its low byte
 *      becomes the video-mode byte 0x8009574C and the function's result;
 *   6. byte 0x8009574D := 1; the display limits (halfwords 0x80095750,
 *      0x80095752) come from the tables at 0x800957CC / 0x800957D8, entry
 *      4 * video-mode byte;
 *   7. fill the two caches 0x8009575C (0x5C bytes) and 0x800957B8 (0x14 bytes,
 *      the PutDispEnv cache) with 0xFF so the next environments are not
 *      suppressed; return the video-mode byte (re-read).
 * Every other mode is ResetGraph's light path: the debug trace, then the jump
 * table's slot 0x34 called with 1 (the independently verified light provider). */
#include <stdint.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"
#include "pe_bios_string.h"
#include "pe_plat/libc.h"
#include "pe_plat/irq_callback.h"
#include "pe_plat/gpu_primitive.h"
#include "pe_plat/gpu_resetgraph.h"
#include "pe_plat/gpu_host.h"

int pe_plat_resetgraph_light_host(int mode);

#define W_STATE    0x8009574Cu   /* library state block (0x80 bytes) */
#define B_VMODE    0x8009574Cu
#define B_FLAG     0x8009574Du
#define H_LIMIT_W  0x80095750u
#define H_LIMIT_H  0x80095752u
#define T_LIMIT_W  0x800957CCu
#define T_LIMIT_H  0x800957D8u
#define W_TABLE    0x80095744u
#define A_CACHE1   0x8009575Cu
#define A_CACHE2   0x800957B8u
#define FMT_RESET  0x800117E0u
#define JTB_SHIPPED 0x80095704u

int pe_plat_gpu_resetgraph(int mode)
{
    uint32_t vm, w, h;
    unsigned low = (unsigned)mode & 7u;
    if (low != 0u && low != 3u && low != 5u) return pe_plat_resetgraph_light_host(mode);
    pe_plat_gpu_host_reset_prepare();   /* host-only library-state defaults (not in the vectors) */
    if (low != 5u) {
        uint32_t args[2];
        args[0] = JTB_SHIPPED; args[1] = W_STATE;
        (void)PE_Bios_Printf(FMT_RESET, args, 2);
    }
    func_80077A28(W_STATE, 0, 0x80);
    (void)func_80073C94();
    pe_plat_gpu_cw(PE_LoadU32(W_TABLE) & 0xFFFFFFu);
    vm = (uint32_t)func_80077144(mode) & 0xFFu;
    PE_StoreU8(B_VMODE, (uint8_t)vm);
    vm = PE_LoadU8(B_VMODE);
    PE_StoreU8(B_FLAG, 1);
    w = PE_LoadU16(T_LIMIT_W + 4u * vm);
    vm = PE_LoadU8(B_VMODE);
    PE_StoreU16(H_LIMIT_W, (uint16_t)w);
    h = PE_LoadU16(T_LIMIT_H + 4u * vm);
    PE_StoreU16(H_LIMIT_H, (uint16_t)h);
    func_80077A28(A_CACHE1, 0xFF, 0x5C);
    func_80077A28(A_CACHE2, 0xFF, 0x14);
    return (int)PE_LoadU8(B_VMODE);
}
