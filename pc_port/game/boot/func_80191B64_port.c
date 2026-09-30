/*
 * Phase 6E-MV1a — func_80191B64 (152 words, 0x80191B64..0x80191DC0,
 * PE.IMG overlay): movie-issue poll worker.  Transcribed; every jal
 * target is translated (7F72C/7F7A8/719E4/7C484/870F0/74F44).
 *
 * Frame temporaries (sp+16/sp+20/sp+24[RECT]) become the rung
 * scratch cells below: sp+16 takes 7C484's dst_a output and is read
 * back as the return value; sp+20 takes dst_b (slot pointer);
 * sp+24 is the 74F44 fill rect.  (7F7A8 ignores its incoming a0 —
 * the matched leaf takes no address — so sp+16 is unwritten until
 * 7C484; 719E4 takes the literal 1, set in the jal delay slot.)
 *
 * Delay-slot notes: the BB0 beqz delay decrements s0 on every pass
 * (zero breaks to BD0, nonzero retries to the DB0 return-0); the
 * bnez delay re-sets a0 = sp+16 for retries; the C84 store is
 * skipped only when w < the record end frame (second bnez), so both
 * w < last and w >= end latch [DBD]; D00 stores h=480 on
 * both rect arms (w = [B0DBB] ? 480 : 320); the D38 signed path
 * keeps full 32-bit wrap so the low half matches the srl chain.
 * Fixed-point: ((int64)a0 * 0x92492493 >> 32) + a0, then >> 3
 * arithmetic — exact mult/mfhi/sra replication.  All sltu compares
 * are unsigned over sign-extended halfwords.
 */

#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_port_compat.h"

#define MV1_SP16 0x801FFF20u
#define MV1_SP20 0x801FFF24u
#define MV1_RECT 0x801FFF28u

extern int func_8007C484(pe_addr_t dst_a, pe_addr_t dst_b);
extern void func_800870F0(uint32_t v);


