/*
 * pe_plat storage -> in-house pe_disc (step P0 adapter).
 *
 * Uses the process-wide active-disc slot the existing providers read, so a
 * disc opened here is the disc the rest of the runtime sees.
 */
#include "pe_plat/storage.h"

#include "pe_disc.h"

static PE_Disc *plat_st_owned;     /* image this layer opened */
static PE_Disc *plat_st_previous;  /* active disc before it */

static void plat_st_install(PE_Disc *d)
{
    pe_plat_storage_disc_close();
    plat_st_previous = PE_Disc_GetActive();
    plat_st_owned = d;
    PE_Disc_SetActive(d);
}

int pe_plat_storage_disc_open(const char *image_path, char *err, size_t err_size)
{
    PE_Disc *d;
    if (!image_path) return 0;
    d = PE_Disc_Open(image_path, err, err_size);
    if (!d) return 0;
    plat_st_install(d);
    return 1;
}

int pe_plat_storage_disc_open_memory(const uint8_t *image, size_t size)
{
    PE_Disc *d = image ? PE_Disc_OpenMemory(image, size) : NULL;
    if (!d) return 0;
    plat_st_install(d);
    return 1;
}

void pe_plat_storage_disc_close(void)
{
    if (!plat_st_owned) return;
    if (PE_Disc_GetActive() == plat_st_owned) PE_Disc_SetActive(plat_st_previous);
    PE_Disc_Close(plat_st_owned);
    plat_st_owned = NULL;
    plat_st_previous = NULL;
}

int pe_plat_storage_disc_present(void) { return PE_Disc_GetActive() != NULL; }

int pe_plat_storage_disc_number(void)
{
    const PE_Disc *d = PE_Disc_GetActive();
    return d ? PE_Disc_BootKind(d) : 0;
}

int pe_plat_storage_volume_id(char *out, size_t out_size)
{
    const PE_Disc *d = PE_Disc_GetActive();
    return d && out && out_size && PE_Disc_VolumeId(d, out, out_size);
}

uint32_t pe_plat_storage_sector_count(void)
{
    const PE_Disc *d = PE_Disc_GetActive();
    return d ? PE_Disc_UserSectorCount(d) : 0u;
}

int pe_plat_storage_read_sectors(uint32_t lba, uint32_t count, void *out)
{
    const PE_Disc *d = PE_Disc_GetActive();
    uint8_t *p = (uint8_t *)out;
    uint32_t i;
    if (!d || !out) return 0;
    if (count > PE_Disc_UserSectorCount(d) || lba > PE_Disc_UserSectorCount(d) - count)
        return 0;
    for (i = 0; i < count; i++)
        if (!PE_Disc_ReadUserSector(d, lba + i, p + (size_t)i * PE_PLAT_SECTOR_USER))
            return 0;
    return 1;
}

int pe_plat_storage_read_raw_sector(uint32_t lba, uint8_t out[PE_PLAT_SECTOR_RAW])
{
    const PE_Disc *d = PE_Disc_GetActive();
    return d && out && PE_Disc_ReadRawSector(d, lba, out);
}

int pe_plat_storage_read(uint32_t lba, uint32_t offset, void *out, uint32_t len)
{
    const PE_Disc *d = PE_Disc_GetActive();
    if (!d || !out) return 0;
    lba += offset / PE_PLAT_SECTOR_USER;
    offset %= PE_PLAT_SECTOR_USER;
    return PE_Disc_ReadUserData(d, lba, offset, out, len);
}

int pe_plat_storage_find_file(const char *path, PePlatFileInfo *out)
{
    const PE_Disc *d = PE_Disc_GetActive();
    uint32_t lba = 0, size = 0;
    if (!d || !path || !PE_Disc_FindFile(d, path, &lba, &size, NULL, 0)) return 0;
    if (out) { out->lba = lba; out->size = size; }
    return 1;
}
