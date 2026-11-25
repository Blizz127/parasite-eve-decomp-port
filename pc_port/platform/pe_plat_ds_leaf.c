/* Command-queue leaves and the status-lane getter (guest 0x8007E594 /
 * 0x8007E5C4 / 0x8007E6B0 / 0x8007FBF0), written clean-room from black-box
 * vectors of the original code (pc_port/tools/pe_ds_leaf_vectors.py), with no
 * SDK source.  Evidence: docs/evidence/native-ds-leaf-20261031/. */
#include "pe_plat/cd.h"
#include "pe_guest_ram.h"

/* The vectors show eight 24-byte records starting at CQ_RECORDS, followed
 * directly by three words: head index, tail index, pending count. */
#define CQ_RECORDS          0x800A3540u
#define CQ_RECORD_SIZE      24u
#define CQ_RECORD_COUNT     8
#define CQ_HEAD             0x800A3600u /* int: index of the oldest record */
#define CQ_TAIL             0x800A3604u /* int: set to the head after a pop */
#define CQ_PENDING          0x800A3608u /* int: records still queued */

/* Record fields touched by a clear (offsets in bytes).  Word 0 is the record
 * key; bytes 4..7 and the byte at 8 are single-byte stores. */
#define CQ_REC_KEY          0x00u /* word */
#define CQ_REC_BYTE4        0x04u
#define CQ_REC_BYTE8        0x08u
#define CQ_REC_BYTE7        0x07u
#define CQ_REC_BYTE6        0x06u
#define CQ_REC_BYTE5        0x05u
#define CQ_REC_WORD_C       0x0Cu
#define CQ_REC_WORD_10      0x10u
#define CQ_REC_WORD_14      0x14u

/* Status lane table: one word per lane, indexed by a signed argument. */
#define STATUS_LANES        0x8009B574u
#define STATUS_LANE_STRIDE  4u

/* Record address for a signed index; no bounds check (32-bit wrap). */
static uint32_t record_addr(int32_t idx)
{
    return CQ_RECORDS + CQ_RECORD_SIZE * (uint32_t)idx;
}

/* Zero a record: key, the byte fields (4, 8, 7, 6, 5 in that order) and the
 * three trailing words.  The vectors show q - 1 as the original's return in
 * every case; the port's prototype is void. */
void pe_plat_ds_cq_clear(uint32_t q)
{
    PE_StoreU32(q + CQ_REC_KEY, 0u);
    PE_StoreU8(q + CQ_REC_BYTE4, 0u);
    PE_StoreU8(q + CQ_REC_BYTE8, 0u);
    PE_StoreU8(q + CQ_REC_BYTE7, 0u);
    PE_StoreU8(q + CQ_REC_BYTE6, 0u);
    PE_StoreU8(q + CQ_REC_BYTE5, 0u);
    PE_StoreU32(q + CQ_REC_WORD_C, 0u);
    PE_StoreU32(q + CQ_REC_WORD_10, 0u);
    PE_StoreU32(q + CQ_REC_WORD_14, 0u);
}

/* Retire the head record while the pending count (signed) is positive, then
 * keep retiring successors whose key word equals the first one's.  Head
 * advances by one with wrap to 0 once it reaches 8 (signed compare, so a
 * negative head just keeps counting up).  Every field is re-read from guest
 * RAM each time because a wild head can make a record overlap the indices.
 * The tail index is set to the final head.  The key of the first record is
 * only read when something will be retired. */
void pe_plat_ds_cq_pop(void)
{
    int32_t head = (int32_t)PE_LoadU32(CQ_HEAD);
    int32_t pending;
    uint32_t key;

    pending = (int32_t)PE_LoadU32(CQ_PENDING);
    if (pending > 0) {
        key = PE_LoadU32(record_addr(head) + CQ_REC_KEY);
        head = (int32_t)PE_LoadU32(CQ_HEAD);
        for (;;) {
            int32_t next, cnt;
            uint32_t next_key;

            pe_plat_ds_cq_clear(record_addr(head));
            next = (int32_t)PE_LoadU32(CQ_HEAD);
            cnt = (int32_t)PE_LoadU32(CQ_PENDING);
            PE_StoreU32(CQ_HEAD, (uint32_t)next + 1u);
            if ((int32_t)((uint32_t)next + 1u) >= CQ_RECORD_COUNT)
                PE_StoreU32(CQ_HEAD, 0u);
            PE_StoreU32(CQ_PENDING, (uint32_t)cnt - 1u);
            head = (int32_t)PE_LoadU32(CQ_HEAD);
            next_key = PE_LoadU32(record_addr(head) + CQ_REC_KEY);
            if (next_key != key) {
                head = (int32_t)PE_LoadU32(CQ_HEAD);
                break;
            }
            pending = (int32_t)PE_LoadU32(CQ_PENDING);
            head = (int32_t)PE_LoadU32(CQ_HEAD);
            if (pending <= 0)
                break;
        }
    }
    PE_StoreU32(CQ_TAIL, (uint32_t)head);
}

/* Address of the record that the next push would use: head + pending, wrapped
 * once by 8, or 0 when 8 or more are pending (signed compare; the head is not
 * read in that case). */
uint32_t pe_plat_ds_cq_next_slot(void)
{
    int32_t pending = (int32_t)PE_LoadU32(CQ_PENDING);
    int32_t idx;
    if (pending >= CQ_RECORD_COUNT)
        return 0u;
    idx = (int32_t)((uint32_t)(int32_t)PE_LoadU32(CQ_HEAD) + (uint32_t)pending);
    if (idx >= CQ_RECORD_COUNT)
        idx -= CQ_RECORD_COUNT;
    return record_addr(idx);
}

/* Word at STATUS_LANES + 4 * idx, returned as is (32-bit address wrap). */
int pe_plat_ds_status_lane(int idx)
{
    return (int)PE_LoadU32(STATUS_LANES + STATUS_LANE_STRIDE * (uint32_t)idx);
}
