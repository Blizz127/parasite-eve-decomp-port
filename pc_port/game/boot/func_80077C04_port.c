/*
 * Phase 6E-B54I — func_80077C04: sprite packet-header setter (setSprt)
 * (translated retail logic, classification 1 — real outlined header inline).
 *
 * Complete retail body including its return delay slot (5 words, verified
 * against the SHA-1-exact retail executable SLUS_006.62 / SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b; file 0x68404; a matching C
 * leaf in the decomp since Phase 5CG — configs/USA/disc1.yaml
 * [0x68404, c, func_80077C04]).  Decoded from build/disc1.candidate.exe:
 *
 *   0x80077C04: 0x24020004  addiu v0, zero, 4
 *   0x80077C08: 0xA0820003  sb    v0, 3(a0)       ; p[3] = 4   (length)
 *   0x80077C0C: 0x24020064  addiu v0, zero, 0x64
 *   0x80077C10: 0x03E00008  jr    ra
 *   0x80077C14: 0xA0820007  sb    v0, 7(a0)       ; delay slot: p[7] = 0x64
 *
 * The delay-slot store at 0x80077C14 is INSIDE this function's footprint
 * (verified word in the SHA-exact image); the next function begins at
 * 0x80077C18 with alignment nops.
 *
 * Psy-Q libgpu origin: setSprt(p) — length 4, code 0x64 (textured 1x1
 * sprite).  Not jal'd directly by func_80030894; it is the second callee
 * of both primitive add/sort wrappers func_800370DC (@0x80037104) and
 * — as its tile twin func_80077C44 — func_80037140.
 *
 * ABI: void func_80077C04(pe_addr_t p).  Writes exactly two bytes and
 * nothing else: no reads, no other stores, no callees, no hardware.
 * Idempotent; first/repeated call identical (order 3 then 7).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80077C04: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80077C04_port.c (src/func_80077C04.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
