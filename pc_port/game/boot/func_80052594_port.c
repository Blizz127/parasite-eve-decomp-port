/*
 * Phase 6E-B27 — func_80052594: string copy into fixed 8-byte buffer.
 *
 * Raw body: 22 words / 0x58, executable 0x80052594..0x800525EB, file
 * offset 0x42D94, live split asm/disc1/42D94.s (yaml segment [0x42D94,
 * asm]).  All 22 words verified exact against the SHA-exact retail
 * executable (SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 *
 * ROM-order operation map (no $gp usage — all addresses via lui/addiu):
 *   prologue  (none — leaf function, no stack frame)
 *   1. $a1 = D_80091694 (lui/addiu); $v1 = $a1 + 8 (buffer end)
 *   2. sltu $v0,$a1,$v1 — always true (8-byte buffer); beqz → skip
 *   3. $a2 = 0xFF (terminator constant)
 *   4. Loop .L800525B0:
 *        lbu $v0,0($a0)         load source byte
 *        nop                     load delay
 *        beq $v0,$a2 → exit     terminator check
 *        nop                     delay slot
 *        sb $v0,0($a1)          store byte to destination
 *        addiu $a1,$a1,1        destination++
 *        sltu $v0,$a1,$v1       check buffer full
 *        bnez $v0 → loop        continue if space remains
 *        addiu $a0,$a0,1        source++ (delay slot)
 *   5. Exit .L800525D4:
 *        $v1 = D_8009169D (lui/addiu)
 *        $v0 = D_8009169D - 9 = D_80091694
 *        $v0 = $a1 - D_80091694 = byte count (0-8)
 *        jr $ra
 *        sb $v0,0($v1)          delay slot: store count at D_8009169D
 *
 * Signature: int func_80052594(pe_addr_t src).  One argument in $a0.
 * Returns byte count (0-8) in $v0.  Side effect: stores count at
 * D_8009169D.  The 0xFF terminator byte is NOT copied.
 *
 * Five executable call sites (jal word 0x0C014965):
 *   func_8005D6F4 @ 0x8005D898 (delay: addu $a0,$v0; return discarded)
 *   func_8004DD64 @ 0x8004E28C (delay: addu $a0,$v0; return discarded)
 *   func_8004DD64 @ 0x8004E428 (delay: addu $a0,$v0; return discarded)
 *   func_8004DD64 @ 0x8004E6A8 (delay: addu $a0,$v0; return discarded)
 *   func_8005C46C @ 0x8005C46C (delay: nop; return discarded)
 *
 * All call sites discard the return value.
 *
 * D_80091694 (8-byte buffer) and D_8009169D (1-byte count) are
 * guest-RAM resident.  Exhaustive executable-wide lui/addiu scan found
 * NO readers — these are write-only stores.  Not $gp-relative
 * ($gp = 0x8009CD70, offset would be -0xB6DC, outside small-data area).
 *
 * Classification: 1 — translated retail logic (leaf, no callees,
 * pure guest-memory, deterministic, idempotent for same source).
 */
#include "psx_compat.h"

#define GA_52594_BUF   0x80091694u  /* 8-byte destination buffer */
#define GA_52594_END   0x8009169Cu  /* buffer end (exclusive) */
#define GA_52594_COUNT 0x8009169Du  /* 1-byte count store */
#define GA_52594_TERM  0xFFu        /* terminator byte */

/* func_80052594: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80052594_port.c (src/func_80052594.c); hand port retired (port3 switch-over A1). */
