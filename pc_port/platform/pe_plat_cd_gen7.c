/* One small libcd data-callback caller (guest 0x8007F960), written clean-room
 * from black-box vectors of the original code (pc_port/tools/
 * pe_cd_leaf_vectors.py cdgen7, checked by pc_port/tools/pe_spu_caller_check.c),
 * with no SDK source.  Behaviour is exactly the guest load and the callback
 * dispatch seen in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

#define ADDR_CD_DATA_CB  0x800B8AB8u  /* registered CD data callback pointer */
#define CD_STATUS_MASK   0xFFu        /* callback receives the low status byte */

/* 0x8007F960: load the data-callback pointer; when it is non-zero, call it with
 * the low byte of status and the response pointer.  No guest stores. */
void pe_plat_cd_text_13cc(uint32_t status, pe_plat_guest_addr response)
{
    uint32_t target = PE_LoadU32(ADDR_CD_DATA_CB);

    if (target != 0)
        (void)PE_Cd_DispatchDataCallback(target, status & CD_STATUS_MASK, response);
}
