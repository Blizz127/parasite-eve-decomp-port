/* Independent host adapter; frozen caller observations, not SDK source.
 * Retail behavior has a separate test-only caller draft. */
#include "pe_sdk.h"
#include "pe_disc.h"
static uint8_t cd_search_bcd(uint32_t value)
{
    return (uint8_t)((value / 10u) * 16u + value % 10u);
}
int pe_plat_cd_search_file_host(pe_addr_t out, const char *path)
{
    if (!path || *path != '\\') return 0;
    PE_Disc *disc = PE_Disc_GetActive();
    if (!disc) return 0;
    uint32_t lba = 0, size = 0;
    char name[16] = {0};
    if (!PE_Disc_FindFile(disc, path, &lba, &size, name, sizeof name)) return 0;
    uint32_t frame = lba + 150u;
    PE_StoreU8(out, cd_search_bcd(frame / 4500u));
    PE_StoreU8(out + 1u, cd_search_bcd((frame % 4500u) / 75u));
    PE_StoreU8(out + 2u, cd_search_bcd(frame % 75u));
    PE_StoreU8(out + 3u, 0);
    PE_StoreU32(out + 4u, size);
    for (uint32_t i = 0; i < sizeof name; ++i) PE_StoreU8(out + 8u + i, (uint8_t)name[i]);
    return 1;
}
