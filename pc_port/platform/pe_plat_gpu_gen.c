/* Three small libgpu callers (guest 0x80077548, 0x80077A00, 0x80076C10),
 * written clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen5, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Each function's
 * behaviour is exactly the guest loads/stores, hardware register accesses,
 * calls and return value seen in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "psx_compat.h"

/* Pointer words holding the GPU register addresses (0x1F801810 / 0x1F801814
 * in the vectors); the registers are reached through them. */
#define ADDR_GPU_DATA_PTR     0x80095850u  /* word: address of the data register */
#define ADDR_GPU_CTRL_PTR     0x80095854u  /* word: address of the control register */

#define GPU_CTRL_COMMAND      0x10000007u  /* value written to the control register */
#define GPU_DATA_COMMAND      0xE1001007u  /* value written to the data register */

#define GPU_DMA_CHANNEL       2u           /* channel argument of 0x80073CF4 */
/* Guest code address observed in the vectors as the handler argument of
 * 0x80073CF4 from 0x80077A00. */
#define GPU_DMA_HANDLER       0x80076EE4u

/* 0x80077548: with the register addresses taken from the two pointer words
 * (each reloaded before use), read the data register, write
 * 0x10000007 to the control register and read it back, write 0xE1001007 to
 * the data register and read it back.  Returns 0.  a0 has no effect in the
 * vectors. */
int pe_plat_gpu_2b04(int a0)
{
    uint32_t data = PE_LoadU32(ADDR_GPU_DATA_PTR);
    uint32_t ctrl;

    (void)a0;
    (void)PE_LoadU32(data);
    ctrl = PE_LoadU32(ADDR_GPU_CTRL_PTR);
    PE_StoreU32(ctrl, GPU_CTRL_COMMAND);
    (void)PE_LoadU32(ctrl);
    data = PE_LoadU32(ADDR_GPU_DATA_PTR);
    PE_StoreU32(data, GPU_DATA_COMMAND);
    (void)PE_LoadU32(data);
    return 0;
}

/* 0x80077A00: register the handler at 0x80076EE4 on channel 2 through
 * 0x80073CF4 (result unused). */
void pe_plat_gpu_2fbc(void)
{
    (void)func_80073CF4(GPU_DMA_CHANNEL, GPU_DMA_HANDLER);
}

/* 0x80076C10: forward to 0x80076C34(a0, a1, 0, a2) (result unused). */
void pe_plat_gpu_21cc(int a0, int a1, int a2)
{
    (void)func_80076C34((pe_addr_t)a0, (pe_addr_t)a1, 0, (uint32_t)a2);
}
