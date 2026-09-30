/*
 * Phase 6E-B16 — func_8006A9E4: PE.IMG streaming resource load
 * (ClearImage + four sector-read/poll cycles + two archive copies).
 *
 * Raw body: 215 words / 0x35C, exe 0x8006A9E4–0x8006AD3F, file offset
 * 0x5B1E4, sole live split asm/disc1/5B1E4.s:12–248; all 215
 * instruction words verified exact against the SHA-exact retail
 * executable (byte-order adjusted spimdisasm comments).
 *
 * Sole call site in the executable: func_8001220C @0x80012284 (jal,
 * nop delay slot), immediately after jal func_8003E680 @0x8001227C and
 * immediately before sw $s3 → D_8009D280 @0x80012290.  The return value
 * is not consumed (v0 never referenced after the call); the function
 * returns void.
 *
 * ROM-order operation map:
 *   1. RECT {x=0, y=0, w=0x3FF, h=0x1FF} on the retail stack;
 *      func_80074F44(&rect, 0, 0, 1) — ClearImage, REAL host SDK
 *      implementation (psx_compat.h "SDK IMPLEMENTED").
 *   2. s2 = lw D_800B0DD8 (PE.IMG LBA, set by the func_800698D4 mount);
 *      s3 = &D_800B0CD8 — both cached in registers for the whole body.
 *   3. Four streaming read/poll cycles; each retry loop re-loads the
 *      destination pointer and the halfword table fields (retail loads
 *      them at the loop top):
 *        A: table D_800930DC (off/end), dest = D_800A8028
 *        B: table D_800930DE (off/end), dest = lw(D_800B0E6C);
 *           calls func_800527C8() at most once per execution inside the
 *           poll loop (retail $s1 one-shot flag, survives -1 restarts)
 *        C: table D_800930E4 (off/end), dest = lw(D_800B0E6C)
 *        D: table D_800930E6 (off/end), dest = lw(D_800B0E6C)
 *      Issue: do { r = func_8006E6A8(s2+off, dest, end-off); } while
 *      (r == -1).  Polls:
 *        A/B poll with func_8006E7E8() until 0; a -1 result RESTARTS
 *        the whole cycle (re-issues the read);
 *        C/D poll with sltu-clamped status (st = (ret != 0)) so the
 *        restart branch is dead retail code — a -1 result is retried
 *        by re-POLLING, never by re-issuing.
 *   4. Copy #1: 0x10A50 bytes from lw(D_800B0E6C) to D_800E2858.
 *      Retail picks lwl/lwr or lw/sw 16-byte-per-iteration loops on
 *      (src|dst)&3; both paths are the same byte-exact copy.
 *   5. Two func_8006E498 archive lookups with exact delay-slot order:
 *        sw &D_800E2858 → D_800B0E20   (BEFORE lookup 1 runs)
 *        r1 = func_8006E498(D_800E2858, 0x57D40D84)
 *        sw r1 → D_800B0E18            (BEFORE lookup 2 runs)
 *        r2 = func_8006E498(lw(D_800B0E20), 0x57D41D84)
 *        sw r2 → D_800B0E1C
 *   6. func_80087090(lw(D_800B0E6C), 1) — SPU-upload retry wrapper
 *      (func_800851A8 loop); UNRESOLVED this rung, routed through the
 *      centralized bootstrap boundary.
 *   7. Copy #2: 0x1400 bytes from lw(D_800B0E6C) to lw(D_800B0E08).
 *      D_800B0E08 is initialized by func_8006A674 to lw(D_800B0E28) +
 *      0x2800 (arena) before func_8006A9E4 runs — never null on this
 *      path; the copy is unconditional retail code with no zero check.
 *   8. jr ra with v0 = last poll leftover; not consumed by the caller.
 *
 * Dependency boundary (per rung rules — proven pre-call behavior is
 * translated, unresolved callees go through the centralized bootstrap
 * boundary, strict mode stops at the FIRST one):
 *   func_80074F44  REAL host SDK (ClearImage)
 *   func_8006E6A8  TRANSLATED this rung (thin sector-count wrapper over
 *                  REAL func_8006E6D4; B54K-AC restores the provider's
 *                  retail sector unit — see func_8006E6A8_port.c)
 *   func_8006E7E8  TRANSLATED this rung (REAL func_800811E4 + RMW)
 *   func_8006E498  TRANSLATED this rung (pure guest table walk)
 *   func_800527C8  TRANSLATED (Phase 6E-B17, multi-subsystem bootstrap
 *                  dispatcher; its own first unresolved callee
 *                  func_800528F0 is the new strict frontier)
 *   func_80087090  TRANSLATED this rung (retry wrapper; its inner
 *                  callee func_800851A8 remains UNRESOLVED)
 *
 * Guest state visible on entry (all produced by earlier translated
 * rungs): D_800B0DD8 = PE.IMG LBA; D_800B0E6C = arena stream buffer;
 * D_800B0E08 = lw(D_800B0E28)+0x2800; D_800930DC..E8 read through the
 * guest-image layer (real values under --disc-image, zeros under the
 * --bootstrap-disc fixture where no exe is loaded — making the cycle
 * sizes 0, which the REAL func_8006E6D4 completes trivially with no
 * active disc; both paths stay deterministic).
 *
 * Write footprint (guest addresses): D_800A8028..+(cycle A bytes),
 * lw(D_800B0E6C)..+(cycles B/C/D bytes), D_800E2858..+0x10A50,
 * D_800B0E18/1C/20, lw(D_800B0E08)..+0x1400, plus the providers'
 * own post-issue state and flag RMWs documented in their files.
 *
 * Classification: 1 — translated retail logic with two unresolved
 * callees routed through the centralized bootstrap boundary.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_bootstrap.h"

/* ── Guest addresses ───────────────────────────────────────────────── */
#define GA_D_800930DC  0x800930DCu   /* halfword LBA off/end table   */
#define GA_D_800A8028  0x800A8028u   /* cycle A destination buffer   */
#define GA_D_800B0DD8  0x800B0DD8u   /* PE.IMG LBA base              */
#define GA_D_800B0E08  0x800B0E08u   /* copy #2 destination pointer  */
#define GA_D_800B0E18  0x800B0E18u   /* lookup result slot 1         */
#define GA_D_800B0E1C  0x800B0E1Cu   /* lookup result slot 2         */
#define GA_D_800B0E20  0x800B0E20u   /* archive base slot            */
#define GA_D_800B0E6C  0x800B0E6Cu   /* stream buffer pointer        */
#define GA_D_800E2858  0x800E2858u   /* archive copy buffer          */

