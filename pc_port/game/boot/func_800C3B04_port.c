/* Original 488-word sprite renderer 800C3B04 and the three COP2 wrappers
 * it calls (79244 RTPS, 79E14 Y-compose, 78C34 ApplyMatrix). Authority:
 * B3390.s / BE50C.s and the pinned Disc1 executable. Native translation
 * only; not matching C. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

/* hit_mul: unused after the GTE rebinds (docs/evidence/native-gte-cmd-20261031) */

/* func_80079244: now pe_plat_gte_cmd2.c (clean-room, docs/evidence/native-gte-cmd-20261031) */

/* func_80078E04: now pe_plat_gte_regs.c (clean-room, docs/evidence/native-gte-setters-20261030) */

#define GA_D_800963E8 0x800963E8u
#define GA_D_800963EC 0x800963ECu
#define GA_D_800963DC 0x800963DCu
#define PE_MATRIX_STACK_MAX 0x280u

/* pe_gte_store_rt_tr: unused after the GTE rebinds (docs/evidence/native-gte-cmd-20261031) */

/* func_80078A94: now pe_plat_gte_stack.c (clean-room, docs/evidence/native-gte-setters-20261030) */

/* func_80078B38: now pe_plat_gte_stack.c (clean-room, docs/evidence/native-gte-setters-20261030) */

/* 5 words 0x8006EC6C..0x8006EC80: sll 16 / sra 14 is (int16)a1 * 4. */
pe_addr_t func_8006EC6C(pe_addr_t base, int offset)
{
    int32_t delta = ((int32_t)offset) << 16;

    delta >>= 14;
    return base + PE_LoadU32(base + (uint32_t)delta);
}

/* func_80078E94: now pe_plat_gte_regs.c (clean-room, docs/evidence/native-gte-setters-20261030) */

/* func_80079274: now pe_plat_gte_cmd2.c (clean-room, docs/evidence/native-gte-cmd-20261031) */

/* func_80079E14: now pe_plat_gte_trig.c (clean-room, docs/evidence/native-gte-trig-20261030) */

/* func_80078C34: now pe_plat_gte_cmd.c (clean-room, docs/evidence/native-gte-cmd-20261031) */

static uint32_t hit_scale_channel(uint8_t channel, uint16_t brightness)
{
    uint32_t product = (uint32_t)channel * (uint32_t)brightness;
    if (0x7FFFu < product) product = 0x7FFFu;
    return product >> 7;
}

