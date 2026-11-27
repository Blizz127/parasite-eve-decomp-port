/* Disc image access: image open, sector read and file lookup.
 * Declaration-only header (no function bodies): the port's host model API for disc image access.
 * Declarations were moved unchanged from the port's own host-model header platform/pe_disc.h
 * (commit e15e3ee7c); comments describe hardware behaviour as documented publicly (psx-spx)
 * in the project's own words.  No SDK source, tables or disassembly are reproduced here. */
#ifndef PE_PLAT_DISC_IMAGE_H
#define PE_PLAT_DISC_IMAGE_H

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
 * must be readable and declare only track 1 as MODE2/2352. If INDEX 01 is
 * present, it must be 00:00:00. Sector 0 sync/mode bytes are always verified. Returns NULL on failure and fills err. */
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
 * (the limits the guest's file search enforces).  On success returns true and fills
 * extent_lba / size; when name_out is non-NULL the stored file identifier
 * (NUL-terminated, truncated to name_size-1) is copied out. On failure,
 * all output arguments are left unchanged. */
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

#endif /* PE_PLAT_DISC_IMAGE_H */
