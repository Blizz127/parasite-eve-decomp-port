/*
 * Phase 6E-A batch 3 — Read-only host access to the user-supplied Disc 1
 * image.
 *
 * Supported format (identified from the actual retail image and its cue
 * sheet, not guessed):
 *   - single-track BIN/CUE, TRACK 01 MODE2/2352
 *   - 2352-byte raw sectors; 2048 bytes of user data at raw offset +24
 *   - ISO9660 primary volume descriptor at user sector 16 ("CD001")
 * The image is opened read-only and is never embedded, copied, staged, or
 * committed.  All reads are bounds-checked against the actual image size;
 * missing, truncated, malformed, or wrong-disc inputs are rejected safely.
 *
 * LBA numbering throughout this API is the absolute ISO9660 user-sector
 * numbering (the same numbering carried by CdlFILE extents), i.e. raw BIN
 * byte offset = lba * 2352 + 24.
 */
#ifndef PE_DISC_H
#define PE_DISC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_DISC_RAW_SECTOR   2352u
#define PE_DISC_USER_SECTOR  2048u
#define PE_DISC_USER_OFFSET  24u    /* MODE2: sync(12) + header(4) + subheader(8) */

typedef struct PE_Disc PE_Disc;

/* ── Lifecycle ──────────────────────────────────────────────────────── */

/* Open and validate a BIN image.  When a sibling .cue exists it is parsed
 * and must declare MODE2/2352; the sync/mode bytes of sector 0 are always
 * verified empirically.  Returns NULL on any failure and fills err. */
PE_Disc *PE_Disc_Open(const char *bin_path, char *err, size_t err_size);

/* Test fixture: wrap an in-memory MODE2/2352 image (no cue check).  The
 * caller retains ownership of image; it must outlive the PE_Disc. */
PE_Disc *PE_Disc_OpenMemory(const uint8_t *image, size_t size);

void PE_Disc_Close(PE_Disc *d);

/* ── Geometry ───────────────────────────────────────────────────────── */

uint32_t PE_Disc_UserSectorCount(const PE_Disc *d);

/* ── Reads (all bounds-checked; false on out-of-range / short data) ─── */

bool PE_Disc_ReadRawSector(const PE_Disc *d, uint32_t lba,
                           uint8_t out[PE_DISC_RAW_SECTOR]);

bool PE_Disc_ReadUserSector(const PE_Disc *d, uint32_t lba,
                            uint8_t out[PE_DISC_USER_SECTOR]);

/* Read len bytes of user data starting at (lba, offset), crossing sector
 * boundaries.  offset must be < 2048.  Fails if the range runs past the
 * end of the image. */
bool PE_Disc_ReadUserData(const PE_Disc *d, uint32_t lba, uint32_t offset,
                          void *out, uint32_t len);

/* ── ISO9660 ────────────────────────────────────────────────────────── */

/* Verify the primary volume descriptor: type 1, "CD001", version 1 at
 * user sector 16. */
bool PE_Disc_VerifyPVD(const PE_Disc *d);

/* ISO9660 volume identifier (PVD +40), trimmed.  Empty on a fixture that
 * has no PVD label. */
bool PE_Disc_VolumeId(const PE_Disc *d, char *out, size_t out_size);

/* Which USA disc this image is.  1 = SLUS_006.62, 2 = SLUS_006.68,
 * 0 = neither boot file present (malformed or non-USA).  The two EXEs
 * are byte-identical; the archives and FMV tracks diverge. */
int PE_Disc_BootKind(const PE_Disc *d);

/* ISO9660 path lookup.  path must start with '\' and use '\' separators,
 * with the version suffix exactly as stored (e.g. "\PE.IMG;1",
 * "\FMV1\PEDISC01.IDF;1").  At most 8 components, each at most 31 chars
 * (the retail DsSearchFile limits).  On success returns true and fills
 * extent_lba / size; when name_out is non-NULL the stored file identifier
 * (NUL-terminated, truncated to name_size-1) is copied out. */
bool PE_Disc_FindFile(const PE_Disc *d, const char *path,
                      uint32_t *extent_lba, uint32_t *size,
                      char *name_out, size_t name_size);

/* ── Active-disc slot ───────────────────────────────────────────────── */

/* The disc providers (pe_libcd.c) consult this process-wide slot.  Tests
 * install a fixture; port_main installs the --disc-image image. */
void     PE_Disc_SetActive(PE_Disc *d);
PE_Disc *PE_Disc_GetActive(void);

#ifdef __cplusplus
}
#endif

#endif /* PE_DISC_H */
