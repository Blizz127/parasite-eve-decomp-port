/* One small memory-card library helper (guest 0x8007E4E0), written clean-room
 * from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen15, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Behaviour is
 * exactly the guest loads and stores seen in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

#define ADDR_PATCH_SOURCE  0x8007E344u  /* first source word */
#define ADDR_PATCH_DEST    0x8000DF80u  /* first destination word */
#define PATCH_WORD_COUNT   28u          /* words copied: 0x70 bytes */
#define PATCH_WORD_SIZE    4u

/* 0x8007E4E0: copy 28 words (0x70 bytes) from guest 0x8007E344..0x8007E3B3 to
 * guest 0x8000DF80..0x8000DFEF, ascending, one 32-bit load then one 32-bit
 * store per word.  The data is read from guest memory, not built in.  The
 * original returns the end of the destination (0xDFF0); the port's entry is
 * void and the value is not compared. */
void pe_plat_card_copy_patch(void)
{
    uint32_t i;

    for (i = 0; i < PATCH_WORD_COUNT; i++) {
        uint32_t off = i * PATCH_WORD_SIZE;
        PE_StoreU32(ADDR_PATCH_DEST + off, PE_LoadU32(ADDR_PATCH_SOURCE + off));
    }
}
