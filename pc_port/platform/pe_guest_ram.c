/*
 * Phase 6D-S — Contiguous 2 MiB PS1 guest RAM implementation.
 */
#include "pe_guest_ram.h"
#include "pe_mmio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Internal state ─────────────────────────────────────────────────── */

static uint8_t *g_pe_ram = NULL;
static uint64_t g_pe_ram_generation;
static uint8_t g_pe_scratchpad[PE_SCRATCHPAD_SIZE];

uint64_t PE_RamGeneration(void)
{
    return g_pe_ram_generation;
}

/* ── Lifecycle ──────────────────────────────────────────────────────── */

void PE_RamInit(void)
{
    if (g_pe_ram) return;  /* already initialized */
    ++g_pe_ram_generation;
    g_pe_ram = (uint8_t *)calloc(1, PE_RAM_SIZE);
    memset(g_pe_scratchpad,0,sizeof(g_pe_scratchpad));
    if (!g_pe_ram) {
        fprintf(stderr, "FATAL: PE_RamInit: cannot allocate %u bytes\n",
                PE_RAM_SIZE);
        abort();
    }
}

void PE_RamReset(void)
{
    if (!g_pe_ram) {
        PE_RamInit();
        return;
    }
    ++g_pe_ram_generation;
    memset(g_pe_ram, 0, PE_RAM_SIZE);
    memset(g_pe_scratchpad,0,sizeof(g_pe_scratchpad));
}

void PE_RamDestroy(void)
{
    free(g_pe_ram);
    g_pe_ram = NULL;
}

/* ── Validation ─────────────────────────────────────────────────────── */

bool PE_AddressIsRam(pe_addr_t address)
{
    return address >= PE_RAM_BASE && address < PE_RAM_END;
}

bool PE_RangeIsRam(pe_addr_t address, size_t size)
{
    if (size == 0) return PE_AddressIsRam(address);
    /* Check for overflow: address + size must not wrap */
    if (address > PE_RAM_END) return false;
    if ((uint64_t)address + (uint64_t)size > (uint64_t)PE_RAM_END) return false;
    return address >= PE_RAM_BASE;
}

bool PE_AddAddress(pe_addr_t base, uint32_t delta, pe_addr_t *result)
{
    if (!result) return false;
    /* Check for overflow */
    if ((uint64_t)base + (uint64_t)delta > 0xFFFFFFFFu) {
        *result = 0;
        return false;
    }
    pe_addr_t r = base + delta;
    if (!PE_AddressIsRam(r)) {
        *result = 0;
        return false;
    }
    *result = r;
    return true;
}

bool PE_RangeIsScratchpad(pe_addr_t address, size_t size)
{
    /* Only KUSEG and KSEG0 aliases, as on the PS1:
     * https://psx-spx.consoledev.net/memorymap/#scratchpad */
    uint32_t offset=(address&0x7FFFFFFFu)-PE_SCRATCHPAD_BASE;
    return offset<PE_SCRATCHPAD_SIZE && size<=PE_SCRATCHPAD_SIZE-offset;
}

/* ── Translation ────────────────────────────────────────────────────── */

/* The 2 MiB of main RAM is visible through three CPU segments: KUSEG
 * 0x00000000, KSEG0 0x80000000 and KSEG1 0xA0000000
 * (https://psx-spx.consoledev.net/memorymap/).  Retail data holds KUSEG
 * aliases (menu/text pointers, command-table handler words), so translation
 * folds a KUSEG/KSEG1 range that lies wholly inside the first 2 MiB of its
 * segment onto KSEG0.  Only the mapped 2 MiB is mirrored (retail programs
 * RAM_SIZE for 2 MiB; 0x00200000.. is not RAM), and a range may not cross the
 * segment's end.  PE_AddressIsRam/PE_RangeIsRam keep their KSEG0-only
 * meaning; callers that validate a guest pointer are unchanged. */
pe_addr_t PE_RamCanonical(pe_addr_t address, size_t size)
{
    uint32_t segment = address & 0xE0000000u;
    uint32_t offset = address & 0x1FFFFFFFu;
    if ((segment == 0x00000000u || segment == 0xA0000000u) &&
        offset < PE_RAM_SIZE && size <= (size_t)(PE_RAM_SIZE - offset))
        return PE_RAM_BASE | offset;
    return address;
}

/* I/O page 0x1F801000..0x1F801FFF (and its KSEG0/KSEG1 aliases): modelled
 * registers go to pe_mmio.h; everything else there stays a loud abort. */
#define PE_IS_IO_PAGE(a) ((((a) & 0x1FFFF000u) == 0x1F801000u) && \
                          (((a) & 0xE0000000u) == 0x00000000u || \
                           ((a) & 0xE0000000u) == 0x80000000u || \
                           ((a) & 0xE0000000u) == 0xA0000000u))

