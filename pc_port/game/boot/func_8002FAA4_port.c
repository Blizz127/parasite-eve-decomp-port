/*
 * PE-CH1 — func_8002FAA4: opcode 0xB7 formation write (translated retail).
 *
 * Complete retail body (13 words / 0x34, exe 0x8002FAA4–0x8002FAD4,
 * file offset 0x202A4). Words dumped from SHA-1-exact
 * build/disc1.candidate.exe (452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 * yaml places this in [0x20210, asm] immediately after func_8002FA10.
 * This worktree has no era/asm split, so the leaf cannot yet be a
 * matching src/ unit. Native port follows the 2FA10 family.
 *
 * Opcode 0xB7 jump table D_800910A0[0xB7] @ 0x8009137C = wrapper
 * func_80018A48, which unpacks 5 script operands then jal this leaf
 * with a0 = *(D_8009D2F0). Sole jal of 2FAA4 is 0x80018A84. No jal
 * inside the leaf. Next function at 0x8002FAD8 starts with the same
 * andi-index prologue (not this rung).
 *
 * ROM (a0 = actor; *a0 = slot body from 0x6F):
 *
 *   i = a1 & 0xFF
 *   p16 = *a0 + i*16 + 0x1C
 *     sb 0,     0(p16)
 *     sb a2,    1(p16)
 *     sb a3,    2(p16)
 *     sb st+10, 3(p16)          lbu
 *     sh st+14, 0xC(p16)        lhu in jr delay
 *
 * Subset of 0x70: no +0xE/+0xF and no +0x7C quartet.
 * m0005i +0x2154: (3,0,6,7,1)
 * (docs/evidence/pe-btl0-field-battle-handoff/FIRST_DAY1_ENCOUNTER.md).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8002FAA4(pe_addr_t actor, unsigned int index,
                   unsigned int a2, unsigned int a3,
                   unsigned int b3, unsigned int half_c)
{
    pe_addr_t body;
    pe_addr_t p16;
    unsigned int i;

    i = index & 0xFFu;
    body = PE_LoadU32(actor);
    p16 = body + i * 16u + 0x1Cu;
    PE_StoreU8(p16 + 0u, 0u);
    PE_StoreU8(p16 + 1u, (uint8_t)a2);
    PE_StoreU8(p16 + 2u, (uint8_t)a3);
    PE_StoreU8(p16 + 3u, (uint8_t)b3);
    PE_StoreU16(p16 + 0xCu, (uint16_t)half_c);
}

/* SEW20: original 2FAD8 (8 words) and B0 wrapper198C4 (16 words).
 * Write both full-width timing fields in the indexed attack record. */
void func_8002FAD8(pe_addr_t actor,unsigned int index,uint32_t first,uint32_t last)
{
    pe_addr_t record=PE_LoadU32(actor)+(index&255u)*16u+0x1Cu;
    PE_StoreU32(record+4u,first);
    PE_StoreU32(record+8u,last);
}

int func_800198C4(pe_addr_t args)
{
    func_8002FAD8(PE_LoadU32(0x8009D2F0u),PE_LoadU8(PE_LoadU32(args)),
                 PE_LoadU32(PE_LoadU32(args+4u)),PE_LoadU32(PE_LoadU32(args+8u)));
    return 1;
}
