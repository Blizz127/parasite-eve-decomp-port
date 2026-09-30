/*
 * Field-menu Items/Escape input handler: the callback func_8004AD9C installs
 * into the page window's input slot +0x2C (and that the route stops on as
 * `PE_MenuInputCallback` 0x8004AE1C).
 *
 * Original: [0x8004AE1C,0x8004AF3C), 0x120 bytes / 72 words, asm/disc1/37CD0.s.
 *
 * Body, transcribed exactly:
 *
 *   list = func_80062A20(node, 0)                 ; window+8 -> the page list
 *   if (event & 0x10000) {                        ; confirm
 *       index = func_80063428(list)               ; unsigned compare
 *       if (index < 6) switch (index) {
 *       case 0: func_8004AF3C(list); break;       ; 0x8004AE84
 *       case 1: func_8004B03C(list); break;       ; 0x8004AE94
 *       case 2: func_8004B13C(list); break;       ; 0x8004AEA4
 *       case 3: func_8004B584(list); break;       ; 0x8004AEB4
 *       case 4: case 5:                          ; 0x8004AEC4 (both entries of
 *           func_8005D994(func_80063428(list)-4); ;   jtbl_80011034 point here)
 *           func_80062F1C(node);
 *           func_800439D8();
 *           func_800525EC();                      ; 0x8004AEE4 (falls through)
 *           break;
 *       }
 *       func_800525EC();                          ; 0x8004AEEC shared tail
 *       return 1;
 *   }
 *   if (event & 0x40) {                           ; cancel/close
 *       func_80062F1C(node);
 *       func_800439D8();
 *       func_80052634();
 *   }
 *   return 1;
 *
 * The jump table is retail data, not a guess:
 * asm/disc1/data/800.rodata.s:1309 `jtbl_80011034` =
 *   {0x8004AE84, 0x8004AE94, 0x8004AEA4, 0x8004AEB4, 0x8004AEC4, 0x8004AEC4}.
 * Note the shared tail is reached once for index >= 6 but twice for index 4/5
 * (the case body's own func_800525EC falls through into it); that is preserved.
 *
 * The case-4/5 close path is native: func_8005D994 (which commits the full
 * heal through func_8005247C).  Every handler the tree installs is now
 * translated; no named boundary remains in this function.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

/* func_8004AE1C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004AE1C_port.c (src/func_8004AE1C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
