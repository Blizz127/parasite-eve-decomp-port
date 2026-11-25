/* One libgpu reset routine (guest 0x80077144, "text_2700"), written clean-room
 * from black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * cdgen9, checked by pc_port/tools/pe_spu_caller_check.c), with no SDK source.
 * Behaviour is exactly the guest loads, register stores, RAM stores and callee
 * calls seen in the vectors.  Only the low three bits of the mode argument
 * select the path. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

/* Guest RAM words written (always zero / the saved mask) on every path. */
#define ADDR_STATE_A        0x80095878u  /* word: read once per call, always zero in the vectors; zeroed */
#define ADDR_SAVED_MASK     0x80095884u  /* word: receives the interrupt mask returned by the first call */
#define ADDR_STATE_B        0x80095874u  /* word: zeroed */

/* Pointer words in guest RAM that hold hardware register addresses. */
#define ADDR_PTR_REG_A      0x80095860u  /* -> register that receives REG_A_VALUE */
#define ADDR_PTR_REG_B      0x80095870u  /* -> register read, then written back with REG_B_SET_BIT or-ed in */
#define ADDR_PTR_REG_C      0x80095854u  /* -> register that receives the command words below */

#define REG_A_VALUE         0x00000401u
#define REG_B_SET_BIT       0x00000800u
#define REG_C_CLEAR         0x00000000u  /* stored on the reset-with-clear paths (modes 0 and 5) */
#define REG_C_CMD_FIRST     0x02000000u  /* stored first on the short paths (modes 1 and 3) ... */
#define REG_C_CMD_SECOND    0x01000000u  /* ... then this one, through a second load of the pointer word */

#define BUF_CLEAR_1_ADDR    0x800A3348u  /* two RAM buffers filled with zero bytes on modes 0 and 5 */
#define BUF_CLEAR_1_SIZE    0x100
#define BUF_CLEAR_2_ADDR    0x800BD030u
#define BUF_CLEAR_2_SIZE    0x1800

#define MODE_SELECT_MASK    7u           /* only these bits of the argument choose the path */
#define MODE_FULL           0u           /* clear buffers, then run the text_2B04 reset; returns its result */
#define MODE_FLAG_SHORT_1   1u           /* register commands only, returns 0 */
#define MODE_FLAG_SHORT_3   3u           /* same as mode 1 */
#define MODE_CLEAR_ONLY     5u           /* clear buffers, no text_2B04 call, returns 0 */
/* Every other value (2, 4, 6, 7) only saves and restores the interrupt mask
 * and returns 0. */

/* Guest callees declared in pe_sdk.h:
 *   func_80073E10(mask)           interrupt-mask setter, returns the previous mask
 *   func_80077A28(dst, byte, n)   byte fill
 *   func_80077548(mode)           text_2B04 */

/* 0x80077144: set the interrupt mask to 0 (keeping the previous mask), zero
 * the three state words (the previous mask goes into the saved-mask word),
 * perform the mode's register and buffer work, then restore the interrupt mask
 * from the saved-mask word.  Returns the text_2B04 result in full mode, else 0. */
int pe_plat_gpu_2700(int mode)
{
    uint32_t sel = (uint32_t)mode & MODE_SELECT_MASK;
    uint32_t reg_a, reg_b, reg_c, v;
    uint16_t previous;
    int result = 0;

    previous = func_80073E10(0);
    PE_StoreU32(ADDR_STATE_A, 0);
    PE_StoreU32(ADDR_SAVED_MASK, previous);
    PE_StoreU32(ADDR_STATE_B, 0);
    (void)PE_LoadU32(ADDR_STATE_A);

    if (sel == MODE_FULL || sel == MODE_FLAG_SHORT_1 || sel == MODE_FLAG_SHORT_3 ||
        sel == MODE_CLEAR_ONLY) {
        reg_a = PE_LoadU32(ADDR_PTR_REG_A);
        PE_StoreU32(reg_a, REG_A_VALUE);
        reg_b = PE_LoadU32(ADDR_PTR_REG_B);
        v = PE_LoadU32(reg_b);
        PE_StoreU32(reg_b, v | REG_B_SET_BIT);

        if (sel == MODE_FLAG_SHORT_1 || sel == MODE_FLAG_SHORT_3) {
            reg_c = PE_LoadU32(ADDR_PTR_REG_C);
            PE_StoreU32(reg_c, REG_C_CMD_FIRST);
            reg_c = PE_LoadU32(ADDR_PTR_REG_C);
            PE_StoreU32(reg_c, REG_C_CMD_SECOND);
        } else {
            reg_c = PE_LoadU32(ADDR_PTR_REG_C);
            PE_StoreU32(reg_c, REG_C_CLEAR);
            func_80077A28(BUF_CLEAR_1_ADDR, 0, BUF_CLEAR_1_SIZE);
            func_80077A28(BUF_CLEAR_2_ADDR, 0, BUF_CLEAR_2_SIZE);
        }
    }

    (void)func_80073E10((uint16_t)PE_LoadU32(ADDR_SAVED_MASK));

    if (sel == MODE_FULL)
        result = func_80077548(mode);
    return result;
}
