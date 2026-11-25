/* libds caller at guest 0x80081268, written clean-room from black-box vectors
 * of the original code (pc_port/tools/pe_cd_leaf_vectors.py cdgen3, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  The behaviour is
 * exactly the guest loads/stores, calls and return value seen in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

/* 0x8007FCBC is not declared by pe_sdk.h; declared here as the checker's
 * stub expects (two int arguments, int result). */

/* Guest data words read or written. */
#define ADDR_DS_ACTIVE     0x8009B6CCu  /* 1 = active; cleared on every exit */
#define ADDR_DS_FLAGS      0x8009B6B8u  /* bit 0 set -> restore the callback */
#define ADDR_DS_CALLBACK   0x8009B6C0u  /* passed to 0x800824F0 when restored */

#define DS_ACTIVE          1u    /* ADDR_DS_ACTIVE value that runs the body */
#define DS_FLAG_CALLBACK   1u    /* bit of ADDR_DS_FLAGS that selects 0x800824F0 */
#define DS_CHECK_ARG0      1     /* a0 of 0x8007FCBC */
#define DS_CHECK_ARG1      0     /* a1 of 0x8007FCBC */
#define DS_END_COMMAND     9u    /* command passed to 0x8007EE84 */
#define DS_END_PARAM       0u    /* param passed to 0x8007EE84 */
#define DS_END_EXTRA       0u    /* extra passed to 0x8007EE84 */
#define DS_END_CALLBACK    0xFFFFFFFFu /* callback (-1) passed to 0x8007EE84 */
#define DS_RESULT_IDLE     1u    /* value returned when not active */

/* 0x80081268: when the active word is not 1, store 0 to it and return.
 * Otherwise read the flags word, call 0x8007F69C (through its guest name) and
 * 0x800822BC, call 0x800824F0 with the callback word when bit 0 of the flags
 * is set, call 0x8007FCBC(1, 0), then 0x8007EE84(9, 0, 0, -1).  The active
 * word is cleared after those calls in every case.  The original returned the
 * 0x8007EE84 result (1 when not active); the native is void and the vectors
 * compare no return value. */
void pe_plat_ds_81268(void)
{
    if (PE_LoadU32(ADDR_DS_ACTIVE) == DS_ACTIVE) {
        uint32_t flags = PE_LoadU32(ADDR_DS_FLAGS);

        func_8007F69C();
        func_800822BC();
        if (flags & DS_FLAG_CALLBACK)
            func_800824F0(PE_LoadU32(ADDR_DS_CALLBACK));
        func_8007FCBC(DS_CHECK_ARG0, DS_CHECK_ARG1);
        func_8007EE84(DS_END_COMMAND, DS_END_PARAM, DS_END_EXTRA, DS_END_CALLBACK);
    }
    PE_StoreU32(ADDR_DS_ACTIVE, 0);
}
