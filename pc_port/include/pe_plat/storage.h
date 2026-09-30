/*
 * pe_plat disc/storage interface (docs/ARCHITECTURE-PORT.md, step P0).
 *
 * Meaning-level: "the user's disc" as numbered 2048-byte user sectors (and
 * raw 2352-byte sectors for XA audio), file lookup by ISO9660 path, which
 * disc of the set is inserted, and its volume label.  The disc is always
 * the user's own image, opened read-only; nothing from it is shipped.
 *
 * Default backend: in-house pe_disc (platform/plat_storage.c).  The
 * asynchronous queue model (libds meaning) and memory-card saves arrive in
 * later steps (P4/P5) on this same header.
 *
 * Game code must include only pe_plat headers (never pe_disc.h).
 */
#ifndef PE_PLAT_STORAGE_H
#define PE_PLAT_STORAGE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_PLAT_SECTOR_USER 2048u
#define PE_PLAT_SECTOR_RAW  2352u

typedef struct {
    uint32_t lba;       /* first user sector */
    uint32_t size;      /* bytes */
} PePlatFileInfo;

/* Insert a disc image (BIN, MODE2/2352).  Returns 1 and makes it the active
 * disc, or 0 and fills err.  Closing an image this layer opened restores
 * whatever disc was active before. */
int  pe_plat_storage_disc_open(const char *image_path, char *err, size_t err_size);
/* Test fixtures: an in-memory image the caller keeps alive. */
int  pe_plat_storage_disc_open_memory(const uint8_t *image, size_t size);
void pe_plat_storage_disc_close(void);
int  pe_plat_storage_disc_present(void);

/* 1 or 2 for the USA set, 0 when unknown. */
int  pe_plat_storage_disc_number(void);
int  pe_plat_storage_volume_id(char *out, size_t out_size);
uint32_t pe_plat_storage_sector_count(void);

/* Reads (bounds-checked; 1 on success). */
int  pe_plat_storage_read_sectors(uint32_t lba, uint32_t count, void *out);
int  pe_plat_storage_read_raw_sector(uint32_t lba, uint8_t out[PE_PLAT_SECTOR_RAW]);
int  pe_plat_storage_read(uint32_t lba, uint32_t offset, void *out, uint32_t len);

/* Path uses '\' separators and the stored version suffix ("\PE.IMG;1"). */
int  pe_plat_storage_find_file(const char *path, PePlatFileInfo *out);

#ifdef __cplusplus
}
#endif

#endif /* PE_PLAT_STORAGE_H */
