/*
 * PE-BTL54 — func_80037548 message-record poll (translated
 * retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 27 words 0x80037548..0x800375B4, SHA-256 from oracle.
 * Zero jal. Scan four 56-byte D_800BCEA8 records for
 * +0x10 == needle (lh), return signed byte0. No match → 0.
 * Matching leaf exists under src/; this is the host translation.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800BCEA8 0x800BCEA8u
#define REC_STRIDE    56u

/* func_80037548: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80037548_port.c (src/func_80037548.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
