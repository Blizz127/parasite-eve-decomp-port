/* Independent StSetStream implementation from black-box vectors.
 * Evidence: docs/evidence/native-cdgen32-20261103/REPORT.md. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/cd.h"
#include "pe_plat/cd_stream.h"

void pe_plat_cd_set_stream(uint32_t mode, int32_t start, int32_t end,
                           uint32_t callback, uint32_t auxiliary)
{
    pe_plat_cd_stream_set_range(1u, (uint32_t)start, (uint32_t)end);
    PE_StoreU32(0x800C0DB8u, 0u);
    PE_StoreU32(0x800B0CC8u, callback);
    PE_StoreU32(0x800A801Cu, mode & 1u);
    PE_StoreU32(0x800B8620u, 0u);
    PE_StoreU32(0x800B6914u, 0u);
    PE_StoreU16(0x800A8018u, 0u);
    PE_StoreU32(0x800A5D54u, 0u);
    PE_StoreU32(0x800B0CCCu, auxiliary);
}
