/*
 * PE-BTL6 — func_8003C5D8 (24 words, zero callees).
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3d050_remainder_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Exclusive 0x8003C5D8..0x8003C638. Live EE=13 site is
 * jal 3C5D8(dest, 50) at 0x8003D764; delay sb -1 at dest+0x8C is
 * the caller's store, not this leaf. a1==0 forces divisor 1.
 * Does not jal 794C4 and does not andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

void PE_Decomp_NoteStopDetail(const char *text);

/* func_8003C5D8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8003C5D8_port.c (src/func_8003C5D8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* 3B144..3B708: projected GT4/GT3/G4/G3 packet submission. NCLIP
 * uses signed screen coordinates and a wrapping 32-bit MAC0 result. */
void func_8003B144(pe_addr_t dest)
{
    static const uint8_t stride[4] = {52,40,36,28};
    pe_addr_t obj = PE_LoadU32(dest);
    pe_addr_t geometry = PE_LoadU32(dest + 0x10u);
    pe_addr_t packet_base = PE_LoadU32(dest + 0x54u);
    uint32_t bank = PE_LoadU32(0x8009CDDCu);
    pe_addr_t ot = PE_LoadU32(0x800B0E38u + bank * 4u);
    unsigned int kind, i, v;
    for (kind = 0; kind < 4u; kind++) {
        unsigned int count = PE_LoadU16(obj + 8u + kind * 2u);
        unsigned int vertices = (kind & 1u) ? 3u : 4u;
        for (i = 0; i < count; i++, geometry += 12u, packet_base += stride[kind] * 2u) {
            pe_addr_t packet = packet_base + bank * stride[kind];
            uint32_t xy[4], depth_sum = 0u;
            uint16_t index[4];
            int32_t area, bucket;
            for (v = 0; v < 3u; v++) {
                index[v] = PE_LoadU16(geometry + 4u + v * 2u);
                xy[v] = PE_LoadU32(0x800B1644u + index[v] * 4u);
            }
            /* Retail leaves SXY0-2, MAC0, FLAG and the scratch area live,
             * including for culled and far-rejected polygons. */
            PE_GTE_WriteData(12u, xy[0]);
            PE_GTE_WriteData(14u, xy[2]);
            PE_GTE_WriteData(13u, xy[1]);
            area = PE_GTE_NCLIP();
            PE_StoreU32(0x1F800000u, (uint32_t)area);
            if (area <= 0) {
                PE_StoreU32(packet, PE_LoadU32(packet) & 0xFF000000u);
                continue;
            }
            if (vertices == 4u) {
                index[3] = PE_LoadU16(geometry + 10u);
                xy[3] = PE_LoadU32(0x800B1644u + index[3] * 4u);
            }
            for (v = 0; v < vertices; v++)
                depth_sum += PE_LoadU32(0x800A636Cu + index[v] * 4u);
            bucket = vertices == 4u ? (int32_t)depth_sum >> 4 : ((int32_t)depth_sum / 3) >> 2;
            if (bucket < 4096) {
                pe_addr_t entry = ot + (uint32_t)bucket * 4u;
                PE_StoreU32(packet, (PE_LoadU32(packet) & 0xFF000000u) | (PE_LoadU32(entry) & 0xFFFFFFu));
                PE_StoreU32(entry, (PE_LoadU32(entry) & 0xFF000000u) | (packet & 0xFFFFFFu));
                for (v = 0; v < vertices; v++)
                    PE_StoreU32(packet + 8u + v * (kind < 2u ? 12u : 8u), xy[v]);
            }
        }
    }
}

