/*
 * PE-CH1 — func_800653B8: mailbox queue append (translated retail logic).
 *
 * Complete retail body (18 words / 0x48, exe 0x800653B8–0x800653FC,
 * file offset 0x55BB8). Words dumped from SHA-1-exact
 * build/disc1.candidate.exe (452fb033f2eaa4b18aa20a5bca60b8125af3a37b),
 * taddr 0x80010000. yaml places this inside the 55430.s asm blob
 * (configs/USA/disc1.yaml [0x55430, asm] … [0x5ADBC, c, func_8006A5BC]);
 * this worktree has no era/asm split, so the leaf cannot yet be a
 * matching src/ unit. Native port follows the func_8006536C family.
 *
 * No SDK/Psy-Q calls, no hardware, no GTE, no callbacks, no clamp at 28.
 *
 * ROM (gp = 0x8009CD70; 0x44($gp) = D_8009CDB4):
 *
 *   lbu  v1, 0x44($gp)          count
 *   lw   t0, 0x10($sp)          extra (5th arg)
 *   sll  v0, v1, 1              count*2
 *   addu v0, v0, v1             count*3
 *   sll  v0, v0, 2              count*12
 *   lui  v1, 0x800A
 *   addiu v1, v1, 0x3180        D_800A3180  (clobbers the saved count)
 *   addu v0, v0, v1             rec
 *   sb   a0, 3(v0)              payload
 *   sb   a1, 2(v0)              dest id
 *   lbu  v1, 0x44($gp)          reload count
 *   sw   a3, 8(v0)              sender
 *   sh   a2, 0(v0)              dest type
 *   sw   t0, 4(v0)              extra
 *   addiu v1, v1, 1
 *   sb   v1, 0x44($gp)
 *   jr   ra
 *   nop
 *
 * Sole jal: func_80017764 @ 0x80017794 (opcode 0x1C; extra sw $zero in
 * the jal delay slot). BTL0 m0004i mailbox 3/4 is payload 3/4 on
 * dest type 0 id 0 (docs/evidence/pe-mbx1-task-state/).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800A3180 0x800A3180u
#define GA_D_8009CDB4 0x8009CDB4u
#define TABLE_STRIDE  0xCu

/* func_800653B8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800653B8_port.c (src/func_800653B8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
