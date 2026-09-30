/*
 * Phase 6E-B25 — func_8005D6F4: resource-buffer + display-state initializer.
 *
 * Raw body: 147 words / 0x24C, executable 0x8005D6F4..0x8005D93F, file
 * offset 0x4DEF4, live split asm/disc1/4CC98.s:1340-1502 (yaml segment
 * [0x4CC98, asm]).  All 147 words verified exact against the SHA-exact
 * retail executable (SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 *
 * ROM-order operation map (retail $gp = 0x8009CD70):
 *   prologue  addiu $sp,-0x18; save $s0/$ra
 *   1. jal func_80071A24(0x800C0DE0, 0x12E4)   REAL BIOS A(28h) bzero,
 *      a1 = 0x12E4 in the delay slot: zeroes 0x800C0DE0..0x800C20C3.
 *   2. a0 = 0x800C0DF0 (base + 0x10); sw 1 → 0x4A8($gp) (D_8009D218).
 *   3. Block-1 state stores (order C8, C0, C4):
 *        sw 0  → 0x358($gp) (D_8009D0C8)
 *        sw a0 → 0x350($gp) (D_8009D0C0) = 0x800C0DF0
 *        sw 8  → 0x354($gp) (D_8009D0C4)
 *      preceded by the retail sltu guard (a0 < base+0x18), a tautology
 *      for these constants — preserved structurally.
 *   4. Fill loop #1: sb 0xFF over [D_8009D0C0, D_8009D0C0+D_8009D0C4)
 *      = 8 bytes at 0x800C0DF0..0x800C0DF7; retail RELOADS C0/C4 from
 *      guest RAM every iteration (reproduced via PE_LoadU32).
 *   5. v0 = lw D_8009D0C8 (always 0 here — stored in step 3);
 *      s0 = lw D_8009D0C0.  beqz v0 taken:
 *        jal func_8005DC4C(a0=0x1E delay slot, a1=0xFF residual fill
 *        register)                               REAL (B26)
 *      Retail sets $a1 = s0 (the copy destination) only AFTER the call
 *      returns (addu $a1,$s0 at 0x8005D794 — not a delay slot).
 *      The not-taken arm (v0 != 0) would pass lbu(v0+4)-1 to
 *      func_8005DC9C — statically dead inside this function (C8 was
 *      just stored 0 and only the fill loop runs in between), preserved
 *      structurally.
 *   6. String copy #1: from the func_8005DC4C return (source) into
 *      D_8009D0C0 (dest) byte-by-byte until the copied byte == 0xFF;
 *      the dest cursor advances once per copied byte INCLUDING the
 *      terminator (retail delay-slot increments).
 *   7. Block 2: a0 = 0x800C0DF0 again; state store order differs from
 *      block 1: sw 1 → D_8009D218, sw 8 → D_8009D0C4, then sltu guard,
 *      sw 0 → D_8009D0C8, sw a0 → D_8009D0C0.  Fill loop #2 (8 bytes,
 *      reload-per-iteration).  NOTE: this re-fills the bytes string
 *      copy #1 just wrote — exact retail behavior, reproduced.
 *   8. Second selection: lw C8 (0) / lw C0; beqz taken:
 *        jal func_8005DC4C(a0=0x1E, a1=0xFF)    REAL (B26, 2nd call)
 *   9. String copy #2 (same mechanics as #6).
 *  10. jal func_8005DC4C(0x1E, dest-after-copy-2)   REAL (B26, 3rd);
 *      here $a1 genuinely carries the post-copy dest cursor (one past
 *      the terminator) — no instruction touches $a1 between copy #2
 *      and this call.
 *  11. jal func_80052594(ret-of-10)                REAL (B27)
 *  12. jal func_8005CCA4()                         REAL (B28)
 *  13. sh 0x0203 → 0x800C1F80   (direct halfword store)
 *      sw 0x00404040 → 0x800C0E44   (direct word store)
 *  14. jal func_800614AC(0x00404040)               REAL (B32)
 *      (a0 built as lui 0x40 / ori 0x4040 in the delay slot)
 *  15. sw 0 → D_800A76A4, D_800A76B0, D_800A76BC, D_800A76C8
 *      (the +4 tick fields of the three B5 timer records at
 *      0x800A76A0/AC/B8 plus the word past them)
 *  16. jal func_8005E884() → r                     REAL (B33)
 *      (retail returns lbu 0x800B0DB1 — unwritten BSS at this point)
 *  17. jal func_8005E850(0, 8 - r)                 REAL (B34/B37)
 *      (a1 = 8 - r formed in the delay slot)
 *  18. jal func_800649D0(0)                        REAL (B35)
 *  19. jal func_80052790(1)                        REAL (B36/B38)
 *  20. v0 = 0xFF; sb 0xFF → 0x800C20A4; sb 0xFF → 0x800C20B4
 *      (terminator bytes at the two record-buffer bases that
 *      func_8005BCBC selects when a0 != 0)
 *   epilogue  restore $ra/$s0; jr $ra; returns 0xFF (unconsumed by the
 *   sole call site).
 *
 * Signature: int func_8005D6F4(void).  Sole executable call site (scan
 * for jal word 0x0C0175BD): func_800527C8 @0x8005283C, nop delay slot
 * (a0 carries the residual 0 from func_8005BCBC's delay-slot setup —
 * the body never reads $a0); unconditional; one-shot per dispatcher
 * invocation; return discarded.
 *
 * Historical Phase 6E-B27 dependency boundary: func_80071A24 is REAL (B21
 * BIOS A(28h) bzero), func_8005DC4C is REAL (B26 message-table lookup),
 * func_80052594 is REAL (B27 string copy into fixed 8-byte buffer),
 * func_8005E884 is REAL (B33 signed-byte alarm-timer query),
 * func_8005E850 is REAL (B34 alarm-timer setter wrapper; its callee
 * func_8006A2E8 is REAL in B37),
 * func_800649D0 is REAL (B35 resource-state reset),
 * func_80052790 is REAL (B36 boolean-state store + notify wrapper; its
 * callee func_80086728 is REAL in B38). All D6F4 direct callees are now
 * translated. func_8005DC9C is a dead arm. After B39, strict mode reaches
 * func_8005332C from func_80051CC4; func_80087090 remains later in the
 * non-strict func_8006A9E4 path.
 *
 * Consumed dependency return: func_8005DC4C is the real guest archive
 * lookup, and its result is the string-copy source. Tests invoking D6F4
 * seed the retail-shaped archive precondition; address zero is never
 * mirrored or replaced by a fallback. func_8005E884 reads D_800B0DB1
 * directly; during boot that byte is unwritten BSS (0).
 *
 * State: D_8009D218/D_8009D0C0/C4/C8 are guest-RAM resident (B24
 * storage audit; shared with func_8005BCBC and the func_8005BD10/BE1C
 * readers).  The bzero range, the two fill ranges, 0x800C1F80,
 * 0x800C0E44, 0x800C20A4/B4 and D_800A76A4..C8 are direct guest stores.
 * Idempotence: NOT idempotent as a whole — the func_8005DC4C boundary
 * calls pop scripted sequences in ROM order, so repeated invocations
 * consume sequence entries; with all defaults the body is
 * state-reproducible (every store unconditionally overwrites).
 * PE_RamReset restores initial conditions.
 *
 * Classification: 1 — translated retail logic; every direct callee is
 * translated.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_bootstrap.h"

extern void func_800614AC(int a0);
extern signed char func_8005E884(void);
extern void func_8005E850(int a0, int a1);
extern void func_80052790(int a0);
extern void func_800649D0(int a0);

#define GA_5D6F4_BUF_BASE  0x800C0DE0u   /* bzero dest / buffer base    */
#define GA_5D6F4_BUF_SEL   0x800C0DF0u   /* base + 0x10 selected buffer */
#define GA_5D6F4_FLAG      0x8009D218u   /* $gp+0x4A8 (func_8005BC98)   */
#define GA_5D6F4_C8        0x8009D0C8u   /* $gp+0x358                   */
#define GA_5D6F4_C0        0x8009D0C0u   /* $gp+0x350                   */
#define GA_5D6F4_C4        0x8009D0C4u   /* $gp+0x354                   */
#define GA_5D6F4_HW203     0x800C1F80u   /* sh 0x0203 target            */
#define GA_5D6F4_W404040   0x800C0E44u   /* sw 0x00404040 target        */
#define GA_5D6F4_REC_A4    0x800A76A4u
#define GA_5D6F4_REC_B0    0x800A76B0u
#define GA_5D6F4_REC_BC    0x800A76BCu
#define GA_5D6F4_REC_C8    0x800A76C8u
#define GA_5D6F4_TERM_A4   0x800C20A4u   /* sb 0xFF record base A       */
#define GA_5D6F4_TERM_B4   0x800C20B4u   /* sb 0xFF record base B       */

