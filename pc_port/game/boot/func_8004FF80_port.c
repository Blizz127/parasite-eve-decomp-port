/*
 * Field-menu Escape sub-page list draw: tier-2 wrapper (hand adapter).
 *
 * Original: matched C leaf src/func_8004FF80.c, [0x8004FF80,0x8004FFA8),
 * 10 words (configs/USA/disc1.yaml:788):
 *
 *     void func_8004FF80(int slot) { func_800638D8(slot, func_80050CB4); }
 *
 * Hand adapter for the guest identity 0x80050CB4 that func_8004B03C stores
 * into list+0x30 (same E5/E6 class as func_8004FF30).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8004FF80: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004FF80_port.c (src/func_8004FF80.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
