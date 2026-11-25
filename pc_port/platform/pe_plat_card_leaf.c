/* Memory-card timer-callback table init (guest 0x80082ADC) and pending-word
 * read-and-clear (guest 0x80082CDC), written clean-room from black-box
 * vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py,
 * entries 0x80082ADC/0x80082CDC), with no SDK source.
 * Evidence: docs/evidence/native-card-leaf-20261031/. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

/* Four-word block in guest RAM written by the init; none of it is read. */
#define CARD_CB_BLOCK        0x800A5AB0u
#define CARD_CB_WORD_0       (CARD_CB_BLOCK + 0x0u)  /* cleared */
#define CARD_CB_WORD_4       (CARD_CB_BLOCK + 0x4u)  /* first callback */
#define CARD_CB_WORD_8       (CARD_CB_BLOCK + 0x8u)  /* second callback */
#define CARD_CB_WORD_C       (CARD_CB_BLOCK + 0xCu)  /* cleared */

/* Guest code addresses observed stored in the vectors (the callbacks the
 * original registers; stored verbatim). */
#define CARD_CB_FIRST_ADDR   0x80082B70u
#define CARD_CB_SECOND_ADDR  0x80082B08u

/* Guest word read, returned, and cleared by the update (constant address in
 * every vector). */
#define CARD_PENDING_WORD    0x8009B78Cu

/* Fill the callback block: first and second callback addresses, then zero
 * the two remaining words.  Stores in the original's order; no loads. */
void pe_plat_card_init_timer_callbacks(void)
{
    PE_StoreU32(CARD_CB_WORD_4, CARD_CB_FIRST_ADDR);
    PE_StoreU32(CARD_CB_WORD_8, CARD_CB_SECOND_ADDR);
    PE_StoreU32(CARD_CB_WORD_0, 0);
    PE_StoreU32(CARD_CB_WORD_C, 0);
}

/* Read the pending word, clear it, and return the value read (all 32 bits,
 * as an int). */
int pe_plat_card_word_update(void)
{
    uint32_t v = PE_LoadU32(CARD_PENDING_WORD);
    PE_StoreU32(CARD_PENDING_WORD, 0);
    return (int)v;
}
