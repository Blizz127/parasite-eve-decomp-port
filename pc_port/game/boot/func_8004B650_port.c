/*
 * Field-menu modal sub-page input handler (installed by func_8004B584 into
 * window+0x2C).
 *
 * Original: [0x8004B650,0x8004B70C), 0xBC bytes / 47 words, asm/disc1/3BD84.s.
 *
 *   if (event & 0x1000) { func_8005E850(0, -1); func_8005267C(); return 1; }
 *   if (event & 0x4000) { func_8005E850(0,  1); func_8005267C(); return 1; }
 *   if (event & 0x10000) { func_80062F1C(node); func_800525EC(); return 1; }
 *   if (event & 0x40) {
 *       func_8005E850(0, D_8009D264 - func_8005E884());  ; alarm-timer delta
 *       func_80062F1C(node); func_80052634(); return 1;
 *   }
 *   return 1;
 *
 * D_8009D264 is the Alarm/timer snapshot func_8004B584 published when the modal
 * window was constructed (the same word func_8004B6CC reads).  func_8005E884()
 * returns the current signed alarm byte, so the cancel arm scrolls the modal by
 * the elapsed delta.  This was the named boundary `PE_MenuInputCallback_8004B650`.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8004B650: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004B650_port.c (src/func_8004B650.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
