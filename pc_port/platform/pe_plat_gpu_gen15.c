/* One small graphics-library helper (guest 0x800719C4), written clean-room
 * from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen15, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Behaviour is
 * exactly the guest load and return value seen in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

#define REC_FLAG_WORD_OFF   0x04u  /* record + 0x04: word holding the flag bits */
#define REC_FLAG_FIELD_BIT  0x08u  /* flag tested: bit 3 of that word */
#define REC_FIELD_PTR_OFF   0x14u  /* record + 0x14: the field address returned */

/* 0x800719C4: read the word at rec+4.  When bit 3 (0x08) of it is set return
 * rec+0x14, otherwise return 0.  The other bits of the word are ignored.  No
 * guest stores. */
pe_addr_t pe_plat_gpu_flag_field(pe_addr_t rec)
{
    if (PE_LoadU32(rec + REC_FLAG_WORD_OFF) & REC_FLAG_FIELD_BIT)
        return rec + REC_FIELD_PTR_OFF;
    return 0;
}