/* Phase 6E-B26: func_8005DC4C is REAL (translated message/string-table
 * lookup, game/boot/func_8005DC4C_port.c).  The statically dead
 * `D_8009D0C8 != 0` arm now calls the REAL func_8005DC9C (matched leaf
 * src/func_8005DC9C.c, hand port in func_800236E8_port.c) exactly as the
 * matched src/func_8005D6F4.c does -- no bootstrap boundary remains. */
extern pe_addr_t func_8005DC9C(uint32_t index);

/* Retail fill loop: sb 0xFF over [C0, C0+C4) with per-iteration guest
 * reloads of C0/C4 (retail load-after-store). */
static void fill_ff(pe_addr_t start)
{
    pe_addr_t p = start;
    for (;;) {
        PE_StoreU8(p, 0xFFu);
        {
            pe_addr_t base = PE_LoadU32(GA_5D6F4_C0);
            pe_addr_t len  = PE_LoadU32(GA_5D6F4_C4);
            p += 1u;
            if (!(p < base + len))
                break;
        }
    }
}

/* Retail string copy: bytes flow source → dest until the copied byte is
 * 0xFF; the dest cursor advances once per copied byte including the
 * terminator (both retail delay-slot increments).  Returns the final
 * dest cursor (one past the terminator). */
