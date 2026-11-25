/* libds text routine 0x80082444, written clean-room from black-box vectors
 * of the original code (pc_port/tools/pe_cd_leaf_vectors.py cdgen6, checked
 * by pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Its behaviour
 * is exactly the guest stores and calls seen in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

#define ADDR_DS_TEXT_STATE  0x800B28F8u  /* word set before the final call */
#define ADDR_DS_ID_STRING   0x8001205Cu  /* 5-byte string compared against the sector */

#define DS_TEXT_STATE_DEFAULT 2u /* state value unless the id string matched */
#define DS_TEXT_STATE_MATCHED 4u /* state value when 0x80071A04 returned 0 */
#define DS_TEXT_MODE_CHECK  1u   /* argument value that triggers the sector check */
#define DS_SECTOR_WORDS     2    /* words fetched by 0x80080AE4 */
#define DS_SECTOR_ID_OFFSET 1u   /* the string is compared at buffer + 1 */
#define DS_SECTOR_ID_LEN    5    /* bytes compared by 0x80071A04 */
#define DS_FRAME_BYTES      8u   /* two-word sector buffer */

/* 0x80082444: when the argument is 1, read two words into a frame buffer
 * with 0x80080AE4 (result unused) and compare 5 bytes at buffer + 1 with the
 * string at 0x8001205C using 0x80071A04; if that returns 0 the state word
 * becomes 4.  In every other case it becomes 2.  The state word is stored
 * before the final call to 0x80081DF8. */
void pe_plat_ds_text_130(unsigned char a0)
{
    uint32_t state = DS_TEXT_STATE_DEFAULT;
    if (a0 == DS_TEXT_MODE_CHECK) {
        pe_addr_t f = PE_GuestFrame_Enter(DS_FRAME_BYTES);
        (void)func_80080AE4(f, DS_SECTOR_WORDS);
        if (func_80071A04(f + DS_SECTOR_ID_OFFSET, ADDR_DS_ID_STRING, DS_SECTOR_ID_LEN) == 0)
            state = DS_TEXT_STATE_MATCHED;
        PE_GuestFrame_Leave(f, DS_FRAME_BYTES);
    }
    PE_StoreU32(ADDR_DS_TEXT_STATE, state);
    func_80081DF8();
}
