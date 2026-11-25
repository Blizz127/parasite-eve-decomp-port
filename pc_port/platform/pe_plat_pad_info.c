/* libpad PadInfoMode (guest 0x80082680), written clean-room by an author who
 * saw neither SDK source nor this port's earlier code: 26040 seeded black-box
 * vectors of the original (pc_port/tools/pe_pad_info_vectors.py); see
 * docs/evidence/native-pad-infomode-20261030/. */
/* libpad PadInfoMode clean-room model, derived only from black-box vectors
 * (vectors.txt).  `term` selects a field of the controller record; term 4
 * indexes a halfword table that the record points at; term 100 returns a record
 * word.  Terms 0, 5, 6, 7, 8, -1 and every other value were observed to
 * return 0. */
#include "pe_plat/pad.h"
#include "pe_guest_ram.h"

#define PAD_REC_TABLE_PTR   0x00u /* term 4: guest pointer to a halfword table */
#define PAD_REC_WORD_100    0x4Cu /* term 100: raw record word (pointer-like)   */
#define PAD_REC_INFO_COUNT  0xE3u /* term 4, offs<0: byte, table entry count */
#define PAD_REC_INFO_BYTE_A 0xE4u /* term 3: byte                              */
#define PAD_REC_INFO_HALF   0xE6u /* term 2: halfword                          */
#define PAD_REC_INFO_BYTE_B 0xE8u /* term 1: byte                              */

#define PAD_TERM_BYTE_B 1
#define PAD_TERM_HALF   2
#define PAD_TERM_BYTE_A 3
#define PAD_TERM_TABLE  4
#define PAD_TERM_WORD   100

uint32_t pe_plat_pad_info_mode(uint32_t port, int32_t term, int32_t offs)
{
    uint32_t rec = pe_plat_pad_record(port);
    uint32_t count;

    if (rec == 0u) {
        return 0u;
    }
    switch (term) {
    case PAD_TERM_BYTE_B:
        return PE_LoadU8(rec + PAD_REC_INFO_BYTE_B);
    case PAD_TERM_HALF:
        return PE_LoadU16(rec + PAD_REC_INFO_HALF);
    case PAD_TERM_BYTE_A:
        return PE_LoadU8(rec + PAD_REC_INFO_BYTE_A);
    case PAD_TERM_TABLE:
        count = PE_LoadU8(rec + PAD_REC_INFO_COUNT);
        if (offs < 0) {
            return count; /* vectors: -1 and -2 report the entry count */
        }
        if ((uint32_t)offs >= count) {
            return 0u; /* vectors: offs at or past the count reads 0 */
        }
        return PE_LoadU16(PE_LoadU32(rec + PAD_REC_TABLE_PTR) + 2u * (uint32_t)offs);
    case PAD_TERM_WORD:
        return PE_LoadU32(rec + PAD_REC_WORD_100);
    default:
        return 0u;
    }
}
