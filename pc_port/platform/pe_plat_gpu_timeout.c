/* GPU wait timeout check (guest 0x80077404), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_gpu_rows_vectors.py
 * `timeout`, checked by pe_gpu_rows_check.c) plus the hardware documentation
 * (psx-spx GPU: GP1(01h) reset command buffer, GP1(02h) acknowledge GPU
 * interrupt; DMA: CHCR / DPCR; interrupt control: I_MASK).  No SDK source,
 * matched C or disassembly of the function was read; see
 * docs/evidence/native-gpu-rows-20261125/.
 *
 * Contract (all derived from vectors):
 *  - VSync(-1) is called first.  If its result (signed) is greater than the
 *    limit word at 0x80095888, or, otherwise, the poll counter at
 *    0x8009588C (read, then stored +1) was above 0xF0000 (signed), the wait
 *    has timed out; else the function returns 0.
 *  - On timeout it prints the GPU state through the BIOS printf (format at
 *    0x80011988; arguments: queue fill (head - tail) & 0x3F from the words
 *    0x80095874 / 0x80095878, GPUSTAT, DMA2 CHCR, DMA2 MADR), then resets the
 *    GPU path: I_MASK saved to 0x80095884 and cleared, queue words cleared,
 *    CHCR2 = 0x401, DPCR |= 0x800, GP1(02h), GP1(01h), I_MASK restored; and
 *    returns -1.
 * The register addresses are read through the guest pointer words
 * (0x80095854 GPU, 0x80095858 MADR, 0x80095860 CHCR, 0x80095870 DPCR,
 * 0x80095674 I_MASK). */
#include "pe_guest_ram.h"
#include "pe_bios_string.h"
#include "pe_plat/gpu_timeout.h"
#include "pe_plat/gpu_regs.h"
#include "psx_compat.h"

#define W_LIMIT   0x80095888u
#define W_COUNT   0x8009588Cu
#define W_SAVED   0x80095884u
#define W_HEAD    0x80095874u
#define W_TAIL    0x80095878u
#define P_GPU     0x80095854u
#define P_MADR    0x80095858u
#define P_CHCR    0x80095860u
#define P_DPCR    0x80095870u
#define P_IMASK   0x80095674u
#define FMT_TIMEOUT 0x80011988u

int32_t pe_plat_gpu_timeout(void)
{
    int32_t now = (int32_t)func_80073A44(-1);
    int32_t limit = (int32_t)PE_LoadU32(W_LIMIT);
    if (now <= limit) {
        int32_t count = (int32_t)PE_LoadU32(W_COUNT);
        PE_StoreU32(W_COUNT, (uint32_t)count + 1u);
        if (count <= 0xF0000) return 0;
    }
    uint32_t gpu = PE_LoadU32(P_GPU);
    if (!pe_gpu_reg_ok(gpu, 4)) return pe_gpu_reg_refuse("gpu_timeout", "func_80077404", gpu);
    uint32_t stat = PE_LoadU32(gpu);
    uint32_t head = PE_LoadU32(W_HEAD);
    uint32_t madr_p = PE_LoadU32(P_MADR);
    uint32_t tail = PE_LoadU32(W_TAIL);
    if (!pe_gpu_reg_ok(madr_p, 4)) return pe_gpu_reg_refuse("gpu_timeout", "func_80077404", madr_p);
    uint32_t madr = PE_LoadU32(madr_p);
    uint32_t chcr_p = PE_LoadU32(P_CHCR);
    if (!pe_gpu_reg_ok(chcr_p, 4)) return pe_gpu_reg_refuse("gpu_timeout", "func_80077404", chcr_p);
    stat = PE_LoadU32(gpu);                 /* status is read again before CHCR */
    uint32_t chcr = PE_LoadU32(chcr_p);
    uint32_t args[4] = {(head - tail) & 0x3Fu, stat, chcr, madr};
    (void)PE_Bios_Printf(FMT_TIMEOUT, args, 4);

    uint32_t im_p = PE_LoadU32(P_IMASK);
    if (!pe_gpu_reg_ok(im_p, 2)) return pe_gpu_reg_refuse("gpu_timeout", "func_80077404", im_p);
    uint16_t mask = PE_LoadU16(im_p);
    PE_StoreU16(im_p, 0);
    PE_StoreU32(W_TAIL, 0);
    uint32_t zero = PE_LoadU32(W_TAIL);
    PE_StoreU32(W_SAVED, mask);
    PE_StoreU32(W_HEAD, zero);
    chcr_p = PE_LoadU32(P_CHCR);
    PE_StoreU32(chcr_p, 0x401u);
    uint32_t dpcr_p = PE_LoadU32(P_DPCR);
    if (!pe_gpu_reg_ok(dpcr_p, 4)) return pe_gpu_reg_refuse("gpu_timeout", "func_80077404", dpcr_p);
    uint32_t dpcr = PE_LoadU32(dpcr_p);
    PE_StoreU32(dpcr_p, dpcr | 0x800u);
    gpu = PE_LoadU32(P_GPU);
    PE_StoreU32(gpu, 0x02000000u);
    gpu = PE_LoadU32(P_GPU);
    PE_StoreU32(gpu, 0x01000000u);
    uint32_t saved = PE_LoadU32(W_SAVED);
    im_p = PE_LoadU32(P_IMASK);
    (void)PE_LoadU16(im_p);
    PE_StoreU16(im_p, (uint16_t)saved);
    return -1;
}