static inline uint8_t *translate_impl(pe_addr_t address, size_t size,
                                       const char *caller)
{
    if (!g_pe_ram) {
        fprintf(stderr, "FATAL: %s: guest RAM not initialized\n", caller);
        abort();
    }
    if (PE_RangeIsScratchpad(address,size))
        return g_pe_scratchpad+((address&0x7FFFFFFFu)-PE_SCRATCHPAD_BASE);
    if (PE_IS_IO_PAGE(address) && PE_MMIO_InShadowBlock(address, size))
        return (uint8_t *)PE_MMIO_Translate(address, size);
    {
        pe_addr_t canonical = PE_RamCanonical(address, size);
        if (!PE_RangeIsRam(canonical, size)) {
            fprintf(stderr, "FATAL: %s: invalid guest address 0x%08X size %zu\n",
                    caller, address, size);
            abort();
        }
        return g_pe_ram + (canonical - PE_RAM_BASE);
    }
}

void *PE_Translate(pe_addr_t address, size_t size)
{
    return translate_impl(address, size, "PE_Translate");
}

const void *PE_TranslateConst(pe_addr_t address, size_t size)
{
    return translate_impl(address, size, "PE_TranslateConst");
}

pe_addr_t PE_HostToGuest(const volatile void *host)
{
    const uint8_t *p = (const uint8_t *)(uintptr_t)host;

    if (p == NULL)
        return 0u;
    if (g_pe_ram && p >= g_pe_ram && p < g_pe_ram + PE_RAM_SIZE)
        return PE_RAM_BASE + (pe_addr_t)(p - g_pe_ram);
    if (p >= g_pe_scratchpad && p < g_pe_scratchpad + PE_SCRATCHPAD_SIZE)
        return PE_SCRATCHPAD_BASE + (pe_addr_t)(p - g_pe_scratchpad);
    fprintf(stderr, "FATAL: PE_HostToGuest: host pointer %p is not guest "
            "memory\n", (const void *)p);
    abort();
}

void PE_Fill(pe_addr_t address, uint32_t len, uint8_t value)
{
    /* Keep the guest length unsigned and validate the complete range before
     * touching memory.  A zero-length fill follows PE_RangeIsRam policy. */
    uint8_t *p = translate_impl(address, (size_t)len, "PE_Fill");
    memset(p, value, (size_t)len);
}

/* ── Little-endian loads ────────────────────────────────────────────── */

uint8_t PE_LoadU8(pe_addr_t address)
{
    if (PE_IS_IO_PAGE(address))   /* modelled or loud (pe_mmio.h) */
        return (uint8_t)PE_MMIO_Load(address, 1);
    uint8_t *p = translate_impl(address, 1, "PE_LoadU8");
    return *p;
}

uint16_t PE_LoadU16(pe_addr_t address)
{
    if (PE_IS_IO_PAGE(address))   /* modelled or loud (pe_mmio.h) */
        return (uint16_t)PE_MMIO_Load(address, 2);
    uint8_t *p = translate_impl(address, 2, "PE_LoadU16");
    /* Little-endian: first byte is LSB */
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

uint32_t PE_LoadU32(pe_addr_t address)
{
    if (PE_IS_IO_PAGE(address))   /* modelled or loud (pe_mmio.h) */
        return (uint32_t)PE_MMIO_Load(address, 4);
    uint8_t *p = translate_impl(address, 4, "PE_LoadU32");
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* ── Little-endian stores ───────────────────────────────────────────── */

void PE_StoreU8(pe_addr_t address, uint8_t value)
{
    if (PE_IS_IO_PAGE(address))   /* modelled or loud (pe_mmio.h) */ {
        PE_MMIO_Store(address, 1, value);
        return;
    }
    uint8_t *p = translate_impl(address, 1, "PE_StoreU8");
    *p = value;
}

void PE_StoreU16(pe_addr_t address, uint16_t value)
{
    if (PE_IS_IO_PAGE(address))   /* modelled or loud (pe_mmio.h) */ {
        PE_MMIO_Store(address, 2, value);
        return;
    }
    uint8_t *p = translate_impl(address, 2, "PE_StoreU16");
    p[0] = (uint8_t)(value & 0xFF);
    p[1] = (uint8_t)((value >> 8) & 0xFF);
}

void PE_StoreU32(pe_addr_t address, uint32_t value)
{
    if (PE_IS_IO_PAGE(address))   /* modelled or loud (pe_mmio.h) */ {
        PE_MMIO_Store(address, 4, value);
        return;
    }
    uint8_t *p = translate_impl(address, 4, "PE_StoreU32");
    p[0] = (uint8_t)(value & 0xFF);
    p[1] = (uint8_t)((value >> 8) & 0xFF);
    p[2] = (uint8_t)((value >> 16) & 0xFF);
    p[3] = (uint8_t)((value >> 24) & 0xFF);
}