#define PE_6A9E4_COPY1_LEN  0x10A50u /* 67792 bytes                  */
#define PE_6A9E4_COPY2_LEN  0x1400u  /* 5120 bytes                   */

extern int       func_8006E6A8(int lba, pe_addr_t dest, int sectors);
extern int       func_8006E7E8(void);
extern pe_addr_t func_8006E498(pe_addr_t base, uint32_t key);

/* Byte-exact guest→guest copy (checked per word).  Retail uses
 * 16-byte-per-iteration lwl/lwr or lw/sw loops selected on (src|dst)&3;
 * both paths produce the same byte sequence, and both retail lengths
 * are multiples of 16. */
static void PE_6A9E4_Copy(pe_addr_t dst, pe_addr_t src, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i += 4)
        PE_StoreU32(dst + i, PE_LoadU32(src + i));
}

/* One streaming cycle: retry the issue while func_8006E6A8 returns -1,
 * reloading the destination pointer and table fields at the loop top
 * exactly like the retail body. */
static void PE_6A9E4_Issue(uint32_t s2_lba, pe_addr_t table,
                           int use_stream_dest)
{
    int r;
    do {
        pe_addr_t dest = use_stream_dest ? PE_LoadU32(GA_D_800B0E6C)
                                         : GA_D_800A8028;
        uint32_t off   = PE_LoadU16(table);
        uint32_t end   = PE_LoadU16(table + 2);
        r = func_8006E6A8((int)(s2_lba + off), dest, (int)(end - off));
    } while (r == -1);
}

/* A/B poll: func_8006E7E8() until 0; a -1 result restarts the whole
 * cycle.  Returns 1 when a restart is required, 0 when complete. */
