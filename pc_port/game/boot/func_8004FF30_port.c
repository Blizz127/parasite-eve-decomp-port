/*
 * Field-menu list draw entry: tier-2 callback-registration wrapper.
 *
 * Original: [0x8004FF30,0x8004FF58), 10 words, matched C span
 * (configs/USA/disc1.yaml:784; evidence
 * docs/evidence/volume-campaign-20260831/func-8004ff30/REPORT.md).  The
 * matching leaf src/func_8004FF30.c is a one-call wrapper:
 *
 *     void func_8004FF30(int slot) {
 *         func_800638D8(slot, func_80050C50);
 *     }
 *
 * It is not auto-derivable: a bare `func_*` used as a value is a 32-bit guest
 * code pointer, not a host pointer, so gen_decomp_ports.py reports the leaf
 * ("indirect call through local", rules E5/E6).  This is the hand adapter for
 * that leaf.  The only adaptation is that the guest callback identity travels
 * as its address 0x80050C50 — exactly the value the 0x3B5EC `lui`/`addiu`
 * materializes at the func_8004AD9C call site (asm/disc1/37CD0.s:4058/4059),
 * which is what that page stores into the list's callback slot +0x30.
 *
 * Call path: func_80062FEC -> func_80062830 -> menu_draw_callback(list+0x30 =
 * 0x8004FF30, list) -> here -> func_800638D8(list, 0x80050C50), the generic
 * list renderer, which invokes the per-cell draw 0x80050C50 for every visible
 * cell.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8004FF30: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004FF30_port.c (src/func_8004FF30.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
