/*
 * Phase 6E-B42 — func_80053B48: register and accumulate a resource-record
 * category.
 *
 * Retail body: 121 instructions / 0x1E4 bytes at
 * 0x80053B48..0x80053D2B (exclusive end 0x80053D2C), executable file
 * offset 0x44348.  The live body is asm/disc1/43724.s:940-1082, selected
 * by configs/USA/disc1.yaml:337.  tools/b42_oracle.py contains the full
 * executable-verified transcription and delay-slot-aware execution model.
 *
 * True ABI:
 *
 *     int32_t func_80053B48(pe_addr_t record)
 *
 * Record types 1..7 and 16..18 map to one of three fixed category records.
 * The function ensures the corresponding 0x200..0x202 ID is present in the
 * current authoritative halfword table, then updates the category record's
 * 16-bit accumulated field.  It returns zero unless the ID table is full;
 * unsupported record types and a full table return one.  A full table does
 * not suppress the later category-record update.
 *
 * Classification: 1 — translated retail resource-record logic.
 */
#include "psx_compat.h"

#define B42_CATEGORY_BASE 0x800A1E64u

static int32_t B42_Sra1(uint32_t value)
{
    return (int32_t)((value >> 1u) |
                     ((value & 0x80000000u) ? 0x80000000u : 0u));
}

int32_t func_80053B48(pe_addr_t record)
{
    uint32_t type = PE_LoadU8(record + 6u);
    int32_t category;
    int32_t table_slot = -1;
    int32_t status = 0;
    pe_addr_t cursor;
    pe_addr_t end;
    pe_addr_t table_base;
    pe_addr_t category_record;
    uint32_t count;
    uint32_t old_accumulated;
    uint32_t amount;
    uint32_t base_value;
    uint32_t accumulated;
    int32_t delta;
    int32_t threshold;

    /* 0x80053B54..0x80053B98: exact record-type classification. */
    if (type < 16u) {
        if (type == 0u || type >= 8u) {
            category = -1;
        } else if (type <= 4u) {
            category = 0;
        } else {
            category = (int32_t)type - 5;
        }
    } else {
        category = (int32_t)(type - 16u);
    }

    if ((uint32_t)category >= 3u)
        return 1;

    /* 0x80053BA4..0x80053C08: search for the category's existing ID. */
    count = (uint32_t)D_8009D050;
    cursor = (pe_addr_t)D_8009D048;
    end = (pe_addr_t)(cursor + (count << 1u));
    if (cursor < end) {
        const int32_t target = category + 0x200;
        while ((int16_t)PE_LoadU16(cursor) != target) {
            cursor += 2u;
            if (cursor >= end)
                break;
        }

        count = (uint32_t)D_8009D050;
        table_base = (pe_addr_t)D_8009D048;
        end = (pe_addr_t)(table_base + (count << 1u));
        if (cursor < end)
            table_slot = B42_Sra1(cursor - table_base);
    }

    /* 0x80053C10..0x80053C98: if absent, install the ID in the first zero
     * signed-halfword slot.  Failure changes the return status but retail
     * continues to the category-record update below. */
    if (table_slot < 0) {
        count = (uint32_t)D_8009D050;
        cursor = (pe_addr_t)D_8009D048;
        end = (pe_addr_t)(cursor + (count << 1u));
        if (cursor < end) {
            while ((int16_t)PE_LoadU16(cursor) != 0) {
                cursor += 2u;
                if (cursor >= end)
                    break;
            }

            count = (uint32_t)D_8009D050;
            table_base = (pe_addr_t)D_8009D048;
            end = (pe_addr_t)(table_base + (count << 1u));
            if (cursor < end)
                table_slot = B42_Sra1(cursor - table_base);
        }

        if (table_slot < 0) {
            status = 1;
        } else {
            table_base = (pe_addr_t)D_8009D048;
            PE_StoreU16((pe_addr_t)(table_base +
                                    ((uint32_t)table_slot << 1u)),
                        (uint32_t)(category + 0x200));
        }
    }

    /* 0x80053C98..0x80053D14: all reads preceding the first store retain
     * retail order and width.  The halfword sum wraps at the store. */
    category_record = (pe_addr_t)(B42_CATEGORY_BASE +
                                  ((uint32_t)category << 5u));
    old_accumulated = (uint32_t)PE_LoadU16(category_record + 0x0Au);
    amount = (uint32_t)PE_LoadU16(record + 0x0Au);
    base_value = (uint32_t)PE_LoadU8(category_record + 0x09u);
    accumulated = old_accumulated + amount;
    delta = (int32_t)(int16_t)PE_LoadU16(category_record + 0x12u);
    PE_StoreU16(category_record + 0x0Au, accumulated);
    threshold = (int32_t)base_value + delta;
    accumulated &= 0xFFFFu;

    if (threshold < 1000) {
        if (threshold >= (int32_t)accumulated)
            return status;
    } else if ((int32_t)accumulated < 1000) {
        return status;
    }

    /* Retail reloads both fields before this second, conditional store. */
    delta = (int32_t)(int16_t)PE_LoadU16(category_record + 0x12u);
    base_value = (uint32_t)PE_LoadU8(category_record + 0x09u);
    threshold = delta + (int32_t)base_value;
    if (threshold >= 1000)
        threshold = 999;
    PE_StoreU16(category_record + 0x0Au, (uint32_t)threshold);
    return status;
}