static int PE_6A9E4_PollRestartable(void)
{
    int st = 1;
    for (;;) {
        if (st == -1)
            return 1;
        st = func_8006E7E8();
        if (st == 0)
            return 0;
    }
}

/* B poll: identical restart semantics plus the func_800527C8
 * invocation guarded by the caller's $s1 one-shot flag — retail clears
 * $s1 ONCE per func_8006A9E4 execution and sets it in the call's delay
 * slot, so the call happens at most once per streaming load even across
 * poll -1 restarts (the restart branch does not reset $s1). */
static int PE_6A9E4_PollRestartable_B(int *s1)
{
    int st = 1;
    for (;;) {
        if (!*s1) {
            func_800527C8();
            *s1 = 1;
        }
        if (st == -1)
            return 1;
        st = func_8006E7E8();
        if (st == 0)
            return 0;
    }
}

/* C/D poll: sltu-clamped status — st = (ret != 0); the retail restart
 * branch (beq st, -1) is dead code because st ∈ {0, 1}.  A -1 result is
 * retried by re-polling, never by re-issuing. */
static void PE_6A9E4_PollClamped(void)
{
    int st = 1;
    while (st != 0)
        st = (func_8006E7E8() != 0);
}

void func_8006A9E4(void)
{
    RECT     rect;
    uint32_t s2_lba;
    pe_addr_t r1, r2;

    /* 1. ClearImage({0, 0, 0x3FF, 0x1FF}, 0, 0, 1) — REAL host SDK. */
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x3FF;
    rect.h = 0x1FF;
    func_80074F44(&rect, 0, 0, 1);

    /* 2. Register-cached for the whole body (retail $s2/$s3). */
    s2_lba = PE_LoadU32(GA_D_800B0DD8);

    /* 3A. Read cycle A: table D_800930DC, dest D_800A8028. */
    for (;;) {
        PE_6A9E4_Issue(s2_lba, GA_D_800930DC, 0);
        if (!PE_6A9E4_PollRestartable())
            break;
    }

    /* 3B. Read cycle B: table D_800930DE, dest stream buffer;
     *     func_800527C8 at most once per execution (retail $s1 scope
     *     covers the whole cycle, including -1 restarts). */
    {
        int s1 = 0;
        for (;;) {
            PE_6A9E4_Issue(s2_lba, GA_D_800930DC + 2, 1);
            if (!PE_6A9E4_PollRestartable_B(&s1))
                break;
        }
    }

    /* 4. Copy #1: stream buffer → D_800E2858, 0x10A50 bytes. */
    PE_6A9E4_Copy(GA_D_800E2858, PE_LoadU32(GA_D_800B0E6C),
                  PE_6A9E4_COPY1_LEN);

    /* 5. Two archive lookups, exact delay-slot store order. */
    PE_StoreU32(GA_D_800B0E20, GA_D_800E2858);
    r1 = func_8006E498(GA_D_800E2858, 0x57D40D84u);
    PE_StoreU32(GA_D_800B0E18, r1);
    r2 = func_8006E498(PE_LoadU32(GA_D_800B0E20), 0x57D41D84u);
    PE_StoreU32(GA_D_800B0E1C, r2);

    /* 3C. Read cycle C: table D_800930E4, dest stream buffer. */
    PE_6A9E4_Issue(s2_lba, GA_D_800930DC + 8, 1);
    PE_6A9E4_PollClamped();

    /* 6. func_80087090(lw(D_800B0E6C), 1) — SPU upload retry wrapper;
     *    func_80087090 is translated; its inner callee func_800851A8
     *    (SPU DMA upload) remains unresolved. */
    func_80087090(PE_LoadU32(GA_D_800B0E6C), 1);

    /* 3D. Read cycle D: table D_800930E6, dest stream buffer. */
    PE_6A9E4_Issue(s2_lba, GA_D_800930DC + 10, 1);
    PE_6A9E4_PollClamped();

    /* 7. Copy #2: stream buffer → lw(D_800B0E08), 0x1400 bytes. */
    PE_6A9E4_Copy(PE_LoadU32(GA_D_800B0E08), PE_LoadU32(GA_D_800B0E6C),
                  PE_6A9E4_COPY2_LEN);
}
