/*
 * Phase 6E-B29 — func_80053D2C: resource-table slot/type operation.
 *
 * Retail body: 80 words, 0x80053D2C..0x80053E6B (exclusive end
 * 0x80053E6C), file offset 0x4452C.  The complete body is transcribed and
 * independently checked by tools/b29_oracle.py.
 *
 * B41 translates the type-1..9 func_80053968 dependency.  B42 translates
 * the type-16..18 func_80053B48 dependency and passes the exact record guest
 * address already held in retail $a0.
 */
#include "psx_compat.h"
#include "pe_guest_ram.h"

extern pe_addr_t func_8005DB44(unsigned int index);

int func_80053D2C(int arg)
{
    unsigned int base = D_8009D048;
    unsigned int end = base + (D_8009D050 << 1);
    unsigned int cursor = base;
    int slot;
    unsigned int record;
    unsigned int type;
    int status = 0;

    /* 80053D4C..80053D7C: first zero halfword, or one-past the table. */
    if (base < end) {
        while (cursor < end) {
            if ((int16_t)PE_LoadU16(cursor) == 0)
                break;
            cursor += 2u;
        }
    }

    /* 80053D80..80053DA8: signed slot index, -1 when full/empty. */
    end = D_8009D048 + (D_8009D050 << 1);
    if (cursor < end)
        slot = (int)((cursor - D_8009D048) >> 1);
    else
        slot = -1;

    /* 80053DA8..80053DB0: IDs below 0x100 use their archive record. */
    if ((int32_t)arg < 0x100) {
        record = func_8005DB44((unsigned int)(arg - 1));
        if (record == 0)
            return status;

        type = PE_LoadU8((pe_addr_t)record + 6u);
        if (type >= 1u && type <= 9u) {
            pe_addr_t result = func_80053968((int32_t)arg);
            status = (result == 0u) ? 1 : 0;
        } else if (type == 10u || (type >= 12u && type <= 15u)) {
            if (slot < 0) {
                status = 1;
            } else {
                PE_StoreU16(D_8009D048 + ((unsigned int)slot << 1),
                            (uint32_t)arg);
            }
        } else if (type >= 16u && type <= 18u) {
            status = func_80053B48((pe_addr_t)record);
        }
        /* type 11 and all out-of-range types return the initial zero. */
    } else if (slot >= 0) {
        PE_StoreU16(D_8009D048 + ((unsigned int)slot << 1),
                    (uint32_t)arg);
    } else {
        status = 1;
    }

    return status;
}
