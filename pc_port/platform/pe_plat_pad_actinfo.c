/* libpad act-info receive/align handlers and combined info (guest 0x80083644 /
 * 0x80083C3C / 0x8008284C / 0x800837C8), written clean-room from black-box
 * vectors of the original code (pc_port/tools/pe_pad_leaf_vectors.py
 * batch3), with no SDK source.  Evidence:
 * docs/evidence/native-pad-actinfo-20261030/.
 * vectors_v3.txt (12660 cases) is now the reference; the revisions it forced
 * (state 4 byte wrap of the entry counter, negative term/offs in info_comb)
 * are recorded in NOTES.md, "v3 revisions". */
#include "pe_plat/pad.h"
#include "pe_guest_ram.h"

/* Record offsets (bytes from the record base). */
#define PAD_REC_BUF_0       0x00u /* word: first buffer pointer (set by load_act_info) */
#define PAD_REC_BUF_1       0x04u /* word: second buffer pointer; zero until load_act_info */
#define PAD_REC_BUF_2       0x08u /* word: third buffer pointer (set by load_act_info);
                                  * also the terminal table pointer read by info_comb */
#define PAD_REC_HANDLER_A   0x14u /* word: handler address (set by load_act_info) */
#define PAD_REC_HANDLER_B   0x18u /* word: second handler address */
#define PAD_REC_ALIGN_PTR   0x20u /* word: pointer to the six align bytes */
#define PAD_REC_INFO_PTR    0x3Cu /* word: pointer to the received info block */
#define PAD_REC_STATE       0x46u /* byte: receive state */
#define PAD_REC_STEP        0x47u /* byte: info entry counter */
#define PAD_REC_DONE        0x49u /* byte: written with 2 when the info is complete */
#define PAD_REC_ALIGN_OUT   0x5Du /* first of six result bytes (align handler) */
#define PAD_REC_ACT_BYTE    0xE3u /* byte: act_size term */
#define PAD_REC_ACT_ALT     0xE4u /* byte: copied from the info block in state 2 */
#define PAD_REC_ACT_HALF    0xE6u /* halfword: written by states 2 and 3 */
#define PAD_REC_ACT_SEL     0xE9u /* byte: act_size term (also the align entry count) */
#define PAD_REC_ACT_COUNT   0xEAu /* byte: info entry count (state 2 stores it, state 4 compares) */
#define PAD_REC_ACT_BASE    0xECu /* word: act_size base, advanced by state 4 */

/* info_comb: terminal table entry layout. */
#define PAD_INFO_ENTRY      8u    /* bytes per terminal entry */
#define PAD_INFO_PTR_OFFS   4u    /* word inside an entry: data pointer */

/* Offsets inside the received info block. */
#define PAD_INFO_BYTE_3     3u    /* state 2: copied to +0xE3 */
#define PAD_INFO_BYTE_4     4u    /* state 2: to +0xE4; state 3: high byte; state 4: entry size */
#define PAD_INFO_BYTE_5     5u    /* state 2: to +0xE9; state 3: low byte */
#define PAD_INFO_BYTE_6     6u    /* state 2: to +0xEA */

/* Receive states (+0x46). */
#define PAD_STATE_2         2u
#define PAD_STATE_3         3u
#define PAD_STATE_4         4u
#define PAD_STATE_ERROR     0xFFu /* state 4 completion, first store */
#define PAD_STATE_DONE      0xFEu /* state 4 completion */
#define PAD_STATE_REQUEST   2u    /* state 4 error path: second store to +0x46 */

/* State 4: each step advances the base by the entry size rounded up to four
 * plus an eight byte header. */
#define PAD_STEP_HEADER     8u
#define PAD_STEP_ROUND      3u

/* Align handler: six output bytes, five byte entries, type byte at +2. */
#define PAD_ALIGN_SLOTS     6u
#define PAD_ENTRY_STRIDE    5u
#define PAD_ENTRY_TYPE      2u
#define PAD_ALIGN_NONE      0xFFu