int func_80191B64(pe_addr_t state)
{
    uint32_t s0 = 2000u;
    int32_t s1 = (int32_t)state;
    int32_t v0;

    v0 = func_8007F72C();
    if (v0 == 1) {
        int32_t v1 = func_8007F7A8();
        if ((uint32_t)v1 != (uint32_t)PE_LoadU16(0x800B0DD4u))
            func_800719E4(1u);
    }
    /* BB0 poll: 7C484(sp+16, sp+20).  The beqz delay decrements s0
     * on every pass: zero breaks to BD0, nonzero retries until the
     * 2000-countdown exhausts (DB0: return 0).  (First-pass a0 is a
     * retail quirk — 1 or stale unless a delay slot set sp+16; the
     * port always passes the scratch cell, matching every retry.
     * The quirk only matters on first-pass promote, which needs
     * lane-1 + a changed 7F7A8 head to even reach.) */
    for (;;) {
        v0 = func_8007C484(MV1_SP16, MV1_SP20);
        s0--;
        if (v0 == 0)
            break;
        if (s0 == 0u)
            return 0;
    }
    /* BD0 window test; C0C fixed-point scale on the else arm. */
    {
        uint32_t rec = PE_LoadU32(0x801D11ACu);
        uint32_t slot = PE_LoadU32(MV1_SP20);
        uint32_t w = PE_LoadU32(slot + 8u);
        int32_t a0 = (int32_t)(int16_t)PE_LoadU16(rec + 8u);
        if (w >= (uint32_t)(a0 - 16)) {
            uint32_t e = 14u - (w + 16u - (uint32_t)a0);
            int32_t v1 = ((int32_t)e >= 0) ? (int32_t)e : 0;
            uint32_t q = (uint32_t)PE_LoadU8(0x800B0DBEu) *
                         (uint32_t)v1;
            int64_t p = (int64_t)(int32_t)q *
                        (int64_t)(int32_t)0x92492493u;
            int32_t v = (int32_t)((uint32_t)(int32_t)(p >> 32) + q);
            int32_t adj = (int32_t)q >> 31;
            v >>= 3;
            func_800870F0((uint32_t)(v - adj));
        }
    }
    /* C44 slot-valid latches (unsigned over sign-extended halves). */
    {
        uint32_t slot = PE_LoadU32(MV1_SP20);
        uint32_t w = PE_LoadU32(slot + 8u);
        uint32_t lim = (uint32_t)(int32_t)(int16_t)PE_LoadU16(
            0x801D11B0u);
        /* 0x80191C58..C88: frame went backwards (w < last) OR reached
         * the movie record's end frame (w >= rec+8) latches [DBD]=1;
         * only w < end skips the C84 store (bnez at 0x80191C7C). */
        if (w < lim) {
            PE_StoreU8(0x801D0DBDu, 1u);
        } else {
            uint32_t rec = PE_LoadU32(0x801D11ACu);
            uint32_t rh = (uint32_t)(int32_t)(int16_t)PE_LoadU16(
                rec + 8u);
            if (w >= rh)
                PE_StoreU8(0x801D0DBDu, 1u);
        }
    }
    /* C8C record publish. */
    {
        uint32_t slot = PE_LoadU32(MV1_SP20);
        int32_t a0 = (int32_t)(int16_t)PE_LoadU16(0x801D0DE0u);
        uint32_t w = PE_LoadU32(slot + 8u);
        uint32_t h1 = (uint32_t)PE_LoadU16(slot + 16u);
        PE_StoreU16(0x801D11B0u, (uint16_t)w);
        if (a0 == (int32_t)h1) {
            int32_t b = (int32_t)(int16_t)PE_LoadU16(0x801D0DE2u);
            uint32_t h2 = (uint32_t)PE_LoadU16(slot + 18u);
            if (b == (int32_t)h2)
                goto copy_dims;
        }
    }
    /* CC8 rect fill: (0,0,[B0DBB]?480:320,480), then refresh DE0/DE2. */
    {
        uint32_t slot = PE_LoadU32(MV1_SP20);
        uint32_t ww = (PE_LoadU8(0x800B0DBBu) != 0u) ? 480u : 320u;
        PE_StoreU16(MV1_RECT, 0u);
        PE_StoreU16(MV1_RECT + 2u, 0u);
        PE_StoreU16(MV1_RECT + 4u, (uint16_t)ww);
        PE_StoreU16(MV1_RECT + 6u, 480u);
        {
            RECT rect;
            rect.x = (int16_t)PE_LoadU16(MV1_RECT + 0u);
            rect.y = (int16_t)PE_LoadU16(MV1_RECT + 2u);
            rect.w = (int16_t)PE_LoadU16(MV1_RECT + 4u);
            rect.h = (int16_t)PE_LoadU16(MV1_RECT + 6u);
            func_80074F44(&rect, 0u, 0u, 0u);
        }
        PE_StoreU16(0x801D0DE0u, PE_LoadU16(slot + 16u));
        PE_StoreU16(0x801D0DE2u, PE_LoadU16(slot + 18u));
    }
    /* D38 signed/unsigned dimension latch. */
    if (PE_LoadU8(0x800B0DBBu) != 0u) {
        int32_t v = (int32_t)(int16_t)PE_LoadU16(0x801D0DE0u);
        uint32_t x = (uint32_t)v + (uint32_t)v + (uint32_t)v;
        uint32_t t = x + (x >> 31);
        uint16_t h = (uint16_t)(t >> 1);
        PE_StoreU16((pe_addr_t)(s1 + 34), h);
        PE_StoreU16((pe_addr_t)(s1 + 26), h);
    } else {
        uint32_t v = (uint32_t)PE_LoadU16(0x801D0DE0u);
        PE_StoreU16((pe_addr_t)(s1 + 34), (uint16_t)v);
        PE_StoreU16((pe_addr_t)(s1 + 26), (uint16_t)v);
    }
copy_dims:
    {
        uint32_t v1 = (uint32_t)PE_LoadU16(0x801D0DE2u);
        PE_StoreU16((pe_addr_t)(s1 + 36), (uint16_t)v1);
        PE_StoreU16((pe_addr_t)(s1 + 28), (uint16_t)v1);
        PE_StoreU16((pe_addr_t)(s1 + 46), (uint16_t)v1);
    }
    return (int32_t)PE_LoadU32(MV1_SP16);
}
