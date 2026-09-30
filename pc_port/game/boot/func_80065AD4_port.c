/*
 * PE-CH2 — func_80065AD4: 16-byte slot message fill (opcode 0x9A callee).
 *
 * 39 words 0x80065AD4..0x80065B70, SHA-256
 * d562b65b075e4532941fd6e0a6be1d5a87a8be522dd56fc4f2e3afa805b8c240.
 * Sole TEXT jal is opcode handler func_800193D8 @ 0x800193F8.
 *
 *   container = *(D_800B1624)
 *   slot      = container + *(container+0x10) + index*16
 *   vertical  = slot+0xA = 0
 *   slot+4    = lbu(slot+4) | (value << 16)   ; bits 8..15 are dropped
 *   slot[ slot[0xC] + count*2 + 1 ] = 0xFF    ; one byte, marking entry `count`
 *   slot+0    |= 6
 *   direction = (int16)slot+8
 *   if direction > 0 and count < value:    slot+8 = -direction
 *   if direction < 0 and value < count:    slot+8 = -direction
 *   (direction == 0 never flips)
 *
 * Note the fill is a single byte store, not a memset: `slot+0xC` is a byte
 * offset and the `count*2 + 1` term targets the high byte of 16-bit entry
 * `count`, i.e. it marks that entry 0xFF-flagged. There is no loop in retail.
 * The `slot+4` update is a byte read (retail `lbu 0x4($a3)`), so the stored
 * word is `low_byte | (value << 16)` — do not preserve bits 8..15.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800B1624 0x800B1624u

int func_80065AD4(unsigned int index, unsigned int value, unsigned int count)
{
    pe_addr_t container = PE_LoadU32(GA_D_800B1624);
    pe_addr_t slot = container + PE_LoadU32(container + 0x10u)
                   + (index << 4);
    int16_t direction;

    PE_StoreU16(slot + 0xAu, 0u);
    PE_StoreU32(slot + 4u,
                (uint32_t)PE_LoadU8(slot + 4u) | (value << 16));
    PE_StoreU8(slot + PE_LoadU32(slot + 0xCu) + count * 2u + 1u, 0xFFu);
    PE_StoreU8(slot, (uint8_t)(PE_LoadU8(slot) | 6u));

    direction = (int16_t)PE_LoadU16(slot + 8u);
    if (direction > 0) {
        if (count < value)
            PE_StoreU16(slot + 8u, (uint16_t)(-direction));
    } else if (direction < 0) {
        if (value < count)
            PE_StoreU16(slot + 8u, (uint16_t)(-direction));
    }
    return 0;
}
