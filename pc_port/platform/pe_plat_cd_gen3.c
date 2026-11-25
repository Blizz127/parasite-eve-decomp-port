/* libds DsFlush (guest 0x8007F69C), written clean-room from black-box vectors
 * of the original code (pc_port/tools/pe_cd_leaf_vectors.py cdgen3, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  The behaviour is
 * exactly the guest stores and calls seen in the vectors; the function reads
 * no guest memory and returns nothing. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

/* Zeroed guest data.  The vectors show a run of zero stores in this order:
 * three words descending at 0x800A3600..0x800A3608, a word at 0x800A3540 and
 * five bytes at 0x800A3544..0x800A3548, then seven 0x18-byte units (four
 * words, then five bytes) from 0x800A354C, then three words 0x800A35F4..
 * 0x800A35FC. */
#define ADDR_DS_TAIL        0x800A3600u  /* three words, cleared highest first */
#define ADDR_DS_HEAD_WORD   0x800A3540u  /* single word, cleared first */
#define ADDR_DS_HEAD_BYTES  0x800A3544u  /* five bytes cleared after it */
#define ADDR_DS_UNITS       0x800A354Cu  /* first of the seven units */
#define ADDR_DS_PRE_TAIL    0x800A35F4u  /* three words just before the tail */
#define DS_UNIT_COUNT       7u
#define DS_UNIT_STRIDE      0x18u        /* bytes per unit */
#define DS_UNIT_WORDS       4u           /* words at the start of a unit */
#define DS_UNIT_BYTES_OFF   0x10u        /* first byte of a unit's byte run */
#define DS_BYTE_RUN         5u           /* bytes in each byte run */

/* Five bytes: the first, then the remaining four from the highest down. */
static void clear_byte_run(pe_addr_t base)
{
    unsigned i;

    PE_StoreU8(base, 0);
    for (i = DS_BYTE_RUN - 1u; i > 0u; i--)
        PE_StoreU8(base + i, 0);
}

static void clear_words(pe_addr_t base, unsigned count)
{
    unsigned i;

    for (i = 0; i < count; i++)
        PE_StoreU32(base + 4u * i, 0);
}

/* 0x8007F69C: call 0x800808BC, clear the data described above, then call
 * 0x80081DF8 and 0x80080930 in that order. */
void pe_plat_ds_flush(void)
{
    unsigned i;

    func_800808BC();

    PE_StoreU32(ADDR_DS_TAIL + 8u, 0);
    PE_StoreU32(ADDR_DS_TAIL + 4u, 0);
    PE_StoreU32(ADDR_DS_TAIL, 0);
    PE_StoreU32(ADDR_DS_HEAD_WORD, 0);
    clear_byte_run(ADDR_DS_HEAD_BYTES);
    for (i = 0; i < DS_UNIT_COUNT; i++) {
        pe_addr_t unit = ADDR_DS_UNITS + i * DS_UNIT_STRIDE;

        clear_words(unit, DS_UNIT_WORDS);
        clear_byte_run(unit + DS_UNIT_BYTES_OFF);
    }
    clear_words(ADDR_DS_PRE_TAIL, 3);

    func_80081DF8();
    func_80080930();
}