/* load_act_info constants. */
#define PAD_LOAD_DONE_FLAG  4u
#define PAD_LOAD_STATE      1u
#define PAD_LOAD_HANDLER_A  0x8008389Cu
#define PAD_LOAD_HANDLER_B  0x80083944u
#define PAD_IDX_STRIDE      4u    /* bytes per slot in the three buffers */

/* Receive one step of the info block for the record's state.  States 2 and 3
 * copy fields out of the block at +0x3C; state 4 walks the entries and, once
 * the entry counter passes the count, finishes the receive.  The entry
 * counter is a byte: it wraps to 0 after 0xFF and the wrapped value is both
 * stored and compared, so a wrapped counter does not finish the receive. */
int pe_plat_pad_recv_step(uint32_t record)
{
    uint32_t state = PE_LoadU8(record + PAD_REC_STATE);

    if (state == PAD_STATE_2) {
        uint32_t p1 = PE_LoadU32(record + PAD_REC_INFO_PTR);
        uint32_t p2 = PE_LoadU32(record + PAD_REC_INFO_PTR);
        uint32_t b3 = PE_LoadU8(p1 + PAD_INFO_BYTE_3);
        uint32_t b4 = PE_LoadU8(p2 + PAD_INFO_BYTE_4);
        uint32_t b5 = PE_LoadU8(PE_LoadU32(record + PAD_REC_INFO_PTR) + PAD_INFO_BYTE_5);
        uint32_t b6 = PE_LoadU8(PE_LoadU32(record + PAD_REC_INFO_PTR) + PAD_INFO_BYTE_6);
        PE_StoreU8(record + PAD_REC_ACT_BYTE, (uint8_t)b3);
        PE_StoreU16(record + PAD_REC_ACT_HALF, 0u);
        PE_StoreU8(record + PAD_REC_ACT_ALT, (uint8_t)b4);
        PE_StoreU8(record + PAD_REC_ACT_SEL, (uint8_t)b5);
        PE_StoreU32(record + PAD_REC_ACT_BASE, 0u);
        PE_StoreU8(record + PAD_REC_ACT_COUNT, (uint8_t)b6);
        return 1;
    }
    if (state == PAD_STATE_3) {
        uint32_t p = PE_LoadU32(record + PAD_REC_INFO_PTR);
        uint32_t hi = PE_LoadU8(p + PAD_INFO_BYTE_4);
        uint32_t lo = PE_LoadU8(p + PAD_INFO_BYTE_5);
        PE_StoreU8(record + PAD_REC_STEP, 0u);
        PE_StoreU16(record + PAD_REC_ACT_HALF, (uint16_t)((hi << 8) | lo));
        return 1;
    }
    if (state == PAD_STATE_4) {
        uint32_t base = PE_LoadU32(record + PAD_REC_ACT_BASE);
        uint32_t step = PE_LoadU8(record + PAD_REC_STEP);
        uint32_t p = PE_LoadU32(record + PAD_REC_INFO_PTR);
        uint32_t size = PE_LoadU8(p + PAD_INFO_BYTE_4);
        uint32_t count = PE_LoadU8(record + PAD_REC_ACT_COUNT);
        step = (uint8_t)(step + 1u);
        PE_StoreU8(record + PAD_REC_STEP, (uint8_t)step);
        PE_StoreU32(record + PAD_REC_ACT_BASE,
                    base + ((size + PAD_STEP_ROUND) & ~PAD_STEP_ROUND) + PAD_STEP_HEADER);
        if (step >= count) {
            uint32_t total = (uint32_t)pe_plat_pad_act_size(record);
            if ((total & 0x80000000u) != 0u) {
                PE_StoreU8(record + PAD_REC_STATE, PAD_STATE_ERROR);
                PE_StoreU8(record + PAD_REC_STATE, PAD_STATE_REQUEST);
            } else {
                PE_StoreU8(record + PAD_REC_STATE, PAD_STATE_DONE);
                PE_StoreU8(record + PAD_REC_DONE, PAD_STATE_REQUEST);
            }
        }
        return 0;
    }
    return 1;
}

/* Align handler: for each entry index k below the count at +0xE9, find the
 * align bytes equal to k.  A matching byte j gets k when the number of
 * matching bytes is at least the entry's type byte, otherwise 0xFF. */
