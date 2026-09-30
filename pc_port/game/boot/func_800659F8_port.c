/*
 * SEW3 — small field-script callees behind opcodes 23 / 72 / 7C / 7D /
 * 7E / 7F / B4 / B5 / BB / BC / D0.
 *
 * Native translations from the retail EXE (build/disc1.candidate.exe,
 * SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b), read with capstone
 * (local/live/mdis.py). Not matching-C claims except where noted.
 *
 * 0x800B1624 is the loaded map container. container+0x10 is the offset
 * of the 16-byte record table used by 659F8/65A60/65A9C (record +0 word:
 * bits 8.. = entry count, byte 0 = flags; record +0xC = offset of the
 * 2-byte entry list). container+0x14 is the offset of the 56-byte record
 * table used by 67678/676CC/67730 (byte 0 = flags, +0x1C/+0x1E = two
 * 8.8 halfwords).
 *
 * 659F8 (26w): for every entry of record a0, byte 1 = a1. returns 0.
 * 65A60 (15w): entry a1 of record a0, byte 1 = a2. returns 0.
 * 65A9C (14w): record a0 byte 0 |= (a1 & 0x30). returns 0.
 * 67678 (21w): 56-byte record a0 byte 0 bit 1 set/clear by a1. returns 0.
 * 676CC (25w): as 67678 for bit 2, then +0x1C = a2>>8, +0x1E = a3>>8.
 * 67730 (28w): as 67678 for bit 3, then +0x1C = (0x10000-a2)>>8,
 *              +0x1E = (0x10000-a3)>>8.
 * 3746C (31w): close the message record (0x800BCEA8, stride 56) whose id
 *              (+0x10, s16) equals (s16)a0 and whose byte 0 is set; at most
 *              one of the four slots, first match wins.
 * 37454 / 375B4 / 375C4: matching src/ C — four shorts at 0x8009CE98..9E;
 *              byte 0x8009CED0 = 1 / 0.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_CONTAINER 0x800B1624u
#define GA_MSG_POOL  0x800BCEA8u
#define MSG_STRIDE   56u

static pe_addr_t sew3_rec16(uint32_t index)
{
    pe_addr_t c = PE_LoadU32(GA_CONTAINER);
    return c + PE_LoadU32(c + 0x10u) + (index << 4);
}

static pe_addr_t sew3_rec56(uint32_t index)
{
    pe_addr_t c = PE_LoadU32(GA_CONTAINER);
    return c + PE_LoadU32(c + 0x14u) + index * 56u;
}

int func_800659F8(uint32_t index, uint32_t value)
{
    pe_addr_t rec = sew3_rec16(index);
    uint32_t count = PE_LoadU32(rec) >> 8;
    pe_addr_t entry = rec + PE_LoadU32(rec + 0xCu);
    uint32_t i;
    for (i = 0u; i < count; i++)
        PE_StoreU8(entry + 1u + i * 2u, (uint8_t)value);
    return 0;
}

int func_80065A60(uint32_t index, uint32_t entry, uint32_t value)
{
    pe_addr_t rec = sew3_rec16(index);
    PE_StoreU8(rec + PE_LoadU32(rec + 0xCu) + (entry << 1) + 1u, (uint8_t)value);
    return 0;
}

/* func_80065A9C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80065A9C_port.c (src/func_80065A9C.c); hand port retired (port3 switch-over A). */

static void sew3_rec56_bit(pe_addr_t rec, uint32_t on, uint8_t bit)
{
    uint8_t flags = PE_LoadU8(rec);
    PE_StoreU8(rec, on ? (uint8_t)(flags | bit) : (uint8_t)(flags & (uint8_t)~bit));
}

/* func_80067678: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80067678_port.c (src/func_80067678.c); hand port retired (audit batch, port3). */

/* func_800676CC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800676CC_port.c (src/func_800676CC.c); hand port retired (audit batch, port3). */

/* func_80067730: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80067730_port.c (src/func_80067730.c); hand port retired (audit batch, port3). */

