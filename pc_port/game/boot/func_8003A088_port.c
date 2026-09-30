/*
 * PE-BTL6 — func_8003A088 mode-0 empty cut + live GTE walk.
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3a088_walk_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b and PE.IMG [428,434).
 *
 * Retail 392 words 0x8003A088..0x8003A6A8. dest+0x28 modes 1/3/4
 * (source copy / rigid attach / parent-relative) follow src/func_8003A088.c.
 * Walk at 0x8003A3B4: parent bytes from dest+0x20. POSE1 adds
 * branch save/restore markers and non-root bone offsets.
 * cop2 0x049E012 = MVMVA sf=1 mx=RT v=IR cv=None lm=0
 * cop2 0x0480012 = MVMVA sf=1 mx=RT v=V0 cv=TR lm=0
 * Integer MAC only (pe_gte). Does not andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include <string.h>
#include "game/decomp_hand/hand_lo_protos.h"

#define CMD_RTIR  0x049E012u
#define CMD_RTV0  0x0480012u

void func_8003A088_mode0_empty_cut(pe_addr_t dest)
{
    pe_addr_t obj;
    int16_t mode;
    int16_t count;

    if (dest == 0u)
        return;

    mode = (int16_t)PE_LoadU16(dest + 0x28u);
    if (mode == 1 || mode == 3 || mode == 4)
        return;

    obj = PE_LoadU32(dest + 0x00u);
    if (obj == 0u)
        return;
    count = (int16_t)PE_LoadU16(obj + 0x18u);
    if (count <= 0)
        return;
}

static void pe_3a088_store_row(pe_addr_t out, unsigned col)
{
    PE_StoreU16(out + col, (uint16_t)g_pe_gte.ir[0]);
    PE_StoreU16(out + 6u + col, (uint16_t)g_pe_gte.ir[1]);
    PE_StoreU16(out + 12u + col, (uint16_t)g_pe_gte.ir[2]);
}

void func_8003A088_mode0_walk_cut(pe_addr_t dest)
{
    pe_addr_t obj;
    pe_addr_t parents;
    pe_addr_t src_base;
    pe_addr_t out;
    pe_addr_t stream;
    int16_t mode;
    unsigned int count;
    struct { int16_t rt[3][3]; int32_t tr[3]; } stack[31];
    unsigned int depth = 0u;
    unsigned int i;

    if (dest == 0u)
        return;

    /* src/func_8003A088.c head: +0x28 selects how the model matrix at
     * +0x34 is seeded before the walk (+0x24 = animation source / parent
     * model, +0x2A = its bone index). */
    mode = (int16_t)PE_LoadU16(dest + 0x28u);
    if (mode == 1) {
        /* Copy the source's bone matrix into +0x34, then walk. */
        func_8003DBE4(dest, PE_LoadU32(dest + 0x24u),
                      (short)PE_LoadU16(dest + 0x2Au));
    } else if (mode == 3) {
        /* Rigid attachment (Aya's weapon, func_8006CC68 sets 3 / bone
         * 0x12): no walk.  +0x9C bit 0x400 copies the model matrix into
         * joint 1; otherwise the parent's bone matrix goes to joints 0/1. */
        if (PE_LoadU16(dest + 0x9Cu) & 0x400u) {
            unsigned int k;
            for (k = 0; k < 9u; k++)
                PE_StoreU16(PE_LoadU32(dest + 0x84u) + 0x20u + k * 2u,
                            PE_LoadU16(dest + 0x34u + k * 2u));
            for (k = 0; k < 3u; k++)
                PE_StoreU32(PE_LoadU32(dest + 0x84u) + 0x34u + k * 4u,
                            PE_LoadU32(dest + 0x48u + k * 4u));
        } else {
            func_8003DD08(dest, PE_LoadU32(dest + 0x24u),
                          (short)PE_LoadU16(dest + 0x2Au));
        }
        return;
    } else if (mode == 4) {
        /* Parent-relative: +0x34 rotation and +0x48 translation are
         * transformed by the parent's bone matrix in place (MVMVA
         * RT*IR per column; RT*V0+TR for the translation, stored from
         * IR1..3 as retail's swc2 $9..$11). */
        pe_addr_t pm = PE_LoadU32(PE_LoadU32(dest + 0x24u) + 0x84u)
            + (pe_addr_t)((int)(int16_t)PE_LoadU16(dest + 0x2Au) * 0x20);
        unsigned c;

        PE_GTE_LoadRT(pm);
        for (c = 0; c < 3u; c++) {
            pe_addr_t col = dest + 0x34u + c * 2u;
            PE_GTE_SetIR((int16_t)PE_LoadU16(col), (int16_t)PE_LoadU16(col + 6u),
                         (int16_t)PE_LoadU16(col + 12u));
            PE_GTE_MVMVA(CMD_RTIR);
            PE_StoreU16(col, (uint16_t)g_pe_gte.ir[0]);
            PE_StoreU16(col + 6u, (uint16_t)g_pe_gte.ir[1]);
            PE_StoreU16(col + 12u, (uint16_t)g_pe_gte.ir[2]);
        }
        PE_GTE_SetV0((int16_t)PE_LoadU16(dest + 0x48u),
                     (int16_t)PE_LoadU16(dest + 0x4Cu),
                     (int16_t)(PE_LoadU32(dest + 0x50u) & 0xFFFFu));
        PE_GTE_MVMVA(CMD_RTV0);
        PE_StoreU32(dest + 0x48u, (uint32_t)(int32_t)g_pe_gte.ir[0]);
        PE_StoreU32(dest + 0x4Cu, (uint32_t)(int32_t)g_pe_gte.ir[1]);
        PE_StoreU32(dest + 0x50u, (uint32_t)(int32_t)g_pe_gte.ir[2]);
    }

    obj = PE_LoadU32(dest + 0x00u);
    if (obj == 0u)
        return;
    count = PE_LoadU16(obj + 0x18u);
    if (count == 0u)
        return;

    PE_GTE_LoadRT(dest + 0x34u);
    parents = PE_LoadU32(dest + 0x20u);
    src_base = PE_LoadU32(dest + 0x58u);
    out = PE_LoadU32(dest + 0x84u);
    stream = PE_LoadU32(dest + 0x80u);

    for (i = 0; i < count; i++) {
        int parent = (int)(int8_t)PE_LoadU8(parents + (pe_addr_t)i);
        pe_addr_t src;
        unsigned col;

        /* Retail -1/-2 save and restore rotation/translation without
         * advancing the output matrix cursor. Host storage represents
         * the within-call stack at scratchpad +0xC. */
        if (parent == -1) {
            if (depth == 31u) {
                Bootstrap_ReturnVoid("func_8003A088_stack_overflow", "func_8003A088");
                return;
            }
            memcpy(stack[depth].rt, g_pe_gte.rt, sizeof(g_pe_gte.rt));
            memcpy(stack[depth].tr, g_pe_gte.tr, sizeof(g_pe_gte.tr));
            depth++;
            continue;
        }
        if (parent == -2) {
            if (depth == 0u) {
                Bootstrap_ReturnVoid("func_8003A088_stack_underflow", "func_8003A088");
                return;
            }
            depth--;
            memcpy(g_pe_gte.rt, stack[depth].rt, sizeof(g_pe_gte.rt));
            memcpy(g_pe_gte.tr, stack[depth].tr, sizeof(g_pe_gte.tr));
            continue;
        }

        src = src_base + (pe_addr_t)parent * 32u;
        for (col = 0; col < 3u; col++) {
            unsigned off = col * 2u;

            PE_GTE_SetIR((int16_t)PE_LoadU16(src + off),
                         (int16_t)PE_LoadU16(src + 6u + off),
                         (int16_t)PE_LoadU16(src + 12u + off));
            PE_GTE_MVMVA(CMD_RTIR);
            pe_3a088_store_row(out, off);
        }

        if (parent == 0) {
            /* ROM lhu src+20 / src+24 → VXY0; lwc2 VZ0 from src+28. */
            PE_GTE_SetV0((int16_t)PE_LoadU16(src + 20u),
                         (int16_t)PE_LoadU16(src + 24u),
                         (int16_t)(PE_LoadU32(src + 28u) & 0xFFFFu));
        } else {
            pe_addr_t rec = PE_LoadU32(dest + 0x04u) + (pe_addr_t)parent * 12u;
            PE_GTE_SetV0(0, 0, (int16_t)PE_LoadU16(rec + 8u));
        }
        PE_GTE_MVMVA(CMD_RTV0);
        PE_StoreU32(out + 20u, (uint32_t)g_pe_gte.mac[0]);
        PE_StoreU32(out + 24u, (uint32_t)g_pe_gte.mac[1]);
        PE_StoreU32(out + 28u, (uint32_t)g_pe_gte.mac[2]);
        PE_GTE_LoadRT(out);

        {
            pe_addr_t rec = PE_LoadU32(dest + 0x04u) + (pe_addr_t)parent * 12u;

            if (PE_LoadU8(rec + 4u) == 1u) {
                pe_addr_t slot = PE_LoadU32(dest + 0x18u)
                    + (pe_addr_t)parent * 16u;

                if ((int16_t)PE_LoadU16(slot + 0x0Eu) >= 0) {
                    uint32_t xy = PE_LoadU32(slot);
                    uint32_t z = PE_LoadU32(slot + 4u);

                    PE_GTE_SetV0((int16_t)(xy & 0xFFFFu),
                                 (int16_t)(xy >> 16),
                                 (int16_t)(z & 0xFFFFu));
                    PE_GTE_MVMVA(CMD_RTV0);
                    PE_StoreU16(stream + 0u, (uint16_t)g_pe_gte.ir[0]);
                    PE_StoreU16(stream + 2u, (uint16_t)g_pe_gte.ir[1]);
                    PE_StoreU16(stream + 4u, (uint16_t)g_pe_gte.ir[2]);
                    stream += 12u;
                }
            }
        }
        out += 32u;
    }
}

