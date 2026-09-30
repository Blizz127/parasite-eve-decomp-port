/*
 * Phase 6E-B2 — Retail executable image backing for guest RAM.
 *
 * On retail, the BIOS reads SYSTEM.CNF, loads the named PS-X EXE into RAM
 * at its taddr, and jumps to it.  Retail code is therefore allowed to READ
 * its own text/data as bytes — and the game actually does: the
 * func_80070D6C lagged-Fibonacci RNG read cursor cycles through 14 words
 * of code below its table (0x80070DCC..0x80070E00, proven from raw MIPS by
 * pc_port/tools/rng_oracle.py against the retail exe).  Faithful RNG output
 * requires those immutable bytes in guest RAM.
 *
 * This module is that general mechanism (not an RNG-specific constant
 * stash): given the active read-only disc image, it parses SYSTEM.CNF for
 * the BOOT= executable, validates the PS-X EXE header, and loads tsize
 * bytes at taddr through bounds-checked guest-RAM stores.  Missing,
 * malformed, truncated, or out-of-range images are rejected safely; the
 * bytes always originate from the user-supplied disc — never embedded.
 */
#ifndef PE_GUEST_IMAGE_H
#define PE_GUEST_IMAGE_H

#include "pe_disc.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Load the disc's boot executable into guest RAM at its PS-X EXE taddr.
 * Returns 0 on success; -1 on any failure (err filled). */
int PE_GuestImage_LoadExe(const PE_Disc *disc, char *err, size_t err_size);

#ifdef __cplusplus
}
#endif

#endif /* PE_GUEST_IMAGE_H */