void func_800C3B04(pe_addr_t style)
{
    pe_addr_t scratch = 0x1F800000u, packet, ot;
    int32_t width, height, half_w, half_h, depth, bias;
    uint32_t bank, scale, denom, product;
    unsigned i;
    PE_StoreU32(0x800E284Cu, scratch);
    PE_StoreU8(scratch + 0x11u, (uint8_t)(PE_LoadU8(style + 0x24u) & 0xF0u));
    PE_StoreU8(scratch + 0x10u, (uint8_t)((uint32_t)(PE_LoadU8(style + 0x24u) - PE_LoadU8(scratch + 0x11u)) << 4u));
    PE_StoreU8(scratch + 0x12u, (uint8_t)(PE_LoadU8(style + 0x25u) << 4u));
    PE_StoreU8(scratch + 0x13u, (uint8_t)(PE_LoadU8(style + 0x25u) >> 4u));
    width = (int32_t)((uint32_t)PE_LoadU8(0x800F345Cu) * PE_LoadU32(style + 0x10u)) >> 12;
    height = (int32_t)((uint32_t)PE_LoadU8(0x800F345Du) * PE_LoadU32(style + 0x14u)) >> 12;
    if (PE_LoadU16(style + 0x28u) != 0x80u) {
        uint32_t last = 0;
        for (i = 0; i < 3; i++) {
            last = hit_scale_channel(PE_LoadU8(style + 0x20u + i), PE_LoadU16(style + 0x28u));
            PE_StoreU8(scratch + 0x80u + i, (uint8_t)last);
        }
        PE_StoreU32(scratch + 0x18u, last);
    } else {
        for (i = 0; i < 3; i++) PE_StoreU8(scratch + 0x80u + i, PE_LoadU8(style + 0x20u + i));
    }
    PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
    depth = func_80079244(style, scratch + 8u, scratch, scratch + 4u);
    PE_StoreU32(scratch + 0xCu, (uint32_t)depth);
    PE_StoreU16(scratch + 0x1Cu, PE_LoadU16(scratch + 8u));
    PE_StoreU16(scratch + 0x1Eu, PE_LoadU16(scratch + 0xAu));
    scale = PE_LoadU32(PE_LoadU32(0x800BCFA8u));
    denom = ((uint32_t)depth << 2u) + scale;
    if (!denom) denom = 1u;
    half_w = (int32_t)(((uint32_t)width << 5u) * scale / denom) >> 1;
    half_h = (int32_t)(((uint32_t)height << 5u) * scale / denom) >> 1;
    bank = PE_LoadU32(0x8009CDDCu);
    packet = PE_LoadU32(0x800B0E58u + bank * 4u) + PE_LoadU32(0x8009CDD8u);
    PE_StoreU8(packet + 3u, 9u);
    PE_StoreU8(packet + 7u, PE_LoadU8(0x800F337Au) ? 0x2Eu : 0x2Cu);
    if ((int16_t)PE_LoadU16(style + 0xCu)) {
        pe_addr_t matrix = scratch + 0x84u;
        PE_StoreU16(scratch + 0x40u, (uint16_t)(0 - half_w));
        PE_StoreU16(scratch + 0x42u, (uint16_t)(0 - half_h));
        PE_StoreU16(scratch + 0x48u, (uint16_t)half_w);
        PE_StoreU16(scratch + 0x4Au, (uint16_t)(0 - half_h));
        PE_StoreU16(scratch + 0x50u, (uint16_t)(0 - half_w));
        PE_StoreU16(scratch + 0x52u, (uint16_t)half_h);
        PE_StoreU16(scratch + 0x58u, (uint16_t)half_w);
        PE_StoreU16(scratch + 0x5Au, (uint16_t)half_h);
        PE_StoreU16(matrix, 0x1000u);
        PE_StoreU16(matrix + 8u, 0x1000u);
        PE_StoreU16(matrix + 16u, 0x1000u);
        PE_StoreU32(matrix + 20u, 0u);
        PE_StoreU32(matrix + 24u, 0u);
        PE_StoreU32(matrix + 28u, 0u);
        PE_StoreU16(matrix + 2u, 0u);
        PE_StoreU16(matrix + 4u, 0u);
        PE_StoreU16(matrix + 6u, 0u);
        PE_StoreU16(matrix + 10u, 0u);
        PE_StoreU16(matrix + 12u, 0u);
        PE_StoreU16(matrix + 14u, 0u);
        func_80079E14((int32_t)(int16_t)PE_LoadU16(style + 0xCu), matrix);
        func_80078C34(matrix, scratch + 0x40u, scratch + 0x60u);
        func_80078C34(matrix, scratch + 0x48u, scratch + 0x68u);
        func_80078C34(matrix, scratch + 0x50u, scratch + 0x70u);
        func_80078C34(matrix, scratch + 0x58u, scratch + 0x78u);
        for (i = 0; i < 4; i++) {
            PE_StoreU16(packet + 8u + i * 8u,
                        (uint16_t)(PE_LoadU16(scratch + 0x1Cu) + PE_LoadU16(scratch + 0x60u + i * 8u)));
            PE_StoreU16(packet + 10u + i * 8u,
                        (uint16_t)(PE_LoadU16(scratch + 0x1Eu) + PE_LoadU16(scratch + 0x62u + i * 8u)));
        }
    } else {
        PE_StoreU16(packet + 8u, (uint16_t)(PE_LoadU16(scratch + 0x1Cu) - half_w));
        PE_StoreU16(packet + 10u, (uint16_t)(PE_LoadU16(scratch + 0x1Eu) - half_h));
        PE_StoreU16(packet + 16u, (uint16_t)(PE_LoadU16(scratch + 0x1Cu) + half_w));
        PE_StoreU16(packet + 18u, (uint16_t)(PE_LoadU16(scratch + 0x1Eu) - half_h));
        PE_StoreU16(packet + 24u, (uint16_t)(PE_LoadU16(scratch + 0x1Cu) - half_w));
        PE_StoreU16(packet + 26u, (uint16_t)(PE_LoadU16(scratch + 0x1Eu) + half_h));
        PE_StoreU16(packet + 32u, (uint16_t)(PE_LoadU16(scratch + 0x1Cu) + half_w));
        PE_StoreU16(packet + 34u, (uint16_t)(PE_LoadU16(scratch + 0x1Eu) + half_h));
    }
    PE_StoreU8(packet + 4u, PE_LoadU8(scratch + 0x80u));
    PE_StoreU8(packet + 5u, PE_LoadU8(scratch + 0x81u));
    PE_StoreU8(packet + 6u, PE_LoadU8(scratch + 0x82u));
    PE_StoreU16(packet + 22u, PE_LoadU16(0x800E27ACu));
    PE_StoreU16(packet + 14u, (uint16_t)func_80077AA4((int32_t)(PE_LoadU16(0x800F341Cu) + PE_LoadU8(scratch + 0x12u)),
                                                      PE_LoadU16(0x800F341Eu) + PE_LoadU8(scratch + 0x13u)));
    PE_StoreU8(packet + 12u, (uint8_t)(PE_LoadU8(scratch + 0x10u) + 1u));
    PE_StoreU8(packet + 13u, (uint8_t)(PE_LoadU8(scratch + 0x11u) + 1u));
    PE_StoreU8(packet + 20u, (uint8_t)(PE_LoadU8(scratch + 0x10u) + PE_LoadU8(0x800F345Cu)));
    PE_StoreU8(packet + 21u, (uint8_t)(PE_LoadU8(scratch + 0x11u) + 1u));
    PE_StoreU8(packet + 28u, (uint8_t)(PE_LoadU8(scratch + 0x10u) + 1u));
    PE_StoreU8(packet + 29u, (uint8_t)(PE_LoadU8(scratch + 0x11u) + PE_LoadU8(0x800F345Du)));
    PE_StoreU8(packet + 36u, (uint8_t)(PE_LoadU8(scratch + 0x10u) + PE_LoadU8(0x800F345Cu)));
    PE_StoreU8(packet + 37u, (uint8_t)(PE_LoadU8(scratch + 0x11u) + PE_LoadU8(0x800F345Du)));
    bias = (int16_t)PE_LoadU16(style + 0x26u);
    ot = PE_LoadU32(0x800B0E38u + bank * 4u) + (uint32_t)(depth + bias) * 4u;
    product = (PE_LoadU32(packet) & 0xFF000000u) | (PE_LoadU32(ot) & 0xFFFFFFu);
    PE_StoreU32(packet, product);
    PE_StoreU32(0x8009CDD8u, PE_LoadU32(0x8009CDD8u) + 0x28u);
    PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (packet & 0xFFFFFFu));
}

/* func_800CDD0C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800CDD0C_port.c (src/func_800CDD0C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800CDE90: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800CDE90_port.c (src/func_800CDE90.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
