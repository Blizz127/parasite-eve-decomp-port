/*
 * Phase 6E-B43/B44, completed in EQP1 — func_8005218C stat update.
 *
 * The complete retail body is 155 instructions / 0x26C bytes at
 * 0x8005218C..0x800523F7 (exclusive end 0x800523F8), executable file
 * offset 0x4298C.  It is live in asm/disc1/42664.s:246-406 under the
 * configs/USA/disc1.yaml [0x42664, asm] split.  tools/b43_oracle.py holds
 * and executes the complete SHA-exact transcription.
 *
 * True top-level ABI: void func_8005218C(void).  All five executable
 * callers use a nop call delay slot and ignore the returned register.
 *
 * B44 translates all seven func_8005B91C calls and their output words.
 * EQP1 completes capacity publication through 52F24 and the final level
 * store, independently checked through the entire retail 2F76C call graph.
 * The native local represents retail sp+0x10 and is never converted to a
 * guest address or written into guest RAM.
 */
#include "psx_compat.h"

#define GA_B43_E06       0x800C0E06u
#define GA_B43_E08       0x800C0E08u
#define GA_B43_TARGET0   0x800C0E28u
#define GA_B43_RECORD_PP 0x8009D254u
#define GA_B43_G30       0x800A1B30u
#define GA_B43_G34       0x800A1B34u

static int32_t B43_S32(uint32_t value)
{
    return (int32_t)value;
}

void func_8005218C(void)
{
    int32_t table_index_out = 0;
    pe_addr_t resource;
    pe_addr_t holder;
    pe_addr_t record;
    uint32_t product;
    int32_t quotient;
    uint16_t quotient_half;
    unsigned int i;

    PE_func_8005B91C_HostOut(
        0, (int32_t)(int16_t)PE_LoadU16(GA_B43_TARGET0),
        &table_index_out, NULL);
    resource = func_8005DBAC(table_index_out);

    product = (PE_LoadU32(GA_B43_G30) + 20u) *
              (uint32_t)PE_LoadU16(resource);
    quotient = B43_S32(product) / 20;
    holder = PE_LoadU32(GA_B43_RECORD_PP);
    quotient_half = (uint16_t)quotient;
    PE_StoreU16(GA_B43_E06, quotient_half);
    if (holder == 0u)
        return;

    record = PE_LoadU32(holder);
    if (record == 0u)
        return;

    {
        int16_t old_min = (int16_t)PE_LoadU16(record + 0x0Cu);
        PE_StoreU16(record + 0x1Cu, quotient_half);
        if ((int16_t)quotient_half < old_min)
            PE_StoreU16(record + 0x0Cu, quotient_half);
    }
    {
        uint16_t current = PE_LoadU16(record + 0x1Cu);
        int16_t old_max = (int16_t)PE_LoadU16(record + 0x0Eu);
        if ((int16_t)current < old_max)
            PE_StoreU16(record + 0x0Eu, current);
    }
    {
        uint16_t current = PE_LoadU16(GA_B43_E06);
        uint16_t old = PE_LoadU16(GA_B43_E08);
        if (current < old)
            PE_StoreU16(GA_B43_E08, current);
    }

    for (i = 1u; i < 7u; i++) {
        pe_addr_t selected;
        uint32_t offset = 0u;

        PE_func_8005B91C_HostOut(
            (int32_t)i,
            (int32_t)(int16_t)PE_LoadU16(GA_B43_TARGET0 + i * 2u),
            &table_index_out, NULL);
        if (i < 6u)
            offset = PE_LoadU32(GA_B43_G34 + (i - 1u) * 4u);
        selected = func_8005DBAC(B43_S32((uint32_t)table_index_out + offset));

        switch (i) {
        case 1u:
            PE_StoreU16(record + 0x1Eu, PE_LoadU16(selected + 0x02u));
            break;
        case 2u:
            PE_StoreU16(record + 0x20u, PE_LoadU16(selected + 0x04u));
            break;
        case 3u:
            PE_StoreU32(record + 0x28u, PE_LoadU32(selected + 0x08u));
            PE_StoreU32(record + 0x30u, PE_LoadU32(selected + 0x0Cu));
            PE_StoreU32(record + 0x2Cu, PE_LoadU32(selected + 0x10u));
            break;
        case 4u:
            PE_StoreU16(record + 0x3Cu, PE_LoadU16(selected + 0x14u));
            PE_StoreU16(record + 0x3Eu, PE_LoadU16(selected + 0x16u));
            break;
        case 5u:
            PE_StoreU16(record + 0x22u, PE_LoadU8(selected + 0x06u));
            break;
        default:
        {
            uint8_t value = PE_LoadU8(selected + 0x07u);
            PE_StoreU16(record + 0x26u, value);
            func_80052F24(value);
            PE_StoreU16(record + 4u, (uint16_t)(PE_LoadU8(0x800C0E0Au) + 1u));
            return;
        }
        }
    }
}
