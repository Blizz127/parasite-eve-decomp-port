/*
 * PE-CH1 — func_80019154: opcode 0x94 mode-word read (translated retail).
 *
 * Complete retail body (7 words / 0x1C, exe 0x80019154–0x8001916C,
 * file offset 0x9954). Words dumped from SHA-1-exact
 * build/disc1.candidate.exe (452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 * yaml places this inside [0x98BC, asm] (98BC.s, before matching
 * func_800192B8 at 0x9AB8). This worktree has no era/asm split, so the
 * leaf cannot yet be a matching src/ unit. Native port follows the
 * 177AC / 17FF0-twin family.
 *
 * Jump table D_800910A0[0x94] @ 0x800912F0 = this function.
 * No jal sites (leaf). Next function starts at 0x80019170 (addiu $sp).
 * Twin of matching src/func_80017FF0.c (opcode 0x89 stores 6 into the
 * same word).
 *
 * ROM:
 *
 *   lw   v1, 0(a0)              dest pointer
 *   lui  v0, 0x800A
 *   lw   v0, -0x2D74(v0)        D_8009D28C
 *   nop
 *   sw   v0, 0(v1)              *dest = mode word
 *   jr   ra
 *   addiu v0, zero, 1           return 1
 *
 * Word-copy only (no arith/bits). Does not store back to D_8009D28C.
 * BTL1 m0005i polls this after 0x89 until the script compares 7
 * (docs/evidence/pe-btl0-field-battle-handoff/).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D28C 0x8009D28Cu

/* func_80019154: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019154_port.c (src/func_80019154.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