static pe_addr_t copy_ff_string(pe_addr_t src, pe_addr_t dst)
{
    uint8_t b = PE_LoadU8(src);
    src += 1u;
    PE_StoreU8(dst, b);
    dst += 1u;
    while (b != 0xFFu) {
        b = PE_LoadU8(src);
        src += 1u;
        PE_StoreU8(dst, b);
        dst += 1u;
    }
    return dst;
}

int func_8005D6F4(void)
{
    pe_addr_t a0, s0, cursor;
    pe_addr_t src;
    int r;

    /* 1. bzero(0x800C0DE0, 0x12E4) — REAL BIOS A(28h) trampoline. */
    func_80071A24(GA_5D6F4_BUF_BASE, 0x12E4u);

    /* 2-3. Block 1: flag + state stores (retail order C8, C0, C4). */
    a0 = GA_5D6F4_BUF_SEL;
    PE_StoreU32(GA_5D6F4_FLAG, 1u);
    if (a0 < GA_5D6F4_BUF_BASE + 0x18u) {   /* retail sltu guard (tautology) */
        PE_StoreU32(GA_5D6F4_C8, 0u);
        PE_StoreU32(GA_5D6F4_C0, a0);
        PE_StoreU32(GA_5D6F4_C4, 8u);
        fill_ff(a0);
    }

    /* 5-6. Selection #1: C8 reads back 0 (stored above) → func_8005DC4C.
     * The v0 != 0 arm would call func_8005DC9C(lbu(v0+4)-1) — dead here,
     * preserved structurally.  Retail @0x8005D78C: jal func_8005DC4C with
     * `addiu $a0,$zero,30` in the delay slot; $a1 still holds the 0xFF
     * fill constant, which the callee never reads, and the copy dest is
     * set only afterwards by `addu $a1,$s0,$zero` @0x8005D794. */
    s0 = PE_LoadU32(GA_5D6F4_C0);
    if (PE_LoadU32(GA_5D6F4_C8) != 0u) {
        pe_addr_t rec = PE_LoadU32(GA_5D6F4_C8);
        unsigned int idx = (unsigned int)PE_LoadU8(rec + 4u) - 1u;
        /* src/func_8005D6F4.c: r = func_8005DC9C(D_8009D0C8[4] - 1);
         * statically dead here (C8 was just stored 0), ported verbatim. */
        src = func_8005DC9C(idx);
    } else {
        src = func_8005DC4C(30u);
    }
    copy_ff_string(src, s0);

    /* 7. Block 2: re-select 0x800C0DF0; retail store order flag, C4,
     * C8, C0 (differs from block 1); re-fill overwrites copy #1. */
    a0 = GA_5D6F4_BUF_SEL;
    PE_StoreU32(GA_5D6F4_FLAG, 1u);
    PE_StoreU32(GA_5D6F4_C4, 8u);
    if (a0 < a0 + 8u) {                     /* retail sltu guard (tautology) */
        PE_StoreU32(GA_5D6F4_C8, 0u);
        PE_StoreU32(GA_5D6F4_C0, a0);
        fill_ff(a0);
    }

    /* 8-9. Selection #2 + string copy #2. */
    s0 = PE_LoadU32(GA_5D6F4_C0);
    if (PE_LoadU32(GA_5D6F4_C8) != 0u) {
        pe_addr_t rec = PE_LoadU32(GA_5D6F4_C8);
        unsigned int idx = (unsigned int)PE_LoadU8(rec + 4u) - 1u;
        /* src/func_8005D6F4.c: r = func_8005DC9C(D_8009D0C8[4] - 1);
         * statically dead here (C8 was just stored 0), ported verbatim. */
        src = func_8005DC9C(idx);
    } else {
        src = func_8005DC4C(30u);           /* retail @0x8005D84C */
    }
    cursor = copy_ff_string(src, s0);

    /* 10-11. Third func_8005DC4C @0x8005D890 (same index 30; $a1 holds the
     * post-copy cursor, which the callee never reads), then func_80052594
     * on its return — retail passes it as $a0 via `addu $a0,$v0,$zero` in
     * the jal delay slot @0x8005D89C. */
    src = func_8005DC4C(30u);
    (void)cursor;                           /* retail residual $a1 */
    func_80052594(src);      /* B27: REAL — copies to D_80091694; retail discards $v0 */

    /* 12. func_8005CCA4() — REAL (B28) */
    func_8005CCA4();

    /* 13. Direct stores: halfword 0x0203, word 0x00404040. */
    PE_StoreU16(GA_5D6F4_HW203, 0x0203u);
    PE_StoreU32(GA_5D6F4_W404040, 0x00404040u);

    /* 14. func_800614AC(0x00404040); return is discarded by retail. */
    (void)func_800614AC(0x00404040);

    /* 15. Timer-record tick fields + trailing word. */
    PE_StoreU32(GA_5D6F4_REC_A4, 0u);
    PE_StoreU32(GA_5D6F4_REC_B0, 0u);
    PE_StoreU32(GA_5D6F4_REC_BC, 0u);
    PE_StoreU32(GA_5D6F4_REC_C8, 0u);

    /* 16. func_8005E884() → r — REAL (B33). */
    r = (int)func_8005E884();

    /* 17. func_8005E850(0, 8-r) — REAL (B34).  Internally routes
     * func_8006A2E8 through the centralized boundary. */
    func_8005E850(0, 8 - r);

    /* 18. func_800649D0(0) — REAL (B35). */
    func_800649D0(0);

    /* 19. func_80052790(1) — REAL (B36).  Internally routes
     * func_80086728 through the centralized boundary. */
    func_80052790(1);

    /* 20. Terminator bytes at the two record-buffer bases. */
    PE_StoreU8(GA_5D6F4_TERM_A4, 0xFFu);
    PE_StoreU8(GA_5D6F4_TERM_B4, 0xFFu);

    return 0xFF;
}
