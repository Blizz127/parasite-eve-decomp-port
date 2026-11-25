/* Memory-card channel reset (guest 0x8007DD74, libcard _card_clear), written
 * clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cardclear), with no SDK source.
 * Evidence: docs/evidence/native-card-clear-20261031/. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

/* Sector number and source argument of the BIOS card write issued for the
 * channel (constant in every vector; the source is the null address). */
#define CARD_CLEAR_SECTOR 0x3F
#define CARD_CLEAR_SRC    0

/* Raise the channel's "new card" state, then issue the card write of sector
 * 0x3F from a null source.  No guest memory is read or written directly.
 * Returns the card write's result unchanged. */
int pe_plat_card_clear(uint32_t chan)
{
    func_8007DDC4((pe_addr_t)chan);
    return func_8007DDB4((pe_addr_t)chan, CARD_CLEAR_SECTOR, CARD_CLEAR_SRC);
}
