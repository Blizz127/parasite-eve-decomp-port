/*
 * Escape list per-cell draw (installed via func_8004FF80 -> func_800638D8).
 *
 * Original: [0x80050CB4,0x80050CF8), 0x44 bytes / 17 words, asm/disc1/41470.s.
 * Same shape as func_80050C70 with the other string key and focus query:
 *
 *   func_80064C54(node + 0x31);
 *   if (func_80064A48() == node) func_80064C80();
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

extern int func_80064A48(void);

/* func_80050CB4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050CB4_port.c (src/func_80050CB4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
