/*
 * TransMatrix (original libgte function at 0x80078C94, bound to
 * pe_plat_gte_translate_matrix through pe_plat/gte.h).
 *
 * Written clean-room from the psx-spx GTE documentation and the
 * hardware-captured / black-box vector sets gte60a-c
 * (docs/evidence/native-gte60-20261126); no SDK source, decompiled source,
 * tables or disassembly were read.
 *
 * Behaviour pinned by the gte60a "RTL" vectors: the three words of the
 * vector (offsets 0, 4, 8) are loaded first, then stored to the matrix
 * translation words (matrix + 0x14, 0x18, 0x1C); the matrix pointer is
 * returned; no GTE register is touched.  Loading all three before storing any
 * is observable when the vector overlaps the translation words.
 */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/gte.h"

uint32_t pe_plat_gte_translate_matrix(uint32_t matrix, uint32_t vector)
{
    const uint32_t x = PE_LoadU32(vector);
    const uint32_t y = PE_LoadU32(vector + 4u);
    const uint32_t z = PE_LoadU32(vector + 8u);
    PE_StoreU32(matrix + 0x14u, x);
    PE_StoreU32(matrix + 0x18u, y);
    PE_StoreU32(matrix + 0x1Cu, z);
    return matrix;
}
