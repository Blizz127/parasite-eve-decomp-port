/*
 * PE-CH1 — func_80030220: opcode 0x5A slot tagged setter (translated retail).
 *
 * Complete retail body (197 words / 0x314, exe 0x80030220–0x80030530,
 * file offset 0x20A20). Words dumped from SHA-1-exact
 * build/disc1.candidate.exe (452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 * yaml [0x20210, asm]. This worktree has no era/asm split, so the leaf
 * cannot yet be a matching src/ unit.
 *
 * Opcode 0x5A wrapper func_80018164 jals here when
 * lbu(*(D_8009D2F0)+0x0C)!=0 (after 0x6F the slot path is live).
 * a0=actor, a1=tag, a2=value. Jal sites: 0x800181B4 (0x5A) and
 * 0x80018288 (0xCE).
 *
 * ROM:
 *   slot = *actor
 *   idx  = (tag & 0xFF) - 40
 *   if idx >= 85: return
 *   jr D_80010C90[idx]
 *
 * m0005i first-play: 40=0 sh+0xE; 41=1 sb+4; 42=2 sb+5;
 * 50=1333 sh+0xB0; 51=1332 sh+0xB2; 52=1334 sh+0xB4.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define TAG_BASE 40u
#define TAG_COUNT 85u

static void patch_bits(pe_addr_t addr, unsigned int value,
                       unsigned int val_mask, unsigned int shift,
                       unsigned int field_mask)
{
    unsigned int w;

    w = PE_LoadU32(addr);
    w = (w & ~field_mask) | ((value & val_mask) << shift);
    PE_StoreU32(addr, w);
}

void func_80030220(pe_addr_t actor, unsigned int tag, unsigned int value)
{
    pe_addr_t slot;
    unsigned int idx;

    slot = PE_LoadU32(actor);
    idx = (tag & 0xFFu) - TAG_BASE;
    if (idx >= TAG_COUNT) {
        return;
    }

    switch (idx + TAG_BASE) {
        case 40: PE_StoreU16(slot + 0x0Eu, (uint16_t)value); break;
        case 41: PE_StoreU8(slot + 0x04u, (uint8_t)value); break;
        case 42: PE_StoreU8(slot + 0x05u, (uint8_t)value); break;
        case 43: PE_StoreU16(slot + 0x0Cu, (uint16_t)value); break;
        case 44: PE_StoreU32(slot + 0x10u, value); break;
        case 45: PE_StoreU32(slot + 0x14u, value); break;
        case 46: PE_StoreU8(slot + 0x07u, (uint8_t)value); break;
        case 47: PE_StoreU8(slot + 0x06u, (uint8_t)value); break;
        case 50: PE_StoreU16(slot + 0xB0u, (uint16_t)value); break;
        case 51: PE_StoreU16(slot + 0xB2u, (uint16_t)value); break;
        case 52: PE_StoreU16(slot + 0xB4u, (uint16_t)value); break;
        case 53: PE_StoreU16(slot + 0xB6u, (uint16_t)value); break;
        case 54: PE_StoreU16(slot + 0xB8u, (uint16_t)value); break;
        case 55: PE_StoreU16(slot + 0xBAu, (uint16_t)value); break;
        case 60: PE_StoreU32(slot + 0x88u, value); break;
        case 61: PE_StoreU16(slot + 0x8Cu, (uint16_t)value); break;
        case 62: PE_StoreU16(slot + 0x8Eu, (uint16_t)value); break;
        case 63: PE_StoreU8(slot + 0x90u, (uint8_t)value); break;
        case 64: PE_StoreU8(slot + 0x91u, (uint8_t)value); break;
        case 65: PE_StoreU8(slot + 0x92u, (uint8_t)value); break;
        case 66: PE_StoreU8(slot + 0x93u, (uint8_t)value); break;
        case 67: PE_StoreU8(slot + 0x94u, (uint8_t)value); break;
        case 68: PE_StoreU8(slot + 0x95u, (uint8_t)value); break;
        case 69: PE_StoreU16(slot + 0x96u, (uint16_t)value); break;
        case 74:
            patch_bits(slot, value, 1u, 4u, 0x10u);
            break;
        case 80:
            PE_StoreU8(slot + 0xA4u, (uint8_t)value);
            if (value == 1u) {
                PE_StoreU16(slot + 0xA6u, 400u);
            } else if (value == 2u) {
                PE_StoreU16(slot + 0xA6u, 70u);
            } else if (value == 3u) {
                PE_StoreU16(slot + 0xA6u, 20u);
            }
            break;
        case 81:
            patch_bits(slot, value, 1u, 20u, 0x100000u);
            break;
        case 90: PE_StoreU16(slot + 0x98u, (uint16_t)value); break;
        case 91: PE_StoreU16(slot + 0x9Au, (uint16_t)value); break;
        case 92: PE_StoreU8(slot + 0x9Eu, (uint8_t)value); break;
        case 93: PE_StoreU8(slot + 0x9Fu, (uint8_t)value); break;
        case 94: PE_StoreU8(slot + 0xAEu, (uint8_t)value); break;
        case 95: PE_StoreU8(slot + 0xAFu, (uint8_t)value); break;
        case 96: PE_StoreU8(slot + 0xBCu, (uint8_t)value); break;
        case 100: patch_bits(slot + 0xCCu, value, 3u, 0u, 0x3u); break;
        case 101: patch_bits(slot + 0xCCu, value, 3u, 2u, 0xCu); break;
        case 102: patch_bits(slot + 0xCCu, value, 3u, 4u, 0x30u); break;
        case 103: patch_bits(slot + 0xCCu, value, 3u, 6u, 0xC0u); break;
        case 104: patch_bits(slot + 0xCCu, value, 3u, 8u, 0x300u); break;
        case 105: patch_bits(slot + 0xCCu, value, 3u, 10u, 0xC00u); break;
        case 106: patch_bits(slot + 0xCCu, value, 3u, 12u, 0x3000u); break;
        case 107: patch_bits(slot + 0xCCu, value, 3u, 14u, 0xC000u); break;
        case 108: patch_bits(slot + 0xCCu, value, 3u, 16u, 0x30000u); break;
        case 109: patch_bits(slot + 0xCCu, value, 1u, 18u, 0x40000u); break;
        case 110: patch_bits(slot + 0xCCu, value, 0x1Fu, 19u, 0xF80000u); break;
        case 120: PE_StoreU16(slot + 0xD0u, (uint16_t)value); break;
        case 121: PE_StoreU8(slot + 0xD6u, (uint8_t)value); break;
        case 122: PE_StoreU8(slot + 0xD7u, (uint8_t)value); break;
        case 123: PE_StoreU16(slot + 0xA0u, (uint16_t)value); break;
        case 124: PE_StoreU16(slot + 0xA2u, (uint16_t)value); break;
        default:
            break;
    }
}
