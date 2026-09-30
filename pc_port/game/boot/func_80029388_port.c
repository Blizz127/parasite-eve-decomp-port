/*
 * Phase 6E-B8 — func_80029388: slot-table clear + default-record init
 * (translated retail logic, classification 1), with its two direct
 * callees — complete audited leaves, not unresolved dependencies.
 *
 * Raw audit (live split asm/disc1/11718.s; func_80029388 glabel at line
 * 9316 — the ONLY split containing it; configs/USA/disc1.yaml keeps the
 * segment as asm.  All 27 words verified against the SHA-1-exact retail
 * executable, taddr 0x80010000):
 *
 *   func_80029388  exe 0x80029388-0x800293F3  27 words / 0x6C  off 0x19B88
 *
 *     0x80029390  jal func_8002F658            (nop delay slot)
 *     0x80029398  addu $a0,$zero,$zero         incoming a0 DISCARDED
 *     loop, i = 0..6 (andi 0xFF counter, sltiu bound 7):
 *       0x800293A0-0x800293B0  i*220 strength-reduction chain
 *                              (sll 3 / subu / sll 3 / subu / sll 2)
 *       0x800293BC  sw $zero, D_800A5D58 + i*220
 *     0x800293D4  sb $zero, 0x530($gp)  ->  0x8009CD70+0x530 = D_8009D2A0
 *     0x800293D8  sb $zero, 0x57C($gp)  ->  0x8009CD70+0x57C = D_8009D2EC
 *     0x800293DC  jal func_80020EFC            (nop delay slot)
 *     epilogue; return void.
 *
 *   func_8002F658  exe 0x8002F658-0x8002F76B  69 words / 0x114  off 0x1FE58
 *     Default-record initializer.  Copies exe rodata through a stack
 *     staging buffer into guest BSS (the stack round-trip is semantically
 *     transparent — rodata, BSS, and stack cannot alias — so the port
 *     copies word-for-word in the retail store order):
 *       0x70 bytes  D_80010928 -> D_800B8A20  (7 x 16-byte blocks,
 *                     4 words each, sequential within each block)
 *       0x18 bytes  D_80010998 -> D_800B0CB0  (6 words, sequential)
 *       sw $zero -> 0x440($gp) = 0x8009D1B0 (D_8009D1B0)
 *       sw $zero -> D_8009D1B4
 *     Read extent 0x80010928-0x800109AF (inside the loaded retail exe:
 *     taddr 0x80010000, tsize 0x1EE000).  Write extent min 0x8009D1B0,
 *     max 0x800B8A8F.  No calls, no SDK, no hardware.  Sole caller:
 *     func_80029388 @0x80029390 (exe-wide jal scan).
 *
 *   func_80020EFC  matching C leaf in the decomp (Phase 5DE, -G 8
 *     -fno-delayed-branch): five $gp-relative byte clears in retail
 *     source order — D_8009CE3C, D_8009D1D4, D_8009D1DC, D_8009D2D8,
 *     D_8009D1F0.  Two exe call sites: func_80029388 @0x800293DC and
 *     func_80029810 @0x8002984C (off the boot path).  Return unused.
 *
 * Call-site audit for func_80029388: exactly ONE distinct exe call site
 * (exe-wide scan for the encoded jal 0x0C00A4E2): func_8003E680
 * @0x8003E700, NOP delay slot — the decomp's matched func_8003E680 C
 * leaf calls func_80029388() with no argument; the addu $a0,$zero,$zero
 * at 0x8003E6FC belongs to the PRECEDING func_800371A4 call.  Return
 * value never consumed.  First-call and repeated-call behavior
 * identical; trivially idempotent (all clears / fixed-value copies).
 *
 * The slot table D_800A5D58 is the same 7 x 220-byte SlotRecord table
 * whose in-use flags the decomp's matched func_8002F9CC leaf clears:
 * struct { unsigned int inUse; unsigned char body[216]; } x7, extent
 * 0x604 to D_800A6360.  func_80029388 clears the same seven in-use
 * words: 0x800A5D58, 0x800A5E34, 0x800A5F10, 0x800A5FEC, 0x800A60C8,
 * 0x800A61A4, 0x800A6280.  Retail simply has two functions that clear
 * the table; both are reproduced faithfully.
 *
 * Dependency-call order (2F658 before the clears, 20EFC after) is
 * statically proven from the raw MIPS and reproduced structurally; it
 * is not dynamically observable because the three functions write
 * disjoint guest regions and consume no return values.
 *
 * All state is guest-RAM-backed via PE_Load/PE_Store — no host copies.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8002F658 addresses */
#define GA_D_80010928 0x80010928u   /* exe rodata: 0x70-byte default record */
#define GA_D_80010998 0x80010998u   /* exe rodata: 0x18-byte default record */
#define GA_D_800B8A20 0x800B8A20u   /* BSS destination of the 0x70 record  */
#define GA_D_800B0CB0 0x800B0CB0u   /* BSS destination of the 0x18 record  */
#define GA_D_8009D1B0 0x8009D1B0u   /* $gp+0x440, zeroed */
#define GA_D_8009D1B4 0x8009D1B4u   /* zeroed */

/* func_80029388 addresses */
#define GA_D_800A5D58 0x800A5D58u   /* SlotRecord table base, stride 220   */
#define GA_D_8009D2A0 0x8009D2A0u   /* $gp+0x530 byte, zeroed */
#define GA_D_8009D2EC 0x8009D2ECu   /* $gp+0x57C byte, zeroed */

/* func_80020EFC addresses (matched decomp leaf, retail source order) */
#define GA_D_8009CE3C 0x8009CE3Cu
#define GA_D_8009D1D4 0x8009D1D4u
#define GA_D_8009D1DC 0x8009D1DCu
#define GA_D_8009D2D8 0x8009D2D8u
#define GA_D_8009D1F0 0x8009D1F0u

/* func_8002F658: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8002F658_port.c (src/func_8002F658.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80020EFC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80020EFC_port.c (src/func_80020EFC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80029388: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80029388_port.c (src/func_80029388.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
