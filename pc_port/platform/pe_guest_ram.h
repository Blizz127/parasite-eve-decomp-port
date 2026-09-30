/*
 * Phase 6D-S — Contiguous 2 MiB PS1 guest RAM model.
 *
 * Every PS1 address in [PE_RAM_BASE, PE_RAM_BASE + PE_RAM_SIZE) maps into
 * one host allocation.  Address arithmetic happens in pe_addr_t (uint32_t);
 * translation to a host pointer happens only at actual load/store sites.
 * No host pointer arithmetic across separate allocations.
 */
#ifndef PE_GUEST_RAM_H
#define PE_GUEST_RAM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Types ──────────────────────────────────────────────────────────── */

typedef uint32_t pe_addr_t;

#define PE_RAM_BASE  0x80000000u
#define PE_RAM_SIZE  0x00200000u    /* 2 MiB */
#define PE_RAM_END   (PE_RAM_BASE + PE_RAM_SIZE)
#define PE_SCRATCHPAD_BASE 0x1F800000u
#define PE_SCRATCHPAD_SIZE 0x400u

/* ── Lifecycle ──────────────────────────────────────────────────────── */

void  PE_RamInit(void);       /* one-time allocation, deterministic zero-fill */
uint64_t PE_RamGeneration(void); /* invalidates host caches after init/reset */
void  PE_RamReset(void);      /* zero-fill the existing allocation (allocates if needed) */
void  PE_RamDestroy(void);    /* free internal allocation  */

/* ── Validation ─────────────────────────────────────────────────────── */

bool  PE_AddressIsRam(pe_addr_t address);
bool  PE_RangeIsRam(pe_addr_t address, size_t size);
/* KUSEG/KSEG1 alias of main RAM -> KSEG0 address (identity otherwise). */
pe_addr_t PE_RamCanonical(pe_addr_t address, size_t size);
/* PS1 fast RAM is a separate 1 KiB authority at 1F800000/9F800000.
 * It is not main RAM or a DMA source; BF800000 is not a valid alias. */
bool  PE_RangeIsScratchpad(pe_addr_t address, size_t size);

/* Add a delta to a guest address with overflow checking.
 * Returns false and sets result=0 on overflow or out-of-range. */
bool  PE_AddAddress(pe_addr_t base, uint32_t delta, pe_addr_t *result);

/* ── Translation to host pointer ────────────────────────────────────── */

/* Accepts validated main RAM or scratchpad. Never hold the pointer across
 * store operations that might invalidate the translation. */
void       *PE_Translate(pe_addr_t address, size_t size);
const void *PE_TranslateConst(pe_addr_t address, size_t size);

/* Reverse translation, for decomp-derived leaves that return a pointer
 * (gen_decomp_ports.py pointer-return adapter).  A host pointer inside the
 * guest RAM allocation maps to its KSEG0 guest address, one inside the
 * scratchpad to 0x1F800000 + offset, and NULL to 0 (retail's null pointer).
 * Anything else (a host stack or heap address) has no guest meaning and
 * aborts loudly — it is never silently narrowed. */
pe_addr_t PE_HostToGuest(const volatile void *host);

/* Checked bulk fill over guest RAM.  The range is validated before any
 * host pointer is obtained; len is an exact unsigned 32-bit guest length. */
void PE_Fill(pe_addr_t address, uint32_t len, uint8_t value);

/* ── Little-endian guest loads / stores ─────────────────────────────── */

uint8_t  PE_LoadU8 (pe_addr_t address);
uint16_t PE_LoadU16(pe_addr_t address);
uint32_t PE_LoadU32(pe_addr_t address);

void PE_StoreU8 (pe_addr_t address, uint8_t  value);
void PE_StoreU16(pe_addr_t address, uint16_t value);
void PE_StoreU32(pe_addr_t address, uint32_t value);

/* ── Convenience macros for accessing named globals ─────────────────── */

/* PE_GUEST_PTR(addr)  — returns a host pointer for a known-valid range.
 * Only use when you have already validated the bounds. */
#define PE_GUEST_PTR(addr)  PE_Translate((addr), 1)

/* PE_GLOBAL(type, addr) — declares a typed accessor for a named PS1 global.
 * Example: #define D_800F34F8  PE_GUEST_PTR(0x800F34F8)
 * Then *(uint32_t*)PE_GUEST_PTR(0x800B0CD8) = 3; */

#ifdef __cplusplus
}
#endif

#endif /* PE_GUEST_RAM_H */
