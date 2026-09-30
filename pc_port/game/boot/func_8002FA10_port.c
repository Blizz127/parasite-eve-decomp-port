/*
 * PE-CH1 — func_8002FA10: opcode 0x70 formation write (translated retail).
 *
 * Complete retail body (37 words / 0x94, exe 0x8002FA10–0x8002FAA0,
 * file offset 0x20210). Words dumped from SHA-1-exact
 * build/disc1.candidate.exe (452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 * yaml places this at the head of [0x20210, asm] immediately after
 * matching func_8002F9CC. This worktree has no era/asm split, so the
 * leaf cannot yet be a matching src/ unit. Native port follows the
 * 2F7D8 / 2F9CC family.
 *
 * Opcode 0x70 jump table D_800910A0[0x70] @ 0x80091260 = wrapper
 * func_8001897C, which unpacks 11 script operands then jal this leaf
 * with a0 = *(D_8009D2F0). Sole jal of 2FA10 is 0x80018A30. No jal
 * inside the leaf. Next function is func_8002FAA4 (0xB7 twin).
 *
 * ROM (a0 = actor; *a0 = slot body from 0x6F):
 *
 *   i = a1 & 0xFF
 *   p16 = *a0 + i*16 + 0x1C
 *     sb 0,     0(p16)
 *     sb a2,    1(p16)
 *     sb a3,    2(p16)
 *     sb st+10, 3(p16)          lbu
 *     sh st+14, 0xC(p16)        lhu
 *     sb st+28, 0xE(p16)        lbu
 *     sb st+2C, 0xF(p16)        lbu
 *   p4 = *a0 + i*4
 *     sb st+18, 0x7C(p4)        lb then sb (low 8)
 *     sb st+1C, 0x7D(p4)
 *     sb st+20, 0x7E(p4)
 *     sb st+24, 0x7F(p4)        jr delay
 *
 * m0005i +0x21A0: (0,0,8,9,1,5,-1,-1,-1,3,15)
 * (docs/evidence/pe-btl0-field-battle-handoff/STATE_TRANSFER.md).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8002FA10(pe_addr_t actor, unsigned int index,
                   unsigned int a2, unsigned int a3,
                   unsigned int b3, unsigned int half_c,
                   int b7c, int b7d, int b7e, int b7f,
                   unsigned int b_e, unsigned int b_f)
{
    pe_addr_t body;
    pe_addr_t p16;
    pe_addr_t p4;
    unsigned int i;

    i = index & 0xFFu;
    body = PE_LoadU32(actor);
    p16 = body + i * 16u + 0x1Cu;
    PE_StoreU8(p16 + 0u, 0u);
    PE_StoreU8(p16 + 1u, (uint8_t)a2);
    PE_StoreU8(p16 + 2u, (uint8_t)a3);
    PE_StoreU8(p16 + 3u, (uint8_t)b3);
    PE_StoreU16(p16 + 0xCu, (uint16_t)half_c);
    PE_StoreU8(p16 + 0xEu, (uint8_t)b_e);
    PE_StoreU8(p16 + 0xFu, (uint8_t)b_f);

    p4 = body + i * 4u;
    PE_StoreU8(p4 + 0x7Cu, (uint8_t)b7c);
    PE_StoreU8(p4 + 0x7Du, (uint8_t)b7d);
    PE_StoreU8(p4 + 0x7Eu, (uint8_t)b7e);
    PE_StoreU8(p4 + 0x7Fu, (uint8_t)b7f);
}
