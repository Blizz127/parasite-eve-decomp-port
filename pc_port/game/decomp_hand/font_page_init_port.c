/* Complete native adapter of src/func_80038D74.c, retail 0x410 bytes.
 * EXE SHA1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 * Font's members are guest bytes/words at 80091A1C..80091A2B; the host
 * pointer size must never determine that record's layout. All CD/RNG/page
 * callees already have native implementations. Retail differential:
 * scripts/run_native_oracle_harnesses.sh (54 cases, callees mocked).
 */
#include "pe_port_compat.h"
#include "game_port.h"

int func_800389DC(int slot);

static int font_random_page(int limit, uint8_t *page)
{
    const int32_t random = (int32_t)func_80071A54();
    const int divisor = limit - 7;
    /* Retail's expanded signed remainder executes BREAK 7/6 here. This
     * is an invalid-data trap, not an unimplemented normal-content path.
     * Consume rand first, exactly as the original does before trapping. */
    if (divisor == 0 || (divisor == -1 && random == INT32_MIN)) {
        PE_Port_RequestStop(PE_PORT_STOP_GUEST_TRAP);
        return 0;
    }
    *page = (uint8_t)(random % divisor + 7);
    return 1;
}

static int font_find_class_two(void)
{
    pe_addr_t p = PE_LoadU32(0x80091A28u) + 1u;
    uint8_t selected = 0;
    int i;

    for (i = 0; i < PE_LoadU8(p + 2u); ++i) {
        if (PE_LoadU8(p + (uint32_t)i + 3u) == 2u) {
            selected = (uint8_t)i;
            i = PE_LoadU8(p + 2u);
        }
    }
    p += 27u;
    for (i = 0; i < PE_LoadU8(p); ++i)
        if (PE_LoadU8(p + (uint32_t)i + 1u) == selected) return i;
    return 255;
}

unsigned char func_80038D74(void)
{
    int status;
    unsigned count = 0, separator = 0;
    const unsigned stop_epoch = PE_Port_StopEpoch();

    for (;;) {
        do {
            status = func_8006E6A8(
                   (int32_t)(PE_LoadU32(0x800B0DD8u) + PE_LoadU16(0x80093176u)),
                   PE_LoadU32(0x800B0E6Cu),
                   (int)PE_LoadU16(0x80093178u) - (int)PE_LoadU16(0x80093176u));
            if (PE_Port_StopEpoch() != stop_epoch) return 0;
        } while (status == -1);
        do {
            status = func_8006E7E8();
            if (PE_Port_StopEpoch() != stop_epoch) return 0;
        } while (status != 0 && status != -1);
        if (status != -1) break;
    }
    PE_StoreU8(0x8009ECD8u, PE_LoadU8(PE_LoadU32(0x800B0E6Cu)));
    if (PE_LoadU8(0x80091A20u) == 0 && PE_LoadU32(0x80091A24u) == 0) {
        do {
            int32_t seed = (int32_t)func_80071A54() % 32767;
            PE_StoreU32(0x80091A24u, (uint32_t)seed);
        } while (PE_LoadU32(0x80091A24u) == 0);
    }
    func_80071A64(PE_LoadU32(0x80091A24u));
    for (unsigned i = 0; i < 69u; ++i) {
        const unsigned key = i + 2u;
        int limit;
        uint8_t page;

        if (key % 10u == 0) {
            PE_StoreU8(0x8009EE24u + i, (uint8_t)separator++);
            continue;
        }
        limit = key < 10u ? 104 : key < 20u ? 109 : 113;
        if ((int)PE_LoadU8(0x8009ECD8u) - 1 < limit)
            limit = (int)PE_LoadU8(0x8009ECD8u) - 1;
        if (!font_random_page(limit, &page)) return 0;
        PE_StoreU8(0x8009EE24u + i, page);
        if ((int)count < (int)PE_LoadU8(0x8009ECD8u) - 9) {
            int unique;
            do {
                unique = 1;
                for (unsigned j = 0; j < i; ++j) {
                    if (PE_LoadU8(0x8009EE24u + j) == PE_LoadU8(0x8009EE24u + i)) {
                        if (!font_random_page(limit, &page)) return 0;
                        PE_StoreU8(0x8009EE24u + i, page);
                        unique = 0;
                    }
                }
            } while (!unique);
        }
        ++count;
    }
    (void)func_800389DC(PE_LoadU8(0x8009EE24u));
    if (PE_Port_StopEpoch() != stop_epoch) return 0;
    PE_StoreU8(0x80091A1Cu, 1u);
    PE_StoreU8(0x80091A1Du, 2u);
    PE_StoreU8(0x80091A1Eu, PE_LoadU8(0x8009EE24u));
    PE_StoreU32(0x80091A28u, 0x8009ECD8u);
    PE_StoreU8(0x80091A1Fu, (uint8_t)font_find_class_two());
    {
        const pe_addr_t data = PE_LoadU32(0x80091A28u);
        return PE_LoadU8(data + PE_LoadU8(data + PE_LoadU8(0x80091A1Fu) + 29u) + 4u);
    }
}
