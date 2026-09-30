/*
 * Field-menu Equipment window draw (installed by func_8004B13C into the window
 * draw slot +0x30; dispatched by func_80062830 -> menu_draw_callback).
 *
 * Original: [0x8004B214,0x8004B394), 0x180 bytes / 96 words, asm/disc1/37CD0.s.
 *
 * The page shows two rows of three lanes (up/down pairs).  It reads the packed
 * colour byte lane selected by the list index and tints each lane's arrow:
 *
 *   index = func_80063428(func_80062A20(node, 0))
 *   focus = func_8005E54C() & 0x1000
 *   func_8005E8A4(0x12, 5)
 *   for lane in 0..2: func_8005EB58(!(index==lane && focus)); func_8005EB64(0x4A);
 *                     (lanes 0/1 also func_8005E8A4(0x20,0))
 *   focus = func_8005E54C() & 0x4000
 *   func_8005E8A4(-0x40, 0x19)
 *   for lane in 0..2: func_8005EB58(!(index==lane && focus)); func_8005EB64(0x4B);
 *                     (lanes 0/1 also func_8005E8A4(0x20,0))
 *
 * The `!(index==lane && focus)` form is the retail
 * `bnez index / bnez focus / a0=1` pair: the colour is dimmed exactly when the
 * lane is selected AND the corresponding pad focus bit is set.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

extern int func_8005E54C(void);

/* func_8004B214: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004B214_port.c (src/func_8004B214.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
