/*
 * Phase 6E-B51 — func_8006E1C0: packed texture-entry LoadImage dispatcher.
 *
 * Raw body: 68 words / 0x110 bytes, executable 0x8006E1C0..0x8006E2CF
 * (exclusive end 0x8006E2D0), file offset 0x5E9C0, live split
 * asm/disc1/5B1E4.s:4047-4119 (yaml segment [0x5B1E4, asm]).  All 68 words
 * verified exact against the SHA-exact retail executable
 * (SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 *
 * True ABI: int func_8006E1C0(pe_addr_t entry, pe_addr_t base).
 * Retail returns v0=0 (addu $v0,$zero,$zero at 0x8006E2B0); all eight
 * executable call sites discard it (each reloads $v0 from the table header
 * immediately after the jal).  a2/a3 are never read.
 *
 * Caller census (all 8 executable sites; identical shape: a0=entry,
 * a1=base in the jal delay slot, return unconsumed, inside a counted
 * 0x14-stride entry loop):
 *   func_8006914C @0x8006949C   func_8006B4F8 @0x8006B65C
 *   func_8006AD40 @0x8006AE48   func_8006B4F8 @0x8006B740   (B50 site: AE48)
 *   func_8006AD40 @0x8006B1DC   func_8006BECC @0x8006C03C
 *   func_8006AD40 @0x8006B254   func_8006C5BC @0x8006C750
 *
 * Entry record layout (0x14-byte stride in the callers' loops):
 *   +0x4  u32  image data offset from base (low 24 bits used)
 *   +0x7  u8   image rect h; retail substitutes 0x100 when zero
 *   +0x8  u32  packed image rect: x = (w>>10)&0x7FF, y = w>>21, w = w&0x3FF
 *   +0xC  u32  CLUT data offset from base (low 24 bits; zero = no CLUT call)
 *   +0xF  u8   CLUT rect h (no zero substitution)
 *   +0x10 u32  packed CLUT rect (same decode as +0x8)
 *
 * ROM-order operation map (retail $gp = 0x8009CD70; this body never
 * touches $gp, rodata, or any global — its only persistent effects are the
 * two callee dispatches):
 *   prologue  addiu $sp,-0x28; save $s0/$s1/$s2/$ra; s0=a0, s2=a1
 *   1. lw [s0+8]  x3 (three separate retail loads) -> sh halfwords to the
 *      stack rect at sp+0x10/+0x12/+0x14.
 *   2. lbu [s0+7]; beqz with delay-slot addiu $v1,0x100: h = byte ? byte :
 *      0x100; sh -> sp+0x16.  s1 = 0x00FFFFFF (lui/ori).
 *   3. lw [s0+4] & s1; a0 = sp+0x10;
 *      jal func_8007506C @0x8006E238, delay slot a1 = s2 + a1.
 *   4. lw [s0+0xC] & s1; beqz -> epilogue (delay slot a0 = sp+0x10).
 *   5. CLUT block: lw [s0+0x10] x3 -> sh sp+0x10/+0x12/+0x14;
 *      lbu [s0+0xF] -> sh sp+0x16 (no 0x100 substitution);
 *      lw [s0+4] & s1 + s2;  lw [s0+0xC] & s1;
 *      jal func_8007506C @0x8006E2A8, delay slot a1 = v0 + a1
 *      (= base + image_off + clut_off).
 *   6. v0 = 0; restore; jr $ra; nop.
 *
 * Callback audit (func_8007506C = PsyQ LoadImage, string-proven in
 * docs/ai_context/sdk_map.md; 24 words at 0x8007506C, live split
 * asm/disc1/654C8.s:283-310):
 *   LoadImage(rect, data) calls the read-only debug validator
 *   func_80074E28("LoadImage", rect) — its 0x11C body stores only to its
 *   own stack frame — then dispatches through the libgpu jump table:
 *   a0 = lw(lw(D_80095744)+0x20), jalr lw(lw(D_80095744)+0x8), a2 = 8,
 *   a3 = data (delay slot).
 *   D_80095744 is statically initialized to 0x80095704 (the 16-word "jtb")
 *   and has no runtime store anywhere in the executable (all 25 references
 *   are lui/lw reads; ResetGraph — the only pre-B51 setup caller — never
 *   writes it or the jtb).  Therefore both dispatches resolve identically
 *   and statically:
 *     target  = jtb[2] = 0x80076C34 (libgpu queue/transfer manager)
 *     a0      = jtb[8] = 0x80076664 (immediate worker, inner jalr $s3)
 *   The callback's state effects (GPU command packet ring at D_800BD03C,
 *   queue indices D_80095874/78/7C, env flag D_80095754, VRAM transfer)
 *   are NOT consumed by this function: both returns are discarded, the
 *   stack rect is rebuilt from entry fields between the calls, and every
 *   post-call load reads the caller-supplied entry record.  An honest
 *   centralized prefix boundary therefore preserves all required state.
 *
 * Boundary representation: the retail rect lives at guest sp+0x10 and is
 * visible only to the callee chain, so it has no persistent guest authority
 * in the port. B53C converts its four signed halfwords to two words by value
 * before entering the exact canonical func_80076C34 target. No host pointer
 * is stored in guest RAM or retained on that path; a deliberately dirty
 * indirect target remains a host-only diagnostic boundary.
 *
 * Classification: 1 — translated retail logic. Phase 6E-B53E issues the
 * first LoadImage DMA through func_80076664 and leaves it pending. B53G
 * completes func_80073CF4 and its func_800746A0 setter, so the second
 * request registers and publishes its ring entry. B53H translates the
 * busy-DMA fast path of the pump func_80076EE4, which on this path reads
 * DMA2 CHCR, finds the first transfer still in flight, and returns 1
 * without touching the ring — so this dispatcher now completes with the
 * queued request unconsumed and the transfer still pending.
 */
#include "psx_compat.h"
#include "game_port.h"

/* func_8006E1C0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006E1C0_port.c (src/func_8006E1C0.c); hand port retired (port3 switch-over N). */
