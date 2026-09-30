/*
 * Original signed-number printer (menu font): draws an optional sign icon and
 * the value's most-significant digits, each 5 px apart.
 *
 * Original: [0x8005FCAC,0x8005FDF0), 0x144 bytes / 81 words, asm/disc1/50074.s.
 *
 *   if (value < 0)      { value = -value; func_8005EB64(0x52); digits = 2; }
 *   else if (value > 0) {                  func_8005EB64(0x89); digits = 3; }
 *   else                {                  digits = 3; }            ; no icon
 *   x += 5                                   ; (only when an icon was drawn)
 *   divisor = 10^(digits-1);
 *   for (i = 0; i < digits; i++) {
 *       d = value / divisor;                 ; MIPS `div` == C truncation
 *       if (i < digits-1 && d == 0) d = -1;  ; leading-zero suppression -> blank
 *       func_8005F874(d);
 *       divisor /= 10;
 *       x += 5;
 *   }
 *
 * func_8005F874 renders -1 as a blank cell (see func_8004DF74_port.c, which draws
 * glyph 88 for negative values).  Both icons and the blank are already native;
 * the only guest state touched is the drawing cursor pair D_8009D124/D_8009D128
 * ($gp + 0x3B4/0x3B8), which func_8005E8A4 also manipulates.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_FCAC_X 0x8009D124u
#define GA_FCAC_Y 0x8009D128u

/* func_8005FCAC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005FCAC_port.c (src/func_8005FCAC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
