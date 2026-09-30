/*
 * Phase 6E-B44 — func_8005B91C: signed table index/interpolation lookup.
 *
 * Retail executable 0x8005B91C..0x8005BA77 (exclusive end 0x8005BA78),
 * file offset 0x4C11C, 0x15C bytes / 87 MIPS-I instructions.  The complete
 * literal body is SHA-verified and independently executed by b44_oracle.py.
 * Its sole callee is translated func_8005DB8C.
 *
 * The retail ABI has two nullable guest output pointers.  B43 instead passes
 * a retail-stack temporary, represented natively by the separate HostOut
 * adapter below.  Both entry points use the same arithmetic core and preserve
 * retail's index-before-fraction write ordering.  No host pointer is ever
 * narrowed to pe_addr_t or stored in guest RAM.
 */
#include "psx_compat.h"
#include <limits.h>

typedef struct {
    int32_t index;
    int32_t fraction;
} B44Result;

static int32_t B44_WrapSub(int32_t left, int32_t right)
{
    return (int32_t)((uint32_t)left - (uint32_t)right);
}

static int32_t B44_WrapMul49(int32_t value)
{
    uint32_t x = (uint32_t)value;
    uint32_t three = (x << 1u) + x;
    return (int32_t)((three << 4u) + x);
}

static B44Result B44_Calculate(int32_t table_number, int32_t key)
{
    B44Result result;
    pe_addr_t base = func_8005DB8C(table_number);
    int32_t index = 64;
    int32_t step = 32;

    do {
        pe_addr_t current_address =
            (pe_addr_t)(base + ((uint32_t)index << 2u));
        int32_t current = (int32_t)PE_LoadU32(current_address);

        if (key >= current) {
            index += step;
        } else {
            int32_t previous = (int32_t)PE_LoadU32(current_address - 4u);
            if (key < previous)
                index -= step;
            else
                step = 0;
        }
        step >>= 1;
    } while (step != 0);

    index -= 1;
    if (index >= 99)
        index = 98;

    result.index = index;
    result.fraction = 0;
    if (index < 98) {
        pe_addr_t selected = (pe_addr_t)(base + ((uint32_t)index << 2u));
        int32_t high = (int32_t)PE_LoadU32(selected + 4u);
        int32_t low = (int32_t)PE_LoadU32(selected);

        if (high != low) {
            int32_t numerator = B44_WrapMul49(B44_WrapSub(key, low));
            int32_t denominator = B44_WrapSub(high, low);

            if (denominator == 0 ||
                (numerator == INT32_MIN && denominator == -1)) {
                abort();
            }
            result.fraction = numerator / denominator;
        }
        if (result.fraction >= 49)
            result.fraction = 48;
    } else {
        result.fraction = 48;
    }
    return result;
}

void func_8005B91C(int32_t table_number, int32_t key,
                   pe_addr_t index_out, pe_addr_t fraction_out)
{
    B44Result result = B44_Calculate(table_number, key);

    if (index_out != 0u)
        PE_StoreU32(index_out, (uint32_t)result.index);
    if (fraction_out != 0u)
        PE_StoreU32(fraction_out, (uint32_t)result.fraction);
}

void PE_func_8005B91C_HostOut(int32_t table_number, int32_t key,
                              int32_t *index_out, int32_t *fraction_out)
{
    B44Result result = B44_Calculate(table_number, key);

    if (index_out != NULL)
        *index_out = result.index;
    if (fraction_out != NULL)
        *fraction_out = result.fraction;
}
