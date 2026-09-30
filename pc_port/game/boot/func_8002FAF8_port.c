/*
 * PE-BTL51 — func_8002FAF8 clip-record tick
 * (translated retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 224 words 0x8002FAF8..0x8002FE78, SHA-256 67f58732…4673.
 * Sole jal from 0x64 / 184EC. JT D_80010A88 on actor+0x0E.
 *
 * ATK21 restores all animation calls and the 6DCE4 positional sound
 * wrapper. The command clip and frame crossings drive record states.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D1A0_BIT 2u



static unsigned int pe_2faf8_frame(pe_addr_t actor)
{
    uint32_t cur;
    uint32_t end;

    cur = PE_LoadU32(actor + 0x14u);
    end = PE_LoadU32(actor + 0x18u);
    if (cur < end)
        return (cur >> 16) + PE_LoadU8(actor + 0x0Fu) + 1u;
    return cur >> 16;
}

static void pe_2faf8_1a680(pe_addr_t actor, unsigned int command)
{
    func_8001A680_command_cut(actor,command&0xFFFFu);
}

/* func_8006DCE4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006DCE4_port.c (src/func_8006DCE4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static void pe_2faf8_activate(pe_addr_t slot, pe_addr_t actor,
                             unsigned int code)
{
    pe_addr_t rec;
    uint32_t word;

    rec = slot + (code << 4) + 0x1Cu;
    if (PE_LoadU8(rec) != 0u)
        return;
    PE_StoreU32(slot + 0x18u, rec);
    word = PE_LoadU32(slot);
    word = (word & 0xFF1FFFFFu) | ((code & 7u) << 21);
    word &= 0xBFFFFFFFu;
    word &= 0x7FFFFFFFu;
    PE_StoreU32(slot, word);
    pe_2faf8_1a680(actor, PE_LoadU8(rec + 2u));
    PE_StoreU32(actor + 0x1Cu, PE_LoadU32(rec + 4u));
}

static int pe_2faf8_tail(pe_addr_t slot, pe_addr_t actor,
                        unsigned int code, int s3)
{
    pe_addr_t cur;
    unsigned int state;

    cur = PE_LoadU32(slot + 0x18u);
    state = PE_LoadU8(cur);
    if (state != 4u) {
        if (PE_LoadU8(slot + (code << 4) + 0x1Cu) != 4u)
            return s3;
    }
    pe_2faf8_1a680(actor, (unsigned int)(int8_t)PE_LoadU8(slot + 6u));
    PE_StoreU32(actor + 0x1Cu, 0x10000u);
    PE_StoreU8(cur, 0u);
    PE_StoreU32(slot + 0x18u, 0u);
    return 1;
}

int func_8002FAF8(pe_addr_t actor, unsigned int code)
{
    pe_addr_t slot;
    pe_addr_t target;
    unsigned int cmd;
    unsigned int frame;
    int s3;
    int8_t kind;

    slot = PE_LoadU32(actor);
    if (slot == 0u || (D_8009D1A0 & GA_D_8009D1A0_BIT) == 0u)
        return 1;
    kind = (int8_t)PE_LoadU8(slot + 5u);
    target = actor;
    if (kind == 2 || kind == 4) {
        pe_addr_t alt;

        alt = PE_LoadU32(actor + 0x18Cu);
        if (alt != 0u)
            target = alt;
    }
    code &= 0xFFu;
    if (code >= 6u)
        return 0;

    s3 = 0;
    frame = pe_2faf8_frame(target);
    cmd = PE_LoadU8(target + 0x0Eu);
    if (cmd >= 0x10u)
        cmd = 2u;

    if (cmd == 0u || cmd == 1u)
        return pe_2faf8_tail(slot, target, code, s3);

    if (cmd >= 2u && cmd <= 5u) {
        pe_2faf8_activate(slot, target, code);
        return pe_2faf8_tail(slot, target, code, s3);
    }

    if ((cmd & 1u) == 0u) {
        unsigned int f;
        pe_addr_t rec;

        f = PE_LoadU8(target + 0x0Fu);
        if (f >= PE_LoadU16(target + 0x1Au) && f < frame) {
            rec = PE_LoadU32(slot + 0x18u);
            pe_2faf8_1a680(target, PE_LoadU8(rec + 3u));
            uint32_t volume=UINT32_MAX;
            (void)PE_SpatialSoundRequest(PE_LoadU32(0x800B0E64u),PE_LoadU16(slot+0xB2u),0u,
                (int16_t)PE_LoadU16(target+0x268u),(int16_t)PE_LoadU16(target+0x26Au),
                (int16_t)PE_LoadU16(target+0x26Cu),&volume);
            PE_M34StackClipSound(slot,target,volume);
            PE_StoreU32(target + 0x1Cu, PE_LoadU32(rec + 8u));
            PE_StoreU8(rec, 1u);
        }
        return pe_2faf8_tail(slot, target, code, s3);
    }

    {
        unsigned int f;
        uint32_t bits;

        f = PE_LoadU8(target + 0x0Fu);
        if (f >= PE_LoadU16(target + 0x1Au) && f < frame) {
            bits = PE_LoadU32(slot);
            if ((bits & 0x40000000u) == 0u) {
                pe_2faf8_1a680(target,
                               (unsigned int)(int8_t)PE_LoadU8(slot + 6u));
                PE_StoreU32(target + 0x1Cu, 0x10000u);
                if (code < 3u) {
                    s3 = 1;
                    PE_StoreU8(PE_LoadU32(slot + 0x18u), 4u);
                }
            }
        }
        if ((int8_t)PE_LoadU8(slot + 5u) == 0) {
            bits = PE_LoadU32(slot);
            if ((bits & 0x6000u) != 0u) {
                pe_addr_t rec;

                rec = PE_LoadU32(slot + 0x18u);
                if (PE_LoadU8(rec) < 2u)
                    PE_StoreU8(rec, 4u);
                s3 = 2;
            }
        }
        if (code < 3u) {
            pe_addr_t rec;
            unsigned int st;

            rec = PE_LoadU32(slot + 0x18u);
            st = PE_LoadU8(rec + 0x0Eu);
            if (st == 1u || st == 3u) {
                unsigned int ff;

                ff = PE_LoadU8(rec + 0x0Fu);
                if (ff < PE_LoadU16(target + 0x16u)
                    && ff >= PE_LoadU16(target + 0x1Au))
                    PE_StoreU8(rec, 3u);
            }
        }
        return pe_2faf8_tail(slot, target, code, s3);
    }
}