/*
 * SEW13 — 3C2E0 (190 words 0x8003C2E0..0x8003C5D8): floor clip. Retail
 * words decoded in local/live/3c2e0.s + raw cop2 dump. For every skinned
 * bone (12-byte record at dest+4, byte +4 == 1): transform the bone origin
 * (16-byte entry at dest+0x18: SVECTOR + s16 radius at +6) by the bone
 * matrix (32-byte at dest+0x84) with MVMVA; skip the bone when the plane
 * `level` lies above origin+radius; otherwise project (x, level, z) through
 * the VIEW matrix with RTPS and, for each vertex of the bone whose
 * transformed y lies below the plane, replace its projected screen entry
 * (0x800B1644 + index*4) with that clip point and count it. When every
 * model vertex (u16 model+0x1A) was clipped, dest+0x9E = 0; otherwise
 * dest+0x9E = (dest+0x9C & 0x200) ? 0 : 1. The scratchpad
 * vector at 1F800000 and projected screen word at 1F800008 remain visible
 * in guest scratchpad, including when the bounds/vertex tests reject them.
 */
static void pe_gte_load_bone(pe_addr_t m)
{
    PE_GTE_LoadRT(m);
}

void func_8003C2E0(pe_addr_t dest, int level, pe_addr_t view)
{
    pe_addr_t model = PE_LoadU32(dest);
    unsigned int bones, bone, clipped = 0u;
    level = (int16_t)level;
    if (model == 0u || (int16_t)PE_LoadU16(dest + 0xBAu) == 0)
        return;
    bones = PE_LoadU8(model + 2u);
    for (bone = 0u; bone < bones; bone++) {
        pe_addr_t rec = PE_LoadU32(dest + 4u) + bone * 12u;
        pe_addr_t matrix = PE_LoadU32(dest + 0x84u) + bone * 32u;
        pe_addr_t entry = PE_LoadU32(dest + 0x18u) + bone * 16u;
        int32_t ox, oy, oz, radius;
        uint32_t clip_xy, clip_z;
        unsigned int start, count, i;
        pe_addr_t src;
        if (PE_LoadU8(rec + 4u) != 1u)
            continue;
        pe_gte_load_bone(matrix);
        PE_GTE_SetV0((int16_t)PE_LoadU16(entry), (int16_t)PE_LoadU16(entry + 2u),
                     (int16_t)PE_LoadU16(entry + 4u));
        PE_GTE_MVMVA(0x0480012u);
        ox = (int16_t)g_pe_gte.ir[0]; oy = (int16_t)g_pe_gte.ir[1]; oz = (int16_t)g_pe_gte.ir[2];
        PE_StoreU16(0x1F800000u, (uint16_t)ox);
        PE_StoreU16(0x1F800002u, (uint16_t)oy);
        PE_StoreU16(0x1F800004u, (uint16_t)oz);
        radius = (int16_t)PE_LoadU16(entry + 6u);
        if (!(level < oy)) {
            if (!(level < oy - radius) && !(level < oy + radius))
                continue;
        }
        PE_StoreU16(0x1F800002u, (uint16_t)level);
        PE_GTE_LoadRT(view);
        PE_GTE_SetV0((int16_t)ox, (int16_t)level, (int16_t)oz);
        PE_GTE_RTPS_coordinates(&clip_xy, &clip_z);
        PE_StoreU32(0x1F800008u, clip_xy);
        start = PE_LoadU16(rec);
        count = PE_LoadU16(rec + 2u);
        src = PE_LoadU32(dest + 8u) + start * 8u;
        pe_gte_load_bone(matrix);
        for (i = 0u; i < count; i++, src += 8u) {
            PE_GTE_SetV0((int16_t)PE_LoadU16(src), (int16_t)PE_LoadU16(src + 2u),
                         (int16_t)PE_LoadU16(src + 4u));
            PE_GTE_MVMVA(0x0480012u);
            PE_StoreU16(0x1F800000u, (uint16_t)g_pe_gte.ir[0]);
            PE_StoreU16(0x1F800002u, (uint16_t)g_pe_gte.ir[1]);
            PE_StoreU16(0x1F800004u, (uint16_t)g_pe_gte.ir[2]);
            if (level < (int16_t)g_pe_gte.ir[1]) {
                PE_StoreU32(0x800B1644u + (start + i) * 4u, clip_xy);
                clipped++;
            }
        }
    }
    if (clipped == PE_LoadU16(model + 0x1Au))
        PE_StoreU8(dest + 0x9Eu, 0u);
    else
        PE_StoreU8(dest + 0x9Eu, (PE_LoadU16(dest + 0x9Cu) & 0x200u) ? 0u : 1u);
}

