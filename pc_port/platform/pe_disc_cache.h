/*
 * Disc cache (host side): retail bytes that tests and tools need, read from
 * the USER'S disc image on demand — never committed.
 *
 * Resolution order for the boot EXE:
 *   1. <cache>/<current>/SLUS_006.62 written by tools/extract/disc_cache.py
 *      (cache = $PE_DISC_CACHE_DIR, else build/disc-cache relative to cwd),
 *      accepted only if its SHA-1 is a known supported EXE;
 *   2. the configured image: $PE_DISC1_BIN, else local/pe_disc1.path —
 *      opened read-only, identified by pe_disc_check, EXE read directly.
 * The bytes are loaded once per process and are immutable.
 */
#ifndef PE_DISC_CACHE_H
#define PE_DISC_CACHE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Path of the configured Disc 1 image into out (env / local file); 0 if
 * one is configured (existence not checked), -1 if none. */
int PE_DiscCache_ImagePath(char *out, size_t out_size);

/* Boot EXE (whole file, PS-X header at 0).  NULL when no disc/cache is
 * available or it fails validation; err (optional) gets the reason. */
const uint8_t *PE_DiscCache_Exe(uint32_t *size, char *err, size_t err_size);

/* Little-endian word of the retail EXE image at a guest address (any
 * KSEG mirror).  0 on success, -1 when the address is outside the EXE's
 * loaded text/data or no EXE is available. */
int PE_DiscCache_ExeWord(uint32_t addr, uint32_t *out);

/* Copy len bytes of the loaded EXE image at guest address addr. */
int PE_DiscCache_ExeRead(uint32_t addr, void *out, uint32_t len);

/* Raw file bytes by source: PE_DISC_SRC_EXE = the boot EXE file (offset 0 =
 * PS-X header), PE_DISC_SRC_PEIMG = PE.IMG (read from the configured image,
 * else <cache>/<current>/PE.IMG).  0 on success, -1 if unavailable/out of
 * range (err optional). */
enum { PE_DISC_SRC_EXE = 0, PE_DISC_SRC_PEIMG = 1 };
int PE_DiscCache_FileRead(int src, uint32_t off, void *out, uint32_t len,
                          char *err, size_t err_size);

#ifdef __cplusplus
}
#endif

#endif /* PE_DISC_CACHE_H */
