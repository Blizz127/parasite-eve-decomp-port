/* SPU reverb attribute update (guest 0x8008D140), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * reverb), with no SDK source.  Evidence: docs/evidence/native-spu-reverb-20261031/. */
#include "pe_plat/audio.h"
#include "pe_guest_ram.h"

/* Pointer global holding the SPU register block base (0x1F801C00); the
 * original reloads it before every register store. */
#define SPU_BASE_PTR        0x8009B3FCu

/* Attribute record: a mask word, then one halfword per mask bit. */
#define ATTR_MASK           0x00u /* word: bit n selects field n */
#define ATTR_FIELDS         0x04u /* first halfword field */
#define ATTR_FIELD_SIZE     2u
#define ATTR_FIELD_COUNT    32u

/* Field n is copied unchanged to the SPU register block at this base plus
 * 2 * n (reverb work-area registers 0x1F801DC0..0x1F801DFE). */
#define SPU_REVERB_REGS     0x1C0u
#define SPU_REG_SIZE        2u

/* For each mask bit set, copy the matching halfword field to its reverb
 * register, in bit order.  The mask word is read once; a field is read only
 * when its bit is set.  A zero mask selects every field (as all ones does). */
void pe_plat_spu_set_reverb_attr(uint32_t attr)
{
    uint32_t mask = PE_LoadU32(attr + ATTR_MASK);
    uint32_t n;

    if (mask == 0u)
        mask = 0xFFFFFFFFu;
    for (n = 0u; n < ATTR_FIELD_COUNT; n++) {
        if ((mask & (1u << n)) == 0u)
            continue;
        {
            uint32_t spu = PE_LoadU32(SPU_BASE_PTR);
            uint16_t v = PE_LoadU16(attr + ATTR_FIELDS + n * ATTR_FIELD_SIZE);

            PE_StoreU16(spu + SPU_REVERB_REGS + n * SPU_REG_SIZE, v);
        }
    }
}