int pe_plat_pad_align_handler(uint32_t record)
{
    uint32_t k;
    for (k = 0; k < PE_LoadU8(record + PAD_REC_ACT_SEL); k++) {
        uint32_t p = PE_LoadU32(record + PAD_REC_ALIGN_PTR);
        uint32_t q, type, j, matches = 0;
        for (j = 0; j < PAD_ALIGN_SLOTS; j++)
            if (PE_LoadU8(p + j) == k)
                matches++;
        q = PE_LoadU32(record + PAD_REC_BUF_1);
        type = PE_LoadU8(q + PAD_ENTRY_STRIDE * k + PAD_ENTRY_TYPE);
        p = PE_LoadU32(record + PAD_REC_ALIGN_PTR);
        for (j = 0; j < PAD_ALIGN_SLOTS; j++)
            if (PE_LoadU8(p + j) == k)
                PE_StoreU8(record + PAD_REC_ALIGN_OUT + j,
                           (uint8_t)(matches >= type ? k : PAD_ALIGN_NONE));
    }
    PE_StoreU8(record + PAD_REC_STATE, PAD_STATE_DONE);
    return 0;
}

/* Combined info for a port: terminal `term` must be below the count byte at
 * +0xEA.  Its entry in the table at +0x08 is eight bytes: a count byte, then
 * a pointer at +4.  Returns the byte at offset `offs` of that pointer when
 * `offs` is below the count, otherwise 0.  `term` and `offs` are signed: a
 * negative `term` returns the +0xEA count byte, and a negative `offs`
 * returns the entry's count byte (no data pointer load in either case). */
int pe_plat_pad_info_comb(int port, int term, int offs)
{
    uint32_t record = pe_plat_pad_record((uint32_t)port);
    uint32_t table, entry, count, data, limit;
    if (record == 0u)
        return 0;
    limit = PE_LoadU8(record + PAD_REC_ACT_COUNT);
    if (term < 0)
        return (int)limit;
    if (term >= (int)limit)
        return 0;
    table = PE_LoadU32(record + PAD_REC_BUF_2);
    entry = table + PAD_INFO_ENTRY * (uint32_t)term;
    count = PE_LoadU8(entry);
    if (offs < 0)
        return (int)count;
    if (offs >= (int)count)
        return 0;
    data = PE_LoadU32(entry + PAD_INFO_PTR_OFFS);
    return (int)PE_LoadU8(data + (uint32_t)offs);
}

/* Start an act-info load for `arg`: only when `arg` is nonzero, the second
 * buffer pointer is still zero and the record is not busy.  Installs the two
 * handlers and lays out the three buffers inside `arg`. */
int pe_plat_pad_load_act_info(uint32_t record, int arg)
{
    uint32_t base, e3, e9, second;
    if (arg == 0)
        return 0;
    if (PE_LoadU32(record + PAD_REC_BUF_1) != 0u)
        return 0;
    if (pe_plat_pad_record_busy(record) != 0)
        return 0;
    e3 = PE_LoadU8(record + PAD_REC_ACT_BYTE);
    e9 = PE_LoadU8(record + PAD_REC_ACT_SEL);
    base = ((uint32_t)arg + 3u) & ~3u;
    second = base + PAD_IDX_STRIDE * ((e3 + 1u) >> 1);
    PE_StoreU8(record + PAD_REC_DONE, PAD_LOAD_DONE_FLAG);
    PE_StoreU8(record + PAD_REC_STATE, PAD_LOAD_STATE);
    PE_StoreU32(record + PAD_REC_HANDLER_A, PAD_LOAD_HANDLER_A);
    PE_StoreU32(record + PAD_REC_HANDLER_B, PAD_LOAD_HANDLER_B);
    PE_StoreU32(record + PAD_REC_BUF_0, base);
    PE_StoreU8(record + PAD_REC_STEP, 0u);
    PE_StoreU32(record + PAD_REC_BUF_1, second);
    PE_StoreU32(record + PAD_REC_BUF_2, second + PAD_IDX_STRIDE * ((5u * e9 + 3u) >> 2));
    return 1;
}
