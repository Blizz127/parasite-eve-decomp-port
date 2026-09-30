/*
 * First-run disc check: identify the user's image against the known-disc
 * table (region x disc number) by boot file + boot EXE SHA-1.
 *
 * Owner decision 2026-09-28 ("run off the disc"): the port and its tests read
 * every retail byte from the user's own image.  This check turns a wrong,
 * damaged or unsupported image into one clear message instead of a crash deep
 * in boot.  Designed for multi-disc: the table carries every disc of every
 * region; callers say which disc numbers they accept.
 *
 * Keep the table in sync with tools/extract/disc_cache.py KNOWN_DISCS
 * (tools/extract/test_disc_cache.py cross-checks both).
 */
#ifndef PE_DISC_CHECK_H
#define PE_DISC_CHECK_H

#include "pe_disc.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PE_KnownDisc {
    const char *region;      /* "USA", "JPN" */
    int disc;                /* 1-based disc number */
    const char *serial;      /* "SLUS-00662" */
    const char *boot;        /* ISO9660 path of the boot EXE, "\\SLUS_006.62;1" */
    const char *exe_sha1;    /* NULL: recognised, no verified dump -> unsupported */
    const char *image_sha1;  /* whole BIN of the reference dump (informational:
                              * tools/extract/disc_cache.py reports a mismatch;
                              * the port does not hash 500 MB at boot) */
} PE_KnownDisc;

extern const PE_KnownDisc g_pe_known_discs[];
extern const unsigned g_pe_known_disc_count;

typedef struct PE_DiscIdentity {
    const PE_KnownDisc *known;   /* NULL on failure */
    char exe_sha1[41];
    uint32_t exe_size;
} PE_DiscIdentity;

/* Identify d.  Returns 0 when the image is a supported, hash-verified disc;
 * -1 otherwise with a user-facing message (what is wrong + remedy) in err.
 * Checks, in order: ISO9660 PVD, a known boot file, SYSTEM.CNF BOOT naming
 * that same file (serial), boot-EXE SHA-1, accepted disc number.
 * accept_mask: bit (n-1) set = disc n accepted (e.g. 1u for Disc 1 only);
 * a verified disc outside the mask is rejected with a "wrong disc" message. */
int PE_DiscCheck_Identify(const PE_Disc *d, unsigned accept_mask,
                          PE_DiscIdentity *out, char *err, size_t err_size);

/* Read the boot EXE file of a known disc into a malloc'd buffer (whole
 * file including the 0x800 PS-X header).  0 on success. */
int PE_DiscCheck_ReadFile(const PE_Disc *d, const char *iso_path,
                          uint8_t **data, uint32_t *size);

/* SHA-1 of a host buffer as 40 lowercase hex chars + NUL. */
void PE_DiscCheck_Sha1Hex(const void *data, size_t size, char out[41]);

#ifdef __cplusplus
}
#endif

#endif /* PE_DISC_CHECK_H */