/* 3AC90..3AF14: view x joint transforms, then three-vertex RTPT batches.
 * The retail loop projects a final complete triple even for count%3 != 0. */
void func_8003AC90(pe_addr_t dest, pe_addr_t view)
{
    pe_addr_t obj = PE_LoadU32(dest);
    pe_addr_t joint = PE_LoadU32(dest + 0x84u);
    unsigned int bone, col, row;
    for (bone = 0; bone < PE_LoadU8(obj + 2u); bone++, joint += 32u) {
        int16_t rotation[3][3];
        int32_t translation[3];
        pe_addr_t rec = PE_LoadU32(dest + 4u) + bone * 12u;
        PE_GTE_LoadRT(view);
        for (col = 0; col < 3u; col++) {
            PE_GTE_SetIR((int16_t)PE_LoadU16(joint + col * 2u),
                         (int16_t)PE_LoadU16(joint + 6u + col * 2u),
                         (int16_t)PE_LoadU16(joint + 12u + col * 2u));
            PE_GTE_MVMVA(CMD_RTIR);
            for (row = 0; row < 3u; row++) rotation[row][col] = (int16_t)g_pe_gte.ir[row];
        }
        PE_GTE_SetV0((int16_t)PE_LoadU16(joint + 20u),
                     (int16_t)PE_LoadU16(joint + 24u),
                     (int16_t)PE_LoadU16(joint + 28u));
        PE_GTE_MVMVA(CMD_RTV0);
        for (row = 0; row < 3u; row++) translation[row] = g_pe_gte.mac[row];
        memcpy(g_pe_gte.rt, rotation, sizeof(rotation));
        memcpy(g_pe_gte.tr, translation, sizeof(translation));
        if (PE_LoadU8(rec + 4u) == 1u) {
            unsigned int start = PE_LoadU16(rec), count = PE_LoadU16(rec + 2u), n;
            pe_addr_t vertices = PE_LoadU32(dest + 8u) + start * 8u;
            for (n = 0; n < count; n += 3u, vertices += 24u) {
                uint32_t xy[3], z[3];
                PE_GTE_SetV0((int16_t)PE_LoadU16(vertices), (int16_t)PE_LoadU16(vertices + 2u), (int16_t)PE_LoadU16(vertices + 4u));
                PE_GTE_SetV1((int16_t)PE_LoadU16(vertices + 8u), (int16_t)PE_LoadU16(vertices + 10u), (int16_t)PE_LoadU16(vertices + 12u));
                PE_GTE_SetV2((int16_t)PE_LoadU16(vertices + 16u), (int16_t)PE_LoadU16(vertices + 18u), (int16_t)PE_LoadU16(vertices + 20u));
                PE_GTE_RTPT_coordinates(xy, z);
                for (row = 0; row < 3u; row++) {
                    PE_StoreU32(0x800B1644u + (start + n + row) * 4u, xy[row]);
                    PE_StoreU32(0x800A636Cu + (start + n + row) * 4u, z[row]);
                }
            }
        }
    }
}
