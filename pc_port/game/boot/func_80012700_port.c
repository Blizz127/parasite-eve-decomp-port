/*
 * PE-CH1 — func_80012700: task-block spawn (translated retail).
 *
 * Complete retail body (29 words / 0x74, exe 0x80012700–0x80012770,
 * file offset 0x2F00). Words dumped from SHA-1-exact
 * build/disc1.candidate.exe (452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 * yaml places this inside [0x2A0C, asm]. This worktree has no era/asm
 * split, so the leaf cannot yet be a matching src/ unit. Native port
 * follows the 653B8 / 17764 / 65400 family and replaces the 65400 hook.
 *
 * Seven jal sites; mailbox path is 0x80065484 and 0x8006553C (a1=0).
 *
 * ROM (gp = 0x8009CD70):
 *
 *   a2 = lw 0x8C($gp)                 D_8009CDFC freelist head
 *   v0 = lw 0x24(a2)                  next
 *   sw v0, 0x8C($gp)                  pop
 *   if a1 != 0:
 *     sw a1, 0x28(a2)
 *     old = lw 0x24(a1)
 *     sw old, 0x24(a2)                # delay; even when old==0
 *     if old: sw a2, 0x28(old)
 *     sw a2, 0x24(a1)
 *   else:
 *     sw 0, 0x28(a2)
 *     sw 0, 0x24(a2)
 *   serial = lhu 0x598($gp)           D_8009D308
 *   sw 0, 0x0C(a2)
 *   sw a0, 0x00(a2)                   entry PC
 *   sw 0, 0x04(a2)
 *   sw 1, 0x10(a2)
 *   sh 0, 0x08(a2)
 *   sh serial, 0x0A(a2)
 *   sh serial+1, 0x598($gp)
 *   return a2
 *
 * Mailbox bit 2 is OR'd by func_80065400 after return. No empty-list
 * guard (ROM has none).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009CDFC 0x8009CDFCu
#define GA_D_8009D308 0x8009D308u

pe_addr_t func_80012700(pe_addr_t entry, unsigned int a1)
{
    pe_addr_t task;
    pe_addr_t next;
    unsigned int serial;

    task = PE_LoadU32(GA_D_8009CDFC);
    next = PE_LoadU32(task + 0x24u);
    PE_StoreU32(GA_D_8009CDFC, next);

    if (a1 != 0u) {
        pe_addr_t old;

        PE_StoreU32(task + 0x28u, a1);
        old = PE_LoadU32(a1 + 0x24u);
        PE_StoreU32(task + 0x24u, old);
        if (old != 0u)
            PE_StoreU32(old + 0x28u, task);
        PE_StoreU32(a1 + 0x24u, task);
    } else {
        PE_StoreU32(task + 0x28u, 0u);
        PE_StoreU32(task + 0x24u, 0u);
    }

    serial = PE_LoadU16(GA_D_8009D308);
    PE_StoreU32(task + 0x0Cu, 0u);
    PE_StoreU32(task + 0x00u, entry);
    PE_StoreU32(task + 0x04u, 0u);
    PE_StoreU32(task + 0x10u, 1u);
    PE_StoreU16(task + 0x08u, 0u);
    PE_StoreU16(task + 0x0Au, (uint16_t)serial);
    PE_StoreU16(GA_D_8009D308, (uint16_t)(serial + 1u));
    return task;
}
