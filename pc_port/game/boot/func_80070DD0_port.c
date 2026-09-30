/*
 * Phase 6E-B2 — func_80070DD0: ranged random wrapper (handwritten retail).
 *
 * Retail evidence (asm/disc1/5F3E4.s:2501-2515, exe 0x80070DD0-0x80070E03,
 * 13 words / 0x34, marked "Handwritten function"):
 *   v1 = ra
 *   v0 = func_80070D6C();  a1 = a1 - a0    (sub in the jal delay slot)
 *   v0 &= 0xFFFF
 *   mult v0, a1                            (SIGNED 32x32 -> hi:lo)
 *   ra = v1
 *   v0 = (lo >> 16) | (hi << 16)           i.e. product >> 16 (64-bit arith)
 *   return a0 + v0                         (add in the jr delay slot)
 *
 * So: func_80070DD0(a, b) = a + ((int64)(rand & 0xFFFF) * (b - a) >> 16),
 * with exact 32-bit wrap on the final add.  Implemented because the Phase
 * 6E-B2 contract tests exercise its boundary behavior directly; it is not
 * on the current strict-mode frontier (its callers 0x8001585C/0x80015888/
 * func_800176FC are not on the boot-to-black path).
 */
#include "psx_compat.h"

int func_80070DD0(int a0, int a1)
{
    unsigned int r = func_80070D6C() & 0xFFFFu;
    int diff = (int)((unsigned int)a1 - (unsigned int)a0);   /* sub */
    long long product = (long long)(int)r * (long long)diff; /* mult */
    int scaled = (int)(product >> 16);     /* (lo>>16)|(hi<<16), arith */
    return (int)((unsigned int)a0 + (unsigned int)scaled);   /* add wrap */
}