/* func_800375B4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800375B4_port.c (src/func_800375B4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8003746C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8003746C_port.c (src/func_8003746C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80037454: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80037454_port.c (src/func_80037454.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800375C4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800375C4_port.c (src/func_800375C4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * 6F820 (51w 0x8006F820..0x8006F8EC): progress-flag record access.
 * index < 0x0B: rec = D_800942E4 + 2572*index; 0x0B..0x15: rec =
 * D_800942E8 + 268*(index-0x0B); otherwise return -13. kind = min(rec[1],
 * 0x55); if D_800942E0[kind] == 0 return -15. mode != 0: *out = rec[0]
 * (u8 as u32). mode == 0: if value < 6, rec[0] = value. Returns rec[0].
 */
int func_8006F820(uint32_t index, uint32_t mode, uint32_t value_or_out)
{
    pe_addr_t rec;
    uint32_t kind;
    if (index >= 0x16u)
        return -13;
    if (index < 0x0Bu)
        rec = PE_LoadU32(0x800942E4u) + 2572u * index;
    else
        rec = PE_LoadU32(0x800942E8u) + 268u * (index - 0x0Bu);
    kind = PE_LoadU8(rec + 1u);
    if (kind >= 0x55u)
        kind = 0x55u;
    if (PE_LoadU32(PE_LoadU32(0x800942E0u) + kind * 4u) == 0u)
        return -15;
    if (mode != 0u)
        PE_StoreU32(value_or_out, PE_LoadU8(rec));
    else if (value_or_out < 6u)
        PE_StoreU8(rec, (uint8_t)value_or_out);
    return (int)PE_LoadU8(rec);
}

/*
 * 1ACE0 (88w 0x8001ACE0..0x8001AE40): place an actor on walkmesh record
 * `index`. gp = 0x8009CD70: gp+0x468 = D_8009D1D8 (sloped plane table, 0
 * when the mesh is flat), gp+0x48C = D_8009D1FC (mesh; records at +0x1C),
 * gp+0x98 = D_8009CE08 (flat height pointer table).
 * Flat: rec = mesh+0x1C + 22*index; y = (s16)*D_8009CE08[rec[1]] << 16.
 * Sloped: rec = mesh+0x1C + 28*index; plane = D1D8 + 12*u16(rec+2);
 *   y = 3708C(rec+4 - 3708C(plane[0], x) - 3708C(plane[2], z), plane[1]).
 * Both: actor+0x1A4 = actor+0x1A8 = rec; actor+0x2C = y; then
 * 1C614(rec, (s16)actor+0x2A, (s16)actor+0x32); finally +0x40/+0x44/+0x48
 * = +0x28/+0x2C/+0x30.
 */
void func_8001ACE0(pe_addr_t actor, uint32_t index)
{
    pe_addr_t sloped = PE_LoadU32(0x8009D1D8u);
    pe_addr_t mesh = PE_LoadU32(0x8009D1FCu);
    pe_addr_t rec;
    uint32_t y;
    index &= 0xFFFFu;
    if (sloped == 0u) {
        rec = PE_LoadU32(mesh + 0x1Cu) + 22u * index;
        PE_StoreU32(actor + 0x1A4u, rec);
        PE_StoreU32(actor + 0x1A8u, rec);
        y = (uint32_t)(int32_t)(int16_t)PE_LoadU16(
                PE_LoadU32(PE_LoadU32(0x8009CE08u) + PE_LoadU8(rec + 1u) * 4u)) << 16;
    } else {
        pe_addr_t plane;
        uint32_t ax, cz;
        rec = PE_LoadU32(mesh + 0x1Cu) + 28u * index;
        PE_StoreU32(actor + 0x1A4u, rec);
        PE_StoreU32(actor + 0x1A8u, rec);
        plane = sloped + 12u * PE_LoadU16(rec + 2u);
        ax = func_8003708C(PE_LoadU32(plane), PE_LoadU32(actor + 0x28u));
        cz = func_8003708C(PE_LoadU32(plane + 8u), PE_LoadU32(actor + 0x30u));
        y = func_8003708C(PE_LoadU32(rec + 4u) - ax - cz, PE_LoadU32(plane + 4u));
    }
    PE_StoreU32(actor + 0x2Cu, y);
    (void)func_8001C614(rec, (int)(int16_t)PE_LoadU16(actor + 0x2Au),
                        (int)(int16_t)PE_LoadU16(actor + 0x32u));
    PE_StoreU32(actor + 0x40u, PE_LoadU32(actor + 0x28u));
    PE_StoreU32(actor + 0x44u, PE_LoadU32(actor + 0x2Cu));
    PE_StoreU32(actor + 0x48u, PE_LoadU32(actor + 0x30u));
}
