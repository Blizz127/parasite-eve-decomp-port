/*
 * PE-CH1 — func_800177AC: opcode 0x1F mailbox poll (translated retail).
 *
 * Complete retail body (7 words / 0x1C, exe 0x800177AC–0x800177C4,
 * file offset 0x7FAC). Words dumped from SHA-1-exact
 * build/disc1.candidate.exe (452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 * yaml places this inside [0x2A0C, asm] (2A0C.s, after 17764, before
 * matching func_80017E9C at 0x869C). This worktree has no era/asm
 * split, so the leaf cannot yet be a matching src/ unit. Native port
 * follows the 653B8 / 17764 / 65400 / 12700 family.
 *
 * Jump table D_800910A0[0x1F] @ 0x8009111C = this function.
 * No jal sites (leaf). Next function starts at 0x800177C8 (addiu $sp).
 *
 * ROM (gp = 0x8009CD70; a0 = script-arg dest pointer):
 *
 *   lw   v0, 0x590($gp)         D_8009D300 current task
 *   lw   v1, 0(a0)              dest pointer
 *   lw   v0, 0x14(v0)           task+0x14 payload
 *   nop
 *   sw   v0, 0(v1)              *dest = payload
 *   jr   ra
 *   addiu v0, zero, 1           return 1
 *
 * No store back to the queue or task+0x14 (no ACK).
 * BTL0 type-0 poll after mailbox 3/4 drain reads that payload word
 * (docs/evidence/pe-mbx1-task-state/).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D300 0x8009D300u

/* func_800177AC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800177AC_port.c (src/func_800177AC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
