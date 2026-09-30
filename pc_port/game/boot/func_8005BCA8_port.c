/*
 * Phase 6E-B9 — func_8005BCA8: empty return stub (translated retail
 * logic, classification 1).
 *
 * Complete retail body (2 words, verified against the SHA-1-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
 * live split configs/USA/disc1.yaml [0x4C4A8, c, func_8005BCA8] — already
 * a matching C leaf in the decomp since Phase 5AH):
 *
 *   VRAM  0x8005BCA8 – 0x8005BCAE exclusive  (end exclusive = 0x8005BCB0)
 *   file  0x4C4A8 – 0x4C4B0 exclusive
 *   size  0x8  (exactly 2 instructions)
 *
 *   0x8005BCA8: 0x03E00008   jr   $ra
 *   0x8005BCAC: 0x00000000   nop  (delay slot)
 *
 * Signature (from register use and sole call site): void func_8005BCA8(void).
 *   - No argument registers are loaded or consumed before the call.
 *   - $v0 is untouched; the sole call site does not use the return value.
 *   - No stack frame, no $gp use, no memory access, no callees, no
 *     hardware, no GTE, no callbacks, no allocation.
 *
 * Call-site audit (exe-wide scan for jal encoding 0x0C016F2A):
 *   Exactly ONE distinct call site, and zero data-pointer references:
 *     func_8003E680 @ 0x8003E708  (file 0x2EF08), delay slot = nop
 *     Immediately preceding: jal func_80029388 @0x8003E700 (nop slot)
 *     Immediately following:  jal func_80068D28 @0x8003E710 (nop slot)
 *   Placed in the boot subsystem-init series after the Phase 6E-B8
 *   func_80029388 rung.  At entry all previously translated state is
 *   visible (RNG warm-up, subsystem registration, timer records,
 *   VBlank callback slot 4, D_8009CE94, slot-table / default records);
 *   this function mutates none of it.
 *
 * Operation map: empty.  Zero absolute or $gp-relative reads/writes,
 * zero loops, zero SDK/Psy-Q calls, zero status polls.  Cannot block.
 * First-call, repeated-call, and post-PE_RamReset behaviour are
 * identical (no state).  Idempotent by construction.
 *
 * Independent reference: exact retail-word verification of the two
 * words is sufficient — a straight-line empty stub has no loops, packed
 * fields, address arithmetic, or state machine that would need a
 * separate interpreter or trace oracle.
 *
 * Neighbours (for split dedup by executable address, not part of this
 * body):
 *   func_8005BC98 @ 0x8005BC98  (matching C: D_8009D218 = 1)
 *   func_8005BCB0 @ 0x8005BCB0  (matching C: return D_8009D218)
 * This leaf owns only the two words at 0x8005BCA8–0x8005BCAC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8005BCA8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005BCA8_port.c (src/func_8005BCA8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
