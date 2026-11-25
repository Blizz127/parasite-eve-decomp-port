/* SPU DMA4 transfer-completion handler (guest 0x8007D614), written clean-room
 * from black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * fidma), with no SDK source.  Evidence: docs/evidence/native-spu-fidma-20261031/. */
#include "pe_sdk.h"
#include "pe_guestcode.h"
#include "pe_guest_ram.h"

/* Pointer global holding the SPU register block base (0x1F801C00). */
#define SPU_BASE_PTR        0x8009B3FCu
#define SPU_CTRL_REG        0x1AAu  /* halfword, offset from the block base */
#define SPU_CTRL_XFER_MASK  0x0030u /* bits 4-5: transfer mode, cleared here */

/* When this word is exactly 0, the SPU wait helper runs first. */
#define STATE_WAIT_WORD     0x8009B44Cu
/* Callback word: a guest function pointer (0 = none). */
#define CALLBACK_WORD       0x8009B434u
#define CALLBACK_SLOT       "D_8009B434"
#define CALLER_NAME         "func_8007D614"
#define CALLBACK_ARG        0xF0000000u /* the one argument the callback gets */

/* DeliverEvent (class, spec) raised when there is no callback. */
#define EVENT_CLASS         0xF0000009u
#define EVENT_SPEC          0x00000020u

/* Register wait loop: at most this many re-reads of the control halfword
 * (sticky bits 4-5 end it by timeout: 1 + 0xF01 reads in all). */
#define WAIT_READS_MAX      0xF01u

/* Order: if the state word is exactly 0, the SPU wait helper is called first.
 * Then the SPU control halfword is read once, written back with bits 4-5
 * cleared, and re-read until those bits read clear or WAIT_READS_MAX re-reads
 * have been made (then it carries on as if cleared: no message, no store).
 * Finally, if the callback word is nonzero that function is called with
 * $a0 = 0xF0000000; otherwise DeliverEvent(0xF0000009, 0x20) is raised.
 * Nothing is returned. */
void pe_plat_spu_fidma(void)
{
    uint32_t spu, ctrl, fn, n;

    if (PE_LoadU32(STATE_WAIT_WORD) == 0u)
        func_8007DCAC();

    spu = PE_LoadU32(SPU_BASE_PTR);
    ctrl = PE_LoadU16(spu + SPU_CTRL_REG);
    PE_StoreU16(spu + SPU_CTRL_REG, (uint16_t)(ctrl & ~SPU_CTRL_XFER_MASK));
    n = 0u;
    do {
        ctrl = PE_LoadU16(spu + SPU_CTRL_REG);
    } while ((ctrl & SPU_CTRL_XFER_MASK) != 0u && ++n < WAIT_READS_MAX);

    fn = PE_LoadU32(CALLBACK_WORD);
    if (fn != 0u) {
        int ret = 0;
        (void)PE_GuestHook_Call(CALLBACK_SLOT, CALLER_NAME, fn, 1u, CALLBACK_ARG, 0u, 0u, 0u, &ret);
    } else {
        PE_Event_Deliver(EVENT_CLASS, EVENT_SPEC);
    }
}
