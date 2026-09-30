/*
 * Field-menu Equipment list1 per-cell draw (the callback func_8004B534 passes
 * to func_800638D8, invoked from func_800634D4's cell loop via
 * menu_draw_callback).
 *
 * Original: [0x80050438,0x800504BC), 0x84 bytes / 33 words, asm/disc1/40B3C.s.
 *
 *   cell  = a0 (cell index)
 *   shift = cell * 8                         ; byte lane in the packed colour
 *   func_8005E8C4();
 *   func_8005E968(0x80 << shift);            ; alpha for this lane
 *   func_8005F5B8(cell + 0x35);              ; text id (0x35 + cell)
 *   func_8005E968(0x808080);                 ; 50% grey
 *   func_8005E8A4(0xA, 3);
 *   func_8005FB74((((D_8009D14C >> shift) & 0xFF) - 0x20) >> 1);
 *   func_8005E914();
 *
 * The `- 0x20` then arithmetic `>> 1` is retail `addiu v0,-0x20; sra a0,v0,1`;
 * the value can be negative, so the shift is arithmetic here.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80050438: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050438_port.c (src/func_80050438.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
