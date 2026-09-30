/*
 * Field-menu Escape sub-page input handler (installed by func_8004B03C).
 *
 * Original: [0x8004B0A4,0x8004B13C), 0x98 bytes / 38 words, asm/disc1/37CD0.s.
 * Same shape as func_8004AFA4 except the confirm arm resets resource state
 * through func_800649D0 (Phase 6E-B35) instead of func_80052790:
 *
 *   list = func_80062A20(node, 0)
 *   if (event & 0x10000) {
 *       func_800525EC();
 *       func_800649D0(func_80063428(list));
 *       func_80062F1C(node);
 *       return 1;
 *   }
 *   if (event & 0x40) { func_80062F1C(node); func_80052634(); }
 *   return 1;
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8004B0A4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004B0A4_port.c (src/func_8004B0A4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