int func_8003AF14(pe_addr_t dest, pe_addr_t scratch)
{
    uint16_t flags;

    if (dest == 0u)
        return 0;
    if (PE_LoadU32(dest) == 0u)
        return 0;
    if ((int16_t)PE_LoadU16(dest + 0xBAu) == 0)
        return 0;

    flags = PE_LoadU16(dest + 0x9Cu);
    if (flags & 0x800u) {
        func_8003CEF8(dest, 0); func_8003CCB0(dest, 0);
        PE_StoreU16(dest + 0x9Cu, PE_LoadU16(dest + 0x9Cu) & 0xF7FFu);
    }
    if (PE_LoadU16(dest + 0x9Cu) & 0x10u)
        func_8003C2E0(dest, (int16_t)PE_LoadU16(dest + 0x9Au), scratch);
    if (PE_LoadU8(dest + 0x9Eu) == 1u) {
        pe_addr_t packets = PE_LoadU32(dest + 0x54u);
        if (packets != 0u && PE_RangeIsRam(PE_RamCanonical(packets, 4u), 4u)) {
            func_8003B144(dest);
        } else {
            pe_addr_t model = PE_LoadU32(dest);
            /* An empty model has no packet accesses; a nonempty one must
             * not silently drop its draw when its packet buffer is absent. */
            if (PE_LoadU16(model + 8u) || PE_LoadU16(model + 10u) ||
                PE_LoadU16(model + 12u) || PE_LoadU16(model + 14u)) {
                PE_Decomp_NoteStopDetail("RENDER/REFUSED missing model packet storage");
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                return 0;
            }
        }
    }
    flags = PE_LoadU16(dest + 0x9Cu);
    if (flags & 0x20u) {
        func_8003B708(dest, (int16_t)PE_LoadU16(0x8009CDDCu));
        if (PE_Port_ShouldStop()) return 0;
        PE_StoreU8(dest + 0x9Fu, PE_LoadU32(0x8009CDDCu) == 0u);
        PE_StoreU16(dest + 0x9Cu, (PE_LoadU16(dest + 0x9Cu) & 0xFFDFu) | 0x40u);
    } else if (flags & 0x40u) {
        func_8003B708(dest, PE_LoadU8(dest + 0x9Fu));
        if (PE_Port_ShouldStop()) return 0;
        PE_StoreU16(dest + 0x9Cu, PE_LoadU16(dest + 0x9Cu) & 0xFFBFu);
    }
    flags = PE_LoadU16(dest + 0x9Cu);
    if (flags & 2u) {
        int completed = func_8003C818(dest);
        if (PE_Port_ShouldStop()) return 0;
        if ((int16_t)completed)
            PE_StoreU16(dest + 0x9Cu, (PE_LoadU16(dest + 0x9Cu) & 0xFFFDu) | 0x200u);
    } else if (flags & 4u) {
        int completed = func_8003C638(dest);
        if (PE_Port_ShouldStop()) return 0;
        if ((int16_t)completed)
            PE_StoreU16(dest + 0x9Cu, PE_LoadU16(dest + 0x9Cu) & 0xFFFBu);
    } else if (flags & 9u) {
        func_8003B97C(dest, 0x800BEA40u);
        if (!(flags & 8u)) func_8003BCE0(dest, 0, (int16_t)PE_LoadU16(0x8009CDDCu));
    }
    if (PE_LoadU16(dest + 0x9Cu) & 8u)
        func_8003C0B4(dest, (int16_t)PE_LoadU16(dest + 0x9Au),
            PE_LoadU8(dest + 0x97u), PE_LoadU8(dest + 0x98u), PE_LoadU8(dest + 0x99u));
    return 0;
}
