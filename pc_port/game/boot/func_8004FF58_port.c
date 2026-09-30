/*
 * Field-menu Items sub-page list draw: tier-2 callback-registration wrapper.
 *
 * Original: matched C leaf src/func_8004FF58.c, [0x8004FF58,0x8004FF80),
 * 10 words (configs/USA/disc1.yaml:786).  The matching leaf is
 *
 *     void func_8004FF58(int slot) { func_800638D8(slot, func_80050C70); }
 *
 * and is not auto-derivable (a bare `func_*` used as a value is a 32-bit guest
 * code pointer, not a host pointer — gen_decomp_ports.py rules E5/E6), so this
 * is the hand adapter for the guest identity 0x80050C70 that func_8004AF3C
 * stores into list+0x30.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8004FF58: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004FF58_port.c (src/func_8004FF58.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
