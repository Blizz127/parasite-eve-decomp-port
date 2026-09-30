/*
 * PE-CH1 — func_80017764: opcode 0x1C mailbox send (translated retail).
 *
 * Complete retail body (18 words / 0x48, exe 0x80017764–0x800177A8,
 * file offset 0x7F64). Words dumped from SHA-1-exact
 * build/disc1.candidate.exe (452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 * yaml places this inside [0x2A0C, asm] (2A0C.s, before matching
 * func_80017E9C at 0x869C). This worktree has no era/asm split, so the
 * leaf cannot yet be a matching src/ unit. Native port follows the
 * func_800653B8 / func_8006536C family.
 *
 * Jump table D_800910A0[0x1C] @ 0x80091110 = this function.
 * Sole jal of func_800653B8 (0x80017794); extra sw $zero in the jal
 * delay slot. Returns 1 (no ACK).
 *
 * ROM (a0 = script-arg triple of guest pointers):
 *
 *   lw   v0, 8(a0)              ptr payload
 *   lw   v1, 4(a0)              ptr dest id
 *   lbu  t0, 0(v0)              payload
 *   lbu  a1, 0(v1)              dest id
 *   lw   v0, 0(a0)              ptr dest type
 *   lui  v1, 0x800A
 *   lw   v1, -0x2D10(v1)        D_8009D2F0 current actor
 *   lhu  a2, 0(v0)              dest type
 *   lhu  a3, 0x24(v1)           actor serial
 *   addu a0, t0, zero           payload → a0
 *   jal  func_800653B8
 *   sw   zero, 0x10(sp)         extra = 0
 *   … teardown …
 *   addiu v0, zero, 1
 *   jr   ra
 *   addiu sp, 0x20
 *
 * BTL0 m0004i mailbox 3/4 is 0x1C (0,0,3) / (0,0,4)
 * (docs/evidence/pe-mbx1-task-state/).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D2F0 0x8009D2F0u

int func_80017764(pe_addr_t args)
{
    pe_addr_t p_payload;
    pe_addr_t p_id;
    pe_addr_t p_type;
    pe_addr_t actor;
    unsigned int payload;
    unsigned int dest_id;
    unsigned int dest_type;
    unsigned int sender;

    p_payload = PE_LoadU32(args + 8u);
    p_id = PE_LoadU32(args + 4u);
    p_type = PE_LoadU32(args + 0u);
    actor = PE_LoadU32(GA_D_8009D2F0);

    payload = PE_LoadU8(p_payload);
    dest_id = PE_LoadU8(p_id);
    dest_type = PE_LoadU16(p_type);
    sender = PE_LoadU16(actor + 0x24u);

    func_800653B8(payload, dest_id, dest_type, sender, 0u);
    return 1;
}
