/*
 * Phase 6E-B3 — func_8003E974: bit-table init + registration sequence.
 *
 * Retail evidence (asm/disc1/2E7D0.s:712-799, exe 0x8003E974-0x8003EAC7,
 * 85 words / 0x154, file offset 0x2F174):
 *   sw $zero, 0x474($gp)     -> 0x8009D1E4 = 0   (retail $gp = 0x8009CD70,
 *   sw $zero, 0x484($gp)     -> 0x8009D1F4 = 0    set by crt0 @ 0x800725B0)
 *   sw $zero, 0x564($gp)     -> 0x8009D2D4 = 0
 *   sw $zero, 0x4C8($gp)     -> 0x8009D238 = 0
 *   sw $zero, 0x4FC($gp)     -> 0x8009D26C = 0
 *   for (a0 = 0; a0 < 0x20; a0++) D_800A76F0[a0] = 0;
 *       // sltiu $a0,0x20 / bnez, pointer advance in the delay slot:
 *       // 32 word stores, 0x800A76F0..0x800A776C (exactly 0x80 bytes)
 *   20 x func_8003EAC8(mask, value)             // a1 set in each jal delay
 *   D_8009D1A0 |= 0x4000;
 *
 * Complete registration table (ROM order; call-site a0, delay-slot a1):
 *   (0x1,0x4000) (0x80,0x1000) (0x100,0x2000) (0x8,0x10) (0x20,0x40)
 *   (0x40,0x80) (0x10,0x20) (0x2,0x1) (0x4,0x8) (0x200,0x2000)
 *   (0x400,0x4000) (0x2000,0x8000) (0x1000000,0x100) (0x2000000,0x200)
 *   (0x4000000,0x400) (0x8000000,0x800) (0x10000000,0x1000)
 *   (0x20000000,0x2000) (0x40000000,0x4000) (0x80000000,0x8000)
 *
 * Contract: void(void); sole caller func_8003E680 @ 0x8003E6D0; $v0 not
 * consumed.  Idempotent: clears rewrite zeros, the |= is fixed-point, and
 * the registrations replay the same arguments.
 *
 * func_8003EAC8 stays an UNRESOLVED provider this rung (63 call sites
 * exe-wide — a shared GTE-indexed table writer, not part of this leaf):
 * it routes through the centralized boundary, so strict mode stops at its
 * first invocation — a valid frontier advance.
 *
 * D_8009D1A0 now shares the original guest-memory word with input and battle.
 *
 * Classification: TRANSLATED retail initialization logic.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"



/* $gp-resolved scalar addresses (retail $gp = 0x8009CD70) */
#define GA_3E974_GP474   0x8009D1E4u
#define GA_3E974_GP484   0x8009D1F4u
#define GA_3E974_GP564   0x8009D2D4u
#define GA_3E974_GP4C8   0x8009D238u
#define GA_3E974_GP4FC   0x8009D26Cu
#define GA_D_800A76F0    0x800A76F0u

/* func_8003E974: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8003E974_port.c (src/func_8003E974.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
