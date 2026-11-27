/* Address leaves 0x80071944, 0x80071964, 0x80071994 (object chunk locators) and
 * 0x80077A64, 0x80077AA4 (texture-page and CLUT words), written clean-room from
 * the black-box vector sets gpu2c (docs/evidence/native-gpu2-20261126), the
 * port's host GPU/DMA model API and psx-spx; no SDK source, decompiled source,
 * tables or disassembly were read.
 *
 * Derivation (every line is traced to the vectors, see AUTHOR-REPORT.md):
 *  - The three locators read one flag word at object+4 and, only when its bit 3
 *    is set, the size word at object+8.  0x80071944 returns object+0xC when bit 3
 *    is set and 0 otherwise (it never reads +8).  0x80071964 returns object+0xC,
 *    plus the +8 word when bit 3 is set.  0x80071994 does the same from
 *    object+0x14.  Sums wrap at 32 bits.
 *  - Texture page (psx-spx GP0(E1h) / polygon texpage attribute): bits 0..3
 *    = (x & 0x3FF) / 64, bit 4 = (y & 0x100) != 0, bits 5..6 = abr & 3,
 *    bits 7..8 = tp & 3, bit 11 = (y & 0x200) != 0.
 *  - CLUT word (psx-spx): bits 0..5 = (x / 16) & 0x3F, bits 6..15 = y & 0x3FF. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/gpuaddr.h"

#define OBJ_FLAGS      4u
#define OBJ_SIZE       8u
#define FLAG_HAS_SIZE  0x8u
#define OPTIONAL_BASE  0xCu
#define PIXEL_CHUNK_BASE 0xCu
#define PIXEL_DATA_BASE  0x14u

uint32_t pe_plat_gpuaddr_optional_chunk(uint32_t object)
{
    uint32_t flags = PE_LoadU32(object + OBJ_FLAGS);
    return (flags & FLAG_HAS_SIZE) ? object + OPTIONAL_BASE : 0u;
}

static uint32_t chunk_after(uint32_t object, uint32_t base)
{
    uint32_t flags = PE_LoadU32(object + OBJ_FLAGS);
    uint32_t at = object + base;
    if (flags & FLAG_HAS_SIZE) at += PE_LoadU32(object + OBJ_SIZE);
    return at;
}

uint32_t pe_plat_gpuaddr_pixel_chunk(uint32_t object) { return chunk_after(object, PIXEL_CHUNK_BASE); }
uint32_t pe_plat_gpuaddr_pixel_data(uint32_t object) { return chunk_after(object, PIXEL_DATA_BASE); }

int pe_plat_gpuaddr_texture_page(int tp, int abr, int x, int y)
{
    uint32_t word = (((uint32_t)x & 0x3FFu) >> 6) | (((uint32_t)y & 0x100u) >> 4) |
                    (((uint32_t)abr & 3u) << 5) | (((uint32_t)tp & 3u) << 7) |
                    (((uint32_t)y & 0x200u) << 2);
    return (int)word;
}

int pe_plat_gpuaddr_clut(int x, int y)
{
    return (int)((((uint32_t)y & 0x3FFu) << 6) | (((uint32_t)x >> 4) & 0x3Fu));
}
