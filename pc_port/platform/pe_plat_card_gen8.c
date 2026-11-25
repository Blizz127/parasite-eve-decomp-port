/* One small libcard caller (guest 0x800840DC), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * cdgen8, checked by pc_port/tools/pe_spu_caller_check.c), with no SDK source.
 * Behaviour is exactly the guest loads, the libcard slot call and the
 * byte-write call seen in the vectors.  No guest stores. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

#define CARD_OBJ_PTR_OFF   0x3Cu         /* word at a0+0x3C: pointer to a flag byte */
#define CARD_OBJ_BYTE_OFF  0x36u         /* byte at a0+0x36, read in one flag case */
#define CARD_FLAG_MASK     0xF0u         /* top nibble of the flag byte ... */
#define CARD_FLAG_SPECIAL  0x80u         /* ... equal to 8 selects the a0+0x36 test */
#define ADDR_CARD_SLOT     0x8009B72Cu   /* libcard slot D_8009B72C (guest fn pointer) */
#define CARD_ERR_BUSY      ((int)-9)     /* result when the byte write reports > 0 ... */
#define CARD_WRITE_OK      0x5A          /* ... except exactly 0x5A, which passes through */

/* Guest 0x800832B4 (libcard byte write) is declared in pe_sdk.h. */

/* 0x800840DC: read the flag byte through the pointer at a0+0x3C.  If its top
 * nibble is 8, the second argument is (byte at a0+0x36 == 0), otherwise 0.
 * Call the slot with (a0, that argument); then write its low byte through
 * func_800832B4(a0, low byte).  A positive write result becomes -9, except the
 * single value 0x5A which is returned as is; zero and negative results are
 * returned unchanged. */
int pe_plat_card_840dc(uint32_t a0)
{
    uint32_t p = PE_LoadU32(a0 + CARD_OBJ_PTR_OFF);
    uint32_t flag = PE_LoadU8(p);
    uint32_t arg = 0;
    uint32_t fn, r;
    int w;

    if ((flag & CARD_FLAG_MASK) == CARD_FLAG_SPECIAL)
        arg = (PE_LoadU8(a0 + CARD_OBJ_BYTE_OFF) == 0) ? 1u : 0u;

    fn = PE_LoadU32(ADDR_CARD_SLOT);
    r = PE_Card_ICall("D_8009B72C (card frame exchange)", fn, 2, a0, arg, 0);

    w = func_800832B4(a0, (int)(r & 0xFFu));
    return (w > 0 && w != CARD_WRITE_OK) ? CARD_ERR_BUSY : w;
}
