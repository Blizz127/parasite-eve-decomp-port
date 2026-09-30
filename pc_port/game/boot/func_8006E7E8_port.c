/*
 * Phase 6E-B16 — func_8006E7E8: disc-read completion poll.
 *
 * Raw body: 19 words / 0x4C, exe 0x8006E7E8–0x8006E833, file offset
 * 0x5EFE8, sole live split asm/disc1/5B1E4.s:4522; all 19 instruction
 * words verified exact against the SHA-exact retail executable.
 *
 * Retail semantics:
 *   a0 = sp+0x10 (stack scratch for the collapsed DsDataSync query)
 *   st = func_800811E4(a0)
 *   if ((uint32_t)(st + 1) < 2)          — st ∈ {-1, 0}
 *       D_800B0CD8 &= 0xFEFFBFFF         — clears 0x01004000 in one RMW
 *   return st
 *
 * The func_800811E4 host provider explicitly ignores its fp argument
 * (the DsDataSync query is collapsed, pe_libcd.c), so the retail stack
 * address is replaced with the documented guest scratch address
 * PE_6E7E8_SYNC_ADDR — the same pattern as func_8006E834's
 * PE_6E834_SYNC_ADDR.  No host pointer is ever placed in guest RAM.
 *
 * Classification: 1 — translated retail logic over an already-real
 * provider.  Stateless between calls; safe to invoke repeatedly.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

/* Retail passes sp+0x10; the host uses fixed guest scratch instead. */
#define PE_6E7E8_SYNC_ADDR 0x801FFEA0u

int func_8006E7E8(void)
{
    int st = func_800811E4(PE_6E7E8_SYNC_ADDR);
    if ((uint32_t)(st + 1) < 2u)
        D_800B0CD8 &= 0xFEFFBFFFu;
    return st;
}
