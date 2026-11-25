/* GTE matrix stack: PushMatrix (guest 0x80078A94) and PopMatrix (guest
 * 0x80078B38), written clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_gte_leaf_vectors.py stack), with no SDK source.
 * Evidence: docs/evidence/native-gte-setters-20261030/. */
#include "pe_plat/gte.h"
#include "pe_guest_ram.h"
#include "pe_bios_string.h"
#include "pe_sdk.h"

#define STACK_INDEX_ADDR    0x800963E8u /* byte offset of the next free frame */
#define STACK_AREA_ADDR     0x800963ECu /* frame storage; frame n is at +32*n */
#define FRAME_BYTES         32u         /* eight words: GTE control regs 0..7 */
#define FRAME_WORDS         8u          /* rotation (0..4), translation (5..7) */
#define STACK_LIMIT         0x280       /* push fails once the index reaches 20 frames */

#define PUSH_FULL_MSG_ADDR  0x8009666Cu /* printf format: push with a full stack */
#define POP_EMPTY_MSG_ADDR  0x8009669Du /* printf format: pop with an empty stack */

/* Index is a signed word (the vectors show pop failing for negative values,
 * and it is not required to be a multiple of 32).  Failure paths read only the
 * index, write nothing and call printf with the message and no arguments. */

void pe_plat_gte_push_matrix(void)
{
    int32_t idx = (int32_t)PE_LoadU32(STACK_INDEX_ADDR);
    uint32_t frame, i;

    if (idx >= STACK_LIMIT) {
        PE_Bios_Printf(PUSH_FULL_MSG_ADDR, NULL, 0);
        return;
    }
    frame = STACK_AREA_ADDR + (uint32_t)idx;
    /* ReadCtrl sign-extends regs 4, 12, 20, so the frame holds the extended
     * word for reg 4; the index is updated after the frame is written. */
    for (i = 0; i < FRAME_WORDS; i++)
        PE_StoreU32(frame + 4u * i, PE_GTE_ReadCtrl(i));
    PE_StoreU32(STACK_INDEX_ADDR, (uint32_t)idx + FRAME_BYTES);
}

void pe_plat_gte_pop_matrix(void)
{
    int32_t idx = (int32_t)PE_LoadU32(STACK_INDEX_ADDR);
    uint32_t frame, i;

    if (idx <= 0) {
        PE_Bios_Printf(POP_EMPTY_MSG_ADDR, NULL, 0);
        return;
    }
    /* The index is stored back first: for an index below 32 the frame
     * overlaps the index word and one frame register reads the new value. */
    idx -= (int32_t)FRAME_BYTES;
    PE_StoreU32(STACK_INDEX_ADDR, (uint32_t)idx);
    frame = STACK_AREA_ADDR + (uint32_t)idx;
    for (i = 0; i < FRAME_WORDS; i++)
        PE_GTE_WriteCtrl(i, PE_LoadU32(frame + 4u * i));
}
